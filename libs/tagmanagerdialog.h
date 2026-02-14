#ifndef TAGMANAGERDIALOG_H
#define TAGMANAGERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TagManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit TagManagerDialog(QWidget *parent = nullptr);

private slots:
    void addTag();
    void renameTag();
    void removeTag();
    void refreshList();
    void onSelectionChanged();

private:
    void setupUI();

    QListWidget *tagList;
    QPushButton *addButton;
    QPushButton *renameButton;
    QPushButton *removeButton;
    QPushButton *closeButton;
};

#endif // TAGMANAGERDIALOG_H
