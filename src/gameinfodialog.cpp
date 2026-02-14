#include "gameinfodialog.h"
#include <QMimeData>

GameInfoDialog::GameInfoDialog(const GameItem &item, QWidget *parent) 
    : QDialog(parent), item(item) {
    setupUI();
    setWindowTitle(tr("Add Game"));
}

void GameInfoDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Shortcut for Paste
    QShortcut *pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    connect(pasteShortcut, &QShortcut::activated, this, &GameInfoDialog::onPasteThumbnail);

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
    // User requested to fix detection issues by allowing manual edit
    this->typeCombo->setEnabled(true);
    
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
    
    // --- Thumbnail & Launch Section ---
    QGroupBox *thumbBox = new QGroupBox(tr("Thumbnail & Launch"));
    QVBoxLayout *thumbLayout = new QVBoxLayout(thumbBox);
    
    // EXE Selection (For Auto Capture & Launching)
    QHBoxLayout *exeLayout = new QHBoxLayout();
    exeLayout->addWidget(new QLabel(tr("Execute File (.exe):")));
    this->exePathEdit = new QLineEdit(this->item.exePath);
    QPushButton *btnBrowseExe = new QPushButton(tr("Browse..."));
    connect(btnBrowseExe, &QPushButton::clicked, this, &GameInfoDialog::onBrowseExeClicked);
    exeLayout->addWidget(this->exePathEdit);
    exeLayout->addWidget(btnBrowseExe);
    thumbLayout->addLayout(exeLayout);
    
    // Preview
    this->imagePreview = new QLabel();
    this->imagePreview->setFixedSize(200, 150);
    this->imagePreview->setStyleSheet("border: 1px solid #ccc; background-color: #eee;");
    this->imagePreview->setAlignment(Qt::AlignCenter);
    this->imagePreview->setText("No Image");
    updateThumbnailPreview(); // Load existing if any
    
    // Capture Controls
    QHBoxLayout *capLayout = new QHBoxLayout();
    capLayout->addWidget(new QLabel(tr("Delay (sec):")));
    this->delaySpin = new QSpinBox();
    this->delaySpin->setRange(1, 60);
    this->delaySpin->setValue(10);
    capLayout->addWidget(this->delaySpin);
    
    QPushButton *btnCapture = new QPushButton(tr("Auto Capture"));
    connect(btnCapture, &QPushButton::clicked, this, &GameInfoDialog::onCaptureClicked);
    capLayout->addWidget(btnCapture);
    
    thumbLayout->addWidget(this->imagePreview, 0, Qt::AlignCenter);
    thumbLayout->addLayout(capLayout);
    mainLayout->addWidget(thumbBox);
    
    // --- Advanced Section (Manual Upload & Metadata) ---
    this->toggleAdvancedBtn = new QPushButton(tr("▶ Advanced / Manual Upload"));
    this->toggleAdvancedBtn->setCheckable(true);
    this->toggleAdvancedBtn->setStyleSheet("text-align: left; font-weight: bold;");
    connect(this->toggleAdvancedBtn, &QPushButton::clicked, this, &GameInfoDialog::onToggleAdvanced);
    mainLayout->addWidget(this->toggleAdvancedBtn);
    
    this->advancedBox = new QGroupBox();
    this->advancedBox->setVisible(false);
    QVBoxLayout *advLayout = new QVBoxLayout(this->advancedBox);
    
    // Source & Code
    QHBoxLayout *metaLayout = new QHBoxLayout();
    
    // Source
    metaLayout->addWidget(new QLabel(tr("Source:")));
    this->sourceCombo = new QComboBox();
    this->sourceCombo->addItems(QStringList() << "Other" << "Steam" << "DLsite" << "DMM" << "GOG");
    this->sourceCombo->setCurrentText(this->item.source.isEmpty() ? "Other" : this->item.source);
    this->sourceCombo->setEditable(true); // Allow custom
    metaLayout->addWidget(this->sourceCombo);
    
    // Game Code
    metaLayout->addWidget(new QLabel(tr("Code:")));
    this->gameCodeEdit = new QLineEdit(this->item.gameCode);
    this->gameCodeEdit->setPlaceholderText("e.g. RJ123456");
    metaLayout->addWidget(this->gameCodeEdit);
    
    advLayout->addLayout(metaLayout);
    
    // Manual Thumbnail
    QHBoxLayout *manualThumbLayout = new QHBoxLayout();
    this->btnBrowseThumb = new QPushButton(tr("Browse Image..."));
    this->btnPasteThumb = new QPushButton(tr("Paste from Clipboard (Ctrl+V)"));
    // Add shortcut hint tooltip
    this->btnPasteThumb->setToolTip(tr("Paste image from clipboard"));
    
    connect(this->btnBrowseThumb, &QPushButton::clicked, this, &GameInfoDialog::onBrowseThumbnail);
    connect(this->btnPasteThumb, &QPushButton::clicked, this, &GameInfoDialog::onPasteThumbnail);
    
    manualThumbLayout->addWidget(this->btnBrowseThumb);
    manualThumbLayout->addWidget(this->btnPasteThumb);
    advLayout->addLayout(manualThumbLayout);
    
    mainLayout->addWidget(this->advancedBox);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    this->saveButton = new QPushButton(tr("Save"));
    this->cancelButton = new QPushButton(tr("Cancel"));
    
    connect(this->saveButton, &QPushButton::clicked, this, &GameInfoDialog::save);
    connect(this->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    btnLayout->addWidget(this->saveButton);
    btnLayout->addWidget(this->cancelButton);
    mainLayout->addLayout(btnLayout);
    
    // Initialize Manager
    this->thumbManager = new ThumbnailManager(this);
    connect(this->thumbManager, &ThumbnailManager::captureFinished, this, &GameInfoDialog::onCaptureFinished);
    connect(this->thumbManager, &ThumbnailManager::captureFailed, this, &GameInfoDialog::onCaptureFailed);
}

