#include "filelisttab.h"
#include "gamemanager.h"
#include <QBrush>
#include <QColor>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QComboBox>
#include <QDir>

FileListTab::FileListTab() {
    setMainUI();
    
    connect(&GameManager::instance(), &GameManager::gameAdded, this, &FileListTab::onGameAdded);
    connect(&GameManager::instance(), &GameManager::gameRemoved, this, &FileListTab::onGameRemoved);
}

void FileListTab::onTypeFilterChanged(int index) {
    Q_UNUSED(index);
    this->currentFilterType = this->typeFilterCombo->currentData().toInt();
    
    // Apply filter to all root items
    for (int i = 0; i < this->mainTree->topLevelItemCount(); ++i) {
        filterItems(this->mainTree->topLevelItem(i));
    }
}

void FileListTab::filterItems(QTreeWidgetItem *item) {
    // Always show the dummy "Loading..." item to allow expansion
    if (item->text(0) == "Loading...") {
        item->setHidden(false);
        return;
    }

    // Determine visibility
    // Logic: Folders are ALWAYS visible to allow navigation.
    // Files are visible if filter is All (-1) OR type matches.
    // Need to parse type from column 1 text or store data?
    // Storing data is better but text parsing is easier given current structure.
    QString typeStr = item->text(1);
    bool isFolder = (typeStr == "Folder");
    
    bool visible = true;
    if (this->currentFilterType != -1) {
        GameType targetType = static_cast<GameType>(this->currentFilterType);
        bool match = false;
        
        switch(targetType) {
            case GameType::Folder: match = (typeStr == "Folder"); break;
            case GameType::Zip: match = (typeStr == "Zip"); break;
            case GameType::SevenZip: match = (typeStr == "7z"); break;
            case GameType::Rar: match = (typeStr == "Rar"); break;
            case GameType::Iso: match = (typeStr == "Iso"); break;
            default: match = false; break;
        }
        visible = match;
    }
    
    item->setHidden(!visible);
    
    // Recurse for children
    for (int i = 0; i < item->childCount(); ++i) {
        filterItems(item->child(i));
    }
}

void FileListTab::setMainUI() {
    this->mainTree = new QTreeWidget();
    this->mainTree->setColumnCount(4);
    this->mainTree->setHeaderLabels(QStringList() << "Game Name" << "Type" << "Original Name" << "Path");
    this->mainTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->mainTree->setContextMenuPolicy(Qt::CustomContextMenu);
    this->mainTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->mainTree->setSortingEnabled(true); // Enable User Sorting
    
    // IMPORTANT: Connect expanded signal for lazy loading
    connect(this->mainTree, &QTreeWidget::itemExpanded, this, &FileListTab::onItemExpanded);
    connect(this->mainTree, &QTreeWidget::customContextMenuRequested, this, &FileListTab::showContextMenu);
    connect(this->mainTree, &QTreeWidget::itemDoubleClicked, this, &FileListTab::onDoubleClicked);

    this->typeFilterCombo = new QComboBox();
    this->typeFilterCombo->addItem("All Types", -1);
    this->typeFilterCombo->addItem("Folder", static_cast<int>(GameType::Folder));
    this->typeFilterCombo->addItem("Zip", static_cast<int>(GameType::Zip));
    this->typeFilterCombo->addItem("7z", static_cast<int>(GameType::SevenZip));
    this->typeFilterCombo->addItem("Rar", static_cast<int>(GameType::Rar));
    this->typeFilterCombo->addItem("Iso", static_cast<int>(GameType::Iso));
    connect(this->typeFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FileListTab::onTypeFilterChanged);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(this->typeFilterCombo);
    mainLayout->addWidget(this->mainTree);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
}

void FileListTab::addGameItem(const GameItem &item) {
    QTreeWidgetItem *parentItem = nullptr;
    
    // Find parent by path
    QFileInfo info(item.filePath);
    QString parentPath = info.absolutePath(); // Should end up being the parent directory's path
    
    // Normalize path separators just in case
    // parentPath = QDir::cleanPath(parentPath);
    
    if (this->itemMap.contains(parentPath)) {
        parentItem = this->itemMap.value(parentPath);
    }
    
    // Create new item
    QTreeWidgetItem *newItem;
    if (parentItem) {
        newItem = new QTreeWidgetItem(parentItem);
    } else {
        newItem = new QTreeWidgetItem(this->mainTree);
    }
    
    // Set text
    newItem->setText(0, item.cleanName);
    QString typeStr;
    switch(item.type) {
        case GameType::Folder: typeStr = "Folder"; break;
        case GameType::Zip: typeStr = "Zip"; break;
        case GameType::SevenZip: typeStr = "7z"; break;
        case GameType::Rar: typeStr = "Rar"; break;
        case GameType::Iso: typeStr = "Iso"; break; 
        default: typeStr = "Unknown"; break;
    }
    newItem->setText(1, typeStr);
    newItem->setText(2, item.originalName);
    newItem->setText(3, item.filePath);
    
    // Save to map if it's a folder, so children can find it
    if (item.type == GameType::Folder) {
        this->itemMap.insert(item.filePath, newItem);
        
        // Add a dummy child to make it expandable (Lazy Load)
        new QTreeWidgetItem(newItem, QStringList() << "Loading...");
    }
    
    // Apply initial filter visibility
    filterItems(newItem);
    
    // Check if already in library
    GameItem existing = GameManager::instance().getGameByPath(item.filePath);
    if (!existing.filePath.isEmpty()) {
        updateItemHighlight(newItem, true);
    }
}

