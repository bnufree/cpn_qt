#include "zchxmapmainwindow.h"
#include "ui_zchxmapmainwindow.h"
#include <QDebug>
#include "zchxoptionsdlg.h"
#include "zchxconfig.h"
#include "glChartCanvas.h"
#include <QThread>
#include <QVBoxLayout>

extern zchxMapMainWindow    *gFrame;
extern QThread              *g_Main_thread;


zchxMapMainWindow::zchxMapMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::zchxMapMainWindow)
    , mEcdisWidget(0)
{

    ui->setupUi(this);
    setMouseTracking(true);
    //工具
    QMenu* tools = this->menuBar()->addMenu(tr("Tools"));
    addCustomAction(tools,tr("Options"),this, SLOT(slotOpenSettingDlg()));
    addCustomAction(tools, tr("Measure distance"), this, SLOT(slotMeasureDistance()));
    addCustomAction(tools, tr("Rotate"), this, SLOT(slotRotate()));

    //导航
    QMenu* navigation = this->menuBar()->addMenu(tr("Navigation"));
    addCustomAction(navigation, tr("North Up"), this, SLOT(slotNorthUp()));
    addCustomAction(navigation, tr("Course Up"), this, SLOT(slotAnyAngleUp()));
    addCustomAction(navigation, tr("Look Ahead Mode"), this, SLOT(slotLookAheadMode(bool)), true);
    addCustomAction(navigation, tr("Zoom In"), this, SLOT(slotZoomIn()));
    addCustomAction(navigation, tr("Zoom Out"), this, SLOT(slotZoomOut()));
    addCustomAction(navigation, tr("Large Scale Chart"), this, SLOT(slotLargeScaleChart()));
    addCustomAction(navigation, tr("Small Scale Chart"), this, SLOT(slotSmallScaleChart()));
    //视图
    QMenu* view = this->menuBar()->addMenu(tr("View"));
//    addCustomAction(view, tr("Enable Chart Quilting"), this, SLOT(slotEnableChartQuilting(bool)));
    addCustomAction(view, tr("Show Chart Outline"), this, SLOT(slotShowChartOutline(bool)));
    addCustomAction(view, tr("Show ENC Text"), this, SLOT(slotShowENCText(bool)));
    addCustomAction(view, tr("Show ENC Lights"), this, SLOT(slotShowENCLights(bool)));
    addCustomAction(view, tr("Show ENC Soundings"), this, SLOT(slotShowENCSoundings(bool)));
    addCustomAction(view, tr("Show ENC Anchoring Info"), this, SLOT(slotShowENCAnchoringInfo(bool)));
    addCustomAction(view, tr("Show ENC Data Quality"), this, SLOT(slotShowENCDataQuality(bool)));
    addCustomAction(view, tr("Show Nav Objects"), this, SLOT(slotShowNavObjects(bool)));
    //颜色模式
    QMenu* color = view->addMenu(tr("Change Color Schem"));
    addCustomAction(color, tr("Day"), this, SLOT(slotChangeColorScheme()), false, ColorScheme::GLOBAL_COLOR_SCHEME_DAY);
    addCustomAction(color, tr("Dusk"), this, SLOT(slotChangeColorScheme()), false, ColorScheme::GLOBAL_COLOR_SCHEME_DUSK);
    addCustomAction(color, tr("Night"), this, SLOT(slotChangeColorScheme()), false, ColorScheme::GLOBAL_COLOR_SCHEME_NIGHT);

    addCustomAction(view, tr("Show Depth Unit"), this, SLOT(slotShowDepthUnit(bool)));
    addCustomAction(view, tr("Show Grid"), this, SLOT(slotShowGrid(bool)));
    addCustomAction(view, tr("Show Depth"), this, SLOT(slotShowDepth(bool)));
    addCustomAction(view, tr("Show Buoy Light Label"), this, SLOT(slotShowBuoyLightLabel(bool)));
    addCustomAction(view, tr("Show Light Discriptions"), this, SLOT(slotShowLightDiscriptions(bool)));
    //显示模式
    QMenu* display = view->addMenu(tr("Show Display Category"));
    addCustomAction(display, tr("Base"), this, SLOT(slotShowDisplayCategory()), false, ColorScheme::GLOBAL_COLOR_SCHEME_DAY);
    addCustomAction(display, tr("Standard"), this, SLOT(slotShowDisplayCategory()), false, ColorScheme::GLOBAL_COLOR_SCHEME_DUSK);
    addCustomAction(display, tr("All"), this, SLOT(slotShowDisplayCategory()), false, ColorScheme::GLOBAL_COLOR_SCHEME_NIGHT);

    //添加窗口
    mEcdisWidget = new glChartCanvas(this);
    if(!ui->centralwidget->layout())
    {
        ui->centralwidget->setLayout(new QVBoxLayout(ui->centralwidget));
    }
    ui->centralwidget->layout()->addWidget(mEcdisWidget);

    g_Main_thread = QThread::currentThread();
    gFrame = this;
}

zchxMapMainWindow::~zchxMapMainWindow()
{
    ZCHX_CFG_INS->UpdateSettings();
    delete ui;
}



void zchxMapMainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);

}




void zchxMapMainWindow::slotOpenSettingDlg()
{
    qDebug()<<"open settings windows now";
    zchxOptionsDlg* dlg = new zchxOptionsDlg(this);
    dlg->show();
}

