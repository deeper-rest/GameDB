#include "gamecarddelegate.h"
#include <QApplication>
#include <QPainterPath>

GameCardDelegate::GameCardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void GameCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Save painter state
    painter->save();

    // Data
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QString text = index.data(Qt::DisplayRole).toString();

    // Layout Constants
    const int padding = 10;
    const int textHeight = 60; // Fixed space for text
    
    QRect rect = option.rect;
    
    // Draw Background (Selection/Hover)
    // We can use style to draw primitive, or draw custom rect.
    // Let's use custom for card look.
    
    QRect cardRect = rect.adjusted(padding, padding, -padding, -padding);
    
    QColor bgColor = Qt::white;
    if (option.state & QStyle::State_Selected) {
        bgColor = option.palette.highlight().color();
    } else if (option.state & QStyle::State_MouseOver) {
        bgColor = option.palette.light().color();
    }
    
    // Rounded Rect
    QPainterPath path;
    path.addRoundedRect(cardRect, 10, 10);
    
    painter->fillPath(path, bgColor);
    
    // Draw Border if selected
    if (option.state & QStyle::State_Selected) {
        QPen pen(option.palette.highlightedText().color(), 2);
        painter->setPen(pen);
        painter->drawPath(path);
    }

    // Draw Icon
    // Icon Area: Top part of card
    QRect iconRect = cardRect;
    iconRect.setBottom(cardRect.bottom() - textHeight);
    
    // Deflate icon rect slightly for padding
    QRect pixmapRect = iconRect.adjusted(5, 5, -5, -5);
    
    if (!icon.isNull()) {
        // Paint Icon
        // Use paint to draw into rect, keeping aspect ratio
        icon.paint(painter, pixmapRect, Qt::AlignCenter, 
                   (option.state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                   (option.state & QStyle::State_Open) ? QIcon::On : QIcon::Off);
    }
    
    // Draw Text
    QRect textRect = cardRect;
    textRect.setTop(cardRect.bottom() - textHeight);
    textRect.adjust(5, 0, -5, -5); // Padding
    
    painter->setPen((option.state & QStyle::State_Selected) ? option.palette.highlightedText().color() : option.palette.text().color());
    
    QTextOption textOption;
    textOption.setAlignment(Qt::AlignCenter);
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    
    painter->drawText(textRect, text, textOption);
    
    painter->restore();
}

QSize GameCardDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Return standard size based on grid size if possible, or fixed size.
    // The view's GridSize usually dictates layout in IconMode, but sizeHint is respected if resizeMode is Adjust.
    return QSize(340, 280); 
}
