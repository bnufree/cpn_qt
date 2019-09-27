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
#include "dychart.h"
#include "OCPNPlatform.h"

#include "s52s57.h"
#include "s52plib.h"

#include "s57chart.h"

#include "mygeom.h"
#include "cutil.h"
#include "georef.h"
//#include "navutil.h"                            // for LogMessageOnce
#include "ocpn_pixel.h"
#include "ocpndc.h"
#include "s52utils.h"
#include "wx28compat.h"
#include "ChartDataInputStream.h"

#include "gdal/cpl_csv.h"
#include "setjmp.h"

#include "ogr_s57.h"
#include "zchxmapmainwindow.h"
#include "ocpn_plugin.h"

//#include "pluginmanager.h"                      // for S57 lights overlay

#include "Osenc.h"
#include "chcanv.h"
#include "SencManager.h"
#include <QDebug>
#include "chcanv.h"

#ifdef __MSVC__
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#include <algorithm>          // for std::sort
#include <map>

#include "ssl_sha1/sha1.h"

#ifdef __MSVC__
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#endif


extern bool GetDoubleAttr(S57Obj *obj, const char *AttrName, double &val);      // found in s52cnsy

void OpenCPN_OGRErrorHandler( CPLErr eErrClass, int nError,
                              const char * pszErrorMsg );               // installed GDAL OGR library error handler


#ifdef ocpnUSE_GL
extern PFNGLGENBUFFERSPROC                 s_glGenBuffers;
extern PFNGLBINDBUFFERPROC                 s_glBindBuffer;
extern PFNGLBUFFERDATAPROC                 s_glBufferData;
extern PFNGLDELETEBUFFERSPROC              s_glDeleteBuffers;
#endif


extern s52plib           *ps52plib;
extern S57ClassRegistrar *g_poRegistrar;
extern QString          g_csv_locn;
extern QString          g_SENCPrefix;
extern FILE              *s_fpdebug;
extern bool              g_bGDAL_Debug;
extern bool              g_bDebugS57;
extern zchxMapMainWindow*          gFrame;
//extern PlugInManager     *g_pi_manager;
extern bool              g_b_overzoom_x;
extern bool              g_b_EnableVBO;
extern SENCThreadManager *g_SencThreadManager;
extern ColorScheme       global_color_scheme;
extern int               g_nCPUCount;

extern int                      g_SENC_LOD_pixels;
extern ChartFrameWork          *gChartFrameWork;


static jmp_buf env_ogrf;                    // the context saved by setjmp();

#define S57_THUMB_SIZE  200

static int              s_bInS57;         // Exclusion flag to prvent recursion in this class init call.
// Init() is not reentrant due to static wxProgressDialog callback....
int s_cnt;

static bool s_ProgressCallBack( void )
{
    bool ret = true;
    s_cnt++;
    if( ( s_cnt % 100 ) == 0 ) {
    }
    return ret;
}

static uint64_t hash_fast64(const void *buf, size_t len, uint64_t seed)
{
    const uint64_t    m = 0x880355f21e6d1965ULL;
    const uint64_t *pos = (const uint64_t *)buf;
    const uint64_t *end = pos + (len >> 3);
    const unsigned char *pc;
    uint64_t h = len * m ^ seed;
    uint64_t v;
    while (pos != end) {
        v = *pos++;
        v ^= v >> 23;
        v *= 0x2127599bf4325c37ULL;
        h ^= v ^ (v >> 47);
        h *= m;
    }
    pc = (const unsigned char*)pos;
    v = 0;
    switch (len & 7) {
    case 7: v ^= (uint64_t)pc[6]<<48;    // FALL THROUGH
    case 6: v ^= (uint64_t)pc[5]<<40;    // FALL THROUGH
    case 5: v ^= (uint64_t)pc[4]<<32;    // FALL THROUGH
    case 4: v ^= (uint64_t)pc[3]<<24;    // FALL THROUGH
    case 3: v ^= (uint64_t)pc[2]<<16;    // FALL THROUGH
    case 2: v ^= (uint64_t)pc[1]<<8;     // FALL THROUGH
    case 1: v ^= (uint64_t)pc[0];
        v ^= v >> 23;
        v *= 0x2127599bf4325c37ULL;
        h ^= v ^ (v >> 47);
        h *= m;
    }

    h ^= h >> 23;
    h *= 0x2127599bf4325c37ULL;
    h ^= h >> 47;
    return h;
}

static unsigned int hash_fast32(const void *buf, size_t len, unsigned int seed)
{
    uint64_t h = hash_fast64(buf, len, seed);
    /* The following trick converts the 64-bit hashcode to a
     * residue over a Fermat Number, in which information from
     * both the higher and lower parts of hashcode shall be
     * retained. */
    return h - (h >> 32);
}

unsigned long connector_key::hash() const
{
    return hash_fast32(k, sizeof k, 0);
}


//----------------------------------------------------------------------------------
//      render_canvas_parms Implementation
//----------------------------------------------------------------------------------

render_canvas_parms::render_canvas_parms()
{
    pix_buff = NULL;
}


render_canvas_parms::~render_canvas_parms( void )
{
}

//----------------------------------------------------------------------------------
//      s57chart Implementation
//----------------------------------------------------------------------------------

s57chart::s57chart()
{

    m_ChartType = CHART_TYPE_S57;
    m_ChartFamily = CHART_FAMILY_VECTOR;

    for( int i = 0; i < PRIO_NUM; i++ )
        for( int j = 0; j < LUPNAME_NUM; j++ )
            razRules[i][j] = NULL;

    m_Chart_Scale = 1;                              // Will be fetched during Init()
    m_Chart_Skew = 0.0;

    // Create ATON arrays, needed by S52PLIB
    pFloatingATONArray = new wxArrayPtrVoid;
    pRigidATONArray = new wxArrayPtrVoid;

    m_tmpup_array = NULL;

    m_DepthUnits = ("METERS");
    m_depth_unit_id = DEPTH_UNIT_METERS;

    bGLUWarningSent = false;

    m_nvaldco = 0;
    m_nvaldco_alloc = 0;
    m_pvaldco_array = NULL;

    m_bExtentSet = false;

    m_pDIBThumbDay = NULL;
    m_pDIBThumbDim = NULL;
    m_pDIBThumbOrphan = NULL;
    m_bbase_file_attr_known = false;

    m_bLinePrioritySet = false;
    m_plib_state_hash = 0;

    m_btex_mem = false;

    ref_lat = 0.0;
    ref_lon = 0.0;

    m_b2pointLUPS = false;
    m_b2lineLUPS = false;

    m_next_safe_cnt = 1e6;
    m_LineVBO_name = -1;
    m_line_vertex_buffer = 0;
    m_this_chart_context =  0;
    m_Chart_Skew = 0;
    m_vbo_byte_length = 0;
    m_SENCthreadStatus = THREAD_INACTIVE;
    bReadyToRender = false;
    m_RAZBuilt = false;
    m_disableBackgroundSENC = true;
}

s57chart::~s57chart()
{

    FreeObjectsAndRules();

    //    delete pFullPath;

    delete pFloatingATONArray;
    delete pRigidATONArray;

    free( m_pvaldco_array );

    free( m_line_vertex_buffer );

    delete m_pDIBThumbOrphan;

    for (unsigned i=0; i<m_pcs_vector.size(); i++)
        delete m_pcs_vector.at(i);

    for (unsigned i=0; i<m_pve_vector.size(); i++)
        delete m_pve_vector.at(i);
    
    m_pcs_vector.clear();
    m_pve_vector.clear();

    for( VE_Hash::iterator it = m_ve_hash.begin(); it != m_ve_hash.end(); ++it ) {
        VE_Element *pedge = it.value();
        if(pedge){
            free(pedge->pPoints);
            delete pedge;
        }
    }
    m_ve_hash.clear();

    for( VC_Hash::iterator itc = m_vc_hash.begin(); itc != m_vc_hash.end(); ++itc ) {
        VC_Element *pcs = itc.value();
        if(pcs) {
            free(pcs->pPoint);
            delete pcs;
        }
    }
    m_vc_hash.clear();

#ifdef ocpnUSE_GL
    if(s_glDeleteBuffers && (m_LineVBO_name > 0))
        s_glDeleteBuffers(1, (GLuint *)&m_LineVBO_name);
#endif
    free (m_this_chart_context);

    if(m_TempFilePath.length() && (m_FullPath != m_TempFilePath)){
        if(QFile::exists(m_TempFilePath) )
            QFile::remove(m_TempFilePath);
    }

    //  Check the SENCThreadManager to see if this chart is queued or active
    if(g_SencThreadManager){
        if(g_SencThreadManager->IsChartInTicketlist(this)){
            g_SencThreadManager->SetChartPointer(this, NULL);
        }
    }

}

void s57chart::GetValidCanvasRegion( const ViewPort& VPoint, OCPNRegion *pValidRegion )
{
    int rxl, rxr;
    int ryb, ryt;
    double easting, northing;
    double epix, npix;

    toSM( m_FullExtent.SLAT, m_FullExtent.WLON, VPoint.lat(), VPoint.lon(), &easting, &northing );
    epix = easting * VPoint.viewScalePPM();
    npix = northing * VPoint.viewScalePPM();

    rxl = (int) round((VPoint.pixWidth() / 2) + epix);
    ryb = (int) round((VPoint.pixHeight() / 2) - npix);

    toSM( m_FullExtent.NLAT, m_FullExtent.ELON, VPoint.lat(), VPoint.lon(), &easting, &northing );
    epix = easting * VPoint.viewScalePPM();
    npix = northing * VPoint.viewScalePPM();

    rxr = (int) round((VPoint.pixWidth() / 2) + epix);
    ryt = (int) round((VPoint.pixHeight() / 2) - npix);

    pValidRegion->Clear();
    pValidRegion->Union( rxl, ryt, rxr - rxl, ryb - ryt );
}

LLRegion s57chart::GetValidRegion()
{
    double ll[8] = {m_FullExtent.SLAT, m_FullExtent.WLON,
                    m_FullExtent.SLAT, m_FullExtent.ELON,
                    m_FullExtent.NLAT, m_FullExtent.ELON,
                    m_FullExtent.NLAT, m_FullExtent.WLON};
    return LLRegion(4, ll);
}

void s57chart::SetColorScheme( ColorScheme cs, bool bApplyImmediate )
{
    if( !ps52plib ) return;
    //  Here we convert (subjectively) the Global ColorScheme
    //  to an appropriate S52 Color scheme, by name.

    switch( cs ){
    case GLOBAL_COLOR_SCHEME_DAY:
        ps52plib->SetPLIBColorScheme(("DAY") );
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        ps52plib->SetPLIBColorScheme(("DUSK") );
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        ps52plib->SetPLIBColorScheme(("NIGHT") );
        break;
    default:
        ps52plib->SetPLIBColorScheme(("DAY") );
        break;
    }

    m_global_color_scheme = cs;

    if( bApplyImmediate ) {
        pDIB.reset();        // Toss any current cache
    }

    //      Clear out any cached bitmaps in the text cache
    ClearRenderedTextCache();

    //      Setup the proper thumbnail bitmap pointer
    ChangeThumbColor(cs);

}

void s57chart::ChangeThumbColor(ColorScheme cs)
{
    if( 0 == m_pDIBThumbDay )
        return;

    switch( cs ){
    default:
    case GLOBAL_COLOR_SCHEME_DAY:
        pThumbData->pDIBThumb = m_pDIBThumbDay;
        m_pDIBThumbOrphan = m_pDIBThumbDim;
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
    case GLOBAL_COLOR_SCHEME_NIGHT: {
        if( NULL == m_pDIBThumbDim ) {
            QImage img = m_pDIBThumbDay->ConvertToImage();


            QImage gimg = img;

            //#ifdef ocpnUSE_ocpnBitmap
            //                      ocpnBitmap *pBMP =  new ocpnBitmap(gimg, m_pDIBThumbDay->GetDepth());
            //#else
            wxBitmap *pBMP = new wxBitmap( gimg );
            //#endif
            m_pDIBThumbDim = pBMP;
            m_pDIBThumbOrphan = m_pDIBThumbDay;
        }

        pThumbData->pDIBThumb = m_pDIBThumbDim;
        break;
    }
    }
}

bool s57chart::GetChartExtent( Extent *pext )
{
    if( m_bExtentSet ) {
        *pext = m_FullExtent;
        return true;
    } else
        return false;
}

static void free_mps(mps_container *mps)
{
    
    if ( mps == 0)
        return;
    if( ps52plib && mps->cs_rules ){
        for(unsigned int i=0 ; i < mps->cs_rules->count() ; i++){
            Rules *rule_chain_top = mps->cs_rules->at(i);
            ps52plib->DestroyRulesChain( rule_chain_top );
        }
        delete mps->cs_rules;
    }
    free( mps );
}

void s57chart::FreeObjectsAndRules()
{
    //      Delete the created ObjRazRules, including the S57Objs
    //      and any child lists
    //      The LUPs of base elements are deleted elsewhere ( void s52plib::DestroyLUPArray ( wxArrayOfLUPrec *pLUPArray ))
    //      But we need to manually destroy any LUPS related to children

    ObjRazRules *top;
    ObjRazRules *nxx;
    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {

            top = razRules[i][j];
            while( top != NULL ) {
                top->obj->nRef--;
                if( 0 == top->obj->nRef )
                    delete top->obj;

                if( top->child ) {
                    ObjRazRules *ctop = top->child;
                    while( ctop ) {
                        delete ctop->obj;

                        if( ps52plib ) ps52plib->DestroyLUP( ctop->LUP );
                        delete ctop->LUP;

                        ObjRazRules *cnxx = ctop->next;
                        delete ctop;
                        ctop = cnxx;
                    }
                }
                free_mps( top->mps );

                nxx = top->next;
                free( top );
                top = nxx;
            }
        }
    }
}

void s57chart::ClearRenderedTextCache()
{
    ObjRazRules *top;
    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            top = razRules[i][j];
            while( top != NULL ) {
                if( top->obj->bFText_Added ) {
                    top->obj->bFText_Added = false;
                    delete top->obj->FText;
                    top->obj->FText = NULL;
                }

                if( top->child ) {
                    ObjRazRules *ctop = top->child;
                    while( ctop ) {
                        if( ctop->obj->bFText_Added ) {
                            ctop->obj->bFText_Added = false;
                            delete ctop->obj->FText;
                            ctop->obj->FText = NULL;
                        }
                        ctop = ctop->next;
                    }
                }

                top = top->next;
            }
        }
    }
}

double s57chart::GetNormalScaleMin( double canvas_scale_factor, bool b_allow_overzoom )
{
    //    if( b_allow_overzoom )
    return m_Chart_Scale * 0.125;
    //    else
    //        return m_Chart_Scale * 0.25;
}
double s57chart::GetNormalScaleMax( double canvas_scale_factor, int canvas_width )
{
    return m_Chart_Scale * 4.0;

}

//-----------------------------------------------------------------------
//              Pixel to Lat/Long Conversion helpers
//-----------------------------------------------------------------------

void s57chart::GetPointPix( ObjRazRules *rzRules, float north, float east, zchxPoint *r )
{
    r->x = roundint(((east - m_easting_vp_center) * m_view_scale_ppm) + m_pixx_vp_center);
    r->y = roundint(m_pixy_vp_center - ((north - m_northing_vp_center) * m_view_scale_ppm));
}

void s57chart::GetPointPix( ObjRazRules *rzRules, zchxPointF *en, zchxPoint *r, int nPoints )
{
    for( int i = 0; i < nPoints; i++ ) {
        r[i].x =  roundint(((en[i].x - m_easting_vp_center) * m_view_scale_ppm) + m_pixx_vp_center);
        r[i].y =  roundint(m_pixy_vp_center - ((en[i].y - m_northing_vp_center) * m_view_scale_ppm));
    }
}

void s57chart::GetPixPoint( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
    if(vpt->projectType() != PROJECTION_MERCATOR)
        printf("s57chart unhandled projection\n");

    //    Use Mercator estimator
    int dx = pixx - ( vpt->pixWidth() / 2 );
    int dy = ( vpt->pixHeight() / 2 ) - pixy;

    double xp = ( dx * cos( vpt->skew() ) ) - ( dy * sin( vpt->skew() ) );
    double yp = ( dy * cos( vpt->skew() ) ) + ( dx * sin( vpt->skew() ) );

    double d_east = xp / vpt->viewScalePPM();
    double d_north = yp / vpt->viewScalePPM();

    double slat, slon;
    fromSM( d_east, d_north, vpt->lat(), vpt->lon(), &slat, &slon );

    *plat = slat;
    *plon = slon;

}

//-----------------------------------------------------------------------
//              Calculate and Set ViewPoint Constants
//-----------------------------------------------------------------------

void s57chart::SetVPParms( const ViewPort &vpt )
{
    //  Set up local SM rendering constants
    m_pixx_vp_center = vpt.pixWidth() / 2.0;
    m_pixy_vp_center = vpt.pixHeight() / 2.0;
    m_view_scale_ppm = vpt.viewScalePPM();

    toSM( vpt.lat(), vpt.lon(), ref_lat, ref_lon, &m_easting_vp_center, &m_northing_vp_center );

    vp_transform.easting_vp_center = m_easting_vp_center;
    vp_transform.northing_vp_center = m_northing_vp_center;
}

bool s57chart::AdjustVP( ViewPort &vp_last, ViewPort &vp_proposed )
{
    if( IsCacheValid() ) {

        //      If this viewpoint is same scale as last...
        if( vp_last.viewScalePPM() == vp_proposed.viewScalePPM() ) {

            double prev_easting_c, prev_northing_c;
            toSM( vp_last.lat(), vp_last.lon(), ref_lat, ref_lon, &prev_easting_c, &prev_northing_c );

            double easting_c, northing_c;
            toSM( vp_proposed.lat(), vp_proposed.lon(), ref_lat, ref_lon, &easting_c, &northing_c );

            //  then require this viewport to be exact integral pixel difference from last
            //  adjusting lat()/lat() and SM accordingly

            double delta_pix_x = ( easting_c - prev_easting_c ) * vp_proposed.viewScalePPM();
            int dpix_x = (int) round ( delta_pix_x );
            double dpx = dpix_x;

            double delta_pix_y = ( northing_c - prev_northing_c ) * vp_proposed.viewScalePPM();
            int dpix_y = (int) round ( delta_pix_y );
            double dpy = dpix_y;

            double c_east_d = ( dpx / vp_proposed.viewScalePPM() ) + prev_easting_c;
            double c_north_d = ( dpy / vp_proposed.viewScalePPM() ) + prev_northing_c;

            double xlat, xlon;
            fromSM( c_east_d, c_north_d, ref_lat, ref_lon, &xlat, &xlon );

            vp_proposed.setLon(xlon);
            vp_proposed.setLat(xlat);

            return true;
        }
    }

    return false;
}

/*
 bool s57chart::IsRenderDelta(ViewPort &vp_last, ViewPort &vp_proposed)
 {
 double last_center_easting, last_center_northing, this_center_easting, this_center_northing;
 toSM ( vp_proposed.lat(), vp_proposed.lon(), ref_lat, ref_lon, &this_center_easting, &this_center_northing );
 toSM ( vp_last.lat(),     vp_last.lon(),     ref_lat, ref_lon, &last_center_easting, &last_center_northing );

 int dx = (int)round((last_center_easting  - this_center_easting)  * vp_proposed.viewScalePPM());
 int dy = (int)round((last_center_northing - this_center_northing) * vp_proposed.viewScalePPM());

 return((dx !=  0) || (dy != 0) || !(IsCacheValid()) || (vp_proposed.viewScalePPM() != vp_last.viewScalePPM()));
 }
 */

void  s57chart::LoadThumb()
{
    QFileInfo fn( m_FullPath );
    QString SENCdir = g_SENCPrefix;

    if( SENCdir.right(1) != zchxFuncUtil::separator() ) SENCdir.append( zchxFuncUtil::separator() );

    QString tsfn( SENCdir );
    tsfn.append(fn.fileName() );
    QString newName = zchxFuncUtil::getNewFileNameWithExt(tsfn, "BMP");
    QFileInfo ThumbFileNameLook( newName );

    wxBitmap *pBMP;
    if( ThumbFileNameLook.exists() ) {
        pBMP = new wxBitmap;

        pBMP->LoadFile( ThumbFileNameLook.absoluteFilePath());
        m_pDIBThumbDay = pBMP;
        m_pDIBThumbOrphan = 0;
        m_pDIBThumbDim = 0;

    }
}


ThumbData *s57chart::GetThumbData( int tnx, int tny, float lat, float lon )
{
    //  Plot the passed lat/lon at the thumbnail bitmap scale
    //  Using simple linear algorithm.
    if( pThumbData->pDIBThumb == 0) {
        LoadThumb();
        ChangeThumbColor(m_global_color_scheme);
    }

    UpdateThumbData( lat, lon );

    return pThumbData;
}

bool s57chart::UpdateThumbData( double lat, double lon )
{
    //  Plot the passed lat/lon at the thumbnail bitmap scale
    //  Using simple linear algorithm.
    int test_x, test_y;
    if( pThumbData->pDIBThumb ) {
        double lat_top = m_FullExtent.NLAT;
        double lat_bot = m_FullExtent.SLAT;
        double lon_left = m_FullExtent.WLON;
        double lon_right = m_FullExtent.ELON;

        // Build the scale factors just as the thumbnail was built
        double ext_max = fmax((lat_top - lat_bot), (lon_right - lon_left));

        double thumb_view_scale_ppm = ( S57_THUMB_SIZE / ext_max ) / ( 1852 * 60 );
        double east, north;
        toSM( lat, lon, ( lat_top + lat_bot ) / 2., ( lon_left + lon_right ) / 2., &east, &north );

        test_x = pThumbData->pDIBThumb->GetWidth() / 2 + (int) ( east * thumb_view_scale_ppm );
        test_y = pThumbData->pDIBThumb->GetHeight() / 2 - (int) ( north * thumb_view_scale_ppm );

    } else {
        test_x = 0;
        test_y = 0;
    }

    if( ( test_x != pThumbData->ShipX ) || ( test_y != pThumbData->ShipY ) ) {
        pThumbData->ShipX = test_x;
        pThumbData->ShipY = test_y;
        return true;
    } else
        return false;
}

void s57chart::SetFullExtent( Extent& ext )
{
    m_FullExtent.NLAT = ext.NLAT;
    m_FullExtent.SLAT = ext.SLAT;
    m_FullExtent.WLON = ext.WLON;
    m_FullExtent.ELON = ext.ELON;

    m_bExtentSet = true;
}

void s57chart::ForceEdgePriorityEvaluate( void )
{
    m_bLinePrioritySet = false;
}

void s57chart::SetLinePriorities( void )
{
    if( !ps52plib ) return;

    //      If necessary.....
    //      Establish line feature rendering priorities

    if( !m_bLinePrioritySet ) {
        ObjRazRules *top;
        ObjRazRules *crnt;

        for( int i = 0; i < PRIO_NUM; ++i ) {

            top = razRules[i][2];           //LINES
            while( top != NULL ) {
                ObjRazRules *crnt = top;
                top = top->next;
                ps52plib->SetLineFeaturePriority( crnt, i );
            }

            //    In the interest of speed, choose only the one necessary area boundary style index
            int j;
            if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
                j = 4;
            else
                j = 3;

            top = razRules[i][j];
            while( top != NULL ) {
                crnt = top;
                top = top->next;               // next object
                ps52plib->SetLineFeaturePriority( crnt, i );
            }

        }


        // Traverse the entire object list again, setting the priority of each line_segment_element
        // to the maximum priority seen for that segment
        for( int i = 0; i < PRIO_NUM; ++i ) {
            for( int j = 0; j < LUPNAME_NUM; j++ ) {
                ObjRazRules *top = razRules[i][j];
                while( top != NULL ) {
                    S57Obj *obj = top->obj;

                    VE_Element *pedge;
                    connector_segment *pcs;
                    line_segment_element *list = obj->m_ls_list;
                    while( list ){
                        switch (list->ls_type){
                        case TYPE_EE:
                        case TYPE_EE_REV:
                            pedge = list->pedge;// (VE_Element *)list->private0;
                            if(pedge)
                                list->priority = pedge->max_priority;
                            break;

                        default:
                            pcs = list->pcs; //(connector_segment *)list->private0;
                            if(pcs)
                                list->priority = pcs->max_priority_cs;
                            break;
                        }

                        list = list->next;
                    }

                    top = top->next;
                }
            }
        }
    }

    //      Mark the priority as set.
    //      Generally only reset by Options Dialog post processing
    m_bLinePrioritySet = true;
}

