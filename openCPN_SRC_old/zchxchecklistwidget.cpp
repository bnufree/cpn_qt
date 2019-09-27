#include "zchxchecklistwidget.h"

zchxCheckListWidget::zchxCheckListWidget(QWidget *parent) : QListWidget(parent)
{
}

unsigned int zchxCheckListWidget::Append(const QString& label, bool val)
{
    QListWidgetItem* item = new QListWidgetItem(label, this);
    item->setCheckState(val == true? Qt::Checked : Qt::Unchecked);
    m_list.append(item);
    return m_list.GetCount() - 1;
}

void zchxCheckListWidget::Check(int index, bool val) {
    QListWidgetItem* item = m_list.Item(index);
    if(item) item->setCheckState(val == true? Qt::Checked : Qt::Unchecked);
}

bool zchxCheckListWidget::IsChecked(int index)
{
  QListWidgetItem* item = m_list.Item(index);
  if(!item) return false;
  return item->checkState() == Qt::Checked;
}

void zchxCheckListWidget::Clear()
{
    while(count() > 0)
    {

        QListWidgetItem * first = this->takeItem(0);
        if(first)
        {
            m_list.removeOne(first);
            delete first;
        }
    }
}
