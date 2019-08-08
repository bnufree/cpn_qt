#ifndef ZCHXMAPMAINWINDOW_H
#define ZCHXMAPMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class zchxMapMainWindow;
}
class QTimer;
class zchxMapMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit zchxMapMainWindow(QWidget *parent = 0);
    ~zchxMapMainWindow();

    void setApplicationName(const QString& name);
    QString applicationName() const {return mApplicationName;}

    quint64 proceeId() const {return mProcessedID;}
    void    setProcessID(quint64 id) {mProcessedID = id;}
    quint64 getProcessIDFromSystem();
    int     GetApplicationMemoryUse(void);
    void    getMemoryStatus();

public slots:
    void    slotMemoryMonitor() {getMemoryStatus();}
    void    slotOpenSettingDlg();

private:
    Ui::zchxMapMainWindow *ui;
    //内存监控
    quint64               mProcessedID;
    QString               mApplicationName;
    quint64               mMemUsed;
    quint64               mMemTotal;

    //程序关闭检测
    bool                  mInCloseWindow;
    QTimer                *mMonitorTimer;
};

#endif // ZCHXMAPMAINWINDOW_H
