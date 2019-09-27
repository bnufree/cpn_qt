﻿/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Database Object
 * Author:   David Register, Mark A Sikes
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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

#include "config.h"
#include "chartdb.h"
#include "chartimg.h"
//#include "chart1.h"
//#include "thumbwin.h"
#include "mbtiles.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#include <stdio.h>
#include <math.h>

#include "chcanv.h"

#include "s57chart.h"
#include "cm93.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include "zchxmapmainwindow.h"

extern ColorScheme GetColorScheme();
extern void         LoadS57();

class s52plib;

//extern ThumbWin     *pthumbwin;
extern int          g_nCacheLimit;
extern int          g_memCacheLimit;
extern s52plib      *ps52plib;
extern ChartDB      *ChartData;
extern zchxMapMainWindow          *gFrame;
extern ChartFrameWork              *gChartFrameWork;



bool G_FloatPtInPolygon(MyFlPoint *rgpts, int wnumpts, float x, float y) ;

// ============================================================================
// ChartStack implementation
// ============================================================================

int ChartStack::GetCurrentEntrydbIndex(void)
{
//    qDebug("nEntry = %d, CurrentStackEntry = %d", nEntry, CurrentStackEntry);
    if(nEntry /*&& b_valid*/)
        return DBIndex[CurrentStackEntry];
    else
        return -1;
}

void ChartStack::SetCurrentEntryFromdbIndex(int current_db_index)
{
//    qDebug("nEntry = %d, CurrentStackEntry = %d, current_db_index = %d", nEntry, CurrentStackEntry, current_db_index);
    for(int i=0 ; i < nEntry ; i++)
    {
        if (current_db_index == DBIndex[i])
        {
            CurrentStackEntry = i;
            break;
        }
    }
//    qDebug("nEntry = %d, CurrentStackEntry = %d, current_db_index = %d", nEntry, CurrentStackEntry, current_db_index);
}

int ChartStack::GetDBIndex(int stack_index)
{
//    qDebug("nEntry = %d, stack_index = %d, max_stack = %d", nEntry, stack_index, MAXSTACK);
    if((stack_index >= 0) && (stack_index < nEntry) && (stack_index < MAXSTACK))
        return DBIndex[stack_index];
    else
        return -1;
}

void ChartStack::SetDBIndex(int stack_index, int db_index)
{
//    qDebug("nEntry = %d, stack_index = %d, db_index = %d", nEntry, stack_index, db_index);
      if((stack_index >= 0) && (stack_index < nEntry) && (stack_index < MAXSTACK))
            DBIndex[stack_index] = db_index;
}


bool ChartStack::DoesStackContaindbIndex(int db_index)
{
      for(int i=0 ; i < nEntry ; i++)
      {
            if(db_index == DBIndex[i])
                  return true;
      }

      return false;
}


void ChartStack::AddChart( int db_add )
{
    if( !ChartData ) return;
    if( !ChartData->IsValid() ) return;
    int db_index = db_add;

    int j = nEntry;
    
    if(db_index >= 0) {
        j++;
        nEntry = j;
        SetDBIndex(j-1, db_index);
    }
    //    Remove exact duplicates, i.e. charts that have exactly the same file name and
    //     nearly the same mod time.
    //    These charts can be in the database due to having the exact same chart in different directories,
    //    as may be desired for some grouping schemes
    //    Note that if the target name is actually a directory, then windows fails to produce a valid
    //    file modification time.  Detect GetFileTime() == 0, and skip the test in this case
    for(int id = 0 ; id < j-1 ; id++)
    {
        if(GetDBIndex(id) != -1)
        {
            ChartTableEntry *pm = ChartData->GetpChartTableEntry(GetDBIndex(id));

            for(int jd = id+1; jd < j; jd++)
            {
                if(GetDBIndex(jd) != -1)
                {
                    ChartTableEntry *pn = ChartData->GetpChartTableEntry(GetDBIndex(jd));
                    if( pm->GetFileTime() && pn->GetFileTime()) {
                        if( labs(pm->GetFileTime() - pn->GetFileTime()) < 60 ) {           // simple test
                            if(*(pn->GetpFileName()) == (*(pm->GetpFileName())))
                                SetDBIndex(jd, -1);           // mark to remove
                        }
                    }
                }
            }
        }
    }

    int id = 0;
    while( (id < j) )
    {
        if(GetDBIndex(id) == -1)
        {
            int jd = id+1;
            while( jd < j )
            {
                int db_index = GetDBIndex(jd);
                SetDBIndex(jd-1, db_index);
                jd++;
            }

            j--;
            nEntry = j;

            id = 0;
        }
        else
            id++;
    }


    //    Sort the stack on scale
    int swap = 1;
    int ti;
    while(swap == 1)
    {
        swap = 0;
        for(int i=0 ; i<j-1 ; i++)
        {
            const ChartTableEntry &m = ChartData->GetChartTableEntry(GetDBIndex(i));
            const ChartTableEntry &n = ChartData->GetChartTableEntry(GetDBIndex(i+1));


            if(n.GetScale() < m.GetScale())
            {
                ti = GetDBIndex(i);
                SetDBIndex(i, GetDBIndex(i+1));
                SetDBIndex(i+1, ti);
                swap = 1;
            }
        }
    }

    
}


// ============================================================================
// ChartDB implementation
// ============================================================================

ChartDB::ChartDB()
{
      pChartCache = new wxArrayPtrVoid;

      SetValid(false);                           // until loaded or created
      UnLockCache();
      
      m_b_busy = false;
      m_ticks = 0;

      //    Report cache policy
      if(g_memCacheLimit)
      {
          qDebug("ChartDB Cache policy:  Application target is %d M Bytes", g_memCacheLimit / 1024);
      }
      else
      {
          qDebug("ChartDB Cache policy:  Max open chart limit is %d.", g_nCacheLimit);
      }

}


ChartDB::~ChartDB()
{
//    Empty the cache
      PurgeCache();
      delete pChartCache;
}

bool ChartDB::LoadBinary(const QString & filename, ArrayOfCDI& dir_array_check)
{
      m_dir_array = dir_array_check;
      return ChartDatabase::Read(filename);

      // Check chartDirs against dir_array_check
}

void ChartDB::DeleteCacheEntry(CacheEntry *pce, bool bDelTexture, const QString &msg)
{
     ChartBase *ch = (ChartBase *)pce->pChart;
     if (msg.isEmpty()) {
         qDebug("%s%s", msg.toUtf8().data(), ch->GetFullPath().toUtf8().data());
     }

     // If this chart should happen to be in the thumbnail window....
//     if(pthumbwin)
//     {
//         if (pthumbwin->pThumbChart == ch)  pthumbwin->pThumbChart = NULL;
//     }

     // The glCanvas may be cacheing some information for this chart     
     if (g_glTextureManager)  g_glTextureManager->PurgeChartTextures(ch, bDelTexture);

     pChartCache->removeOne(pce);
     delete ch;
     delete pce;
}

