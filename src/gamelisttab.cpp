#include "gamelisttab.h"

GameListTab::GameListTab() {
    setupUI();
    refreshList();
    
    connect(&GameManager::instance(), &GameManager::libraryUpdated, this, &GameListTab::refreshList);
}

void GameListTab::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Search
    this->searchEdit = new QLineEdit();
    this->searchEdit->setPlaceholderText(tr("Search games..."));
    connect(this->searchEdit, &QLineEdit::textChanged, this, &GameListTab::onSearchChanged);
    layout->addWidget(this->searchEdit);
    
    // Table
    this->gameTable = new QTableWidget();
    this->gameTable->setColumnCount(6); // Name, Folder Name, Type, Korean, Tags, Path
    this->gameTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Folder Name" << "Type" << "Korean" << "Tags" << "Path");
    this->gameTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->gameTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    this->gameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->gameTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->gameTable->setContextMenuPolicy(Qt::CustomContextMenu);
    
    connect(this->gameTable, &QTableWidget::cellDoubleClicked, this, &GameListTab::onDoubleClicked);
    connect(this->gameTable, &QTableWidget::customContextMenuRequested, this, &GameListTab::showContextMenu);
    
    layout->addWidget(this->gameTable);
}

void GameListTab::refreshList() {
    this->gameTable->setRowCount(0);
    QList<GameItem> games = GameManager::instance().getGames();
    QString filter = this->searchEdit->text().toLower();
    
    for (const auto &game : games) {
        if (!filter.isEmpty() && !game.cleanName.toLower().contains(filter) && !game.folderName.toLower().contains(filter)) {
            continue;
        }
        
        int row = this->gameTable->rowCount();
        this->gameTable->insertRow(row);
        
        this->gameTable->setItem(row, 0, new QTableWidgetItem(game.cleanName));
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
    }
}

void GameListTab::onSearchChanged(const QString &text) {
    refreshList();
}

void GameListTab::onDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    QString path = this->gameTable->item(row, 5)->text(); // Path is now col 5
    QFileInfo info(path);
    QString folderPath = info.absolutePath();
    if (info.isDir()) folderPath = info.absoluteFilePath();
    
    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
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