void zchxMapMainWindow::slotMeasureDistance()
{

}

void zchxMapMainWindow::slotNorthUp()
{

}

void zchxMapMainWindow::slotAnyAngleUp()
{

}

void zchxMapMainWindow::slotLookAheadMode(bool sts)
{

}

void zchxMapMainWindow::slotZoomIn()
{
     if(mEcdisWidget) mEcdisWidget->Zoom( 2.0, false );
}

void zchxMapMainWindow::slotZoomOut()
{
     if(mEcdisWidget) mEcdisWidget->Zoom( 0.5, false );
}

void zchxMapMainWindow::slotLargeScaleChart()
{

}

void zchxMapMainWindow::slotSmallScaleChart()
{

}

void zchxMapMainWindow::slotEnableChartQuilting(bool sts)
{
    if(mEcdisWidget &&  mEcdisWidget->GetQuiltMode() != sts)
    {
        mEcdisWidget->ToggleCanvasQuiltMode();
    }
}

void zchxMapMainWindow::slotShowENCAnchoringInfo(bool sts)
{

}

void zchxMapMainWindow::slotShowBuoyLightLabel(bool sts)
{

}

void zchxMapMainWindow::slotShowChartOutline(bool sts)
{
    if(!mEcdisWidget) return;
    if(mEcdisWidget->GetShowOutlines() == sts) return;
    mEcdisWidget->SetShowOutlines(sts);

}

void zchxMapMainWindow::slotShowDepth(bool sts)
{
    slotShowENCSoundings(sts);
}

void zchxMapMainWindow::slotShowDepthUnit(bool sts)
{

}

void zchxMapMainWindow::slotShowDisplayCategory()
{

}

void zchxMapMainWindow::slotShowENCDataQuality(bool sts)
{

}

void zchxMapMainWindow::slotShowENCLights(bool sts)
{

}

void zchxMapMainWindow::slotShowENCSoundings(bool sts)
{
    if(mEcdisWidget && mEcdisWidget->GetShowENCDepth() != sts)
    {
        mEcdisWidget->SetShowENCDepth(sts);
    }
}

void zchxMapMainWindow::slotShowENCText(bool sts)
{
    if(!mEcdisWidget) return;
    if(mEcdisWidget->GetShowENCText() == sts) return;
    mEcdisWidget->SetShowENCText(sts);
}

void zchxMapMainWindow::slotShowGrid(bool sts)
{
    if(!mEcdisWidget) return;
    if(mEcdisWidget->GetShowGrid() == sts) return;
    mEcdisWidget->SetShowGrid(sts);
}

void zchxMapMainWindow::slotShowLightDiscriptions(bool sts)
{

}

void zchxMapMainWindow::slotShowNavObjects(bool sts)
{

}

void zchxMapMainWindow::slotChangeColorScheme()
{

}

QAction* zchxMapMainWindow::addCustomAction(QMenu *menu, const QString &text, const QObject *receiver, const char *slot,  bool check, const QVariant &data)
{
    if(!menu || !slot || !receiver || strlen(slot) == 0) return 0;
    if(mActionMap.contains(text)) return 0;

    QAction *result = menu->addAction(text);
    if(QString(slot).contains("(bool)"))
    {
        connect(result, SIGNAL(toggled(bool)), receiver, slot);
        result->setCheckable(true);
        result->setChecked(check);
    } else
    {
        connect(result,SIGNAL(triggered()), receiver, slot);
        result->setCheckable(false);
    }

    result->setData(data);
    mActionMap[text] = result;
    return result;
}

void zchxMapMainWindow::setActionCheckSts(const QString &action, bool check)
{
    if(!mActionMap.contains(action)) return;
    if(!mActionMap[action]->isCheckable()) return;
    mActionMap[action]->setChecked(check);
}

void zchxMapMainWindow::setActionEnableSts(const QString &action, bool check)
{
    if(!mActionMap.contains(action)) return;
    mActionMap[action]->setEnabled(check);
}

bool zchxMapMainWindow::ProcessOptionsDialog( int rr, ArrayOfCDI *pNewDirArray )
{
    bool b_need_refresh = false;                // Do we need a full reload?

    if( ( rr & VISIT_CHARTS )
            && ( ( rr & CHANGE_CHARTS ) || ( rr & FORCE_UPDATE ) || ( rr & SCAN_UPDATE ) ) ) {
        if(mEcdisWidget)
        {
            mEcdisWidget->UpdateChartDatabaseInplace( *pNewDirArray, ( ( rr & FORCE_UPDATE ) == FORCE_UPDATE ),  true );

            b_need_refresh = true;
        }
    }

//    if(mEcdisWidget) mEcdisWidget->canvasRefreshGroupIndex();
}


void zchxMapMainWindow::InvalidateAllGL()
{
}


void zchxMapMainWindow::slotRotateDegree(double angle)
{
    if(mEcdisWidget) mEcdisWidget->RotateDegree(angle);
}

void zchxMapMainWindow::slotRoateRad(double rad)
{
    if(mEcdisWidget) mEcdisWidget->Rotate(rad);
}

void zchxMapMainWindow::slotRotate()
{
    static double rotate = 60;
    static double coeff = -1.0;
    slotRotateDegree(rotate);
    rotate += coeff * 60.0;
    if(fabs(rotate) > 60)
    {
        coeff *= (-1);
        rotate = 0;
    }
}