void ChartDB::DeleteCacheEntry(int i, bool bDelTexture, const QString &msg)
{
     CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
     if (pce) DeleteCacheEntry(pce, bDelTexture, msg);
}

void ChartDB::PurgeCache()
{
    qDebug("Chart cache purge");
    QMutexLocker locker(&m_cache_mutex);
    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        DeleteCacheEntry(0, true);
    }
    pChartCache->clear();
}


void ChartDB::PurgeCachePlugins()
{
    //    Empty the cache
    qDebug("Chart cache PlugIn purge");
    QMutexLocker locker(&m_cache_mutex);
    unsigned int nCache = pChartCache->count();
    unsigned int i = 0;
    while(i < nCache){
        CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
        ChartBase *Ch = (ChartBase *)pce->pChart;

        if(CHART_TYPE_PLUGIN == Ch->GetChartType()){
            DeleteCacheEntry(pce, true);
            
            nCache = pChartCache->count();       // restart the while loop
            i = 0;

        } else {
            i++;
        }
    }
}


void ChartDB::ClearCacheInUseFlags(void)
{
    QMutexLocker locker(&m_cache_mutex);
    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
        pce->b_in_use = false;
    }
}


//      Try to purge and delete charts from the cache until the application memory used is
//      until the application memory used is less than {factor * Limit}
//      Purge charts on LRU policy
void ChartDB::PurgeCacheUnusedCharts( double factor)
{
    QMutexLocker locker(&m_cache_mutex);
    //    Use memory limited cache policy, if defined....
    if(g_memCacheLimit)
    {
        //    Check memory status to see if above limit
        int mem_used;
        zchxFuncUtil::getMemoryStatus(0, &mem_used);
        int mem_limit = g_memCacheLimit * factor;

        int nl = pChartCache->count();       // max loop count, by definition

        QString msg("Purging unused chart from cache: ");
        //printf("Try Purge count:  %d\n", nl);
        while( (mem_used > mem_limit) && (nl>0) )
        {
            if( pChartCache->count() < 2 ){
                nl = 0;
                break;
            }

            CacheEntry *pce = FindOldestDeleteCandidate( false );
            if(pce){
                // don't purge background spooler
                DeleteCacheEntry(pce, false /*true*/, msg);
                //printf("DCE, new count is:  %d\n", pChartCache->count());
            }
            else {
                break;
            }

            zchxFuncUtil::getMemoryStatus(0, &mem_used);
            nl--;
        }
    }

    //    Else use chart count cache policy, if defined....
    else if(g_nCacheLimit)
    {
        //    Check chart count to see if above limit
        double fac10 = factor * 10;
        int chart_limit = g_nCacheLimit * fac10 / 10;

        int nl = pChartCache->count();       // max loop count, by definition

        QString msg("Purging unused chart from cache: ");
        while( (nl > chart_limit) && (nl>0) )
        {
            if( pChartCache->count() < 2 ){
                nl = 0;
                break;
            }

            CacheEntry *pce = FindOldestDeleteCandidate( false );
            if(pce){
                // don't purge background spooler
                DeleteCacheEntry(pce, false /*true*/, msg);
            }
            else {
                break;
            }

            nl = pChartCache->count();
        }
    }

}




//-------------------------------------------------------------------------------------------------------
//      Create a Chart
//      This version creates a fully functional UI-capable chart.
//-------------------------------------------------------------------------------------------------------

ChartBase *ChartDB::GetChart(const char *theFilePath, ChartClassDescriptor &chart_desc) const
{
    QString fileName = QString::fromUtf8(theFilePath);
    QFile fn(fileName);


    if(!fn.exists()) {
        //    Might be a directory
        if( !QDir(fileName).exists() ) {
            qDebug("   ...file does not exist: %s", theFilePath);
            return NULL;
        }
    }
    ChartBase *pch = NULL;
    QString chartExt = QFileInfo(fn).suffix().toUpper();

    //现在只处理S57的.000数据
#if 1
    if(chartExt == "000" || chartExt == "S57")
    {
        LoadS57();
        pch = new s57chart;
        return pch;
    }
#else
    if (chartExt == "XZ") {
        QString npath = fileName;
        npath = npath.left(npath.length()-3); //去掉.xz
        fn.setFileName(npath);
        chartExt = QFileInfo(npath).suffix().toUpper();
    }
    if (chartExt == "KAP") {
        pch = new ChartKAP;
    }
    else if (chartExt == "GEO") {
        pch = new ChartGEO;
    }
    else if (chartExt == "MBTILES") {
        pch = new ChartMBTiles;
    }
    else if (chartExt == "000" || chartExt == "S57") {
        LoadS57();
        pch = new s57chart;
    }
    else if (chart_desc.m_descriptor_type == PLUGIN_DESCRIPTOR) {
        qDebug()<<"chart plugin not supported yet";
        LoadS57();
        ChartPlugInWrapper *cpiw = new ChartPlugInWrapper(chart_desc.m_class_name);
        pch = (ChartBase *)cpiw;
    }
    else
    {
        QRegularExpression rxName("[0-9]+");
        QRegularExpression rxExt("[A-G]");
        if (rxName.match(QFileInfo(fn).fileName()).hasMatch() && rxExt.match(chartExt).hasMatch())
        {
            pch = new cm93compchart;
        } else {
            //    Might be a directory
            if( QDir(fileName).exists() )
                pch = new cm93compchart;
        }
    }
#endif

    return pch;
}

//      Build a Chart Stack, and add the indicated chart to the stack, even if the chart does not
//      cover the lat/lon specification

int ChartDB::BuildChartStack(ChartStack * cstk, float lat, float lon, int db_add, int groupIndex )
{
    BuildChartStack(cstk, lat, lon, groupIndex);
    
    if (db_add >= 0 )
    {
        cstk->AddChart( db_add );
    }
    
    return cstk->nEntry;
}


