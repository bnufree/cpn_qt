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
#include "dychart.h"
#include "viewport.h"
#include "glTexCache.h"
#include "glTextureDescriptor.h"

#include "chcanv.h"
#include "glChartCanvas.h"
#include "Quilt.h"
#include "chartbase.h"
#include "chartimg.h"
#include "chartdb.h"
#include "OCPNPlatform.h"
#include "FontMgr.h"
#include "mipmap/mipmap.h"
#include "zchxmapmainwindow.h"

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif

#include "texcmp/squish.h"
#include "lz4/lz4.h"
#include "lz4/lz4hc.h"
#include "gl/glext.h"
#include "gl/gl_private.h"

#include <QProgressDialog>


//typedef         QList<ChartCanvas*>     arrayofCanvasPtr;

extern double                   gLat, gLon, gCog, gSog, gHdt;

extern int g_mipmap_max_level;
extern GLuint g_raster_format;
extern int          g_nCacheLimit;
extern int          g_memCacheLimit;
extern ChartDB      *ChartData;
extern zchxGLOptions    g_GLOptions;
extern long g_tex_mem_used;
extern int              g_tile_size;
extern int              g_uncompressed_tile_size;
extern int              g_nCPUCount;

extern bool             b_inCompressAllCharts;
extern zchxMapMainWindow         *gFrame;
//extern arrayofCanvasPtr  g_canvasArray;

extern OCPNPlatform *g_Platform;
extern ColorScheme global_color_scheme;

extern PFNGLGETCOMPRESSEDTEXIMAGEPROC s_glGetCompressedTexImage;
//extern bool GetMemoryStatus( int *mem_total, int *mem_used );
extern QThread*         g_Main_thread;

bool bthread_debug;
bool g_throttle_squish;

glTextureManager   *g_glTextureManager = 0;

#include "ssl_sha1/sha1.h"

QString CompressedCachePath(const QString& src)
{
    QString path = src;
    int colon = path.indexOf(":", 0);
    if(colon >= 0) path.remove(colon, 1);
    
    /* replace path separators with ! */
    QString separator = g_Platform->GetPathSeparator();
    int pos = 0;
    while( (pos = path.indexOf(separator, pos)) >= 0)
    {
        path.replace(pos, 1, ("!"));
    }

    //  Obfuscate the compressed chart file name, to (slightly) protect some encrypted raster chart data.
    QByteArray buf = path.toUtf8();
    unsigned char sha1_out[20];
    sha1( (unsigned char *) buf.data(), buf.size(), sha1_out );

    QString sha1_res;
    for (unsigned int i=0 ; i < 20 ; i++){
        QString s;
        s.sprintf("%02X", sha1_out[i]);
        sha1_res += s;
    }

    return g_Platform->GetDataDir() + separator + QString("raster_texture_cache") + separator + sha1_res;
    
}

int g_mipmap_max_level = 4;
    
        
static double chart_dist(int index)
{
    double d;
    float  clon;
    float  clat;
    const ChartTableEntry &cte = ChartData->GetChartTableEntry(index);
    // if the chart contains ownship position set the distance to 0
    if (cte.GetBBox().Contains(gLat, gLon))
        d = 0.;
    else {
        // find the nearest edge 
        double t;
        clon = (cte.GetLonMax() + cte.GetLonMin())/2;
        d = DistGreatCircle(cte.GetLatMax(), clon, gLat, gLon);
        t = DistGreatCircle(cte.GetLatMin(), clon, gLat, gLon);
        if (t < d)
            d = t;
            
        clat = (cte.GetLatMax() + cte.GetLatMin())/2;
        t = DistGreatCircle(clat, cte.GetLonMin(), gLat, gLon);
        if (t < d)
            d = t;
        t = DistGreatCircle(clat, cte.GetLonMax(), gLat, gLon);
        if (t < d)
            d = t;
    }
    return d;
}

struct MyInt{
    int idx;
    MyInt() {idx = 0;}
    MyInt(int id) {idx = id;}

    bool sort(const MyInt& p1, const MyInt& p2)
    {
        return (int)(chart_dist(p1.idx) - chart_dist(p2.idx));
    }
};

int CompareInts(int n1, int n2)
{
    double d1 = chart_dist(n1);
    double d2 = chart_dist(n2);
    return (int)(d1 - d2);
}

class MySortedArrayInt : public QList<int>
{
public:
    MySortedArrayInt():QList<int>() {}
    void Append(int i)
    {
        this->append(i);
        qSort(this->begin(), this->end(), CompareInts);
    }
};


static MySortedArrayInt idx_sorted_by_distance/*(CompareInts)*/;

class compress_target
{
public:
    QString chart_path;
    double distance;
};


typedef QList<compress_target*>  ArrayOfCompressTargets;
//WX_DEFINE_OBJARRAY(ArrayOfCompressTargets);


JobTicket::JobTicket()
{
    for(int i=0 ; i < 10 ; i++) {
        compcomp_size_array[i] = 0;
        comp_bits_array[i] = NULL;
        compcomp_bits_array[i] = NULL;
    }
}

/* reduce pixel values to 5/6/5, because this is the format they are stored
 *   when compressed anyway, and this way the compression algorithm will use
 *   the exact same color in  adjacent 4x4 tiles and the result is nicer for our purpose.
 *   the lz4 compressed texture is smaller as well. */
static 
void FlattenColorsForCompression(unsigned char *data, int dim, bool swap_colors=true)
{
    #ifdef __WXMSW__ /* undo BGR flip from ocpn_pixel (if ocpnUSE_ocpnBitmap is defined) */
    if(swap_colors)
        for(int i = 0; i<dim*dim; i++) {
            int off = 3*i;
            unsigned char t = data[off + 0];
            data[off + 0] = data[off + 2] & 0xfc;
            data[off + 1] &= 0xf8;
            data[off + 2] = t & 0xfc;
        }
        else
            #endif
            for(int i = 0; i<dim*dim; i++) {
                int off = 3*i;
                data[off + 0] &= 0xfc;
                data[off + 1] &= 0xf8;
                data[off + 2] &= 0xfc;
            }
}

