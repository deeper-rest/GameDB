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

#include "gamedata.h"
#include "tagmanager.h"

class GameInfoDialog : public QDialog {
    Q_OBJECT

public:
    explicit GameInfoDialog(const GameItem &item, QWidget *parent = nullptr);
    GameItem getGameItem() const;

private slots:
    void save();

private:
    void setupUI();

    GameItem item;
    QLineEdit *nameEdit;
    QLineEdit *folderNameEdit;
    QLineEdit *pathEdit;
    QComboBox *typeCombo;
    QCheckBox *koreanCheck;
    QListWidget *tagList;
    
    QPushButton *saveButton;
    QPushButton *cancelButton;
};

#endif // GAMEINFODIALOG_H