int ChartDB::BuildChartStack(ChartStack * cstk, float lat, float lon, int groupIndex)
{
    int i=0;
    int j=0;

    if(!IsValid())     return 0;                           // Database is not properly initialized
    if(!cstk)         return 0;                           // Chartstack not ready yet

    int nEntry = GetChartTableEntries();
    for(int db_index=0 ; db_index<nEntry ; db_index++)
    {
        const ChartTableEntry &cte = GetChartTableEntry(db_index);
        //    Check to see if the candidate chart is in the currently active group
        bool b_group_add = false;
        if(groupIndex > 0)
        {
            const int ng = cte.GetGroupArray().size();
            for(int ig=0 ; ig < ng; ig++)
            {
                if(groupIndex == cte.GetGroupArray()[ig])
                {
                    b_group_add = true;
                    break;
                }
            }
        }
        else
            b_group_add = true;

        bool b_pos_add = false;
        if(b_group_add)
        {
            //  Plugin loading is deferred, so the chart may have been disabled elsewhere.
            //  Tentatively reenable the chart so that it appears in the piano.
            //  It will get disabled later if really not useable
            if(cte.GetChartType() == CHART_TYPE_PLUGIN){
                ChartTableEntry *pcte = (ChartTableEntry*)&cte;
                pcte->ReEnable();
            }

            if(CheckPositionWithinChart(db_index, lat, lon)  &&  (j < MAXSTACK) )
                b_pos_add = true;

            //    Check the special case where chart spans the international dateline
            else if( (cte.GetLonMax() > 180.) && (cte.GetLonMin() < 180.) )
            {
                if(CheckPositionWithinChart(db_index, lat, lon + 360.)  &&  (j < MAXSTACK) )
                    b_pos_add = true;
            }
            //    Western hemisphere, some type of charts
            else if( (cte.GetLonMax() > 180.) && (cte.GetLonMin() > 180.) )
            {
                if(CheckPositionWithinChart(db_index, lat, lon + 360.)  &&  (j < MAXSTACK) )
                    b_pos_add = true;
            }


        }

        bool b_available = true;
        //  Verify PlugIn charts are actually available
        if(b_group_add && b_pos_add && (cte.GetChartType() == CHART_TYPE_PLUGIN)){

            ChartTableEntry *pcte = (ChartTableEntry*)&cte;
            if( !IsChartAvailable(db_index) ){
                pcte->SetAvailable(false);
                b_available = false;
            }
            else{
                pcte->SetAvailable(true);
                pcte->ReEnable();
            }
        }

        if(b_group_add && b_pos_add && b_available){                // add it
            j++;
            cstk->nEntry = j;
            cstk->SetDBIndex(j-1, db_index);
        }
    }

    cstk->nEntry = j;


    //    Remove exact duplicates, i.e. charts that have exactly the same file name and nearly the same mod time
    //    These charts can be in the database due to having the exact same chart in different directories,
    //    as may be desired for some grouping schemes
    //    Note that if the target name is actually a directory, then windows fails to produce a valid
    //    file modification time.  Detect GetFileTime() == 0, and skip the test in this case
    for(int id = 0 ; id < j-1 ; id++)
    {
        if(cstk->GetDBIndex(id) != -1)
        {
            const ChartTableEntry &ctem = GetChartTableEntry(cstk->GetDBIndex(id));

            for(int jd = id+1; jd < j; jd++)
            {
                if(cstk->GetDBIndex(jd) != -1)
                {
                    const ChartTableEntry &cten = GetChartTableEntry(cstk->GetDBIndex(jd));
                    if( ctem.GetFileTime() && cten.GetFileTime()) {
                        if( labs(ctem.GetFileTime() - cten.GetFileTime()) < 60 ) {           // simple test
                            if(*(cten.GetpFileName()) == *(ctem.GetpFileName()))
                                cstk->SetDBIndex(jd, -1);           // mark to remove
                        }
                    }
                }
            }
        }
    }

    int id = 0;
    while( (id < j) )
    {
        if(cstk->GetDBIndex(id) == -1)
        {
            int jd = id+1;
            while( jd < j )
            {
                int db_index = cstk->GetDBIndex(jd);
                cstk->SetDBIndex(jd-1, db_index);
                jd++;
            }

            j--;
            cstk->nEntry = j;

            id = 0;
        }
        else
            id++;
    }






    //    Sort the stack on scale
    int swap = 1;
    int ti;
    while(swap == 1)
    {
        swap = 0;
        for(i=0 ; i<j-1 ; i++)
        {
            const ChartTableEntry &m = GetChartTableEntry(cstk->GetDBIndex(i));
            const ChartTableEntry &n = GetChartTableEntry(cstk->GetDBIndex(i+1));


            if(n.GetScale() < m.GetScale())
            {
                ti = cstk->GetDBIndex(i);
                cstk->SetDBIndex(i, cstk->GetDBIndex(i+1));
                cstk->SetDBIndex(i+1, ti);
                swap = 1;
            }
        }
    }



    cstk->b_valid = true;

    return j;
}

bool ChartDB::IsChartInGroup(const int db_index, const int group)
{
    ChartTableEntry *pt = (ChartTableEntry *)&GetChartTableEntry(db_index);

    //    Check to see if the candidate chart is in the designated group
    bool b_in_group = false;
    if(group > 0)
    {
        for(unsigned int ig=0 ; ig < pt->GetGroupArray().size(); ig++)
        {
            if(group == pt->GetGroupArray()[ig])
            {
                b_in_group = true;
                break;
            }
        }
    }
    else
        b_in_group = true;

    return b_in_group;
}

bool ChartDB::IsENCInGroup(const int groupIndex)
{
    // Walk the database, looking in specified group for any vector chart
    bool retVal = false;

    for(int db_index=0 ; db_index<GetChartTableEntries() ; db_index++){
        const ChartTableEntry &cte = GetChartTableEntry(db_index);

        //    Check to see if the candidate chart is in the currently active group
        bool b_group_add = false;
        if(groupIndex > 0)
        {
            const int ng = cte.GetGroupArray().size();
            for(int ig=0 ; ig < ng; ig++){
                if(groupIndex == cte.GetGroupArray()[ig]){
                    b_group_add = true;
                    break;
                }
            }
        }
        else
            b_group_add = true;


        if(b_group_add){
            if(cte.GetChartFamily() == CHART_FAMILY_VECTOR){
                retVal = true;
                break;   // the outer for loop
            }
        }
    }

    return retVal;
}

//-------------------------------------------------------------------
//    Check to see it lat/lon is within a database chart at index
//-------------------------------------------------------------------
bool ChartDB::CheckPositionWithinChart(int index, float lat, float lon)
{
    const ChartTableEntry *pt = &GetChartTableEntry(index);

    //    First check on rough Bounding box

    if((lat <= pt->GetLatMax()) &&
            (lat >= pt->GetLatMin()) &&
            (lon >= pt->GetLonMin()) &&
            (lon <= pt->GetLonMax()))
    {
        //    Double check on Primary Ply points polygon

        bool bInside = G_FloatPtInPolygon((MyFlPoint *)pt->GetpPlyTable(),
                                          pt->GetnPlyEntries(),
                                          lon, lat);

        if(bInside )
        {
            if(pt->GetnAuxPlyEntries())
            {
                for(int k=0 ; k<pt->GetnAuxPlyEntries() ; k++)
                {
                    bool bAuxInside = G_FloatPtInPolygon((MyFlPoint *)pt->GetpAuxPlyTableEntry(k),
                                                         pt->GetAuxCntTableEntry(k),lon, lat);
                    if(bAuxInside)
                        return true;;
                }

            }
            else
                return true;
        }
    }

    return false;
}