/* return malloced data which is the etc compressed texture of the source */
static 
void CompressDataETC(const unsigned char *data, int dim, int size,
                     unsigned char *tex_data, volatile bool &b_abort)
{
    Q_ASSERT(dim*dim == 2*size || (dim < 4 && size==8)); // must be 4bpp
    uint64_t *tex_data64 = (uint64_t*)tex_data;
    
    int mbrow = qMin(4, dim), mbcol = qMin(4, dim);
    uint8_t block[48] = {};
    for(int row=0; row<dim; row+=4) {
        for(int col=0; col<dim; col+=4) {
            for(int brow=0; brow<mbrow; brow++)
                for(int bcol=0; bcol<mbcol; bcol++)
                    memcpy(block + (bcol*4+brow)*3,
                           data + ((row+brow)*dim + col+bcol)*3, 3);
                    
            extern uint64_t ProcessRGB( const uint8_t* src );
            *tex_data64++ = ProcessRGB( block );
        }
        if(b_abort)
            break;
    }
}

static
bool CompressUsingGPU(const unsigned char *data, int dim, int size,
                      unsigned char *tex_data, int level, bool inplace)
{
    if( !s_glGetCompressedTexImage )
        return false;
    
    GLuint comp_tex;
    if(!inplace) {
        glGenTextures(1, &comp_tex);
        glBindTexture(GL_TEXTURE_2D, comp_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        level = 0;
    }

    glTexImage2D(GL_TEXTURE_2D, level, g_raster_format,
                 dim, dim, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    
    GLint compressed;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);
    /* if the compression has been successful */
    if (compressed == GL_TRUE){
        // If our compressed size is reasonable, save it.
        GLint compressedSize;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
                                 GL_TEXTURE_COMPRESSED_IMAGE_SIZE,
                                 &compressedSize);
        
        if (compressedSize != size)
            return false;
            
        // Read back the compressed texture.
        s_glGetCompressedTexImage(GL_TEXTURE_2D, level, tex_data);
    }

    if(!inplace)
        glDeleteTextures(1, &comp_tex);
    
    return true;
}

static 
void GetLevel0Map( glTextureDescriptor *ptd,  const QRect &rect, QString &chart_path )
{
    // Load level 0 uncompressed data
    QRect ncrect(rect);
    ptd->map_array[0] = 0;
    
    ChartBase *pChart = ChartData->OpenChartFromDB( chart_path, FULL_INIT );
    if( !pChart ) {
        ptd->map_array[0] = (unsigned char *) calloc( ncrect.width() * ncrect.height() * 4, 1 );
        return;
    }
    
        //    Prime the pump with the "zero" level bits, ie. 1x native chart bits
    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pChart );
        
    if( pBSBChart ) {
        unsigned char *t_buf = (unsigned char *) malloc( ncrect.width() * ncrect.height() * 4 );
        pBSBChart->GetChartBits( ncrect, t_buf, 1 );
        
        //    and cache them here
        ptd->map_array[0] = t_buf;
    } else {
        ptd->map_array[0] = (unsigned char *) calloc( ncrect.width() * ncrect.height() * 4, 1 );
        return;
    }
}



void GetFullMap( glTextureDescriptor *ptd,  const QRect &rect, QString chart_path, int level)
{
    
    //  Confirm that the uncompressed bits are all available, get them if not there yet
    if( ptd->map_array[level]) 
        return;

    // find next lower level with map_array
    int first_level;
    for(first_level = level; first_level; first_level--)
        if( ptd->map_array[first_level - 1] )
            break;

    //Get level 0 bits from chart?
    if( !first_level ) {
        GetLevel0Map( ptd, rect, chart_path );
        first_level = 1;
    }

    int dim = g_GLOptions.m_iTextureDimension;
    for(int i=0; i<=level; i++) {
        if(i >= first_level) {
            ptd->map_array[i] = (unsigned char *) malloc( dim * dim * 3 );
            MipMap_24( 2*dim, 2*dim, ptd->map_array[i - 1], ptd->map_array[i] );
        }
        dim /= 2;
    }
}

int TextureDim(int level)
{
    int dim = g_GLOptions.m_iTextureDimension;
    for(int i=0 ; i < level ; i++)
        dim /= 2;
    return dim;
}

int TextureTileSize(int level, bool compressed)
{
    if(level == g_mipmap_max_level + 1)
        return 0;

    int size;
    if(compressed) {
        size = g_tile_size;
        for(int i=0 ; i < level ; i++){
            size /= 4;
            if(size < 8)
                size = 8;
        }
    } else {
        size = g_uncompressed_tile_size;
        for(int i=0 ; i < level ; i++)
            size /= 4;
    }

    return size;
}

bool JobTicket::DoJob()
{
    if(!m_rect.isEmpty())  return DoJob(m_rect);

    // otherwise this ticket covers all the rects in the chart
    ChartBase *pchart = ChartData->OpenChartFromDB( m_ChartPath, FULL_INIT );
    if(!pchart)
        return false;

    ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );
    if(!pBSBChart)
        return false;

    int size_X = pBSBChart->GetSize_X();
    int size_Y = pBSBChart->GetSize_Y();

    int dim = g_GLOptions.m_iTextureDimension;
        
    int nx_tex = ceil( (float)size_X / dim );
    int ny_tex = ceil( (float)size_Y / dim );
        
    int nt = ny_tex * nx_tex;
        
    QRect rect(0, 0, dim, dim);
    for( int y = 0; y < ny_tex; y++ ) {
        
        if( pthread ) {
            OCPN_CompressionThreadMsg msg;
            msg.nstat = y;
            msg.nstat_max = ny_tex;
            msg.type = 1;
            msg.m_ticket = this;
            emit pthread->signalSendCompressionMsg(msg);
        }
        for( int x = 0; x < nx_tex; x++ ) {
            if(!DoJob(rect))
                return false;
            
            pFact->UpdateCacheAllLevels(rect, global_color_scheme, compcomp_bits_array, compcomp_size_array);

            for(int i=0 ; i < g_mipmap_max_level+1 ; i++) {
                free(comp_bits_array[i]), comp_bits_array[i] = 0;
                free(compcomp_bits_array[i]), compcomp_bits_array[i] = 0;
            }

            
            rect.setX(rect.x() + rect.width());
        }
        rect.setY(rect.y() + rect.height());
    }
    
    return true;
}

