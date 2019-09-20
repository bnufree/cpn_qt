#ifndef ZCHXMAPMAINWINDOW_H
#define ZCHXMAPMAINWINDOW_H

#include <QMainWindow>
#include <QVariant>
#include "_def.h"

#define TIMER_GFRAME_1 999

namespace Ui {
class zchxMapMainWindow;
}
class ChartCanvas;
class ChartBase;
#if 0
class QTimer;
class zchxOptionsDlg;
class zchxConfig;
class ChartCanvas;
class ChartDB;
class OCPNPlatform;
#endif


QColor GetGlobalColor(const QString& colorName);
void InitializeUserColors( void );
void DeInitializeUserColors( void );

class zchxMapMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit zchxMapMainWindow(QWidget *parent = 0);
    ~zchxMapMainWindow();
    void    setActionCheckSts(const QString& action, bool check);
    void    setActionEnableSts(const QString& action, bool check);
    bool    ProcessOptionsDialog( int rr, ArrayOfCDI *pNewDirArray );
    bool    UpdateChartDatabaseInplace( ArrayOfCDI &DirArray, bool b_force, bool b_prog, const QString &ChartListFileName );
    ColorScheme GetColorScheme();
    void ToggleColorScheme();
    bool DoChartUpdate( void );
    void SetChartUpdatePeriod();
    ChartCanvas *GetPrimaryCanvas();
//    QWidget* getGlChartCanvas();
//    double GetBestVPScale( ChartBase *pchart );

public slots:
    //工具
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
    void    slotEnableChartQuilting(bool sts);              //是否Quilt填充
    void    slotShowChartQuilting(bool sts);
    void    slotShowENCText(bool sts);
    void    slotShowENCLights(bool sts);
    void    slotShowENCSoundings(bool sts);                 //是否显示水深
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
    void    RefreshAllCanvas( bool bErase = true);
    void    SetGPSCompassScale();
    double  GetMag(double a);
    void    InvalidateAllGL();
    void    slotRotateDegree(double angle);
    void    slotRoateRad(double rad);
private slots:
//    void    slotInitEcidsAsDelayed();
    void    slotRotate();
protected:
    void    resizeEvent(QResizeEvent* e);

private:
    QAction* addCustomAction(QMenu* menu, const QString &text, const QObject *receiver, const char* slot, bool check = false, const QVariant& data = QVariant());
//    void     initBeforeCreateCanvas();
//    void     CreateCanvasLayout();
private slots:
    void    slotOnFrameTimer1Out();

private:
    Ui::zchxMapMainWindow *ui;

    //菜单管理
    QMap<QString, QAction*>     mActionMap;
    //显示窗口
    ChartCanvas*                mEcdisWidget;
};

#endif // ZCHXMAPMAINWINDOW_H
