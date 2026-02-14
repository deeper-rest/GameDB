#ifndef GAMELISTTAB_H
#define GAMELISTTAB_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QMessageBox>
#include "gamemanager.h"

class GameListTab : public QWidget {
    Q_OBJECT

public:
    GameListTab();
    void refreshList();

private slots:
    void onSearchChanged(const QString &text);
    void onDoubleClicked(int row, int column);
    void showContextMenu(const QPoint &pos);
    void openFileLocation();
    void removeGame();

    void onRowClicked(int row, int column);
    void runGame(QString exePath);
    void openGameFolder(QString path);

private:
    void setupUI();
    
    QLineEdit *searchEdit;
    QTableWidget *gameTable;
    int expandedRow;
};

#endif // GAMELISTTAB_H