#if 0
void s57chart::SetLinePriorities( void )
{
    if( !ps52plib ) return;

    //      If necessary.....
    //      Establish line feature rendering priorities

    if( !m_bLinePrioritySet ) {
        ObjRazRules *top;
        ObjRazRules *crnt;

        for( int i = 0; i < PRIO_NUM; ++i ) {

            top = razRules[i][2];           //LINES
            while( top != NULL ) {
                ObjRazRules *crnt = top;
                top = top->next;
                ps52plib->SetLineFeaturePriority( crnt, i );
            }

            //    In the interest of speed, choose only the one necessary area boundary style index
            int j;
            if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
                j = 4;
            else
                j = 3;

            top = razRules[i][j];
            while( top != NULL ) {
                crnt = top;
                top = top->next;               // next object
                ps52plib->SetLineFeaturePriority( crnt, i );
            }

        }


        // Traverse the entire object list again, setting the priority of each line_segment_element
        // to the maximum priority seen for that segment
        for( int i = 0; i < PRIO_NUM; ++i ) {
            for( int j = 0; j < LUPNAME_NUM; j++ ) {
                ObjRazRules *top = razRules[i][j];
                while( top != NULL ) {
                    S57Obj *obj = top->obj;

                    VE_Element *pedge;
                    connector_segment *pcs;
                    line_segment_element *list = obj->m_ls_list;
                    while( list ){
                        switch (list->type){
                        case TYPE_EE:

                            pedge = (VE_Element *)list->private0;
                            if(pedge)
                                list->priority = pedge->max_priority;
                            break;

                        default:
                            pcs = (connector_segment *)list->private0;
                            if(pcs)
                                list->priority = pcs->max_priority;
                            break;
                        }

                        list = list->next;
                    }

                    top = top->next;
                }
            }
        }
    }

    //      Mark the priority as set.
    //      Generally only reset by Options Dialog post processing
    m_bLinePrioritySet = true;
}
#endif

int s57chart::GetLineFeaturePointArray(S57Obj *obj, void **ret_array)
{
    //  Walk the line segment list once to get the required array size

    int nPoints = 0;
    line_segment_element *ls_list = obj->m_ls_list;
    while( ls_list){
        if( (ls_list->ls_type == TYPE_EE) || (ls_list->ls_type == TYPE_EE_REV) )
            nPoints += ls_list->pedge->nCount;
        else
            nPoints += 2;
        ls_list = ls_list->next;
    }

    if(!nPoints){
        *ret_array = 0;
        return 0;
    }

    //  Allocate the buffer
    float *br = (float *)malloc(nPoints * 2 * sizeof(float));
    *ret_array = br;

    // populate the buffer
    unsigned char *source_buffer = (unsigned char *)GetLineVertexBuffer();
    ls_list = obj->m_ls_list;
    while( ls_list){
        size_t vbo_offset = 0;
        size_t count = 0;
        if( (ls_list->ls_type == TYPE_EE) || (ls_list->ls_type == TYPE_EE_REV) ){
            vbo_offset = ls_list->pedge->vbo_offset;
            count = ls_list->pedge->nCount;
        }
        else{
            vbo_offset = ls_list->pcs->vbo_offset;
            count = 2;
        }

        memcpy(br, source_buffer + vbo_offset, count * 2 * sizeof(float));
        br += count * 2;
        ls_list = ls_list->next;
    }

    return nPoints;

}


#if 0
int s57chart::GetLineFeaturePointArray(S57Obj *obj, void **ret_array)
{
    //  Walk the line segment list once to get the required array size

    int nPoints = 0;
    line_segment_element *ls_list = obj->m_ls_list;
    while( ls_list){
        nPoints += ls_list->n_points;
        ls_list = ls_list->next;
    }

    if(!nPoints){
        *ret_array = 0;
        return 0;
    }

    //  Allocate the buffer
    float *br = (float *)malloc(nPoints * 2 * sizeof(float));
    *ret_array = br;

    // populate the buffer
    unsigned char *source_buffer = (unsigned char *)GetLineVertexBuffer();
    ls_list = obj->m_ls_list;
    while( ls_list){
        memcpy(br, source_buffer + ls_list->vbo_offset, ls_list->n_points * 2 * sizeof(float));
        br += ls_list->n_points * 2;
        ls_list = ls_list->next;
    }

    return nPoints;

}
#endif

typedef struct segment_pair{
    float e0, n0, e1, n1;
}_segment_pair;


void s57chart::AssembleLineGeometry( void )
{
    // Walk the hash tables to get the required buffer size

    //  Start with the edge hash table
    size_t nPoints = 0;
    VE_Hash::iterator it;
    for( it = m_ve_hash.begin(); it != m_ve_hash.end(); ++it ) {
        VE_Element *pedge = it.value();
        if( pedge ) {
            nPoints += pedge->nCount;
        }
    }

    //    printf("time0 %f\n", sw.GetTime());



    std::map<long long, connector_segment *> ce_connector_hash;
    std::map<long long, connector_segment *> ec_connector_hash;
    std::map<long long, connector_segment *> cc_connector_hash;

    std::map<long long, connector_segment *>::iterator csit;
    
    int ndelta = 0;

    //  Define a vector to temporarily hold the geometry for the created pcs elements

    std::vector<segment_pair> connector_segment_vector;
    size_t seg_pair_index = 0;

    //  Get the end node connected segments.  To do this, we
    //  walk the Feature array and process each feature that potentially has a LINE type element
    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            ObjRazRules *top = razRules[i][j];
            while( top != NULL ) {
                S57Obj *obj = top->obj;

                if( (!obj->m_ls_list) && (obj->m_n_lsindex) )     // object has not been processed yet
                {
                    line_segment_element list_top;
                    list_top.next = 0;

                    line_segment_element *le_current = &list_top;

                    for( int iseg = 0; iseg < obj->m_n_lsindex; iseg++ ) {

                        if(!obj->m_lsindex_array)
                            continue;
                        
                        int seg_index = iseg * 3;
                        int *index_run = &obj->m_lsindex_array[seg_index];

                        //  Get first connected node
                        unsigned int inode = *index_run++;

                        //  Get the edge
                        bool edge_dir = true;
                        int venode = *index_run++;
                        if(venode < 0){
                            venode = -venode;
                            edge_dir = false;
                        }

                        VE_Element *pedge = 0;
                        if(venode){
                            if(m_ve_hash.find(venode) != m_ve_hash.end())
                                pedge = m_ve_hash[venode];
                        }

                        //  Get end connected node
                        unsigned int enode = *index_run++;

                        //  Get first connected node
                        VC_Element *ipnode = 0;
                        ipnode = m_vc_hash[inode];

                        //  Get end connected node
                        VC_Element *epnode = 0;
                        epnode = m_vc_hash[enode];


                        if( ipnode ) {
                            if(pedge && pedge->nCount)
                            {

                                //      The initial node exists and connects to the start of an edge

                                long long key = ((unsigned long long)inode<<32) + venode;
                                
                                connector_segment *pcs = NULL;
                                csit = ce_connector_hash.find( key );
                                if( csit == ce_connector_hash.end() ){
                                    ndelta += 2;
                                    pcs = new connector_segment;
                                    ce_connector_hash[key] = pcs;

                                    // capture and store geometry
                                    segment_pair pair;
                                    float *ppt = ipnode->pPoint;
                                    pair.e0 = *ppt++;
                                    pair.n0 = *ppt;

                                    if(edge_dir){
                                        pair.e1 = pedge->pPoints[ 0 ];
                                        pair.n1 = pedge->pPoints[ 1 ];
                                    }
                                    else{
                                        int last_point_index = (pedge->nCount -1) * 2;
                                        pair.e1 = pedge->pPoints[ last_point_index ];
                                        pair.n1 = pedge->pPoints[ last_point_index + 1 ];
                                    }

                                    connector_segment_vector.push_back(pair);
                                    pcs->vbo_offset = seg_pair_index;               // use temporarily
                                    seg_pair_index ++;

                                    // calculate the centroid of this connector segment, used for viz testing
                                    double lat, lon;
                                    fromSM( (pair.e0 + pair.e1)/2, (pair.n0 + pair.n1)/2, ref_lat, ref_lon, &lat, &lon );
                                    pcs->cs_lat_avg = lat;
                                    pcs->cs_lon_avg = lon;

                                }
                                else
                                    pcs = csit->second;


                                line_segment_element *pls = new line_segment_element;
                                pls->next = 0;
                                //                            pls->n_points = 2;
                                pls->priority = 0;
                                pls->pcs = pcs;
                                pls->ls_type = TYPE_CE;

                                le_current->next = pls;             // hook it up
                                le_current = pls;

                            }
                        }

                        if(pedge && pedge->nCount){
                            line_segment_element *pls = new line_segment_element;
                            pls->next = 0;
                            //                        pls->n_points = pedge->nCount;
                            pls->priority = 0;
                            pls->pedge = pedge;
                            pls->ls_type = TYPE_EE;
                            if( !edge_dir )
                                pls->ls_type = TYPE_EE_REV;


                            le_current->next = pls;             // hook it up
                            le_current = pls;

                        }   //pedge

                        // end node
                        if( epnode ) {

                            if(ipnode){
                                if(pedge && pedge->nCount){

                                    long long key = ((unsigned long long)venode<<32) + enode;
                                    
                                    connector_segment *pcs = NULL;
                                    csit = ec_connector_hash.find( key );
                                    if( csit == ec_connector_hash.end() ){
                                        ndelta += 2;
                                        pcs = new connector_segment;
                                        ec_connector_hash[key] = pcs;

                                        // capture and store geometry
                                        segment_pair pair;

                                        if(!edge_dir){
                                            pair.e0 = pedge->pPoints[ 0 ];
                                            pair.n0 = pedge->pPoints[ 1 ];
                                        }
                                        else{
                                            int last_point_index = (pedge->nCount -1) * 2;
                                            pair.e0 = pedge->pPoints[ last_point_index ];
                                            pair.n0 = pedge->pPoints[ last_point_index + 1 ];
                                        }


                                        float *ppt = epnode->pPoint;
                                        pair.e1 = *ppt++;
                                        pair.n1 = *ppt;

                                        connector_segment_vector.push_back(pair);
                                        pcs->vbo_offset = seg_pair_index;               // use temporarily
                                        seg_pair_index ++;

                                        // calculate the centroid of this connector segment, used for viz testing
                                        double lat, lon;
                                        fromSM( (pair.e0 + pair.e1)/2, (pair.n0 + pair.n1)/2, ref_lat, ref_lon, &lat, &lon );
                                        pcs->cs_lat_avg = lat;
                                        pcs->cs_lon_avg = lon;

                                    }
                                    else
                                        pcs = csit->second;

                                    line_segment_element *pls = new line_segment_element;
                                    pls->next = 0;
                                    pls->priority = 0;
                                    pls->pcs = pcs;
                                    pls->ls_type = TYPE_EC;

                                    le_current->next = pls;             // hook it up
                                    le_current = pls;


                                }
                                else {
                                    long long key = ((unsigned long long)inode<<32) + enode;
                                    
                                    connector_segment *pcs = NULL;
                                    csit = cc_connector_hash.find( key );
                                    if( csit == cc_connector_hash.end() ){
                                        ndelta += 2;
                                        pcs = new connector_segment;
                                        cc_connector_hash[key] = pcs;

                                        // capture and store geometry
                                        segment_pair pair;

                                        float *ppt = ipnode->pPoint;
                                        pair.e0 = *ppt++;
                                        pair.n0 = *ppt;

                                        ppt = epnode->pPoint;
                                        pair.e1 = *ppt++;
                                        pair.n1 = *ppt;

                                        connector_segment_vector.push_back(pair);
                                        pcs->vbo_offset = seg_pair_index;               // use temporarily
                                        seg_pair_index ++;

                                        // calculate the centroid of this connector segment, used for viz testing
                                        double lat, lon;
                                        fromSM( (pair.e0 + pair.e1)/2, (pair.n0 + pair.n1)/2, ref_lat, ref_lon, &lat, &lon );
                                        pcs->cs_lat_avg = lat;
                                        pcs->cs_lon_avg = lon;

                                    }
                                    else
                                        pcs = csit->second;

                                    line_segment_element *pls = new line_segment_element;
                                    pls->next = 0;
                                    pls->priority = 0;
                                    pls->pcs = pcs;
                                    pls->ls_type = TYPE_CC;

                                    le_current->next = pls;             // hook it up
                                    le_current = pls;


                                }
                            }
                        }


                    }  // for

                    //  All done, so assign the list to the object
                    obj->m_ls_list = list_top.next;    // skipping the empty first placeholder element

                    //  Rarely, some objects are improperly coded, e.g. cm93
                    //  If found, signal this downstream for NIL processing
                    if(obj->m_ls_list == NULL){
                        obj->m_n_lsindex = 0;
                    }

                    // we are all finished with the line segment index array, per object
                    free(obj->m_lsindex_array);
                    obj->m_lsindex_array = NULL;
                }
                
                top = top->next;
            }
        }
    }
    //    printf("time1 %f\n", sw.GetTime());

    //  We have the total VBO point count, and a nice hashmap of the connector segments
    nPoints += ndelta;          // allow for the connector segments

    size_t vbo_byte_length = 2 * nPoints * sizeof(float);
    
    unsigned char *buffer_offset;
    size_t offset;
    
    if(0 == m_vbo_byte_length){
        m_line_vertex_buffer = (float *)malloc( vbo_byte_length);
        m_vbo_byte_length = vbo_byte_length;
        buffer_offset = (unsigned char *)m_line_vertex_buffer;
        offset = 0;
    }
    else{
        m_line_vertex_buffer = (float *)realloc( m_line_vertex_buffer, m_vbo_byte_length + vbo_byte_length );
        buffer_offset = (unsigned char *)m_line_vertex_buffer + m_vbo_byte_length;
        offset = m_vbo_byte_length;
        m_vbo_byte_length = m_vbo_byte_length + vbo_byte_length;
    }

    float *lvr = (float *)buffer_offset;
    
    //      Copy and edge points as floats,
    //      and recording each segment's offset in the array
    for( it = m_ve_hash.begin(); it != m_ve_hash.end(); ++it ) {
        VE_Element *pedge = it.value();
        int key = it.key();
        if( pedge ) {
            memcpy(lvr, pedge->pPoints, pedge->nCount * 2 * sizeof(float));
            lvr += pedge->nCount * 2;

            pedge->vbo_offset = offset;
            offset += pedge->nCount * 2 * sizeof(float);
        }
        //         else
        //             int yyp = 4;        //TODO Why are zero elements being inserted into m_ve_hash?
    }

    //      Now iterate on the hashmaps, adding the connector segments in the temporary vector to the VBO buffer
    //      At the  same time, populate a vector, storing the pcs pointers to allow destruction at this class dtor.
    //      This will allow us to destroy (automatically) the pcs hashmaps, and save some storage


    for( csit = ce_connector_hash.begin(); csit != ce_connector_hash.end(); ++csit )
    {
        connector_segment *pcs = csit->second;
        m_pcs_vector.push_back(pcs);

        segment_pair pair = connector_segment_vector.at(pcs->vbo_offset);
        *lvr++ = pair.e0;
        *lvr++ = pair.n0;
        *lvr++ = pair.e1;
        *lvr++ = pair.n1;

        pcs->vbo_offset = offset;
        offset += 4 * sizeof(float);
    }

    for( csit = ec_connector_hash.begin(); csit != ec_connector_hash.end(); ++csit )
    {
        connector_segment *pcs = csit->second;
        m_pcs_vector.push_back(pcs);

        segment_pair pair = connector_segment_vector.at(pcs->vbo_offset);
        *lvr++ = pair.e0;
        *lvr++ = pair.n0;
        *lvr++ = pair.e1;
        *lvr++ = pair.n1;

        pcs->vbo_offset = offset;
        offset += 4 * sizeof(float);
    }

    for( csit = cc_connector_hash.begin(); csit != cc_connector_hash.end(); ++csit )
    {
        connector_segment *pcs = csit->second;
        m_pcs_vector.push_back(pcs);

        segment_pair pair = connector_segment_vector.at(pcs->vbo_offset);
        *lvr++ = pair.e0;
        *lvr++ = pair.n0;
        *lvr++ = pair.e1;
        *lvr++ = pair.n1;

        pcs->vbo_offset = offset;
        offset += 4 * sizeof(float);
    }

    // And so we can empty the temp buffer
    connector_segment_vector.clear();

    // We can convert the edge hashmap to a vector, to allow  us to destroy the hashmap
    // and at the same time free up the point storage in the VE_Elements, since all the points
    // are now in the VBO buffer
    for( it = m_ve_hash.begin(); it != m_ve_hash.end(); ++it ) {
        VE_Element *pedge = it.value();
        if(pedge){
            m_pve_vector.push_back(pedge);
            free(pedge->pPoints);
        }
    }
    m_ve_hash.clear();


    // and we can empty the connector hashmap,
    // and at the same time free up the point storage in the VC_Elements, since all the points
    // are now in the VBO buffer
    for( VC_Hash::iterator itc = m_vc_hash.begin(); itc != m_vc_hash.end(); ++itc ) {
        VC_Element *pcs = itc.value();
        if(pcs)
            free(pcs->pPoint);
        delete pcs;
    }
    m_vc_hash.clear();




}




void s57chart::BuildLineVBO( void )
{
#ifdef ocpnUSE_GL
    // cm93 cannot efficiently use VBO, since the edge list is discovered incrementally,
    // and this would require rebuilding the VBO for each new cell that is loaded.

    if(CHART_TYPE_CM93 == GetChartType())
        return;

    if(!g_b_EnableVBO)
        return;

    if(m_LineVBO_name == -1){

        //      Create the VBO
        GLuint vboId;
        (s_glGenBuffers)(1, &vboId);

        // bind VBO in order to use
        (s_glBindBuffer)(GL_ARRAY_BUFFER, vboId);

        // upload data to VBO
        glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
        (s_glBufferData)(GL_ARRAY_BUFFER, m_vbo_byte_length, m_line_vertex_buffer, GL_STATIC_DRAW);

        glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
        (s_glBindBuffer)(GL_ARRAY_BUFFER, 0);

        //  Loop and populate all the objects
        for( int i = 0; i < PRIO_NUM; ++i ) {
            for( int j = 0; j < LUPNAME_NUM; j++ ) {
                ObjRazRules *top = razRules[i][j];
                while( top != NULL ) {
                    S57Obj *obj = top->obj;
                    obj->auxParm2 = vboId;
                    top = top->next;
                }
            }
        }

        m_LineVBO_name = vboId;

    }
#endif
}


/*              RectRegion:
 *                      This is the Screen region desired to be updated.  Will be either 1 rectangle(full screen)
 *                      or two rectangles (panning with FBO accelerated pan logic)
 *
 *              Region:
 *                      This is the LLRegion describing the quilt active region for this chart.
 *
 *              So, Actual rendering area onscreen should be clipped to the intersection of the two regions.
 */

bool s57chart::RenderRegionViewOnGL( QGLContext* glc, const ViewPort& VPoint,
                                     const OCPNRegion &RectRegion, const LLRegion &Region )
{
    if(!m_RAZBuilt) return false;

    return DoRenderRegionViewOnGL( glc, VPoint, RectRegion, Region, false );
}

bool s57chart::RenderOverlayRegionViewOnGL( QGLContext *glc, const ViewPort& VPoint,
                                            const OCPNRegion &RectRegion, const LLRegion &Region )
{
    if(!m_RAZBuilt) return false;

    return DoRenderRegionViewOnGL( glc, VPoint, RectRegion, Region, true );
}

bool s57chart::RenderRegionViewOnGLNoText( QGLContext *glc, const ViewPort& VPoint,
                                           const OCPNRegion &RectRegion, const LLRegion &Region )
{
//    qDebug()<<"start render region view";
    if(!m_RAZBuilt)
    {
        qDebug()<<"end render region view with no raz build";
        return false;
    }

    bool b_text = ps52plib->GetShowS57Text();
    ps52plib->m_bShowS57Text = false;
    bool b_ret =  DoRenderRegionViewOnGL( glc, VPoint, RectRegion, Region, false );
    ps52plib->m_bShowS57Text = b_text;
    
//    qDebug()<<"start render region end normal";
    return b_ret;
}

bool s57chart::RenderViewOnGLTextOnly(QGLContext *glc, const ViewPort& VPoint)
{
    if(!m_RAZBuilt) return false;

#ifdef ocpnUSE_GL
    
    if( !ps52plib ) return false;
    
    SetVPParms( VPoint );
    
    glPushMatrix(); //    Adjust for rotation
    glChartCanvas::RotateToViewPort(VPoint);

    glChartCanvas::DisableClipRegion();
    DoRenderOnGLText( glc, VPoint );

    glPopMatrix();
    
    
#endif
    return true;
}

bool s57chart::DoRenderRegionViewOnGL(QGLContext *glc, const ViewPort& VPoint,
                                       const OCPNRegion &RectRegion, const LLRegion &Region, bool b_overlay )
{
    if(!m_RAZBuilt) return false;

#ifdef ocpnUSE_GL

    if( !ps52plib ) return false;

    if( g_bDebugS57 ) printf( "\n" );

    SetVPParms( VPoint );

    ps52plib->PrepareForRender();

    if( m_plib_state_hash != ps52plib->GetStateHash() ) {
        m_bLinePrioritySet = false;                     // need to reset line priorities
        UpdateLUPs( this );                               // and update the LUPs
        ClearRenderedTextCache();                       // and reset the text renderer,
        //for the case where depth(height) units change
        ResetPointBBoxes( m_last_vp, VPoint );
        SetSafetyContour();

        m_plib_state_hash = ps52plib->GetStateHash();

    }

    if( VPoint.viewScalePPM() != m_last_vp.viewScalePPM() ) {
        ResetPointBBoxes( m_last_vp, VPoint );
    }

    BuildLineVBO();
    SetLinePriorities();

    //        Clear the text declutter list
    ps52plib->ClearTextList();

    ViewPort vp = VPoint;

    // region always has either 1 or 2 rectangles (full screen or panning rectangles)
    for(OCPNRegionIterator upd ( RectRegion ); upd.HaveRects(); upd.NextRect()) {
        LLRegion chart_region = vp.GetLLRegion(upd.GetRect());
        chart_region.Intersect(Region);

        if(!chart_region.Empty()) {

            //TODO  I think this needs nore work for alternate Projections...
            //  cm93 vpoint crossing Greenwich, panning east, was rendering areas incorrectly.
            ViewPort cvp = glChartCanvas::ClippedViewport(VPoint, chart_region);

            if(CHART_TYPE_CM93 == GetChartType()){
                // for now I will revert to the faster rectangle clipping now that rendering order is resolved
                //                glChartCanvas::SetClipRegion(cvp, chart_region);
                glChartCanvas::SetClipRect(cvp, upd.GetRect(), false);
            }
            else
                glChartCanvas::SetClipRect(cvp, upd.GetRect(), false);

            ps52plib->m_last_clip_rect = upd.GetRect();
            glPushMatrix(); //    Adjust for rotation
            glChartCanvas::RotateToViewPort(VPoint);
            DoRenderOnGL( glc, cvp );
            glPopMatrix();
            glChartCanvas::DisableClipRegion();
        }
    }

    //      Update last_vp to reflect current state
    m_last_vp = VPoint;


    //      CALLGRIND_STOP_INSTRUMENTATION

#endif
    return true;
}

bool s57chart::DoRenderOnGL(QGLContext *glc, const ViewPort& VPoint )
{
#ifdef ocpnUSE_GL

    int i;
    ObjRazRules *top;
    ObjRazRules *crnt;
    ViewPort tvp = VPoint;                    // undo const  TODO fix this in PLIB

    //      Render the areas quickly
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
            top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3]; // Area Plain Boundaries

        while( top != NULL ) {
//            qDebug()<<"render area:"<<top->obj->FeatureName;
            crnt = top;
            top = top->next;               // next object
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderAreaToGL( glc, crnt, &tvp );
        }
    }

    //    Render the lines and points
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
            top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3]; // Area Plain Boundaries
        while( top != NULL ) {
//            qDebug()<<"rerender area???:"<<top->obj->FeatureName;
            crnt = top;
            top = top->next;               // next object
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToGL( glc, crnt, &tvp );
        }

        top = razRules[i][2];           //LINES
        while( top != NULL ) {
//            qDebug()<<"render line:"<<top->obj->FeatureName;
            crnt = top;
            top = top->next;
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToGL( glc, crnt, &tvp );
        }

        if( ps52plib->m_nSymbolStyle == SIMPLIFIED )
            top = razRules[i][0];       //SIMPLIFIED Points
        else
            top = razRules[i][1];           //Paper Chart Points Points

        int cnt = 0;
        while( top != NULL ) {
//            qDebug()<<"render point:"<<top->obj->FeatureName;
            crnt = top;
            top = top->next;
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToGL( glc, crnt, &tvp );
            cnt++;
        }

    }

#endif          //#ifdef ocpnUSE_GL

    return true;
}

bool s57chart::DoRenderOnGLText( QGLContext *glc, const ViewPort& VPoint )
{
#ifdef ocpnUSE_GL
    
    int i;
    ObjRazRules *top;
    ObjRazRules *crnt;
    ViewPort tvp = VPoint;                    // undo const  TODO fix this in PLIB

#if 0    
    //      Render the areas quickly
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
            top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3];           // Area Plain Boundaries

        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            crnt->sm_transform_parms = &vp_transform;
            ///                ps52plib->RenderAreaToGL( glc, crnt, &tvp );
        }
    }
