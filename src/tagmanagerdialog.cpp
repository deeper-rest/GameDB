#include "tagmanagerdialog.h"
#include "tagmanager.h"
#include <QInputDialog>
#include <QMessageBox>

TagManagerDialog::TagManagerDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Manage Tags"));
    resize(300, 400);
    setupUI();
    refreshList();
    
    // Connect to TagManager signals to auto-refresh if changed externally (or by itself)
    connect(&TagManager::instance(), &TagManager::tagAdded, this, &TagManagerDialog::refreshList);
    connect(&TagManager::instance(), &TagManager::tagRenamed, this, &TagManagerDialog::refreshList);
    connect(&TagManager::instance(), &TagManager::tagRemoved, this, &TagManagerDialog::refreshList);
}

void TagManagerDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    this->tagList = new QListWidget();
    connect(this->tagList, &QListWidget::itemSelectionChanged, this, &TagManagerDialog::onSelectionChanged);
    mainLayout->addWidget(this->tagList);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    this->addButton = new QPushButton(tr("Add"));
    this->renameButton = new QPushButton(tr("Rename"));
    this->removeButton = new QPushButton(tr("Remove"));
    
    this->renameButton->setEnabled(false);
    this->removeButton->setEnabled(false);
    
    connect(this->addButton, &QPushButton::clicked, this, &TagManagerDialog::addTag);
    connect(this->renameButton, &QPushButton::clicked, this, &TagManagerDialog::renameTag);
    connect(this->removeButton, &QPushButton::clicked, this, &TagManagerDialog::removeTag);
    
    btnLayout->addWidget(this->addButton);
    btnLayout->addWidget(this->renameButton);
    btnLayout->addWidget(this->removeButton);
    mainLayout->addLayout(btnLayout);
    
    this->closeButton = new QPushButton(tr("Close"));
    connect(this->closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(this->closeButton);
}

void TagManagerDialog::refreshList() {
    this->tagList->clear();
    QStringList tags = TagManager::instance().getTags();
    this->tagList->addItems(tags);
    
    this->renameButton->setEnabled(false);
    this->removeButton->setEnabled(false);
}

void TagManagerDialog::onSelectionChanged() {
    bool hasSelection = !this->tagList->selectedItems().isEmpty();
    this->renameButton->setEnabled(hasSelection);
    this->removeButton->setEnabled(hasSelection);
}

void TagManagerDialog::addTag() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add Tag"),
                                         tr("Tag Name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        TagManager::instance().addTag(text);
    }
}

void TagManagerDialog::renameTag() {
    QListWidgetItem *item = this->tagList->currentItem();
    if (!item) return;
    
    QString oldTag = item->text();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Rename Tag"),
                                         tr("New Tag Name:"), QLineEdit::Normal,
                                         oldTag, &ok);
    if (ok && !text.isEmpty() && text != oldTag) {
        TagManager::instance().renameTag(oldTag, text);
    }
}

void TagManagerDialog::removeTag() {
    QListWidgetItem *item = this->tagList->currentItem();
    if (!item) return;
    
    QString tag = item->text();
    if (QMessageBox::question(this, tr("Remove Tag"), 
                              tr("Are you sure you want to remove tag '%1'?\nThis will remove it from all games.").arg(tag)) 
        == QMessageBox::Yes) {
        TagManager::instance().removeTag(tag);
    }
}
