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

void MainWindow::openTagManager() {
    TagManagerDialog dialog(this);
    dialog.exec();
}

void MainWindow::setMainUI() {
    resize(1200, 675);

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

    QHBoxLayout *dirLayout = new QHBoxLayout();
    this->dirPathLabel = new QLabel(tr("No Directory Selected"));
    QPushButton *dirBtn = new QPushButton(tr("Select Directory"));
    QPushButton *tagBtn = new QPushButton(tr("Manage Tags"));
    
    connect(dirBtn, &QPushButton::clicked, this, &MainWindow::getDirPath);
    connect(tagBtn, &QPushButton::clicked, this, &MainWindow::openTagManager);
    
    dirLayout->addWidget(this->dirPathLabel);
    dirLayout->addWidget(dirBtn);
    dirLayout->addWidget(tagBtn);
    
    this->selectDirFrame->setLayout(dirLayout);
}

void MainWindow::setMainTabUI() {
    this->mainTabWidget = new QTabWidget();
    this->mainTabWidget->setStyleSheet(MAINTAB_STYLESHEET);

    this->fileListTab = new FileListTab();
    this->gameListTab = new GameListTab();
    
    // Forward the signal from tab to scanner (Wait, we did this in constructor? NO. fileListTab didn't exist in constructor before setMainUI call)
    // Actually, `setMainUI` creates `fileListTab`.
    // And `MainWindow` constructor calls `setMainUI` THEN connects.
    // So the pointer is valid.
    
    this->mainTabWidget->addTab(this->fileListTab, tr("파일/폴더 목록"));
    this->mainTabWidget->addTab(this->gameListTab, tr("게임 목록"));
    
    connect(this->fileListTab, &FileListTab::requestAddGame, this, &MainWindow::showGameInfoDialog);
}

void MainWindow::getDirPath() {
    QString path = QFileDialog::getExistingDirectory(this, tr("폴더 선택"), "./", QFileDialog::ShowDirsOnly);
    
    if (path.isEmpty()) return;
    
    this->dirPath = path;
    if (this->dirPathLabel) this->dirPathLabel->setText(path);

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

void MainWindow::showGameInfoDialog(const GameItem &item) {
    GameInfoDialog dialog(item, this);
    if (dialog.exec() == QDialog::Accepted) {
        GameManager::instance().addGame(dialog.getGameItem());
    }
}