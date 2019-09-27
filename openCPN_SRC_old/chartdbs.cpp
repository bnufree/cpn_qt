/**************************************************************************
*
* Project:  ChartManager
* Purpose:  Basic Chart Info Storage
* Author:   David Register, Mark A Sikes
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

#include "chartdbs.h"
#include "chartbase.h"
//#include "pluginmanager.h"
#include "mbtiles.h"
#include "mygeom.h"                     // For DouglasPeucker();
#include "FlexHash.h"

#include "OCPNPlatform.h"
#include <QProgressDialog>
#ifndef UINT32
#define UINT32 unsigned int
#endif

#include <QDir>
#include <QRegExp>

//extern PlugInManager    *g_pi_manager;
extern QString         gWorldMapLocation;

static int s_dbVersion;                         //    Database version currently in use at runtime
//  Needed for ChartTableEntry::GetChartType() only
//  TODO This can go away at opencpn Version 1.3.8 and above....
///////////////////////////////////////////////////////////////////////

bool FindMatchingFile(const QString &theDir, const char *theRegEx, int nameLength, QString &theMatch) {
    QDir dir(theDir);
    QStringList list = dir.entryList();
    QRegExp rePattern(theRegEx);
    foreach (QString file, list)
    {
        if (file.length() == (unsigned int)nameLength && rePattern.exactMatch(file))
        {
            return true;
        }
    }
    return false;
}


static ChartFamilyEnum GetChartFamily(int charttype)
{
    ChartFamilyEnum cf;

    switch( charttype)
    {
    case        CHART_TYPE_KAP:      cf = CHART_FAMILY_RASTER; break;
    case        CHART_TYPE_GEO:      cf = CHART_FAMILY_RASTER; break;
    case        CHART_TYPE_S57:      cf = CHART_FAMILY_VECTOR; break;
    case        CHART_TYPE_CM93:     cf = CHART_FAMILY_VECTOR; break;
    case        CHART_TYPE_CM93COMP: cf = CHART_FAMILY_VECTOR; break;
    case        CHART_TYPE_DUMMY:    cf = CHART_FAMILY_RASTER; break;
    case        CHART_TYPE_UNKNOWN:  cf = CHART_FAMILY_UNKNOWN; break;
    default:                         cf = CHART_FAMILY_UNKNOWN; break;
    }
    return cf;
}


///////////////////////////////////////////////////////////////////////
// ChartTableHeader
///////////////////////////////////////////////////////////////////////

void ChartTableHeader::Read(FileReadWrite& is)
{
    is.Read(this, sizeof(ChartTableHeader));
}

void ChartTableHeader::Write(FileReadWrite &os)
{
    char vb[5];
    sprintf(vb, "V%03d", DB_VERSION_CURRENT);

    memcpy(dbVersion, vb, 4);
    os.Write(this, sizeof(ChartTableHeader));
}

bool ChartTableHeader::CheckValid()
{
    char vb[5];
    sprintf(vb, "V%03d", DB_VERSION_CURRENT);
    if (strncmp(vb, dbVersion, sizeof(dbVersion)))
    {
        char vbo[5];
        memcpy(vbo, dbVersion, 4);
        vbo[4] = 0;
        qDebug("   Warning: found incorrect chart db version: %s", vbo);

        // Try previous version....
        sprintf(vb, "V%03d", DB_VERSION_PREVIOUS);
        if (strncmp(vb, dbVersion, sizeof(dbVersion)))
        {
            return false;
        } else
        {
            qDebug("   Scheduling db upgrade to current db version on Options->Charts page visit...");
            return true;
        }


    }
    else
    {
        char vbo[5];
        memcpy(vbo, dbVersion, 4);
        vbo[4] = 0;
        qDebug("Loading chart db version: %s", vbo);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////
// ChartTableEntry
///////////////////////////////////////////////////////////////////////

void ChartTableEntry::SetScale( int scale )
{
    Scale = scale;
    rounding = 0;
    // XXX find the right rounding
    if (Scale >= 1000)
        rounding = 5 *pow(10, log10(Scale) -2);
}

ChartTableEntry::ChartTableEntry(ChartBase &theChart)
{
    Clear();

    char *pt = (char *)malloc(strlen(theChart.GetFullPath().toUtf8().data()) + 1);
    strcpy(pt, theChart.GetFullPath().toUtf8().data());
    pFullPath = pt;


    SetScale(theChart.GetNativeScale());

    ChartType = theChart.GetChartType();
    ChartFamily = theChart.GetChartFamily();

    Skew = theChart.GetChartSkew();
    ProjectionType = theChart.GetChartProjectionType();

    QDateTime ed = theChart.GetEditionDate();
    if(theChart.GetEditionDate().isValid())
        edition_date = theChart.GetEditionDate().toTime_t();

    QFileInfo fn(theChart.GetFullPath());
    if(fn.lastModified().isValid())
        file_date = fn.lastModified().toTime_t();

    m_pfilename = new QString;           // create and populate helper members
    *m_pfilename = fn.fileName();
    m_psFullPath = new QString;
    *m_psFullPath = theChart.GetFullPath();
    
    Extent ext;
    theChart.GetChartExtent(&ext);
    LatMax = ext.NLAT;
    LatMin = ext.SLAT;
    LonMin = ext.WLON;
    LonMax = ext.ELON;

    m_bbox.Set(LatMin, LonMin, LatMax, LonMax);

    // Fill in the PLY information
    //  LOD calculation
    int LOD_pixels = 1;
    double scale_max_zoom = Scale / 4;
    
    double display_ppm = 1 / .00025;     // nominal for most LCD displays
    double meters_per_pixel_max_scale = scale_max_zoom / display_ppm;
    double LOD_meters = meters_per_pixel_max_scale * LOD_pixels;
    
    //    double LOD_meters = 5;
    
    // If  COVR table has only one entry, us it for the primary Ply Table
    if (theChart.GetCOVREntries() == 1) {
        nPlyEntries = theChart.GetCOVRTablePoints(0);

        if(nPlyEntries > 5 && (LOD_meters > .01)){
            std::vector<int> index_keep{0, nPlyEntries-1, 1, nPlyEntries-2};

            double *DPbuffer = (double *)malloc(2 * nPlyEntries * sizeof(double));

            double *pfed = DPbuffer;
            Plypoint *ppp = (Plypoint *)theChart.GetCOVRTableHead(0);

            for (int i = 0; i < nPlyEntries; i++) {
                *pfed++ = ppp->ltp;
                *pfed++ = ppp->lnp;
                ppp++;
            }

            DouglasPeucker(DPbuffer, 1, nPlyEntries-2, LOD_meters/(1852 * 60), &index_keep);
            //              printf("DB DP Reduction: %d/%d\n", index_keep.size(), nPlyEntries);

            // Mark the keepers by adding a simple constant to ltp
            for(unsigned int i=0 ; i < index_keep.size() ; i++){
                DPbuffer[2*index_keep[i]] += 2000.;
            }


            float *pf = (float *)malloc(2 * index_keep.size() * sizeof(float));
            float *pfe = pf;

            for (int i = 0; i < nPlyEntries; i++) {
                if(DPbuffer[2 * i] > 1000.){
                    *pfe++ = DPbuffer[2*i] - 2000.;
                    *pfe++ = DPbuffer[(2*i) + 1];
                }
            }

            pPlyTable = pf;
            nPlyEntries = index_keep.size();
            free( DPbuffer );
        }
        else {
            float *pf = (float *)malloc(2 * nPlyEntries * sizeof(float));
            pPlyTable = pf;
            float *pfe = pf;
            Plypoint *ppp = (Plypoint *)theChart.GetCOVRTableHead(0);

            for (int i = 0; i < nPlyEntries; i++) {
                *pfe++ = ppp->ltp;
                *pfe++ = ppp->lnp;
                ppp++;
            }
        }
    }
    // Else create a rectangular primary Ply Table from the chart extents
    // and create AuxPly table from the COVR tables
    else {
        // Create new artificial Ply table from chart extents
        nPlyEntries = 4;
        float *pf1 = (float *)malloc(2 * 4 * sizeof(float));
        pPlyTable = pf1;
        float *pfe = pf1;
        Extent fext;
        theChart.GetChartExtent(&fext);

        *pfe++ = fext.NLAT; //LatMax;
        *pfe++ = fext.WLON; //LonMin;

        *pfe++ = fext.NLAT; //LatMax;
        *pfe++ = fext.ELON; //LonMax;

        *pfe++ = fext.SLAT; //LatMin;
        *pfe++ = fext.ELON; //LonMax;

        *pfe++ = fext.SLAT; //LatMin;
        *pfe++ = fext.WLON; //LonMin;

        // Fill in the structure for pAuxPlyTable

        nAuxPlyEntries = theChart.GetCOVREntries();
        Q_ASSERT(nAuxPlyEntries);
        float **pfp = (float **)malloc(nAuxPlyEntries * sizeof(float *));
        float **pft0 = pfp;
        int *pip = (int *)malloc(nAuxPlyEntries * sizeof(int));

        for (int j = 0 ; j < nAuxPlyEntries; j++) {
            int nPE = theChart.GetCOVRTablePoints(j);

            if(nPE > 5 && (LOD_meters > .01)){
                std::vector<int> index_keep{0,nPE-1, 1, nPE-2};

                double *DPbuffer = (double *)malloc(2 * nPE * sizeof(double));

                double *pfed = DPbuffer;
                Plypoint *ppp = (Plypoint *)theChart.GetCOVRTableHead(j);

                for (int i = 0; i < nPE; i++) {
                    *pfed++ = ppp->ltp;
                    *pfed++ = ppp->lnp;
                    ppp++;
                }

                DouglasPeucker(DPbuffer, 1, nPE-2, LOD_meters/(1852 * 60), &index_keep);
                //                 printf("DBa DP Reduction: %d/%d\n", index_keep.size(), nPE);

                // Mark the keepers by adding a simple constant to ltp
                for(unsigned int i=0 ; i < index_keep.size() ; i++){
                    DPbuffer[2*index_keep[i]] += 2000.;
                }


                float *pf = (float *)malloc(2 * index_keep.size() * sizeof(float));
                float *pfe = pf;

                for (int i = 0; i < nPE; i++) {
                    if(DPbuffer[2 * i] > 1000.){
                        *pfe++ = DPbuffer[2*i] - 2000.;
                        *pfe++ = DPbuffer[(2*i) + 1];
                    }
                }

                pft0[j] = pf;
                pip[j] = index_keep.size();
                free( DPbuffer );
            }
            else {
                float *pf_entry = (float *)malloc(theChart.GetCOVRTablePoints(j) * 2 * sizeof(float));
                memcpy(pf_entry, theChart.GetCOVRTableHead(j), theChart.GetCOVRTablePoints(j) * 2 * sizeof(float));
                pft0[j] = pf_entry;
                pip[j] = theChart.GetCOVRTablePoints(j);
            }
        }

        pAuxPlyTable = pfp;
        pAuxCntTable = pip;

    }
    
    //  Get and populate the NoCovr tables
    
    nNoCovrPlyEntries = theChart.GetNoCOVREntries();
    if (nNoCovrPlyEntries == 0)
        return;

    float **pfpnc = (float **)malloc(nNoCovrPlyEntries * sizeof(float *));
    float **pft0nc = pfpnc;
    int *pipnc = (int *)malloc(nNoCovrPlyEntries * sizeof(int));
    
    for (int j = 0 ; j < nNoCovrPlyEntries; j++) {
        float *pf_entry = (float *)malloc(theChart.GetNoCOVRTablePoints(j) * 2 * sizeof(float));
        memcpy(pf_entry, theChart.GetNoCOVRTableHead(j), theChart.GetNoCOVRTablePoints(j) * 2 * sizeof(float));
        pft0nc[j] = pf_entry;
        pipnc[j] = theChart.GetNoCOVRTablePoints(j);
    }

    pNoCovrPlyTable = pfpnc;
    pNoCovrCntTable = pipnc;
    
}

///////////////////////////////////////////////////////////////////////

ChartTableEntry::~ChartTableEntry()
{
    qDebug()<<"delete chart entry now!!!!";
    free(pFullPath);
    free(pPlyTable);
    
    for (int i = 0; i < nAuxPlyEntries; i++)
        free(pAuxPlyTable[i]);
    free(pAuxPlyTable);
    free(pAuxCntTable);

    if (nNoCovrPlyEntries) {
        for (int i = 0; i < nNoCovrPlyEntries; i++)
            free( pNoCovrPlyTable[i] );
        free( pNoCovrPlyTable );
        free( pNoCovrCntTable );
    }
    
    delete m_pfilename;
    delete m_psFullPath;
}

///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////

bool ChartTableEntry::IsEarlierThan(const ChartTableEntry *cte) const {
    if(!cte) return true;
    QDateTime mine = QDateTime::fromTime_t(edition_date);
    QDateTime theirs = QDateTime::fromTime_t(cte->edition_date);
    return ( mine < theirs );
}

bool ChartTableEntry::IsEqualTo(const ChartTableEntry *cte) const {
    if(!cte) return true;
    QDateTime mine = QDateTime::fromTime_t(edition_date);
    QDateTime theirs = QDateTime::fromTime_t(cte->edition_date);
    return ( mine == theirs );
}

///////////////////////////////////////////////////////////////////////

static int convertChartType(int charttype)
{
    //    Hackeroo here....
    //    dB version 14 had different ChartType Enum, patch it here
    if(s_dbVersion == 14)
    {
        switch(charttype)
        {
        case 0: return CHART_TYPE_KAP;
        case 1: return CHART_TYPE_GEO;
        case 2: return CHART_TYPE_S57;
        case 3: return CHART_TYPE_CM93;
        case 4: return CHART_TYPE_CM93COMP;
        case 5: return CHART_TYPE_UNKNOWN;
        case 6: return CHART_TYPE_DONTCARE;
        case 7: return CHART_TYPE_DUMMY;
        default: return CHART_TYPE_UNKNOWN;
        }
    }
    return charttype;
}

static int convertChartFamily(int charttype, int chartfamily)
{
    if(s_dbVersion < 18)
    {
        switch(charttype)
        {
        case CHART_TYPE_KAP:
        case CHART_TYPE_GEO:
            return CHART_FAMILY_RASTER;

        case CHART_TYPE_S57:
        case CHART_TYPE_CM93:
        case CHART_TYPE_CM93COMP:
            return CHART_FAMILY_VECTOR;

        default:
            return CHART_FAMILY_UNKNOWN;
        }
    }
    return chartfamily;
}


bool ChartTableEntry::Read(const ChartDatabase *pDb, FileReadWrite &is)
{
    char path[4096], *cp;

    Clear();

    //      Allow reading of current db format, and maybe others
    ChartDatabase *pD = (ChartDatabase *)pDb;
    int db_version = pD->GetVersion();

    if(db_version == 18)
    {
        // Read the path first
        for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
        pFullPath = (char *)malloc(cp - path + 1);
        strncpy(pFullPath, path, cp - path + 1);
//        qDebug("  Chart %s", pFullPath);
        
        //  Create and populate the helper members
        m_pfilename = new QString;
        QString fullfilename = QString::fromUtf8(pFullPath);
        QFileInfo fn(fullfilename);
        *m_pfilename = fn.fileName();
        m_psFullPath = new QString;
        *m_psFullPath = fullfilename;
        
        // Read the table entry
        ChartTableEntry_onDisk_18 cte;
        is.Read(&cte, sizeof(ChartTableEntry_onDisk_18));
        
        //    Transcribe the elements....
        EntryOffset = cte.EntryOffset;
        ChartType = cte.ChartType;
        ChartFamily = cte.ChartFamily;
        LatMax = cte.LatMax;
        LatMin = cte.LatMin;
        LonMax = cte.LonMax;
        LonMin = cte.LonMin;

        m_bbox.Set(LatMin, LonMin, LatMax, LonMax);
        
        Skew = cte.skew;
        ProjectionType = cte.ProjectionType;
        
        SetScale(cte.Scale);
        edition_date = cte.edition_date;
        file_date = cte.file_date;
        
        nPlyEntries = cte.nPlyEntries;
        nAuxPlyEntries = cte.nAuxPlyEntries;
        
        nNoCovrPlyEntries = cte.nNoCovrPlyEntries;
        
        bValid = cte.bValid;
        
        if (nPlyEntries) {
            int npeSize = nPlyEntries * 2 * sizeof(float);
            pPlyTable = (float *)malloc(npeSize);
            is.Read(pPlyTable, npeSize);
        }
        
        if (nAuxPlyEntries) {
            int napeSize = nAuxPlyEntries * sizeof(int);
            pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
            pAuxCntTable = (int *)malloc(napeSize);
            is.Read(pAuxCntTable, napeSize);
            
            for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
            }
        }
        
        if (nNoCovrPlyEntries) {
            int napeSize = nNoCovrPlyEntries * sizeof(int);
            pNoCovrCntTable = (int *)malloc(napeSize);
            is.Read(pNoCovrCntTable, napeSize);
            
            pNoCovrPlyTable = (float **)malloc(nNoCovrPlyEntries * sizeof(float *));
            for (int i = 0; i < nNoCovrPlyEntries; i++) {
                int nfSize = pNoCovrCntTable[i] * 2 * sizeof(float);
                pNoCovrPlyTable[i] = (float *)malloc(nfSize);
                is.Read(pNoCovrPlyTable[i], nfSize);
            }
        }
    }
    
    else if(db_version == 17)
    {
        // Read the path first
        for (cp = path; (*cp = (char)(is.GetC())) != 0; cp++);
        pFullPath = (char *)malloc(cp - path + 1);
        strncpy(pFullPath, path, cp - path + 1);
        qDebug("  Chart %s", pFullPath);
        
        //  Create and populate the helper members
        m_pfilename = new QString;
        QString fullfilename = QString::fromUtf8(pFullPath);
        QFileInfo fn(fullfilename);
        *m_pfilename = fn.fileName();
        m_psFullPath = new QString;
        *m_psFullPath = fullfilename;
        
        // Read the table entry
        ChartTableEntry_onDisk_17 cte;
        is.Read(&cte, sizeof(ChartTableEntry_onDisk_17));
        
        //    Transcribe the elements....
        EntryOffset = cte.EntryOffset;
        ChartType = cte.ChartType;
        LatMax = cte.LatMax;
        LatMin = cte.LatMin;
        LonMax = cte.LonMax;
        LonMin = cte.LonMin;

        m_bbox.Set(LatMin, LatMax, LonMin, LonMax);
        
        Skew = cte.skew;
        ProjectionType = cte.ProjectionType;
        
        SetScale(cte.Scale);
        edition_date = cte.edition_date;
        file_date = cte.file_date;
        
        nPlyEntries = cte.nPlyEntries;
        nAuxPlyEntries = cte.nAuxPlyEntries;
        
        nNoCovrPlyEntries = cte.nNoCovrPlyEntries;
        
        bValid = cte.bValid;
        
        if (nPlyEntries) {
            int npeSize = nPlyEntries * 2 * sizeof(float);
            pPlyTable = (float *)malloc(npeSize);
            is.Read(pPlyTable, npeSize);
        }

        if (nAuxPlyEntries) {
            int napeSize = nAuxPlyEntries * sizeof(int);
            pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
            pAuxCntTable = (int *)malloc(napeSize);
            is.Read(pAuxCntTable, napeSize);
            
            for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
            }
        }

        if (nNoCovrPlyEntries) {
            int napeSize = nNoCovrPlyEntries * sizeof(int);
            pNoCovrCntTable = (int *)malloc(napeSize);
            is.Read(pNoCovrCntTable, napeSize);
            
            pNoCovrPlyTable = (float **)malloc(nNoCovrPlyEntries * sizeof(float *));
            for (int i = 0; i < nNoCovrPlyEntries; i++) {
                int nfSize = pNoCovrCntTable[i] * 2 * sizeof(float);
                pNoCovrPlyTable[i] = (float *)malloc(nfSize);
                is.Read(pNoCovrPlyTable[i], nfSize);
            }
        }
    }
    
    else if(db_version == 16)
    {
        // Read the path first
        for (cp = path; (*cp = (char)(is.GetC())) != 0; cp++);
        // TODO: optimize prepended dir
        pFullPath = (char *)malloc(cp - path + 1);
        strncpy(pFullPath, path, cp - path + 1);
        qDebug("  Chart %s", pFullPath);

        //  Create and populate the helper members
        m_pfilename = new QString;
        QString fullfilename = QString::fromUtf8(pFullPath);
        QFileInfo fn(fullfilename);
        *m_pfilename = fn.fileName();
        m_psFullPath = new QString;
        *m_psFullPath = fullfilename;

        // Read the table entry
        ChartTableEntry_onDisk_16 cte;
        is.Read(&cte, sizeof(ChartTableEntry_onDisk_16));

        //    Transcribe the elements....
        EntryOffset = cte.EntryOffset;
        ChartType = cte.ChartType;
        LatMax = cte.LatMax;
        LatMin = cte.LatMin;
        LonMax = cte.LonMax;
        LonMin = cte.LonMin;

        m_bbox.Set(LatMin, LatMax, LonMin, LonMax);

        Skew = cte.skew;
        ProjectionType = cte.ProjectionType;

        SetScale(cte.Scale);
        edition_date = cte.edition_date;
        file_date = cte.file_date;

        nPlyEntries = cte.nPlyEntries;
        nAuxPlyEntries = cte.nAuxPlyEntries;

        bValid = cte.bValid;

        if (nPlyEntries) {
            int npeSize = nPlyEntries * 2 * sizeof(float);
            pPlyTable = (float *)malloc(npeSize);
            is.Read(pPlyTable, npeSize);
        }

        if (nAuxPlyEntries) {
            int napeSize = nAuxPlyEntries * sizeof(int);
            pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
            pAuxCntTable = (int *)malloc(napeSize);
            is.Read(pAuxCntTable, napeSize);

            for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
            }
        }
    }
    
    else if(db_version == 15)
    {
        // Read the path first
        for (cp = path; (*cp = (char)is.GetC()) != 0; cp++);
        // TODO: optimize prepended dir
        pFullPath = (char *)malloc(cp - path + 1);
        strncpy(pFullPath, path, cp - path + 1);
        qDebug("  Chart %s", pFullPath);

        // Read the table entry
        ChartTableEntry_onDisk_15 cte;
        is.Read(&cte, sizeof(ChartTableEntry_onDisk_15));

        //    Transcribe the elements....
        EntryOffset = cte.EntryOffset;
        ChartType = cte.ChartType;
        LatMax = cte.LatMax;
        LatMin = cte.LatMin;
        LonMax = cte.LonMax;
        LonMin = cte.LonMin;

        m_bbox.Set(LatMin, LatMax, LonMin, LonMax);

        SetScale(cte.Scale);
        edition_date = cte.edition_date;
        file_date = cte.file_date;

        nPlyEntries = cte.nPlyEntries;
        nAuxPlyEntries = cte.nAuxPlyEntries;

        bValid = cte.bValid;

        if (nPlyEntries) {
            int npeSize = nPlyEntries * 2 * sizeof(float);
            pPlyTable = (float *)malloc(npeSize);
            is.Read(pPlyTable, npeSize);
        }

        if (nAuxPlyEntries) {
            int napeSize = nAuxPlyEntries * sizeof(int);
            pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
            pAuxCntTable = (int *)malloc(napeSize);
            is.Read(pAuxCntTable, napeSize);

            for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
            }
        }
    }
    else if(db_version == 14)
    {
        // Read the path first
        for (cp = path; (*cp = (char)(is.GetC())) != 0; cp++);
        pFullPath = (char *)malloc(cp - path + 1);
        strncpy(pFullPath, path, cp - path + 1);
        qDebug("  Chart %s", pFullPath);

        // Read the table entry
        ChartTableEntry_onDisk_14 cte;
        is.Read(&cte, sizeof(ChartTableEntry_onDisk_14));

        //    Transcribe the elements....
        EntryOffset = cte.EntryOffset;
        ChartType = cte.ChartType;
        LatMax = cte.LatMax;
        LatMin = cte.LatMin;
        LonMax = cte.LonMax;
        LonMin = cte.LonMin;

        m_bbox.Set(LatMin, LatMax, LonMin, LonMax);

        SetScale(cte.Scale);
        edition_date = cte.edition_date;
        file_date = 0;                        //  file_date does not exist in V14;
        nPlyEntries = cte.nPlyEntries;
        nAuxPlyEntries = cte.nAuxPlyEntries;
        bValid = cte.bValid;

        if (nPlyEntries) {
            int npeSize = nPlyEntries * 2 * sizeof(float);
            pPlyTable = (float *)malloc(npeSize);
            is.Read(pPlyTable, npeSize);
        }

        if (nAuxPlyEntries) {
            int napeSize = nAuxPlyEntries * sizeof(int);
            pAuxPlyTable = (float **)malloc(nAuxPlyEntries * sizeof(float *));
            pAuxCntTable = (int *)malloc(napeSize);
            is.Read(pAuxCntTable, napeSize);

            for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
                int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
                pAuxPlyTable[nAuxPlyEntry] = (float *)malloc(nfSize);
                is.Read(pAuxPlyTable[nAuxPlyEntry], nfSize);
            }
        }
    }
    ChartFamily = convertChartFamily(ChartType, ChartFamily);
    ChartType = convertChartType(ChartType);

    return true;
}

///////////////////////////////////////////////////////////////////////

bool ChartTableEntry::Write(const ChartDatabase *pDb, FileReadWrite &os)
{
    os.Write(pFullPath, strlen(pFullPath) + 1);

    //      Write the current version type only
    //      Create an on_disk table entry
    ChartTableEntry_onDisk_18 cte;

    //    Transcribe the elements....
    cte.EntryOffset = EntryOffset;
    cte.ChartType = ChartType;
    cte.ChartFamily = ChartFamily;
    cte.LatMax = LatMax;
    cte.LatMin = LatMin;
    cte.LonMax = LonMax;
    cte.LonMin = LonMin;

    cte.Scale = Scale;
    cte.edition_date = edition_date;
    cte.file_date = file_date;

    cte.nPlyEntries = nPlyEntries;
    cte.nAuxPlyEntries = nAuxPlyEntries;

    cte.skew = Skew;
    cte.ProjectionType = ProjectionType;

    cte.bValid = bValid;

    cte.nNoCovrPlyEntries = nNoCovrPlyEntries;
    
    os.Write(&cte, sizeof(ChartTableEntry_onDisk_18));
    qDebug("  Wrote Chart %s", pFullPath);

    //      Write out the tables
    if (nPlyEntries) {
        int npeSize = nPlyEntries * 2 * sizeof(float);
        os.Write(pPlyTable, npeSize);
    }

    if (nAuxPlyEntries) {
        int napeSize = nAuxPlyEntries * sizeof(int);
        os.Write(pAuxCntTable, napeSize);

        for (int nAuxPlyEntry = 0; nAuxPlyEntry < nAuxPlyEntries; nAuxPlyEntry++) {
            int nfSize = pAuxCntTable[nAuxPlyEntry] * 2 * sizeof(float);
            os.Write(pAuxPlyTable[nAuxPlyEntry], nfSize);
        }
    }
    
    if (nNoCovrPlyEntries ) {
        int ncSize = nNoCovrPlyEntries * sizeof(int);
        os.Write(pNoCovrCntTable, ncSize);
        
        for (int i = 0; i < nNoCovrPlyEntries; i++) {
            int nctSize = pNoCovrCntTable[i] * 2 * sizeof(float);
            os.Write(pNoCovrPlyTable[i], nctSize);
        }
    }
    

    return true;
}

///////////////////////////////////////////////////////////////////////

void ChartTableEntry::Clear()
{
    //    memset(this, 0, sizeof(ChartTableEntry));
    
    pFullPath = NULL;
    pPlyTable = NULL;
    pAuxPlyTable = NULL;
    pAuxCntTable = NULL;
    bValid = false;;
    pNoCovrCntTable = NULL;
    pNoCovrPlyTable = NULL;
    
    nNoCovrPlyEntries = 0;
    nAuxPlyEntries = 0;
    
    m_pfilename = NULL;             // a helper member, not on disk
    m_psFullPath = NULL;
    
}

///////////////////////////////////////////////////////////////////////

void ChartTableEntry::Disable()
{
    // Mark this chart in the database, so that it will not be seen during this run
    // How?  By setting the chart bounding box to an absurd value
    // TODO... Fix this heinous hack
    LatMax += (float) 1000.;
    LatMin += (float)1000.;
}

void ChartTableEntry::ReEnable()
{
    if(LatMax >90.){
        LatMax -= (float) 1000.;
        LatMin -= (float) 1000.;
    }
}

std::vector<float> ChartTableEntry::GetReducedPlyPoints()
{
    if(m_reducedPlyPoints.size())
        return m_reducedPlyPoints;
    
    //  Reduce the LOD of the chart outline PlyPoints
    float LOD_meters = 1;
    
    float plylat, plylon;
    const int nPoints = GetnPlyEntries();

    float *fpo = GetpPlyTable();

    double *ppd = new double[nPoints * 2];
    double *ppsm = new double[nPoints * 2];
    double *npr = ppd;
    double *npsm= ppsm;
    for( int i = 0; i < nPoints; i++ ) {
        plylat = fpo[i*2];
        plylon = fpo[i*2+1];

        double x, y;
        toSM(plylat, plylon, fpo[0], fpo[1], &x, &y);

        *npr++ = plylon;
        *npr++ = plylat;
        *npsm++ = x;
        *npsm++ = y;
    }

    std::vector<int> index_keep;
    if(nPoints > 10){
        index_keep.push_back(0);
        index_keep.push_back(nPoints-1);
        index_keep.push_back(1);
        index_keep.push_back(nPoints-2);


        DouglasPeuckerM(ppsm, 1, nPoints-2, LOD_meters , &index_keep);

    }
    else {
        index_keep.resize(nPoints);
        for(int i = 0 ; i < nPoints ; i++)
            index_keep[i] = i;
    }

    double *ppr = ppd;
    for(int ip = 0 ; ip < nPoints ; ip++){
        double x = *ppr++;
        double y = *ppr++;

        for(unsigned int j=0 ; j < index_keep.size() ; j++){
            if(index_keep[j] == ip){
                m_reducedPlyPoints.push_back(x);
                m_reducedPlyPoints.push_back(y);
                break;
            }
        }
    }
    
    delete[] ppd;
    delete[] ppsm;
    
    int nprr = m_reducedPlyPoints.size() / 2;

    return m_reducedPlyPoints;

}

std::vector<float> ChartTableEntry::GetReducedAuxPlyPoints( int iTable)
{
    //  Maybe need to initialize the vector
    if( !m_reducedAuxPlyPointsVector.size()){
        std::vector<float> vec;
        for(int i=0 ; i < GetnAuxPlyEntries() ; i++){
            m_reducedAuxPlyPointsVector.push_back(vec);
        }
    }
    
    std::vector<float> vec;

    //  Invalid parameter
    if((unsigned int)iTable >= m_reducedAuxPlyPointsVector.size())
        return vec;
    
    if( m_reducedAuxPlyPointsVector.at(iTable).size())
        return m_reducedAuxPlyPointsVector.at(iTable);
    
    //  Reduce the LOD of the chart outline PlyPoints
    float LOD_meters = 1.0;
    
    const int nPoints = GetAuxCntTableEntry(iTable);
    float *fpo = GetpAuxPlyTableEntry(iTable);

    double *ppd = new double[nPoints * 2];
    double *ppsm = new double[nPoints * 2];
    double *npr = ppd;
    double *npsm= ppsm;
    float plylat, plylon;
    
    for( int i = 0; i < nPoints; i++ ) {
        plylat = fpo[i*2];
        plylon = fpo[i*2+1];

        double x, y;
        toSM(plylat, plylon, fpo[0], fpo[1], &x, &y);

        *npr++ = plylon;
        *npr++ = plylat;
        *npsm++ = x;
        *npsm++ = y;
    }


    std::vector<int> index_keep;
    if(nPoints > 10 ){
        index_keep.push_back(0);
        index_keep.push_back(nPoints-1);
        index_keep.push_back(1);
        index_keep.push_back(nPoints-2);

        DouglasPeuckerM(ppsm, 1, nPoints - 2, LOD_meters, &index_keep);

    }
    else {
        index_keep.resize(nPoints);
        for(int i = 0 ; i < nPoints ; i++)
            index_keep[i] = i;
    }

    int nnn = index_keep.size();

    double *ppr = ppd;
    for(int ip = 0 ; ip < nPoints ; ip++){
        double x = *ppr++;
        double y = *ppr++;

        for(unsigned int j=0 ; j < index_keep.size() ; j++){
            if(index_keep[j] == ip){
                vec.push_back(x);
                vec.push_back(y);
                break;
            }
        }
    }
    
    delete[] ppd;
    delete[] ppsm;

    m_reducedAuxPlyPointsVector[iTable] = vec;
    
    int nprr = vec.size() / 2;
    
    return vec;

}

///////////////////////////////////////////////////////////////////////
// ChartDatabase
///////////////////////////////////////////////////////////////////////
ChartDatabase::ChartDatabase()
{
    bValid = false;
    m_ChartTableEntryDummy.Clear();

    UpdateChartClassDescriptorArray();
}

void ChartDatabase::UpdateChartClassDescriptorArray(void)
{
    m_ChartClassDescriptorArray.clear();

    //    Create and add the descriptors for the default chart types recognized
    ChartClassDescriptor *pcd;

    pcd = new ChartClassDescriptor("ChartKAP", "*.kap", BUILTIN_DESCRIPTOR);
    m_ChartClassDescriptorArray.append(pcd);
    pcd = new ChartClassDescriptor("ChartGEO", "*.geo", BUILTIN_DESCRIPTOR);
    m_ChartClassDescriptorArray.append(pcd);
    pcd = new ChartClassDescriptor("s57chart", "*.000", BUILTIN_DESCRIPTOR);
    m_ChartClassDescriptorArray.append(pcd);
    pcd = new ChartClassDescriptor("s57chart", "*.s57", BUILTIN_DESCRIPTOR);
    m_ChartClassDescriptorArray.append(pcd);
    pcd = new ChartClassDescriptor("cm93compchart", "00300000.a", BUILTIN_DESCRIPTOR);
    m_ChartClassDescriptorArray.append(pcd);
    pcd = new ChartClassDescriptor("ChartMBTiles", "*.mbtiles", BUILTIN_DESCRIPTOR);
    m_ChartClassDescriptorArray.append(pcd);

    //      //    If the PlugIn Manager exists, get the array of dynamically loadable chart class names
    //      if(g_pi_manager)
    //      {
    //            wxArrayString array = g_pi_manager->GetPlugInChartClassNameArray();
    //            for(unsigned int j = 0 ; j < array.count() ; j++)
    //            {
    //                  //    Instantiate a blank chart to retrieve the directory search mask for this chart type
    //                  QString class_name = array[j];
    //                  ChartPlugInWrapper *cpiw = new ChartPlugInWrapper(class_name);
    //                  if(cpiw)
    //                  {
    //                        QString mask = cpiw->GetFileSearchMask();

    //            //    Create a new descriptor
    //                        ChartClassDescriptor *picd = new ChartClassDescriptor(class_name, mask, PLUGIN_DESCRIPTOR);

    //            //    Add descriptor to the database array member
    //                        m_ChartClassDescriptorArray.Add(picd);

    //                        delete cpiw;
    //                  }
    //            }
    //      }

}


const ChartTableEntry &ChartDatabase::GetChartTableEntry(int index) const
{
    if(index < m_nentries)
        return *(active_chartTable[index]);
    else
        return m_ChartTableEntryDummy;
}

ChartTableEntry *ChartDatabase::GetpChartTableEntry(int index) const
{
    if(index < m_nentries)
        return const_cast<ChartTableEntry*>(active_chartTable[index]);
    else
        return (ChartTableEntry *)&m_ChartTableEntryDummy;
}

bool ChartDatabase::CompareChartDirArray( ArrayOfCDI& test_array )
{
    //  Compare the parameter "test_array" with this.m_dir_array
    //    Return true if functionally identical (order does not signify).
    
    if(test_array.count() != m_dir_array.count())
        return false;
    
    bool bfound_inner;
    unsigned int nfound_outer = 0;
    
    for(unsigned int i = 0 ; i < test_array.count() ; i++){
        ChartDirInfo p = test_array[i];
        bfound_inner = false;
        for(unsigned int j = 0 ; j < m_dir_array.count() ; j++){
            ChartDirInfo q = m_dir_array[j];
            
            if(p.fullpath == q.fullpath){
                bfound_inner = true;
                break;
            }
        }
        if(bfound_inner)
            nfound_outer++;
    }
    
    return (nfound_outer == test_array.count());
    
}

QString ChartDatabase::GetMagicNumberCached(QString dir)
{
    for(unsigned int j = 0 ; j < m_dir_array.count() ; j++){
        ChartDirInfo q = m_dir_array[j];
        if(dir == q.fullpath)
            return q.magic_number;
    }
    
    return QString();

}



bool ChartDatabase::Read(const QString &filePath)
{
    int entries;

    bValid = false;

    QFile file(filePath);
    if (!file.exists()) return false;

    m_DBFileName = filePath;

    FileReadWrite ifs(filePath, FileReadWrite::E_READ);
    if(!ifs.IsOK()) return false;

    ChartTableHeader cth;
    cth.Read(ifs);
    if (!cth.CheckValid()) return false;

    //      Capture the version number
    char vbo[5];
    memcpy(vbo, cth.GetDBVersionString(), 4);
    vbo[4] = 0;
    m_dbversion = atoi(&vbo[1]);
    s_dbVersion = m_dbversion;                  // save the static copy

    qDebug("Chartdb:Reading %d directory entries, %d table entries", cth.GetDirEntries(), cth.GetTableEntries());
    qDebug("Chartdb: Chart directory list follows");
    if(0 == cth.GetDirEntries()) qDebug("  Nil");

    int ind = 0;
    for (int iDir = 0; iDir < cth.GetDirEntries(); iDir++) {
        QString dir;
        int dirlen;
        ifs.Read(&dirlen, sizeof(int));
        while (dirlen > 0) {
            char dirbuf[1024];
            int alen = dirlen > 1023 ? 1023 : dirlen;
            if(ifs.Read(&dirbuf, alen) == alen) goto read_error;
            dirbuf[alen] = 0;
            dirlen -= alen;
            dir.append(QString::fromUtf8(dirbuf));
        }
        qDebug("  Chart directory #%d: %s", iDir, dir.toUtf8().data());
        m_chartDirs.append(dir);
    }

    entries = cth.GetTableEntries();
    active_chartTable.reserve(entries);
    active_chartTable_pathindex.clear();
    while (entries-- /*&& entry.Read(this, ifs)*/) {
        ChartTableEntry *entry = new ChartTableEntry;
        if(!entry->Read(this, ifs))
        {
            delete entry;
            break;
        }
        entry->SetAvailable(true);
        active_chartTable_pathindex[QString::fromUtf8(entry->GetpFullPath())] = ind++;
        active_chartTable.append(entry);
    }
    bValid = true;
    m_nentries = active_chartTable.count();
    return true;