#endif
    
    //    Render the lines and points
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
            top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3]; // Area Plain Boundaries

        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToGLText( glc, crnt, &tvp );
        }

        top = razRules[i][2];           //LINES
        while( top != NULL ) {
            crnt = top;
            top = top->next;
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToGLText( glc, crnt, &tvp );
        }

        if( ps52plib->m_nSymbolStyle == SIMPLIFIED )
            top = razRules[i][0];       //SIMPLIFIED Points
        else
            top = razRules[i][1];           //Paper Chart Points Points

        while( top != NULL ) {
            crnt = top;
            top = top->next;
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToGLText( glc, crnt, &tvp );
        }

    }
    
#endif          //#ifdef ocpnUSE_GL
    
    return true;
}


bool s57chart::RenderRegionViewOnDCNoText( QPainter* dc, const ViewPort& VPoint,
                                           const OCPNRegion &Region )
{
    if(!m_RAZBuilt)
        return false;
    
    bool b_text = ps52plib->GetShowS57Text();
    ps52plib->m_bShowS57Text = false;
    bool b_ret = DoRenderRegionViewOnDC( dc, VPoint, Region, false );
    ps52plib->m_bShowS57Text = b_text;
    
    return true;
}

bool s57chart::RenderRegionViewOnDCTextOnly( QPainter* dc, const ViewPort& VPoint,
                                             const OCPNRegion &Region )
{
#if 0
    if(!dc.IsOk())
        return false;

    SetVPParms( VPoint );

    //  If the viewport is rotated, there will only be one rectangle in the region
    //  so we can take a shortcut...
    if(fabs(VPoint.rotation) > .01){
        DCRenderText( dc, VPoint );
    }
    else{
        ViewPort temp_vp = VPoint;
        double temp_lon_left, temp_lat_bot, temp_lon_right, temp_lat_top;
        
        //    Decompose the region into rectangles,
        OCPNRegionIterator upd( Region ); // get the requested rect list
        while( upd.HaveRects() ) {
            wxRect rect = upd.GetRect();
            
            zchxPoint p;
            p.x = rect.x;
            p.y = rect.y;
            
            temp_vp.GetLLFromPix( p, &temp_lat_top, &temp_lon_left);
            
            p.x += rect.width;
            p.y += rect.height;
            temp_vp.GetLLFromPix( p, &temp_lat_bot, &temp_lon_right);

            if( temp_lon_right < temp_lon_left )        // presumably crossing Greenwich
                temp_lon_right += 360.;


            temp_vp.GetBBox().Set(temp_lat_bot, temp_lon_left, temp_lat_top, temp_lon_right);

            wxDCClipper clip(dc, rect);
            DCRenderText( dc, temp_vp );
            
            upd.NextRect();
        }
    }
#endif
    return true;
}

bool s57chart::RenderRegionViewOnDC( QPainter* dc, const ViewPort& VPoint,
                                     const OCPNRegion &Region )
{
    if(!m_RAZBuilt)
        return false;

    return DoRenderRegionViewOnDC( dc, VPoint, Region, false );
}

bool s57chart::RenderOverlayRegionViewOnDC( QPainter* dc, const ViewPort& VPoint,
                                            const OCPNRegion &Region )
{
    if(!m_RAZBuilt)
        return false;
    return DoRenderRegionViewOnDC( dc, VPoint, Region, true );
}

bool s57chart::DoRenderRegionViewOnDC( QPainter* dc, const ViewPort& VPoint,
                                       const OCPNRegion &Region, bool b_overlay )
{
#if 0
    SetVPParms( VPoint );

    bool force_new_view = false;

    if( Region != m_last_Region ) force_new_view = true;

    ps52plib->PrepareForRender();

    if( m_plib_state_hash != ps52plib->GetStateHash() ) {
        m_bLinePrioritySet = false;                     // need to reset line priorities
        UpdateLUPs( this );                               // and update the LUPs
        ClearRenderedTextCache();                       // and reset the text renderer,
        //for the case where depth(height) units change
        ResetPointBBoxes( m_last_vp, VPoint );
        SetSafetyContour();
    }

    if( VPoint.viewScalePPM() != m_last_vp.viewScalePPM() ) {
        ResetPointBBoxes( m_last_vp, VPoint );
    }

    SetLinePriorities();

    bool bnew_view = DoRenderViewOnDC( dc, VPoint, DC_RENDER_ONLY, force_new_view );

    //    If quilting, we need to return a lon()ed bitmap instead of the original golden item
    if( VPoint.b_quilt ) {
        if( m_plon()eBM ) {
            if( ( m_plon()eBM->GetWidth() != VPoint.pixWidth() )
                    || ( m_plon()eBM->GetHeight() != VPoint.pixHeight() ) ) {
                m_plon()eBM.reset();
            }
        }
        if( nullptr == m_plon()eBM )
            m_plon()eBM.reset(new wxBitmap( VPoint.pixWidth(), VPoint.pixHeight(), -1 ));

        wxMemoryDC dc_lon()e;
        dc_lon()e.SelectObject( *m_plon()eBM );

#ifdef ocpnUSE_DIBSECTION
        ocpnMemDC memdc, dc_org;
#else
        wxMemoryDC memdc, dc_org;
#endif

        pDIB->SelectIntoDC( dc_org );

        //    Decompose the region into rectangles, and fetch them into the target dc
        OCPNRegionIterator upd( Region ); // get the requested rect list
        while( upd.HaveRects() ) {
            wxRect rect = upd.GetRect();
            dc_lon()e.Blit( rect.x, rect.y, rect.width, rect.height, &dc_org, rect.x, rect.y );
            upd.NextRect();
        }

        dc_lon()e.SelectObject( wxNullBitmap );
        dc_org.SelectObject( wxNullBitmap );

        //    Create a mask
        if( b_overlay ) {
            wxColour nodat = GetGlobalColor( _T ( "NODTA" ) );
            wxColour nodat_sub = nodat;

#ifdef ocpnUSE_ocpnBitmap
            nodat_sub = wxColour( nodat.Blue(), nodat.Green(), nodat.Red() );
#endif
            // Mask is owned by the bitmap.
            m_plon()eBM->SetMask( new wxMask( *m_plon()eBM, nodat_sub ) );
        }

        dc.SelectObject( *m_plon()eBM );
    } else
        pDIB->SelectIntoDC( dc );

    m_last_Region = Region;
#endif

    return true;

}

bool s57chart::RenderViewOnDC( QPainter* dc, const ViewPort& VPoint )
{
    //    CALLGRIND_START_INSTRUMENTATION

    SetVPParms( VPoint );

    ps52plib->PrepareForRender();

    if( m_plib_state_hash != ps52plib->GetStateHash() ) {
        m_bLinePrioritySet = false;                     // need to reset line priorities
        UpdateLUPs( this );                               // and update the LUPs
        ClearRenderedTextCache();                       // and reset the text renderer
        SetSafetyContour();
    }

    SetLinePriorities();

    bool bnew_view = DoRenderViewOnDC( dc, VPoint, DC_RENDER_ONLY, false );

    pDIB->SelectIntoDC( dc );

    return bnew_view;

    //    CALLGRIND_STOP_INSTRUMENTATION

}

bool s57chart::DoRenderViewOnDC( QPainter* dc, const ViewPort& VPoint, RenderTypeEnum option,
                                 bool force_new_view )
{
    return true;
#if 0
    bool bnewview = false;
    zchxPoint rul, rlr;
    bool bNewVP = false;

    bool bReallyNew = false;

    double easting_ul, northing_ul;
    double easting_lr, northing_lr;
    double prev_easting_ul = 0., prev_northing_ul = 0.;
    double prev_easting_lr, prev_northing_lr;

    if( ps52plib->GetPLIBColorScheme() != m_lastColorScheme ) bReallyNew = true;
    m_lastColorScheme = ps52plib->GetPLIBColorScheme();

    if( VPoint.viewScalePPM() != m_last_vp.viewScalePPM() ) bReallyNew = true;

    //      If the scale is very small, do not use the cache to avoid harmonic difficulties...
    if( VPoint.chart_scale > 1e8 ) bReallyNew = true;

    wxRect dest( 0, 0, VPoint.pixWidth(), VPoint.pixHeight() );
    if( m_last_vprect != dest ) bReallyNew = true;
    m_last_vprect = dest;

    if( m_plib_state_hash != ps52plib->GetStateHash() ) {
        bReallyNew = true;
        m_plib_state_hash = ps52plib->GetStateHash();
    }

    if( bReallyNew ) {
        bNewVP = true;
        pDIB.reset();
        bnewview = true;
    }

    //      Calculate the desired rectangle in the last cached image space
    if( m_last_vp.IsValid() ) {
        easting_ul = m_easting_vp_center - ( ( VPoint.pixWidth() / 2 ) / m_viewScalePPM() );
        northing_ul = m_northing_vp_center + ( ( VPoint.pixHeight() / 2 ) / m_viewScalePPM() );
        easting_lr = easting_ul + ( VPoint.pixWidth() / m_viewScalePPM() );
        northing_lr = northing_ul - ( VPoint.pixHeight() / m_viewScalePPM() );

        double last_easting_vp_center, last_northing_vp_center;
        toSM( m_last_vp.lat(), m_last_vp.lon(), ref_lat, ref_lon, &last_easting_vp_center,
              &last_northing_vp_center );

        prev_easting_ul = last_easting_vp_center
                - ( ( m_last_vp.pixWidth() / 2 ) / m_viewScalePPM() );
        prev_northing_ul = last_northing_vp_center
                + ( ( m_last_vp.pixHeight() / 2 ) / m_viewScalePPM() );
        prev_easting_lr = easting_ul + ( m_last_vp.pixWidth() / m_viewScalePPM() );
        prev_northing_lr = northing_ul - ( m_last_vp.pixHeight() / m_viewScalePPM() );

        double dx = ( easting_ul - prev_easting_ul ) * m_viewScalePPM();
        double dy = ( prev_northing_ul - northing_ul ) * m_viewScalePPM();

        rul.x = (int) round((easting_ul - prev_easting_ul) * m_viewScalePPM());
        rul.y = (int) round((prev_northing_ul - northing_ul) * m_viewScalePPM());

        rlr.x = (int) round((easting_lr - prev_easting_ul) * m_viewScalePPM());
        rlr.y = (int) round((prev_northing_ul - northing_lr) * m_viewScalePPM());

        if( ( fabs( dx - wxRound( dx ) ) > 1e-5 ) || ( fabs( dy - wxRound( dy ) ) > 1e-5 ) ) {
            if( g_bDebugS57 ) printf(
                        "s57chart::DoRender  Cache miss on non-integer pixel delta %g %g\n", dx, dy );
            rul.x = 0;
            rul.y = 0;
            rlr.x = 0;
            rlr.y = 0;
            bNewVP = true;
        }

        else if( ( rul.x != 0 ) || ( rul.y != 0 ) ) {
            if( g_bDebugS57 ) printf( "newvp due to rul\n" );
            bNewVP = true;
        }
    } else {
        rul.x = 0;
        rul.y = 0;
        rlr.x = 0;
        rlr.y = 0;
        bNewVP = true;
    }

    if( force_new_view ) bNewVP = true;

    //      Using regions, calculate re-usable area of pDIB

    OCPNRegion rgn_last( 0, 0, VPoint.pixWidth(), VPoint.pixHeight() );
    OCPNRegion rgn_new( rul.x, rul.y, rlr.x - rul.x, rlr.y - rul.y );
    rgn_last.Intersect( rgn_new );            // intersection is reusable portion

    if( bNewVP && (nullptr != pDIB) && !rgn_last.isEmpty() ) {
        int xu, yu, wu, hu;
        rgn_last.GetBox( xu, yu, wu, hu );

        int desx = 0;
        int desy = 0;
        int srcx = xu;
        int srcy = yu;

        if( rul.x < 0 ) {
            srcx = 0;
            desx = -rul.x;
        }
        if( rul.y < 0 ) {
            srcy = 0;
            desy = -rul.y;
        }

        ocpnMemDC dc_last;
        pDIB->SelectIntoDC( dc_last );

        ocpnMemDC dc_new;
        PixelCache *pDIBNew = new PixelCache( VPoint.pixWidth(), VPoint.pixHeight(), BPP );
        pDIBNew->SelectIntoDC( dc_new );

        //        printf("reuse blit %d %d %d %d %d %d\n",desx, desy, wu, hu,  srcx, srcy);
        dc_new.Blit( desx, desy, wu, hu, (wxDC *) &dc_last, srcx, srcy, wxCOPY );

        //        Ask the plib to adjust the persistent text rectangle list for this canvas shift
        //        This ensures that, on pans, the list stays in registration with the new text renders to come
        ps52plib->AdjustTextList( desx - srcx, desy - srcy, VPoint.pixWidth(), VPoint.pixHeight() );

        dc_new.SelectObject( wxNullBitmap );
        dc_last.SelectObject( wxNullBitmap );

        pDIB.reset(pDIBNew);

        //              OK, now have the re-useable section in place
        //              Next, build the new sections

        pDIB->SelectIntoDC( dc );

        OCPNRegion rgn_delta( 0, 0, VPoint.pixWidth(), VPoint.pixHeight() );
        OCPNRegion rgn_reused( desx, desy, wu, hu );
        rgn_delta.Subtract( rgn_reused );

        OCPNRegionIterator upd( rgn_delta ); // get the update rect list
        while( upd.HaveRects() ) {
            wxRect rect = upd.GetRect();

            //      Build temp ViewPort on this region

            ViewPort temp_vp = VPoint;
            double temp_lon_left, temp_lat_bot, temp_lon_right, temp_lat_top;

            double temp_northing_ul = prev_northing_ul - ( rul.y / m_viewScalePPM() )
                    - ( rect.y / m_viewScalePPM() );
            double temp_easting_ul = prev_easting_ul + ( rul.x / m_viewScalePPM() )
                    + ( rect.x / m_viewScalePPM() );
            fromSM( temp_easting_ul, temp_northing_ul, ref_lat, ref_lon, &temp_lat_top,
                    &temp_lon_left );

            double temp_northing_lr = temp_northing_ul - ( rect.height / m_viewScalePPM() );
            double temp_easting_lr = temp_easting_ul + ( rect.width / m_viewScalePPM() );
            fromSM( temp_easting_lr, temp_northing_lr, ref_lat, ref_lon, &temp_lat_bot,
                    &temp_lon_right );

            temp_vp.GetBBox().Set( temp_lat_bot, temp_lon_left,
                                   temp_lat_top, temp_lon_right );

            //      Allow some slop in the viewport
            //    TODO Investigate why this fails if greater than 5 percent
            double margin = fmin(temp_vp.GetBBox().GetLonRange(), temp_vp.GetBBox().GetLatRange())
                    * 0.05;
            temp_vp.GetBBox().EnLarge( margin );

            //      And Render it new piece on the target dc
            //     printf("New Render, rendering %d %d %d %d \n", rect.x, rect.y, rect.width, rect.height);

            DCRenderRect( dc, temp_vp, &rect );

            upd.NextRect();
        }

        dc.SelectObject( wxNullBitmap );

        bnewview = true;

        //      Update last_vp to reflect the current cached bitmap
        m_last_vp = VPoint;

    }

    else if( bNewVP || (nullptr == pDIB )) {
        pDIB.reset(new PixelCache( VPoint.pixWidth(), VPoint.pixHeight(), ZCHXBPP ));     // destination

        wxRect full_rect( 0, 0, VPoint.pixWidth(), VPoint.pixHeight() );
        pDIB->SelectIntoDC( dc );

        //        Clear the text declutter list
        ps52plib->ClearTextList();

        DCRenderRect( dc, VPoint, &full_rect );

        dc.SelectObject( wxNullBitmap );

        bnewview = true;

        //      Update last_vp to reflect the current cached bitmap
        m_last_vp = VPoint;

    }

    return bnewview;
#endif

}

int s57chart::DCRenderRect( QPainter* dcinput, const ViewPort& vp, QRect* rect )
{
#if 0
    int i;
    ObjRazRules *top;
    ObjRazRules *crnt;

    wxASSERT(rect);
    ViewPort tvp = vp;                    // undo const  TODO fix this in PLIB

    //    This does not work due to some issue with ref data of allocated buffer.....
    //    render_canvas_parms pb_spec( rect->x, rect->y, rect->width, rect->height,  GetGlobalColor ( _T ( "NODTA" ) ));

    render_canvas_parms pb_spec;

    pb_spec.depth = ZCHXBPP;
    pb_spec.pb_pitch = ( ( rect->width * pb_spec.depth / 8 ) );
    pb_spec.lclip = rect->x;
    pb_spec.rclip = rect->x + rect->width - 1;
    pb_spec.pix_buff = (unsigned char *) malloc( rect->height * pb_spec.pb_pitch );
    pb_spec.width = rect->width;
    pb_spec.height = rect->height;
    pb_spec.x = rect->x;
    pb_spec.y = rect->y;

#ifdef ocpnUSE_ocpnBitmap
    pb_spec.b_revrgb = true;
#else
    pb_spec.b_revrgb = false;
#endif

    // Preset background
    wxColour color = GetGlobalColor( _T ( "NODTA" ) );
    unsigned char r, g, b;
    if( color.IsOk() ) {
        r = color.Red();
        g = color.Green();
        b = color.Blue();
    } else
        r = g = b = 0;

    if( pb_spec.depth == 24 ) {
        for( int i = 0; i < pb_spec.height; i++ ) {
            unsigned char *p = pb_spec.pix_buff + ( i * pb_spec.pb_pitch );
            for( int j = 0; j < pb_spec.width; j++ ) {
                *p++ = r;
                *p++ = g;
                *p++ = b;
            }
        }
    } else {
        int color_int = ( ( r ).append(16 ) + ( ( g ).append(8 ) + ( b );

        for( int i = 0; i < pb_spec.height; i++ ) {
            int *p = (int *) ( pb_spec.pix_buff + ( i * pb_spec.pb_pitch ) );
            for( int j = 0; j < pb_spec.width; j++ ) {
                *p++ = color_int;
            }
        }
    }

    //      Render the areas quickly
    for( i = 0; i < PRIO_NUM; ++i ) {
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
            top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3]; // Area Plain Boundaries

        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderAreaToDC( &dcinput, crnt, &tvp, &pb_spec );
        }
    }

    //      Convert the Private render canvas into a bitmap
#ifdef ocpnUSE_ocpnBitmap
    ocpnBitmap *pREN = new ocpnBitmap( pb_spec.pix_buff, pb_spec.width, pb_spec.height,
                                       pb_spec.depth );
#else
    wxImage *prender_image = new wxImage(pb_spec.width, pb_spec.height, false);
    prender_image->SetData((unsigned char*)pb_spec.pix_buff);
    wxBitmap *pREN = new wxBitmap(*prender_image);

#endif

    //      Map it into a temporary DC
    wxMemoryDC dc_ren;
    dc_ren.SelectObject( *pREN );

    //      Blit it onto the target dc
    dcinput.Blit( pb_spec.x, pb_spec.y, pb_spec.width, pb_spec.height, (wxDC *) &dc_ren, 0, 0 );

    //      And clean up the mess
    dc_ren.SelectObject( wxNullBitmap );

#ifdef ocpnUSE_ocpnBitmap
    free( pb_spec.pix_buff );
#else
    delete prender_image;           // the image owns the data
    // and so will free it in due course
#endif

    delete pREN;

    //      Render the rest of the objects/primitives
    DCRenderLPB( dcinput, vp, rect );

    return 1;
}

bool s57chart::DCRenderLPB( wxMemoryDC& dcinput, const ViewPort& vp, wxRect* rect )
{
    int i;
    ObjRazRules *top;
    ObjRazRules *crnt;
    ViewPort tvp = vp;                    // undo const  TODO fix this in PLIB

    for( i = 0; i < PRIO_NUM; ++i ) {
        //      Set up a Clipper for Lines
        wxDCClipper *pdcc = NULL;
        if( rect ) {
            wxRect nr = *rect;
            //         pdcc = new wxDCClipper(dcinput, nr);
        }

        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
            top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3];           // Area Plain Boundaries
        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToDC( &dcinput, crnt, &tvp );
        }

        top = razRules[i][2];           //LINES
        while( top != NULL ) {
            crnt = top;
            top = top->next;
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToDC( &dcinput, crnt, &tvp );
        }

        if( ps52plib->m_nSymbolStyle == SIMPLIFIED )
            top = razRules[i][0];       //SIMPLIFIED Points
        else
            top = razRules[i][1];           //Paper Chart Points Points

        while( top != NULL ) {
            crnt = top;
            top = top->next;
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToDC( &dcinput, crnt, &tvp );
        }

        //      Destroy Clipper
        if( pdcc ) delete pdcc;
    }

    /*
     printf("Render Lines                  %ldms\n", stlines.Time());
     printf("Render Simple Points          %ldms\n", stsim_pt.Time());
     printf("Render Paper Points           %ldms\n", stpap_pt.Time());
     printf("Render Symbolized Boundaries  %ldms\n", stasb.Time());
     printf("Render Plain Boundaries       %ldms\n\n", stapb.Time());
     */
#endif
    return true;
}

bool s57chart::DCRenderText( QPainter* dcinput, const ViewPort& vp )
{
#if 0
    int i;
    ObjRazRules *top;
    ObjRazRules *crnt;
    ViewPort tvp = vp;                    // undo const  TODO fix this in PLIB
    
    for( i = 0; i < PRIO_NUM; ++i ) {
        
        if( ps52plib->m_nBoundaryStyle == SYMBOLIZED_BOUNDARIES )
            top = razRules[i][4]; // Area Symbolized Boundaries
        else
            top = razRules[i][3]; // Area Plain Boundaries

        while( top != NULL ) {
            crnt = top;
            top = top->next;               // next object
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToDCText( &dcinput, crnt, &tvp );
        }

        top = razRules[i][2];           //LINES
        while( top != NULL ) {
            crnt = top;
            top = top->next;
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToDCText( &dcinput, crnt, &tvp );
        }

        if( ps52plib->m_nSymbolStyle == SIMPLIFIED )
            top = razRules[i][0];       //SIMPLIFIED Points
        else
            top = razRules[i][1];           //Paper Chart Points Points

        while( top != NULL ) {
            crnt = top;
            top = top->next;
            crnt->sm_transform_parms = &vp_transform;
            ps52plib->RenderObjectToDCText( &dcinput, crnt, &tvp );
        }
    }
#endif

    return true;
}




bool s57chart::IsCellOverlayType( char *pFullPath )
{
    QFileInfo fn( QString::fromUtf8(pFullPath ) );
    //      Get the "Usage" character
    QString cname = zchxFuncUtil::getFileName(fn.fileName());
    if(cname.length() >= 3)
        return ( (cname[2] == 'L') || (cname[2] == 'A'));
    else
        return false;
}

InitReturn s57chart::Init( const QString& name, ChartInitFlag flags )
{
    // Really can only Init and use S57 chart if the S52 Presentation Library is present and OK
    if( (NULL ==ps52plib) || !(ps52plib->m_bOK) )
        return INIT_FAIL_REMOVE;
    
    QString ext;
    if(name.toUpper().endsWith(".XZ")) {
        QString file_name = name.left(name.length()-3);
        ext = zchxFuncUtil::getFileExt( QFileInfo(file_name).fileName());
        QString file_name_no_ext = zchxFuncUtil::getFileName( QFileInfo(file_name).fileName());
        
        // decompress to temp file to allow seeking
        m_TempFilePath = zchxFuncUtil::getTempDir() + zchxFuncUtil::separator() + file_name_no_ext;

        if(/*!wxFileExists(m_TempFilePath) && */!DecompressXZFile(name, m_TempFilePath)) {
            QFile::remove(m_TempFilePath);
            return INIT_FAIL_REMOVE;
        }
    } else {
        m_TempFilePath = name;
        ext = zchxFuncUtil::getFileExt( QFileInfo(name).fileName());
    }
    m_FullPath = name;

    //    Use a static semaphore flag to prevent recursion
    if( s_bInS57 ) {
        //          printf("s57chart::Init() recursion..., retry\n");
        //          ZCHX_LOGMSG(("Recursion"));
        return INIT_FAIL_NOERROR;
    }

    s_bInS57++;

    InitReturn ret_value = INIT_OK;

    m_Description = name;

    QFileInfo fn( m_TempFilePath );

    //      Get the "Usage" character
    QString cname = zchxFuncUtil::getFileName(fn.fileName());
    m_usage_char = cname[2].unicode();

    //  Establish a common reference point for the chart
    ref_lat = ( m_FullExtent.NLAT + m_FullExtent.SLAT ) / 2.;
    ref_lon = ( m_FullExtent.WLON + m_FullExtent.ELON ) / 2.;

    if( flags == THUMB_ONLY ) {

        // Look for Thumbnail
        // LoadThumb();

        s_bInS57--;
        return INIT_OK;
    }

    if( flags == HEADER_ONLY ) {
        if( ext == ("000") ) {
            if( !GetBaseFileAttr( fn.absoluteFilePath() ) )
                ret_value = INIT_FAIL_REMOVE;
            else {
                if( !CreateHeaderDataFromENC() )
                    ret_value = INIT_FAIL_REMOVE;
                else
                    ret_value = INIT_OK;
            }
        } else if( ext == ("S57") ) {
            m_SENCFileName = m_TempFilePath;
            if( !CreateHeaderDataFromSENC() ) ret_value = INIT_FAIL_REMOVE;
            else
                ret_value = INIT_OK;
        }

        s_bInS57--;
        return ret_value;

    }

    //      Full initialization from here

    if( !m_bbase_file_attr_known ) {
        if( !GetBaseFileAttr( m_TempFilePath ) )
            ret_value = INIT_FAIL_REMOVE;
        else
            m_bbase_file_attr_known = true;
    }

    if( ext == ("000") ) {
        if( m_bbase_file_attr_known ) {

            int sret = FindOrCreateSenc( m_FullPath );
            if(sret == BUILD_SENC_PENDING){
                s_bInS57--;
                return INIT_OK;
            }
            
            if( sret != BUILD_SENC_OK ) {
                if( sret == BUILD_SENC_NOK_RETRY ) ret_value = INIT_FAIL_RETRY;
                else
                    ret_value = INIT_FAIL_REMOVE;
            } else
                ret_value = PostInit( flags, m_global_color_scheme );

        }

    }

    else if( ext == ("S57") ) {

        m_SENCFileName = m_TempFilePath;
        ret_value = PostInit( flags, m_global_color_scheme );

    }

    
    s_bInS57--;
    return ret_value;

}

