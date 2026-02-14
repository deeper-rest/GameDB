#include "mainwindow.h"

MainWindow::MainWindow() {
    setMainUI();
    
    // Initialize Scanner and Thread
    this->workerThread = new QThread(this);
    this->scanner = new GameScanner();
    this->scanner->moveToThread(this->workerThread);
    
    connect(this->workerThread, &QThread::finished, this->scanner, &QObject::deleteLater);
    connect(this->scanner, &GameScanner::gameFound, this, &MainWindow::onGameFound);
    connect(this->scanner, &GameScanner::scanFinished, this, &MainWindow::onScanFinished);
    
    // We connect signal from UI to Scanner in thread
    // Connect scanRequested directly to scanner::scanDirectory (which is now a slot)
    connect(this->fileListTab, &FileListTab::scanRequested, this->scanner, &GameScanner::scanDirectory);
}

MainWindow::~MainWindow() {
    this->workerThread->quit();
    this->workerThread->wait();
}

void MainWindow::setMainUI() {
    resize(1200, 675);

    this->findDirPath = new QPushButton("파일 위치");
    QObject::connect(this->findDirPath, &QPushButton::clicked, this, &MainWindow::getDirPath);
    
    this->mainLayout = new QGridLayout();
    this->mainWidget = new QWidget();

    setSelectDirUI();
    setMainTabUI();

    this->mainLayout->addWidget(this->selectDirFrame, 0, 0, 1, 1);
    this->mainLayout->addWidget(this->mainTabWidget, 2, 0, 1, 1);
    this->mainLayout->setRowStretch(0, 3);
    this->mainLayout->setRowStretch(1, 1);
    this->mainLayout->setRowStretch(2, 12);
    this->mainLayout->setColumnStretch(0, 1);

    
    this->mainWidget->setLayout(this->mainLayout);

    setCentralWidget(this->mainWidget);
}

void MainWindow::setSelectDirUI() {
    this->selectDirFrame = new QGroupBox(tr("폴더 선택"));
    
    this->selectDirFrame->setStyleSheet(MAINBOX_STYLESHEET);

    this->selectDirLayout = new QGridLayout(this->selectDirFrame);
    this->findDirPath = new QPushButton(tr("선택"));

    QObject::connect(this->findDirPath, &QPushButton::clicked, this, &MainWindow::getDirPath);
    
    this->selectDirLayout->addWidget(this->findDirPath, 0, 0, 1, 1);
}

void MainWindow::setMainTabUI() {
    this->mainTabWidget = new QTabWidget();
    this->mainTabWidget->setStyleSheet(MAINTAB_STYLESHEET);

    this->fileListTab = new FileListTab();
    
    // Forward the signal from tab to scanner (Wait, we did this in constructor? NO. fileListTab didn't exist in constructor before setMainUI call)
    // Actually, `setMainUI` creates `fileListTab`.
    // And `MainWindow` constructor calls `setMainUI` THEN connects.
    // So the pointer is valid.
    
    this->mainTabWidget->addTab(this->fileListTab, tr("파일/폴더 목록"));
    this->mainTabWidget->addTab(new QWidget(), "Test2");
}

void MainWindow::getDirPath() {
    this->dirPath = QFileDialog::getExistingDirectory(this, tr("폴더 선택"), "./", QFileDialog::ShowDirsOnly);
    
    if (this->dirPath.isEmpty()) return;

    // Clear existing
    this->fileListTab->clearItems();

    // Start Scan in Thread
    // To invoke method on object in another thread safely:
    QMetaObject::invokeMethod(this->scanner, "scanDirectory", Qt::QueuedConnection, Q_ARG(QString, this->dirPath));
    
    if (!this->workerThread->isRunning()) {
        this->workerThread->start();
    }
}

void MainWindow::onGameFound(GameItem item) {
    this->fileListTab->addGameItem(item);
}

void MainWindow::onScanFinished() {
    // Scan finished
    // We can update UI status here if we had a status bar
}