#if 1// defined( __UNIX__ ) && !defined(__WXOSX__)  // high resolution stopwatch for pro
class OCPNStopWatch
{
public:
    OCPNStopWatch() { Start(); }
    void Start() { clock_gettime(CLOCK_REALTIME, &tp); }

    double Time() {
        timespec tp_end;
        clock_gettime(CLOCK_REALTIME, &tp_end);
        return (tp_end.tv_sec - tp.tv_sec) * 1.e3 + (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
    }

private:
    timespec tp;
};
#else
class OCPNStopWatch : public wxStopWatch
{ };
#endif

static void throttle_func(void *data)
{
    if(QThread::currentThread() != g_Main_thread) {
        OCPNStopWatch *sww = (OCPNStopWatch *)data;
        if(sww->Time() > 1) {
            sww->Start();
            QThread::sleep(2);
        }
    }
}

bool JobTicket::DoJob(const QRect &rect)
{
    unsigned char *bit_array[10];
    for(int i=0 ; i < 10 ; i++)
        bit_array[i] = 0;

    QRect ncrect(rect);

    bit_array[0] = level0_bits;
    level0_bits = NULL;

    if(!bit_array[0]) {
        //  Grab a copy of the level0 chart bits
        // we could alternately subsample grabbing leveln chart bits
        // directly here to speed things up...
        ChartBase *pchart;
        int index;
    
        if(ChartData){
            index =  ChartData->FinddbIndex( m_ChartPath );
            pchart = ChartData->OpenChartFromDBAndLock(index, FULL_INIT );

            if(pchart && ChartData->IsChartLocked( index )){
                ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );
                if( pBSBChart ) {
                    bit_array[0] = (unsigned char *) malloc( ncrect.width() * ncrect.height() * 4 );
                    pBSBChart->GetChartBits( ncrect, bit_array[0], 1 );
                }
                ChartData->UnLockCacheChart(index);
            }
            else
                bit_array[0] = NULL;
        }
    }
    
    //OK, got the bits?
    int ssize, dim;
    if(!bit_array[0] )
        return false;
    
    //  Fill in the rest of the private uncompressed array
    dim = g_GLOptions.m_iTextureDimension;
    dim /= 2;
    for( int i = 1 ; i < g_mipmap_max_level+1 ; i++ ){
        size_t nmalloc = qMax(dim * dim * 3, 4*4*3);
        bit_array[i] = (unsigned char *) malloc( nmalloc );
        MipMap_24( 2*dim, 2*dim, bit_array[i - 1], bit_array[i] );
        dim /= 2;
    }
        
    int texture_level = 0;
    for( int level = level_min_request; level < g_mipmap_max_level+1 ; level++ ){
        int dim = TextureDim(level);
        int size = TextureTileSize(level, true);
        unsigned char *tex_data = (unsigned char*)malloc(size);
        if(g_raster_format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
            // color range fit is worse quality but twice as fast
            int flags = squish::kDxt1 | squish::kColourRangeFit;
            
            if( g_GLOptions.m_bTextureCompressionCaching) {
                /* use slower cluster fit since we are building the cache for
                 * better quality, this takes roughly 25% longer and uses about
                 * 10% more disk space (result doesn't compress as well with lz4) */
                flags = squish::kDxt1 | squish::kColourClusterFit;
            }
            
            OCPNStopWatch sww;
            squish::CompressImageRGBpow2_Flatten_Throttle_Abort( bit_array[level], dim, dim, tex_data, flags,
                                                                 true, b_throttle ? throttle_func : 0, &sww, b_abort );
            
        } else if(g_raster_format == GL_ETC1_RGB8_OES) 
            CompressDataETC(bit_array[level], dim, size, tex_data, b_abort);
        else if(g_raster_format == GL_COMPRESSED_RGB_FXT1_3DFX) {
            if(!CompressUsingGPU(bit_array[level], dim, size, tex_data, texture_level, binplace)) {
                b_abort = true;
                break;
            }

            if(binplace)
                g_tex_mem_used += size;

            texture_level++;
        }
        comp_bits_array[level] = tex_data;
            
        if(b_abort){
            for( int i = 0; i < g_mipmap_max_level+1; i++ ){
                free( bit_array[i] );
                bit_array[i] = 0;
            }
            return false;
        }
    }
        
    //  All done with the uncompressed data in the thread
    for( int i = 0; i < g_mipmap_max_level+1; i++ ) {
        free( bit_array[i] );
        bit_array[i] = 0;
    }

    if(b_throttle)
        QThread::sleep(1);
    
    if(b_abort)
        return false;

    if(bpost_zip_compress) {
            
        int max_compressed_size = LZ4_COMPRESSBOUND(g_tile_size);
        for(int level = level_min_request; level < g_mipmap_max_level+1 ; level++){
            if(b_abort)
                return false;

            unsigned char *compressed_data = (unsigned char *)malloc(max_compressed_size);
            int csize = TextureTileSize(level, true);

            char *src = (char *)comp_bits_array[level];
            int compressed_size = LZ4_compressHC2( src, (char *)compressed_data, csize, 4);
            // shrink buffer to actual size.
            // This will greatly reduce ram usage, ratio usually 10:1
            // there might be a more efficient way than realloc...
            compressed_data = (unsigned char*)realloc(compressed_data, compressed_size);
            compcomp_bits_array[level] = compressed_data;
            compcomp_size_array[level] = compressed_size;
        }
    }