QString s57chart::buildSENCName( const QString& name)
{
    QFileInfo fn(name);
    QString newName = zchxFuncUtil::getNewFileNameWithExt(fn.absoluteFilePath(), "S57");
    //    fn.SetExt( ("S57") );
    QString file_name = /*fn.GetFullName()*/QFileInfo(newName).fileName();
    
    //      Set the proper directory for the SENC files
    QString SENCdir = g_SENCPrefix;
    
    if( SENCdir.right(1) != zchxFuncUtil::separator() )
        SENCdir.append( zchxFuncUtil::separator() );
    
#if 1
    QString source_dir = fn.absolutePath();
    QByteArray buf = source_dir.toUtf8();
    unsigned char sha1_out[20];
    sha1( (unsigned char *) buf.data(), strlen(buf.data()), sha1_out );
    
    QString sha1;
    for (unsigned int i=0 ; i < 6 ; i++){
        QString s;
        s.sprintf("%02X", sha1_out[i]);
        sha1 += s;
    }
    sha1 += "_";
    file_name.insert(0,sha1);
#endif    
    
    QString  target = SENCdir;
    target.append(file_name);
    QFileInfo tsfn( target );
    
    return tsfn.absoluteFilePath();
}

//-----------------------------------------------------------------------------------------------
//    Find or Create a relevent SENC file from a given .000 ENC file
//    Returns with error code, and associated SENC file name in m_S57FileName
//-----------------------------------------------------------------------------------------------
int s57chart::FindOrCreateSenc( const QString& name, bool b_progress )
{
    //  This method may be called for a compressed .000 cell, so check and decompress if necessary
    QString ext;
    if(name.toUpper().endsWith(".XZ")) {
        QString file_name = name.left(name.length() - 3);
        ext = zchxFuncUtil::getFileExt(QFileInfo(file_name).fileName());
        
        // decompress to temp file to allow seeking
        m_TempFilePath = zchxFuncUtil::getTempDir() + zchxFuncUtil::separator() +
                zchxFuncUtil::getFileName(QFileInfo(file_name).fileName());
        
        if(/*!wxFileExists(m_TempFilePath) &&*/ !DecompressXZFile(name, m_TempFilePath)) {
            QFile::remove(m_TempFilePath);
            return INIT_FAIL_REMOVE;
        }
    } else {
        m_TempFilePath = name;
        ext = zchxFuncUtil::getFileExt(QFileInfo(name).fileName());
    }
    m_FullPath = name;
    
    if( !m_bbase_file_attr_known ) {
        if( !GetBaseFileAttr( m_TempFilePath ) )
            return INIT_FAIL_REMOVE;
        else
            m_bbase_file_attr_known = true;
    }
    
    //      Establish location for SENC files
    m_SENCFileName = buildSENCName( name );
    
    int build_ret_val = 1;

    bool bbuild_new_senc = false;
    m_bneed_new_thumbnail = false;

    QFileInfo FileName000( m_TempFilePath );

    //      Look for SENC file in the target directory

    qDebug("S57chart::Checking SENC file: %s", m_SENCFileName.toUtf8().data());
    
    {
        int force_make_senc = 0;

        if( QFile::exists(m_SENCFileName) ){                    // SENC file exists

            Osenc senc;
            if(senc.ingestHeader( m_SENCFileName ) )
            {
                bbuild_new_senc = true;
                qDebug("    Rebuilding SENC due to ingestHeader failure.");
            } else{

                int senc_file_version = senc.getSencReadVersion();

                int last_update = senc.getSENCReadLastUpdate();

                QString str = senc.getSENCFileCreateDate();
                QDateTime SENCCreateDate = QDateTime::fromString(str, ("yyyyMMdd"));

                /*if( SENCCreateDate.isValid() )
                    SENCCreateDate.setTime(QTime(0));   */                // to midnight

                //                wxULongLong size000 = senc.getFileSize000();
                //                QString ssize000 = senc.getsFileSize000();

                QString senc_base_edtn = senc.getSENCReadBaseEdition();
                long isenc_edition = senc_base_edtn.toLong();
                long ifile_edition = m_edtn000.toLong();
                
                //              Anything to do?
                //force_make_senc = 1;
                //  SENC file version has to be correct for other tests to make sense
                if( senc_file_version != CURRENT_SENC_FORMAT_VERSION ){
                    bbuild_new_senc = true;
                    qDebug("    Rebuilding SENC due to SENC format update.");
                }

                //  Senc EDTN must be the same as .000 file EDTN.
                //  This test catches the usual case where the .000 file is updated from the web,
                //  and all updates (.001, .002, etc.)  are subsumed.
                
                else if( ifile_edition > isenc_edition ){
                    bbuild_new_senc = true;
                    qDebug("    Rebuilding SENC due to cell edition update.");
                    QString msg;
                    msg = ("    Last edition recorded in SENC: ");
                    msg += senc_base_edtn;
                    msg += ("  most recent edition cell file: ");
                    msg += m_edtn000;
                    qDebug()<<msg;
                }
                else {
                    //    See if there are any new update files  in the ENC directory
                    int most_recent_update_file = GetUpdateFileArray( FileName000, NULL, m_date000, m_edtn000 );

                    if( ifile_edition == isenc_edition ){
                        if( most_recent_update_file > last_update ){
                            bbuild_new_senc = true;
                            qDebug("    Rebuilding SENC due to incremental cell update.");
                            QString msg;
                            msg.sprintf("    Last update recorded in SENC: %d   most recent update file: %d", last_update, most_recent_update_file);
                            qDebug()<<msg;
                        }
                    }

                    //          Make simple tests to see if the .000 file is "newer" than the SENC file representation
                    //          These tests may be redundant, since the DSID:EDTN test above should catch new base files
                    QDateTime OModTime000 = FileName000.lastModified();
                    //                    OModTime000.setTime(QTime(0));                      // to midnight
                    if( SENCCreateDate.isValid() ){
                        if( OModTime000 >  SENCCreateDate  ){
                            qDebug("    Rebuilding SENC due to Senc vs cell file time check.");
                            bbuild_new_senc = true;
                        }
                    }
                    else{
                        bbuild_new_senc = true;
                        qDebug("    Rebuilding SENC due to SENC create time invalid.");
                    }


                    //                     int Osize000l = FileName000.GetSize().GetLo();
                    //                     int Osize000h = FileName000.GetSize().GetHi();
                    //                     QString t;
                    //                     t.sprintf(("%d%d"), Osize000h, Osize000l);
                    //                     if( !t.IsSameAs( ssize000) )
                    //                         bbuild_new_senc = true;

                }

                if( force_make_senc )
                    bbuild_new_senc = true;

            }
        }
        else if( !QFile::exists(m_SENCFileName ) )                    // SENC file does not exist
        {
            qDebug("    Rebuilding SENC due to missing SENC file.");
            bbuild_new_senc = true;
        }
    }

    if( bbuild_new_senc ) {
        m_bneed_new_thumbnail = true; // force a new thumbnail to be built in PostInit()
        build_ret_val = BuildSENCFile( m_TempFilePath, m_SENCFileName, b_progress );
        
        if(BUILD_SENC_PENDING == build_ret_val)
            return BUILD_SENC_PENDING;
        if( BUILD_SENC_NOK_PERMANENT == build_ret_val )
            return INIT_FAIL_REMOVE;
        if( BUILD_SENC_NOK_RETRY == build_ret_val )
            return INIT_FAIL_RETRY;
    }

    return INIT_OK;
}

InitReturn s57chart::PostInit( ChartInitFlag flags, ColorScheme cs )
{

    //    SENC file is ready, so build the RAZ structure
    if( 0 != BuildRAZFromSENCFile( m_SENCFileName ) ) {
        qDebug("   Cannot load SENC file:%s ", m_SENCFileName.toUtf8().data()) ;

        return INIT_FAIL_RETRY;
    }

    //      Check for and if necessary rebuild Thumbnail
    //      Going to be in the global (user) SENC file directory
#if 1
    QString SENCdir = g_SENCPrefix;
    if( SENCdir.right(1) != zchxFuncUtil::separator() )
        SENCdir.append( zchxFuncUtil::separator() );
    
    QFileInfo s57File(m_SENCFileName);
    QString file_name_part = zchxFuncUtil::getFileName(s57File.fileName());
    QFileInfo ThumbFileName( SENCdir, file_name_part.mid(13 ).append(".").append("BMP"));

    if( !ThumbFileName.exists() || m_bneed_new_thumbnail )
    {
        BuildThumbnail( ThumbFileName.absoluteFilePath() );

        //  Update the member thumbdata structure
        if( ThumbFileName.exists() ) {
            wxBitmap *pBMP_NEW;
#ifdef ocpnUSE_ocpnBitmap
            pBMP_NEW = new ocpnBitmap;
#else
            pBMP_NEW = new wxBitmap;
#endif
            if( pBMP_NEW->LoadFile( ThumbFileName.absoluteFilePath() ) ) {
                delete pThumbData;
                pThumbData = new ThumbData;
                m_pDIBThumbDay = pBMP_NEW;
                //                    pThumbData->pDIBThumb = pBMP_NEW;
            }
        }
    }
#endif

    //    Set the color scheme
    m_global_color_scheme = cs;
    SetColorScheme( cs, false );

    //    Build array of contour values for later use by conditional symbology

    BuildDepthContourArray();
    m_RAZBuilt = true;
    bReadyToRender = true;

    return INIT_OK;
}

void s57chart::ClearDepthContourArray( void )
{

    if( m_nvaldco_alloc ) {
        free (m_pvaldco_array);
    }
    m_nvaldco_alloc = 5;
    m_nvaldco = 0;
    m_pvaldco_array = (double *) calloc( m_nvaldco_alloc, sizeof(double) );
}

void s57chart::BuildDepthContourArray( void )
{
    //    Build array of contour values for later use by conditional symbology

    if( 0 == m_nvaldco_alloc ) {
        m_nvaldco_alloc = 5;
        m_pvaldco_array = (double *) calloc( m_nvaldco_alloc, sizeof(double) );
    }

    ObjRazRules *top;
    // some ENC have a lot of DEPCNT objects but they seem to store them
    // in VALDCO order, try to take advantage of that.
    double prev_valdco = 0.0;

    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {

            top = razRules[i][j];
            while( top != NULL ) {
                if( !strncmp( top->obj->FeatureName, "DEPCNT", 6 ) ) {
                    double valdco = 0.0;
                    if( GetDoubleAttr( top->obj, "VALDCO", valdco ) ) {
                        if (valdco != prev_valdco) {
                            prev_valdco = valdco;
                            m_nvaldco++;
                            if( m_nvaldco > m_nvaldco_alloc ) {
                                void *tr = realloc( (void *) m_pvaldco_array,
                                                    m_nvaldco_alloc * 2 * sizeof(double) );
                                m_pvaldco_array = (double *) tr;
                                m_nvaldco_alloc *= 2;
                            }
                            m_pvaldco_array[m_nvaldco - 1] = valdco;
                        }
                    }
                }
                ObjRazRules *nxx = top->next;
                top = nxx;
            }
        }
    }
    std::sort( m_pvaldco_array, m_pvaldco_array + m_nvaldco );
    SetSafetyContour();
}


void s57chart::SetSafetyContour(void)
{
    // Iterate through the array of contours in this cell, choosing the best one to
    // render as a bold "safety contour" in the PLIB.

    //    This method computes the smallest chart DEPCNT:VALDCO value which
    //    is greater than or equal to the current PLIB mariner parameter S52_MAR_SAFETY_CONTOUR

    double mar_safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);

    int i = 0;
    if( NULL != m_pvaldco_array ) {
        for( i = 0; i < m_nvaldco; i++ ) {
            if( m_pvaldco_array[i] >= mar_safety_contour )
                break;
        }

        if( i < m_nvaldco )
            m_next_safe_cnt = m_pvaldco_array[i];
        else
            m_next_safe_cnt = (double) 1e6;
    } else {
        m_next_safe_cnt = (double) 1e6;
    }

    // A safety contour greater than "Deep Depth" makes no sense...
    // So, declare "no suitable safety depth contour"
    if(m_next_safe_cnt > S52_getMarinerParam(S52_MAR_DEEP_CONTOUR))
        m_next_safe_cnt = (double) 1e6;

}

void s57chart::InvalidateCache()
{
    pDIB.reset();
}

bool s57chart::BuildThumbnail( const QString &bmpname )
{
    QFileInfo ThumbFileName( bmpname );

    //      Make the target directory if needed
    QDir dir(ThumbFileName.absolutePath());
    if( true != dir.exists()  ) {
        if( !dir.mkpath(ThumbFileName.absolutePath()) ) {
            qDebug("   Cannot create BMP file directory for :%s ",
                   ThumbFileName.absolutePath().toUtf8().data() );
            return false;
        }
    }

    //      Set up a private ViewPort
    ViewPort vp;

    vp.setLon((m_FullExtent.ELON + m_FullExtent.WLON ) / 2.);
    vp.setLat((m_FullExtent.NLAT + m_FullExtent.SLAT ) / 2.);

    float ext_max =
            fmax((m_FullExtent.NLAT - m_FullExtent.SLAT), (m_FullExtent.ELON - m_FullExtent.WLON));

    vp.setViewScalePPM((S57_THUMB_SIZE / ext_max ) / ( 1852 * 60 ));

    vp.setPixHeight(S57_THUMB_SIZE);
    vp.setPixWidth(S57_THUMB_SIZE);

    vp.setProjectionType( PROJECTION_MERCATOR );

    vp.getBBOXPtr()->Set( m_FullExtent.SLAT, m_FullExtent.WLON,
                      m_FullExtent.NLAT, m_FullExtent.ELON );

    vp.setChartScale(10000000 - 1);
    vp.setRefScale(vp.chartScale());
    vp.validate();

    // cause a clean new render
    pDIB.reset();

    SetVPParms( vp );

    //      Borrow the OBJLArray temporarily to set the object type visibility for this render
    //      First, make a copy for the curent OBJLArray viz settings, setting current value to invisible

    unsigned int OBJLCount = ps52plib->pOBJLArray->count();
    //      int *psave_viz = new int[OBJLCount];
    int *psave_viz = (int *) malloc( OBJLCount * sizeof(int) );

    int *psvr = psave_viz;
    OBJLElement *pOLE;
    unsigned int iPtr;

    for( iPtr = 0; iPtr < OBJLCount; iPtr++ ) {
        pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->at( iPtr ) );
        *psvr++ = pOLE->nViz;
        pOLE->nViz = 0;
    }

    //      Also, save some other settings
    bool bsavem_bShowSoundgp = ps52plib->m_bShowSoundg;
    bool bsave_text = ps52plib->m_bShowS57Text;
    
    // SetDisplayCategory may clear Noshow array
    ps52plib->SaveObjNoshow();

    //      Now, set up what I want for this render
    for( iPtr = 0; iPtr < OBJLCount; iPtr++ ) {
        pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->at( iPtr ) );
        if( !strncmp( pOLE->OBJLName, "LNDARE", 6 ) ) pOLE->nViz = 1;
        if( !strncmp( pOLE->OBJLName, "DEPARE", 6 ) ) pOLE->nViz = 1;
    }

    
    ps52plib->m_bShowSoundg = false;
    ps52plib->m_bShowS57Text = false;
    
    //      Use display category MARINERS_STANDARD to force use of OBJLArray
    DisCat dsave = ps52plib->GetDisplayCategory();
    ps52plib->SetDisplayCategory( MARINERS_STANDARD );

    ps52plib->AddObjNoshow( "BRIDGE" );
    ps52plib->AddObjNoshow( "GATCON" );
    
    double safety_depth = S52_getMarinerParam(S52_MAR_SAFETY_DEPTH);
    S52_setMarinerParam(S52_MAR_SAFETY_DEPTH, -100);
    double safety_contour = S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);
    S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, -100);
#if 0

#ifdef ocpnUSE_DIBSECTION
    ocpnMemDC memdc, dc_org;
#else
    wxMemoryDC memdc, dc_org;
#endif

    //      set the color scheme
    ps52plib->SaveColorScheme();
    ps52plib->SetPLIBColorScheme(("DAY") );
    //      Do the render
    DoRenderViewOnDC( memdc, vp, DC_RENDER_ONLY, true );

    //      Release the DIB
    memdc.SelectObject( wxNullBitmap );

    //      Restore the plib to previous state
    psvr = psave_viz;
    for( iPtr = 0; iPtr < OBJLCount; iPtr++ ) {
        pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->at( iPtr ) );
        pOLE->nViz = *psvr++;
    }

    ps52plib->SetDisplayCategory(dsave);
    ps52plib->RestoreObjNoshow();

    ps52plib->RemoveObjNoshow( "BRIDGE" );
    ps52plib->RemoveObjNoshow( "GATCON" );
    
    ps52plib->m_bShowSoundg = bsavem_bShowSoundgp;
    ps52plib->m_bShowS57Text = bsave_text;
    
    S52_setMarinerParam(S52_MAR_SAFETY_DEPTH, safety_depth);
    S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, safety_contour);
    
    //      Reset the color scheme
    ps52plib->RestoreColorScheme();

    //       delete psave_viz;
    free( psave_viz );

    //      lon()e pDIB into pThumbData;
    wxBitmap bmp( vp.pixWidth(), vp.pixHeight()/*,  ZCHXBPP*/);

    wxMemoryDC dc_lon()e;
    dc_lon()e.SelectObject( bmp );

    pDIB->SelectIntoDC( dc_org );

    dc_lon()e.Blit( 0, 0, vp.pixWidth(), vp.pixHeight(), (wxDC *) &dc_org, 0, 0 );

    dc_lon()e.SelectObject( wxNullBitmap );
    dc_org.SelectObject( wxNullBitmap );

    //   Save the file
    return bmp.SaveFile( ThumbFileName.absoluteFilePath(), "BMP" );
#endif
    return true;
}

//    Read the .000 ENC file and create required Chartbase data structures
bool s57chart::CreateHeaderDataFromENC( void )
{
    if( !InitENCMinimal( m_TempFilePath ) ) {
        qDebug("   Cannot initialize ENC file:%s ", m_TempFilePath.toUtf8().data() );

        return false;
    }

    OGRFeature *pFeat;
    int catcov;
    float LatMax, LatMin, LonMax, LonMin;
    LatMax = -90.;
    LatMin = 90.;
    LonMax = -179.;
    LonMin = 179.;

    m_pCOVRTablePoints = NULL;
    m_pCOVRTable = NULL;

    //  Create arrays to hold geometry objects temporarily
    MyFloatPtrArray *pAuxPtrArray = new MyFloatPtrArray;
    std::vector<int> auxCntArray, noCovrCntArray;

    MyFloatPtrArray *pNoCovrPtrArray = new MyFloatPtrArray;

    //Get the first M_COVR object
    pFeat = GetChartFirstM_COVR( catcov );

    while( pFeat ) {
        //    Get the next M_COVR feature, and create possible additional entries for COVR
        OGRPolygon *poly = (OGRPolygon *) ( pFeat->GetGeometryRef() );
        OGRLinearRing *xring = poly->getExteriorRing();

        int npt = xring->getNumPoints();

        float *pf = NULL;

        if( npt >= 3 ) {
            pf = (float *) malloc( 2 * npt * sizeof(float) );
            float *pfr = pf;

            for( int i = 0; i < npt; i++ ) {
                OGRPoint p;
                xring->getPoint( i, &p );

                if( catcov == 1 ) {
                    LatMax = fmax(LatMax, p.getY());
                    LatMin = fmin(LatMin, p.getY());
                    LonMax = fmax(LonMax, p.getX());
                    LonMin = fmin(LonMin, p.getX());
                }

                pfr[0] = p.getY();             // lat
                pfr[1] = p.getX();             // lon

                pfr += 2;
            }

            if( catcov == 1 ) {
                pAuxPtrArray->append( pf );
                auxCntArray.push_back( npt );
            }
            else if( catcov == 2 ){
                pNoCovrPtrArray->append( pf );
                noCovrCntArray.push_back( npt );
            }
        }


        delete pFeat;
        pFeat = GetChartNextM_COVR( catcov );
    }         // while

    //    Allocate the storage

    m_nCOVREntries = auxCntArray.size();

    //    Create new COVR entries

    if( m_nCOVREntries >= 1 ) {
        m_pCOVRTablePoints = (int *) malloc( m_nCOVREntries * sizeof(int) );
        m_pCOVRTable = (float **) malloc( m_nCOVREntries * sizeof(float *) );

        for( unsigned int j = 0; j < (unsigned int) m_nCOVREntries; j++ ) {
            m_pCOVRTablePoints[j] = auxCntArray[j];
            m_pCOVRTable[j] = pAuxPtrArray->at( j );
        }
    }

    else                                     // strange case, found no CATCOV=1 M_COVR objects
    {
        qDebug("   ENC contains no useable M_COVR, CATCOV=1 features:  %s", m_TempFilePath.toUtf8().data() );
    }


    //      And for the NoCovr regions
    m_nNoCOVREntries = noCovrCntArray.size();

    if( m_nNoCOVREntries ) {
        //    Create new NoCOVR entries
        m_pNoCOVRTablePoints = (int *) malloc( m_nNoCOVREntries * sizeof(int) );
        m_pNoCOVRTable = (float **) malloc( m_nNoCOVREntries * sizeof(float *) );

        for( unsigned int j = 0; j < (unsigned int) m_nNoCOVREntries; j++ ) {
            m_pNoCOVRTablePoints[j] = noCovrCntArray[j];
            m_pNoCOVRTable[j] = pNoCovrPtrArray->at( j );
        }
    }
    else {
        m_pNoCOVRTablePoints = NULL;
        m_pNoCOVRTable = NULL;
    }

    delete pAuxPtrArray;
    delete pNoCovrPtrArray;


    if( 0 == m_nCOVREntries ) {                        // fallback
        qDebug("   ENC contains no M_COVR features:  %s", m_TempFilePath.toUtf8().data() );

        qDebug("   Calculating Chart Extents as fallback.");

        OGREnvelope Env;

        //    Get the reader
        S57Reader *pENCReader = m_pENCDS->GetModule( 0 );

        if( pENCReader->GetExtent( &Env, true ) == OGRERR_NONE ) {

            LatMax = Env.MaxY;
            LonMax = Env.MaxX;
            LatMin = Env.MinY;
            LonMin = Env.MinX;

            m_nCOVREntries = 1;
            m_pCOVRTablePoints = (int *) malloc( sizeof(int) );
            *m_pCOVRTablePoints = 4;
            m_pCOVRTable = (float **) malloc( sizeof(float *) );
            float *pf = (float *) malloc( 2 * 4 * sizeof(float) );
            *m_pCOVRTable = pf;
            float *pfe = pf;

            *pfe++ = LatMax;
            *pfe++ = LonMin;

            *pfe++ = LatMax;
            *pfe++ = LonMax;

            *pfe++ = LatMin;
            *pfe++ = LonMax;

            *pfe++ = LatMin;
            *pfe++ = LonMin;

        } else {
            qDebug("   Cannot calculate Extents for ENC:  %s", m_TempFilePath.toUtf8().data() );

            return false;                     // chart is completely unusable
        }
    }

    //    Populate the chart's extent structure
    m_FullExtent.NLAT = LatMax;
    m_FullExtent.SLAT = LatMin;
    m_FullExtent.ELON = LonMax;
    m_FullExtent.WLON = LonMin;
    m_bExtentSet = true;

    //    Set the chart scale
    m_Chart_Scale = GetENCScale();

    QString nice_name;
    GetChartNameFromTXT( m_TempFilePath, nice_name );
    m_Name = nice_name;


    return true;
}

