#include "zchxmapmainwindow.h"
#include "ui_zchxmapmainwindow.h"
#include <windows.h>
#include <psapi.h>
#include <QDebug>
#include <QTimer>

zchxMapMainWindow::zchxMapMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::zchxMapMainWindow)
{
    ui->setupUi(this);
    this->menuBar()->addAction("Settings", this, SLOT(slotOpenSettingDlg()));
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
}