    return true;
}

//  On Windows, we will use a translator to convert SEH exceptions (e.g. access violations),
//    into c++ standard exception handling method.
//  This class and helper function facilitate the conversion.

//  We only do this in the compression worker threads, as they are vulnerable due to possibly errant code in 
//  the chart database management class, especially on low memory systems where chart cahing is stressed heavily.

#ifdef __WXMSW__
class SE_Exception
{
private:
    unsigned int nSE;
public:
    SE_Exception() {}
    SE_Exception( unsigned int n ) : nSE( n ) {}
    ~SE_Exception() {}
    unsigned int getSeNumber() { return nSE; }
};

void my_translate(unsigned int code, _EXCEPTION_POINTERS *ep)
{
    throw SE_Exception();
}
#endif



CompressionPoolThread::CompressionPoolThread(JobTicket *ticket) : QThread(0)
{
    qRegisterMetaType<OCPN_CompressionThreadMsg>("const OCPN_CompressionThreadMsg&");
    m_ticket = ticket;
    setPriority(QThread::LowPriority);
}

void CompressionPoolThread::run()
{
    if(!m_ticket) return;

    if(!m_ticket->DoJob())  m_ticket->b_isaborted = true;

    OCPN_CompressionThreadMsg msg;
    msg.m_ticket = m_ticket;
    msg.type = 0;

    emit signalSendCompressionMsg(msg);

    return;
}

//      ProgressInfoItem Implementation





//      glTextureManager Implementation
glTextureManager::glTextureManager() : QObject(0), m_timer(0)
{
    // ideally we would use the cpu count -1, and only launch jobs
    // when the idle load average is sufficient (greater than 1)
    int nCPU =  qMax(1, g_Platform->getCpuCorNum());
    if(g_nCPUCount > 0)
        nCPU = g_nCPUCount;

    if (nCPU < 1) 
        // obviously there's at least one CPU!
        nCPU = 1;

    m_max_jobs =  qMax(nCPU, 1);
    m_prevMemUsed = 0;    

    if(bthread_debug)
        printf(" nCPU: %d    m_max_jobs :%d\n", nCPU, m_max_jobs);
    
    m_progDialog = NULL;
    
    for(int i=0 ; i < m_max_jobs ; i++)
        progList.append(ProgressInfoItem());

    
    m_ticks = 0;
    m_skip = false;
    m_bcompact = false;
    m_skipout = false;
    
    m_timer = new QTimer;
    m_timer->setInterval(500);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
    m_timer->start();
}

glTextureManager::~glTextureManager()
{
    if(m_timer)
    {
        m_timer->stop();
        delete m_timer;
    }
//    ClearAllRasterTextures();
    ClearJobList();
}

#define NBAR_LENGTH 40

void glTextureManager::OnEvtThread( const OCPN_CompressionThreadMsg & event )
{
    JobTicket *ticket = event.m_ticket;
    if(event.type ==1){
        if(!m_progDialog){
            // currently unreachable, but...
            return;
        }
        // Look for a matching entry...
        bool bfound = false;
        ProgressInfoItem * item = 0;
        for(int i=0; i<progList.size(); i++)
        {
            item = &(progList[i]);
            if(item->file_path == ticket->m_ChartPath)
            {
                bfound = true;
                break;
            }
        }
        if (!bfound)
        {
            item = 0;
            // look for an empty slot
            int i = 0;
            while (i < progList.length())
            {
                item = &(progList[i]);
                if(item->file_path.isEmpty())
                {
                    bfound = true;
                    item->file_path = ticket->m_ChartPath;
                    break;
                } else
                {
                    item = 0;
                }
            }
        }

        if(bfound)
        {
            QString msgx;
            if(1)
            {
                int bar_length = NBAR_LENGTH;
                if(m_bcompact) bar_length = 20;
                
                msgx += QString("\n[");
                QString block = QString("").sprintf("%c", 0x2588);
                float cutoff = -1.;
                if (event.nstat_max != 0)
                    cutoff = ((event.nstat+1) / (float)event.nstat_max) * bar_length;
                for(int i=0 ; i < bar_length ; i++){
                    if(i <= cutoff)
                        msgx += block;
                    else
                        msgx += QString("-");
                }
                msgx += QString("]");

                if(!m_bcompact){
                    QString msgy;
                    msgy.sprintf("  [%3d/%3d]  ", event.nstat+1, event.nstat_max);
                    msgx += msgy;

                    QFileInfo fn(ticket->m_ChartPath);
                    msgx += fn.filePath();
                }
            } else
            {
                msgx.sprintf("\n %3d/%3d", event.nstat+1, event.nstat_max);
            }
            
            if(item) item->msgx = msgx;
        }

        // Ready to compose
        QString msg;
        foreach (ProgressInfoItem item, progList) {
            msg += item.msgx + QString("\n");
        }
        if(m_skipout)
        {
            m_progMsg = QString("Skipping, please wait...\n\n");
        }
        if(m_progDialog)
        {
            m_progDialog->setLabelText( m_progMsg + msg);
            m_progDialog->setValue(m_jcnt);
        }
        
//        if (!m_progDialog->Update(m_jcnt, m_progMsg + msg, &m_skip ))
//            m_skip = true;
        if(m_skip)
            m_skipout = true;
        return;
    }
    
    if(ticket->b_isaborted || ticket->b_abort){
        for(int i=0 ; i < g_mipmap_max_level+1 ; i++) {
            free(ticket->comp_bits_array[i]);
            free( ticket->compcomp_bits_array[i] );
        }
        
        if(bthread_debug)
            qDebug( "    Abort job: %08X  Jobs running: %d             Job count: %lu   \n",
                    ticket->ident, GetRunningJobCount(), (unsigned long)todo_list.count());
    } else if(!ticket->b_inCompressAll) {
        //   Normal completion from here
        glTextureDescriptor *ptd = ticket->pFact->GetpTD( ticket->m_rect );
        if(ptd) {
            for(int i=0 ; i < g_mipmap_max_level+1 ; i++)
                ptd->comp_array[i] = ticket->comp_bits_array[i];

            if(ticket->bpost_zip_compress){
                for(int i=0 ; i < g_mipmap_max_level+1 ; i++){
                    ptd->compcomp_array[i] = ticket->compcomp_bits_array[i];
                    ptd->compcomp_size[i] = ticket->compcomp_size_array[i];
                }
            }

                    
                    
            // We need to force a refresh to replace the uncompressed texture
            // This frees video memory and is also really required if we had
            // gone up a mipmap level
            gFrame->InvalidateAllGL();
            ptd->compdata_ticks = 10;
        }

        if(bthread_debug)
            qDebug( "    Finished job: %08X  Jobs running: %d             Job count: %lu   \n",
                    ticket->ident, GetRunningJobCount(), (unsigned long)todo_list.count());
    }

    //      Free all possible memory
    if(ticket->b_inCompressAll) { // if compressing all write cache here
        ChartBase *pchart = ChartData->OpenChartFromDB(ticket->m_ChartPath, FULL_INIT );
        ChartData->DeleteCacheChart(pchart);
        delete ticket->pFact;
    }


    for(int i=0; i<progList.size(); i++)
    {
        ProgressInfoItem *item = &(progList[i]);
        if(item->file_path == ticket->m_ChartPath)
        {
            item->file_path.clear();
            break;
        }
    }
    


    if(g_raster_format != GL_COMPRESSED_RGB_FXT1_3DFX) {
        running_list.removeOne(ticket);
        StartTopJob();
    }

    delete ticket;
}