//    Read the .S57 oSENC file (CURRENT_SENC_FORMAT_VERSION >= 200) and create required Chartbase data structures
bool s57chart::CreateHeaderDataFromoSENC( void )
{
    bool ret_val = true;
    if (!QFile::exists(m_SENCFileName) )
    {
        qDebug("   Cannot open SENC file :%s", m_SENCFileName.toUtf8().data() );
        return false;
    }

    Osenc senc;
    if(senc.ingestHeader( m_SENCFileName ) ) return false;


    // Get Chartbase member elements from the oSENC file records in the header

    // Scale
    m_Chart_Scale = senc.getSENCReadScale();

    // Nice Name
    m_Name = senc.getReadName();

    // ID
    m_ID = senc.getReadID();

    // Extents
    Extent &ext = senc.getReadExtent();

    m_FullExtent.ELON = ext.ELON;
    m_FullExtent.WLON = ext.WLON;
    m_FullExtent.NLAT = ext.NLAT;
    m_FullExtent.SLAT = ext.SLAT;
    m_bExtentSet = true;


    //Coverage areas
    SENCFloatPtrArray &AuxPtrArray = senc.getSENCReadAuxPointArray();
    std::vector<int> &AuxCntArray = senc.getSENCReadAuxPointCountArray();

    m_nCOVREntries = AuxCntArray.size();

    m_pCOVRTablePoints = (int *) malloc( m_nCOVREntries * sizeof(int) );
    m_pCOVRTable = (float **) malloc( m_nCOVREntries * sizeof(float *) );

    for( unsigned int j = 0; j < (unsigned int) m_nCOVREntries; j++ ) {
        m_pCOVRTablePoints[j] = AuxCntArray[j];
        m_pCOVRTable[j] = (float *) malloc( AuxCntArray[j] * 2 * sizeof(float) );
        memcpy( m_pCOVRTable[j], AuxPtrArray[j],
                AuxCntArray[j] * 2 * sizeof(float) );
    }

    // NoCoverage areas
    SENCFloatPtrArray &NoCovrPtrArray = senc.getSENCReadNOCOVRPointArray();
    std::vector<int> &NoCovrCntArray = senc.getSENCReadNOCOVRPointCountArray();

    m_nNoCOVREntries = NoCovrCntArray.size();

    if( m_nNoCOVREntries ) {
        //    Create new NoCOVR entries
        m_pNoCOVRTablePoints = (int *) malloc( m_nNoCOVREntries * sizeof(int) );
        m_pNoCOVRTable = (float **) malloc( m_nNoCOVREntries * sizeof(float *) );

        for( unsigned int j = 0; j < (unsigned int) m_nNoCOVREntries; j++ ) {
            int npoints = NoCovrCntArray[j];
            m_pNoCOVRTablePoints[j] = npoints;
            m_pNoCOVRTable[j] = (float *) malloc( npoints * 2 * sizeof(float) );
            memcpy( m_pNoCOVRTable[j], NoCovrPtrArray[j],
                    npoints * 2 * sizeof(float) );
        }
    }


    //  Misc
    m_SE = m_edtn000;
    m_datum_str = ("WGS84");
    m_SoundingsDatum = ("MEAN LOWER LOW WATER");


    int senc_file_version = senc.getSencReadVersion();

    int last_update = senc.getSENCReadLastUpdate();

    QString str = senc.getSENCFileCreateDate();
    QDateTime SENCCreateDate = QDateTime::fromString(str, ("yyyyMMdd"));

    //    if( SENCCreateDate.IsValid() )
    //        SENCCreateDate.ResetTime();                   // to midnight

    QString senc_base_edtn = senc.getSENCReadBaseEdition();

    return ret_val;
}



//    Read the .S57 SENC file and create required Chartbase data structures
bool s57chart::CreateHeaderDataFromSENC( void )
{
    bool ret_val = true;
    if(CURRENT_SENC_FORMAT_VERSION >= 200)
        return CreateHeaderDataFromoSENC();

    return false;
    
}

/*    This method returns the smallest chart DEPCNT:VALDCO value which
 is greater than or equal to the specified value
 */
bool s57chart::GetNearestSafeContour( double safe_cnt, double &next_safe_cnt )
{
    int i = 0;
    if( NULL != m_pvaldco_array ) {
        for( i = 0; i < m_nvaldco; i++ ) {
            if( m_pvaldco_array[i] >= safe_cnt ) break;
        }

        if( i < m_nvaldco ) next_safe_cnt = m_pvaldco_array[i];
        else
            next_safe_cnt = (double) 1e6;
        return true;
    } else {
        next_safe_cnt = (double) 1e6;
        return false;
    }
}

/*
 --------------------------------------------------------------------------
 Build a list of "associated" DEPARE and DRGARE objects from a given
 object. to be "associated" means to be physically intersecting,
 overlapping, or contained within, depending upon the geometry type
 of the given object.
 --------------------------------------------------------------------------
 */

ListOfS57Obj *s57chart::GetAssociatedObjects( S57Obj *obj )
{
    int disPrioIdx;
    bool gotit;

    ListOfS57Obj *pobj_list = new ListOfS57Obj;
    pobj_list->clear();

    double lat, lon;
    fromSM( ( obj->x * obj->x_rate ) + obj->x_origin, ( obj->y * obj->y_rate ) + obj->y_origin,
            ref_lat, ref_lon, &lat, &lon );
    //    What is the entry object geometry type?

    switch( obj->Primitive_type ){
    case GEO_POINT:
        //  n.b.  This logic not perfectly right for LINE and AREA features
        //  It uses the object reference point for testing, instead of the decomposed
        //  line or boundary geometry.  Thus, it may fail on some intersecting relationships.
        //  Judged acceptable, in favor of performance implications.
        //  DSR
    case GEO_LINE:
    case GEO_AREA:
        ObjRazRules *top;
        disPrioIdx = 1;         // PRIO_GROUP1:S57 group 1 filled areas

        gotit = false;
        top = razRules[disPrioIdx][3];     // PLAIN_BOUNDARIES
        while( top != NULL ) {
            if( top->obj->bIsAssociable ) {
                if( top->obj->BBObj.Contains( lat, lon ) ) {
                    if( IsPointInObjArea( lat, lon, 0.0, top->obj ) ) {
                        pobj_list->append( *(top->obj) );
                        gotit = true;
                        break;
                    }
                }
            }

            ObjRazRules *nxx = top->next;
            top = nxx;
        }

        if( !gotit ) {
            top = razRules[disPrioIdx][4];     // SYMBOLIZED_BOUNDARIES
            while( top != NULL ) {
                if( top->obj->bIsAssociable ) {
                    if( top->obj->BBObj.Contains( lat, lon ) ) {
                        if( IsPointInObjArea( lat, lon, 0.0, top->obj ) ) {
                            pobj_list->append( *(top->obj) );
                            break;
                        }
                    }
                }

                ObjRazRules *nxx = top->next;
                top = nxx;
            }
        }

        break;

    default:
        break;
    }

    return pobj_list;
}

void s57chart::GetChartNameFromTXT( const QString& FullPath, QString &Name )
{

    QFileInfo fn( FullPath );

    QString target_name = zchxFuncUtil::getFileName(fn.fileName());
    target_name.remove(target_name.size()-1, 1);

    QString dir_name = fn.absolutePath();

    QDir dir( dir_name );                         // The directory containing the file

    QFileInfoList FileList = dir.entryInfoList();
    //    Iterate on the file list...

    bool found_name = false;
    QString name;

    for( unsigned int j = 0; j < FileList.count(); j++ ) {
        QFileInfo file( FileList[j] );
        QString ext = zchxFuncUtil::getFileExt(file.fileName());
        if( ext.toUpper() == ("TXT") ) {
            //  Look for the line beginning with the name of the .000 file
            QFile text_file( file.absoluteFilePath() );
            //  Suppress log messages on bad file reads
            if( !text_file.open(QIODevice::ReadOnly) ) {
                //                if( !text_file.Open(wxConvISO8859_1) )
                qDebug("   Error Reading ENC .TXT file: %s", file.absoluteFilePath().toUtf8().data() );
                continue;
            }

            while( 1 ) {
                QString str = text_file.readLine();
                if( 0 == target_name.compare( str.mid( 0, target_name.length() ), Qt::CaseInsensitive ) ) { // found it
                    QString tname = str.mid( str.indexOf('-') + 1);
                    name = tname.mid( tname.indexOf(' ') + 1);
                    found_name = true;
                    break;
                }
                if(text_file.atEnd()) break;
            }
            text_file.close();
            if( found_name ) break;
        }
    }

    Name = name;

}

//---------------------------------------------------------------------------------
//      S57 Database methods
//---------------------------------------------------------------------------------

//-------------------------------
//
// S57 OBJECT ACCESSOR SECTION
//
//-------------------------------

const char *s57chart::getName( OGRFeature *feature )
{
    return feature->GetDefnRef()->GetName();
}

static int ExtensionCompare( const QString& first, const QString& second )
{
    QFileInfo fn1( first );
    QFileInfo fn2( second );
    QString ext1( fn1.suffix() );
    QString ext2( fn2.suffix() );

    return ext1.compare(ext2 );
}


int s57chart::GetUpdateFileArray( const QFileInfo& file000, QStringList *UpFiles,
                                  QDateTime date000, QString edtn000)
{
    QString DirName000 = file000.absolutePath();
    QDir dir( DirName000 );
    if(!dir.exists()){
        DirName000.insert(0, zchxFuncUtil::separator());
        DirName000.insert(0, ".");
        dir.setPath(DirName000);
        if(!dir.exists()){
            return 0;
        }
    }
    
    int flags = QDir::NoFilter;
    
    // Check dir structure
    //  We look to see if the directory one level above where the .000 file is located happens to be "perfectly numeric" in name.
    //  If so, the dataset is presumed to be organized with each update in its own directory.
    //  So, we search for updates from this level, recursing into subdirs.
    //    wxFileName fnDir( DirName000 );
    //    fnDir.RemoveLastDir();
    //    QString sdir = fnDir.GetPath();
    //    wxFileName fnTest(sdir);
    //    QString sname = fnTest.GetName();
    QString sdir = DirName000;
    if(sdir.right(1) == zchxFuncUtil::separator())
    {
        sdir.remove(sdir.size() - 1, 1);
    }
    int last_index = sdir.lastIndexOf("/");
    if(last_index >= 0) sdir.remove(last_index, sdir.size() - last_index);
    QDir fnTest(sdir);
    QString sname = fnTest.dirName();
    long tmps = 0;
    bool ok = false;
    tmps = sname.toLong(&ok);
    if(ok){
        //        dir.Open(sdir);
        DirName000 = sdir;
        flags |= QDir::Dirs;
        dir.setPath(DirName000);
    }
    
    QString ext;
    QStringList *dummy_array;
    int retval = 0;
    
    if( UpFiles == NULL )
        dummy_array = new QStringList;
    else
        dummy_array = UpFiles;
    
    //这里需要获取的带路径的全名
    QFileInfoList possibleFiles = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for(unsigned int i=0 ; i < possibleFiles.count() ; i++){
        QFileInfo file(possibleFiles[i]);
        QString filename = file.absoluteFilePath();
        ext = file.suffix();
        ok = true;
        long tmp = ext.toLong(&ok);
        //  Files of interest have the same base name is the target .000 cell,
        //  and have numeric extension
        if( ok && ( file.fileName() == file000.fileName() ) ) {
            QString FileToAdd = filename;
            
            QByteArray buffer = FileToAdd.toUtf8();             // Check file namme for convertability
            
            if( buffer.data() && !zchxFuncUtil::isSameAs(filename, "CATALOG.031", false ) )           // don't process catalogs
            {
                //          We must check the update file for validity
                //          1.  Is update field DSID:EDTN  equal to base .000 file DSID:EDTN?
                //          2.  Is update file DSID.ISDT greater than or equal to base .000 file DSID:ISDT
                
                QDateTime umdate;
                QString sumdate;
                QString umedtn;
                DDFModule *poModule = new DDFModule();
                if( !poModule->Open( FileToAdd.toUtf8().data() ) ) {
                    qDebug("   s57chart::BuildS57File  Unable to open update file %s", FileToAdd.toUtf8().data() );
                } else {
                    poModule->Rewind();
                    
                    //    Read and parse DDFRecord 0 to get some interesting data
                    //    n.b. assumes that the required fields will be in Record 0....  Is this always true?
                    
                    DDFRecord *pr = poModule->ReadRecord();                              // Record 0
                    //    pr->Dump(stdout);
                    
                    //  Fetch ISDT(Issue Date)
                    char *u = NULL;
                    if( pr ) {
                        u = (char *) ( pr->GetStringSubfield( "DSID", 0, "ISDT", 0 ) );
                        
                        if( u ) {
                            if( strlen( u ) ) sumdate = QString::fromUtf8(u );
                        }
                    } else {
                        qDebug("   s57chart::BuildS57File  DDFRecord 0 does not contain DSID:ISDT in update file %s", FileToAdd.toUtf8().data() );
                        sumdate = ("20000101");           // backstop, very early, so wont be used
                    }
                    
                    umdate = QDateTime::fromString(sumdate, ("yyyyMMdd") );
                    if( !umdate.isValid() ) umdate  = QDateTime::fromString( "20000101", "yyyyMMdd" );

                    //                                     umdate.ResetTime();
                    
                    //    Fetch the EDTN(Edition) field
                    if( pr ) {
                        u = NULL;
                        u = (char *) ( pr->GetStringSubfield( "DSID", 0, "EDTN", 0 ) );
                        if( u ) {
                            if( strlen( u ) ) umedtn = QString::fromUtf8(u);
                        }
                    } else {
                        qDebug("   s57chart::BuildS57File  DDFRecord 0 does not contain DSID:EDTN in update file %s", FileToAdd.toUtf8().data() );

                        umedtn = ("1");                // backstop
                    }
                }
                
                delete poModule;
                
                if( ( !(umdate < date000 ) ) && ( umedtn == edtn000  ) ) // Note polarity on Date compare....
                    dummy_array->append( FileToAdd );                    // Looking for umdate >= m_date000
            }
        }
    }
    
    //      Sort the candidates
    qSort(dummy_array->begin(), dummy_array->end(), ExtensionCompare);
    //      Get the update number of the last in the list
    if( dummy_array->count() ) {
        QString Last = dummy_array->last();
        QFileInfo fnl( Last );
        ext = fnl.suffix();
        QByteArray buffer=ext.toUtf8();
        if(buffer.data())
            retval = atoi( buffer.data() );
    }
    
    if( UpFiles == NULL ) delete dummy_array;

    return retval;
}


int s57chart::ValidateAndCountUpdates( const QFileInfo& file000, const QString CopyDir,
                                       QString &LastUpdateDate, bool b_copyfiles )
{

    int retval = 0;

    //       QString DirName000 = file000.GetPath((int)(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));
    //       wxDir dir(DirName000);
    QStringList *UpFiles = new QStringList;
    retval = GetUpdateFileArray( file000, UpFiles, m_date000, m_edtn000);

    if( UpFiles->count() ) {
        //      The s57reader of ogr requires that update set be sequentially complete
        //      to perform all the updates.  However, some NOAA ENC distributions are
        //      not complete, as apparently some interim updates have been  withdrawn.
        //      Example:  as of 20 Dec, 2005, the update set for US5MD11M.000 includes
        //      US5MD11M.017, ...018, and ...019.  Updates 001 through 016 are missing.
        //
        //      Workaround.
        //      Create temporary dummy update files to fill out the set before invoking
        //      ogr file open/ingest.  Delete after SENC file create finishes.
        //      Set starts with .000, which has the effect of copying the base file to the working dir

        bool chain_broken_mssage_shown = false;

        if( b_copyfiles ) {
            m_tmpup_array = new QStringList;       // save a list of created files for later erase

            for( int iff = 0; iff < retval + 1; iff++ ) {
                QString newFile = zchxFuncUtil::getNewFileNameWithExt(m_TempFilePath, QString("").sprintf("%03d", iff ));
                QFileInfo ufile( newFile );

                //      Create the target update file name
                QString cp_ufile = CopyDir;
                if( cp_ufile.right(1) != zchxFuncUtil::separator() ) cp_ufile.append(zchxFuncUtil::separator());
                cp_ufile.append( ufile.fileName() );

                //      Explicit check for a short update file, possibly left over from a crash...
                int flen = 0;
                if( ufile.exists() ) flen = ufile.size();
                if( flen > 25  )        // a valid update file or base file
                {
                    //      Copy the valid file to the SENC directory
                    if(!QFile::copy(ufile.absoluteFilePath(), cp_ufile))
                    {
                        QString msg( ("   Cannot copy temporary working ENC file ") );
                        msg.append( ufile.absoluteFilePath() );
                        msg.append( (" to ") );
                        msg.append( cp_ufile );
                        qDebug()<<msg;
                    }
                }

                else {
                    // Create a dummy ISO8211 file with no real content
                    // Correct this.  We should break the walk, and notify the user  See FS#1406

                    if( !chain_broken_mssage_shown ){
                        QMessageBox::warning(0,
                                             "OpenCPN Create SENC Warning",
                                             "S57 Cell Update chain incomplete.\nENC features may be incomplete or inaccurate.\nCheck the logfile for details.");
                        chain_broken_mssage_shown = true;
                    }

                    QString msg( ("WARNING---ENC Update chain incomplete. Substituting NULL update file: "));
                    msg += ufile.absoluteFilePath();
                    qDebug()<<msg;
                    qDebug("   Subsequent ENC updates may produce errors.") ;
                    qDebug("   This ENC exchange set should be updated and SENCs rebuilt.") ;

                    bool bstat;
                    DDFModule *dupdate = new DDFModule;
                    dupdate->Initialize( '3', 'L', 'E', '1', '0', "!!!", 3, 4, 4 );
                    bstat = !( dupdate->Create( cp_ufile.toUtf8().data() ) == 0 );
                    delete dupdate;

                    if( !bstat ) {
                        QString msg( ("   Error creating dummy update file: ") );
                        msg.append( cp_ufile );
                        qDebug()<< msg;
                    }
                }

                m_tmpup_array->append( cp_ufile );
            }
        }

        //      Extract the date field from the last of the update files
        //      which is by definition a valid, present update file....
        QString  lastFileName = zchxFuncUtil::getNewFileNameWithExt(m_TempFilePath, QString("").sprintf("%03d", retval));
        QFileInfo lastfile( lastFileName );
        bool bSuccess;
        DDFModule oUpdateModule;
        bSuccess = !( oUpdateModule.Open( lastfile.absoluteFilePath().toUtf8().data(), true ) == 0 );

        if( bSuccess ) {
            //      Get publish/update date
            oUpdateModule.Rewind();
            DDFRecord *pr = oUpdateModule.ReadRecord();                     // Record 0

            int nSuccess;
            char *u = NULL;

            if( pr ) u = (char *) ( pr->GetStringSubfield( "DSID", 0, "ISDT", 0, &nSuccess ) );

            if( u ) {
                if( strlen( u ) ) {
                    LastUpdateDate = QString::fromUtf8( u);
                }
            } else {
                QDateTime now = QDateTime::currentDateTime();
                LastUpdateDate = now.toString("yyyyMMdd") ;
            }
        }
    }

    delete UpFiles;
    return retval;
}


QString s57chart::GetISDT( void )
{
    if( m_date000.isValid() ) return m_date000.toString("yyyyMMdd" );
    else
        return ("Unknown");
}

bool s57chart::GetBaseFileAttr( const QString& file000 )
{
    if( !QFile::exists( file000 ) ) return false;

    QString FullPath000 = file000;
    DDFModule *poModule = new DDFModule();
    if( !poModule->Open( FullPath000.toUtf8().data() ) ) {
        QString msg( ("   s57chart::BuildS57File  Unable to open ") );
        msg.append( FullPath000 );
        qDebug()<< msg ;
        delete poModule;
        return false;
    }

    poModule->Rewind();

    //    Read and parse DDFRecord 0 to get some interesting data
    //    n.b. assumes that the required fields will be in Record 0....  Is this always true?

    DDFRecord *pr = poModule->ReadRecord();                               // Record 0
    //    pr->Dump(stdout);

    //    Fetch the Geo Feature Count, or something like it....
    m_nGeoRecords = pr->GetIntSubfield( "DSSI", 0, "NOGR", 0 );
    if( !m_nGeoRecords ) {
        QString msg( ("   s57chart::BuildS57File  DDFRecord 0 does not contain DSSI:NOGR ") );
        qDebug()<< msg ;

        m_nGeoRecords = 1;                // backstop
    }

    //  Use ISDT(Issue Date) here, which is the same as UADT(Updates Applied) for .000 files
    QString date000;
    char *u = (char *) ( pr->GetStringSubfield( "DSID", 0, "ISDT", 0 ) );
    if( u ) date000 = QString::fromUtf8( u );
    else {
        QString msg( ("   s57chart::BuildS57File  DDFRecord 0 does not contain DSID:ISDT ") );
        qDebug()<< msg ;

        date000 = ("20000101");             // backstop, very early, so any new files will update?
    }
    m_date000 = QDateTime::fromString( date000, ("yyyyMMdd") );
    if( !m_date000.isValid() ) m_date000 = QDateTime::fromString(("20000101"), ("yyyyMMdd") );

    //    m_date000.ResetTime();

    //    Fetch the EDTN(Edition) field
    u = (char *) ( pr->GetStringSubfield( "DSID", 0, "EDTN", 0 ) );
    if( u ) m_edtn000 = QString::fromUtf8(u );
    else {
        QString msg( ("   s57chart::BuildS57File  DDFRecord 0 does not contain DSID:EDTN ") );
        qDebug()<< msg ;

        m_edtn000 = ("1");                // backstop
    }

    m_SE = m_edtn000;

    //      Fetch the Native Scale by reading more records until DSPM is found
    m_native_scale = 0;
    for( ; pr != NULL; pr = poModule->ReadRecord() ) {
        if( pr->FindField( "DSPM" ) != NULL ) {
            m_native_scale = pr->GetIntSubfield( "DSPM", 0, "CSCL", 0 );
            break;
        }
    }
    if( !m_native_scale ) {
        QString msg( ("   s57chart::BuildS57File  ENC not contain DSPM:CSCL ") );
        qDebug()<< msg ;

        m_native_scale = 1000;                // backstop
    }

    delete poModule;

    return true;
}

int s57chart::BuildSENCFile( const QString& FullPath000, const QString& SENCFileName, bool b_progress )
{
    
    //  LOD calculation
    double display_ppm = 1 / .00025;     // nominal for most LCD displays
    double meters_per_pixel_max_scale = GetNormalScaleMin(0,g_b_overzoom_x)/display_ppm;
    m_LOD_meters = meters_per_pixel_max_scale * g_SENC_LOD_pixels;

    //  Establish a common reference point for the chart
    ref_lat = ( m_FullExtent.NLAT + m_FullExtent.SLAT ) / 2.;
    ref_lon = ( m_FullExtent.WLON + m_FullExtent.ELON ) / 2.;

    if(!m_disableBackgroundSENC){
        if(g_SencThreadManager){
            SENCJobTicket *ticket = new SENCJobTicket();
            ticket->m_LOD_meters = m_LOD_meters;
            ticket->ref_lat = ref_lat;
            ticket->ref_lon = ref_lon;
            ticket->m_FullPath000 = FullPath000;
            ticket->m_SENCFileName = SENCFileName;
            ticket->m_chart = this;
            
            m_SENCthreadStatus = g_SencThreadManager->appendJob(ticket);
            bReadyToRender = true;
            return BUILD_SENC_PENDING;

        }
        else
            return BUILD_SENC_NOK_RETRY;

    }
    else{
        Osenc senc;

        senc.setRegistrar( g_poRegistrar );
        senc.setRefLocn(ref_lat, ref_lon);
        senc.SetLODMeters(m_LOD_meters);

//        OCPNPlatform::ShowBusySpinner();

        int ret = senc.createSenc200( FullPath000, SENCFileName, b_progress );

//        OCPNPlatform::HideBusySpinner();
        
        if(ret == ERROR_INGESTING000)
            return BUILD_SENC_NOK_PERMANENT;
        else
            return ret;
    }
}