void FileListTab::clearItems() {
    this->mainTree->clear();
    this->itemMap.clear();
}

void FileListTab::onItemExpanded(QTreeWidgetItem *item) {
    // Check if it has a dummy child
    if (item->childCount() == 1 && item->child(0)->text(0) == "Loading...") {
        // Remove dummy
        item->removeChild(item->child(0));
        
        // Request scan for this path
        QString path = item->text(3); // Column 3 is Path
        emit scanRequested(path);
    }
}

void FileListTab::showContextMenu(const QPoint &pos) {
    QPoint globalPos = this->mainTree->viewport()->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.addAction("Open Location", this, SLOT(openFileLocation()));
    myMenu.addAction("Rename to Game Name", this, SLOT(renameFolder()));

    myMenu.exec(globalPos);
}

void FileListTab::openFileLocation() {
    QTreeWidgetItem *item = this->mainTree->currentItem();
    if (!item) return;
    
    QString path = item->text(3);
    QFileInfo info(path);
    QString folderPath = info.absolutePath();
    if (info.isDir()) folderPath = info.absoluteFilePath();
    
    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

void FileListTab::renameFolder() {
    QTreeWidgetItem *item = this->mainTree->currentItem();
    if (!item) return;

    QString currentPath = item->text(3);
    QString newName = item->text(0);
    QFileInfo info(currentPath);
    
    if (!info.isDir()) {
        QMessageBox::warning(this, "Warning", "Renaming is currently supported for folders only.");
        return;
    }

    if (info.fileName() == newName) {
         QMessageBox::information(this, "Info", "Name is already correct.");
         return;
    }

    bool ok;
    QString text = QInputDialog::getText(this, tr("Rename Folder"),
                                         tr("New Name:"), QLineEdit::Normal,
                                         newName, &ok);
    if (ok && !text.isEmpty()) {
        QDir parentDir = info.dir();
        QString newPath = parentDir.absoluteFilePath(text); // Correct API usage? 
        // QDir::absoluteFilePath joins dir path with argument
        
        if (parentDir.rename(info.fileName(), text)) {
            // Update UI
            item->setText(3, newPath);
            item->setText(2, text);
            
            // Allow children map update?
            // If we rename a folder, the path changes.
            // Any children added LATER will look for the NEW path.
            // But immediate children already in the tree are pointers, so they are fine?
            // BUT: If user expands a child folder now, the child folder's path (stored in item) is still OLD path!
            // This is a common issue with TreeViews + Renaming.
            // For now, let's just update the map entry so NEW items can be added correctly.
            // Fixing recursive paths for existing items is complex.
            
            this->itemMap.remove(currentPath);
            this->itemMap.insert(newPath, item);
            
            QMessageBox::information(this, "Success", "Renamed successfully.");
        } else {
            QMessageBox::critical(this, "Error", "Failed to rename folder.\nCheck permissions or if folder is in use.");
        }
    }
}

void FileListTab::onDoubleClicked(QTreeWidgetItem *item, int column) {
    Q_UNUSED(column);
    if (!item) return;
    
    // Construct GameItem from tree item
    GameItem gameItem;
    gameItem.cleanName = item->text(0);
    gameItem.originalName = item->text(2);
    gameItem.filePath = item->text(3);
    
    QString typeStr = item->text(1);
    GameType type = GameType::Unknown;
    if (typeStr == "Folder") type = GameType::Folder;
    else if (typeStr == "Zip") type = GameType::Zip;
    else if (typeStr == "7z") type = GameType::SevenZip;
    else if (typeStr == "Rar") type = GameType::Rar;
    else if (typeStr == "Iso") type = GameType::Iso;
    gameItem.type = type;

    emit requestAddGame(gameItem);
}

void FileListTab::updateItemHighlight(QTreeWidgetItem *item, bool isSaved) {
    int columnCount = item->columnCount();
    if (isSaved) {
        // Light yellow background for saved games
        QBrush highlightBrush(QColor(255, 255, 200)); 
        for (int i = 0; i < columnCount; ++i) {
            item->setBackground(i, highlightBrush);
            item->setForeground(i, QBrush(Qt::black)); // Ensure text is readable
        }
        item->setToolTip(0, "Game already in library");
    } else {
        // Reset color - use default brush to respect theme
        for (int i = 0; i < columnCount; ++i) {
            item->setBackground(i, QBrush()); 
            item->setForeground(i, QBrush());
        }
        item->setToolTip(0, "");
    }
}

void FileListTab::onGameAdded(const GameItem &item) {
    // We need to find the item in the tree.
    // Iterating whole tree might be slow, but let's try finding by match.
    // Since our tree structure is Folder -> File, native findItems might work if we search recursively.
    
    QList<QTreeWidgetItem*> items = this->mainTree->findItems(item.filePath, Qt::MatchExactly | Qt::MatchRecursive, 3);
    for (auto *treeItem : items) {
        updateItemHighlight(treeItem, true);
    }
}

void FileListTab::onGameRemoved(const QString &path) {
    QList<QTreeWidgetItem*> items = this->mainTree->findItems(path, Qt::MatchExactly | Qt::MatchRecursive, 3);
    for (auto *treeItem : items) {
        updateItemHighlight(treeItem, false);
    }
}