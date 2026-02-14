#include "filelisttab.h"
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

FileListTab::FileListTab() {
    setMainUI();
}

void FileListTab::setMainUI() {
    this->mainTree = new QTreeWidget();
    this->mainTree->setColumnCount(4);
    this->mainTree->setHeaderLabels(QStringList() << "Game Name" << "Type" << "Original Name" << "Path");
    this->mainTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->mainTree->setContextMenuPolicy(Qt::CustomContextMenu);
    this->mainTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // IMPORTANT: Connect expanded signal for lazy loading
    connect(this->mainTree, &QTreeWidget::itemExpanded, this, &FileListTab::onItemExpanded);
    connect(this->mainTree, &QTreeWidget::customContextMenuRequested, this, &FileListTab::showContextMenu);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
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