#include "gamelisttab.h"
#include "gamedetailwidget.h"
#include "gameinfodialog.h"
#include "gamecarddelegate.h"
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
#include "tagmanager.h"

GameListTab::GameListTab() {
    setupUI();
    refreshList();
    
    connect(&GameManager::instance(), &GameManager::libraryUpdated, this, &GameListTab::refreshList);
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
    this->tagFilterCombo = new QComboBox();
    this->tagFilterCombo->addItem("All Tags", "All");
    updateTagFilterCombo();
    connect(this->tagFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GameListTab::onTagFilterChanged);
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
    this->sortOrderBtn->setText("Asc"); // Default Asc for Name
    this->sortOrderBtn->setCheckable(true); // Checked = Desc? Or just toggle text/icon
    this->sortOrderBtn->setFixedWidth(50); // Fixed Width for UI Stability
    connect(this->sortOrderBtn, &QToolButton::clicked, this, &GameListTab::onSortOrderChanged);
    topLayout->addWidget(this->sortOrderBtn);

    layout->addLayout(topLayout);
    
    this->viewStack = new QStackedWidget();

    // Table
    this->gameTable = new QTableWidget();
    this->gameTable->setColumnCount(7); // Name, Folder Name, Type, Korean, Tags, Last Played, Path
    this->gameTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Folder Name" << "Type" << "Korean" << "Tags" << "Last Played" << "Path");
    this->gameTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    // this->gameTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch); // Too Wide
    
    // Set explicit widths
    this->gameTable->setColumnWidth(4, 150); // Tags - narrower
    this->gameTable->setColumnWidth(5, 140); // Last Played - wide enough for time
    this->gameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->gameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->gameTable->setContextMenuPolicy(Qt::CustomContextMenu);
    this->gameTable->setIconSize(QSize(48, 48)); // Set icon size
    
    // Enable Header Click
    connect(this->gameTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &GameListTab::onHeaderClicked);
    
    connect(this->gameTable, &QTableWidget::cellClicked, this, &GameListTab::onRowClicked);
    connect(this->gameTable, &QTableWidget::cellDoubleClicked, this, &GameListTab::onDoubleClicked);
    connect(this->gameTable, &QTableWidget::customContextMenuRequested, this, &GameListTab::showContextMenu);
    
    this->viewStack->addWidget(this->gameTable);
    
    // Card List
    this->gameListWidget = new QListWidget();
    this->gameListWidget->setViewMode(QListWidget::IconMode);
    this->gameListWidget->setIconSize(QSize(320, 200)); // Standard 16:10 aspect
    this->gameListWidget->setResizeMode(QListWidget::Adjust);
    // Grid Size needs to be larger than Icon Size to fit text
    // Icon Height 200 + Text Space (approx 60-80px)
    this->gameListWidget->setGridSize(QSize(340, 280)); 
    this->gameListWidget->setMovement(QListView::Static);
    this->gameListWidget->setSpacing(15);
    this->gameListWidget->setUniformItemSizes(true);
    this->gameListWidget->setWordWrap(true);
    
    // Set Custom Delegate
    this->gameListWidget->setItemDelegate(new GameCardDelegate(this->gameListWidget));
    
    connect(this->gameListWidget, &QListWidget::itemClicked, this, &GameListTab::onCardClicked);
    this->viewStack->addWidget(this->gameListWidget);

    layout->addWidget(this->viewStack);
}

