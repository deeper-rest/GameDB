#include "thumbnailmanager.h"
#include <QGuiApplication>
#include <QScreen>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <windows.h>
#include <QWindow>

ThumbnailManager::ThumbnailManager(QObject *parent) : QObject(parent) {
    this->process = new QProcess(this);
    this->timer = new QTimer(this);
    this->timer->setSingleShot(true);
    
    connect(this->timer, &QTimer::timeout, this, &ThumbnailManager::onTimeout);
    connect(this->process, &QProcess::finished, this, &ThumbnailManager::onProcessFinished);
}

ThumbnailManager::~ThumbnailManager() {
    if (this->process->state() != QProcess::NotRunning) {
        this->process->terminate();
    }
}

void ThumbnailManager::startCapture(const QString &exePath, int delaySec) {
    if (exePath.isEmpty()) {
        emit captureFailed("Executable path is empty.");
        return;
    }
    
    this->currentExePath = exePath;
    
    QFileInfo info(exePath);
    this->process->setProgram(exePath);
    this->process->setWorkingDirectory(info.absolutePath());
    
    qDebug() << "Starting process:" << exePath;
    this->process->start();
    
    if (!this->process->waitForStarted()) {
         emit captureFailed("Failed to start executable: " + this->process->errorString());
         return;
    }
    
    qDebug() << "Process started with PID:" << this->process->processId();
    qDebug() << "Waiting for" << delaySec << "seconds...";
    
    this->timer->start(delaySec * 1000);
}

// Struct to pass data to EnumWindows callback
struct EnumData {
    DWORD processId;
    HWND bestHandle;
};

// Callback for EnumWindows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    EnumData* data = reinterpret_cast<EnumData*>(lParam);
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    
    if (data->processId != processId) return TRUE; // Not our process
    
    // Check if visible
    if (!IsWindowVisible(hwnd)) return TRUE;
    
    // Check if it has a title (avoid hidden/utility windows)
    int length = GetWindowTextLength(hwnd);
    if (length == 0) return TRUE;
    
    // Found a candidate. 
    // Usually the first visible window with a title is the main window.
    data->bestHandle = hwnd;
    return FALSE; // Stop enumeration
}

void ThumbnailManager::onTimeout() {
    qDebug() << "Timeout reached. Attempting capture...";
    
    if (this->process->state() == QProcess::NotRunning) {
        emit captureFailed("Process exited before capture.");
        return;
    }
    
    captureWindow(this->process->processId());
}

void ThumbnailManager::captureWindow(qint64 pid) {
    EnumData data;
    data.processId = static_cast<DWORD>(pid);
    data.bestHandle = NULL;
    
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
    
    if (data.bestHandle) {
        qDebug() << "Found Window Handle:" << data.bestHandle;
        
        // Bring to front to ensure it's not covered by GameDB
        ShowWindow(data.bestHandle, SW_RESTORE);
        SetForegroundWindow(data.bestHandle);
        
        // Wait a bit for the window to come to front and repaint
        // 500ms should be enough
        QTimer::singleShot(500, [this, pid, bestHandle = data.bestHandle]() {
            doCapture(pid, reinterpret_cast<WId>(bestHandle));
        });
        
    } else {
        emit captureFailed("Could not find a visible window for the process.");
        // Process cleanup if not found? 
        this->process->terminate();
    }
}

void ThumbnailManager::doCapture(qint64 pid, WId windowId) {
    Q_UNUSED(pid);
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        // grabWindow requires WId (which is HWND on Windows)
        QPixmap pixmap = screen->grabWindow(windowId);
        
        if (!pixmap.isNull()) {
            QString path = saveThumbnail(pixmap);
            emit captureFinished(path);
        } else {
            emit captureFailed("Failed to grab window content.");
        }
    } else {
         emit captureFailed("Primary screen not found.");
    }
    
    // Cleanup process after capture attempt
    this->process->terminate();
}

QString ThumbnailManager::saveThumbnail(const QPixmap &pixmap) {
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataLocation);
    if (!dir.exists("thumbnails")) {
        dir.mkdir("thumbnails");
    }
    
    QString filename = QString("thumb_%1.png").arg(QDateTime::currentMSecsSinceEpoch());
    QString fullPath = dir.filePath("thumbnails/" + filename);
    
    if (pixmap.save(fullPath, "PNG")) {
        return fullPath;
    }
    return QString();
}

void ThumbnailManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    // If finished before timeout, we can't capture.
    if (this->timer->isActive()) {
        this->timer->stop();
        // emit captureFailed("Process exited early."); // This might trigger confusingly if we called terminate()
    }
}
