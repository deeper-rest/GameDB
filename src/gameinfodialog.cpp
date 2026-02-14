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
    
    // Set current index based on item.type
    int index = this->typeCombo->findData(static_cast<int>(this->item.type));
    if (index != -1) this->typeCombo->setCurrentIndex(index);
    this->typeCombo->setEnabled(false); // Type is determined by file extension usually, let's keep it fixed for now
    
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(this->typeCombo);
    mainLayout->addLayout(typeLayout);

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
    accept();
}

GameItem GameInfoDialog::getGameItem() const {
    return this->item;
}