void GameInfoDialog::updateThumbnailPreview() {
    if (!this->item.thumbnailPath.isEmpty() && QFile::exists(this->item.thumbnailPath)) {
        QPixmap pix(this->item.thumbnailPath);
        this->imagePreview->setPixmap(pix.scaled(this->imagePreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        this->imagePreview->setText("No Image");
    }
}

void GameInfoDialog::onBrowseExeClicked() {
    QString dir = this->item.filePath;
    if (QFileInfo(dir).isFile()) dir = QFileInfo(dir).absolutePath();
    
    QString path = QFileDialog::getOpenFileName(this, tr("Select Executable"), dir, "Executables (*.exe)");
    if (!path.isEmpty()) {
        this->exePathEdit->setText(path);
    }
}

void GameInfoDialog::onCaptureClicked() {
    QString exe = this->exePathEdit->text();
    if (exe.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an executable file first."));
        return;
    }
    
    this->saveButton->setEnabled(false);
    this->imagePreview->setText("Capturing...");
    
    QString name = this->nameEdit->text();
    if (name.isEmpty()) name = this->item.cleanName;
    
    this->thumbManager->startCapture(exe, name, this->delaySpin->value());
}

void GameInfoDialog::onCaptureFinished(const QString &path) {
    this->item.thumbnailPath = path;
    updateThumbnailPreview();
    this->saveButton->setEnabled(true);
    QMessageBox::information(this, tr("Success"), tr("Thumbnail captured successfully!"));
}

void GameInfoDialog::onCaptureFailed(const QString &reason) {
    this->imagePreview->setText("Failed");
    this->saveButton->setEnabled(true);
    QMessageBox::critical(this, tr("Capture Failed"), reason);
}

void GameInfoDialog::onToggleAdvanced() {
    bool visible = this->toggleAdvancedBtn->isChecked();
    this->advancedBox->setVisible(visible);
    this->toggleAdvancedBtn->setText(visible ? tr("▼ Advanced / Manual Upload") : tr("▶ Advanced / Manual Upload"));
}

void GameInfoDialog::onBrowseThumbnail() {
    QString path = QFileDialog::getOpenFileName(this, tr("Select Thumbnail"), "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!path.isEmpty()) {
        this->item.thumbnailPath = path;
        updateThumbnailPreview();
    }
}

void GameInfoDialog::onPasteThumbnail() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    const QMimeData *mime = clipboard->mimeData();
    
    if (mime->hasImage()) {
        QPixmap pix = qvariant_cast<QPixmap>(mime->imageData());
        if (!pix.isNull()) {
            // Save to file
            QString name = this->item.cleanName;
            // Sanitize name
            static QRegularExpression re("[\\\\/:*?\"<>|]");
            name.replace(re, "");
            name = name.simplified().replace(" ", "_");
            if (name.isEmpty()) name = "game";
            
            QString fileName = QString("%1_thumb_manual.png").arg(name);
            QString saveLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir dir(saveLocation);
            // Ensure thumbnails dir? Just put in root of AppData for now or create subfolder.
            // Following existing pattern (ThumbnailManager seems to use SaveLocation directly or passed path).
            // Let's use same dir as games.json
            
            QString fullPath = dir.filePath(fileName);
            if (pix.save(fullPath, "PNG")) {
                this->item.thumbnailPath = fullPath;
                updateThumbnailPreview();
                QMessageBox::information(this, tr("Success"), tr("Image pasted and saved."));
            } else {
                QMessageBox::warning(this, tr("Error"), tr("Failed to save pasted image."));
            }
        }
    } else {
        QMessageBox::information(this, tr("Info"), tr("No image in clipboard."));
    }
}

// Override keyPressEvent to handle Ctrl+V?
// Or just let button do it.
// Let's add keyPressEvent logic to the class in a separate edit if needed.
// For now, rely on button.

void GameInfoDialog::save() {
    this->item.cleanName = this->nameEdit->text();
    this->item.folderName = this->folderNameEdit->text();
    this->item.type = static_cast<GameType>(this->typeCombo->currentData().toInt());
    this->item.koreanSupport = this->koreanCheck->isChecked();
    this->item.exePath = this->exePathEdit->text();
    
    // Extended Metadata
    if (this->sourceCombo) {
        this->item.source = this->sourceCombo->currentText();
        this->item.gameCode = this->gameCodeEdit->text();
    }
    
    qDebug() << "Saving GameItem. Thumbnail:" << this->item.thumbnailPath;
    
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