read_error:
    bValid = false;
    m_nentries = active_chartTable.count();
    return false;
}

///////////////////////////////////////////////////////////////////////

bool ChartDatabase::Write(const QString &filePath)
{
    QFileInfo file(filePath);
    QDir dir(file.absolutePath());

    if (!dir.exists() && !dir.mkpath(dir.path())) return false;

    FileReadWrite ofs(filePath, FileReadWrite::E_Write);
    if(!ofs.IsOK()) return false;

    ChartTableHeader cth(m_chartDirs.count(), active_chartTable.count());
    cth.Write(ofs);

    for (int iDir = 0; iDir < cth.GetDirEntries(); iDir++) {
        QString &dir = m_chartDirs[iDir];
        int dirlen = dir.length();
        char s[200];
        strncpy(s, dir.toUtf8().data(), 199);
        s[199] = 0;
        dirlen = strlen(s);
        ofs.Write(&dirlen, sizeof(int));
        ofs.Write(s, dirlen);
    }

    for (UINT32 iTable = 0; iTable < active_chartTable.size(); iTable++)
    {
        active_chartTable[iTable]->Write(this, ofs);
    }

    //      Explicitly set the version
    m_dbversion = DB_VERSION_CURRENT;
    return true;
}

///////////////////////////////////////////////////////////////////////
QString SplitPath(QString s, QString tkd, int nchar, int offset, int *pn_split)
{
    QString r;
    int ncr = 0;

    int rlen = offset;
    QStringList tkz = s.split(tkd);
    foreach (QString token, tkz) {
        if((rlen + (int)token.length() + 1) < nchar)
        {
            r += token;
            r += tkd[0];
            rlen += token.length() + 1;
        }
        else
        {
            r += ("\n");
            ncr ++;
            for(int i=0 ; i< offset ; i++){ r += (" "); }
            r += token;
            r += tkd[0];
            rlen = offset + token.length() + 1;
        }
    }

    if(pn_split) *pn_split = ncr;

    return r.mid(0, r.length()-1);             // strip the last separator char

}


