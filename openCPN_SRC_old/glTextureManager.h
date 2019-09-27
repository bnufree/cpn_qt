/******************************************************************************
 *
 * Project:  OpenCPN
 * Authors:  David Register
 *           Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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
 ***************************************************************************
 */

#ifndef __GLTEXTUREMANAGER_H__
#define __GLTEXTUREMANAGER_H__

#include <QString>
#include <QList>
#include <QThread>
#include <QHash>
#include <QTimer>
#include <QRect>

class QProgressDialog;
class ProgressInfoItem
{
public:
    ProgressInfoItem(){}
    ~ProgressInfoItem(){}
    
    QString file_path;
    QString msgx;
};
typedef QList<ProgressInfoItem>      ProgressInfoList;

class JobTicket;
struct OCPN_CompressionThreadMsg
{
public:
    int        type;
    int        nstat;
    int        nstat_max;
    JobTicket  * m_ticket;

    OCPN_CompressionThreadMsg()
    {
        m_ticket = 0;
    }
};

Q_DECLARE_METATYPE(OCPN_CompressionThreadMsg)

class CompressionPoolThread : public QThread
{
    Q_OBJECT
public:
    CompressionPoolThread(JobTicket *ticket = 0);
    void run();
signals:
    void signalSendCompressionMsg(const OCPN_CompressionThreadMsg& msg);
private:

    JobTicket           *m_ticket;
};







class CompressionPoolThread;
class glTexFactory;
class JobTicket
{
public:
    JobTicket();
    ~JobTicket() { free(level0_bits); }
    bool DoJob();
    bool DoJob(const QRect &rect);
    
    glTexFactory *pFact;
    QRect      m_rect;
    int         level_min_request;
    int         ident;
    bool        b_throttle;
    
    CompressionPoolThread *pthread;
    unsigned char *level0_bits;
    unsigned char *comp_bits_array[10];
    QString    m_ChartPath;
    bool        b_abort;
    bool        b_isaborted;
    bool        bpost_zip_compress;
    bool        binplace;
    unsigned char *compcomp_bits_array[10];
    int         compcomp_size_array[10];
    bool        b_inCompressAll;
};

typedef QList<JobTicket*>    JobList;

//      This is a hashmap with Chart full path as key, and glTexFactory as value
typedef QHash<QString, glTexFactory*> ChartPathHashTexfactType;

//      glTextureManager Definition
class ChartBase;
class glTextureManager : public QObject
{
    Q_OBJECT
public:
    glTextureManager();
    ~glTextureManager();

    bool ScheduleJob( glTexFactory *client, const QRect &rect, int level_min,
                      bool b_throttle_thread, bool b_nolimit, bool b_postZip, bool b_inplace);

    int GetRunningJobCount(){ return running_list.count(); }
    int GetJobCount(){ return GetRunningJobCount() + todo_list.count(); }
    bool AsJob( QString const &chart_path ) const;
    void PurgeJobList( QString chart_path = QString() );
    void ClearJobList();
    void ClearAllRasterTextures(void);
    bool PurgeChartTextures(ChartBase *pc, bool b_purge_factory = false);
    bool TextureCrunch(double factor);
    bool FactoryCrunch(double factor);
    void BuildCompressedCache();
    
    //    This is a hash table
    //    key is Chart full path
    //    Value is glTexFactory*
    ChartPathHashTexfactType   m_chart_texfactory_hash;
public slots:
    void OnEvtThread( const OCPN_CompressionThreadMsg& event );
    void OnTimer();

private:    
    bool DoJob( JobTicket *pticket );
    bool DoThreadJob(JobTicket* pticket);
    bool StartTopJob();
    
    JobList             running_list;
    JobList             todo_list;
    int                 m_max_jobs;

    int		m_prevMemUsed;

    QTimer*     m_timer;
    size_t      m_ticks;
    QProgressDialog *m_progDialog;
    QString    m_progMsg;
    unsigned int  m_jcnt;
    ProgressInfoList    progList;
    bool        m_skip;
    bool        m_skipout;
    bool        m_bcompact;
};

class glTextureDescriptor;
void GetFullMap( glTextureDescriptor *ptd,  const QRect &rect, QString chart_path, int level);
int TextureDim(int level);
int TextureTileSize(int level, bool compressed);

#endif
