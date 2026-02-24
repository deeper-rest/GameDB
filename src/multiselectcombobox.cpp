#include "multiselectcombobox.h"
#include <QEvent>
#include <QMouseEvent>
#include <QLineEdit>
#include <QCursor>

MultiSelectComboBox::MultiSelectComboBox(QWidget *parent)
    : QComboBox(parent), isUpdating(false)
{
    model = new QStandardItemModel(this);
    listView = new QListView(this);
    
    this->setModel(model);
    this->setView(listView);
    
    listView->viewport()->installEventFilter(this);
    
    // Make line edit readonly
    this->setEditable(true);
    this->lineEdit()->setReadOnly(true);
    
    connect(model, &QStandardItemModel::itemChanged, this, &MultiSelectComboBox::onItemChanged);
}

void MultiSelectComboBox::addCheckableItem(const QString &text, const QVariant &userData)
{
    QStandardItem *item = new QStandardItem(text);
    item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    item->setData(Qt::Unchecked, Qt::CheckStateRole);
    item->setData(userData, Qt::UserRole);
    model->appendRow(item);
}

void MultiSelectComboBox::clearItems()
{
    isUpdating = true;
    model->clear();
    isUpdating = false;
    updateText();
}

void MultiSelectComboBox::hidePopup()
{
    int width = this->view()->width();
    int height = this->view()->height();
    int x = QCursor::pos().x() - this->view()->mapToGlobal(QPoint(0,0)).x();
    int y = QCursor::pos().y() - this->view()->mapToGlobal(QPoint(0,0)).y();
    if (x >= 0 && x <= width && y >= 0 && y <= height) {
        // Did click inside the list, do nothing
    } else {
        QComboBox::hidePopup();
    }
}

bool MultiSelectComboBox::eventFilter(QObject *object, QEvent *event)
{
    if (object == listView->viewport() && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = listView->indexAt(mouseEvent->pos());
        if (index.isValid()) {
            QStandardItem *item = model->itemFromIndex(index);
            if (item->checkState() == Qt::Checked) {
                item->setCheckState(Qt::Unchecked);
            } else {
                item->setCheckState(Qt::Checked);
            }
            return true; // Stop event so combobox doesn't close
        }
    }
    return QComboBox::eventFilter(object, event);
}

void MultiSelectComboBox::onItemChanged(QStandardItem *changedItem)
{
    if (isUpdating) return;
    isUpdating = true;

    QString itemData = changedItem->data(Qt::UserRole).toString();
    bool isChecked = (changedItem->checkState() == Qt::Checked);

    if (itemData == "All") {
        for (int i = 0; i < model->rowCount(); ++i) {
            model->item(i)->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        }
    } else {
        QStandardItem *allItem = nullptr;
        bool allOthersChecked = true;

        for (int i = 0; i < model->rowCount(); ++i) {
            QStandardItem *curr = model->item(i);
            if (curr->data(Qt::UserRole).toString() == "All") {
                allItem = curr;
            } else {
                if (curr->checkState() == Qt::Unchecked) {
                    allOthersChecked = false;
                }
            }
        }

        if (allItem) {
            allItem->setCheckState(allOthersChecked ? Qt::Checked : Qt::Unchecked);
        }
    }

    isUpdating = false;
    updateText();
    emit selectionChanged();
}

QStringList MultiSelectComboBox::getSelectedData() const
{
    QStringList result;
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        if (item->checkState() == Qt::Checked) {
            result.append(item->data(Qt::UserRole).toString());
        }
    }
    return result;
}

void MultiSelectComboBox::setSelectedData(const QStringList &dataList)
{
    isUpdating = true;
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        QString itemData = item->data(Qt::UserRole).toString();

        if (dataList.contains("All") || dataList.contains(itemData)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
    isUpdating = false;
    updateText();
}

void MultiSelectComboBox::updateText()
{
    QStringList texts;
    bool allSelected = false;
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        if (item->checkState() == Qt::Checked) {
            if (item->data(Qt::UserRole).toString() == "All") {
                allSelected = true;
                break;
            }
            texts.append(item->text());
        }
    }

    if (allSelected || texts.isEmpty()) {
        this->lineEdit()->setText("All Tags");
    } else {
        this->lineEdit()->setText(texts.join(", "));
    }
}
