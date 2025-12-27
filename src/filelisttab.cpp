#include "filelisttab.h"
#include <QVBoxLayout>

FileListTab::FileListTab() {
    setMainUI();
}

void FileListTab::setMainUI() {
    this->mainList = new QListWidget();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(this->mainList);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
}

void FileListTab::addItems(QStringList list) {
    for (int i=0; i < list.size(); i++){
        this->mainList->addItem(list[i]);
    }
}