QString ChartDatabase::GetFullChartInfo(ChartBase *pc, int dbIndex, int *char_width, int *line_count)
{
    QString r;
    int lc = 0;
    int max_width = 0;
    int ncr;
    int target_width = 60;

    const ChartTableEntry &cte = GetChartTableEntry(dbIndex);
    if(1)       //TODO why can't this be cte.GetbValid()?
    {
        QString line = (" ChartFile:  ");
        QString longline = QString::fromUtf8(cte.GetpFullPath());
        /*
            if(CHART_TYPE_CM93COMP == cte.GetChartType())
            {
                  //    Remove filename and two trailing directories
                  //    to make it pretty....
                  QString path;
                  const QString full_path(cte.GetpFullPath(), wxConvUTF8);
                  wxFileName::SplitPath(full_path, &path, NULL, NULL);

                  wxFileName fn(path);
                  fn.RemoveLastDir();
                  longline = fn.GetPath();
                  longline += _T("...");
            }
*/
        if(longline.length() > target_width)
        {
            line += SplitPath(longline, "/,\\", target_width, 15, &ncr);
            max_width = qMax(max_width, target_width+4);
            lc += ncr;
        }
        else
        {
            line += longline;
            max_width = qMax(max_width, line.length()+4);
        }


        r += line;
        r += ("\n");
        lc++;

        line.clear();
        if(pc)
        {
            line = (" Name:  ");
            QString longline = pc->GetName();

            QString tkz;
            if(longline.indexOf(' ') != -1)     // assume a proper name
                tkz = (" ");
            else
                tkz = ("/,\\");                   // else a file name

            if(longline.length() > target_width)
            {
                line += SplitPath(pc->GetName(), tkz, target_width, 12, &ncr);
                max_width = qMax(max_width, target_width+4);
                lc += ncr;
            }
            else
            {
                line += longline;
                max_width = qMax(max_width, line.length()+4);
            }

        }

        line += ("\n");
        r += line;
        lc++;

        if(pc)      // chart is loaded and available
            line.sprintf(" %s:  1:%d", "Scale", pc->GetNativeScale() );
        else
            line.sprintf(" %s:  1:%d", "Scale", cte.GetScale() );

        line += ("\n");
        max_width = qMax(max_width, line.length());
        r += line;
        lc++;

        if(pc)
        {
            line.clear();
            line = (" ID:  ");
            line += pc->GetID();
            line += ("\n");
            max_width = qMax(max_width, line.length());
            r += line;
            lc++;

            line.clear();
            line = (" Depth Units:  ");
            line += pc->GetDepthUnits();
            line += ("\n");
            max_width = qMax(max_width, line.length());
            r += line;
            lc++;

            line.clear();
            line = (" Soundings:  ");
            line += pc->GetSoundingsDatum();
            line += ("\n");
            max_width = qMax(max_width, line.length());
            r += line;
            lc++;

            line.clear();
            line = (" Datum:  ");
            line += pc->GetDatumString();
            line += ("\n");
            max_width = qMax(max_width, line.length());
            r += line;
            lc++;
        }

        line = (" Projection:  ");
        if(PROJECTION_UNKNOWN == cte.GetChartProjectionType())
            line += ("Unknown");
        else if(PROJECTION_MERCATOR == cte.GetChartProjectionType())
            line += ("Mercator");
        else if(PROJECTION_TRANSVERSE_MERCATOR == cte.GetChartProjectionType())
            line += ("Transverse Mercator");
        else if(PROJECTION_POLYCONIC == cte.GetChartProjectionType())
            line += ("Polyconic");
        else if(PROJECTION_WEB_MERCATOR == cte.GetChartProjectionType())
            line += ("Web Mercator (EPSG:3857)");
        line += ("\n");
        max_width = qMax(max_width, line.length());
        r += line;
        lc++;

        line.clear();
        if(pc)
        {
            line = (" Source Edition:  ");
            line += pc->GetSE();
            line += ("\n");
            max_width = qMax(max_width, line.length());
            r += line;
            lc++;

            line.clear();
            line = (" Updated:  ");
            QDateTime ed = pc->GetEditionDate();
            line += ed.toString("yyyy-MM-dd");
            line += ("\n");
            max_width = qMax(max_width, line.length());
            r += line;
            lc++;
        }

        line.clear();
        if(pc && pc->GetExtraInfo().length())
        {
            line += pc->GetExtraInfo();
            line += ("\n");
            max_width = qMax(max_width, line.length());
            r += line;
            lc++;

        }
    }

    if(line_count) *line_count = lc;
    if(char_width) *char_width = max_width;

    return r;
}

