#ifndef ZCHXCHECKLISTWIDGET_H
#define ZCHXCHECKLISTWIDGET_H

#include <QListWidget>

class zchxCheckListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit zchxCheckListWidget(QWidget *parent = 0);
    unsigned int Append(const QString& label, bool benable = true);
    unsigned int GetCount() { return m_list.count(); }
    void Clear();
    void Check(int index, bool val);
    bool IsChecked(int index);
private:

signals:

public slots:


private:
    QList<QListWidgetItem*> m_list;
};

#endif // ZCHXCHECKLISTWIDGET_H
