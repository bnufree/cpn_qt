﻿#ifndef ZCHXMAPMAINWINDOW_H
#define ZCHXMAPMAINWINDOW_H

#include <QMainWindow>
#include <QVariant>
#include "_def.h"

#define TIMER_GFRAME_1 999

namespace Ui {
class zchxMapMainWindow;
}
class QTimer;
class zchxOptionsDlg;
class zchxConfig;
class ChartCanvas;
class ChartBase;
class ChartDB;
class OCPNPlatform;


QColor GetGlobalColor(QString colorName);

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

    void DoStackDown( ChartCanvas *cc );
    void DoStackUp( ChartCanvas *cc );
    void DoStackDelta( ChartCanvas *cc, int direction );
    void ToggleColorScheme();
    bool DoChartUpdate( void );
    void UpdateRotationState( double rotation );
    void SetChartUpdatePeriod();
    ChartCanvas *GetPrimaryCanvas();
//    QWidget* getGlChartCanvas();
    double GetBestVPScale( ChartBase *pchart );

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
    void    RefreshAllCanvas( bool bErase = true);
    void    SetGPSCompassScale();
    double  GetMag(double a);
    void    InvalidateAllGL();

private:
    QAction* addCustomAction(QMenu* menu, const QString &text, const QObject *receiver, const char* slot, bool check = false, const QVariant& data = QVariant());
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