// ----------------------------------------------------------------------------
// Create Chart Table Database by directory search
//    resulting in valid pChartTable in (this)
// ----------------------------------------------------------------------------
bool ChartDatabase::Create(ArrayOfCDI &dir_array, QProgressDialog *pprog)
{
    m_dir_array = dir_array;

    bValid = false;

    m_chartDirs.clear();
    active_chartTable.clear();
    active_chartTable_pathindex.clear();

    Update(dir_array, true, pprog);                   // force the update the reload everything

    bValid = true;

    //      Explicitly set the version
    m_dbversion = DB_VERSION_CURRENT;

    return true;
}





// ----------------------------------------------------------------------------
// Update existing ChartTable Database by directory search
//    resulting in valid pChartTable in (this)
// ----------------------------------------------------------------------------
bool ChartDatabase::Update(ArrayOfCDI& dir_array, bool bForce, QProgressDialog *pprog)
{
    qDebug("dir_array size: %d force:%d", dir_array.size(), bForce);
    m_dir_array = dir_array;

    bValid = false;               // database is not useable right now...

    //  Mark all charts provisionally invalid
    for(unsigned int i=0 ; i<active_chartTable.count() ; i++)
        active_chartTable[i]->SetValid(false);

    m_chartDirs.clear();

    if(bForce) active_chartTable.clear();

    bool lbForce = bForce;

    //    Do a dB Version upgrade if the current one is obsolete
    if(s_dbVersion != DB_VERSION_CURRENT)
    {

        active_chartTable.clear();
        lbForce = true;
        s_dbVersion = DB_VERSION_CURRENT;         // Update the static indicator
        m_dbversion = DB_VERSION_CURRENT;         // and the member

    }

    //  Get the new charts

    for(unsigned int j=0 ; j<dir_array.count() ; j++)
    {
        ChartDirInfo dir_info = dir_array[j];

        QString dir_magic;
        QStringList name_filter("poly-*-1.dat");
        QDir dir(dir_info.fullpath);
        QFileInfoList file_list = dir.entryInfoList(name_filter);
        if( file_list.size() > 0 ) {
            //If some polygons exist in the directory, set it as the one to use for GSHHG
            //TODO: We should probably compare the version and maybe resolutions available with what is currently used...
            gWorldMapLocation = dir_info.fullpath + zchxFuncUtil::separator();
        }
        TraverseDirAndAddCharts(dir_info, pprog, dir_magic, lbForce);

        //  Update the dir_list entry, even if the magic values are the same

        dir_info.magic_number = dir_magic;
        dir_array.replace(j, dir_info);
        m_chartDirs.append(dir_info.fullpath);
    }           //for


    for(unsigned int i=0 ; i<active_chartTable.count() ; i++)
    {
        if(!active_chartTable[i]->GetbValid())
        {
            active_chartTable.removeAt(i);
            i--;                 // entry is gone, recheck this index for next entry
        }
    }

    //    And once more, setting the Entry index field
    active_chartTable_pathindex.clear();
    for(unsigned int i=0 ; i<active_chartTable.count() ; i++) {
        active_chartTable_pathindex[QString::fromUtf8(active_chartTable[i]->GetpFullPath())] = i;
        active_chartTable[i]->SetEntryOffset( i );
    }

    m_nentries = active_chartTable.count();

    bValid = true;
    return true;
}

