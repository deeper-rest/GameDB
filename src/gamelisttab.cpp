#include "gamelisttab.h"
#include "gamedetailwidget.h"
#include "gameinfodialog.h"
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

GameListTab::GameListTab() {
    setupUI();
    refreshList();
    
    connect(&GameManager::instance(), &GameManager::libraryUpdated, this, &GameListTab::refreshList);
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
    
    // View Toggle
    this->viewToggleBtn = new QToolButton();
    this->viewToggleBtn->setText("Card"); // Button says what to switch TO (or current mode? usually toggle)
    // Let's make it checkable: Unchecked = List, Checked = Card. Text can indicate current or target.
    // Let's stick to text indicating the OTHER view, or Icons.
    // Simple text for now: "Card View"
    this->viewToggleBtn->setText("Card View");
    this->viewToggleBtn->setCheckable(true);
    connect(this->viewToggleBtn, &QToolButton::clicked, this, &GameListTab::onViewToggle);
    topLayout->addWidget(this->viewToggleBtn);

    layout->addLayout(topLayout);
    
    this->viewStack = new QStackedWidget();

    // Table
    this->gameTable = new QTableWidget();
    this->gameTable->setColumnCount(6); // Name, Folder Name, Type, Korean, Tags, Path
    this->gameTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Folder Name" << "Type" << "Korean" << "Tags" << "Path");
    this->gameTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->gameTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    this->gameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->gameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->gameTable->setContextMenuPolicy(Qt::CustomContextMenu);
    this->gameTable->setIconSize(QSize(48, 48)); // Set icon size
    
    connect(this->gameTable, &QTableWidget::cellClicked, this, &GameListTab::onRowClicked);
    connect(this->gameTable, &QTableWidget::cellDoubleClicked, this, &GameListTab::onDoubleClicked);
    connect(this->gameTable, &QTableWidget::customContextMenuRequested, this, &GameListTab::showContextMenu);
    
    this->viewStack->addWidget(this->gameTable);
    
    // Card List
    this->gameListWidget = new QListWidget();
    this->gameListWidget->setViewMode(QListWidget::IconMode);
    this->gameListWidget->setIconSize(QSize(320, 240)); // Adjusted Portrait size
    this->gameListWidget->setResizeMode(QListWidget::Adjust);
    this->gameListWidget->setGridSize(QSize(360, 260)); // Enforce grid cell size
    this->gameListWidget->setMovement(QListView::Static);
    this->gameListWidget->setSpacing(15);
    this->gameListWidget->setUniformItemSizes(true);
    this->gameListWidget->setWordWrap(true);
    
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
        
        this->gameTable->setItem(row, 5, new QTableWidgetItem(game.filePath));
        
        // Add to List Widget
        QListWidgetItem *listItem = new QListWidgetItem(game.cleanName);
        if (!game.thumbnailPath.isEmpty() && QFile::exists(game.thumbnailPath)) {
            listItem->setIcon(QIcon(game.thumbnailPath));
        } else {
            // Placeholder
            QPixmap placeholder(320, 240);
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
    
    QString path = this->gameTable->item(row, 5)->text(); // Path is now col 5
    
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
    // Get GameItem data. 
    // Wait, since we are inserting/removing rows, maintaining mapping to GameManager list is tricky if we use index.
    // Ideally we store hidden data in the row?
    // Current refreshList populates table 1:1 with filtered list.
    // If we filter, index matches the filtered list.
    // BUT since we insert a row, the visual index mismatches the list index for subsequent items.
    // Solution: Get data BEFORE inserting row.
    // Identify which game it is from the Hidden Path Column (Col 5).
    
    QTableWidgetItem *pathItem = this->gameTable->item(row, 5);
    if (!pathItem) return;
    QString path = pathItem->text();
    
    GameItem item = GameManager::instance().getGameByPath(path);
    
    // Insert Row
    this->gameTable->insertRow(row + 1);
    this->gameTable->setSpan(row + 1, 0, 1, 6); // Span all columns
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