void GameListTab::refreshList() {
    this->gameTable->setRowCount(0);
    this->gameListWidget->clear();
    
    this->expandedRow = -1; // Reset expansion on refresh
    QList<GameItem> games = GameManager::instance().getGames();
    QString filter = this->searchEdit->text().toLower();
    int infoTypeData = this->typeFilterCombo->currentData().toInt();
    QString tagFilter = this->tagFilterCombo->currentData().toString();
    
    // Filter First
    QList<GameItem> filteredGames;
    for (const auto &game : games) {
        // Name Filter
        if (!filter.isEmpty() && !game.cleanName.toLower().contains(filter) && !game.folderName.toLower().contains(filter)) {
            continue;
        }
        // Type Filter
        if (infoTypeData != -1) {
            if (static_cast<int>(game.type) != infoTypeData) {
                continue;
            }
        }
        // Tag Filter
        if (tagFilter != "All") {
            if (!game.tags.contains(tagFilter)) {
                continue;
            }
        }
        filteredGames.append(game);
    }
    
    // Sort
    int sortIndex = this->sortCombo->currentIndex(); // 0: Name, 1: Last Played
    // Map table columns to sort index if needed, or use a separate sort state.
    // Let's use sortCombo as the source of truth for "Type" of sort.
    // If we click header, we should update sortCombo if possible, or support more sort types.
    // Current Combo: Name(0), Last Played(1).
    // Table Cols: Name(0), Folder(1), Type(2), Korean(3), Tags(4), LastPlayed(5), Path(6)
    
    // Extended Sort Logic
    // We can use the column index directly for sorting if we extend the enum/logic.
    // Let's rely on sortCombo's user data or just index for simple cases, 
    // BUT since we want to support ALL columns, we need a member variable `currentSortColumn`?
    // OR we just map visual columns to sort logic.
    
    // To keep it simple and sync with UI:
    // If sortIndex is 0 (Name) -> Sort by Name
    // If sortIndex is 1 (Last Played) -> Sort by Date
    // What if user clicked "Folder"? There is no combo item for it.
    // We should probably add items to combo OR just use internal state and update combo text to "Custom" or match if possible.
    
    // Let's add all columns to combo?
    // 0: Name, 1: Folder, 2: Type, 3: Korean, 4: Tags, 5: Last Played.
    // This requires updating setupUI. Let's do that first (in next step or assume done).
    // For now, let's assume we update the combo to have all these.
    
    bool ascending = !this->sortOrderBtn->isChecked(); 
    
    std::sort(filteredGames.begin(), filteredGames.end(), [sortIndex, ascending](const GameItem &a, const GameItem &b) {
        // Sort Logic based on Index
        // 0: Name, 1: Folder, 2: Type, 3: Korean, 4: Tags, 5: Last Played
        bool result = false;
        if (sortIndex == 0) result = a.cleanName < b.cleanName;
        else if (sortIndex == 1) result = a.folderName < b.folderName;
        else if (sortIndex == 2) result = static_cast<int>(a.type) < static_cast<int>(b.type);
        else if (sortIndex == 3) result = a.koreanSupport < b.koreanSupport;
        else if (sortIndex == 4) result = a.tags.join("") < b.tags.join(""); // Simplified tag sort
        else if (sortIndex == 5) result = a.lastPlayed < b.lastPlayed;
        else result = a.cleanName < b.cleanName;
        
        return ascending ? result : !result;
    });

    for (const auto &game : filteredGames) {        
        int row = this->gameTable->rowCount();
        this->gameTable->insertRow(row);
        
        QTableWidgetItem *nameItem = new QTableWidgetItem(game.cleanName);
        if (!game.thumbnailPath.isEmpty() && QFile::exists(game.thumbnailPath)) {
            QIcon icon(game.thumbnailPath);
            nameItem->setIcon(icon);
        }
        this->gameTable->setItem(row, 0, nameItem);
        this->gameTable->setItem(row, 1, new QTableWidgetItem(game.folderName));
        
        QString typeStr;
        switch(game.type) {
            case GameType::Folder: typeStr = "Folder"; break;
            case GameType::Zip: typeStr = "Zip"; break;
            case GameType::SevenZip: typeStr = "7z"; break;
            case GameType::Rar: typeStr = "Rar"; break;
            case GameType::Iso: typeStr = "Iso"; break; 
            default: typeStr = "Unknown"; break;
        }
        this->gameTable->setItem(row, 2, new QTableWidgetItem(typeStr));
        
        // Korean
        this->gameTable->setItem(row, 3, new QTableWidgetItem(game.koreanSupport ? "O" : "X"));
        // Center align the O/X
        this->gameTable->item(row, 3)->setTextAlignment(Qt::AlignCenter);
        
        // Tags
        this->gameTable->setItem(row, 4, new QTableWidgetItem(game.tags.join(", ")));
        
        // Last Played
        QString lastPlayedStr = "-";
        if (game.lastPlayed.isValid()) {
            lastPlayedStr = game.lastPlayed.toString("yyyy-MM-dd HH:mm");
        }
        this->gameTable->setItem(row, 5, new QTableWidgetItem(lastPlayedStr));

        this->gameTable->setItem(row, 6, new QTableWidgetItem(game.filePath));
        
        // Add to List Widget
        QListWidgetItem *listItem = new QListWidgetItem(game.cleanName);
        if (!game.thumbnailPath.isEmpty() && QFile::exists(game.thumbnailPath)) {
            listItem->setIcon(QIcon(game.thumbnailPath));
        } else {
            // Placeholder
            QPixmap placeholder(320, 200);
            placeholder.fill(QColor(200, 200, 200)); // Light Gray
            listItem->setIcon(QIcon(placeholder));
        }
        listItem->setData(Qt::UserRole, game.filePath);
        // Optional: Tooltip
        listItem->setToolTip(game.cleanName);
        
        this->gameListWidget->addItem(listItem);
    }
}