//-------------------------------------------------------------------
//    Find Chart dbIndex
//-------------------------------------------------------------------

int ChartDatabase::FinddbIndex(QString PathToFind)
{
#if 0
    //    Find the chart
    for(unsigned int i=0 ; i<active_chartTable.count() ; i++)
    {
        QString s = QString(active_chartTable[i].GetpFullPath(), wxConvUTF8);
        if(s.IsSameAs(PathToFind))
        {
            return i;
        }
    }
#else
    if(active_chartTable_pathindex.find(PathToFind) != active_chartTable_pathindex.end())
        return active_chartTable_pathindex[PathToFind];
#endif

    return -1;
}



//-------------------------------------------------------------------
//    Disable Chart
//-------------------------------------------------------------------

int ChartDatabase::DisableChart(QString& PathToDisable)
{
    int index = FinddbIndex(PathToDisable);
    if( index != -1 ) {
        ChartTableEntry *pentry = active_chartTable[index];
        pentry->Disable();
        return 1;
    }
    return 0;
}

// ----------------------------------------------------------------------------
//  Traverse the given directory looking for charts
//  If bupdate is true, also search the existing database for a name match.
//  If target chart is already in database, mark the entry valid and skip additional processing
// ----------------------------------------------------------------------------

int ChartDatabase::TraverseDirAndAddCharts(ChartDirInfo& dir_info, QProgressDialog *pprog, QString &dir_magic, bool bForce)
{
    //    Extract the true dir name and magic number from the compound string
    QString dir_path = dir_info.fullpath;
    QString old_magic = dir_info.magic_number;
    QString new_magic = old_magic;
    dir_magic = old_magic;              // provisionally the same

    int nAdd = 0;

    bool b_skipDetectDirChange = false;
    bool b_dirchange = false;

    // Does this directory actually exist?
    if(!QDir(dir_path).exists())  return 0;

    // Check to see if this is a cm93 directory root
    // If so, skip the DetectDirChange since it may be very slow
    // and give no information
    // Assume a change has happened, and process accordingly
    bool b_cm93 = Check_CM93_Structure(dir_path);
    if(b_cm93)
    {
        b_skipDetectDirChange = true;
        b_dirchange = true;
    }

    //    Quick scan the directory to see if it has changed
    //    If not, there is no need to scan again.....
    if(!b_skipDetectDirChange)
        b_dirchange = DetectDirChange(dir_path, old_magic, new_magic, pprog);

    if( !bForce && !b_dirchange)
    {
        qDebug("   No change detected on directory:%s ", dir_path.toLatin1().data());
        //    Traverse the database, and mark as valid all charts coming from this dir,
        //    or anywhere in its tree

        QFileInfo fn_dir(dir_path, ("stuff"));
        unsigned int dir_path_count = fn_dir.absolutePath().split(zchxFuncUtil::separator()).size();
        if(pprog) pprog->setWindowTitle("OpenCPN Chart Scan....");
        int nEntries = active_chartTable.count();

        for(int ic=0 ; ic<nEntries ; ic++)
        {

            QFileInfo fn(QString::fromUtf8(active_chartTable[ic]->GetpFullPath()));
            QString tmp_path = fn.absolutePath();
            while(tmp_path.split(zchxFuncUtil::separator()).size() >= dir_path_count)
            {
                if(tmp_path == dir_path)
                {
                    active_chartTable[ic]->SetValid(true);
                    //                             if(pprog)
                    //                                  pprog->Update((ic * 100) /nEntries, fn.GetFullPath());

                    break;
                }
                int last_index = tmp_path.lastIndexOf(zchxFuncUtil::separator());
                tmp_path.remove(last_index, tmp_path.length() - last_index);
            }
        }

        return 0;
    }

    //    There presumably was a change in the directory contents.  Return the new magic number
    dir_magic = new_magic;

    //    Look for all possible defined chart classes
    for(unsigned int i = 0 ; i < m_ChartClassDescriptorArray.count() ; i++)
    {
        nAdd += SearchDirAndAddCharts(dir_path, *(m_ChartClassDescriptorArray[i]), pprog);
    }

    return nAdd;
}

