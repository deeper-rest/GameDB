#ifndef FILELISTTAB_H
#define FILELISTTAB_H

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QMap>
#include "gamedata.h"

class FileListTab : public QWidget {
    Q_OBJECT

public:
    FileListTab();

    void addGameItem(const GameItem &item);
    void clearItems();

signals:
    void scanRequested(const QString &path);
    void requestAddGame(const GameItem &item);

private slots:
    void showContextMenu(const QPoint &pos);
    void openFileLocation();
    void renameFolder();
    void onItemExpanded(QTreeWidgetItem *item);
    void onDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void setMainUI();
    QTreeWidget *mainTree;
    
    // Helper to find parent items quickly.
    // Key: Absolute Path of the folder
    // Value: Pointer to QTreeWidgetItem
    QMap<QString, QTreeWidgetItem*> itemMap;
};

#endif