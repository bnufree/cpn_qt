#include "zchxmapmainwindow.h"
#include "ui_zchxmapmainwindow.h"
#include <windows.h>
#include <psapi.h>
#include <QDebug>
#include <QTimer>
#include "_def.h"
#include "zchxoptionsdlg.h"
#include "zchxconfig.h"

zchxMapMainWindow::zchxMapMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::zchxMapMainWindow),
    mOptionDlg(0),
    mConfigObj(new zchxConfig)
{
    ui->setupUi(this);
    //工具
    QMenu* tools = this->menuBar()->addMenu(tr("Tools"));
    addCustomAction(tools,tr("Options"),this, SLOT(slotOpenSettingDlg()));
    addCustomAction(tools, tr("Measure distance"), this, SLOT(slotMeasureDistance()));

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
    addCustomAction(view, tr("Enable Chart Quilting"), this, SLOT(slotEnableChartQuilting(bool)));
    addCustomAction(view, tr("Show Chart Quilting"), this, SLOT(slotShowChartQuilting(bool)));
    addCustomAction(view, tr("Show ENC Text"), this, SLOT(slotShowChartBar(bool)));
    addCustomAction(view, tr("Zoom In"), this, SLOT(slotShowENCText(bool)));
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

    mMonitorTimer = new QTimer();
    mMonitorTimer->setInterval(30000);
    connect(mMonitorTimer, SIGNAL(timeout()), this, SLOT(slotMemoryMonitor()));
    mMonitorTimer->start();
}

zchxMapMainWindow::~zchxMapMainWindow()
{
    delete ui;
}


void zchxMapMainWindow::setApplicationName(const QString &name)
{
    mApplicationName = name;
}

quint64 zchxMapMainWindow::getProcessIDFromSystem()
{
#if 1
    return GetCurrentProcessId();
#else
    quint64 id = 0;
    HANDLE    hToolHelp32Snapshot;
    hToolHelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32    pe = { sizeof(PROCESSENTRY32) };
    BOOL  isSuccess = Process32First(hToolHelp32Snapshot, &pe);
    while (isSuccess)
    {
        size_t len = WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, wcslen(pe.szExeFile), NULL, 0, NULL, NULL);
        char *des = (char *)malloc(sizeof(char) * (len + 1));
        WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, wcslen(pe.szExeFile), des, len, NULL, NULL);
        des[len] = '\0';
        if (!strcmp(des, mApplicationName.toStdString().c_str()))
        {
            free(des);
            id = pe.th32ProcessID;
            break;
        }
        free(des);
        isSuccess = Process32Next(hToolHelp32Snapshot, &pe);
    }
    CloseHandle(hToolHelp32Snapshot);
    return id;
#endif
}

int zchxMapMainWindow::GetApplicationMemoryUse( void )
{
    int memsize = -1;
#if 0
    if(mApplicationName.size() == 0 && mProcessedID == 0)
    {
        qDebug()<<"application memsize cannot get for name not set yet.";
        return memsize;
    }
    if(mProcessedID == 0)
    {
        //先获取当前的进程号
        qDebug()<<"now start to get process id of application name:"<<mApplicationName;
        quint64 id = getProcessIDFromSystem();
        qDebug()<<"get process id from system is:"<<id;
        if(id <= 0) return memsize;
        mProcessedID = id;
    }


    HANDLE hProcess/* = GetCurrentProcess()*/;
    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
#else
    HANDLE hProcess = GetCurrentProcess();
#endif
    if( NULL == hProcess ) return 0;
    PROCESS_MEMORY_COUNTERS pmc;

    if( GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) ) {
        memsize = pmc.WorkingSetSize / 1024;
    }

    CloseHandle( hProcess );
    return memsize;
}

void  zchxMapMainWindow::getMemoryStatus()
{
    mMemUsed = GetApplicationMemoryUse();
    if(mMemTotal == 0)
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof( statex );
        GlobalMemoryStatusEx( &statex );
        mMemTotal = statex.ullTotalPhys / 1024;
    }
    qDebug()<<"memory total:"<<mMemTotal<<"  app used:"<<mMemUsed;
}

void zchxMapMainWindow::slotOpenSettingDlg()
{
    qDebug()<<"open settings windows now";
    if(!mOptionDlg) mOptionDlg = new zchxOptionsDlg(this);
    mOptionDlg->show();
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

}

void zchxMapMainWindow::slotZoomOut()
{

}

void zchxMapMainWindow::slotLargeScaleChart()
{

}

void zchxMapMainWindow::slotSmallScaleChart()
{

}

void zchxMapMainWindow::slotEnableChartQuilting(bool sts)
{

}

void zchxMapMainWindow::slotShowENCAnchoringInfo(bool sts)
{

}

void zchxMapMainWindow::slotShowBuoyLightLabel(bool sts)
{

}

void zchxMapMainWindow::slotShowChartBar(bool sts)
{

}

void zchxMapMainWindow::slotShowChartQuilting(bool sts)
{

}

void zchxMapMainWindow::slotShowDepth(bool sts)
{

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

}

void zchxMapMainWindow::slotShowENCText(bool sts)
{

}

void zchxMapMainWindow::slotShowGrid(bool sts)
{

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