//-------------------------------------------------------------------
//    Compare Chart Stacks
//-------------------------------------------------------------------
bool ChartDB::EqualStacks(ChartStack *pa, ChartStack *pb)
{
    if((pa == 0) || (pb == 0))
        return false;
    if((!pa->b_valid) || (!pb->b_valid))
        return false;
    if(pa->nEntry != pb->nEntry)
        return false;

    for(int i=0 ; i<pa->nEntry ; i++)
    {
        if(pa->GetDBIndex(i) != pb->GetDBIndex(i))
            return false;
    }

    return true;
}

//-------------------------------------------------------------------
//    Copy Chart Stacks
//-------------------------------------------------------------------
bool ChartDB::CopyStack(ChartStack *pa, ChartStack *pb)
{
    if((pa == 0) || (pb == 0))
        return false;
    pa->nEntry = pb->nEntry;

    for(int i=0 ; i<pa->nEntry ; i++)
        pa->SetDBIndex(i, pb->GetDBIndex(i));

    pa->CurrentStackEntry = pb->CurrentStackEntry;

    pa->b_valid = pb->b_valid;


    return true;
}


QString ChartDB::GetFullPath(ChartStack *ps, int stackindex)
{
    int dbIndex = ps->GetDBIndex(stackindex);
    Q_ASSERT( dbIndex >= 0 );

    return QString::fromUtf8(GetChartTableEntry(dbIndex).GetpFullPath());
}

//-------------------------------------------------------------------
//    Get PlyPoint from stack
//-------------------------------------------------------------------

int ChartDB::GetCSPlyPoint(ChartStack *ps, int stackindex, int plyindex, float *lat, float *lon)
{
    int dbIndex = ps->GetDBIndex(stackindex);
    Q_ASSERT( dbIndex >= 0 );

    const ChartTableEntry &entry = GetChartTableEntry( dbIndex );
    if(entry.GetnPlyEntries())
    {
        float *fp = entry.GetpPlyTable();
        fp += plyindex*2;
        *lat = *fp;
        fp++;
        *lon = *fp;
    }

    return entry.GetnPlyEntries();
}




//-------------------------------------------------------------------
//    Get Chart Scale
//-------------------------------------------------------------------
int ChartDB::GetStackChartScale(ChartStack *ps, int stackindex, char *buf, int nbuf)
{
    int dbindex = ps->GetDBIndex(stackindex);
    Q_ASSERT(dbindex >= 0);

    const ChartTableEntry &entry = GetChartTableEntry( dbindex );
    int sc = entry.GetScale();
    if(buf)
        sprintf(buf, "%d", sc);

    return sc;
}

//-------------------------------------------------------------------
//    Find ChartStack entry index corresponding to Full Path name, if present
//-------------------------------------------------------------------
int ChartDB::GetStackEntry(ChartStack *ps, QString fp)
{
    for(int i=0 ; i<ps->nEntry ; i++)
    {
        int dbindex = ps->GetDBIndex( i );
        Q_ASSERT(dbindex >= 0);

        const ChartTableEntry &entry = GetChartTableEntry( dbindex );
        if(fp ==  QString::fromUtf8(entry.GetpFullPath()) )
            return i;
    }

    return -1;
}

//-------------------------------------------------------------------
//    Get CSChart Type
//-------------------------------------------------------------------
ChartTypeEnum ChartDB::GetCSChartType(ChartStack *ps, int stackindex)
{
    if ( IsValid() ) {
        int dbindex = ps->GetDBIndex(stackindex);
        if (dbindex >= 0)
            return (ChartTypeEnum)GetChartTableEntry(dbindex).GetChartType();
    }
    return CHART_TYPE_UNKNOWN;
}


ChartFamilyEnum ChartDB::GetCSChartFamily(ChartStack *ps, int stackindex)
{
    if( IsValid() )
    {
        int dbindex = ps->GetDBIndex(stackindex);
        if (dbindex >= 0) {
            const ChartTableEntry &entry = GetChartTableEntry( dbindex );

            ChartTypeEnum type = (ChartTypeEnum)entry.GetChartType();
            switch(type)
            {
            case  CHART_TYPE_KAP:          return CHART_FAMILY_RASTER;
            case  CHART_TYPE_GEO:          return CHART_FAMILY_RASTER;
            case  CHART_TYPE_S57:          return CHART_FAMILY_VECTOR;
            case  CHART_TYPE_CM93:         return CHART_FAMILY_VECTOR;
            case  CHART_TYPE_CM93COMP:     return CHART_FAMILY_VECTOR;
            case  CHART_TYPE_DUMMY:        return CHART_FAMILY_RASTER;
            default:                       return CHART_FAMILY_UNKNOWN;
            }
        }
    }
    return CHART_FAMILY_UNKNOWN;
}


std::vector<int> ChartDB::GetCSArray(ChartStack *ps)
{
    std::vector<int> ret;

    if(ps)
    {
        ret.reserve(ps->nEntry);
        for(int i=0 ; i<ps->nEntry ; i++)
        {
            ret.push_back(ps->GetDBIndex(i));
        }
    }

    return ret;
}


bool ChartDB::IsChartInCache(int dbindex)
{
    bool bInCache = false;

    QMutexLocker locker(&m_cache_mutex);

    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
        if(pce->dbIndex == dbindex)
        {
            if (pce->pChart != 0 && ((ChartBase *)pce->pChart)->IsReadyToRender())
                bInCache = true;
            break;
        }
    }

    return bInCache;
}


bool ChartDB::IsChartInCache(QString path)
{
    bool bInCache = false;
    QMutexLocker locker(&m_cache_mutex);

    //    Search the cache
    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
        if(pce->FullPath == path)
        {
            if (pce->pChart != 0 && ((ChartBase *)pce->pChart)->IsReadyToRender())
                bInCache = true;
            break;
        }
    }
    return bInCache;
}

bool ChartDB::IsChartLocked( int index )
{
    QMutexLocker locker(&m_cache_mutex);
    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
        if(pce->dbIndex == index)
        {
            bool ret = pce->n_lock > 0;
            return ret;
        }
    }
    
    return false;
}

bool ChartDB::LockCacheChart( int index )
{
    //    Search the cache
    bool ret = false;
    QMutexLocker locker(&m_cache_mutex);

    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
        if(pce->dbIndex == index )
        {
            pce->n_lock++;
            ret = true;
            break;
        }
    }
    return ret;
}

