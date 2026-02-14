#ifndef GAMEINFODIALOG_H
#define GAMEINFODIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "gamedata.h"

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
    QLineEdit *pathEdit;
    QComboBox *typeCombo;
    QPushButton *saveButton;
    QPushButton *cancelButton;
};

#endif // GAMEINFODIALOG_H
