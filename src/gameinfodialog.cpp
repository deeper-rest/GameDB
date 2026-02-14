#include "gameinfodialog.h"

GameInfoDialog::GameInfoDialog(const GameItem &item, QWidget *parent) 
    : QDialog(parent), item(item) {
    setupUI();
    setWindowTitle(tr("Add Game"));
}

void GameInfoDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Name
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(tr("Name:"));
    this->nameEdit = new QLineEdit(this->item.cleanName);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(this->nameEdit);
    mainLayout->addLayout(nameLayout);
    
    // Folder Name
    QHBoxLayout *folderLayout = new QHBoxLayout();
    QLabel *folderLabel = new QLabel(tr("Folder/File Name:"));
    // Derive folder name if empty
    QString folderName = this->item.folderName;
    if (folderName.isEmpty()) {
        QFileInfo info(this->item.filePath);
        folderName = info.fileName();
    }
    this->folderNameEdit = new QLineEdit(folderName);
    this->folderNameEdit->setReadOnly(true); // User requested just to store it, maybe editable? User said "actual game name, folder location, folder name". Let's make it read-only for now as it comes from file system.
    folderLayout->addWidget(folderLabel);
    folderLayout->addWidget(this->folderNameEdit);
    mainLayout->addLayout(folderLayout);

    // Path (Read Only)
    QHBoxLayout *pathLayout = new QHBoxLayout();
    QLabel *pathLabel = new QLabel(tr("Path:"));
    this->pathEdit = new QLineEdit(this->item.filePath);
    this->pathEdit->setReadOnly(true);
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(this->pathEdit);
    mainLayout->addLayout(pathLayout);
    
    // Type
    QHBoxLayout *typeLayout = new QHBoxLayout();
    QLabel *typeLabel = new QLabel(tr("Type:"));
    this->typeCombo = new QComboBox();
    this->typeCombo->addItem("Folder", static_cast<int>(GameType::Folder));
    this->typeCombo->addItem("Zip", static_cast<int>(GameType::Zip));
    this->typeCombo->addItem("7z", static_cast<int>(GameType::SevenZip));
    this->typeCombo->addItem("Rar", static_cast<int>(GameType::Rar));
    this->typeCombo->addItem("Iso", static_cast<int>(GameType::Iso));
    
    int index = this->typeCombo->findData(static_cast<int>(this->item.type));
    if (index != -1) this->typeCombo->setCurrentIndex(index);
    this->typeCombo->setEnabled(false);
    
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(this->typeCombo);
    mainLayout->addLayout(typeLayout);
    
    // Korean Support
    this->koreanCheck = new QCheckBox(tr("Korean Support"));
    this->koreanCheck->setChecked(this->item.koreanSupport);
    mainLayout->addWidget(this->koreanCheck);
    
    // Tags
    QLabel *tagLabel = new QLabel(tr("Tags:"));
    mainLayout->addWidget(tagLabel);
    
    this->tagList = new QListWidget();
    QStringList allTags = TagManager::instance().getTags();
    for (const QString &tag : allTags) {
        QListWidgetItem *item = new QListWidgetItem(tag, this->tagList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (this->item.tags.contains(tag)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
    mainLayout->addWidget(this->tagList);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    this->saveButton = new QPushButton(tr("Save"));
    this->cancelButton = new QPushButton(tr("Cancel"));
    
    connect(this->saveButton, &QPushButton::clicked, this, &GameInfoDialog::save);
    connect(this->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    btnLayout->addWidget(this->saveButton);
    btnLayout->addWidget(this->cancelButton);
    mainLayout->addLayout(btnLayout);
}

void GameInfoDialog::save() {
    this->item.cleanName = this->nameEdit->text();
    this->item.folderName = this->folderNameEdit->text();
    this->item.koreanSupport = this->koreanCheck->isChecked();
    
    this->item.tags.clear();
    for (int i = 0; i < this->tagList->count(); ++i) {
        QListWidgetItem *tItem = this->tagList->item(i);
        if (tItem->checkState() == Qt::Checked) {
            this->item.tags.append(tItem->text());
        }
    }
    
    accept();
}

GameItem GameInfoDialog::getGameItem() const {
    return this->item;
}
