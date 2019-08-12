#ifndef ZCHXS57LISTCTRLBOX_H
#define ZCHXS57LISTCTRLBOX_H

#include <QListWidget>

class zchxS57ListCtrlBox : public QListWidget
{
    Q_OBJECT
public:
    explicit zchxS57ListCtrlBox(QWidget *parent = 0);
    int     append(const QString& item, bool checked);
    bool    ischecked(int id);
    void    setChecked(int id, bool checked);
signals:

public slots:
private:
    QMap<QString, int>      mIDs;
};

#endif // ZCHXS57LISTCTRLBOX_H
