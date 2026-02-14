#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <QPushButton>
#include <QStringList>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QThread>

#include <iostream>
#include <string>

#include "filelisttab.h"
#include "gamescanner.h"
#include "gamelisttab.h"
#include "gameinfodialog.h"
#include "gamemanager.h"
#include "tagmanagerdialog.h"

#define MAINBOX_STYLESHEET ".QGroupBox{border:1px solid; border-radius:4px; margin-top:10px; padding: 10px} .QGroupBox::title {subcontrol-origin: margin; subcontrol-position: top left; left: 10px; padding: 0 1px;}"
#define MAINTAB_STYLESHEET R"(QTabWidget::pane {border: 1px solid #aaa; background-color: #ffffff; border-radius: 4px; border-top-left-radius: 0px; padding: 5px;} QTabBar::tab {background-color: #f0f0f0; border: 1px solid #aaa; border-top-left-radius: 4px; border-top-right-radius: 4px; padding: 5px 12px; margin-right: 2px;} QTabBar::tab:hover {background-color: #e8e8e8;} QTabBar::tab:selected {background-color: #ffffff; border-bottom-color: #ffffff;})"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private:
    void setMainUI();
    void setSelectDirUI();
    void setMainTabUI();


    QWidget *mainWidget;
    QGridLayout *mainLayout;
    QLabel *dirPathLabel;
    QStringList *fileList;
    QString dirPath;

    QGridLayout *selectDirLayout;
    QGroupBox *selectDirFrame;
    
    QTabWidget *mainTabWidget;

    FileListTab *fileListTab;
    GameListTab *gameListTab;
    
    GameScanner *scanner;
    QThread *workerThread;

private slots:
    void getDirPath();
    void onGameFound(GameItem item);
    void onScanFinished();
    void showGameInfoDialog(const GameItem &item);
    void openTagManager();
};

#endif