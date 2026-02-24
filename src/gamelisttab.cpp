#include "gamelisttab.h"
#include "gamedetailwidget.h"
#include "gameinfodialog.h"
#include "gamecarddelegate.h"
#include <QTimer>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QFileInfo>
#include <QProcess>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include "tagmanager.h"

// Custom Proxy Model for advanced filtering
class GameListFilterProxyModel : public QSortFilterProxyModel {
public:
    GameListFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

    QString searchText;
    int typeFilter = -1;
    QStringList tagFilters;
    bool filterAllTags = true;

    void updateFilter() {
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        
        GameType type = static_cast<GameType>(sourceModel()->data(index, GameRoles::TypeRole).toInt());
        QString cleanName = sourceModel()->data(index, GameRoles::CleanNameRole).toString().toLower();
        QString folderName = sourceModel()->data(index, GameRoles::FolderNameRole).toString().toLower();
        QStringList tags = sourceModel()->data(index, GameRoles::TagsRole).toStringList();

        if (typeFilter != -1 && static_cast<int>(type) != typeFilter)
            return false;

        if (!searchText.isEmpty() && !cleanName.contains(searchText) && !folderName.contains(searchText))
            return false;

        if (!filterAllTags) {
            for (const QString &t : tagFilters) {
                if (!tags.contains(t))
                    return false;
            }
        }
        return true;
    }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
        int sortCol = sortColumn(); 
        
        // 0: Name, 1: Folder, 2: Type, 3: Korean, 4: Tags, 5: Last Played
        if (sortCol == 2) {
            return sourceModel()->data(left, GameRoles::TypeRole).toInt() < sourceModel()->data(right, GameRoles::TypeRole).toInt();
        } else if (sortCol == 3) {
            bool lKor = sourceModel()->data(left, Qt::ForegroundRole).value<QColor>() == QColor(Qt::darkGreen);
            bool rKor = sourceModel()->data(right, Qt::ForegroundRole).value<QColor>() == QColor(Qt::darkGreen);
            return lKor < rKor;
        } else if (sortCol == 5) {
            return sourceModel()->data(left, GameRoles::LastPlayedRole).toDateTime() < sourceModel()->data(right, GameRoles::LastPlayedRole).toDateTime();
        }
        
        // Default String Sort for Name, Folder Name, Tags
        return sourceModel()->data(left, Qt::DisplayRole).toString() < sourceModel()->data(right, Qt::DisplayRole).toString();
    }
};

GameListTab::GameListTab() : expandedRow(-1) {
    libraryModel = new GameLibraryModel(this);
    proxyModel = new GameListFilterProxyModel(this);
    proxyModel->setSourceModel(libraryModel);
    // Sort logic uses column 0 initially
    proxyModel->sort(0, Qt::AscendingOrder);

    setupUI();
    refreshList();
    
    connect(&TagManager::instance(), &TagManager::tagAdded, [&](const QString &){ updateTagFilterCombo(); });
    connect(&TagManager::instance(), &TagManager::tagRemoved, [&](const QString &){ updateTagFilterCombo(); });
    connect(&TagManager::instance(), &TagManager::tagRenamed, [&](const QString &, const QString &){ updateTagFilterCombo(); });
}

