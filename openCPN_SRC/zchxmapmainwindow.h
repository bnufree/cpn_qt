#ifndef ZCHXMAPMAINWINDOW_H
#define ZCHXMAPMAINWINDOW_H

#include <QMainWindow>
#include <QVariant>

namespace Ui {
class zchxMapMainWindow;
}
class QTimer;
class zchxOptionsDlg;
class zchxConfig;

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
    void    setActionCheckSts(const QString& action, bool check);
    void    setActionEnableSts(const QString& action, bool check);
    zchxConfig*     getConfigObj() {return mConfigObj;}


public slots:
    //工具
    void    slotMemoryMonitor() {getMemoryStatus();}
    void    slotOpenSettingDlg();
    void    slotMeasureDistance();
    //导航
    void    slotNorthUp();
    void    slotAnyAngleUp();
    void    slotLookAheadMode(bool sts);
    void    slotZoomIn();
    void    slotZoomOut();
    void    slotLargeScaleChart();
    void    slotSmallScaleChart();
    //视图
    void    slotEnableChartQuilting(bool sts);
    void    slotShowChartQuilting(bool sts);
    void    slotShowChartBar(bool sts);
    void    slotShowENCText(bool sts);
    void    slotShowENCLights(bool sts);
    void    slotShowENCSoundings(bool sts);
    void    slotShowENCAnchoringInfo(bool sts);
    void    slotShowENCDataQuality(bool sts);
    void    slotShowNavObjects(bool sts);
    void    slotChangeColorScheme();
    void    slotShowDepthUnit(bool sts);
    void    slotShowGrid(bool sts);
    void    slotShowDepth(bool sts);
    void    slotShowBuoyLightLabel(bool sts);
    void    slotShowLightDiscriptions(bool sts);
    void    slotShowDisplayCategory();

private:
    QAction* addCustomAction(QMenu* menu, const QString &text, const QObject *receiver, const char* slot, bool check = false, const QVariant& data = QVariant());

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
    //菜单管理
    QMap<QString, QAction*>     mActionMap;
    //配置对话框
    zchxOptionsDlg*             mOptionDlg;
    //配置文件
    zchxConfig*                 mConfigObj;
};

#endif // ZCHXMAPMAINWINDOW_H
