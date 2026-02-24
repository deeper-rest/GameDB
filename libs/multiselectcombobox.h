#ifndef MULTISELECTCOMBOBOX_H
#define MULTISELECTCOMBOBOX_H

#include <QComboBox>
#include <QStandardItemModel>
#include <QListView>

class MultiSelectComboBox : public QComboBox {
    Q_OBJECT

public:
    explicit MultiSelectComboBox(QWidget *parent = nullptr);
    QStringList getSelectedData() const;
    void setSelectedData(const QStringList &dataList);
    
    void addCheckableItem(const QString &text, const QVariant &userData);
    void clearItems();

signals:
    void selectionChanged();

protected:
    void hidePopup() override;
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void onItemChanged(QStandardItem *item);

private:
    void updateText();
    QStandardItemModel *model;
    QListView *listView;
    bool isUpdating;
};

#endif // MULTISELECTCOMBOBOX_H
