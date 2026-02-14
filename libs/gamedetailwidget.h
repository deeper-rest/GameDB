#ifndef GAMEDETAILWIDGET_H
#define GAMEDETAILWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "gamedata.h"

class GameDetailWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameDetailWidget(const GameItem &item, QWidget *parent = nullptr);

signals:
    void playGame(QString exePath);
    void openFolder(QString path);

private slots:
    void onPlayClicked();
    void onOpenClicked();

private:
    void setupUI();

    GameItem item;
    QLabel *thumbnailLabel;
    QLabel *infoLabel;
    QPushButton *playButton;
    QPushButton *openButton;
};

#endif // GAMEDETAILWIDGET_H