void glTextureManager::OnTimer()
{
    m_ticks++;
    
    //  Scrub all the TD's, looking for any completed compression jobs
    //  that have finished
    //  In the interest of not disturbing the GUI, process only one TD per tick
    if(g_GLOptions.m_bTextureCompression) {
        for(ChartPathHashTexfactType::iterator itt = m_chart_texfactory_hash.begin();
            itt != m_chart_texfactory_hash.end(); ++itt ) {
            glTexFactory *ptf = itt.value();
            if(ptf && ptf->OnTimer())
            {
                //break;
            }
        }
    }

#if 0
    if((m_ticks % 4/*120*/) == 0){
    
    // inventory
    int mem_total, mem_used;
    GetMemoryStatus(&mem_total, &mem_used);

    int map_size = 0;
    int comp_size = 0;
    int compcomp_size = 0;
    
    for(ChartPathHashTexfactType::iterator itt = m_chart_texfactory_hash.begin();
        itt != m_chart_texfactory_hash.end(); ++itt ) {
        glTexFactory *ptf = itt->second;

        ptf->AccumulateMemStatistics(map_size, comp_size, compcomp_size);
    }

    int m1 = 1024 * 1024;
//    QString path = wxFileName(m_ChartPath).GetName();
    printf("%6d %6ld Map: %10d  Comp:%10d  CompComp: %10d \n", mem_used/1024, g_tex_mem_used/m1, map_size, comp_size, compcomp_size);//, path.mb_str().data());
  
///    qDebug() << "inv" << map_size/m1 << comp_size/m1 << compcomp_size/m1 << g_tex_mem_used/m1 << mem_used/1024;
    }
#endif
}


bool glTextureManager::ScheduleJob(glTexFactory* client, const QRect &rect, int level,
                                   bool b_throttle_thread, bool b_nolimit, bool b_postZip, bool b_inplace)
{
    QString chart_path = client->GetChartPath();
    if(!b_nolimit)
    {
        if(todo_list.count() >= 50)
        {
            // remove last job which is least important
            JobTicket *ticket = todo_list.last();
            todo_list.removeLast();
            delete ticket;
        }

        //  Avoid adding duplicate jobs, i.e. the same chart_path, and the same rectangle
        for(int i=0; i<todo_list.size(); i++)
        {
            JobTicket *ticket = todo_list[i];
            if( (ticket->m_ChartPath == chart_path) && (ticket->m_rect == rect)) {
                // bump to front
                todo_list.removeAt(i);
                todo_list.insert(0, ticket);
                ticket->level_min_request = level;
                return false;
            }
        }

        // avoid duplicate worker jobs
        for(int i=0; i<running_list.size(); i++)
        {
            JobTicket *ticket = running_list[i];
            if(ticket->m_rect == rect && ticket->m_ChartPath == chart_path) {
                return false;
            }
        }
    }
    
    JobTicket *pt = new JobTicket;
    pt->pFact = client;
    pt->m_rect = rect;
    pt->level_min_request = level;
    glTextureDescriptor *ptd = client->GetOrCreateTD( pt->m_rect );
    pt->ident = (ptd->tex_name << 16) + level;
    pt->b_throttle = b_throttle_thread;
    pt->m_ChartPath = chart_path;

    pt->level0_bits = NULL;
    pt->b_abort = false;
    pt->b_isaborted = false;
    pt->bpost_zip_compress = b_postZip;
    pt->binplace = b_inplace;
    pt->b_inCompressAll = b_inCompressAllCharts;
    

    /* do we compress in ram using builtin libraries, or do we
       upload to the gpu and use the driver to perform compression?
       we have builtin libraries for DXT1 (squish) and ETC1 (etcpak)
       FXT1 must use the driver, ETC1 cannot, and DXT1 can use the driver
       but the results are worse and don't compress well.

    additionally, if we use the driver we must stay single threaded in this thread
    (unless we created multiple opengl contexts), but with with our own libraries,
    we can use multiple threads to take advantage of multiple cores */

    if(g_raster_format != GL_COMPRESSED_RGB_FXT1_3DFX) {
        todo_list.insert(0, pt); // push to front as a stack
        if(bthread_debug){
            int mem_used;
            zchxFuncUtil::getMemoryStatus(0, &mem_used);
            qDebug( "Adding job: %08X  Job Count: %lu  mem_used %d\n", pt->ident, (unsigned long)todo_list.count(), mem_used);
        }
 
        StartTopJob();
    }
    else {
        // give level 0 buffer to the ticket
        pt->level0_bits = ptd->map_array[0];
        ptd->map_array[0] = NULL;
        
        pt->DoJob();

//        OCPN_CompressionThreadEvent Nevent(wxEVT_OCPN_COMPRESSIONTHREAD, 0);
//        Nevent.type = 0;
//        Nevent.SetTicket(pt);
//        ProcessEventLocally(Nevent);
        // from here m_ticket is undefined (if deleted in event handler)
    }
    return true;
}

