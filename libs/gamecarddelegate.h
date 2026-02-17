#ifndef GAMECARDDELEGATE_H
#define GAMECARDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>

class GameCardDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit GameCardDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // GAMECARDDELEGATE_H