void ChartDB::UnLockCacheChart( int index )
{
    //    Search the cache
    QMutexLocker locker(&m_cache_mutex);
    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
        if(pce->dbIndex == index)
        {
            if(pce->n_lock > 0)
                pce->n_lock--;
            break;
        }
    }
}

void ChartDB::UnLockAllCacheCharts()
{
    //    Walk the cache
    QMutexLocker locker(&m_cache_mutex);

    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
        if(pce->n_lock > 0)
            pce->n_lock--;
    }
}


//-------------------------------------------------------------------
//    Open Chart
//-------------------------------------------------------------------
ChartBase *ChartDB::OpenChartFromDB(int index, ChartInitFlag init_flag)
{
    return OpenChartUsingCache(index, init_flag);
}

ChartBase *ChartDB::OpenChartFromDB(QString chart_path, ChartInitFlag init_flag)
{
    int dbii = FinddbIndex( chart_path );
    return OpenChartUsingCache(dbii, init_flag);
}


ChartBase *ChartDB::OpenChartFromStack(ChartStack *pStack, int StackEntry, ChartInitFlag init_flag)
{
    return OpenChartUsingCache(pStack->GetDBIndex(StackEntry), init_flag);
}

ChartBase *ChartDB::OpenChartFromDBAndLock( int index, ChartInitFlag init_flag, bool lock )
{
    QMutexLocker locker(&m_critSect);
    ChartBase *pret = OpenChartUsingCache(index, init_flag);
    if (lock && pret)
        LockCacheChart( index );
    return pret;
}

ChartBase *ChartDB::OpenChartFromDBAndLock(QString chart_path, ChartInitFlag init_flag)
{
    int dbii = FinddbIndex( chart_path );
    return OpenChartFromDBAndLock(dbii, init_flag);
}

CacheEntry *ChartDB::FindOldestDeleteCandidate( bool blog)
{
    CacheEntry *pret = 0;
    

    unsigned int nCache = pChartCache->count();
    if(nCache > 1)
    {
        if(blog) qDebug("Searching chart cache for oldest entry");
        int LRUTime = m_ticks;
        int iOldest = 0;
        for(unsigned int i=0 ; i<nCache ; i++)
        {
            CacheEntry *pce = (CacheEntry *)(pChartCache->at(i));
            if(pce->RecentTime < LRUTime && !pce->n_lock){
                if (!isSingleChart((ChartBase *)(pce->pChart))) {
                    LRUTime = pce->RecentTime;
                    iOldest = i;
                }
            }
        }
        int dt = m_ticks - LRUTime;

        CacheEntry *pce = (CacheEntry *)(pChartCache->at(iOldest));
        ChartBase *pDeleteCandidate =  (ChartBase *)(pce->pChart);

        if( !pce->n_lock &&  !isSingleChart(pDeleteCandidate))
        {
            if(blog) qDebug("Oldest unlocked cache index is %d, delta t is %d", iOldest, dt);
            pret = pce;
        } else
        {
            qDebug("All chart in cache locked, size: %d", nCache);
        }

    }

    
    return pret;
}



