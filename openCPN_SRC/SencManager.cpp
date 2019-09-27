/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include <thread>

#include "s57chart.h"
#include "Osenc.h"
//#include "chart1.h"
#include "chcanv.h"
//#include "zchxLog.h"
#include "zchxmapmainwindow.h"
#include "glChartCanvas.h"


extern zchxMapMainWindow*          gFrame;
extern SENCThreadManager *g_SencThreadManager;
extern ColorScheme       global_color_scheme;
extern int               g_nCPUCount;
S57ClassRegistrar *g_poRegistrar = 0;



//----------------------------------------------------------------------------------
//      SENCJobTicket Implementation
//----------------------------------------------------------------------------------
SENCJobTicket::SENCJobTicket()
{
    m_SENCResult = SENC_BUILD_INACTIVE;
    m_status = THREAD_INACTIVE;
}


//----------------------------------------------------------------------------------
//      SENCThreadManager Implementation
//----------------------------------------------------------------------------------
SENCThreadManager::SENCThreadManager(QObject* parent):QObject(parent)
{
    // ideally we would use the cpu count -1, and only launch jobs
    // when the idle load average is sufficient (greater than 1)
     int nCPU =  fmax(1, std::thread::hardware_concurrency());
     if(g_nCPUCount > 0)    nCPU = g_nCPUCount;
 
      // obviously there's at least one CPU!
     if (nCPU < 1)   nCPU = 1;

    m_max_jobs =  fmax(nCPU - 1, 1);
    //m_max_jobs = 1;

//    if(bthread_debug)
    qDebug(QString("").sprintf(" SENC: nCPU: %d    m_max_jobs :%d\n", nCPU, m_max_jobs).toStdString().data());
}

SENCThreadManager::~SENCThreadManager()
{
//    ClearJobList();
}

bool SENCThreadManager::appendJobWithCheck(SENCJobTicket *ticket)
{
    //  Do not add a job if there is already a job pending for this chart, by name
    QMutexLocker locker(&mMutex);
    for(size_t i=0 ; i < ticket_list.size() ; i++){
        if(ticket_list[i]->m_FullPath000 == ticket->m_FullPath000)
            return false;
    }

    ticket->m_status = THREAD_PENDING;
    ticket_list.push_back(ticket);
    return true;
}

SENCThreadStatus SENCThreadManager::appendJob(SENCJobTicket *ticket)
{
    if(appendJobWithCheck(ticket))
    {
        startJob();
    }
    return THREAD_PENDING;
}

SENCJobTicket* SENCThreadManager::getWorkJob(int& total)
{
    QMutexLocker locker(&mMutex);
    // Get the running job count
    total = 0;
    for(size_t i=0 ; i < ticket_list.size() ; i++){
        if(ticket_list[i]->m_status == THREAD_STARTED)
            total++;
    }
    if(total == m_max_jobs) return NULL;

    SENCJobTicket *startCandidate = NULL;
    // OK to start one?
    for(size_t i=0 ; i < ticket_list.size() ; i++){
        if(ticket_list[i]->m_status == THREAD_PENDING){
            startCandidate = ticket_list[i];
            break;
        }
    }
    return startCandidate;
}

void SENCThreadManager::startJob()
{
    int nRunning = 0;
    SENCJobTicket *startCandidate = getWorkJob(nRunning);
    if(startCandidate)
    {
        nRunning++;
        SENCBuildThread *thread = new SENCBuildThread( startCandidate, this);
        connect(thread, SIGNAL(finished()), this, SLOT(slotRecvSENCThreadFinished()));
        startCandidate->m_thread = thread;
        startCandidate->m_status = THREAD_STARTED;
        thread->setPriority(QThread::NormalPriority);
        thread->start();
    }
    
    if(nRunning){
        QString count;
        count.sprintf("  %ld", ticket_list.size());
        gFrame->getWidget()->SetAlertString( "Preparing vector chart  " + count);
    }  else{
        gFrame->getWidget()->SetAlertString( (""));
    }
}


int SENCThreadManager::GetJobCount()
{
    QMutexLocker locker(&mMutex);
    return ticket_list.size();
}

bool SENCThreadManager::IsChartInTicketlist(s57chart *chart)
{
    QMutexLocker locker(&mMutex);
     for(size_t i=0 ; i < ticket_list.size() ; i++){
         if(ticket_list[i]->m_chart == chart)
             return true;
     }
    return false;

}

bool SENCThreadManager::SetChartPointer(s57chart *chart, void *new_ptr)
{
    QMutexLocker locker(&mMutex);
    // Find the ticket
     for(size_t i=0 ; i < ticket_list.size() ; i++){
         if(ticket_list[i]->m_chart == chart){
             ticket_list[i]->m_chart = (s57chart *)new_ptr;
             return true;
         }
     }
    return false;
}

void SENCThreadManager::removeJob(SENCJobTicket *ticket)
{
    QMutexLocker locker(&mMutex);
    ticket_list.removeOne(ticket);
    if(ticket)
    {
        delete ticket;
        ticket = 0;
    }
}

 
#define NBAR_LENGTH 40

void SENCThreadManager::slotRecvSENCThreadFinished()
{
    SENCBuildThread* thread = qobject_cast<SENCBuildThread*>(sender());
    if(!thread) return;

    if(thread->m_ticket->m_SENCResult == SENC_BUILD_DONE_NOERROR)
    {
        //通知地图,重新加载地图数据
        emit signalRefreshAllEcids();
    } else if(thread->m_chart)
    {        //有错误的情况,提示错误信息
        gFrame->getWidget()->SetAlertString(QString("error occured when prepare chart: %1").arg(thread->m_chart->GetFullPath()));
    }
    removeJob(thread->m_ticket);
    delete thread;
    //遍历任务队列,找到还没有开始的任务,将任务开始
    startJob();
}

//----------------------------------------------------------------------------------
//      SENCBuildThread Implementation
//----------------------------------------------------------------------------------


SENCBuildThread::SENCBuildThread(SENCJobTicket *ticket, SENCThreadManager *manager)
:QThread(0),m_chart(0)
{
    m_FullPath000 = ticket->m_FullPath000;
    m_SENCFileName = ticket->m_SENCFileName;
    m_manager = manager;
    m_ticket = ticket;
    if(m_ticket) m_chart = m_ticket->m_chart;
}

void SENCBuildThread::run()
{
    // Start the SENC build
    Osenc senc;
    senc.setRegistrar( g_poRegistrar );
    senc.setRefLocn(m_ticket->ref_lat, m_ticket->ref_lon);
    senc.SetLODMeters(m_ticket->m_LOD_meters);
    senc.setNoErrDialog( true );
    m_ticket->m_SENCResult = SENC_BUILD_STARTED;
    m_ticket->m_status = THREAD_STARTED;

    int ret = senc.createSenc200( m_FullPath000, m_SENCFileName, false );
    m_ticket->m_status = THREAD_FINISHED;
    m_ticket->m_SENCResult = (ret == SENC_NO_ERROR ? SENC_BUILD_DONE_NOERROR : SENC_BUILD_DONE_ERROR);
    //重新生成规则文件
    if(ret == SENC_NO_ERROR)
    {
        if(m_chart) m_chart->PostInit(FULL_INIT, global_color_scheme);
    }

}