bool glTextureManager::StartTopJob()
{
    if(todo_list.size() == 0) return false;
    JobTicket *ticket = todo_list.first();

    //  Is it possible to start another job?
    if(GetRunningJobCount() >= qMax(m_max_jobs - ticket->b_throttle, 1))  return false;

    todo_list.removeFirst();

    glTextureDescriptor *ptd = ticket->pFact->GetpTD( ticket->m_rect );
    // don't need the job if we already have the compressed data
    if(ptd->comp_array[0]) {
        delete ticket;
        return StartTopJob();
    }

    if(ptd->map_array[0]) {
        if(ticket->level_min_request == 0) {
            // give level 0 buffer to the ticket
            ticket->level0_bits = ptd->map_array[0];
            ptd->map_array[0] = NULL;
        } else {
            // would be nicer to use reference counters
            int size = TextureTileSize(0, false);
            ticket->level0_bits = (unsigned char*)malloc(size);
            memcpy(ticket->level0_bits, ptd->map_array[0], size);
        }
    }

    running_list.append(ticket);
    DoThreadJob(ticket);

    return true;
}


bool glTextureManager::DoThreadJob(JobTicket* pticket)
{
    if(bthread_debug)
        qDebug( "  Starting job: %08X  Jobs running: %d Jobs left: %lu\n", pticket->ident, GetRunningJobCount(), (unsigned long)todo_list.count());
    
    ///    qDebug() << "Starting job" << GetRunningJobCount() <<  (unsigned long)todo_list.GetCount() << g_tex_mem_used;
    CompressionPoolThread *t = new CompressionPoolThread( pticket);
    connect(t, SIGNAL(signalSendCompressionMsg(OCPN_CompressionThreadMsg)), this, SLOT(OnEvtThread(OCPN_CompressionThreadMsg)));
    pticket->pthread = t;
    
    t->start();
    
    return true;
    
}

bool glTextureManager::AsJob( QString const &chart_path ) const
{
    if(chart_path.length() > 0){
        for(int i=0; i<running_list.size(); i++)
        {
            JobTicket *ticket = running_list[i];
            if(ticket->m_ChartPath == chart_path) {
                return true;
            }
        }
    }
    return false;
}

void glTextureManager::PurgeJobList( QString chart_path )
{
    if(chart_path.length())
    {
        for(int i=0; i<todo_list.size();)
        {
            JobTicket *ticket = running_list[i];
            if(ticket->m_ChartPath == chart_path) {
                running_list.removeAt(i);
                delete ticket;
                if(bthread_debug)
                {
                    qDebug("Pool:  Purge pending job for purged chart\n");
                }
                continue;
            }
            i++;
        }

        for(int i=0; i<todo_list.size(); i++)
        {
            JobTicket *ticket = running_list[i];
            if(ticket->m_ChartPath == chart_path) {
                ticket->b_abort = true;
            }
        }

        if(bthread_debug)
        {
            qDebug("Pool:  Purge, todo count: %lu\n", (long unsigned)todo_list.count());
        }
    }
    else {
        while(todo_list.size()){
            JobTicket *ticket = todo_list.takeFirst();
            delete ticket;
        }
        //  Mark all running tasks for "abort"
        for(int i=0; i<todo_list.size(); i++)
        {
            JobTicket *ticket = running_list[i];
            ticket->b_abort = true;
        }
    }        
}

void glTextureManager::ClearJobList()
{
    while(todo_list.size()){
        JobTicket *ticket = todo_list.takeFirst();
        delete ticket;
    }
}


void glTextureManager::ClearAllRasterTextures( void )
{
    
    //     Delete all the TexFactory instances
    ChartPathHashTexfactType::iterator itt;
    for( itt = m_chart_texfactory_hash.begin(); itt != m_chart_texfactory_hash.end(); ++itt ) {
        glTexFactory *ptf = itt.value();
        
        delete ptf;
    }
    m_chart_texfactory_hash.clear();
    
    if(g_tex_mem_used != 0)
    {
        qDebug("Texture memory use calculation error\n");
    }
}

bool glTextureManager::PurgeChartTextures( ChartBase *pc, bool b_purge_factory )
{
    //    Look for the texture factory for this chart
    ChartPathHashTexfactType::iterator ittf = m_chart_texfactory_hash.find( pc->GetHashKey() );
    
    //    Found ?
    if( ittf != m_chart_texfactory_hash.end() )
    {
        glTexFactory *pTexFact = ittf.value();
        if(pTexFact)
        {
            if( b_purge_factory)
            {
                m_chart_texfactory_hash.erase(ittf);                // This chart  becoming invalid            
                delete pTexFact;
            }
            return true;
        }
        else
        {
            m_chart_texfactory_hash.erase(ittf);
            return false;
        }
    }
    return false;
}