void GameListTab::onSearchChanged(const QString &text) {
    refreshList();
}

void GameListTab::onTypeFilterChanged(int index) {
    Q_UNUSED(index);
    refreshList();
}

void GameListTab::onTagFilterChanged(int index) {
    Q_UNUSED(index);
    refreshList();
}

void GameListTab::updateTagFilterCombo() {
    QString current = this->tagFilterCombo->currentData().toString();
    this->tagFilterCombo->blockSignals(true);
    this->tagFilterCombo->clear();
    this->tagFilterCombo->addItem("All Tags", "All");
    
    QStringList tags = TagManager::instance().getTags();
    tags.sort(); // Sort tags alphabetically
    
    for (const QString &tag : tags) {
        this->tagFilterCombo->addItem(tag, tag);
    }
    
    // Restore selection if possible
    int idx = this->tagFilterCombo->findData(current);
    if (idx != -1) this->tagFilterCombo->setCurrentIndex(idx);
    else this->tagFilterCombo->setCurrentIndex(0);
    
    this->tagFilterCombo->blockSignals(false);
}

void GameListTab::onSortChanged(int index) {
    Q_UNUSED(index);
    refreshList();
}

void GameListTab::onSortOrderChanged() {
    if (this->sortOrderBtn->isChecked()) {
        this->sortOrderBtn->setText("Desc");
    } else {
        this->sortOrderBtn->setText("Asc");
    }
    refreshList();
}

void GameListTab::onHeaderClicked(int logicalIndex) {
    if (logicalIndex < 0 || logicalIndex > 5) return; // Ignore Path column (6)
    
    // Check if we are already sorting by this column
    if (this->sortCombo->currentIndex() == logicalIndex) {
        // Toggle Order
        this->sortOrderBtn->click(); // Simulate click to toggle state and refresh
    } else {
        // Change Column
        // We need to ensure Sort Combo has items for 0..5
        this->sortCombo->setCurrentIndex(logicalIndex);
        // Default to Ascending when switching columns? Or keep?
        // Let's keep current order or reset to Asc.
        // Usually reset to Asc is standard.
        if (this->sortOrderBtn->isChecked()) {
            this->sortOrderBtn->click(); // Reset to Asc
        }
    }
}

void GameListTab::onDoubleClicked(int row, int column) {
    Q_UNUSED(row);
    Q_UNUSED(column);
    // Double click action disabled as per user request
}

void GameListTab::showContextMenu(const QPoint &pos) {
    QPoint globalPos = this->gameTable->viewport()->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.addAction("Open Location", this, SLOT(openFileLocation()));
    myMenu.addAction("Remove from Library", this, SLOT(removeGame()));

    myMenu.exec(globalPos);
}

void GameListTab::openFileLocation() {
    int row = this->gameTable->currentRow();
    if (row < 0) return;
    onDoubleClicked(row, 0);
}

void GameListTab::removeGame() {
    int row = this->gameTable->currentRow();
    if (row < 0) return;
    
    QString path = this->gameTable->item(row, 6)->text(); // Path is now col 6
    
    if (QMessageBox::question(this, "Remove Game", "Are you sure you want to remove this game from the library?") == QMessageBox::Yes) {
        GameManager::instance().removeGameByPath(path);
    }
}

