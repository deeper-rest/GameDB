#ifndef GAMEINFODIALOG_H
#define GAMEINFODIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QListWidget>
#include <QFileDialog>
#include <QGroupBox>
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>

#include "gamedata.h"
#include "tagmanager.h"
#include "thumbnailmanager.h"
#include <QSpinBox>
#include <QClipboard>
#include <QGuiApplication>
#include <QBuffer>

class GameInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit GameInfoDialog(const GameItem &item, QWidget *parent = nullptr);
    GameItem getGameItem() const;

private slots:
    void save();
    void onCaptureClicked();
    void onBrowseExeClicked();
    void onCaptureFinished(const QString &path);
    void onCaptureFailed(const QString &reason);
    void onToggleAdvanced();
    void onBrowseThumbnail();
    void onPasteThumbnail();

private:
    void setupUI();
    void updateThumbnailPreview();

    GameItem item;
    QLineEdit *nameEdit;
    QLineEdit *folderNameEdit;
    QLineEdit *pathEdit;
    QComboBox *typeCombo;
    QCheckBox *koreanCheck;
    QListWidget *tagList;
    
    // Thumbnail
    QLabel *imagePreview;
    QLineEdit *exePathEdit;
    QSpinBox *delaySpin;
    ThumbnailManager *thumbManager;
    
    QPushButton *saveButton;
    QPushButton *cancelButton;

    // Advanced
    QGroupBox *advancedBox;
    QPushButton *toggleAdvancedBtn;
    QComboBox *sourceCombo;
    QLineEdit *gameCodeEdit;
    QPushButton *btnBrowseThumb;
    QPushButton *btnPasteThumb;
};

#endif // GAMEINFODIALOG_H