int s57chart::BuildRAZFromSENCFile( const QString& FullPath )
{
    int ret_val = 0;                    // default is OK

    Osenc sencfile;

    // Set up the containers for ingestion results.
    // These will be populated by Osenc, and owned by the caller (this).
    S57ObjVector Objects;
    VE_ElementVector VEs;
    VC_ElementVector VCs;

    sencfile.setRefLocn(ref_lat, ref_lon);

    int srv = sencfile.ingest200(FullPath, &Objects, &VEs, &VCs);

    if(srv != SENC_NO_ERROR){
        qDebug()<< sencfile.getLastError() ;
        //TODO  Clean up here, or massive leaks result
        return 1;
    }

    //  Get the cell Ref point as recorded in the SENC
    Extent ext = sencfile.getReadExtent();

    m_FullExtent.ELON = ext.ELON;
    m_FullExtent.WLON = ext.WLON;
    m_FullExtent.NLAT = ext.NLAT;
    m_FullExtent.SLAT = ext.SLAT;
    m_bExtentSet = true;

    ref_lat = (ext.NLAT + ext.SLAT) / 2.;
    ref_lon = (ext.ELON + ext.WLON) / 2.;

    //  Process the Edge feature arrays.

    //    Create a hash map of VE_Element pointers as a chart class member
    int n_ve_elements = VEs.size();

//    qDebug()<<"vc size:"<<VCs.size()<<" ve size:"<<VEs.size();
    double scale = gChartFrameWork->GetBestVPScale(this);
    int nativescale = GetNativeScale();

    for( int i = 0; i < n_ve_elements; i++ ) {

        VE_Element *vep = VEs.at( i );
        if(vep && vep->nCount){
            //  Get a bounding box for the edge
            double east_max = -1e7; double east_min = 1e7;
            double north_max = -1e7; double north_min = 1e7;

            float *vrun = vep->pPoints;
            for(size_t i=0 ; i < vep->nCount; i++){
                east_max = fmax(east_max, *vrun);
                east_min = fmin(east_min, *vrun);
                vrun++;

                north_max = fmax(north_max, *vrun);
                north_min = fmin(north_min, *vrun);
                vrun++;
            }

            double lat1, lon1, lat2, lon2;
            fromSM( east_min, north_min, ref_lat, ref_lon, &lat1, &lon1 );
            fromSM( east_max, north_max, ref_lat, ref_lon, &lat2, &lon2 );
            vep->edgeBBox.Set( lat1, lon1, lat2, lon2);
        }

        m_ve_hash[vep->index] = vep;
    }


    //    Create a hash map VC_Element pointers as a chart class member
    int n_vc_elements = VCs.size();

    for( int i = 0; i < n_vc_elements; i++ ) {
        VC_Element *vcp = VCs.at( i );
        m_vc_hash[vcp->index] = vcp;
    }


    VEs.clear();        // destroy contents, no longer needed
    VCs.clear();
    
    //Walk the vector of S57Objs, associating LUPS, instructions, etc...

    for(unsigned int i=0 ; i < Objects.size() ; i++){

        S57Obj *obj = Objects[i];

        //      This is where Simplified or Paper-Type point features are selected
        LUPrec *LUP;
        LUPname LUP_Name = PAPER_CHART;

        const QString objnam  = obj->GetAttrValueAsString("OBJNAM");
        if (objnam.length() > 0) {
            const QString fe_name = QString(obj->FeatureName);
//            g_pi_manager->SendVectorChartObjectInfo( FullPath, fe_name, objnam, obj->m_lat, obj->m_lon, scale, nativescale );
        }
        //If there is a localized object name and it actually is different from the object name, send it as well...
        const QString nobjnam  = obj->GetAttrValueAsString("NOBJNM");
        if (nobjnam.length() > 0 && nobjnam != objnam) {
            const QString fe_name = QString(obj->FeatureName);
//            g_pi_manager->SendVectorChartObjectInfo( FullPath, fe_name, nobjnam, obj->m_lat, obj->m_lon, scale, nativescale );
        }

        switch( obj->Primitive_type ){
        case GEO_POINT:
        case GEO_META:
        case GEO_PRIM:

            if( PAPER_CHART == ps52plib->m_nSymbolStyle )
                LUP_Name = PAPER_CHART;
            else
                LUP_Name = SIMPLIFIED;

            break;

        case GEO_LINE:
            LUP_Name = LINES;
            break;

        case GEO_AREA:
            if( PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle )
                LUP_Name = PLAIN_BOUNDARIES;
            else
                LUP_Name = SYMBOLIZED_BOUNDARIES;

            break;
        }

        LUP = ps52plib->S52_LUPLookup( LUP_Name, obj->FeatureName, obj );

        if( NULL == LUP ) {
            qDebug("   Could not find LUP for :%s", obj->FeatureName );
            delete obj;
            obj = NULL;
            Objects[i] = NULL;
        } else {
            //              Convert LUP to rules set
            ps52plib->_LUP2rules( LUP, obj );

            //              Add linked object/LUP to the working set
            _insertRules( obj, LUP, this );

            //              Establish Object's Display Category
            obj->m_DisplayCat = LUP->DISC;

            //              Establish objects base display priority
            obj->m_DPRI = LUP->DPRI - '0';

            //  Is this a category-movable object?
            if( !strncmp(obj->FeatureName, "OBSTRN", 6) ||
                    !strncmp(obj->FeatureName, "WRECKS", 6) ||
                    !strncmp(obj->FeatureName, "DEPCNT", 6) ||
                    !strncmp(obj->FeatureName, "UWTROC", 6) )
            {
                obj->m_bcategory_mutable = true;
            }
            else{
                obj->m_bcategory_mutable = false;
            }
        }

        //      Build/Maintain the ATON floating/rigid arrays
        if( obj && (GEO_POINT == obj->Primitive_type) ) {

            // set floating platform
            if( ( !strncmp( obj->FeatureName, "LITFLT", 6 ) )
                    || ( !strncmp( obj->FeatureName, "LITVES", 6 ) )
                    || ( !strncasecmp( obj->FeatureName, "BOY", 3 ) ) ) {
                pFloatingATONArray->append( obj );
            }

            // set rigid platform
            if( !strncasecmp( obj->FeatureName, "BCN", 3 ) ) {
                pRigidATONArray->append( obj );
            }


            //    Mark the object as an ATON
            if( ( !strncmp( obj->FeatureName, "LIT", 3 ) )
                    || ( !strncmp( obj->FeatureName, "LIGHTS", 6 ) )
                    || ( !strncasecmp( obj->FeatureName, "BCN", 3 ) )
                    || ( !strncasecmp( obj->FeatureName, "BOY", 3 ) ) ) {
                obj->bIsAton = true;
            }
        }

    }   // Objects iterator


    //   Decide on pub date to show

    QDateTime d000 = QDateTime::fromString(sencfile.getBaseDate(), ("yyyyMMdd") );
    if( !d000.isValid() )
        d000= QDateTime::fromString(("20000101"), ("yyyyMMdd") );

    QDateTime updt = QDateTime::fromString( sencfile.getUpdateDate(), ("yyyyMMdd") );
    if( !updt.isValid() )
        updt = QDateTime::fromString( ("20000101"), ("yyyyMMdd") );

    if(updt < d000)
        m_PubYear.sprintf("%4d", updt.date().year());
    else
        m_PubYear.sprintf("%4d", d000.date().year());



    //    Set some base class values
    QDateTime upd = updt;
    if( !upd.isValid() )
        upd = QDateTime::fromString( "20000101", ("yyyyMMdd") );

    //    upd.ResetTime();
    m_EdDate = upd;

    m_SE = sencfile.getSENCReadBaseEdition();

    QString supdate;
    supdate.sprintf(" / %d", sencfile.getSENCReadLastUpdate());
    m_SE += supdate;


    m_datum_str = ("WGS84");

    m_SoundingsDatum = ("MEAN LOWER LOW WATER");
    m_ID = sencfile.getReadID();
    m_Name = sencfile.getReadName();

    ObjRazRules *top;

    AssembleLineGeometry();

    //  Set up the chart context
    m_this_chart_context = (chart_context *)calloc( sizeof(chart_context), 1);
    m_this_chart_context->chart = this;
    m_this_chart_context->vertex_buffer = GetLineVertexBuffer();

    //  Loop and populate all the objects
    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            top = razRules[i][j];
            while( top != NULL ) {
                S57Obj *obj = top->obj;
                obj->m_chart_context = m_this_chart_context;
                top = top->next;
            }
        }
    }


    return ret_val;
}



int s57chart::_insertRules( S57Obj *obj, LUPrec *LUP, s57chart *pOwner )
{
    ObjRazRules *rzRules = NULL;
    int disPrioIdx = 0;
    int LUPtypeIdx = 0;

    if( LUP == NULL ) {
        //      printf("SEQuencer:_insertRules(): ERROR no rules to insert!!\n");
        return 0;
    }

    // find display priority index       --talky version
    switch( LUP->DPRI ){
    case PRIO_NODATA:
        disPrioIdx = 0;
        break;  // no data fill area pattern
    case PRIO_GROUP1:
        disPrioIdx = 1;
        break;  // S57 group 1 filled areas
    case PRIO_AREA_1:
        disPrioIdx = 2;
        break;  // superimposed areas
    case PRIO_AREA_2:
        disPrioIdx = 3;
        break;  // superimposed areas also water features
    case PRIO_SYMB_POINT:
        disPrioIdx = 4;
        break;  // point symbol also land features
    case PRIO_SYMB_LINE:
        disPrioIdx = 5;
        break;  // line symbol also restricted areas
    case PRIO_SYMB_AREA:
        disPrioIdx = 6;
        break;  // area symbol also traffic areas
    case PRIO_ROUTEING:
        disPrioIdx = 7;
        break;  // routeing lines
    case PRIO_HAZARDS:
        disPrioIdx = 8;
        break;  // hazards
    case PRIO_MARINERS:
        disPrioIdx = 9;
        break;  // VRM & EBL, own ship
    default:
        printf( "SEQuencer:_insertRules():ERROR no display priority!!!\n" );
    }

    // find look up type index
    switch( LUP->TNAM ){
    case SIMPLIFIED:
        LUPtypeIdx = 0;
        break; // points
    case PAPER_CHART:
        LUPtypeIdx = 1;
        break; // points
    case LINES:
        LUPtypeIdx = 2;
        break; // lines
    case PLAIN_BOUNDARIES:
        LUPtypeIdx = 3;
        break; // areas
    case SYMBOLIZED_BOUNDARIES:
        LUPtypeIdx = 4;
        break; // areas
    default:
        printf( "SEQuencer:_insertRules():ERROR no look up type !!!\n" );
    }

    // insert rules
    rzRules = (ObjRazRules *) malloc( sizeof(ObjRazRules) );
    rzRules->obj = obj;
    obj->nRef++;                         // Increment reference counter for delete check;
    rzRules->LUP = LUP;
    //    rzRules->chart = pOwner;
    rzRules->next = razRules[disPrioIdx][LUPtypeIdx];
    rzRules->child = NULL;
    rzRules->mps = NULL;
    razRules[disPrioIdx][LUPtypeIdx] = rzRules;

    return 1;
}

void s57chart::ResetPointBBoxes( const ViewPort &vp_last, const ViewPort &vp_this )
{
    ObjRazRules *top;
    ObjRazRules *nxx;

    double d = vp_last.viewScalePPM() / vp_this.viewScalePPM();

    for( int i = 0; i < PRIO_NUM; ++i ) {
        for( int j = 0; j < 2; ++j ) {
            top = razRules[i][j];

            while( top != NULL ) {
                if( !top->obj->geoPtMulti )                      // do not reset multipoints
                {
                    if(top->obj->BBObj.GetValid()) { // scale bbobj
                        double lat = top->obj->m_lat, lon = top->obj->m_lon;

                        double lat1 = (lat - top->obj->BBObj.GetMinLat()) * d;
                        double lat2 = (lat - top->obj->BBObj.GetMaxLat()) * d;

                        double minlon = top->obj->BBObj.GetMinLon();
                        double maxlon = top->obj->BBObj.GetMaxLon();

                        double lon1 = (lon - minlon) * d;
                        double lon2 = (lon - maxlon) * d;

                        top->obj->BBObj.Set( lat - lat1, lon - lon1,
                                             lat - lat2, lon - lon2 );

                        // this method is very close, but errors accumulate
                        top->obj->BBObj.Invalidate();
                    }
                }

                nxx = top->next;
                top = nxx;
            }
        }
    }
}

//      Traverse the ObjRazRules tree, and fill in
//      any Lups/rules not linked on initial chart load.
//      For example, if chart was loaded with PAPER_CHART symbols,
//      locate and load the equivalent SIMPLIFIED symbology.
//      Likewise for PLAIN/SYMBOLIZED boundaries.
//
//      This method is usually called after a chart display style
//      change via the "Options" dialog, to ensure all symbology is
//      present iff needed.

void s57chart::UpdateLUPs( s57chart *pOwner )
{
    ObjRazRules *top;
    ObjRazRules *nxx;
    LUPrec *LUP;
    for( int i = 0; i < PRIO_NUM; ++i ) {
        //  SIMPLIFIED is set, PAPER_CHART is bare
        if( ( razRules[i][0] ) && ( NULL == razRules[i][1] ) ) {
            m_b2pointLUPS = true;
            top = razRules[i][0];

            while( top != NULL ) {
                LUP = ps52plib->S52_LUPLookup( PAPER_CHART, top->obj->FeatureName, top->obj );
                if( LUP ) {
                    //  A POINT object can only appear in two places in the table, SIMPLIFIED or PAPER_CHART
                    //  although it is allowed for the Display priority to be different for each
                    if(top->obj->nRef < 2) {
                        ps52plib->_LUP2rules( LUP, top->obj );
                        _insertRules( top->obj, LUP, pOwner );
                        top->obj->m_DisplayCat = LUP->DISC;
                    }
                }

                nxx = top->next;
                top = nxx;
            }
        }

        //  PAPER_CHART is set, SIMPLIFIED is bare
        if( ( razRules[i][1] ) && ( NULL == razRules[i][0] ) ) {
            m_b2pointLUPS = true;
            top = razRules[i][1];

            while( top != NULL ) {
                LUP = ps52plib->S52_LUPLookup( SIMPLIFIED, top->obj->FeatureName, top->obj );
                if( LUP ) {
                    if(top->obj->nRef < 2) {
                        ps52plib->_LUP2rules( LUP, top->obj );
                        _insertRules( top->obj, LUP, pOwner );
                        top->obj->m_DisplayCat = LUP->DISC;
                    }
                }

                nxx = top->next;
                top = nxx;
            }
        }

        //  PLAIN_BOUNDARIES is set, SYMBOLIZED_BOUNDARIES is bare
        if( ( razRules[i][3] ) && ( NULL == razRules[i][4] ) ) {
            m_b2lineLUPS = true;
            top = razRules[i][3];

            while( top != NULL ) {
                LUP = ps52plib->S52_LUPLookup( SYMBOLIZED_BOUNDARIES, top->obj->FeatureName,
                                               top->obj );
                if( LUP ) {
                    ps52plib->_LUP2rules( LUP, top->obj );
                    _insertRules( top->obj, LUP, pOwner );
                    top->obj->m_DisplayCat = LUP->DISC;
                }

                nxx = top->next;
                top = nxx;
            }
        }

        //  SYMBOLIZED_BOUNDARIES is set, PLAIN_BOUNDARIES is bare
        if( ( razRules[i][4] ) && ( NULL == razRules[i][3] ) ) {
            m_b2lineLUPS = true;
            top = razRules[i][4];

            while( top != NULL ) {
                LUP = ps52plib->S52_LUPLookup( PLAIN_BOUNDARIES, top->obj->FeatureName, top->obj );
                if( LUP ) {
                    ps52plib->_LUP2rules( LUP, top->obj );
                    _insertRules( top->obj, LUP, pOwner );
                    top->obj->m_DisplayCat = LUP->DISC;
                }

                nxx = top->next;
                top = nxx;
            }
        }

        //  Traverse this priority level again,
        //  clearing any object CS rules and flags,
        //  so that the next render operation will re-evaluate the CS

        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            top = razRules[i][j];
            while( top != NULL ) {
                top->obj->bCS_Added = 0;
                free_mps( top->mps );
                top->mps = 0;
                if (top->LUP)
                    top->obj->m_DisplayCat = top->LUP->DISC;

                nxx = top->next;
                top = nxx;
            }
        }

        //  Traverse this priority level again,
        //  clearing any object CS rules and flags of any child list,
        //  so that the next render operation will re-evaluate the CS

        for( int j = 0; j < LUPNAME_NUM; j++ ) {
            top = razRules[i][j];
            while( top != NULL ) {
                if( top->child ) {
                    ObjRazRules *ctop = top->child;
                    while( NULL != ctop ) {
                        ctop->obj->bCS_Added = 0;
                        free_mps( ctop->mps );
                        ctop->mps = 0;

                        if (ctop->LUP)
                            ctop->obj->m_DisplayCat = ctop->LUP->DISC;
                        ctop = ctop->next;
                    }
                }
                nxx = top->next;
                top = nxx;
            }
        }

    }

    //    Clear the dynamically created Conditional Symbology LUP Array
    // This can not be done on a per-chart basis, since the plib services all charts
    // TODO really should make the dynamic LUPs belong to the chart class that created them
}


ListOfObjRazRules *s57chart::GetObjRuleListAtLatLon( float lat, float lon, float select_radius,
                                                     ViewPort *VPoint, int selection_mask )
{

    ListOfObjRazRules *ret_ptr = new ListOfObjRazRules;

    //    Iterate thru the razRules array, by object/rule type

    ObjRazRules *top;

    for( int i = 0; i < PRIO_NUM; ++i ) {

        if(selection_mask & MASK_POINT){
            // Points by type, array indices [0..1]

            int point_type = ( ps52plib->m_nSymbolStyle == SIMPLIFIED ) ? 0 : 1;
            top = razRules[i][point_type];

            while( top != NULL ) {
                if( top->obj->npt == 1 )       // Do not select Multipoint objects (SOUNDG) yet.
                {
                    if( ps52plib->ObjectRenderCheck( top, VPoint ) ) {
                        if( DoesLatLonSelectObject( lat, lon, select_radius, top->obj ) )
                            ret_ptr->append( *top );
                    }
                }

                //    Check the child branch, if any.
                //    This is where Multipoint soundings are captured individually
                if( top->child ) {
                    ObjRazRules *child_item = top->child;
                    while( child_item != NULL ) {
                        if( ps52plib->ObjectRenderCheck( child_item, VPoint ) ) {
                            if( DoesLatLonSelectObject( lat, lon, select_radius, child_item->obj ) )
                                ret_ptr->append( *child_item );
                        }

                        child_item = child_item->next;
                    }
                }

                top = top->next;
            }
        }

        if(selection_mask & MASK_AREA){
            // Areas by boundary type, array indices [3..4]

            int area_boundary_type = ( ps52plib->m_nBoundaryStyle == PLAIN_BOUNDARIES ) ? 3 : 4;
            top = razRules[i][area_boundary_type];           // Area nnn Boundaries
            while( top != NULL ) {
                if( ps52plib->ObjectRenderCheck( top, VPoint ) ) {
                    if( DoesLatLonSelectObject( lat, lon, select_radius, top->obj ) ) ret_ptr->append(*top );
                }

                top = top->next;
            }         // while
        }

        if(selection_mask & MASK_LINE){
            // Finally, lines
            top = razRules[i][2];           // Lines

            while( top != NULL ) {
                if( ps52plib->ObjectRenderCheck( top, VPoint ) ) {
                    if( DoesLatLonSelectObject( lat, lon, select_radius, top->obj ) ) ret_ptr->append(*top );
                }

                top = top->next;
            }
        }
    }

    return ret_ptr;
}

bool s57chart::DoesLatLonSelectObject( float lat, float lon, float select_radius, S57Obj *obj )
{
    switch( obj->Primitive_type ){
    //  For single Point objects, the integral object bounding box contains the lat/lon of the object,
    //  possibly expanded by text or symbol rendering
    case GEO_POINT: {
        if( !obj->BBObj.GetValid() ) return false;

        if( 1 == obj->npt ) {
            //  Special case for LIGHTS
            //  Sector lights have had their BBObj expanded to include the entire drawn sector
            //  This is too big for pick area, can be confusing....
            //  So make a temporary box at the light's lat/lon, with select_radius size
            if( !strncmp( obj->FeatureName, "LIGHTS", 6 ) ) {
                double olon, olat;
                fromSM( ( obj->x * obj->x_rate ) + obj->x_origin,
                        ( obj->y * obj->y_rate ) + obj->y_origin, ref_lat, ref_lon, &olat,
                        &olon );

                // Double the select radius to adjust for the fact that LIGHTS has
                // a 0x0 BBox to start with, which makes it smaller than all other
                // rendered objects.
                LLBBox sbox;
                sbox.Set(olat, olon, olat, olon);

                if( sbox.ContainsMarge( lat, lon, select_radius ) ) return true;
            }

            else if( obj->BBObj.ContainsMarge( lat, lon, select_radius ) ) return true;
        }

        //  For MultiPoint objects, make a bounding box from each point's lat/lon
        //  and check it
        else {
            if( !obj->BBObj.GetValid() ) return false;

            //  Coarse test first
            if( !obj->BBObj.ContainsMarge( lat, lon, select_radius ) ) return false;
            //  Now decomposed soundings, one by one
            double *pdl = obj->geoPtMulti;
            for( int ip = 0; ip < obj->npt; ip++ ) {
                double lon_point = *pdl++;
                double lat_point = *pdl++;
                LLBBox BB_point;
                BB_point.Set( lat_point, lon_point, lat_point, lon_point );
                if( BB_point.ContainsMarge( lat, lon, select_radius ) ) {
                    //                                  index = ip;
                    return true;
                }
            }
        }

        break;
    }
    case GEO_AREA: {
        //  Coarse test first
        if( !obj->BBObj.ContainsMarge( lat, lon, select_radius ) ) return false;
        else
            return IsPointInObjArea( lat, lon, select_radius, obj );
    }

    case GEO_LINE: {
        //  Coarse test first
        if( !obj->BBObj.ContainsMarge( lat, lon, select_radius ) )
            return false;

        float sel_rad_meters = select_radius * 1852 * 60;       // approximately
        double easting, northing;
        toSM( lat, lon, ref_lat, ref_lon, &easting, &northing );

        if( obj->geoPt ) {

            //  Line geometry is carried in SM or CM93 coordinates, so...
            //  make the hit test using SM coordinates, converting from object points to SM using per-object conversion factors.

            pt *ppt = obj->geoPt;
            int npt = obj->npt;

            double xr = obj->x_rate;
            double xo = obj->x_origin;
            double yr = obj->y_rate;
            double yo = obj->y_origin;

            double north0 = ( ppt->y * yr ) + yo;
            double east0 = ( ppt->x * xr ) + xo;
            ppt++;

            for( int ip = 1; ip < npt; ip++ ) {

                double north = ( ppt->y * yr ) + yo;
                double east = ( ppt->x * xr ) + xo;

                //    A slightly less coarse segment bounding box check
                if( northing >= ( fmin(north, north0) - sel_rad_meters ) ) if( northing
                                                                               <= ( fmax(north, north0) + sel_rad_meters ) ) if( easting
                                                                                                                                 >= ( fmin(east, east0) - sel_rad_meters ) ) if( easting
                                                                                                                                                                                 <= ( fmax(east, east0) + sel_rad_meters ) ) {
                    return true;
                }

                north0 = north;
                east0 = east;
                ppt++;
            }
        }
        else{                       // in oSENC V2, Array of points is stored in prearranged VBO array.
            if( obj->m_ls_list ){
                
                float *ppt;
                unsigned char *vbo_point = (unsigned char *)obj->m_chart_context->chart->GetLineVertexBuffer();
                line_segment_element *ls = obj->m_ls_list;

                while(ls && vbo_point){
                    int nPoints;
                    if( (ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV) ){
                        ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
                        nPoints = ls->pedge->nCount;
                    }
                    else{
                        ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
                        nPoints = 2;
                    }
                    
                    float north0 = ppt[1];
                    float east0 = ppt[0];

                    ppt += 2;
                    
                    for(int ip=0 ; ip < nPoints - 1 ; ip++){

                        float north = ppt[1];
                        float east = ppt[0];

                        if( northing >= ( fmin(north, north0) - sel_rad_meters ) )
                            if( northing <= ( fmax(north, north0) + sel_rad_meters ) )
                                if( easting >= ( fmin(east, east0) - sel_rad_meters ) )
                                    if( easting <= ( fmax(east, east0) + sel_rad_meters ) ) {
                                        return true;
                                    }

                        north0 = north;
                        east0 = east;

                        ppt += 2;
                    }

                    ls = ls->next;
                }
            }
        }

        break;
    }

    case GEO_META:
    case GEO_PRIM:

        break;
    }

    return false;
}