bool glTextureManager::TextureCrunch(double factor)
{
    
    double hysteresis = 0.90;

    bool bGLMemCrunch = g_tex_mem_used > (double)(g_GLOptions.m_iTextureMemorySize * 1024 * 1024) * factor;
    if( ! bGLMemCrunch )
        return false;
    
    
    ChartPathHashTexfactType::iterator it0;
    for( it0 = m_chart_texfactory_hash.begin(); it0 != m_chart_texfactory_hash.end(); ++it0 ) {
        glTexFactory *ptf = it0.value();
        if(!ptf) continue;
        QString chart_full_path = ptf->GetChartPath();
        
        bGLMemCrunch = g_tex_mem_used > (double)(g_GLOptions.m_iTextureMemorySize * 1024 * 1024) * factor *hysteresis;
        if(!bGLMemCrunch) break;

               // For each canvas
//        for(unsigned int i=0 ; i < g_canvasArray.count() ; i++){
            ChartCanvas *cc = /*g_canvasArray.at(i)*/gFrame->GetPrimaryCanvas();
            if(cc){ 
                if( cc->GetVP().b_quilt )          // quilted
                {
                        if( cc->m_pQuilt->IsComposed() &&
                            !cc->m_pQuilt->IsChartInQuilt( chart_full_path ) ) {
                            ptf->DeleteSomeTextures( g_GLOptions.m_iTextureMemorySize * 1024 * 1024 * factor *hysteresis);
                            }
                }
                else      // not quilted
                {
                    if(cc->m_singleChart->GetFullPath() != chart_full_path)
                    {
                        ptf->DeleteSomeTextures( g_GLOptions.m_iTextureMemorySize * 1024 * 1024 * factor  *hysteresis);
                    }
                }
            }
//        }
    }
    
    return true;
}

#define MAX_CACHE_FACTORY 50
bool glTextureManager::FactoryCrunch(double factor)
{
    if (m_chart_texfactory_hash.size() == 0) {
        /* nothing to free */
        return false;
    }

    int mem_used, mem_start;
    zchxFuncUtil::getMemoryStatus(0, &mem_used);
    double hysteresis = 0.90;
    mem_start = mem_used;
    ChartPathHashTexfactType::iterator it0;

    bool bMemCrunch = ( g_memCacheLimit && ( (mem_used > (double)(g_memCacheLimit) * factor *hysteresis && 
                       mem_used > (double)(m_prevMemUsed) * factor *hysteresis)
                      || (m_chart_texfactory_hash.size() > MAX_CACHE_FACTORY)));
    
    if(!bMemCrunch)
        return false;
        
    //  Need more, so delete the oldest factory
    //      Find the oldest unused factory
    int lru_oldest = 2147483647;
    glTexFactory *ptf_oldest = NULL;
        
    for( it0 = m_chart_texfactory_hash.begin(); it0 != m_chart_texfactory_hash.end(); ++it0 ) {
        glTexFactory *ptf = it0.value();
        if(!ptf)
            continue;
        QString chart_full_path = ptf->GetChartPath();
        
        // we better have to find one because glTexFactory keep cache texture open
        // and ocpn will eventually run out of file descriptors
        
        // For each canvas
//        for(unsigned int i=0 ; i < g_canvasArray.count() ; i++){
            ChartCanvas *cc = /*g_canvasArray.at(i);*/gFrame->GetPrimaryCanvas();
            if(cc){
                
                if( cc->GetVP().b_quilt )          // quilted
                {
                    if( cc->m_pQuilt->IsComposed() &&
                        !cc->m_pQuilt->IsChartInQuilt( chart_full_path ) ) {
                
                        int lru = ptf->GetLRUTime();
                        if(lru < lru_oldest && !ptf->BackgroundCompressionAsJob()){
                            lru_oldest = lru;
                            ptf_oldest = ptf;
                        }
                    }
                } else {
                    if( cc->m_singleChart->GetFullPath() != chart_full_path) {
                        int lru = ptf->GetLRUTime();
                        if(lru < lru_oldest && !ptf->BackgroundCompressionAsJob()){
                            lru_oldest = lru;
                            ptf_oldest = ptf;
                        }
                    }
                }
            }
//        }
    }
                    
    //      Found one?
    if(!ptf_oldest)
        return false;

    ptf_oldest->FreeSome( g_memCacheLimit * factor * hysteresis);

    zchxFuncUtil::getMemoryStatus(0, &mem_used);

    bMemCrunch = ( g_memCacheLimit && ( (mem_used > (double)(g_memCacheLimit) * factor *hysteresis && 
                            mem_used > (double)(m_prevMemUsed) * factor *hysteresis)
                            || (m_chart_texfactory_hash.size() > MAX_CACHE_FACTORY)));
    
    if(!bMemCrunch)
        return false;
    
    //  Need more, so delete the oldest chart too
        
    m_chart_texfactory_hash.remove(ptf_oldest->GetHashKey());                // This chart  becoming invalid
                
    delete ptf_oldest;
    
    return true;
}