ChartBase *ChartDB::OpenChartUsingCache(int dbindex, ChartInitFlag init_flag)
{
    if((dbindex < 0) || (dbindex > GetChartTableEntries()-1))
        return NULL;

    //      printf("Opening chart %d   lock: %d\n", dbindex, m_b_locked);
    const ChartTableEntry &cte = GetChartTableEntry(dbindex);
    QString ChartFullPath = QString::fromUtf8(cte.GetpFullPath());
    ChartTypeEnum chart_type = (ChartTypeEnum)cte.GetChartType();
    ChartFamilyEnum chart_family = (ChartFamilyEnum)cte.GetChartFamily();

//    QString msg1;
//    msg1.sprintf("OpenChartUsingCache:  type %d  ", chart_type);
//    qDebug()<<(msg1 + ChartFullPath);

    if(cte.GetLatMax() > 90.0)          // Chart has been disabled...
        return NULL;

    ChartBase *Ch = NULL;
    CacheEntry *pce = NULL;
    int old_lock = 0;

    bool bInCache = false;

    //    Search the cache
    {
        QMutexLocker lock(&m_cache_mutex);
        unsigned int nCache = pChartCache->count();
        m_ticks++;
        for(unsigned int i=0 ; i<nCache ; i++)
        {
            pce = (CacheEntry *)(pChartCache->at(i));
            if(pce->FullPath == ChartFullPath)
            {
                Ch = (ChartBase *)pce->pChart;
                bInCache = true;
                break;
            }
        }

        if(bInCache)
        {
//            QString msg;
//            msg.sprintf("OpenChartUsingCache, IN cache: cache size: %d\n", (int)pChartCache->count());
//            qDebug()<<msg;
            if(FULL_INIT == init_flag)                            // asking for full init?
            {
                if(Ch->IsReadyToRender())
                {
                    if(pce){
                        pce->RecentTime = m_ticks;           // chart is OK
                        pce->b_in_use = true;
                    }
                    return Ch;
                }
                else
                {
//                    if(pthumbwin && pthumbwin->pThumbChart == Ch)
//                        pthumbwin->pThumbChart = NULL;
                    delete Ch;                                  // chart is not useable
                    old_lock = pce->n_lock;
                    pChartCache->removeOne(pce);                   // so remove it
                    delete pce;

                    bInCache = false;
                }
            }
            else                                                  // assume if in cache, the chart can do thumbnails
            {
                if(pce){
                    pce->RecentTime = m_ticks;
                    pce->b_in_use = true;
                }
                return Ch;
            }
        }

        if(!bInCache)                    // not in cache
        {
            m_b_busy = true;
            if( !m_b_locked ) {
                //    Use memory limited cache policy, if defined....
                if(g_memCacheLimit)
                {
                    //    Check memory status to see if enough room to open another chart
                    int mem_used;
                    zchxFuncUtil::getMemoryStatus(0, &mem_used);

//                    qDebug("OpenChartUsingCache, NOT in cache:   cache size: %d",  (int)pChartCache->count());
//                    qDebug("   OpenChartUsingCache:  type %d  %s ", chart_type, ChartFullPath.toUtf8().data());

                    if((mem_used > g_memCacheLimit * 8 / 10) && (pChartCache->count() > 2)) {
                        QString msg("Removing oldest chart from cache: ");
                        while (1)
                        {
                            CacheEntry *pce = FindOldestDeleteCandidate(true);
                            if (pce == 0)
                                break;                      // no possible delete candidate

                            // purge texture cache, really need memory here
                            DeleteCacheEntry(pce, true, msg);

                            zchxFuncUtil::getMemoryStatus(0, &mem_used);
                            if((mem_used < g_memCacheLimit * 8 / 10) || (pChartCache->count() <= 2))
                                break;

                        }  // while
                    }
                }

                else        // Use n chart cache policy, if memory-limit  policy is not used
                {
                    //      Limit cache to n charts, tossing out the oldest when space is needed
                    unsigned int nCache = pChartCache->count();
                    if (nCache > (unsigned int)g_nCacheLimit && nCache > 2)
                    {
                        QString msg("Removing oldest chart from cache:");
                        while (nCache > (unsigned int)g_nCacheLimit)
                        {
                            CacheEntry *pce = FindOldestDeleteCandidate( true );
                            if (pce == 0)
                                break;
                            
                            DeleteCacheEntry(pce, true, msg);
                            nCache--;
                        }
                    }
                    
                }
            }
        }
    } // unlock

    if(!bInCache)                    // not in cache
    {
//        qDebug("Creating new chart");

#if 0

        if(chart_type == CHART_TYPE_KAP)
            Ch = new ChartKAP();

        else if(chart_type == CHART_TYPE_GEO)
            Ch = new ChartGEO();

        else if(chart_type == CHART_TYPE_MBTILES)
            Ch = new ChartMBTiles();

        else if(chart_type == CHART_TYPE_S57)
#endif
        if(chart_type == CHART_TYPE_S57){
            LoadS57();
            Ch = new s57chart();
            s57chart *Chs57 = static_cast<s57chart*>(Ch);

            Chs57->SetNativeScale(cte.GetScale());

            //    Explicitely set the chart extents from the database to
            //    support the case wherein the SENC file has not yet been built
            Extent ext;
            ext.NLAT = cte.GetLatMax();
            ext.SLAT = cte.GetLatMin();
            ext.WLON = cte.GetLonMin();
            ext.ELON = cte.GetLonMax();
            Chs57->SetFullExtent(ext);
        }
#if 0
        else if(chart_type == CHART_TYPE_CM93)
        {
            LoadS57();
            Ch = new cm93chart();
            cm93chart *Chcm93 = static_cast<cm93chart*>(Ch);

            Chcm93->SetNativeScale(cte.GetScale());

            //    Explicitely set the chart extents from the database to
            //    support the case wherein the SENC file has not yet been built
            Extent ext;
            ext.NLAT = cte.GetLatMax();
            ext.SLAT = cte.GetLatMin();
            ext.WLON = cte.GetLonMin();
            ext.ELON = cte.GetLonMax();
            Chcm93->SetFullExtent(ext);
        }

        else if(chart_type == CHART_TYPE_CM93COMP)
        {
            LoadS57();
            Ch = new cm93compchart();

            cm93compchart *Chcm93 = static_cast<cm93compchart*>(Ch);

            Chcm93->SetNativeScale(cte.GetScale());

            //    Explicitely set the chart extents from the database to
            //    support the case wherein the SENC file has not yet been built
            Extent ext;
            ext.NLAT = cte.GetLatMax();
            ext.SLAT = cte.GetLatMin();
            ext.WLON = cte.GetLonMin();
            ext.ELON = cte.GetLonMax();
            Chcm93->SetFullExtent(ext);
        }

        else if(chart_type == CHART_TYPE_PLUGIN)
        {
            QFileInfo fn(ChartFullPath);
            QString ext = fn.suffix();
            ext.insert(0, "*.");
            QString ext_upper = ext.toUpper();
            QString ext_lower = ext.toLower();
            QString chart_class_name;

            //    Search the array of chart class descriptors to find a match
            //    bewteen the search mask and the the chart file extension

            for(unsigned int i=0 ; i < m_ChartClassDescriptorArray.count() ; i++)
            {
                if(m_ChartClassDescriptorArray[i]->m_descriptor_type == PLUGIN_DESCRIPTOR)
                {
                    if(m_ChartClassDescriptorArray[i]->m_search_mask == ext_upper)
                    {
                        chart_class_name = m_ChartClassDescriptorArray[i]->m_class_name;
                        break;
                    }
                    if(m_ChartClassDescriptorArray[i]->m_search_mask == ext_lower)
                    {
                        chart_class_name = m_ChartClassDescriptorArray[i]->m_class_name;
                        break;
                    }
                    if(QRegExp(m_ChartClassDescriptorArray.at(i)->m_search_mask).exactMatch(ChartFullPath)) {
                        chart_class_name = m_ChartClassDescriptorArray.at(i)->m_class_name;
                        break;
                    }

                }
            }

            //                chart_class_name = cte.GetChartClassName();
            if(chart_class_name.length())
            {
                qDebug()<<"chart plugin not supported yet."<<chart_class_name;
                ChartPlugInWrapper *cpiw = new ChartPlugInWrapper(chart_class_name);
                Ch = (ChartBase *)cpiw;
                if(chart_family == CHART_FAMILY_VECTOR)
                    LoadS57();
            }
        }
#endif


        else{
            Ch = NULL;
            qDebug("Unknown chart type");
        }


        if(Ch)
        {
            InitReturn ir;
            s52plib *plib = ps52plib;
            QString msg_fn(ChartFullPath);
            msg_fn.replace("%", "%%");

            //    Vector charts need a PLIB for useful display....
            if((chart_family != CHART_FAMILY_VECTOR) ||((chart_family == CHART_FAMILY_VECTOR) && plib) )
            {
                qDebug("Initializing Chart %s", msg_fn.toUtf8().data());
                ir = Ch->Init(ChartFullPath, init_flag);    // using the passed flag
                Ch->SetColorScheme(/*pParent->*/GetColorScheme());
            }
            else
            {
                qDebug("   No PLIB, Skipping vector chart  %s", msg_fn.toStdString().c_str());
                ir = INIT_FAIL_REMOVE;

            }

            if(INIT_OK == ir)
            {

                //    always cache after a new chart has been created
                //    or it may leak CacheEntry in createthumbnail
                //                        if(FULL_INIT == init_flag)
                {
                    pce = new CacheEntry;
                    pce->FullPath = ChartFullPath;
                    pce->pChart = Ch;
                    pce->dbIndex = dbindex;
                    //                              printf("    Adding chart %d\n", dbindex);
                    pce->RecentTime = m_ticks;
                    pce->n_lock = old_lock;

                    QMutexLocker locker(&m_cache_mutex);
                    pChartCache->append((void *)pce);
                }
            }
            else if(INIT_FAIL_REMOVE == ir)                 // some problem in chart Init()
            {
                qDebug("Problem initializing Chart %s", msg_fn.toStdString().c_str());

                delete Ch;
                Ch = NULL;

                //          Mark this chart in the database, so that it will not be seen during this run, but will stay in the database
                DisableChart(ChartFullPath);
            }
            else if((INIT_FAIL_RETRY == ir) || (INIT_FAIL_NOERROR == ir))   // recoverable problem in chart Init()
            {
                qDebug("Recoverable problem initializing Chart %s", msg_fn.toStdString().c_str());
                delete Ch;
                Ch = NULL;
            }


            if(INIT_OK != ir)
            {
                if(1/*INIT_FAIL_NOERROR != ir*/)
                {
                    qDebug("   OpenChartFromStack... Error opening chart %s ... return code %d", msg_fn.toStdString().c_str(), ir);
                }
            }

        }


        m_b_busy = false;

        return Ch;
    }

    return NULL;
}