QString s57chart::GetAttributeDecode( QString& att, int ival )
{
    QString ret_val = ("");

    //  Get the attribute code from the acronym
    const char *att_code;

    QString file( g_csv_locn );
    file.append( ("/s57attributes.csv") );

    if( !QFile::exists( file ) ) {
        QString msg( ("   Could not open ") );
        msg.append( file );
        qDebug()<< msg ;

        return ret_val;
    }

    att_code = MyCSVGetField( file.toUtf8().data(), "Acronym",                  // match field
                              att.toUtf8().data(),               // match value
                              CC_ExactString, "Code" );             // return field

    // Now, get a nice description from s57expectedinput.csv
    //  This will have to be a 2-d search, using ID field and Code field

    // Ingest, and get a pointer to the ingested table for "Expected Input" file
    QString ei_file( g_csv_locn );
    ei_file.append( ("/s57expectedinput.csv") );

    if( !QFile::exists( ei_file ) ) {
        QString msg( ("   Could not open ") );
        msg.append( ei_file );
        qDebug()<< msg ;

        return ret_val;
    }

    CSVTable *psTable = CSVAccess( ei_file.toUtf8().data() );
    CSVIngest( ei_file.toUtf8().data() );

    char **papszFields = NULL;
    int bSelected = false;

    /* -------------------------------------------------------------------- */
    /*      Scan from in-core lines.                                        */
    /* -------------------------------------------------------------------- */
    int iline = 0;
    while( !bSelected && iline + 1 < psTable->nLineCount ) {
        iline++;
        papszFields = CSVSplitLine( psTable->papszLines[iline] );

        if( !strcmp( papszFields[0], att_code ) ) {
            if( atoi( papszFields[1] ) == ival ) {
                ret_val = QString::fromUtf8( papszFields[2] );
                bSelected = true;
            }
        }

        CSLDestroy( papszFields );
    }

    return ret_val;

}

//----------------------------------------------------------------------------------

