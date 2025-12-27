#include "mainwindow.h"


MainWindow::MainWindow() {
    setMainUI();
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

    this->mainTabWidget->addTab(this->fileListTab, tr("파일/폴더 목록"));
    this->mainTabWidget->addTab(new QWidget(), "Test2");
}

void MainWindow::getDirPath() {
    this->dirPath = QFileDialog::getExistingDirectory(this, tr("폴더 선택"), "./", QFileDialog::ShowDirsOnly);

    QStringList list;
    foreach(QString item, QDir(this->dirPath).entryList(QDir::AllDirs)){
        // qDebug() << item.absoluteFilePath();
        if (item != "." && item != "..")
            list << item;
    }
    // qDebug() << list;
    this->fileListTab->addItems(list);

}