// 
bool ChartDB::DeleteCacheChart(ChartBase *pDeleteCandidate)
{
    bool retval = false;
    QMutexLocker locker(&m_cache_mutex);
    if(!isSingleChart(pDeleteCandidate))
    {
        // Find the chart in the cache
        CacheEntry *pce = NULL;
        for(unsigned int i=0 ; i< pChartCache->count() ; i++)
        {
            pce = (CacheEntry *)(pChartCache->at(i));
            if((ChartBase *)(pce->pChart) == pDeleteCandidate)
            {
                break;
            }
        }

        if(pce)
        {
            if(pce->n_lock > 0)
                pce->n_lock--;

            if( pce->n_lock == 0) {
                DeleteCacheEntry( pce);
                retval = true;
            }
        }
    }

    return retval;
}

/*
*/
void ChartDB::ApplyColorSchemeToCachedCharts(ColorScheme cs)
{
    ChartBase *Ch;
    CacheEntry *pce;
    //    Search the cache

    QMutexLocker locker(&m_cache_mutex);

    unsigned int nCache = pChartCache->count();
    for(unsigned int i=0 ; i<nCache ; i++)
    {
        pce = (CacheEntry *)(pChartCache->at(i));
        Ch = (ChartBase *)pce->pChart;
        if(Ch)
            Ch->SetColorScheme(cs, true);

    }

}

//-------------------------------------------------------------------
//    Open a chart from the stack with conditions
//      a) Search Direction Start
//      b) Requested Chart Type
//-------------------------------------------------------------------

ChartBase *ChartDB::OpenStackChartConditional(ChartStack *ps, int index_start, bool bSearchDir, ChartTypeEnum New_Type, ChartFamilyEnum New_Family_Fallback)
{
    int index;

    int delta_index;
    ChartBase *ptc = NULL;

    if(bSearchDir == 1)
        delta_index = -1;

    else
        delta_index = 1;



    index = index_start;

    while((index >= 0) && (index < ps->nEntry))
    {
        ChartTypeEnum chart_type = (ChartTypeEnum)GetCSChartType(ps, index);
        if((chart_type == New_Type) || (New_Type == CHART_TYPE_DONTCARE))
        {
            ptc = OpenChartFromStack(ps, index);
            if (NULL != ptc)
                break;
        }
        index += delta_index;
    }

    //    Fallback, no useable chart of specified type found, so try for family match
    if(NULL == ptc)
    {
        index = index_start;

        while((index >= 0) && (index < ps->nEntry))
        {
            ChartFamilyEnum chart_family = GetCSChartFamily(ps, index);
            if(chart_family == New_Family_Fallback)
            {
                ptc = OpenChartFromStack(ps, index);

                if (NULL != ptc)
                    break;

            }
            index += delta_index;
        }
    }

    return ptc;
}

void ChartDB::createDomTagAndNode(QDomElement *root, QDomDocument& doc, const QString &tag, const QString &text)
{
    if(!root) return;
    QDomElement node = doc.createElement("tag" );
    QDomText tnode = doc.createTextNode(text);
    node.appendChild(tnode );
    root->appendChild(node);
}

//extern arrayofCanvasPtr  g_canvasArray;

bool ChartDB::isSingleChart(ChartBase *chart)
{
   if (chart == nullptr)
       return false;
   if(gChartFrameWork && gChartFrameWork->m_singleChart == chart){
      return true;
   }
   return false;
}