bool ChartDatabase::DetectDirChange(const QString & dir_path, const QString & magic, QString &new_magic, QProgressDialog *pprog)
{
    if(pprog) pprog->setWindowTitle("OpenCPN Directory Scan....");

    //    parse the magic number
    long long unsigned int nmagic = magic.toULongLong();
    quint64 nacc = 0;

    //    Get an arraystring of all files
    QDir dir(dir_path);
    QFileInfoList FileList = dir.entryInfoList(QDir::Files, QDir::Name);
    int n_files = FileList.size();

    FlexHash hash( sizeof(nacc) );
    hash.Reset();

    //Traverse the list of files, getting their interesting stuff to add to accumulator
    for(int ifile=0 ; ifile < n_files ; ifile++)
    {
        if(pprog && (ifile % (n_files / 60 + 1)) == 0)
        {
            pprog->setLabelText(dir_path);
            pprog->setValue(qMin((ifile * 100) /n_files, 100));
        }

        QFileInfo file = FileList[ifile];

        // NOTE. Do not ever try to optimize this code by combining `QString` calls.
        // Otherwise `fileNameUTF8` will point to a stale buffer overwritten by garbage.
        QString fileNameNative = file.filePath();
        QByteArray fileNameUTF8 = fileNameNative.toUtf8();
        hash.Update( fileNameUTF8.data(), fileNameUTF8.length() );

        //    File Size;
        qint64 size = file.size();
        //wxULongLong fileSize = ( ( size != wxInvalidSize ) ? size : 0 );
        hash.Update( &size, sizeof(size)  );

        //    Mod time, in ticks
        qint64 fileTime = file.lastModified().toTime_t();
        hash.Update( &fileTime, ( sizeof fileTime ) );

    }

    hash.Finish();
    hash.Receive( &nacc );

    //    Return the calculated magic number
//    new_magic.sprintf("%ul", nacc);
    new_magic.setNum(nacc);

    //    And do the test
    if(new_magic != magic)
        return true;
    else
        return false;
}