void GameListTab::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Top Bar (Search + Filter)
    QHBoxLayout *topLayout = new QHBoxLayout();
    
    // Search
    this->searchEdit = new QLineEdit();
    this->searchEdit->setPlaceholderText(tr("Search games..."));
    connect(this->searchEdit, &QLineEdit::textChanged, this, &GameListTab::onSearchChanged);
    topLayout->addWidget(this->searchEdit);
    
    // Type Filter
    this->typeFilterCombo = new QComboBox();
    this->typeFilterCombo->addItem("All Types", -1);
    this->typeFilterCombo->addItem("Folder", static_cast<int>(GameType::Folder));
    this->typeFilterCombo->addItem("Zip", static_cast<int>(GameType::Zip));
    this->typeFilterCombo->addItem("7z", static_cast<int>(GameType::SevenZip));
    this->typeFilterCombo->addItem("Rar", static_cast<int>(GameType::Rar));
    this->typeFilterCombo->addItem("Iso", static_cast<int>(GameType::Iso));
    connect(this->typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GameListTab::onTypeFilterChanged);
    topLayout->addWidget(this->typeFilterCombo);
    
    // Tag Filter
    this->tagFilterCombo = new MultiSelectComboBox();
    updateTagFilterCombo();
    connect(this->tagFilterCombo, &MultiSelectComboBox::selectionChanged, this, &GameListTab::onTagFilterChanged);
    topLayout->addWidget(this->tagFilterCombo);
    
    // View Toggle
    this->viewToggleBtn = new QToolButton();
    this->viewToggleBtn->setText("Card View");
    this->viewToggleBtn->setCheckable(true);
    connect(this->viewToggleBtn, &QToolButton::clicked, this, &GameListTab::onViewToggle);
    topLayout->addWidget(this->viewToggleBtn);
    
    // Sort Combo
    this->sortCombo = new QComboBox();
    this->sortCombo->addItem("Name", 0);
    this->sortCombo->addItem("Folder Name", 1);
    this->sortCombo->addItem("Type", 2);
    this->sortCombo->addItem("Korean", 3);
    this->sortCombo->addItem("Tags", 4);
    this->sortCombo->addItem("Last Played", 5);
    connect(this->sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GameListTab::onSortChanged);
    topLayout->addWidget(this->sortCombo);
    
    // Sort Order
    this->sortOrderBtn = new QToolButton();
    this->sortOrderBtn->setText("Asc"); 
    this->sortOrderBtn->setCheckable(true);
    this->sortOrderBtn->setFixedWidth(50);
    connect(this->sortOrderBtn, &QToolButton::clicked, this, &GameListTab::onSortOrderChanged);
    topLayout->addWidget(this->sortOrderBtn);

    layout->addLayout(topLayout);
    
    this->viewStack = new QStackedWidget();

    // Table View
    this->gameTable = new QTableView();
    this->gameTable->setModel(proxyModel);
    
    // Hide Path Column
    this->gameTable->setColumnHidden(6, true);
    
    this->gameTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->gameTable->setColumnWidth(4, 150); 
    this->gameTable->setColumnWidth(5, 140); 
    
    this->gameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->gameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->gameTable->setContextMenuPolicy(Qt::CustomContextMenu);
    this->gameTable->setIconSize(QSize(48, 48)); 
    
    // Disable automatic sorting so we can control it via click
    this->gameTable->setSortingEnabled(false); 
    
    connect(this->gameTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &GameListTab::onHeaderClicked);
    connect(this->gameTable, &QTableView::clicked, this, &GameListTab::onRowClicked);
    connect(this->gameTable, &QTableView::doubleClicked, this, &GameListTab::onDoubleClicked);
    connect(this->gameTable, &QTableView::customContextMenuRequested, this, &GameListTab::showContextMenu);
    
    this->viewStack->addWidget(this->gameTable);
    
    // Card List View
    this->gameListView = new QListView();
    this->gameListView->setModel(proxyModel);
    this->gameListView->setViewMode(QListView::IconMode);
    
    // For icon mode, QListView requires an item delegate to draw custom GameItem cards since data isn't just an icon.
    this->gameListView->setItemDelegate(new GameCardDelegate(this->gameListView));
    
    this->gameListView->setResizeMode(QListView::Adjust);
    this->gameListView->setGridSize(QSize(340, 280)); 
    this->gameListView->setMovement(QListView::Static);
    this->gameListView->setSpacing(15);
    this->gameListView->setUniformItemSizes(true);
    this->gameListView->setWordWrap(true);
    
    connect(this->gameListView, &QListView::clicked, this, &GameListTab::onCardClicked);
    this->viewStack->addWidget(this->gameListView);

    layout->addWidget(this->viewStack);
}

