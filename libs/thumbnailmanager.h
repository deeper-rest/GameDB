#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QPixmap>
#include <QString>

class ThumbnailManager : public QObject {
    Q_OBJECT

public:
    explicit ThumbnailManager(QObject *parent = nullptr);
    ~ThumbnailManager();
    
    void startCapture(const QString &exePath, int delaySec);

signals:
    void captureFinished(const QString &imagePath);
    void captureFailed(const QString &reason);

private slots:
    void onTimeout();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *process;
    QTimer *timer;
    QString currentExePath;
    
    void captureWindow(qint64 pid);
    void doCapture(qint64 pid, WId windowId);
    QString saveThumbnail(const QPixmap &pixmap);
};

#endif // THUMBNAILMANAGER_H
