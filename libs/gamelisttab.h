#ifndef GAMELISTTAB_H
#define GAMELISTTAB_H

#include <QWidget>
#include <QTableView>
#include <QListView>
#include <QStackedWidget>
#include <QComboBox>
#include <QToolButton>
#include <QSortFilterProxyModel>
#include "gamemanager.h"
#include "multiselectcombobox.h"
#include "gamelibrarymodel.h"

class GameListFilterProxyModel;

class GameListTab : public QWidget {
    Q_OBJECT

public:
    GameListTab();
    void refreshList();

private slots:
    void onSearchChanged(const QString &text);
    void onDoubleClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &pos);
    void openFileLocation();
    void removeGame();

    void onRowClicked(const QModelIndex &index);
    void runGame(QString exePath);
    void openGameFolder(QString path);
    void onTypeFilterChanged(int index);
    void onEditGameRequested(QString path);

    // Card View Slots
    void onViewToggle();
    void onCardClicked(const QModelIndex &index);
    
    // Sort Slots
    void onSortChanged(int index);
    void onSortOrderChanged();
    void onHeaderClicked(int logicalIndex);
    
    // Tag Filter Slot
    void onTagFilterChanged();
    void updateTagFilterCombo();

private:
    void setupUI();
    
    QLineEdit *searchEdit;
    QComboBox *typeFilterCombo;
    MultiSelectComboBox *tagFilterCombo;
    QToolButton *viewToggleBtn;
    
    // Sorting
    QComboBox *sortCombo;
    QToolButton *sortOrderBtn;
    bool sortAscending = false; 

    QStackedWidget *viewStack;
    QTableView *gameTable;
    QListView *gameListView;
    
    GameLibraryModel *libraryModel;
    GameListFilterProxyModel *proxyModel;
    
    int expandedRow;
};

#endif // GAMELISTTAB_H