void GameListTab::refreshList() {
    // With Model/View, refreshList just updates the proxy filter state. No UI recreation!
    proxyModel->searchText = this->searchEdit->text().toLower();
    proxyModel->typeFilter = this->typeFilterCombo->currentData().toInt();
    
    proxyModel->tagFilters = this->tagFilterCombo->getSelectedData();
    proxyModel->filterAllTags = proxyModel->tagFilters.contains("All") || proxyModel->tagFilters.isEmpty();
    
    proxyModel->updateFilter();
}

void GameListTab::onSearchChanged(const QString &text) {
    Q_UNUSED(text);
    refreshList();
}

void GameListTab::onTypeFilterChanged(int index) {
    Q_UNUSED(index);
    refreshList();
}

void GameListTab::onTagFilterChanged() {
    // Defer the refresh slightly to keep UI ultra responsive
    QTimer::singleShot(50, this, &GameListTab::refreshList);
}

void GameListTab::updateTagFilterCombo() {
    QStringList currentSelection = this->tagFilterCombo->getSelectedData();
    this->tagFilterCombo->blockSignals(true);
    this->tagFilterCombo->clearItems();
    
    this->tagFilterCombo->addCheckableItem("All Tags", "All");
    
    QStringList tags = TagManager::instance().getTags();
    tags.sort();
    
    for (const QString &tag : tags) {
        this->tagFilterCombo->addCheckableItem(tag, tag);
    }
    
    if (currentSelection.isEmpty()) {
        currentSelection.append("All");
    }
    this->tagFilterCombo->setSelectedData(currentSelection);
    this->tagFilterCombo->blockSignals(false);
}

void GameListTab::onSortChanged(int index) {
    int logicalIndex = this->sortCombo->currentData().toInt();
    if (logicalIndex == -1) logicalIndex = index; // Fallback
    
    Qt::SortOrder order = this->sortOrderBtn->isChecked() ? Qt::DescendingOrder : Qt::AscendingOrder;
    proxyModel->sort(logicalIndex, order);
}

void GameListTab::onSortOrderChanged() {
    if (this->sortOrderBtn->isChecked()) {
        this->sortOrderBtn->setText("Desc");
    } else {
        this->sortOrderBtn->setText("Asc");
    }
    onSortChanged(this->sortCombo->currentIndex());
}

void GameListTab::onHeaderClicked(int logicalIndex) {
    if (logicalIndex < 0 || logicalIndex > 5) return; 
    
    if (this->sortCombo->currentIndex() == logicalIndex) {
        this->sortOrderBtn->click(); 
    } else {
        this->sortCombo->setCurrentIndex(logicalIndex);
        if (this->sortOrderBtn->isChecked()) {
            this->sortOrderBtn->click(); 
        }
    }
}

void GameListTab::onDoubleClicked(const QModelIndex &index) {
    Q_UNUSED(index);
}

void GameListTab::showContextMenu(const QPoint &pos) {
    QPoint globalPos = this->gameTable->viewport()->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.addAction("Open Location", this, SLOT(openFileLocation()));
    myMenu.addAction("Remove from Library", this, SLOT(removeGame()));

    myMenu.exec(globalPos);
}

void GameListTab::openFileLocation() {
    QModelIndex index = this->gameTable->currentIndex();
    if (!index.isValid()) return;
    
    QString path = proxyModel->data(index, GameRoles::FilePathRole).toString();
    openGameFolder(path);
}

void GameListTab::removeGame() {
    QModelIndex index = this->gameTable->currentIndex();
    if (!index.isValid()) return;
    
    QString path = proxyModel->data(index, GameRoles::FilePathRole).toString();
    
    if (QMessageBox::question(this, "Remove Game", "Are you sure you want to remove this game from the library?") == QMessageBox::Yes) {
        GameManager::instance().removeGameByPath(path);
    }
}