bool ChartDatabase::IsChartDirUsed(const QString &theDir)
{
    QString dir(theDir);
    if (dir.endsWith(zchxFuncUtil::separator())) dir.remove(dir.size()-1, 1);

    dir.append("*");
    for (UINT32 i = 0; i < active_chartTable.count(); i++) {
        if (QRegExp(dir).exactMatch(*(active_chartTable[i]->GetpsFullPath())))
            return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
// Validate a given directory as a cm93 root database
// If it appears to be a cm93 database, then return true
//-----------------------------------------------------------------------------
bool ChartDatabase::Check_CM93_Structure(QString dir_name)
{
    QString filespec;

    QRegExp test("[0-9]+");

    QDir dirt(dir_name);

    bool b_maybe_found_cm93 = false;
    QStringList fileList = dirt.entryList(QDir::Dirs);
    QString candidate;
    foreach (QString str, fileList) {
        if(test.exactMatch(str)&& (str.length() == 8))
        {
            candidate = str;
            b_maybe_found_cm93 = true;
            break;
        }
    }

    if(b_maybe_found_cm93)
    {
        QString dir_next = dir_name;
        dir_next += ("/");
        dir_next += candidate;
        QDir dir_n(dir_next);
        if(!dir_n.exists()) return false;

        QStringList fileList = dirt.entryList(QDir::Files);
        QString candidate_n;
        QRegExp test_n("^[A-Ga-g]");
        bool b_probably_found_cm93 = false;
        foreach (QString str, fileList) {
            if(test_n.exactMatch(str)&& (str.length() == 1))
            {
                candidate_n = str;
                b_probably_found_cm93 = true;
                break;
            }
        }

        if(b_probably_found_cm93)           // found a directory that looks
            //like {dir_name}/12345678/A
            //probably cm93
        {
            // make sure the dir exists
            QString dir_luk = dir_next;
            dir_luk += ("/");
            dir_luk += candidate_n;
            if(QFile::exists(dir_luk))
                return true;

        }
    }

    return false;
}


/*
//-----------------------------------------------------------------------------
// Validate a given directory as a cm93 root database
// If it appears to be a cm93 database, then return the name of an existing cell file
// File name will be unique with respect to member element m_cm93_filename_array
// If not cm93, return empty string
//-----------------------------------------------------------------------------
QString ChartDatabase::Get_CM93_FileName(QString dir_name)
{
      QString filespec;

      QRegularExpression test(_T("[0-9]+"));

      QDir dirt(dir_name);
      QString candidate;

      bool b_maybe_found_cm93 = false;
      bool b_cont = dirt.GetFirst(&candidate);

      while(b_cont)
      {
            if(test.Matches(candidate)&& (candidate.length() == 8))
            {
                  b_maybe_found_cm93 = true;
                  break;
            }

            b_cont = dirt.GetNext(&candidate);

      }

      if(b_maybe_found_cm93)
      {
            QString dir_next = dir_name;
            dir_next += _T("/");
            dir_next += candidate;
            if(QDir::Exists(dir_next))
            {
                  QDir dir_n(dir_next);
                  QString candidate_n;

                  QRegularExpression test_n(_T("^[A-Ga-g]"));
                  bool b_probably_found_cm93 = false;
                  bool b_cont_n = dir_n.GetFirst(&candidate_n);
                  while(b_cont_n)
                  {
                        if(test_n.Matches(candidate_n) && (candidate_n.length() == 1))
                        {
                              b_probably_found_cm93 = true;
                              break;
                        }
                        b_cont_n = dir_n.GetNext(&candidate_n);
                  }

                  if(b_probably_found_cm93)           // found a directory that looks like {dir_name}/12345678/A   probably cm93
                  {                                   // and we want to try and shorten the recursive search
                        // make sure the dir exists
                        QString dir_luk = dir_next;
                        dir_luk += _T("/");
                        dir_luk += candidate_n;
                        if(QDir::Exists(dir_luk))
                        {
                              QString msg(_T("Found probable CM93 database in "));
                              msg += dir_name;
                              ZCHX_LOGMSG(msg);

                              QString dir_name_plus = dir_luk;                 // be very specific about the dir_name,

                              QDir dir_get(dir_name_plus);
                              QString one_file;
                              dir_get.GetFirst(&one_file);

                              //    We must return a unique file name, i.e. one that has not bee seen
                              //    before in this invocation of chart dir scans.
                              bool find_unique = false;
                              while(!find_unique)
                              {
                                    find_unique = true;
                                    for(unsigned int ifile=0; ifile < m_cm93_filename_array.count(); ifile++)
                                    {
                                          if(m_cm93_filename_array[ifile] == one_file)
                                                find_unique = false;
                                    }
                                    if(!find_unique)
                                          dir_get.GetNext(&one_file);
                              }

                              m_cm93_filename_array.Add(one_file);

                              filespec = one_file;
                        }

                  }
            }
      }

      return filespec;
}
*/



// ----------------------------------------------------------------------------
// Populate Chart Table by directory search for specified file type
// If bupdate flag is true, search the Chart Table for matching chart.
//  if target chart is already in table, mark it valid and skip chart processing
// ----------------------------------------------------------------------------

#include <QHash>
typedef QHash<QString, int> ChartCollisionsHashMap;

int ChartDatabase::SearchDirAndAddCharts(QString& dir_name_base,
                                         ChartClassDescriptor &chart_desc,
                                         QProgressDialog *pprog)
{
    qDebug("Searching directory: %s for %s", dir_name_base.toUtf8().data(), chart_desc.m_search_mask.toUtf8().data());
    QDir dir(dir_name_base);
    if(!dir.exists()) return 0;

    QString dir_name = dir_name_base;
    QString filespec = chart_desc.m_search_mask.toUpper();
    QString lowerFileSpec = chart_desc.m_search_mask.toLower();
    QString filespecXZ = filespec + (".xz");
    QString lowerFileSpecXZ = lowerFileSpec + (".xz");
    QString filename;

    //    Count the files
    QFileInfoList FileList;
//    int gaf_flags = QDir_DEFAULT;                  // as default, recurse into subdirs


    //    Here is an optimization for MSW/cm93 especially
    //    If this directory seems to be a cm93, and we are not explicitely looking for cm93, then abort
    //    Otherwise, we will be looking thru entire cm93 tree for non-existent .KAP files, etc.

    bool b_found_cm93 = false;
    bool b_cm93 = Check_CM93_Structure(dir_name);
    if(b_cm93)
    {
        if (filespec != ("00300000.A"))
            return false;
        else {
            filespec = dir_name;
            b_found_cm93 = true;
        }
    }


    if(!b_found_cm93)
    {
        // Note that `QDir::GetAllFiles()` appends to the list rather than replaces existing contents.
        QDir dir(dir_name);
        FileList.append(dir.entryInfoList(QStringList()<<filespec<<filespecXZ, QDir::Files, QDir::Name));
//        FileList.append(dir.entryList(QStringList()<<filespecXZ));
//        qSort(FileList.begin(), FileList.end()); // Sorted processing order makes the progress bar more meaningful to the user.
    }
    else {                            // This is a cm93 dataset, specified as yada/yada/cm93
        //cm93的情况不处理
//        QString dir_plus = dir_name;
//        dir_plus += QString(zchxFuncUtil::separator());
//        FileList.append(dir_plus);

    }

    int nFile = FileList.count();


    if(!nFile)
        return false;


    int nDirEntry = 0;

    //    Check to see if there are any charts in the DB which refer to this directory
    //    If none at all, there is no need to scan the DB for fullpath match of each potential addition
    //    and bthis_dir_in_dB is false.
    bool bthis_dir_in_dB = IsChartDirUsed(dir_name);

    if(pprog)
    {
        pprog->setRange(0, nFile);
        pprog->setWindowTitle("OpenCPN Chart Add....");
    }

    // build a hash table based on filename (without directory prefix) of
    // the chart to fast to detect identical charts
    ChartCollisionsHashMap collision_map;
    int nEntry = active_chartTable.count();
    for(int i=0 ; i<nEntry ; i++) {
        QString table_file_name = QString::fromUtf8(active_chartTable[i]->GetpFullPath());
        QFileInfo table_file(table_file_name);
        collision_map[table_file.fileName()] = i;
    }

    int nFileProgressQuantum = qMax( nFile / 100, 2 );
    double rFileProgressRatio = 100.0 / qMax( nFile, 1 );

    for(int ifile=0 ; ifile < nFile ; ifile++)
    {
        QFileInfo file(FileList[ifile]);
        QString full_name = file.absoluteFilePath();
        QString file_name = file.fileName();
        //    Validate the file name again, considering MSW's semi-random treatment of case....
        // TODO...something fishy here - may need to normalize saved name?
        //本身过滤的时候就进行了文件过滤,又不处理cm93,所以去掉正则表达式
//        if(!QRegExp(lowerFileSpec).exactMatch(filename) && !QRegExp(filespec).exactMatch(filename) &&
//                !QRegExp(lowerFileSpecXZ).exactMatch(filename) && !QRegExp(filespecXZ).exactMatch(filename) &&
//                !b_found_cm93) {
//            qDebug("FileSpec test failed for:%s", file_name.toUtf8().data());
//            continue;
//        }

        if( pprog && ( ( ifile % nFileProgressQuantum ) == 0 ) )
        {
//            pprog->setValue(static_cast<int>( (ifile+1) * rFileProgressRatio ));
//            pprog->setLabelText(full_name );
        }

        ChartTableEntry *pnewChart = NULL;
        bool bAddFinal = true;
        int b_add_msg = 0;

        // Check the collisions map looking for duplicates, and choosing the right one.
        ChartCollisionsHashMap::const_iterator collision_ptr = collision_map.find( file_name );
        bool collision = ( collision_ptr != collision_map.end() );
        bool file_path_is_same = false;
        bool file_time_is_same = false;
        ChartTableEntry *pEntry = NULL;
        QString table_file_name;

        if( collision ) {
            pEntry = active_chartTable[collision_ptr.value()];
            table_file_name = QString::fromUtf8(pEntry->GetpFullPath());
            file_path_is_same = bthis_dir_in_dB && (full_name == table_file_name);

            // If the chart full file paths are exactly the same, select the newer one.
            if( file_path_is_same ) {
                b_add_msg++;

                //    Check the file modification time
                time_t t_oldFile = pEntry->GetFileTime();
                time_t t_newFile = file.lastModified().toTime_t();

                if( t_newFile <= t_oldFile )
                {
                    file_time_is_same = true;
                    bAddFinal = false;
                    pEntry->SetValid(true);
                }
                else
                {
                    bAddFinal = true;
                    pEntry->SetValid(false);
                }
            }
        }

        QString msg_fn(full_name);
        msg_fn.replace("%", "%%");
        if( file_time_is_same ) {
            // Produce the same output without actually calling `CreateChartTableEntry()`.
            qDebug("Loading chart data for %s", msg_fn.toUtf8().data());
        } else {
            pnewChart = CreateChartTableEntry(full_name, chart_desc);
            if(!pnewChart)
            {
                bAddFinal = false;
                qDebug("   CreateChartTableEntry() failed for file: %s", msg_fn.toUtf8().data());
            }
        }

        if ( !collision || !pnewChart )
        {
            // Do nothing.
        }
        else if( file_path_is_same )
        {
            qDebug("   Replacing older chart file of same path: %s", msg_fn.toUtf8().data());
        }
        else if( !file_time_is_same )
        {
            //  Look at the chart file name (without directory prefix) for a further check for duplicates
            //  This catches the case in which the "same" chart is in different locations,
            //  and one may be newer than the other.
            b_add_msg++;

            if( pnewChart->IsEarlierThan(pEntry) )
            {
                QFileInfo table_file(table_file_name);
                //    Make sure the compare file actually exists
                if( table_file.isReadable() )
                {
                    pEntry->SetValid(true);
                    bAddFinal = false;
                    qDebug("   Retaining newer chart file of same name: %s", msg_fn.toUtf8().data());

                }
            }
            else if( pnewChart->IsEqualTo(pEntry) )
            {
                //    The file names (without dir prefix) are identical,
                //    and the mod times are identical
                //    Prsume that this is intentional, in order to facilitate
                //    having the same chart in multiple groups.
                //    So, add this chart.
                bAddFinal = true;
            }
            else
            {
                pEntry->SetValid(false);
                bAddFinal = true;
                qDebug("   Replacing older chart file of same name: %s", msg_fn.toUtf8().data());
            }
        }


        if(bAddFinal)
        {
            if(0 == b_add_msg)
            {
//                qDebug("   Adding chart file: %s", msg_fn.toUtf8().data());
            }
            collision_map[file_name] = active_chartTable.count();
            active_chartTable.append(pnewChart);
            nDirEntry++;
//            delete pnewChart;
        }
        else
        {
            if (pnewChart)
                delete pnewChart;
            //                    ZCHX_LOGMSG(QString::Format(_T("   Not adding chart file: %s"), msg_fn.c_str()));
        }
    }
    if(pprog) pprog->setValue(100);

    m_nentries = active_chartTable.count();

    return nDirEntry;
}


bool ChartDatabase::AddChart( QString &chartfilename, ChartClassDescriptor &chart_desc, QProgressDialog *pprog,
                              int isearch, bool bthis_dir_in_dB)
{
    bool rv = false;
    QFileInfo file(chartfilename);
    QString full_name = file.filePath();  //路径+名字+扩展名
    QString file_name = file.fileName();  //名字+扩展名
    
    //    Validate the file name again, considering MSW's semi-random treatment of case....
    // TODO...something fishy here - may need to normalize saved name?
    //    if(!file_name.Matches(lowerFileSpec) && !file_name.Matches(filespec) && !b_found_cm93)
    //        continue;
    
    if(pprog)
    {
        pprog->setLabelText(full_name);
        pprog->setValue(qMin((m_pdifile * 100) /m_pdnFile, 100));
    }
    
    
    ChartTableEntry *pnewChart = NULL;
    bool bAddFinal = true;
    int b_add_msg = 0;
    QString msg_fn(full_name);
    msg_fn.replace("%", "%%");
    
    pnewChart = CreateChartTableEntry(full_name, chart_desc);
    if(!pnewChart)
    {
        bAddFinal = false;
        qDebug("   CreateChartTableEntry() failed for file: %s", msg_fn.toUtf8().data());
        return false;
    }
    else         // traverse the existing database looking for duplicates, and choosing the right one
    {
        int nEntry = active_chartTable.count();
        for(int i=0 ; i<nEntry ; i++)
        {
            QString *ptable_file_name = active_chartTable[isearch]->GetpsFullPath();

            //    If the chart full file paths are exactly the same, select the newer one
            if(bthis_dir_in_dB && full_name == *ptable_file_name)
            {
                b_add_msg++;

                //    Check the file modification time
                time_t t_oldFile = active_chartTable[isearch]->GetFileTime();
                time_t t_newFile = file.lastModified().toTime_t();

                if( t_newFile <= t_oldFile )
                {
                    bAddFinal = false;
                    active_chartTable[isearch]->SetValid(true);
                }
                else
                {
                    bAddFinal = true;
                    active_chartTable[isearch]->SetValid(false);
                    qDebug("   Replacing older chart file of same path: %s", msg_fn.toUtf8().data());
                }

                break;
            }


            //  Look at the chart file name (without directory prefix) for a further check for duplicates
            //  This catches the case in which the "same" chart is in different locations,
            //  and one may be newer than the other.
            QFileInfo table_file(*ptable_file_name);

            if( table_file.fileName() == file_name )
            {
                b_add_msg++;

                if(pnewChart->IsEarlierThan(active_chartTable[isearch]))
                {
                    //    Make sure the compare file actually exists
                    if(table_file.isReadable())
                    {
                        active_chartTable[isearch]->SetValid(true);
                        bAddFinal = false;
                        qDebug("   Retaining newer chart file of same name: %s", msg_fn.toUtf8().data());

                    }
                }
                else if(pnewChart->IsEqualTo(active_chartTable[isearch]))
                {
                    //    The file names (without dir prefix) are identical,
                    //    and the mod times are identical
                    //    Prsume that this is intentional, in order to facilitate
                    //    having the same chart in multiple groups.
                    //    So, add this chart.
                    bAddFinal = true;
                }

                else
                {
                    active_chartTable[isearch]->SetValid(false);
                    bAddFinal = true;
                    qDebug("   Replacing older chart file of same name: %s", msg_fn.toUtf8().data());
                }

                break;
            }

            //TODO    Look at the chart ID as a further check against duplicates


            isearch++;
            if(nEntry == isearch)
                isearch = 0;
        }     // for
    }


    if(bAddFinal)
    {
        if(0 == b_add_msg)
        {
            qDebug("   Adding chart file: %s", msg_fn.toUtf8().data());
        }

        active_chartTable.append(pnewChart);
//        delete pnewChart;
        rv = true;
    }
    else
    {
        delete pnewChart;
        //                  ZCHX_LOGMSG(QString::Format(_T("   Not adding chart file: %s"), msg_fn.c_str()));
        rv = false;
    }

    m_nentries = active_chartTable.count();

    return rv;
}

bool ChartDatabase::AddSingleChart( QString &ChartFullPath, bool b_force_full_search )
{
    qDebug("   AddSingleChart() start for file: %s", ChartFullPath.toUtf8().data());
    //  Find a relevant chart class descriptor
    QFileInfo fn(ChartFullPath);
    QString ext = fn.suffix();
    ext.insert(0, "*.");
    QString ext_upper = ext.toUpper();
    QString ext_lower = ext.toLower();
    QString dir_name = fn.absolutePath();

    //    Search the array of chart class descriptors to find a match
    //    bewteen the search mask and the the chart file extension

    ChartClassDescriptor desc;
    for(unsigned int i=0 ; i < m_ChartClassDescriptorArray.count() ; i++)
    {
        if(m_ChartClassDescriptorArray[i]->m_descriptor_type == PLUGIN_DESCRIPTOR)
        {
            if(m_ChartClassDescriptorArray[i]->m_search_mask == ext_upper)
            {
                desc = *(m_ChartClassDescriptorArray[i]);
                break;
            }
            if(m_ChartClassDescriptorArray[i]->m_search_mask == ext_lower)
            {
                desc = *(m_ChartClassDescriptorArray[i]);
                break;
            }
        }
    }
    
    //  If we know that we need to do a full recursive search of the db,
    //  then there is no need to verify it by doing a directory match
    bool b_recurse = true;
    if(!b_force_full_search)
        b_recurse = IsChartDirUsed(dir_name);
    
    bool rv = AddChart( ChartFullPath, desc, NULL, 0,  b_recurse );

    //  remove duplicates marked in AddChart()

    for(unsigned int i=0 ; i<active_chartTable.count() ; i++)
    {
        if(!active_chartTable[i]->GetbValid())
        {
            active_chartTable.removeAt(i);
            i--;                 // entry is gone, recheck this index for next entry
        }
    }
    
    //    Update the Entry index fields
    for(unsigned int i=0 ; i<active_chartTable.count() ; i++)
        active_chartTable[i]->SetEntryOffset( i );

    //  Get a new magic number
    QString new_magic;
    DetectDirChange(dir_name, (""), new_magic, 0);

    
    //    Update (clone) the CDI array
    bool bcfound = false;
    ArrayOfCDI NewChartDirArray;
    
    ArrayOfCDI ChartDirArray = GetChartDirArray();
    for(unsigned int i=0 ; i < ChartDirArray.count(); i++)
    {
        ChartDirInfo cdi = ChartDirArray[i];
        
        ChartDirInfo newcdi = cdi;
        
        //      If entry is found that matches this cell, clear the magic number.
        if( newcdi.fullpath == dir_name ){
            newcdi.magic_number = new_magic;
            bcfound = true;
        }
        
        NewChartDirArray.append(newcdi);
    }
    
    if( !bcfound ){
        ChartDirInfo cdi;
        cdi.fullpath = dir_name;
        cdi.magic_number = new_magic;
        NewChartDirArray.append ( cdi );
    }
    
    
    // Update the database master copy of the CDI array
    SetChartDirArray( NewChartDirArray );
    
    //  Update the list of chart dirs.
    m_chartDirs.clear();
    
    for(unsigned int i=0 ; i < GetChartDirArray().count(); i++)
    {
        ChartDirInfo cdi = GetChartDirArray()[i];
        m_chartDirs.append(cdi.fullpath );
    }
    
    m_nentries = active_chartTable.count();
    
    return rv;
    
}


bool ChartDatabase::RemoveSingleChart( QString &ChartFullPath )
{
    bool rv = false;

    //  Walk the chart table, looking for the target
    for(unsigned int i=0 ; i<active_chartTable.count() ; i++) {
        if( !strcmp( ChartFullPath.toUtf8().data(), GetChartTableEntry(i).GetpFullPath() ) ){
            active_chartTable.removeAt(i);
            break;
        }
    }

    
    //    Update the EntryOffset fields for the array
    ChartTableEntry *pcte;
    
    for(unsigned int i=0 ; i<active_chartTable.count() ; i++){
        pcte = GetpChartTableEntry(i);
        pcte->SetEntryOffset( i );
    }

    //  Check and update the dir array
    QFileInfo fn(ChartFullPath);
    QString fd = fn.absolutePath();
    if( !IsChartDirUsed(fd) ){
        
        //      Clone a new array, removing the unused directory,
        ArrayOfCDI NewChartDirArray;
        
        ArrayOfCDI ChartDirArray = GetChartDirArray();
        for(unsigned int i=0 ; i < ChartDirArray.count(); i++){
            ChartDirInfo cdi = ChartDirArray[i];
            
            ChartDirInfo newcdi = cdi;
            
            if( newcdi.fullpath != fd )
                NewChartDirArray.append(newcdi);
        }
        
        SetChartDirArray( NewChartDirArray );
    }

    //  Update the list of chart dirs.
    m_chartDirs.clear();
    for(unsigned int i=0 ; i < GetChartDirArray().count(); i++)
    {
        ChartDirInfo cdi = GetChartDirArray()[i];
        m_chartDirs.append(cdi.fullpath );
    }
    
    m_nentries = active_chartTable.count();
    
    return rv;
}



///////////////////////////////////////////////////////////////////////
// Create a Chart object
///////////////////////////////////////////////////////////////////////

ChartBase *ChartDatabase::GetChart(const char *theFilePath, ChartClassDescriptor &chart_desc) const
{
    // TODO: support non-UI chart factory
    return NULL;
}

///////////////////////////////////////////////////////////////////////
// Create Chart Table entry by reading chart header info, etc.
///////////////////////////////////////////////////////////////////////

ChartTableEntry* ChartDatabase::CreateChartTableEntry(const QString &filePath, ChartClassDescriptor &chart_desc)
{
    QString msg_fn(filePath);
    msg_fn.replace("%", "%%");
//    qDebug("Loading chart data for %s", msg_fn.toUtf8().data());

    ChartBase *pch = GetChart(filePath.toUtf8().data(), chart_desc);
    if (pch == NULL) {
        qDebug("   ...creation failed for %s", msg_fn.toUtf8().data());
        return NULL;
    }

    InitReturn rc = pch->Init(filePath, HEADER_ONLY);
    if (rc != INIT_OK) {
        delete pch;
        qDebug("   ...initialization failed for %s", msg_fn.toUtf8().data());
        return NULL;
    }


    ChartTableEntry *ret_val = new ChartTableEntry(*pch);
    ret_val->SetValid(true);

    delete pch;

    return ret_val;
}

bool ChartDatabase::GetCentroidOfLargestScaleChart(double *clat, double *clon, ChartFamilyEnum family)
{
    int cur_max_i = -1;
    int cur_max_scale = 0;

    int nEntry = active_chartTable.count();

    for(int i=0 ; i<nEntry ; i++)
    {
        if(GetChartFamily(active_chartTable[i]->GetChartType()) == family)
        {
            if(active_chartTable[i]->GetScale() > cur_max_scale)
            {
                cur_max_scale = active_chartTable[i]->GetScale();
                cur_max_i = i;
            }
        }
    }

    if(cur_max_i == -1)
        return false;                             // nothing found
    else
    {
        *clat = (active_chartTable[cur_max_i]->GetLatMax() + active_chartTable[cur_max_i]->GetLatMin()) / 2.;
        *clon = (active_chartTable[cur_max_i]->GetLonMin() + active_chartTable[cur_max_i]->GetLonMax()) /2.;
    }
    return true;
}

//-------------------------------------------------------------------
//    Get DBChart Projection
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartProj(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex]->GetChartProjectionType();
    else
        return PROJECTION_UNKNOWN;
}

//-------------------------------------------------------------------
//    Get DBChart Family
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartFamily(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex]->GetChartFamily();
    else
        return CHART_FAMILY_UNKNOWN;
}