void glTextureManager::BuildCompressedCache()
{
    idx_sorted_by_distance.clear();

    // Building the cache may take a long time....
    // Be a little smarter.
    // Build a sorted array of chart database indices, sorted on distance from the ownship currently.
    // This way, a user may build a few charts textures for immediate use, then "skip" out on the rest until later.
    int count = 0;
    for(int i = 0; i<ChartData->GetChartTableEntries(); i++) {
        /* skip if not kap */
        const ChartTableEntry &cte = ChartData->GetChartTableEntry(i);
        ChartTypeEnum chart_type = (ChartTypeEnum)cte.GetChartType();
        if(chart_type == CHART_TYPE_PLUGIN){
            if(cte.GetChartFamily() != CHART_FAMILY_RASTER)
                continue;
        }
        else{
            if(chart_type != CHART_TYPE_KAP)
                continue;
        }
        
        QString CompressedCacheFilePath = CompressedCachePath(ChartData->GetDBChartFileName(i));
//        wxFileName fn(CompressedCacheFilePath);
        //        if(fn.FileExists()) /* skip if file exists */
        //            continue;

        idx_sorted_by_distance.Append(i);

        count++;
    }

    if(count == 0)
        return;

    qDebug("BuildCompressedCache() count = %d", count);

    if(m_timer)m_timer->stop();
    PurgeJobList();
    if (GetRunningJobCount()) {

        qDebug("Starting compressor pool drain");
        time_t stall = QDateTime::currentDateTime().toTime_t();
        #define THREAD_WAIT_SECONDS 5
        time_t end = stall + THREAD_WAIT_SECONDS;

        int n_comploop = 0;
        while(stall < end ) {
            stall = QDateTime::currentDateTime().toTime_t();;
            qDebug("Time: %d  Job Count: %d", n_comploop, GetRunningJobCount());
            if(!GetRunningJobCount()) break;
            QThread::sleep(1);
            n_comploop++;
        }

        qDebug("Finished compressor pool drain..Time: %d  Job Count: %d", n_comploop, GetRunningJobCount());
    }
    ClearAllRasterTextures();
    b_inCompressAllCharts = true;

    //  Build another array of sorted compression targets.
    //  We need to do this, as the chart table will not be invariant
    //  after the compression threads start, so our index array will be invalid.

    ArrayOfCompressTargets ct_array;
    for(unsigned int j = 0; j<idx_sorted_by_distance.count(); j++) {
        int i = idx_sorted_by_distance[j];

        const ChartTableEntry &cte = ChartData->GetChartTableEntry(i);
        double distance = chart_dist(i);

        QString filename = QString::fromUtf8(cte.GetpFullPath());

        compress_target *pct = new compress_target;
        pct->distance = distance;
        pct->chart_path = filename;

        ct_array.append(pct);
    }

    QString msg0;
    msg0 = ("                                                                               \n  \n  ");

    for(int i=0 ; i < m_max_jobs+1 ; i++)
        msg0 += ("\n                                             ");

    m_progDialog = new QProgressDialog();

    QFont qFont = FontMgr::Get().getSacledFontDefaultSize("Dialog");
    int fontSize = qFont.pointSize();
    QFont sFont;
    QSize csz = gFrame->rect().size();
    if(csz.width() < 500 || csz.height() < 500)
        sFont = FontMgr::Get().FindOrCreateFont( 10, "Microsoft YH", QFont::StyleNormal, QFont::Normal);
    else
        sFont = FontMgr::Get().FindOrCreateFont( fontSize, "Microsoft YH", QFont::StyleNormal, QFont::Normal);
    
    m_progDialog->setFont(sFont );
    
    //  Should we use "compact" screen layout?
    QFontMetrics fm(sFont);
    int height = fm.height();
    int width = fm.width("[WWWWWWWWWWWWWWWWWWWWWWWWWWWWWW]");
    if(width > (csz.width() / 2)) m_bcompact = true;

    m_progDialog->setWindowTitle(tr("OpenCPN Compressed Cache Update"));
    m_progDialog->setLabelText(msg0);
    m_progDialog->setMaximum(count+1);
    m_progDialog->setMinimum(0);

    //    Make sure the dialog is big enough to be readable
    m_progDialog->hide();
    QSize sz = m_progDialog->size();
    sz.setWidth(csz.width() * 9 / 10);
    m_progDialog->resize(sz );
    m_progDialog->show();
    m_progDialog->raise();

    m_skipout = false;
    m_skip = false;
    int yield = 0;

    for( m_jcnt = 0; m_jcnt<ct_array.count(); m_jcnt++) {

        QString filename = ct_array[m_jcnt]->chart_path;
        QString CompressedCacheFilePath = CompressedCachePath(filename);
        double distance = ct_array[m_jcnt]->distance;

        ChartBase *pchart = ChartData->OpenChartFromDBAndLock( filename, FULL_INIT );
        if(!pchart) /* probably a corrupt chart */
            continue;

        // bad things if more than one texfactory for a chart
        g_glTextureManager->PurgeChartTextures( pchart, true );

        ChartBaseBSB *pBSBChart = dynamic_cast<ChartBaseBSB*>( pchart );
        if(pBSBChart == 0)
            continue;
            
        glTexFactory *tex_fact = new glTexFactory(pchart, g_raster_format);

        m_progMsg.sprintf("Distance from Ownship:  %4.0f NMi\n", distance);
        m_progMsg.insert(0, ("Preparing RNC Cache...\n"));

        if(m_skipout) {
            g_glTextureManager->PurgeJobList();
            ChartData->DeleteCacheChart(pchart);
            delete tex_fact;
            break;
        }

        int size_X = pBSBChart->GetSize_X();
        int size_Y = pBSBChart->GetSize_Y();

        int tex_dim = g_GLOptions.m_iTextureDimension;

        int nx_tex = ceil( (float)size_X / tex_dim );
        int ny_tex = ceil( (float)size_Y / tex_dim );

        int nt = ny_tex * nx_tex;

        QRect rect(0, 0, tex_dim, tex_dim);
        for( int y = 0; y < ny_tex; y++ ) {
            for( int x = 0; x < nx_tex; x++ ) {
                for(int level = 0; level < g_mipmap_max_level + 1; level++ ) {
                    if(!tex_fact->IsLevelInCache( level, rect, global_color_scheme )){
                        goto schedule;
                    }
                }
                rect.setLeft( rect.left() + rect.width());
            }
            rect.setTop(rect.top() + rect.height());
        }
        //  Nothing to do
        //  Free all possible memory
        ChartData->DeleteCacheChart(pchart);
        delete tex_fact;
        yield++;
        if (yield == 200) {
            QThread::yieldCurrentThread();
            yield = 0;
            m_progDialog->setValue(m_jcnt);
        }
        continue;

        // some work to do
        schedule:

        yield = 0;
        ScheduleJob(tex_fact, QRect(), 0, false, true, true, false);
        while(!m_skip) {
            QThread::yieldCurrentThread();
            int cnt = GetJobCount() - GetRunningJobCount();
            if(!cnt)
                break;
            QThread::sleep(1);
        }

        if(m_skipout) {
            g_glTextureManager->PurgeJobList();
            ChartData->DeleteCacheChart(pchart);
            delete tex_fact;
            break;
        }
    }
    
    while(GetRunningJobCount()) {
        QThread::sleep(1);
        QThread::yieldCurrentThread();
    }
    
    b_inCompressAllCharts = false;
    m_timer->start(500);
    
    delete m_progDialog;
    m_progDialog = nullptr;
}