bool s57chart::IsPointInObjArea( float lat, float lon, float select_radius, S57Obj *obj )
{
    bool ret = false;

    if( obj->pPolyTessGeo ) {
        if( !obj->pPolyTessGeo->IsOk() )
            obj->pPolyTessGeo->BuildDeferredTess();

        PolyTriGroup *ppg = obj->pPolyTessGeo->Get_PolyTriGroup_head();

        TriPrim *pTP = ppg->tri_prim_head;

        MyPoint pvert_list[3];

        //  Polygon geometry is carried in SM coordinates, so...
        //  make the hit test thus.
        double easting, northing;
        toSM( lat, lon, ref_lat, ref_lon, &easting, &northing );

        //  On some chart types (e.g. cm93), the tesseleated coordinates are stored differently.
        //  Adjust the pick point (easting/northing) to correspond.
        if( !ppg->m_bSMSENC ) {
            double y_rate = obj->y_rate;
            double y_origin = obj->y_origin;
            double x_rate = obj->x_rate;
            double x_origin = obj->x_origin;

            double northing_scaled = ( northing - y_origin ) / y_rate;
            double easting_scaled = ( easting - x_origin ) / x_rate;
            northing = northing_scaled;
            easting = easting_scaled;
        }

        while( pTP ) {
            //  Coarse test
            if( pTP->tri_box.Contains( lat, lon ) ) {

                if(ppg->data_type == DATA_TYPE_DOUBLE) {
                    double *p_vertex = pTP->p_vertex;

                    switch( pTP->type ){
                    case PTG_TRIANGLE_FAN: {
                        for( int it = 0; it < pTP->nVert - 2; it++ ) {
                            pvert_list[0].x = p_vertex[0];
                            pvert_list[0].y = p_vertex[1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                    case PTG_TRIANGLE_STRIP: {
                        for( int it = 0; it < pTP->nVert - 2; it++ ) {
                            pvert_list[0].x = p_vertex[( it * 2 )];
                            pvert_list[0].y = p_vertex[( it * 2 ) + 1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                    case PTG_TRIANGLES: {
                        for( int it = 0; it < pTP->nVert; it += 3 ) {
                            pvert_list[0].x = p_vertex[( it * 2 )];
                            pvert_list[0].y = p_vertex[( it * 2 ) + 1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                    }
                }
                else {
                    float *p_vertex = (float *)pTP->p_vertex;

                    switch( pTP->type ){
                    case PTG_TRIANGLE_FAN: {
                        for( int it = 0; it < pTP->nVert - 2; it++ ) {
                            pvert_list[0].x = p_vertex[0];
                            pvert_list[0].y = p_vertex[1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                    case PTG_TRIANGLE_STRIP: {
                        for( int it = 0; it < pTP->nVert - 2; it++ ) {
                            pvert_list[0].x = p_vertex[( it * 2 )];
                            pvert_list[0].y = p_vertex[( it * 2 ) + 1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                    case PTG_TRIANGLES: {
                        for( int it = 0; it < pTP->nVert; it += 3 ) {
                            pvert_list[0].x = p_vertex[( it * 2 )];
                            pvert_list[0].y = p_vertex[( it * 2 ) + 1];

                            pvert_list[1].x = p_vertex[( it * 2 ) + 2];
                            pvert_list[1].y = p_vertex[( it * 2 ) + 3];

                            pvert_list[2].x = p_vertex[( it * 2 ) + 4];
                            pvert_list[2].y = p_vertex[( it * 2 ) + 5];

                            if( G_PtInPolygon( (MyPoint *) pvert_list, 3, easting, northing ) ) {
                                ret = true;
                                break;
                            }
                        }
                        break;
                    }
                    }
                }
            }
            pTP = pTP->p_next;
        }

    }           // if pPolyTessGeo

    return ret;
}


QString s57chart::GetObjectAttributeValueAsString( S57Obj *obj, int iatt, QString curAttrName )
{
    QString value;
    S57attVal *pval;

    pval = obj->attVal->at( iatt );
    switch( pval->valType ){
    case OGR_STR: {
        if( pval->value ) {
            QString val_str = QString::fromUtf8( (char *) ( pval->value ) );
            bool ok = false;
            long ival = val_str.toLong(&ok);
            if( ok) {
                if( 0 == ival )
                    value = ("Unknown");
                else {
                    QString decode_val = GetAttributeDecode( curAttrName, ival );
                    if( !decode_val.isEmpty() ) {
                        value = decode_val;
                        QString iv;
                        iv.sprintf( (" (%d)"), (int) ival );
                        value.append( iv );
                    } else
                        value.sprintf( ("%d"), (int) ival );
                }
            }

            else if( val_str.isEmpty() )
                value = ("Unknown");

            else {
                value.clear();
                QString value_increment;
                QStringList tk =  val_str.split( "," );
                int iv = 0;
                if( tk.size() > 0 ) {
                    int i=0;
                    while( i<tk.size() ) {
                        QString token = tk[i++];
                        bool ok = false;
                        long ival = token.toLong(&ok);
                        if( ok ) {
                            QString decode_val = GetAttributeDecode( curAttrName, ival );

                            value_increment.sprintf( (" (%d)"), (int) ival );

                            if( !decode_val.isEmpty() )
                                value_increment.insert(0, decode_val);

                            if( iv ) value_increment.insert(0, ", " );
                            value.append( value_increment );

                        }
                        else{
                            if(iv) value.append((","));
                            value.append( token );
                        }

                        iv++;
                    }
                }
                else
                    value.append( val_str );
            }
        } else
            value = ("[NULL VALUE]");

        break;
    }

    case OGR_INT: {
        int ival = *( (int *) pval->value );
        QString decode_val = GetAttributeDecode( curAttrName, ival );

        if( !decode_val.isEmpty() ) {
            value = decode_val;
            QString iv;
            iv.sprintf( "(%d)", ival );
            value.append( iv );
        } else
            value.sprintf( "(%d)", ival );

        break;
    }
    case OGR_INT_LST:
        break;

    case OGR_REAL: {
        double dval = *( (double *) pval->value );
        QString val_suffix = (" m");

        //    As a special case, convert some attribute values to feet.....
        if( ( curAttrName == ("VERCLR") ) || ( curAttrName == ("VERCCL") ) || ( curAttrName == ("VERCOP") )
                || ( curAttrName == ("HEIGHT") ) || ( curAttrName == ("HORCLR") ) ) {
            switch( ps52plib->m_nDepthUnitDisplay ){
            case 0:                       // feet
            case 2:                       // fathoms
                dval = dval * 3 * 39.37 / 36;              // feet
                val_suffix = (" ft");
                break;
            default:
                break;
            }
        }

        else if( ( curAttrName == ("VALSOU") ) || ( curAttrName == ("DRVAL1") )
                 || ( curAttrName == ("DRVAL2") ) || ( curAttrName == ("VALDCO") ) ) {
            switch( ps52plib->m_nDepthUnitDisplay ){
            case 0:                       // feet
                dval = dval * 3 * 39.37 / 36;              // feet
                val_suffix = (" ft");
                break;
            case 2:                       // fathoms
                dval = dval * 3 * 39.37 / 36;              // fathoms
                dval /= 6.0;
                val_suffix = (" fathoms");
                break;
            default:
                break;
            }
        }

        else if( curAttrName == ("SECTR1") ) val_suffix = ("&deg;");
        else if( curAttrName == ("SECTR2") ) val_suffix = ("&deg;");
        else if( curAttrName == ("ORIENT") ) val_suffix = ("&deg;");
        else if( curAttrName == ("VALNMR") ) val_suffix = (" Nm");
        else if( curAttrName == ("SIGPER") ) val_suffix = ("s");
        else if( curAttrName == ("VALACM") ) val_suffix = (" Minutes/year");
        else if( curAttrName == ("VALMAG") ) val_suffix = ("&deg;");
        else if( curAttrName == ("CURVEL") ) val_suffix = (" kt");

        if( dval - floor( dval ) < 0.01 ) value.sprintf( "%2.0f", dval );
        else
            value.sprintf( ("%4.1f"), dval );

        value .append( val_suffix);

        break;
    }

    case OGR_REAL_LST: {
        break;
    }
    }
    return value;
}

QString s57chart::GetAttributeValueAsString( S57attVal *pAttrVal, QString AttrName )
{
    if(NULL == pAttrVal)
        return ("");

    QStringList value;
    switch( pAttrVal->valType )
    {
    case OGR_STR:
    {
        if( pAttrVal->value ) {
            QString val_str = QString::fromUtf8((char *) ( pAttrVal->value ) );
            bool ok = false;
            long ival = val_str.toLong(&ok);
            if( ok ) {
                if( 0 == ival )
                    value.append("Unknown");
                else {
                    QString decode_val = GetAttributeDecode( AttrName, ival );
                    if( !decode_val.isEmpty() ) {
                        value.append(decode_val);
                        QString iv;
                        iv.sprintf("(%d)", (int) ival );
                        value.append( iv );
                    } else
                        value.append(QString("").sprintf("%d", (int) ival ));
                }
            }

            else if( val_str.isEmpty() ) value.append("Unknown");

            else {
                value.clear();
                QString value_increment;
                QStringList tk = val_str.split(",");
                int iv = 0;
                int i = 0;
                while( i<tk.size() ) {
                    QString token = tk[i++];
                    bool ok = false;
                    long ival = token.toLong(&ok);
                    if( ok ) {
                        QString decode_val = GetAttributeDecode( AttrName, ival );
                        if( !decode_val.isEmpty() ) value_increment = decode_val;
                        else
                            value_increment.sprintf( (" %d"), (int) ival );

                        if( iv ) value_increment.insert(0 ,", ");
                    }
                    value.append( value_increment );

                    iv++;
                }
                value.append( val_str );
            }
        } else
            value.append("[NULL VALUE]");

        break;
    }

    case OGR_INT:
    {
        int ival = *( (int *) pAttrVal->value );
        QString decode_val = GetAttributeDecode( AttrName, ival );

        if( !decode_val.isEmpty() ) {
            value.append(decode_val);
            QString iv;
            iv.sprintf( ("(%d)"), ival );
            value.append( iv );
        } else
            value.append(QString("").sprintf( ("(%d)"), ival ));

        break;
    }
    case OGR_INT_LST:
        break;

    case OGR_REAL: {
        double dval = *( (double *) pAttrVal->value );
        QString val_suffix = (" m");

        //    As a special case, convert some attribute values to feet.....
        if( ( AttrName == ("VERCLR") ) || ( AttrName == ("VERCCL") ) || ( AttrName == ("VERCOP") )
                || ( AttrName == ("HEIGHT") ) || ( AttrName == ("HORCLR") ) ) {
            switch( ps52plib->m_nDepthUnitDisplay ){
            case 0:                       // feet
            case 2:                       // fathoms
                dval = dval * 3 * 39.37 / 36;              // feet
                val_suffix = (" ft");
                break;
            default:
                break;
            }
        }

        else if( ( AttrName == ("VALSOU") ) || ( AttrName == ("DRVAL1") )
                 || ( AttrName == ("DRVAL2") ) ) {
            switch( ps52plib->m_nDepthUnitDisplay ){
            case 0:                       // feet
                dval = dval * 3 * 39.37 / 36;              // feet
                val_suffix = (" ft");
                break;
            case 2:                       // fathoms
                dval = dval * 3 * 39.37 / 36;              // fathoms
                dval /= 6.0;
                val_suffix = (" fathoms");
                break;
            default:
                break;
            }
        }

        else if( AttrName == ("SECTR1") ) val_suffix = ("&deg;");
        else if( AttrName == ("SECTR2") ) val_suffix = ("&deg;");
        else if( AttrName == ("ORIENT") ) val_suffix = ("&deg;");
        else if( AttrName == ("VALNMR") ) val_suffix = (" Nm");
        else if( AttrName == ("SIGPER") ) val_suffix = ("s");
        else if( AttrName == ("VALACM") ) val_suffix = (" Minutes/year");
        else if( AttrName == ("VALMAG") ) val_suffix = ("&deg;");
        else if( AttrName == ("CURVEL") ) val_suffix = (" kt");

        if( dval - floor( dval ) < 0.01 ) value.append(QString("").sprintf( ("%2.0f"), dval ));
        else
            value.append(QString("").sprintf("%4.1f", dval ));

        value.append(val_suffix);

        break;
    }

    case OGR_REAL_LST: {
        break;
    }
    }
    return value.join("");
}

bool s57chart::CompareLights( const S57Light* l1, const S57Light* l2 )
{
    int positionDiff = l1->position.compare( l2->position );
    if( positionDiff != 0 ) return true;


    int attrIndex1 = l1->attributeNames.indexOf( ("SECTR1") );
    int attrIndex2 = l2->attributeNames.indexOf( ("SECTR1") );

    // This should put Lights without sectors last in the list.
    if( attrIndex1 == -1 && attrIndex2 == -1 ) return false;
    if( attrIndex1 != -1 && attrIndex2 == -1 ) return true;
    if( attrIndex1 == -1 && attrIndex2 != -1 ) return false;

    double angle1 = l1->attributeValues.at( attrIndex1 ).toDouble();
    double angle2 = l2->attributeValues.at( attrIndex2 ).toDouble();

    return angle1 < angle2;
}

static const char *type2str( GeoPrim_t type)
{
    const char *r = "Unknown";
    switch(type) {
    case GEO_POINT:
        return "Point";
        break;
    case GEO_LINE:
        return "Line";
        break;
    case GEO_AREA:
        return "Area";
        break;
    case GEO_META:
        return "Meta";
        break;
    case GEO_PRIM:
        return "Prim";
        break;
    }
    return r;
}

QString s57chart::CreateObjDescriptions( ListOfObjRazRules* rule_list )
{
    QString ret_val;
    int attrCounter;
    QString curAttrName, value;
    bool isLight = false;
    QString className;
    QString classDesc;
    QString classAttributes;
    QString objText;
    QString lightsHtml;
    QString positionString;
    std::vector<S57Light*> lights;
    S57Light* curLight = nullptr;
    QFileInfo file ;

    int list_size = rule_list->size();
    for(int i  = list_size - 1; i >= 0; i --)
    {
        ObjRazRules &current = (*rule_list)[i];
        positionString.clear();
        objText.clear();

        // Soundings have no information, so don't show them
        if( 0 == strncmp( current.LUP->OBCL, "SOUND", 5 ) ) continue;

        if( current.obj->Primitive_type == GEO_META ) continue;
        if( current.obj->Primitive_type == GEO_PRIM ) continue;

        className = QString::fromUtf8(current.obj->FeatureName );

        // Lights get grouped together to make display look nicer.
        isLight = !strcmp( current.obj->FeatureName, "LIGHTS" );

        //    Get the object's nice description from s57objectclasses.csv
        //    using cpl_csv from the gdal library

        const char *name_desc;
        if( g_csv_locn.length() )
        {
            QString oc_file( g_csv_locn );
            oc_file.append( ("/s57objectclasses.csv") );
            name_desc = MyCSVGetField( oc_file.toUtf8().data(), "Acronym",                  // match field
                                       current.obj->FeatureName,            // match value
                                       CC_ExactString, "ObjectClass" );             // return field
        } else
            name_desc = "";

        // In case there is no nice description for this object class, use the 6 char class name
        if( 0 == strlen( name_desc ) ) {
            name_desc = current.obj->FeatureName;
            QLatin1Char lchar(name_desc[0]);
            classDesc = QString(lchar);
            classDesc.append(QString::fromUtf8(name_desc + 1 ).toLower());
        } else {
            classDesc = QString::fromUtf8( name_desc );
        }

        //    Show LUP
        if( g_bDebugS57 ) {
            QString index;

            classAttributes = ("");
            index.sprintf( ("Feature Index: %d<br>"), current.obj->Index );
            classAttributes .append(index);

            QString LUPstring;
            LUPstring.sprintf( ("LUP RCID:  %d<br>"), current.LUP->RCID );
            classAttributes.append( LUPstring );

            QString Bbox;
            LLBBox bbox = current.obj->BBObj;
            Bbox.sprintf( ("Lat/Lon box:  %g %g %g %g<br>"), bbox.GetMinLat(), bbox.GetMaxLat() , bbox.GetMinLon(), bbox.GetMaxLon() );
            classAttributes.append(Bbox);

            QString Type;
            Type.sprintf( (" Type:  %s<br>"), type2str(current.obj->Primitive_type));
            classAttributes.append(Type);

            LUPstring = ("    LUP ATTC: ");
            if( current.LUP->ATTArray.size() ) LUPstring += QString(current.LUP->ATTArray[0]);
            LUPstring += ("<br>");
            classAttributes.append(LUPstring);

            LUPstring = ("    LUP INST: ");
            if( current.LUP->INST ) LUPstring += *( current.LUP->INST );
            LUPstring += ("<br><br>");
            classAttributes.append(LUPstring);

        }

        if( GEO_POINT == current.obj->Primitive_type ) {
            double lon, lat;
            fromSM( ( current.obj->x * current.obj->x_rate ) + current.obj->x_origin,
                    ( current.obj->y * current.obj->y_rate ) + current.obj->y_origin, ref_lat,
                    ref_lon, &lat, &lon );

            if( lon > 180.0 ) lon -= 360.;

            positionString.clear();
            positionString += zchxFuncUtil::toSDMM( 1, lat );
            positionString += (" ");
            positionString += zchxFuncUtil::toSDMM( 2, lon );

            if( isLight ) {
                curLight = new S57Light;
                curLight->position = positionString;
                curLight->hasSectors = false;
                lights.push_back( curLight );
            }
        }

        //    Get the Attributes and values, making sure they can be converted from UTF8
        if(current.obj->att_array) {
            char *curr_att = current.obj->att_array;

            attrCounter = 0;

            QString attribStr;
            int noAttr = 0;
            attribStr += ("<table border=0 cellspacing=0 cellpadding=0>");

            if( g_bDebugS57 ) {
                ret_val .append("<p>") .append( classAttributes);
            }

            bool inDepthRange = false;

            while( attrCounter < current.obj->n_attr ) {
                //    Attribute name
                curAttrName = QString::fromUtf8(QByteArray(curr_att, 6));
                noAttr++;

                // Sort out how some kinds of attibutes are displayed to get a more readable look.
                // DEPARE gets just its range. Lights are grouped.

                if( isLight ) {
                    Q_ASSERT( curLight != nullptr);
                    curLight->attributeNames.append( curAttrName );
                    if( curAttrName.startsWith( ("SECTR") ) ) curLight->hasSectors = true;
                } else {
                    if( curAttrName == ("DRVAL1") ) {
                        attribStr.append("<tr><td><font size=-1>");
                        inDepthRange = true;
                    } else if( curAttrName == ("DRVAL2") ) {
                        attribStr .append(" - ");
                        inDepthRange = false;
                    } else {
                        if( inDepthRange ) {
                            attribStr .append ("</font></td></tr>\n");
                            inDepthRange = false;
                        }
                        attribStr.append("<tr><td valign=top><font size=-2>");
                        if(curAttrName == ("catgeo"))
                            attribStr .append ("CATGEO");
                        else
                            attribStr .append( curAttrName);
                        attribStr .append ("</font></td><td>&nbsp;&nbsp;</td><td valign=top><font size=-1>");
                    }
                }

                // What we need to do...
                // Change senc format, instead of (S), (I), etc, use the attribute types fetched from the S57attri...csv file
                // This will be like (E), (L), (I), (F)
                //  will affect lots of other stuff.  look for S57attVal.valType
                // need to do this in creatsencrecord above, and update the senc format.

                value = GetObjectAttributeValueAsString( current.obj, attrCounter, curAttrName );
                
                // If the atribute value is a filename, change the value into a link to that file
                QString AttrNamesFiles = ("PICREP,TXTDSC,NTXTDS"); //AttrNames that might have a filename as value
                if ( AttrNamesFiles.indexOf(curAttrName) != Q_INDEX_NOT_FOUND )
                {
                    if ( value.indexOf(".XML") == Q_INDEX_NOT_FOUND )
                    { // Don't show xml files
                        file.setFile(GetFullPath());
                        file.setFile( file.absolutePath(), value );
                        if( file.exists() )
                            value.sprintf("<a href=\"%s\">%s</a>", file.absoluteFilePath().toUtf8().data(), zchxFuncUtil::getFileName(file.fileName()).toUtf8().data() );
                        else
                            value = value + ("&nbsp;&nbsp;<font color=\"red\">[ ") + ("this file is not available") + (" ]</font>");
                    }
                }

                if( isLight ) {
                    assert( curLight != nullptr);
                    curLight->attributeValues.append( value );
                } else {
                    if( curAttrName == ("INFORM") || curAttrName == ("NINFOM") )
                        value.replace(("|"), ("<br>") );
                    
                    if(curAttrName == ("catgeo"))
                        attribStr.append(type2str(current.obj->Primitive_type));
                    else
                        attribStr .append( value);

                    if( !( curAttrName == ("DRVAL1") ) ) {
                        attribStr.append ("</font></td></tr>\n");
                    }
                }

                attrCounter++;
                curr_att += 6;

            }             //while attrCounter < current.obj->n_attr

            if( !isLight ) {
                attribStr .append ("</table>\n");

                objText += ("<b>") + classDesc + ("</b> <font size=-2>(") + className
                        + (")</font>") + ("<br>");

                if( positionString.length() ) objText .append("<font size=-2>").append(positionString)
                        .append("</font><br>\n");

                if( noAttr > 0 ) objText.append(attribStr);

                if( i != 0 ) objText += ("<hr noshade>");
                objText += ("<br>");
                ret_val.append( objText);
            }

        }
    } // Object for loop
    
    // Add the additional info files
    file.setFile(GetFullPath() );
    QString AddFiles = QString("").sprintf("<hr noshade><br><b>Additional info files attached to: </b> <font size=-2>%s</font><br><table border=0 cellspacing=0 cellpadding=3>", file.fileName().toUtf8().data() );
    QDir dir(file.absolutePath());
    QStringList file_filter_list;
    file_filter_list.append("*.TXT");
    file_filter_list.append("*.txt");
    QFileInfoList files = dir.entryInfoList(file_filter_list, QDir::Filter::Files);
    if ( files.count() > 0 )
    {
        for ( size_t i=0; i < files.count(); i++){
            file = files[i];
            AddFiles.append(QString("").sprintf("<tr><td valign=top><font size=-2><a href=\"%s\">%s</a></font></td><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp</td>",
                                                file.absoluteFilePath().toUtf8().data(),
                                                zchxFuncUtil::getFileName(file.fileName())))  ;
            if ( files.count() > ++i){
                file = files[i];
                AddFiles.append(QString("").sprintf("<td valign=top><font size=-2><a href=\"%s\">%s</a></font></td>",
                                                    file.absoluteFilePath().toUtf8().data(),
                                                    zchxFuncUtil::getFileName(file.fileName())));
            }
        }

        AddFiles.append("</table>");
        ret_val.append(AddFiles);
    }
    
    if( !lights.empty() ) {
        assert( curLight != nullptr);

        // For lights we now have all the info gathered but no HTML output yet, now
        // run through the data and build a merged table for all lights.

        std::sort(lights.begin(), lights.end(), s57chart::CompareLights);

        QString lastPos;

        for(S57Light* thisLight: lights) {
            int attrIndex;

            if( thisLight->position != lastPos ) {

                lastPos = thisLight->position;

                if( thisLight != *lights.begin() )
                    lightsHtml.append("</table>\n<hr noshade>\n");

                lightsHtml.append("<b>Light</b> <font size=-2>(LIGHTS)</font><br>");
                lightsHtml.append("<font size=-2>");
                lightsHtml.append(thisLight->position);
                lightsHtml.append("</font><br>\n");

                if( curLight->hasSectors )
                    lightsHtml.append("<font size=-2>(Sector angles are True Bearings from Seaward)</font><br>");

                lightsHtml.append("<table>");
            }

            lightsHtml.append("<tr>");
            lightsHtml.append("<td><font size=-1>");

            QString colorStr;
            attrIndex = thisLight->attributeNames.indexOf( ("COLOUR") );
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                QString color = thisLight->attributeValues.at( attrIndex );
                if( color == ("red (3)") )
                    colorStr = ("<table border=0><tr><td bgcolor=red>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                else if( color == ("green (4)") )
                    colorStr = ("<table border=0><tr><td bgcolor=green>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                else if( color == ("white (1)") )
                    colorStr = ("<table border=0><tr><td bgcolor=white>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                else if( color == ("yellow (6)") )
                    colorStr = ("<table border=0><tr><td bgcolor=yellow>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                else if( color == ("blue (5)") )
                    colorStr = ("<table border=0><tr><td bgcolor=blue>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                else
                    colorStr = ("<table border=0><tr><td bgcolor=grey>&nbsp;?&nbsp;</td></tr></table> ");
            }

            int visIndex = thisLight->attributeNames.indexOf("LITVIS" );
            if(visIndex != Q_INDEX_NOT_FOUND){
                QString vis = thisLight->attributeValues.at( visIndex );
                if(vis.contains( ("8"))){
                    if( attrIndex != Q_INDEX_NOT_FOUND ) {
                        QString color = thisLight->attributeValues.at( attrIndex );
                        if( color == ("red (3)") )
                            colorStr = ("<table border=0><tr><td bgcolor=DarkRed>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                        if( color == ("green (4)") )
                            colorStr = ("<table border=0><tr><td bgcolor=DarkGreen>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                        if( color == ("white (1)") )
                            colorStr = ("<table border=0><tr><td bgcolor=GoldenRod>&nbsp;&nbsp;&nbsp;</td></tr></table> ");
                    }
                }
            }
            
            lightsHtml.append(colorStr);
            
            lightsHtml.append("</font></td><td><font size=-1><nobr><b>");

            attrIndex = thisLight->attributeNames.indexOf("LITCHR");
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                QString character = thisLight->attributeValues[attrIndex];
                lightsHtml.append(character.left(character.indexOf('(')));
                lightsHtml.append(" ");
            }

            attrIndex = thisLight->attributeNames.indexOf("SIGGRP" );
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                lightsHtml.append(thisLight->attributeValues[attrIndex]);
                lightsHtml.append(" ");
            }

            attrIndex = thisLight->attributeNames.indexOf("SIGPER") ;
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                lightsHtml.append(thisLight->attributeValues[attrIndex]);
                lightsHtml.append(" ");
            }

            attrIndex = thisLight->attributeNames.indexOf( ("HEIGHT") );
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                lightsHtml.append(thisLight->attributeValues[attrIndex]);
                lightsHtml.append(" ");
            }

            attrIndex = thisLight->attributeNames.indexOf( ("VALNMR") );
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                lightsHtml.append(thisLight->attributeValues[attrIndex]);
                lightsHtml.append(" ");
            }

            lightsHtml.append("</b>");

            attrIndex = thisLight->attributeNames.indexOf( ("SECTR1") );
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                lightsHtml.append("(");
                lightsHtml.append(thisLight->attributeValues[attrIndex]);
                lightsHtml.append(" - ");
                attrIndex = thisLight->attributeNames.indexOf( ("SECTR2") );
                lightsHtml.append(thisLight->attributeValues[attrIndex]);
                lightsHtml.append(") ");
            }

            lightsHtml.append("</nobr>");

            attrIndex = thisLight->attributeNames.indexOf(("CATLIT") );
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                lightsHtml.append("<nobr>");
                QString temp = thisLight->attributeValues[attrIndex];
                lightsHtml.append(temp.left(temp.indexOf('(')));
                lightsHtml.append("</nobr> ");
            }

            attrIndex = thisLight->attributeNames.indexOf( ("EXCLIT") );
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                lightsHtml.append("<nobr>");
                QString temp = thisLight->attributeValues[attrIndex];
                lightsHtml.append(temp.left(temp.indexOf('(')));
                lightsHtml.append("</nobr> ");
            }

            attrIndex = thisLight->attributeNames.indexOf( ("OBJNAM") );
            if( attrIndex != Q_INDEX_NOT_FOUND ) {
                lightsHtml.append("<br><nobr>");
                lightsHtml.append(thisLight->attributeValues[attrIndex].left( 1 ).toUpper());
                lightsHtml.append(thisLight->attributeValues[attrIndex].mid( 1 ));
                lightsHtml.append("</nobr> ");
            }

            lightsHtml.append("</font></td>");
            lightsHtml.append("</tr>");

            thisLight->attributeNames.clear();
            thisLight->attributeValues.clear();
            delete thisLight;
        }
        lightsHtml.append("</table><hr noshade>\n");
        lightsHtml.append(ret_val);

        lights.clear();
        ret_val = lightsHtml;
    }

    return ret_val;
}

//------------------------------------------------------------------------
//
//          S57 ENC (i.e. "raw") DataSet support functions
//          Not bulletproof, so call carefully
//
//------------------------------------------------------------------------
bool s57chart::InitENCMinimal( const QString &FullPath )
{
    if( NULL == g_poRegistrar ) {
        qDebug("   Error: No ClassRegistrar in InitENCMinimal.");
        return false;
    }

    m_pENCDS.reset( new OGRS57DataSource );

    m_pENCDS->SetS57Registrar( g_poRegistrar );             ///172

    if( !m_pENCDS->OpenMin( FullPath.toUtf8().data(), true ) )       ///172
        return false;

    S57Reader *pENCReader = m_pENCDS->GetModule( 0 );
    pENCReader->SetClassBased( g_poRegistrar );

    pENCReader->Ingest();

    return true;
}

OGRFeature *s57chart::GetChartFirstM_COVR( int &catcov )
{
    //    Get the reader
    S57Reader *pENCReader = m_pENCDS->GetModule( 0 );

    if( ( NULL != pENCReader ) && ( NULL != g_poRegistrar ) ) {

        //      Select the proper class
        g_poRegistrar->SelectClass( "M_COVR" );

        //      Build a new feature definition for this class
        OGRFeatureDefn *poDefn = S57GenerateObjectClassDefn( g_poRegistrar,
                                                             g_poRegistrar->GetOBJL(), pENCReader->GetOptionFlags() );

        //      Add this feature definition to the reader
        pENCReader->AddFeatureDefn( poDefn );

        //    Also, add as a Layer to Datasource to ensure proper deletion
        m_pENCDS->AddLayer( new OGRS57Layer( m_pENCDS.get(), poDefn, 1 ) );

        //      find this feature
        OGRFeature *pobjectDef = pENCReader->ReadNextFeature( poDefn );
        if( pobjectDef ) {
            //  Fetch the CATCOV attribute
            catcov = pobjectDef->GetFieldAsInteger( "CATCOV" );
            return pobjectDef;
        }

        else {
            return NULL;
        }
    } else
        return NULL;
}

OGRFeature *s57chart::GetChartNextM_COVR( int &catcov )
{
    catcov = -1;

    //    Get the reader
    S57Reader *pENCReader = m_pENCDS->GetModule( 0 );

    //    Get the Feature Definition, stored in Layer 0
    OGRFeatureDefn *poDefn = m_pENCDS->GetLayer( 0 )->GetLayerDefn();

    if( pENCReader ) {
        OGRFeature *pobjectDef = pENCReader->ReadNextFeature( poDefn );

        if( pobjectDef ) {
            catcov = pobjectDef->GetFieldAsInteger( "CATCOV" );
            return pobjectDef;
        }

        return NULL;
    } else
        return NULL;

}

int s57chart::GetENCScale( void )
{
    if( nullptr == m_pENCDS ) return 0;

    //    Assume that chart has been initialized for minimal ENC access
    //    which implies that the ENC has been fully ingested, and some
    //    interesting values have been extracted thereby.

    //    Get the reader
    S57Reader *pENCReader = m_pENCDS->GetModule( 0 );

    if( pENCReader ) return pENCReader->GetCSCL();       ///172
    else
        return 1;
}

/************************************************************************/
/*                       OpenCPN_OGRErrorHandler()                      */
/*                       Use Global wxLog Class                         */
/************************************************************************/

void OpenCPN_OGRErrorHandler( CPLErr eErrClass, int nError, const char * pszErrorMsg )
{

#define ERR_BUF_LEN 2000

    char buf[ERR_BUF_LEN + 1];

    if( eErrClass == CE_Debug ) sprintf( buf, " %s", pszErrorMsg );
    else if( eErrClass == CE_Warning ) sprintf( buf, "   Warning %d: %s\n", nError, pszErrorMsg );
    else
        sprintf( buf, "   ERROR %d: %s\n", nError, pszErrorMsg );

    if( g_bGDAL_Debug  || ( CE_Debug != eErrClass) ) {          // log every warning or error
        QString msg( buf );
        qDebug()<< msg ;
    }

    //      Do not simply return on CE_Fatal errors, as we don't want to abort()

    if( eErrClass == CE_Fatal ) {
        longjmp( env_ogrf, 1 );                  // jump back to the setjmp() point
    }

}

//      In GDAL-1.2.0, CSVGetField is not exported.......
//      So, make my own simplified copy
/************************************************************************/
/*                           MyCSVGetField()                            */
/*                                                                      */
/************************************************************************/

const char *MyCSVGetField( const char * pszFilename, const char * pszKeyFieldName,
                           const char * pszKeyFieldValue, CSVCompareCriteria eCriteria, const char * pszTargetField )

{
    char **papszRecord;
    int iTargetField;

    /* -------------------------------------------------------------------- */
    /*      Find the correct record.                                        */
    /* -------------------------------------------------------------------- */
    papszRecord = CSVScanFileByName( pszFilename, pszKeyFieldName, pszKeyFieldValue, eCriteria );

    if( papszRecord == NULL ) return "";

    /* -------------------------------------------------------------------- */
    /*      Figure out which field we want out of this.                     */
    /* -------------------------------------------------------------------- */
    iTargetField = CSVGetFileFieldId( pszFilename, pszTargetField );
    if( iTargetField < 0 ) return "";

    if( iTargetField >= CSLCount( papszRecord ) ) return "";

    return ( papszRecord[iTargetField] );
}

//------------------------------------------------------------------------
//
//          Some s57 Utilities
//          Meant to be called "bare", usually with no class instance.
//
//------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Get Chart Extents
//----------------------------------------------------------------------------------

bool s57_GetChartExtent( const QString& FullPath, Extent *pext )
{
    //   Fix this  find extents of which?? layer??
    /*
     OGRS57DataSource *poDS = new OGRS57DataSource;
     poDS->Open(pFullPath, true);

     if( poDS == NULL )
     return false;

     OGREnvelope Env;
     S57Reader   *poReader = poDS->GetModule(0);
     poReader->GetExtent(&Env, true);

     pext->NLAT = Env.MaxY;
     pext->ELON = Env.MaxX;
     pext->SLAT = Env.MinY;
     pext->WLON = Env.MinX;

     delete poDS;
     */
    return false;
}

void s57_DrawExtendedLightSectors( ocpnDC& dc, ViewPort& viewport, std::vector<s57Sector_t>& sectorlegs ) {
    float rangeScale = 0.0;

    if( sectorlegs.size() > 0 ) {
        std::vector<int> sectorangles;
        for( unsigned int i=0; i<sectorlegs.size(); i++ ) {
            if( fabs( sectorlegs[i].sector1 - sectorlegs[i].sector2 ) < 0.3 )
                continue;

            double endx, endy;
            ll_gc_ll( sectorlegs[i].pos.y, sectorlegs[i].pos.x,
                      sectorlegs[i].sector1 + 180.0, sectorlegs[i].range,
                      &endy, &endx );

            zchxPoint end1 = viewport.GetPixFromLL( endy, endx );

            ll_gc_ll( sectorlegs[i].pos.y, sectorlegs[i].pos.x,
                      sectorlegs[i].sector2 + 180.0, sectorlegs[i].range,
                      &endy, &endx );

            zchxPoint end2 = viewport.GetPixFromLL( endy, endx );

            zchxPoint lightPos = viewport.GetPixFromLL( sectorlegs[i].pos.y, sectorlegs[i].pos.x );

            // Make sure arcs are well inside viewport.
            float rangePx = sqrtf( powf( (float) (lightPos.x - end1.x), 2) +
                                   powf( (float) (lightPos.y - end1.y), 2) );
            rangePx /= 3.0;
            if( rangeScale == 0.0 ) {
                rangeScale = 1.0;
                if( rangePx > viewport.pixHeight() / 3 ) {
                    rangeScale *= (viewport.pixHeight() / 3) / rangePx;
                }
            }

            rangePx = rangePx * rangeScale;

            int legOpacity;
            QPen arcpen(sectorlegs[i].color, 12, Qt::SolidLine );
            arcpen.setCapStyle(Qt::RoundCap );
            dc.SetPen(arcpen );

            float angle1, angle2;
            angle1 = -(sectorlegs[i].sector2 + 90.0) - viewport.rotation() * 180.0 / PI;
            angle2 = -(sectorlegs[i].sector1 + 90.0) - viewport.rotation() * 180.0 / PI;
            if( angle1 > angle2 ) {
                angle2 += 360.0;
            }
            int lpx = lightPos.x;
            int lpy = lightPos.y;
            int npoints = 0;
            zchxPoint arcpoints[150]; // Size relates to "step" below.

            float step = 3.0;
            while( (step < 15) && ((rangePx * sin(step * PI / 180.)) < 10) ) step += 2.0; // less points on small arcs

            // Make sure we start and stop exactly on the leg lines.
            int narc = ( angle2 - angle1 ) / step;
            narc++;
            step = ( angle2 - angle1 ) / (float)narc;

            if( sectorlegs[i].isleading && (angle2 - angle1 < 60)  ) {
                zchxPoint yellowCone[3];
                yellowCone[0] = lightPos;
                yellowCone[1] = end1;
                yellowCone[2] = end2;
                arcpen = QPen( QColor( 0,0,0,0 ), 1, Qt::SolidLine );
                dc.SetPen( arcpen );
                QColor c = sectorlegs[i].color;
                c.setRgb(c.red(), c.green(), c.blue(), 0.6*c.alpha() );
                dc.SetBrush( QBrush( c ) );
                dc.StrokePolygon( 3, yellowCone, 0, 0 );
                legOpacity = 50;
            } else {
                for( float a = angle1; a <= angle2 + 0.1; a += step ) {
                    int x = lpx + (int) ( rangePx * cos( a * PI / 180. ) );
                    int y = lpy - (int) ( rangePx * sin( a * PI / 180. ) );
                    arcpoints[npoints] = zchxPoint(x, y);
                    npoints++;
                }
                dc.StrokeLines( npoints, arcpoints );
                legOpacity = 128;
            }

            arcpen = QPen( QColor( 0,0,0,legOpacity ), 1, Qt::SolidLine );
            dc.SetPen(arcpen );

            // Only draw each leg line once.

            bool haveAngle1 = false;
            bool haveAngle2 = false;
            int sec1 = (int)sectorlegs[i].sector1;
            int sec2 = (int)sectorlegs[i].sector2;
            if(sec1 > 360) sec1 -= 360;
            if(sec2 > 360) sec2 -= 360;

            if((sec2 == 360) && (sec1 == 0))  //FS#1437
                continue;

            for( unsigned int j=0; j<sectorangles.size(); j++ ) {

                if( sectorangles[j] == sec1 ) haveAngle1 = true;
                if( sectorangles[j] == sec2 ) haveAngle2 = true;
            }

            if( ! haveAngle1 ) {
                dc.StrokeLine( lightPos.toPoint(), end1.toPoint() );
                sectorangles.push_back( sec1 );
            }

            if( ! haveAngle2 ) {
                dc.StrokeLine( lightPos.toPoint(), end2.toPoint() );
                sectorangles.push_back( sec2 );
            }
        }
    }
}

bool s57_CheckExtendedLightSectors( ChartFrameWork *cc, int mx, int my, ViewPort& viewport, std::vector<s57Sector_t>& sectorlegs )
{
    if( !cc )  return false;
    
    double cursor_lat, cursor_lon;
    static float lastLat, lastLon;

    if( !ps52plib || !ps52plib->m_bExtendLightSectors )
        return false;

    //    ChartPlugInWrapper *target_plugin_chart = NULL;
    s57chart *Chs57 = NULL;

    ChartBase *target_chart = cc->getGL()->GetChartAtCursor();
    if( target_chart ){
        if( (target_chart->GetChartType() == CHART_TYPE_PLUGIN) && (target_chart->GetChartFamily() == CHART_FAMILY_VECTOR) )
        {
            //            target_plugin_chart = dynamic_cast<ChartPlugInWrapper *>(target_chart);
            //暂且不处理
            qDebug()<<"CHART_TYPE_PLUGIN && CHART_FAMILY_VECTOR not supported yet";
            return false;
        }
        else
            Chs57 = dynamic_cast<s57chart*>( target_chart );
    }


    cc->GetCanvasPixPoint ( mx, my, cursor_lat, cursor_lon );

    if( lastLat == cursor_lat && lastLon == cursor_lon ) return false;

    lastLat = cursor_lat;
    lastLon = cursor_lon;
    bool newSectorsNeedDrawing = false;

    bool bhas_red_green = false;
    bool bleading_attribute = false;

    int opacity = 100;
    if( cc->getGL()->GetColorScheme() == GLOBAL_COLOR_SCHEME_DUSK ) opacity = 50;
    if( cc->getGL()->GetColorScheme() == GLOBAL_COLOR_SCHEME_NIGHT) opacity = 20;

    int yOpacity = (float)opacity*1.3; // Matched perception of white/yellow with red/green

    if( /*target_plugin_chart || */Chs57  )
    {
        // Go get the array of all objects at the cursor lat/lon
        float selectRadius = 16 / ( viewport.viewScalePPM() * 1852 * 60 );

        ListOfObjRazRules* rule_list = NULL;
        //        ListOfPI_S57Obj* pi_rule_list = NULL;
        if( Chs57 )
            rule_list = Chs57->GetObjRuleListAtLatLon( cursor_lat, cursor_lon, selectRadius, &viewport, MASK_POINT );
        //        else if( target_plugin_chart )
        //            pi_rule_list = g_pi_manager->GetPlugInObjRuleListAtLatLon( target_plugin_chart,
        //                                                                       cursor_lat, cursor_lon, selectRadius, viewport );


        sectorlegs.clear();

        zchxPointF lightPosD(0,0);
        zchxPointF objPos;

        char *curr_att = NULL;
        int n_attr = 0;
        wxArrayOfS57attVal *attValArray = NULL;

        //        ObjRazRules *snode = NULL;
        //        ListOfPI_S57Obj::Node *pnode = NULL;

        //        if(Chs57 && rule_list)
        //        {
        //            snode = rule_list->last();
        //        }
        //        else if( target_plugin_chart && pi_rule_list)
        //            pnode = pi_rule_list->GetLast();

        if(Chs57 && rule_list)
        {
            bool is_light = false;
            int size = rule_list->size();
            for(int i=size-1; i>=0; i++)
            {
                ObjRazRules current = rule_list->at(i);
                S57Obj* light = current.obj;
                if( !strcmp( light->FeatureName, "LIGHTS" ) ) {
                    objPos = zchxPointF( light->m_lat, light->m_lon );
                    curr_att = light->att_array;
                    n_attr = light->n_attr;
                    attValArray = light->attVal;
                    is_light = true;
                }


                //  Ready to go
                int attrCounter;
                double sectr1 = -1;
                double sectr2 = -1;
                double valnmr = -1;
                QString curAttrName;
                QColor color;

                if( lightPosD.x == 0 && lightPosD.y == 0.0 )
                    lightPosD = objPos;

                if( is_light && (lightPosD == objPos) ) {

                    if( curr_att ) {
                        bool bviz = true;

                        attrCounter = 0;
                        int noAttr = 0;
                        bool inDepthRange = false;
                        s57Sector_t sector;

                        bleading_attribute = false;

                        while( attrCounter < n_attr ) {
                            curAttrName = QString::fromUtf8(QByteArray(curr_att, 6) );
                            noAttr++;
                            S57attVal *pAttrVal = NULL;
                            if( attValArray )  pAttrVal = attValArray->at(attrCounter);

                            QString value = s57chart::GetAttributeValueAsString( pAttrVal, curAttrName );

                            if( curAttrName == ("LITVIS") ){
                                if(value.startsWith(("obsc")) )
                                    bviz = false;
                            }
                            if( curAttrName == ("SECTR1") ) sectr1 = value.toDouble();
                            if( curAttrName == ("SECTR2") ) sectr2 = value.toDouble();
                            if( curAttrName == ("VALNMR") ) valnmr = value.toDouble();
                            if( curAttrName == ("COLOUR") ) {
                                if( value == ("red(3)") ) {
                                    color = QColor( 255, 0, 0, opacity );
                                    sector.iswhite = false;
                                    bhas_red_green = true;
                                }

                                if( value == ("green(4)") ) {
                                    color = QColor( 0, 255, 0, opacity );
                                    sector.iswhite = false;
                                    bhas_red_green = true;
                                }
                            }

                            if( curAttrName == ("EXCLIT") ) {
                                if( value.contains("(3)") ) valnmr = 1.0;  // Fog lights.
                            }

                            if( curAttrName == ("CATLIT") ){
                                if( value.toUpper().startsWith( ("DIRECT")) || value.toUpper().startsWith(("LEAD")) )
                                    bleading_attribute = true;
                            }

                            attrCounter++;
                            curr_att += 6;
                        }

                        if( ( sectr1 >= 0 ) && ( sectr2 >= 0 ) ) {
                            if( sectr1 > sectr2 ) {             // normalize
                                sectr2 += 360.0;
                            }

                            sector.pos.x = objPos.y;              // lon
                            sector.pos.y = objPos.x;

                            sector.range = (valnmr > 0.0) ? valnmr : 2.5; // Short default range.
                            sector.sector1 = sectr1;
                            sector.sector2 = sectr2;

                            if(!color.isValid()){
                                color = QColor( 255, 255, 0, yOpacity );
                                sector.iswhite = true;
                            }
                            sector.color = color;
                            sector.isleading = false;           // tentative judgment, check below

                            if( bleading_attribute )
                                sector.isleading = true;

                            bool newsector = true;
                            for( unsigned int i=0; i<sectorlegs.size(); i++ ) {
                                if( sectorlegs[i].pos == sector.pos &&
                                        sectorlegs[i].sector1 == sector.sector1 &&
                                        sectorlegs[i].sector2 == sector.sector2 ) {
                                    newsector = false;
                                    //  In the case of duplicate sectors, choose the instance with largest range.
                                    //  This applies to the case where day and night VALNMR are different, and so
                                    //  makes the vector result independent of the order of day/night light features.
                                    sectorlegs[i].range = fmax(sectorlegs[i].range, sector.range);
                                }
                            }

                            if(!bviz)
                                newsector = false;

                            if((sector.sector2 == 360) && ( sector.sector1 == 0))  //FS#1437
                                newsector = false;

                            if( newsector ) {
                                sectorlegs.push_back( sector );
                                newSectorsNeedDrawing = true;
                            }
                        }
                    }
                }
            }               // end of while
        }

        if(rule_list) {
            rule_list->clear();
            delete rule_list;
        }

        //        if(pi_rule_list) {
        //            pi_rule_list->clear();
        //            delete pi_rule_list;
        //        }
    }

#if 0
    //  Work with the sector legs vector to identify  and mark "Leading Lights"
    int ns = sectorlegs.size();
    if( sectorlegs.size() > 0 ) {
        for( unsigned int i=0; i<sectorlegs.size(); i++ ) {
            if( fabs( sectorlegs[i].sector1 - sectorlegs[i].sector2 ) < 0.5 )
                continue;

            if(((sectorlegs[i].sector2 - sectorlegs[i].sector1) < 15)  && sectorlegs[i].iswhite ) {
                //      Check to see if this sector has a visible range greater than any other white light

                if( sectorlegs.size() > 1 ) {
                    bool bleading = true;
                    for( unsigned int j=0; j<sectorlegs.size(); j++ ) {
                        if(i == j)
                            continue;
                        if((sectorlegs[j].iswhite) && (sectorlegs[i].range <= sectorlegs[j].range) ){
                            if((sectorlegs[j].sector2 - sectorlegs[j].sector1) >= 15){  // test sector should not be a leading light
                                bleading = false;    // cannot be a sector, since its range is <= another white light
                                break;
                            }
                        }
                    }

                    if(bleading)
                        sectorlegs[i].isleading = true;
                }
            }
            else
                sectorlegs[i].isleading = false;

        }
    }
#endif

    //  Work with the sector legs vector to identify  and mark "Leading Lights"
    //  Sectors with CATLIT "Leading" or "Directional" attribute set have already been marked
    for( unsigned int i=0; i<sectorlegs.size(); i++ ) {

        if(((sectorlegs[i].sector2 - sectorlegs[i].sector1) < 15) ) {
            if( sectorlegs[i].iswhite && bhas_red_green )
                sectorlegs[i].isleading = true;
        }
    }


    return newSectorsNeedDrawing;
}