void GameListTab::onRowClicked(int row, int column) {
    Q_UNUSED(column);
    
    // 1. If logic for detail row
    if (this->expandedRow != -1 && row == this->expandedRow + 1) {
        return; // Clicked on the detail widget itself (though usually consumed by widget)
    }
    
    // 2. Collapse existing if any
    if (this->expandedRow != -1) {
        int oldExpanded = this->expandedRow;
        this->gameTable->removeRow(oldExpanded + 1);
        this->expandedRow = -1;
        
        // Adjust clicked row index if it was below the expanded row
        if (row > oldExpanded + 1) {
            row--; 
        } else if (row == oldExpanded) {
            // Clicked the same row to collapse
            return;
        }
    }
    
    // 3. Expand new row
    
    QTableWidgetItem *pathItem = this->gameTable->item(row, 6);
    if (!pathItem) return;
    QString path = pathItem->text();
    
    GameItem item = GameManager::instance().getGameByPath(path);
    
    // Insert Row
    this->gameTable->insertRow(row + 1);
    this->gameTable->setSpan(row + 1, 0, 1, 7); // Span all columns (now 7)
    this->gameTable->setRowHeight(row + 1, 280); // Height for detail widget
    
    GameDetailWidget *detail = new GameDetailWidget(item);
    connect(detail, &GameDetailWidget::playGame, this, &GameListTab::runGame);
    connect(detail, &GameDetailWidget::openFolder, this, &GameListTab::openGameFolder);
    connect(detail, &GameDetailWidget::requestEdit, this, &GameListTab::onEditGameRequested);
    
    this->gameTable->setCellWidget(row + 1, 0, detail);
    
    this->expandedRow = row;
}

void GameListTab::runGame(QString exePath) {
    if (exePath.isEmpty()) return;
    
    QFileInfo info(exePath);
    QString workingDir = info.absolutePath();
    
    // Update Last Played
    // We need to find which game corresponds to this exe path.
    // For now, if we trust the context, we should pass the game path or item.
    // Actually, GameManager's updateLastPlayed takes a 'path' (filePath of the game item).
    // The runGame slot receives exePath... wait.
    // The signals from GameDetailWidget send 'exePath'.
    // We need to know which GameItem triggered this.
    // BUT! Since GameItems have unique paths, we can search for the game with this exePath?
    // Not unique enough if multiple games share same exe? Unlikely for folder games.
    // Better: Iterate games and find one with this exePath.
    
    // Optimization: Pass GameItem path to runGame?
    // Let's stick to existing signature for now and search.
    
    QList<GameItem> games = GameManager::instance().getGames();
    for (const auto &g : games) {
        // If it's a folder game, exePath matches g.exePath.
        // If it's an archive, exePath might be temp?
        // Let's assume folder games for now as that's the main focus.
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
    
    // Include GameInfoDialog header if not already included? 
    // It's likely needed. Let's assume it is or add it.
    // Wait, need to check includes.
    
    GameInfoDialog dialog(item, this);
    if (dialog.exec() == QDialog::Accepted) {
        GameManager::instance().updateGame(dialog.getGameItem());
    }
}

void GameListTab::onViewToggle() {
    if (this->viewToggleBtn->isChecked()) {
        this->viewStack->setCurrentWidget(this->gameListWidget);
        this->viewToggleBtn->setText("List View");
    } else {
        this->viewStack->setCurrentWidget(this->gameTable);
        this->viewToggleBtn->setText("Card View");
    }
}

void GameListTab::onCardClicked(QListWidgetItem *item) {
    if (!item) return;
    
    QString path = item->data(Qt::UserRole).toString();
    GameItem gameItem = GameManager::instance().getGameByPath(path);
    if (gameItem.filePath.isEmpty()) return;

    QDialog dialog(this);
    dialog.setWindowTitle(gameItem.cleanName);
    dialog.setMinimumWidth(600);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    // Remove margins for cleaner look
    layout->setContentsMargins(0, 0, 0, 0); 
    
    GameDetailWidget *detail = new GameDetailWidget(gameItem);
    layout->addWidget(detail);
    
    connect(detail, &GameDetailWidget::playGame, this, &GameListTab::runGame);
    connect(detail, &GameDetailWidget::openFolder, this, &GameListTab::openGameFolder);
    connect(detail, &GameDetailWidget::requestEdit, [&](QString p){
        dialog.accept(); // Close the detail dialog
        this->onEditGameRequested(p);
    });
    
    dialog.exec();
}
