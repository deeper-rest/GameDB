#ifndef FILELISTTAB_H
#define FILELISTTAB_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QList>
#include <QHeaderView>

class FileListTab : public QWidget {
    Q_OBJECT

public:
    FileListTab();

    void addItems(QStringList list);

private:
    void setMainUI();

    QListWidget *mainList;
};

#endif