//-------------------------------------------------------------------
//    Get DBChart FullFileName
//-------------------------------------------------------------------
QString ChartDatabase::GetDBChartFileName(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
    {
        return QString::fromUtf8(active_chartTable[dbIndex]->GetpFullPath());
    }
    else
        return ("");
}


//-------------------------------------------------------------------
//    Get DBChart Type
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartType(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex]->GetChartType();
    else
        return 0;
}

//-------------------------------------------------------------------
//    Get DBChart Skew
//-------------------------------------------------------------------
float ChartDatabase::GetDBChartSkew(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex]->GetChartSkew();
    else
        return 0.;
}

//-------------------------------------------------------------------
//    Get DBChart Scale
//-------------------------------------------------------------------
int ChartDatabase::GetDBChartScale(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
        return active_chartTable[dbIndex]->GetScale();
    else
        return 1;
}

//-------------------------------------------------------------------
//    Get Lat/Lon Bounding Box from db
//-------------------------------------------------------------------
bool ChartDatabase::GetDBBoundingBox(int dbIndex, LLBBox &box)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
    {
        const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
        box.Set(entry.GetLatMin(), entry.GetLonMin(),
                entry.GetLatMax(), entry.GetLonMax());
    }

    return true;
}

const LLBBox &ChartDatabase::GetDBBoundingBox(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) )
    {
        const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
        return entry.GetBBox();
    }
    else
    {
        return m_dummy_bbox;
    }
}


//-------------------------------------------------------------------
//    Get PlyPoint from Database
//-------------------------------------------------------------------
int ChartDatabase::GetDBPlyPoint(int dbIndex, int plyindex, float *lat, float *lon)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
    {
        const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
        if(entry.GetnPlyEntries())
        {
            float *fp = entry.GetpPlyTable();
            fp += plyindex*2;
            if(lat)
                *lat = *fp;
            fp++;
            if(lon)
                *lon = *fp;
        }
        return entry.GetnPlyEntries();
    }
    else
        return 0;
}

//-------------------------------------------------------------------
//    Get AuxPlyPoint from Database
//-------------------------------------------------------------------
int ChartDatabase::GetDBAuxPlyPoint(int dbIndex, int plyindex, int ply, float *lat, float *lon)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
    {
        const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
        if(entry.GetnAuxPlyEntries())
        {
            float *fp = entry.GetpAuxPlyTableEntry(ply);

            fp += plyindex*2;
            if(lat)
                *lat = *fp;
            fp++;
            if(lon)
                *lon = *fp;
        }

        return entry.GetAuxCntTableEntry(ply);
    }
    else
        return 0;
}

int  ChartDatabase::GetnAuxPlyEntries(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
    {
        const ChartTableEntry &entry = GetChartTableEntry(dbIndex);
        return entry.GetnAuxPlyEntries();
    }
    else
        return 0;
}

//-------------------------------------------------------------------
//      Get vector of reduced Plypoints
//-------------------------------------------------------------------
std::vector<float> ChartDatabase::GetReducedPlyPoints(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size())){
        ChartTableEntry *pentry = GetpChartTableEntry(dbIndex);
        if(pentry)
            return pentry->GetReducedPlyPoints();
    }
    
    std::vector<float> dummy;
    return dummy;
}

//-------------------------------------------------------------------
//      Get vector of reduced AuxPlypoints
//-------------------------------------------------------------------
std::vector<float> ChartDatabase::GetReducedAuxPlyPoints(int dbIndex, int iTable)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size())){
        ChartTableEntry *pentry = GetpChartTableEntry(dbIndex);
        if(pentry)
            return pentry->GetReducedAuxPlyPoints( iTable );
    }
    
    std::vector<float> dummy;
    return dummy;
}


bool  ChartDatabase::IsChartAvailable(int dbIndex)
{
    if((bValid) && (dbIndex >= 0) && (dbIndex < (int)active_chartTable.size()))
    {
        ChartTableEntry *pentry = GetpChartTableEntry(dbIndex);
        
        //      If not PLugIn chart, assume always available
        if(pentry->GetChartType() != CHART_TYPE_PLUGIN)
            return true;
        
        QString *path = pentry->GetpsFullPath();
        QFileInfo fn(*path);
        QString ext = fn.suffix();
        ext.insert(0, "*.");
        QString ext_upper = ext.toUpper();
        QString ext_lower = ext.toLower();
        
        //    Search the array of chart class descriptors to find a match
        //    between the search mask and the the chart file extension
        
        for(unsigned int i=0 ; i < m_ChartClassDescriptorArray.count() ; i++)
        {
            if(m_ChartClassDescriptorArray[i]->m_descriptor_type == PLUGIN_DESCRIPTOR){
                
                QString search_mask = m_ChartClassDescriptorArray[i]->m_search_mask;

                if(search_mask == ext_upper) {
                    return true;
                }
                if(search_mask == ext_lower) {
                    return true;
                }
                if(QRegExp(search_mask).exactMatch(*path)) {
                    return true;
                }
                
            }
        }
    }

    return false;
}

void ChartDatabase::ApplyGroupArray(ChartGroupArray *pGroupArray)
{
    QString separator(zchxFuncUtil::separator());
    for(unsigned int ic=0 ; ic < active_chartTable.count(); ic++)
    {
        ChartTableEntry *pcte = active_chartTable[ic];

        pcte->ClearGroupArray();

        QString *chart_full_path = pcte->GetpsFullPath();

        for(unsigned int igroup = 0; igroup < pGroupArray->count(); igroup++)
        {
            ChartGroup *pGroup = pGroupArray->at(igroup);
            for(auto& elem : pGroup->m_element_array )
            {
                QString element_root = elem->m_element_name;

                //  The element may be a full single chart name
                //  If so, add it
                //  Otherwise, append a sep character so that similar paths are distinguished.
                //  See FS#1060
                if(chart_full_path != element_root) element_root.append(separator);    // Prevent comingling similar looking path names
                if(chart_full_path->startsWith(element_root))
                {
                    bool b_add = true;
                    for(unsigned int k=0 ; k < elem->m_missing_name_array.size(); k++)
                    {
                        QString missing_item = elem->m_missing_name_array[k];
                        if(chart_full_path->startsWith(missing_item))
                        {
                            if(chart_full_path == missing_item) // missing item is full chart name
                            {
                                b_add = false;
                                break;
                            }
                            else
                            {
                                if(zchxFuncUtil::isDirExist(missing_item))     // missing item is a dir
                                {
                                    b_add = false;
                                    break;
                                }
                            }
                        }
                    }

                    if(b_add)
                        pcte->AddIntToGroupArray( igroup + 1 );
                }
            }
        }
    }

}
