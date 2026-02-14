#include "gamedetailwidget.h"
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QMessageBox>

GameDetailWidget::GameDetailWidget(const GameItem &item, QWidget *parent) 
    : QWidget(parent), item(item) {
    setupUI();
}

void GameDetailWidget::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 10);
    mainLayout->setSpacing(20);
    
    // Left: Large Thumbnail
    this->thumbnailLabel = new QLabel();
    this->thumbnailLabel->setFixedSize(320, 240);
    this->thumbnailLabel->setStyleSheet("border: 1px solid #ccc; background-color: #000;");
    this->thumbnailLabel->setAlignment(Qt::AlignCenter);
    
    if (!this->item.thumbnailPath.isEmpty() && QFile::exists(this->item.thumbnailPath)) {
        QPixmap pix(this->item.thumbnailPath);
        this->thumbnailLabel->setPixmap(pix.scaled(this->thumbnailLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        this->thumbnailLabel->setText("No Image");
        this->thumbnailLabel->setStyleSheet("border: 1px solid #ccc; background-color: #eee; color: #555;");
    }
    mainLayout->addWidget(this->thumbnailLabel);
    
    // Right: Info & Buttons
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignTop);
    
    // Game Title
    QLabel *titleLabel = new QLabel(this->item.cleanName);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    rightLayout->addWidget(titleLabel);
    
    // Metadata
    QString infoText = QString("Type: %1\nFolder Name: %2\nKorean Support: %3\nTags: %4")
        .arg(static_cast<int>(this->item.type)) // Simplified for now
        .arg(this->item.folderName)
        .arg(this->item.koreanSupport ? "O" : "X")
        .arg(this->item.tags.join(", "));
    
    this->infoLabel = new QLabel(infoText);
    this->infoLabel->setStyleSheet("font-size: 14px; color: #333; margin-bottom: 20px;");
    rightLayout->addWidget(this->infoLabel);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    this->playButton = new QPushButton(tr("▶ Run Game"));
    this->playButton->setMinimumHeight(40);
    this->playButton->setStyleSheet("font-size: 14px; font-weight: bold; background-color: #4CAF50; color: white; border-radius: 4px;");
    
    this->openButton = new QPushButton(tr("📂 Open Folder"));
    this->openButton->setMinimumHeight(40);
    
    connect(this->playButton, &QPushButton::clicked, this, &GameDetailWidget::onPlayClicked);
    connect(this->openButton, &QPushButton::clicked, this, &GameDetailWidget::onOpenClicked);
    
    btnLayout->addWidget(this->playButton);
    btnLayout->addWidget(this->openButton);
    btnLayout->addStretch();
    
    rightLayout->addLayout(btnLayout);
    rightLayout->addStretch();
    
    mainLayout->addLayout(rightLayout);
}

void GameDetailWidget::onPlayClicked() {
    if (this->item.exePath.isEmpty()) {
        QMessageBox::warning(this, tr("Cannot Run"), tr("No executable file specified for this game.\nPlease set it in Game Info."));
        return;
    }
    
    emit playGame(this->item.exePath);
}

void GameDetailWidget::onOpenClicked() {
    emit openFolder(this->item.filePath);
}
