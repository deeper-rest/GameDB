#ifndef GAMELISTTAB_H
#define GAMELISTTAB_H

#include <QWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QToolButton>
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
    void onTypeFilterChanged(int index);
    void onEditGameRequested(QString path);

    // Card View Slots
    void onViewToggle();
    void onCardClicked(QListWidgetItem *item);
    
    // Sort Slots
    void onSortChanged(int index);
    void onSortOrderChanged();

private:
    void setupUI();
    
    QLineEdit *searchEdit;
    QComboBox *typeFilterCombo;
    QToolButton *viewToggleBtn;
    
    // Sorting
    QComboBox *sortCombo;
    QToolButton *sortOrderBtn;
    bool sortAscending = false; // Default Descending (Newest/Z-A) or Ascending? Usually Asc for Name, Desc for Date.

    QStackedWidget *viewStack;
    QTableWidget *gameTable;
    QListWidget *gameListWidget;
    
    int expandedRow;
};

#endif // GAMELISTTAB_H