void GameListTab::onRowClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    int row = index.row();
    
    // Logic for index expansion using Index widget mapping
    // QTableView handles this via setIndexWidget natively!
    if (this->expandedRow == row) {
        // Collapse
        this->gameTable->setIndexWidget(proxyModel->index(row, 0), nullptr);
        this->gameTable->setRowHeight(row, gameTable->verticalHeader()->defaultSectionSize());
        this->expandedRow = -1;
        return;
    }
    
    // Collapse old
    if (this->expandedRow != -1) {
        this->gameTable->setIndexWidget(proxyModel->index(this->expandedRow, 0), nullptr);
        this->gameTable->setRowHeight(this->expandedRow, gameTable->verticalHeader()->defaultSectionSize());
    }
    
    GameItem item = proxyModel->data(index, GameRoles::GameItemRole).value<GameItem>();
    
    GameDetailWidget *detail = new GameDetailWidget(item);
    connect(detail, &GameDetailWidget::playGame, this, &GameListTab::runGame);
    connect(detail, &GameDetailWidget::openFolder, this, &GameListTab::openGameFolder);
    connect(detail, &GameDetailWidget::requestEdit, this, &GameListTab::onEditGameRequested);
    
    // Expand new row (span across columns inside the index 0)
    this->gameTable->setIndexWidget(proxyModel->index(row, 0), detail);
    this->gameTable->setRowHeight(row, 280); 
    
    this->expandedRow = row;
}

void GameListTab::runGame(QString exePath) {
    if (exePath.isEmpty()) return;
    
    QFileInfo info(exePath);
    QString workingDir = info.absolutePath();
    
    QList<GameItem> games = GameManager::instance().getGames();
    for (const auto &g : games) {
        if (g.exePath == exePath || g.filePath == exePath) {
            GameManager::instance().updateLastPlayed(g.filePath);
            break;
        }
    }

    bool success = QProcess::startDetached(exePath, QStringList(), workingDir);
    if (!success) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to launch game:\n%1").arg(exePath));
    }
}

void GameListTab::openGameFolder(QString path) {
    if (path.isEmpty()) return;
    
    QFileInfo info(path);
    QString openPath = info.absolutePath();
    if (info.isDir()) openPath = info.absoluteFilePath();
    
    QDesktopServices::openUrl(QUrl::fromLocalFile(openPath));
}

void GameListTab::onEditGameRequested(QString path) {
    GameItem item = GameManager::instance().getGameByPath(path);
    if (item.filePath.isEmpty()) return;
    
    GameInfoDialog dialog(item, this);
    if (dialog.exec() == QDialog::Accepted) {
        GameManager::instance().updateGame(dialog.getGameItem());
    }
}

void GameListTab::onViewToggle() {
    if (this->viewToggleBtn->isChecked()) {
        this->viewStack->setCurrentWidget(this->gameListView);
        this->viewToggleBtn->setText("List View");
    } else {
        this->viewStack->setCurrentWidget(this->gameTable);
        this->viewToggleBtn->setText("Card View");
        // Reset row expansion if any
        if (this->expandedRow != -1) {
            this->gameTable->setIndexWidget(proxyModel->index(this->expandedRow, 0), nullptr);
            this->gameTable->setRowHeight(this->expandedRow, gameTable->verticalHeader()->defaultSectionSize());
            this->expandedRow = -1;
        }
    }
}

void GameListTab::onCardClicked(const QModelIndex &index) {
    if (!index.isValid()) return;
    
    GameItem gameItem = proxyModel->data(index, GameRoles::GameItemRole).value<GameItem>();
    if (gameItem.filePath.isEmpty()) return;

    QDialog dialog(this);
    dialog.setWindowTitle(gameItem.cleanName);
    dialog.setMinimumWidth(600);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(0, 0, 0, 0); 
    
    GameDetailWidget *detail = new GameDetailWidget(gameItem);
    layout->addWidget(detail);
    
    connect(detail, &GameDetailWidget::playGame, this, &GameListTab::runGame);
    connect(detail, &GameDetailWidget::openFolder, this, &GameListTab::openGameFolder);
    connect(detail, &GameDetailWidget::requestEdit, [&](QString p){
        dialog.accept(); 
        this->onEditGameRequested(p);
    });
    
    dialog.exec();
}
