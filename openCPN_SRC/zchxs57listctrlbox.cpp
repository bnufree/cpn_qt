﻿#include "zchxs57listctrlbox.h"

zchxS57ListCtrlBox::zchxS57ListCtrlBox(QWidget *parent) : QListWidget(parent)
{
    mIDs.clear();
}

int zchxS57ListCtrlBox::append(const QString &lable, bool checked)
{
    QListWidgetItem* item = new QListWidgetItem(this);
    item->setCheckState(checked == true ? Qt::Checked : Qt::Unchecked);
    addItem(item);
    int index = count() - 1;
    mIDs[item] = index;
    return index;
}

bool zchxS57ListCtrlBox::ischecked(int id)
{
    QListWidgetItem* item = this->item(id);
    return item->checkState() == Qt::Checked;
}

void zchxS57ListCtrlBox::setChecked(int id, bool checked)
{
    QListWidgetItem* item = this->item(id);
    item->setCheckState(checked == true ? Qt::Checked : Qt::Unchecked);
}