QDomDocument ChartDB::GetXMLDescription(int dbIndex, bool b_getGeom)
{
    QDomDocument doc;
    if(!IsValid() || (dbIndex >= GetChartTableEntries()))
        return doc;
#if 0
    bool b_remove = !IsChartInCache(dbIndex);

    QDomElement pcell_node;
    //   Open the chart, without cacheing it
    ChartBase *pc = OpenChartFromDB(dbIndex, HEADER_ONLY);
    b_remove = !IsChartInCache(dbIndex);
    const ChartTableEntry &cte = GetChartTableEntry(dbIndex);


    if( CHART_FAMILY_RASTER ==  (ChartFamilyEnum)cte.GetChartFamily())
    {
        pcell_node = doc.createElement("chart");
        QString path = GetDBChartFileName(dbIndex);
        createDomTagAndNode(&pcell_node, doc, "path", GetDBChartFileName(dbIndex));

        QFileInfo name(path);
        createDomTagAndNode(&pcell_node, doc, "name", name.fileName());

        if(pc)
        {
            createDomTagAndNode(&pcell_node, doc, "lname", pc->GetName());
        }

        createDomTagAndNode(&pcell_node, doc, "cscale", QString("").sprintf("%d",cte.GetScale()));

        QDateTime file_date = QDateTime::fromTime_t(cte.GetFileTime()).toUTC();
        QString sfile_date = file_date.toString("yyyy-MM-dd T hh:mm:ss Z");
        createDomTagAndNode(&pcell_node, doc, "local_file_datetime_iso8601", sfile_date);
        if(pc)
        {
            createDomTagAndNode(&pcell_node, doc, "source_edition", pc->GetSE());
            QDateTime sdt = pc->GetEditionDate();
            QString ssdt = "Unknown";
            if(sdt.isValid()) ssdt = sdt.toString("Ymd");
            createDomTagAndNode(&pcell_node, doc, "source_date", ssdt);
        }
    }

    else if( CHART_FAMILY_VECTOR ==  (ChartFamilyEnum)cte.GetChartFamily() )
    {
        pcell_node = doc.createElement( "cell" );
        QString path = GetDBChartFileName(dbIndex);
        createDomTagAndNode(&pcell_node, doc, "path", GetDBChartFileName(dbIndex));

        QFileInfo name(path);
        createDomTagAndNode(&pcell_node, doc, "name", name.fileName() );
        createDomTagAndNode(&pcell_node, doc, "cscale", QString("").sprintf("%d",cte.GetScale()));

        QDateTime file_date = QDateTime::fromTime_t(cte.GetFileTime()).toUTC();
        QString sfile_date = file_date.toString("yyyy-MM-dd T hh:mm:ss Z");
        createDomTagAndNode(&pcell_node, doc, "local_file_datetime_iso8601", sfile_date);
        if(pc)
        {
            createDomTagAndNode(&pcell_node, doc, "edth", pc->GetSE());
        }

        s57chart *pcs57 = dynamic_cast<s57chart*>(pc);
        if(pcs57)
        {
            createDomTagAndNode(&pcell_node, doc, "isdt", pcs57->GetISDT());

            QString LastUpdateDate;
            int updn = pcs57->ValidateAndCountUpdates( path, "", LastUpdateDate, false);
            createDomTagAndNode(&pcell_node, doc, "updn", QString::number(updn));
            createDomTagAndNode(&pcell_node, doc, "uadt", LastUpdateDate);
        }
    }
    if(!pcell_node.isNull() && b_getGeom)
    {
        QDomElement node = doc.createElement("cov" );
        pcell_node.appendChild ( node );

        //    Primary table
        if(cte.GetnPlyEntries())
        {
            QDomElement panelnode = doc.createElement("panel");
            node.appendChild ( panelnode );
            QString panel_no;
            panel_no.sprintf("%d", 0);
            QDomText anode = doc.createTextNode(panel_no );
            panelnode.appendChild ( anode );

            float *pf = cte.GetpPlyTable();
            for(int j=0 ; j < cte.GetnPlyEntries() ; j++)
            {
                QDomElement vnode = doc.createElement("vertex" );
                panelnode.appendChild ( vnode );

                QDomElement latnode = doc.createElement("lat" );
                vnode.appendChild ( latnode );

                float l = *pf++;
                QString sl;
                sl.sprintf("%.5f", l);
                QDomText vtnode = doc.createTextNode(sl);
                latnode.appendChild ( vtnode );

                QDomElement lonnode = doc.createElement( "lon" );
                vnode.appendChild ( lonnode );

                float ll = *pf++;
                QString sll;
                sll.sprintf("%.5f", ll);
                QDomText vtlnode = doc.createTextNode(sll);
                lonnode.appendChild ( vtlnode );
            }
        }


        for(int i=0 ; i < cte.GetnAuxPlyEntries() ; i++)
        {
            QDomElement panelnode = doc.createElement( "panel" );
            node.appendChild ( panelnode );

            QString panel_no;
            panel_no.sprintf("%d", i+1);
            QDomText anode = doc.createTextNode(panel_no );
            panelnode.appendChild ( anode );

            float *pf = cte.GetpAuxPlyTableEntry(i);
            for(int j=0 ; j < cte.GetAuxCntTableEntry(i) ; j++)
            {
                QDomElement vnode = doc.createElement("vertex" );
                panelnode.appendChild ( vnode );

                QDomElement latnode = doc.createElement("lat" );
                vnode.appendChild ( latnode );

                float l = *pf++;
                QString sl;
                sl.sprintf("%.5f", l);
                QDomText vtnode = doc.createTextNode(sl);
                latnode.appendChild ( vtnode );

                QDomElement lonnode = doc.createElement( "lon" );
                vnode.appendChild ( lonnode );

                float ll = *pf++;
                QString sll;
                sll.sprintf("%.5f", ll);
                QDomText vtlnode = doc.createTextNode(sll);
                lonnode.appendChild ( vtlnode );
            }
        }

    }

//    doc.SetRoot(pcell_node);

    if(b_remove) DeleteCacheChart(pc);
#endif
    return doc;
}





//  Private version of PolyPt testing using floats instead of doubles

bool Intersect(MyFlPoint p1, MyFlPoint p2, MyFlPoint p3, MyFlPoint p4) ;
int CCW(MyFlPoint p0, MyFlPoint p1, MyFlPoint p2) ;

/*************************************************************************


 * FUNCTION:   G_FloatPtInPolygon
 *
 * PURPOSE
 * This routine determines if the point passed is in the polygon. It uses

 * the classical polygon hit-testing algorithm: a horizontal ray starting

 * at the point is extended infinitely rightwards and the number of
 * polygon edges that intersect the ray are counted. If the number is odd,
 * the point is inside the polygon.
 *
 * RETURN VALUE
 * (bool) true if the point is inside the polygon, false if not.
 *************************************************************************/


bool G_FloatPtInPolygon(MyFlPoint *rgpts, int wnumpts, float x, float y)

{

    MyFlPoint  *ppt, *ppt1 ;
    int   i ;
    MyFlPoint  pt1, pt2, pt0 ;
    int   wnumintsct = 0 ;

    pt0.x = x;
    pt0.y = y;

    pt1 = pt2 = pt0 ;
    pt2.x = 1.e6;

    // Now go through each of the lines in the polygon and see if it
    // intersects
    for (i = 0, ppt = rgpts ; i < wnumpts-1 ; i++, ppt++)
    {
        ppt1 = ppt;
        ppt1++;
        if (Intersect(pt0, pt2, *ppt, *(ppt1)))
            wnumintsct++ ;
    }

    // And the last line
    if (Intersect(pt0, pt2, *ppt, *rgpts))
        wnumintsct++ ;

    //   return(wnumintsct&1);

    //       If result is false, check the degenerate case where test point lies on a polygon endpoint
    if(!(wnumintsct&1))
    {
        for (i = 0, ppt = rgpts ; i < wnumpts ; i++, ppt++)
        {
            if(((*ppt).x == x) && ((*ppt).y == y))
                return true;
        }
    }
    else
        return true;

    return false;
}


/*************************************************************************


 * FUNCTION:   Intersect
 *
 * PURPOSE
 * Given two line segments, determine if they intersect.
 *
 * RETURN VALUE
 * true if they intersect, false if not.
 *************************************************************************/


inline bool Intersect(MyFlPoint p1, MyFlPoint p2, MyFlPoint p3, MyFlPoint p4) {
    return ((( CCW(p1, p2, p3) * CCW(p1, p2, p4)) <= 0)
            && (( CCW(p3, p4, p1) * CCW(p3, p4, p2)  <= 0) )) ;

}
/*************************************************************************


 * FUNCTION:   CCW (CounterClockWise)
 *
 * PURPOSE
 * Determines, given three points, if when travelling from the first to
 * the second to the third, we travel in a counterclockwise direction.
 *
 * RETURN VALUE
 * (int) 1 if the movement is in a counterclockwise direction, -1 if
 * not.
 *************************************************************************/


inline int CCW(MyFlPoint p0, MyFlPoint p1, MyFlPoint p2) {
    float dx1, dx2 ;
    float dy1, dy2 ;

    dx1 = p1.x - p0.x ; dx2 = p2.x - p0.x ;
    dy1 = p1.y - p0.y ; dy2 = p2.y - p0.y ;

    /* This is basically a slope comparison: we don't do divisions because

    * of divide by zero possibilities with pure horizontal and pure
    * vertical lines.
    */
    return ((dx1 * dy2 > dy1 * dx2) ? 1 : -1) ;

}

