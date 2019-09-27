/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S52 Presentation Library
 * Authors:   David Register
 *            Jesper Weissglas
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

#include <math.h>
#include <stdlib.h>

#include "config.h"

#include "georef.h"
#include "viewport.h"

#include "s52plib.h"
#include "mygeom.h"
#include "cutil.h"
#include "s52utils.h"
#include "chartsymbols.h"
#include "TexFont.h"
//#include "ocpn_plugin.h"
#include "gdal/cpl_csv.h"
#include <QFile>
#include "zchxconfig.h"
#include "bitmap.h"
#include <QFont>
#include <QPainter>
#include <QDateTime>
#include "OCPNPlatform.h"
#include "FontMgr.h"


#ifndef PROJECTION_MERCATOR
#define PROJECTION_MERCATOR 1
#endif


extern float g_GLMinCartographicLineWidth;
extern float g_GLMinSymbolLineWidth;
extern double  g_overzoom_emphasis_base;
extern bool    g_oz_vector_scale;
extern float g_ChartScaleFactorExp;
extern int g_chart_zoom_modifier_vector;
extern zchxConfig*      g_config;


float g_scaminScale;

extern PFNGLGENBUFFERSPROC                 s_glGenBuffers;
extern PFNGLBINDBUFFERPROC                 s_glBindBuffer;
extern PFNGLBUFFERDATAPROC                 s_glBufferData;
extern PFNGLDELETEBUFFERSPROC              s_glDeleteBuffers;

void DrawAALine( QPainter *pDC, int x0, int y0, int x1, int y1, QColor clrLine, int dash, int space );
extern bool GetDoubleAttr( S57Obj *obj, const char *AttrName, double &val );
void PLIBDrawGLThickLine( float x1, float y1, float x2, float y2, QPen pen, bool b_hiqual );

void LoadS57Config();
extern QColor GetGlobalColor(const QString& str);


//  S52_TextC Implementation
S52_TextC::S52_TextC()
{ 
    pcol = NULL;
    pFont = NULL;
    texobj = 0;
    bnat = false;
    bspecial_char = false;
}

S52_TextC::~S52_TextC()
{
    if(texobj){
        glDeleteTextures(1, (GLuint *)(&this->texobj) );
    }
}

//      In GDAL-1.2.0, CSVGetField is not exported.......
//      So, make my own simplified copy
/************************************************************************/
/*                           MyCSVGetField()                            */
/*                                                                      */
/************************************************************************/

const char *MyPLIBCSVGetField( const char * pszFilename, const char * pszKeyFieldName,
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

QString GetS57AttributeDecode( QString& att, int ival )
{
    QString ret_val = "";
    
    QString s57data_dir = QString("%1/s57data").arg(zchxFuncUtil::getDataDir());
    if( !s57data_dir.length() ) return ret_val;

    //  Get the attribute code from the acronym
    const char *att_code;
    
    QString file = QString("%1/s57attributes.csv").arg(s57data_dir);
    if( !QFile::exists(file ) ) {
        qDebug( "   Could not open %s", file.toUtf8().data() );
        return ret_val;
    }
    
    att_code = MyPLIBCSVGetField( file.toUtf8().data(), "Acronym",                  // match field
                                  att.toUtf8().data(),               // match value
                                  CC_ExactString, "Code" );             // return field
    
    // Now, get a nice description from s57expectedinput.csv
    //  This will have to be a 2-d search, using ID field and Code field
    
    // Ingest, and get a pointer to the ingested table for "Expected Input" file
    QString ei_file = s57data_dir;
    ei_file.append( "/s57expectedinput.csv" );
    
    if( !QFile::exists(ei_file ) ) {
        qDebug( "   Could not open %s", ei_file.toUtf8().data() );
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
                ret_val = QString::fromUtf8(papszFields[2]);
                bSelected = true;
            }
        }
        
        CSLDestroy( papszFields );
    }
    
    return ret_val;
    
}


//-----------------------------------------------------------------------------
//      Comparison Function for LUPArray sorting
//      Note Global Scope
//-----------------------------------------------------------------------------
#ifndef _COMPARE_LUP_DEFN_
#define _COMPARE_LUP_DEFN_

int CompareLUPObjects( LUPrec *item1, LUPrec *item2 )
{
    // sort the items by their name...
    int ir = strcmp( item1->OBCL, item2->OBCL );
    
    if( ir != 0 )
        return ir;
    int c1 = item1->ATTArray.size();
    int c2 = item2->ATTArray.size();
    
    if( c1 != c2 )
        return c2 - c1;
    return item1->nSequence - item2->nSequence;
}

#endif














//-----------------------------------------------------------------------------
//      LUPArrayContainer implementation
//-----------------------------------------------------------------------------
LUPArrayContainer::LUPArrayContainer()
{
    //   Build the initially empty sorted arrays of LUP Records, per LUP type.
    //   Sorted on object name, e.g. ACHARE.  Why sorted?  Helps in the S52_LUPLookup method....
    LUPArray = new wxArrayOfLUPrec(/* CompareLUPObjects */);
}

LUPArrayContainer::~LUPArrayContainer()
{
    if( LUPArray ) {
        for( unsigned int il = 0; il < LUPArray->count(); il++ )
            s52plib::DestroyLUP( LUPArray->at( il ) );
        
        LUPArray->clear();
        delete LUPArray;
    }
    
    LUPArrayIndexHash::iterator it;
    for( it = IndexHash.begin(); it != IndexHash.end(); ++it ){
        free( it.value());
    }
}

LUPHashIndex *LUPArrayContainer::GetArrayIndexHelper( const char *objectName )
{
    // Look for the key
    QString key = QString::fromUtf8(objectName);
    LUPArrayIndexHash::iterator it = IndexHash.find( key );
    
    if( it == IndexHash.end() )
    {
        //      Key not found, needs to be added
        LUPHashIndex *pindex = (LUPHashIndex *)malloc(sizeof(LUPHashIndex));
        pindex->n_start = -1;
        pindex->count = 0;
        IndexHash[key] = pindex;
        
        //      Find the first matching entry in the LUP Array
        int index = 0;
        int index_max = LUPArray->count();
        int first_match = 0;
        int ocnt = 0;
        LUPrec *LUPCandidate;
        
        //        This technique of extracting proper LUPs depends on the fact that
        //        the LUPs have been sorted in their array, by OBCL.
        //        Thus, all the LUPS with the same OBCL will be grouped together
        
        while( !first_match && ( index < index_max ) ) {
            LUPCandidate = LUPArray->at( index );
            if( !strcmp( objectName, LUPCandidate->OBCL ) ) {
                pindex->n_start = index;
                first_match = 1;
                ocnt++;
                index++;
                break;
            }
            index++;
        }
        
        while( first_match && ( index < index_max ) ) {
            LUPCandidate = LUPArray->at( index );
            if( !strcmp( objectName, LUPCandidate->OBCL ) ) {
                ocnt++;
            } else {
                break;
            }
            
            index++;
        }
        
        pindex->count = ocnt;
        
        return pindex;
    } else
    {
        return it.value();              // return a pointer to the found record
    }
    
}


//-----------------------------------------------------------------------------
//      s52plib implementation
//-----------------------------------------------------------------------------
s52plib::s52plib( const QString& PLib, bool b_forceLegacy )
{
    m_plib_file = PLib;

    pOBJLArray = new wxArrayPtrVoid;

    condSymbolLUPArray = NULL; // Dynamic Conditional Symbology

    _symb_sym = NULL;

    m_txf_ready = false;
    m_txf = NULL;

    ChartSymbols::InitializeGlobals();

    m_bOK = !( S52_load_Plib( PLib, b_forceLegacy ) == 0 );

    m_bShowS57Text = false;
    m_bShowS57ImportantTextOnly = false;
    m_colortable_index = 0;

    _symb_symR = NULL;
    bUseRasterSym = false;

    //      Sensible defaults
    m_nSymbolStyle = PAPER_CHART;
    m_nBoundaryStyle = PLAIN_BOUNDARIES;
    m_nDisplayCategory = OTHER;
    m_nDepthUnitDisplay = 1; // metres
    
    UpdateMarinerParams();

    ledge = new int[2000];
    redge = new int[2000];

    //    Defaults
    m_VersionMajor = 3;
    m_VersionMinor = 2;

    canvas_pix_per_mm = 3.;
    m_rv_scale_factor = 1.0;
    
    //        Set up some default flags
    m_bDeClutterText = false;
    m_bShowAtonText = true;
    m_bShowNationalTexts = false;

    m_bShowSoundg = true;
    m_bShowLdisText = true;
    m_bExtendLightSectors = true;

    // Set a few initial states
    AddObjNoshow( "M_QUAL" );
    m_lightsOff = false;
    m_anchorOn = true;
    m_qualityOfDataOn = false;

    GenerateStateHash();

    HPGL = new RenderFromHPGL( this );
    
    //  Set defaults for OCPN version, may be overridden later
    m_coreVersionMajor = 4;
    m_coreVersionMinor = 6;
    m_coreVersionPatch = 0;
    
    m_myConfig = GetStateHash();

    // GL Options/capabilities
    m_useStencil = false;
    m_useStencilAP = false;
    m_useScissors = false;
    m_useFBO = false;
    m_useVBO = false;
    m_TextureFormat = -1;
    SetGLPolygonSmoothing( true );
    SetGLLineSmoothing( true );
    
}

s52plib::~s52plib()
{
    delete areaPlain_LAC;
    delete line_LAC ;
    delete areaSymbol_LAC;
    delete pointSimple_LAC;
    delete pointPaper_LAC;
    
    S52_flush_Plib();


    //      Free the OBJL Array Elements
    for( unsigned int iPtr = 0; iPtr < pOBJLArray->count(); iPtr++ )
        free( pOBJLArray->at( iPtr ) );

    delete pOBJLArray;

    delete[] ledge;
    delete[] redge;

    ChartSymbols::DeleteGlobals();

    delete HPGL;
}

void s52plib::SetOCPNVersion(int major, int minor, int patch)
{
    m_coreVersionMajor = major;
    m_coreVersionMinor = minor;
    m_coreVersionPatch = patch;
}

void s52plib::SetGLOptions(bool b_useStencil,
                           bool b_useStencilAP,
                           bool b_useScissors,
                           bool b_useFBO,
                           bool b_useVBO,
                           int  nTextureFormat)
{
    // Set GL Options/capabilities
    m_useStencil = b_useStencil;
    m_useStencilAP = b_useStencilAP;
    m_useScissors = b_useScissors;
    m_useFBO = b_useFBO;
    m_useVBO = b_useVBO;
    m_TextureFormat = nTextureFormat;
}

void s52plib::SetPPMM( float ppmm )
{ 
    canvas_pix_per_mm = ppmm;
    
    // We need a supplemental scale factor for HPGL vector symbol rendering.
    //  This will cause raster and vector symbols to be rendered harmoniously
    
    //  We do this by making an arbitrary measurement and declaration:
    // We declare that the nominal size of a "flare" light rendered as HPGL vector should be roughly twice the
    // size of a simplified lateral bouy rendered as raster.
    
    // Referring to the chartsymbols.xml file, we find that the dimension of a flare light is 810 units,
    // and a raster BOYLAT is 16 pix.
    
    m_rv_scale_factor = 2.0 * (1600. / (810 * ppmm));
    
}    

//      Various static helper methods

void s52plib::DestroyLUP( LUPrec *pLUP )
{
    Rules *top = pLUP->ruleList;
    DestroyRulesChain(top);
    
    for(unsigned int i = 0 ; i < pLUP->ATTArray.size() ; i++)
        free (pLUP->ATTArray[i]);
    
    delete pLUP->INST;
}

void s52plib::DestroyRulesChain( Rules *top )
{
    while( top != NULL ) {
        Rules *Rtmp = top->next;
        
        free( top->INST0 ); // free the Instruction string head

        if( top->b_private_razRule ) // need to free razRule?
        {
            Rule *pR = top->razRule;
            delete pR->exposition.LXPO;
            
            free( pR->vector.LVCT );
            
            delete pR->bitmap.SBTM;
            
            free( pR->colRef.SCRF );
            
            ClearRulesCache( pR );
            
            free( pR );
        }
        
        free( top );
        top = Rtmp;
    }
    
}



DisCat s52plib::findLUPDisCat(const char *objectName, LUPname TNAM)
{
    LUPArrayContainer *plac = SelectLUPArrayContainer( TNAM );
    
    wxArrayOfLUPrec *LUPArray = SelectLUPARRAY( TNAM  );
    if(LUPArray)
    {
        //      Find the first matching entry in the LUP Array
        int index = 0;
        int index_max = LUPArray->count();
        LUPrec *LUPCandidate;


        while(  index < index_max  ) {
            LUPCandidate = LUPArray->at( index );
            if( !strcmp( objectName, LUPCandidate->OBCL ) ) {
                return LUPCandidate->DISC;
            }
            index++;
        }
    }
    
    return (DisCat)(-1);
}






bool s52plib::GetAnchorOn()
{
    //  Investigate and report the logical condition that "Anchoring Condition" is shown
    
    int old_vis =  0;
    OBJLElement *pOLE = NULL;

    if(  MARINERS_STANDARD == GetDisplayCategory()){
        old_vis = m_anchorOn;
    }
    else if(OTHER == GetDisplayCategory())
        old_vis = true;

    //other cat
    //const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL", "SBDARE" };

    old_vis &= !IsObjNoshow("SBDARE");

    return (old_vis != 0);
}

bool s52plib::GetQualityOfData()
{
    //  Investigate and report the logical condition that "Quality of Data Condition" is shown
    
    int old_vis =  0;
    OBJLElement *pOLE = NULL;

    if(  MARINERS_STANDARD == GetDisplayCategory()){
        for( unsigned int iPtr = 0; iPtr < pOBJLArray->count(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( pOBJLArray->at( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "M_QUAL", 6 ) ) {
                old_vis = pOLE->nViz;
                break;
            }
        }
    }
    else if(OTHER == GetDisplayCategory())
        old_vis = true;

    old_vis &= !IsObjNoshow("M_QUAL");

    return (old_vis != 0);
}


void s52plib::SetGLRendererString(const QString &renderer)
{
}

/*
 Update the S52 Conditional Symbology Parameter Set to reflect the
 current state of the library member options.
 */

void s52plib::UpdateMarinerParams( void )
{

    //      Symbol Style
    if( SIMPLIFIED == m_nSymbolStyle ) S52_setMarinerParam( S52_MAR_SYMPLIFIED_PNT, 1.0 );
    else
        S52_setMarinerParam( S52_MAR_SYMPLIFIED_PNT, 0.0 );

    //      Boundary Style
    if( SYMBOLIZED_BOUNDARIES == m_nBoundaryStyle ) S52_setMarinerParam( S52_MAR_SYMBOLIZED_BND,
                                                                         1.0 );
    else
        S52_setMarinerParam( S52_MAR_SYMBOLIZED_BND, 0.0 );

}

void s52plib::GenerateStateHash()
{
    unsigned char state_buffer[512];  // Needs to be at least this big...
    memset(state_buffer, 0, sizeof(state_buffer));
    
    int time = QDateTime::currentDateTimeUtc().toTime_t();
    memcpy(state_buffer, &time, sizeof(int));
    
    size_t offset = sizeof(int);           // skipping the time int, first element
    
    for(int i=0 ; i < S52_MAR_NUM ; i++){
        if( (offset + sizeof(double)) < sizeof(state_buffer)){
            double t = S52_getMarinerParam((S52_MAR_param_t) i);
            memcpy( &state_buffer[offset], &t, sizeof(double));
            offset += sizeof(double);
        }
    }
    
    for(unsigned int i=0 ; i < m_noshow_array.count() ; i++){
        if( (offset + 6) < sizeof(state_buffer)){
            memcpy(&state_buffer[offset], m_noshow_array[i].obj, 6) ;
            offset += 6;
        }
    }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
    { memcpy(&state_buffer[offset], &m_bShowSoundg, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
    { memcpy(&state_buffer[offset], &m_bShowS57Text, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
    { memcpy(&state_buffer[offset], &m_bShowS57ImportantTextOnly, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
    { memcpy(&state_buffer[offset], &m_bDeClutterText, sizeof(bool)); offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
    { memcpy(&state_buffer[offset], &m_bShowNationalTexts, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
    { memcpy(&state_buffer[offset], &m_bShowAtonText, sizeof(bool));  offset += sizeof(bool); }

    if(offset + sizeof(bool) < sizeof(state_buffer))
    { memcpy(&state_buffer[offset], &m_bShowLdisText, sizeof(bool));  offset += sizeof(bool); }
    
    if(offset + sizeof(bool) < sizeof(state_buffer))
    { memcpy(&state_buffer[offset], &m_bExtendLightSectors, sizeof(bool));  offset += sizeof(bool); }

    m_state_hash = crc32buf(state_buffer, offset );
    
}

wxArrayOfLUPrec* s52plib::SelectLUPARRAY( LUPname TNAM  )
{
    switch( TNAM ){
    case SIMPLIFIED:
        return pointSimple_LAC->GetLUPArray();
    case PAPER_CHART:
        return pointPaper_LAC->GetLUPArray();
    case LINES:
        return line_LAC->GetLUPArray();
    case PLAIN_BOUNDARIES:
        return areaPlain_LAC->GetLUPArray();
    case SYMBOLIZED_BOUNDARIES:
        return areaSymbol_LAC->GetLUPArray();
    default:
        return NULL;
    }
}

LUPArrayContainer *s52plib::SelectLUPArrayContainer( LUPname TNAM )
{
    switch( TNAM ){
    case SIMPLIFIED:
        return pointSimple_LAC;
    case PAPER_CHART:
        return pointPaper_LAC;
    case LINES:
        return line_LAC;
    case PLAIN_BOUNDARIES:
        return areaPlain_LAC;
    case SYMBOLIZED_BOUNDARIES:
        return areaSymbol_LAC;
    default:
        return NULL;
    }
}


extern Cond condTable[];

LUPrec *s52plib::FindBestLUP( wxArrayOfLUPrec *LUPArray, unsigned int startIndex, unsigned int count, S57Obj *pObj, bool bStrict )
{
    //  Check the parameters
    if( 0 == count )
        return NULL;
    if( startIndex >= LUPArray->count() )
        return NULL;
    
    // setup default return to the first LUP that matches Feature name.
    LUPrec *LUP = LUPArray->at( startIndex );

    int nATTMatch = 0;
    int countATT = 0;
    bool bmatch_found = false;

    if( pObj->att_array == NULL )
        goto check_LUP;       // object has no attributes to compare, so return "best" LUP

    for( unsigned int i = 0; i < count; ++i ) {
        LUPrec *LUPCandidate = LUPArray->at( startIndex + i );
        
        if( !LUPCandidate->ATTArray.size() )
            continue;        // this LUP has no attributes coded

        countATT = 0;
        char *currATT = pObj->att_array;
        int attIdx = 0;

        for( unsigned int iLUPAtt = 0; iLUPAtt < LUPCandidate->ATTArray.size(); iLUPAtt++ ) {

            // Get the LUP attribute name
            char *slatc = LUPCandidate->ATTArray[iLUPAtt];

            if( slatc && (strlen(slatc) < 6) )
                goto next_LUP_Attr;         // LUP attribute value not UTF8 convertible (never seen in PLIB 3.x)

            if( slatc ){
                char *slatv = slatc + 6;
                while( attIdx < pObj->n_attr ) {
                    if( 0 == strncmp( slatc, currATT, 6 ) ) {
                        //OK we have an attribute name match
                        
                        
                        bool attValMatch = false;
                        
                        // special case (i)
                        if( !strncmp( slatv, " ", 1 ) ) {        // any object value will match wild card (S52 para 8.3.3.4)
                            ++countATT;
                            goto next_LUP_Attr;
                        }
                        
                        // special case (ii)
                        //TODO  Find an ENC with "UNKNOWN" DRVAL1 or DRVAL2 and debug this code
                        if( !strncmp( slatv, "?", 1) ){          // if LUP attribute value is "undefined"

                            //  Match if the object does NOT contain this attribute
                            goto next_LUP_Attr;
                        }
                        
                        
                        //checking against object attribute value
                        S57attVal *v = ( pObj->attVal->at( attIdx ) );
                        
                        switch( v->valType ){
                        case OGR_INT: // S57 attribute type 'E' enumerated, 'I' integer
                        {
                            int LUP_att_val = atoi( slatv );
                            if( LUP_att_val == *(int*) ( v->value ) )
                                attValMatch = true;
                            break;
                        }
                            
                        case OGR_INT_LST: // S57 attribute type 'L' list: comma separated integer
                        {
                            int a;
                            char ss[41];
                            strncpy( ss, slatv, 39 );
                            ss[40] = '\0';
                            char *s = &ss[0];

                            int *b = (int*) v->value;
                            sscanf( s, "%d", &a );

                            while( *s != '\0' ) {
                                if( a == *b ) {
                                    sscanf( ++s, "%d", &a );
                                    b++;
                                    attValMatch = true;

                                } else
                                    attValMatch = false;
                            }
                            break;
                        }
                        case OGR_REAL: // S57 attribute type'F' float
                        {
                            double obj_val = *(double*) ( v->value );
                            float att_val = atof( slatv );
                            if( fabs( obj_val - att_val ) < 1e-6 )
                                if( obj_val == att_val  )
                                    attValMatch = true;
                            break;
                        }
                            
                        case OGR_STR: // S57 attribute type'A' code string, 'S' free text
                        {
                            //    Strings must be exact match
                            //    n.b. OGR_STR is used for S-57 attribute type 'L', comma-separated list

                            //QString cs( (char *) v->value, wxConvUTF8 ); // Attribute from object
                            //if( LATTC.Mid( 6 ) == cs )
                            if( !strcmp((char *) v->value, slatv))
                                attValMatch = true;
                            break;
                        }
                            
                        default:
                            break;
                        } //switch
                        
                        // value match
                        if( attValMatch )
                            ++countATT;

                        goto next_LUP_Attr;
                    } // if attribute name match
                    
                    //  Advance to the next S57obj attribute
                    currATT += 6;
                    ++attIdx;
                    
                } //while
            } //if
            
next_LUP_Attr:

            currATT = pObj->att_array; // restart the object attribute list
            attIdx = 0;
        } // for iLUPAtt
        
        //      Create a "match score", defined as fraction of candidate LUP attributes
        //      actually matched by feature.
        //      Used later for resolving "ties"
        
        int nattr_matching_on_candidate = countATT;
        int nattrs_on_candidate = LUPCandidate->ATTArray.size();
        double candidate_score = ( 1. * nattr_matching_on_candidate )
                / ( 1. * nattrs_on_candidate );
        
        //       According to S52 specs, match must be perfect,
        //         and the first 100% match is selected
        if( candidate_score == 1.0 ) {
            LUP = LUPCandidate;
            bmatch_found = true;
            break; // selects the first 100% match
        }
        
    } //for loop
    

check_LUP:
    //  In strict mode, we require at least one attribute to match exactly
    
    if( bStrict ) {
        if( nATTMatch == 0 ) // nothing matched
            LUP = NULL;
    } else {
        //      If no match found, return the first LUP in the list which has no attributes
        if( !bmatch_found ) {
            for( unsigned int j = 0; j < count; ++j ) {
                LUPrec *LUPtmp = NULL;
                
                LUPtmp = LUPArray->at( startIndex + j );
                if( !LUPtmp->ATTArray.size() ) {
                    return LUPtmp;
                }
            }
        }
    }
    
    return LUP;
}




// scan foward stop on ; or end-of-record
#define SCANFWRD        while( !(*str == ';' || *str == '\037')) ++str;

#define INSTRUCTION(s,t)        if(0==strncmp(s,str,2)){\
    str+=3;\
    r->ruleType = t;\
    r->INSTstr  = str;

Rules *s52plib::StringToRules( const QString& str_in )
{
    QByteArray buffer=str_in.toUtf8();
    if(!buffer.data()) return NULL;

    size_t len = strlen( buffer.data() );
    char *str0 = (char *) calloc( len + 1, 1 );
    memcpy( str0, buffer.data(), len );
    char *str = str0;

    Rules *top;
    Rules *last;
    char strk[20];

    //    Allocate and pre-clear the Rules structure
    Rules *r = (Rules*) calloc( 1, sizeof(Rules) );
    top = r;
    last = top;

    r->INST0 = str0; // save the head for later free

    while( *str != '\0' ) {
        if( r->ruleType ) // in the loop, r has been used
        {
            r = (Rules*) calloc( 1, sizeof(Rules) );
            last->next = r;
            last = r;
        }

        // parse Symbology instruction in string

        // Special Case for Circular Arc,  (opencpn private)
        // Allocate a Rule structure to be used to hold a cached bitmap of the created symbol
        INSTRUCTION ( "CA",RUL_ARC_2C )
                r->razRule = (Rule*) calloc( 1, sizeof(Rule) );
        r->b_private_razRule = true; // mark this raxRule to be free'd later
        SCANFWRD
    }

    // Special Case for MultPoint Soundings
    INSTRUCTION ( "MP",RUL_MUL_SG )
            SCANFWRD
}

// SHOWTEXT
INSTRUCTION ( "TX",RUL_TXT_TX )
SCANFWRD
}

INSTRUCTION ( "TE",RUL_TXT_TE )
SCANFWRD
}

// SHOWPOINT

if( 0 == strncmp( "SY", str, 2 ) ) {
    str += 3;
    r->ruleType = RUL_SYM_PT;
    r->INSTstr = str;

    strncpy( strk, str, 8 );
    strk[8] = 0;
    QString key = QString::fromUtf8(strk);

    r->razRule = ( *_symb_sym )[key];

    if( r->razRule == NULL ) r->razRule = ( *_symb_sym )["QUESMRK1"];

    SCANFWRD
}

// SHOWLINE
INSTRUCTION ( "LS",RUL_SIM_LN )
SCANFWRD
}

INSTRUCTION ( "LC",RUL_COM_LN )
strncpy( strk, str, 8 );
strk[8] = 0;
QString key = QString::fromUtf8(strk);

r->razRule = ( *_line_sym )[key];

if( r->razRule == NULL ) r->razRule = ( *_symb_sym )["QUESMRK1"];
SCANFWRD
}

// SHOWAREA
INSTRUCTION ( "AC",RUL_ARE_CO )
SCANFWRD
}

INSTRUCTION ( "AP",RUL_ARE_PA )
strncpy( strk, str, 8 );
strk[8] = 0;
QString key = QString::fromUtf8(strk);

r->razRule = ( *_patt_sym )[key];
if( r->razRule == NULL ) r->razRule = ( *_patt_sym )["QUESMRK1V"];
SCANFWRD
}

// CALLSYMPROC

if( 0 == strncmp( "CS", str, 2 ) ) {
    str += 3;
    r->ruleType = RUL_CND_SY;
    r->INSTstr = str;

    //      INSTRUCTION("CS",RUL_CND_SY)
    char stt[9];
    strncpy( stt, str, 8 );
    stt[8] = 0;
    QString key = QString::fromUtf8(stt);
    r->razRule = ( *_cond_sym )[key];
    if( r->razRule == NULL ) r->razRule = ( *_cond_sym )["QUESMRK1"];
    SCANFWRD
}

++str;
}

//  If it should happen that no rule is built, delete the initially allocated rule
if( 0 == top->ruleType ) {
    if( top->INST0 ) free( top->INST0 );

    free( top );

    top = NULL;
}

//   Traverse the entire rule set tree, pruning after first unallocated (dead) rule
r = top;
while( r ) {
    if( 0 == r->ruleType ) {
        free( r );
        last->next = NULL;
        break;
    }

    last = r;
    Rules *n = r->next;
    r = n;
}

//   Traverse the entire rule set tree, adding sequence numbers
r = top;
int i = 0;
while( r ) {
    r->n_sequence = i++;

    r = r->next;
}

return top;
}

int s52plib::_LUP2rules( LUPrec *LUP, S57Obj *pObj )
{
    if( NULL == LUP ) return -1;
    // check if already parsed
    if( LUP->ruleList != NULL ) {
        return 0;
    }

    if( LUP->INST != NULL ) {
        Rules *top = StringToRules( *LUP->INST );
        LUP->ruleList = top;

        return 1;
    } else
        return 0;
}


int s52plib::S52_load_Plib( const QString& PLib, bool b_forceLegacy )
{

    pAlloc = new wxArrayPtrVoid;

    //   Create the Rule Lookup Hash Tables
    _line_sym = new RuleHash; // line
    _patt_sym = new RuleHash; // pattern
    _symb_sym = new RuleHash; // symbol
    _cond_sym = new RuleHash; // conditional

    line_LAC = new LUPArrayContainer;
    areaPlain_LAC= new LUPArrayContainer;
    areaSymbol_LAC= new LUPArrayContainer;
    pointSimple_LAC= new LUPArrayContainer;
    pointPaper_LAC= new LUPArrayContainer;
    
    condSymbolLUPArray = new wxArrayOfLUPrec( /*CompareLUPObjects*/ ); // dynamic Cond Sym LUPs

    m_unused_color.R = 2;
    m_unused_color.G = 2;
    m_unused_color.B = 2;
    m_unused_QColor.setRgb(2,2,2);

#if 0
    // First, honor the user attempt for force Lecagy mode.
    // Next, try to load symbols using the newer XML/PNG format.
    // If this fails, try Legacy S52RAZDS.RLE file.

    if( b_forceLegacy ) {
        RazdsParser parser;
        useLegacyRaster = true;
        if( parser.LoadFile( this, PLib ) ) {
            QString msg( "Loaded legacy PLIB data: " );
            msg += PLib;
            ZCHX_LOGMSG( msg );
        } else
            return 0;
    } else {
        ChartSymbols chartSymbols;
        useLegacyRaster = false;
        if( !chartSymbols.LoadConfigFile( this, PLib ) ) {
            RazdsParser parser;
            useLegacyRaster = true;
            if( parser.LoadFile( this, PLib ) ) {
                QString msg( "Loaded legacy PLIB data: " );
                msg += PLib;
                ZCHX_LOGMSG( msg );
            } else
                return 0;
        }
    }
#endif
    ChartSymbols chartSymbols;
    useLegacyRaster = false;
    if( !chartSymbols.LoadConfigFile( this, PLib ) ) {
        qDebug("Could not load XML PLib symbol file: %s", PLib.toUtf8().data());
        return 0;
    }

    //   Initialize the _cond_sym Hash Table from the jump table found in S52CNSY.CPP
    //   Hash Table indices are the literal CS Strings, e.g. "RESARE02"
    //   Hash Results Values are the Rule *, i.e. the CS procedure entry point

    for( int i = 0; condTable[i].condInst != NULL; ++i ) {
        QString index = QString::fromUtf8(condTable[i].name);
        ( *_cond_sym )[index] = (Rule *) ( condTable[i].condInst );
    }

    QString s57data_dir = zchxFuncUtil::getDataDir();
    s57data_dir += "/s57data";
    
    QString oc_file( s57data_dir );
    oc_file.append( "/s57objectclasses.csv" );

    PreloadOBJLFromCSV( oc_file );

    return 1;
}

void s52plib::ClearRulesCache( Rule *pR ) //  Clear out any existing cached symbology
{
    switch( pR->parm0 ){
    case ID_wxBitmap: {
        QBitmap *pbm = (QBitmap *) ( pR->pixelPtr );
        delete pbm;
        pR->pixelPtr = NULL;
        pR->parm0 = ID_EMPTY;
        break;
    }
    case ID_RGBA: {
        unsigned char *p = (unsigned char *) ( pR->pixelPtr );
        free( p );
        pR->pixelPtr = NULL;
        pR->parm0 = ID_EMPTY;
        break;
    }
    case ID_GL_PATT_SPEC: {
        render_canvas_parms *pp = (render_canvas_parms *) ( pR->pixelPtr );
        free( pp->pix_buff );
#ifdef ocpnUSE_GL
        if(pp->OGL_tex_name) glDeleteTextures( 1, (GLuint *) &pp->OGL_tex_name );
#endif
        delete pp;
        pR->pixelPtr = NULL;
        pR->parm0 = ID_EMPTY;
        break;
    }
    case ID_RGB_PATT_SPEC: {
        render_canvas_parms *pp = (render_canvas_parms *) ( pR->pixelPtr );
        free( pp->pix_buff );
        delete pp;
        pR->pixelPtr = NULL;
        pR->parm0 = ID_EMPTY;
        break;
    }
    case ID_EMPTY:
        break;
    default:
        assert(false);
        break;
    }
}

void s52plib::DestroyPatternRuleNode( Rule *pR )
{
    DestroyRuleNode(pR);
}

void s52plib::DestroyRuleNode( Rule *pR )
{
    if( !pR )
        return;

    delete pR->exposition.LXPO;

    free( pR->vector.LVCT );

    delete pR->bitmap.SBTM;

    free( pR->colRef.SCRF );

    ClearRulesCache( pR ); //  Clear out any existing cached symbology
}

void s52plib::DestroyRules( RuleHash *rh )
{
    RuleHash::iterator it;
    Rule *pR;

    for( it = ( *rh ).begin(); it != ( *rh ).end(); ++it ) {
        pR = it.value();
        DestroyRuleNode( pR );
    }

    rh->clear();
    delete rh;
}

void s52plib::FlushSymbolCaches( void )
{
    if( !useLegacyRaster ) ChartSymbols::LoadRasterFileForColorTable( m_colortable_index, true );

    RuleHash *rh = _symb_sym;

    if( !rh ) return;

    RuleHash::iterator it;
    Rule *pR;

    for( it = ( *rh ).begin(); it != ( *rh ).end(); ++it ) {
        pR = it.value();
        if( pR ) ClearRulesCache( pR );
    }

    //    Flush any pattern definitions
    rh = _patt_sym;

    if( !rh ) return;

    for( it = ( *rh ).begin(); it != ( *rh ).end(); ++it ) {
        pR = it.value();
        if( pR ) ClearRulesCache( pR );
    }

    //    OpenGL Hashmaps
    CARC_Hash::iterator ita;
    for( ita = m_CARC_hashmap.begin(); ita != m_CARC_hashmap.end(); ++ita ) {
        CARC_Buffer buffer = ita.value();
        delete [] buffer.data;
    }
    m_CARC_hashmap.clear();

#ifdef ocpnUSE_GL    
    CARC_DL_Hash::iterator itd;
    for( itd = m_CARC_DL_hashmap.begin(); itd != m_CARC_DL_hashmap.end(); ++itd ) {
        GLuint list = itd.value();
        glDeleteLists( list, 1 );
    }
    m_CARC_DL_hashmap.clear();
    
#endif
}

void s52plib::DestroyPattRules( RuleHash *rh )
{
    DestroyRules(rh);
}


void s52plib::DestroyLUPArray( wxArrayOfLUPrec *pLUPArray )
{
    if( pLUPArray ) {
        for( unsigned int il = 0; il < pLUPArray->count(); il++ )
            DestroyLUP( pLUPArray->at( il ) );

        pLUPArray->clear();

        delete pLUPArray;
    }
}

void s52plib::ClearCNSYLUPArray( void )
{
    if( condSymbolLUPArray ) {
        for( unsigned int i = 0; i < condSymbolLUPArray->count(); i++ )
            DestroyLUP( condSymbolLUPArray->at( i ) );

        condSymbolLUPArray->clear();
    }
}

bool s52plib::S52_flush_Plib()
{
    if(!m_bOK)
        return false;

#ifdef ocpnUSE_GL
    //    OpenGL Hashmaps
    CARC_Hash::iterator ita;
    for( ita = m_CARC_hashmap.begin(); ita != m_CARC_hashmap.end(); ++ita ) {
        CARC_Buffer buffer = ita.value();
        delete [] buffer.data;
    }
    m_CARC_hashmap.clear();

    CARC_DL_Hash::iterator itd;
    for( itd = m_CARC_DL_hashmap.begin(); itd != m_CARC_DL_hashmap.end(); ++itd ) {
        GLuint list = itd.value();
        glDeleteLists( list, 1 );
    }
    m_CARC_DL_hashmap.clear();
    
#endif
    
    DestroyLUPArray( condSymbolLUPArray );

    //      Destroy Rules
    DestroyRules( _line_sym );
    DestroyPattRules( _patt_sym );
    DestroyRules( _symb_sym );

    if( _symb_symR ) DestroyRules( _symb_symR );

    //      Special case for CS
    _cond_sym->clear();
    delete ( _cond_sym );

    for( unsigned int ipa = 0; ipa < pAlloc->count(); ipa++ ) {
        void *t = pAlloc->at( ipa );
        free( t );
    }

    pAlloc->clear();
    delete pAlloc;

    return true;
}

LUPrec *s52plib::S52_LUPLookup( LUPname LUP_Name, const char * objectName, S57Obj *pObj, bool bStrict )
{
    LUPrec *LUP = NULL;

    LUPArrayContainer *plac = SelectLUPArrayContainer( LUP_Name );
    
    LUPHashIndex *hip = plac->GetArrayIndexHelper( objectName );
    int nLUPs = hip->count;
    int nStartIndex = hip->n_start;
    
    LUP = FindBestLUP( plac->GetLUPArray(), nStartIndex, nLUPs, pObj, bStrict );
    
    return LUP;
}


void  s52plib::SetPLIBColorScheme( ColorScheme cs )
{
    QString SchemeName;
    switch( cs ){
    case GLOBAL_COLOR_SCHEME_DAY:
        SchemeName = "DAY";
        break;
    case GLOBAL_COLOR_SCHEME_DUSK:
        SchemeName = "DUSK";
        break;
    case GLOBAL_COLOR_SCHEME_NIGHT:
        SchemeName = "NIGHT";
        break;
    default:
        SchemeName = "DAY";
        break;
    }
    
    SetPLIBColorScheme( SchemeName );
}



void s52plib::SetPLIBColorScheme( QString scheme )
{
    QString str_find;
    str_find = scheme;
    m_colortable_index = 0; // default is the first color in the table

    // Of course, it also depends on the plib version...
    // plib version 3.2 calls "DAY" colr as "DAY_BRIGHT"

    if( ( GetMajorVersion() == 3 ) && ( GetMinorVersion() == 2 ) ) {
        if( scheme == "DAY") str_find = "DAY_BRIGHT";
    }
    m_colortable_index = ChartSymbols::FindColorTable( scheme );

    //    if( !useLegacyRaster ) ChartSymbols::LoadRasterFileForColorTable( m_colortable_index );

    if( !useLegacyRaster ) ChartSymbols::SetColorTableIndex( m_colortable_index );
    
    m_ColorScheme = scheme;
}

S52color* s52plib::getColor( const char *colorName )
{
    S52color* c;
    c = ChartSymbols::GetColor( colorName, m_colortable_index );
    return c;
}

QColor s52plib::getQColor( const QString &colorName )
{
    QColor c;
    c = ChartSymbols::GetQColor( colorName, m_colortable_index );
    return c;
}

//----------------------------------------------------------------------------------
//
//              Object Rendering Module
//
//----------------------------------------------------------------------------------

//-----------------------------
//
// S52 TEXT COMMAND WORD PARSER
//
//-----------------------------
#define APOS   '\047'
#define MAXL       512

char *s52plib::_getParamVal( ObjRazRules *rzRules, char *str, char *buf, int bsz )
// Symbology Command Word Parameter Value Parser.
//
//      str: psz to Attribute of interest
//
//      Results:Put in 'buf' one of:
//  1- LUP constant value,
//  2- ENC value,
//  3- LUP default value.
// Return pointer to the next field in the string (delim is ','), NULL to abort
{
    QString value;
    int defval = 0; // default value
    int len = 0;
    char *ret_ptr = str;
    char *tmp = buf;

    if(!buf)
        return NULL;

    buf[0] = 0;
    // parse constant parameter with concatenation operator "'"
    if( str != NULL ) {
        if( *ret_ptr == APOS ) {
            ret_ptr++;
            while( *ret_ptr != APOS &&  *ret_ptr != '\0' && len < ( bsz - 1 )) {
                ++len;
                *tmp++ = *ret_ptr++;
            }
            *tmp = '\0';
            ret_ptr++; // skip "'"
            ret_ptr++; // skip ","

            return ret_ptr;
        }

        while( *ret_ptr != ',' && *ret_ptr != ')' && *ret_ptr != '\0' && len < ( bsz - 1 ) ) {
            *tmp++ = *ret_ptr++;
            ++len;
        }
        *tmp = '\0';

        ret_ptr++; // skip ',' or ')'
    }
    if( len < 6 )
        return ret_ptr;

    // chop string if default value present
    if( len > 6 && *( buf + 6 ) == '=' ) {
        *( buf + 6 ) = '\0';
        defval = 1;
    }

    value = rzRules->obj->GetAttrValueAsString( buf );
    QByteArray buffer = value.toUtf8();
    if(!buffer.data()) return ret_ptr;

    if( value.isEmpty() ) {
        if( defval )
            _getParamVal( rzRules, buf + 7, buf, bsz - 7 ); // default value --recursion
        else {
            return NULL; // abort
        }
    } else {

        //    Special case for conversion of some vertical (height) attributes to feet
        if( ( !strncmp( buf, "VERCLR", 6 ) ) || ( !strncmp( buf, "VERCCL", 6 ) ) || ( !strncmp( buf, "VERCOP", 6 ) ) ) {
            switch( m_nDepthUnitDisplay )
            {
            case 0: // feet
            case 2: // fathoms
            {
                double ft_val = value.toDouble();
                ft_val = ft_val * 3 * 39.37 / 36; // feet
                value.sprintf("%4.1f", ft_val );
                break;
            }
            default:
                break;
            }
        }

        // special case when ENC returns an index for particular attribute types
        if( !strncmp( buf, "NATSUR", 6 ) ) {
            QString natsur_att("NATSUR" );
            QString result;
            QString svalue = value;
            QStringList tkz = svalue.split("," );

            int icomma = 0;
            int k =0;
            while( k < tkz.size() ) {
                if( icomma )
                    result += ( "," );

                QString token = tkz[k++];
                bool ok = false;
                long i = token.toLong(&ok);
                if( ok){
                    QString nat;
                    if( !m_natsur_hash[i].isEmpty() )            // entry available?
                        nat = m_natsur_hash[i];
                    else {
                        nat = GetS57AttributeDecode( natsur_att, (int)i );
                        m_natsur_hash[i] = nat;            // cache the entry
                    }

                    if( !nat.isEmpty() )
                        result += nat; // value from ENC
                    else
                        result += ( "unk" );
                }
                else
                    result += ( "unk" );

                icomma++;
            }

            value = result;
        }

        QByteArray buffer = value.toUtf8();
        if(buffer.data()){
            unsigned int len = qMin((unsigned int)(strlen(buffer.data())), (unsigned int)bsz-1);
            memcpy( buf, buffer.data(), len );
            buf[len] = 0;
        }
        else
            *buf = 0;
    }

    return ret_ptr;
}

char *s52plib::_parseTEXT( ObjRazRules *rzRules, S52_TextC *text, char *str0 )
{
    char buf[MAXL]; // output string

    char *str = str0;
    if( text ) {
        memset(buf, 0, 4);
        str = _getParamVal( rzRules, str, &text->hjust, MAXL ); // HJUST
        str = _getParamVal( rzRules, str, &text->vjust, MAXL ); // VJUST
        str = _getParamVal( rzRules, str, &text->space, MAXL ); // SPACE

        // CHARS
        str = _getParamVal( rzRules, str, buf, MAXL );
        text->style = buf[0];
        text->weight = buf[1];
        text->width = buf[2];
        text->bsize = atoi( buf + 3 );

        str = _getParamVal( rzRules, str, buf, MAXL );
        text->xoffs = atoi( buf );
        str = _getParamVal( rzRules, str, buf, MAXL );
        text->yoffs = atoi( buf );
        str = _getParamVal( rzRules, str, buf, MAXL );
        text->pcol = getColor( buf );
        str = _getParamVal( rzRules, str, buf, MAXL );
        text->dis = atoi( buf ); // Text Group, used for "Important" text detection
    }
    return str;
}

S52_TextC *s52plib::S52_PL_parseTX( ObjRazRules *rzRules, Rules *rules, char *cmd )
{
    S52_TextC *text = NULL;
    char *str = NULL;
    char val[MAXL]; // value of arg
    char strnobjnm[7] = { "NOBJNM" };
    char valn[MAXL]; // value of arg

    valn[0] = 0;
    str = (char*) rules->INSTstr;

    if( m_bShowNationalTexts && NULL != strstr( str, "OBJNAM" ) ) // in case user wants the national text shown and the rule contains OBJNAM, try to get the value
    {
        _getParamVal( rzRules, strnobjnm, valn, MAXL );
        if( !strcmp( strnobjnm, valn ) )
            valn[0] = '\0'; //NOBJNM is not defined
        else
            valn[MAXL - 1] = '\0'; // make sure the string terminates
    }

    str = _getParamVal( rzRules, str, val, MAXL ); // get ATTRIB list

    if( NULL == str ) return 0; // abort this command word if mandatory param absent

    val[MAXL - 1] = '\0'; // make sure the string terminates

    text = new S52_TextC;
    str = _parseTEXT( rzRules, text, str );
    if( NULL != text )
    {
        if ( valn[0] != '\0' ) {
            text->frmtd = QString::fromUtf8(valn );
            text->bnat = true;
        }
        else {
            text->frmtd = QString::fromUtf8(val );
            text->bnat = false;
        }
    }

    //  We check to see if the formatted text has any "special" characters
    QByteArray buf = text->frmtd.toUtf8();
    
    unsigned int n = text->frmtd.length();
    for(unsigned int i=0 ; i < n ; ++i){
        unsigned char c = buf[i];
        if(c > 127){
            text->bspecial_char = true;
            break;
        }
    }
    
    return text;
}

S52_TextC *s52plib::S52_PL_parseTE( ObjRazRules *rzRules, Rules *rules, char *cmd )
// same as S52_PL_parseTX put parse 'C' format first
{
    char arg[MAXL]; // ATTRIB list
    char fmt[MAXL]; // FORMAT
    char buf[MAXL]; // output string
    char *b = buf;
    char *parg = arg;
    char *pf = fmt;
    S52_TextC *text = NULL;

    char *str = (char*) rules->INSTstr;

    if( str && *str ) {
        str = _getParamVal( rzRules, str, fmt, MAXL ); // get FORMAT

        str = _getParamVal( rzRules, str, arg, MAXL ); // get ATTRIB list
        if( NULL == str ) return 0; // abort this command word if mandatory param absent

        //*b = *pf;
        while( *pf != '\0' ) {

            // begin a convertion specification
            if( *pf == '%' ) {
                char val[MAXL]; // value of arg
                char tmp[MAXL] = { '\0' }; // temporary format string
                char *t = tmp;
                int cc = 0; // 1 == Conversion Character found
                //*t = *pf;

                // get value for this attribute
                parg = _getParamVal( rzRules, parg, val, MAXL );
                if( NULL == parg ) return 0; // abort

                if( 0 == strcmp( val, "2147483641" ) ) return 0;

                *t = *pf; // stuff the '%'

                // scan for end at convertion character
                do {
                    *++t = *++pf; // fill conver spec

                    switch( *pf ){
                    case 'c':
                    case 's':
                        b += sprintf( b, tmp, val );
                        cc = 1;
                        break;
                    case 'f':
                        b += sprintf( b, tmp, atof( val ) );
                        cc = 1;
                        break;
                    case 'd':
                    case 'i':
                        b += sprintf( b, tmp, atoi( val ) );
                        cc = 1;
                        break;
                    }
                } while( !cc );
                pf++; // skip conv. char

            } else
                *b++ = *pf++;
        }

        *b = 0;
        text = new S52_TextC;
        str = _parseTEXT( rzRules, text, str );
        if( NULL != text ) text->frmtd = QString::fromUtf8(buf);
        
        //  We check to see if the formatted text has any "special" characters
        QByteArray buf = text->frmtd.toUtf8();
        
        unsigned int n = text->frmtd.length();
        for(unsigned int i=0 ; i < n ; ++i){
            unsigned char c =buf[i];
            if(c > 127){
                text->bspecial_char = true;
                break;
            }
        }
        
    }

    return text;
}

static void rotate(QRect *r, ViewPort const &vp)
{
    float cx = vp.pixWidth()/2.;
    float cy = vp.pixHeight()/2.;
    float c = cosf(vp.rotation() );
    float s = sinf(vp.rotation() );
    float x = r->x() -cx;
    float y = r->y() -cy;
    r->setX( x*c - y*s +cx);
    r->setY( x*s + y*c +cy);
}

bool s52plib::RenderText(S52_TextC *ptext, int x, int y, QRect *pRectDrawn,
                         S57Obj *pobj, bool bCheckOverlap, ViewPort *vp )
{
#ifdef DrawText
#undef DrawText
#define FIXIT
#endif
    bool bdraw = true;
    
    QFont *scaled_font = ptext->pFont;
    int w_scaled = 0;
    int h_scaled = 0;
    int descent = 0;
    int exlead = 0;
    
    double sfactor = vp->refScale()/vp->chartScale();
    double scale_factor = qMax((sfactor - g_overzoom_emphasis_base)  / 4., 1.);
    
    if(!g_oz_vector_scale || !vp->quilt())
        scale_factor = 1.0;
    
    //  Place an upper bound on the scaled text size
    scale_factor = qMin(scale_factor, 4.0);

    bool b_force_no_texture = false;
    if(scale_factor > 1.){
        b_force_no_texture = true;

        int old_size = ptext->pFont->pointSize();
        int new_size = old_size * scale_factor;
        scaled_font->setPointSize(new_size);
        scaled_font->setUnderline(false);
        //            wxScreenDC sdc;
        //            sdc.GetTextExtent( ptext->frmtd, &w_scaled, &h_scaled, &descent, &exlead, scaled_font ); // measure the text
        QFontMetrics mecs(*scaled_font);
        w_scaled = mecs.width(ptext->frmtd);
        h_scaled = mecs.height();
        descent = mecs.descent();
        exlead = mecs.leading();

        // Has font size changed?  If so, clear the cached bitmap, and rebuild it
        if( (h_scaled - descent) != ptext->rendered_char_height){
            glDeleteTextures(1, (GLuint *)&ptext->texobj);
            ptext->texobj = 0;
        }

        // We cannot get the font ascent value to remove the interline spacing from the font "height".
        // So we have to estimate based on conventional Arial metrics
        ptext->rendered_char_height = (h_scaled - descent) * 8 / 10;

    }
    // We render string with "special" characters the old, hard way, since we don't necessarily have the glyphs in our font,
    // or if we do we would need a hashmap to cache and extract them
    // And we also do this if the text is to be scaled up artificially.
    if( (ptext->bspecial_char) || b_force_no_texture) {
        if( !ptext->texobj ) // is texture ready?
        {
            //                wxScreenDC sdc;

            if(scale_factor <= 1.){
                //                    sdc.GetTextExtent( ptext->frmtd, &w_scaled, &h_scaled, &descent, &exlead, scaled_font ); // measure the text
                QFontMetrics mecs(*scaled_font);
                w_scaled = mecs.width(ptext->frmtd);
                h_scaled = mecs.height();
                descent = mecs.descent();
                exlead = mecs.leading();
                // We cannot get the font ascent value to remove the interline spacing from the font "height".
                // So we have to estimate based on conventional Arial metrics
                ptext->rendered_char_height = (h_scaled - descent) * 8 / 10;
            }

            ptext->text_width = w_scaled;
            ptext->text_height = h_scaled;

            /* make power of 2 */
            int tex_w, tex_h;
            for(tex_w = 1; tex_w < ptext->text_width; tex_w *= 2);
            for(tex_h = 1; tex_h < ptext->text_height; tex_h *= 2);

            QBitmap bmp( tex_w, tex_h );
            QPainter mdc;
            mdc.begin(&bmp);
            mdc.setFont(*( scaled_font ) );
            //  Render the text as white on black, so that underlying anti-aliasing of
            //  QPainter text rendering can be extracted and converted to alpha-channel values.

            mdc.setBackground(QBrush( QColor( 0, 0, 0 ) ) );
            mdc.setBackgroundMode(Qt::TransparentMode );
            QPen pen = mdc.pen();
            pen.setColor(QColor( 255, 255, 255 ) );
            mdc.setPen(pen);
            mdc.drawText(0, 0, ptext->frmtd);
            mdc.end();
            QImage image = bmp.toImage();
            int ws = image.width(), hs = image.height();

            ptext->RGBA_width = ws;
            ptext->RGBA_height = hs;
            unsigned char *pRGBA = (unsigned char *) malloc( 4 * ws * hs );

            unsigned char *d = image.bits();
            unsigned char *pdest = pRGBA;
            S52color *ccolor = ptext->pcol;

            if(d){
                for( int y = 0; y < hs; y++ )
                    for( int x = 0; x < ws; x++ ) {
                        unsigned char r, g, b;
                        int off = ( y * ws + x );

                        r = d[off * 3 + 0];
                        g = d[off * 3 + 1];
                        b = d[off * 3 + 2];

                        pdest[off * 4 + 0] = ccolor->R;
                        pdest[off * 4 + 1] = ccolor->G;
                        pdest[off * 4 + 2] = ccolor->B;

                        int alpha = ( r + g + b ) / 3;
                        pdest[off * 4 + 3] = (unsigned char) ( alpha & 0xff );
                    }
            }


            int draw_width = ptext->RGBA_width;
            int draw_height = ptext->RGBA_height;

            glEnable( GL_TEXTURE_2D );

            GLuint texobj;
            glGenTextures( 1, &texobj );

            glBindTexture( GL_TEXTURE_2D, texobj );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST/*GL_LINEAR*/ );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, draw_width, draw_height, 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, pRGBA );

            free( pRGBA );

            ptext->texobj = texobj;
        } // Building texobj

        //    Render the texture
        if( ptext->texobj ) {
            //  Adjust the y position to account for the convention that S52 text is drawn
            //  with the lower left corner at the specified point, instead of the wx convention
            //  using upper right corner
            int yadjust = 0;
            int xadjust = 0;

            yadjust =  -ptext->rendered_char_height;


            //  Add in the offsets, specified in units of nominal font height
            yadjust += ptext->yoffs * ( ptext->rendered_char_height );
            //  X offset specified in units of average char width
            xadjust += ptext->xoffs * ptext->avgCharWidth;

            // adjust for text justification
            int w = ptext->avgCharWidth * ptext->frmtd.length();
            switch ( ptext->hjust){
            case '1':               // centered
                xadjust -= w/2;
                break;
            case '2':               // right
                xadjust -= w;
                break;
            case '3':               // left (default)
            default:
                break;
            }

            switch ( ptext->vjust){
            case '3':               // top
                yadjust += ptext->rendered_char_height;
                break;
            case '2':               // centered
                yadjust += ptext->rendered_char_height/2;
                break;
            case '1':               // bottom (default)
            default:
                break;
            }

            int xp = x;
            int yp = y;

            if(fabs(vp->rotation()) > 0.01){
                float c = cosf(-vp->rotation() );
                float s = sinf(-vp->rotation() );
                float x = xadjust;
                float y = yadjust;
                xp += x*c - y*s;
                yp += x*s + y*c;

            }
            else{
                xp+= xadjust;
                yp+= yadjust;
            }


            pRectDrawn->setX( xp );
            pRectDrawn->setY( yp );
            pRectDrawn->setWidth( ptext->text_width );
            pRectDrawn->setHeight( ptext->text_height );

            if( bCheckOverlap ) {
                if( CheckTextRectList( *pRectDrawn, ptext ) )
                    bdraw = false;
            }

            if( bdraw ) {

                int draw_width = ptext->text_width;
                int draw_height = ptext->text_height;

                extern GLenum       g_texture_rectangle_format;

                glEnable( GL_BLEND );
                glEnable( GL_TEXTURE_2D );

                glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

                glBindTexture( GL_TEXTURE_2D, ptext->texobj );

                glPushMatrix();
                glTranslatef(xp, yp, 0);

                /* undo previous rotation to make text level */
                glRotatef(vp->rotation()*180/PI, 0, 0, -1);


                float tx1 = 0, tx2 = draw_width;
                float ty1 = 0, ty2 = draw_height;

                if(g_texture_rectangle_format == GL_TEXTURE_2D) {

                    tx1 /= ptext->RGBA_width, tx2 /= ptext->RGBA_width;
                    ty1 /= ptext->RGBA_height, ty2 /= ptext->RGBA_height;
                }


                glBegin( GL_QUADS );

                glTexCoord2f( tx1, ty1 );  glVertex2i( 0, 0 );
                glTexCoord2f( tx2, ty1 );  glVertex2i( draw_width, 0 );
                glTexCoord2f( tx2, ty2 );  glVertex2i( draw_width, draw_height );
                glTexCoord2f( tx1, ty2 );  glVertex2i( 0, draw_height );

                glEnd();

                glPopMatrix();

                glDisable( g_texture_rectangle_format );
                glDisable( GL_BLEND );


            } // bdraw

        }

        bdraw = true;
    }

    else {                                          // render using cached texture glyphs
        TexFont *f_cache = GetTexFont(ptext->pFont);
        int w, h;
        f_cache->GetTextExtent(ptext->frmtd, &w, &h);

        // We don't store descent/ascent info for font texture cache
        // So we have to estimate based on conventional Arial metrics
        ptext->rendered_char_height = h * 65 / 100;

        //  Adjust the y position to account for the convention that S52 text is drawn
        //  with the lower left corner at the specified point, instead of the wx convention
        //  using upper right corner
        int yadjust = 0;
        int xadjust = 0;

        yadjust =  -ptext->rendered_char_height;


        //  Add in the offsets, specified in units of nominal font height
        yadjust += ptext->yoffs * ( ptext->rendered_char_height );
        //  X offset specified in units of average char width
        xadjust += ptext->xoffs * ptext->avgCharWidth;

        // adjust for text justification
        switch ( ptext->hjust){
        case '1':               // centered
            xadjust -= w/2;
            break;
        case '2':               // right
            xadjust -= w;
            break;
        case '3':               // left (default)
        default:
            break;
        }

        switch ( ptext->vjust){
        case '3':               // top
            yadjust += ptext->rendered_char_height;
            break;
        case '2':               // centered
            yadjust += ptext->rendered_char_height/2;
            break;
        case '1':               // bottom (default)
        default:
            break;
        }

        int xp = x;
        int yp = y;


        if(fabs(vp->rotation()) > 0.01){
            float c = cosf(-vp->rotation() );
            float s = sinf(-vp->rotation() );
            float x = xadjust;
            float y = yadjust;
            xadjust =  x*c - y*s;
            yadjust =  x*s + y*c;

        }

        xp+= xadjust;
        yp+= yadjust;

        pRectDrawn->setX( xp );
        pRectDrawn->setY( yp );
        pRectDrawn->setWidth( w );
        pRectDrawn->setHeight( h );

        if( bCheckOverlap ) {
            if(fabs( vp->rotation() ) > .01){
                rotate(pRectDrawn, *vp );
            }
            if( CheckTextRectList( *pRectDrawn, ptext ) ) bdraw = false;
        }

        if( bdraw ) {
            QColor wcolor = FontMgr::Get().GetFontColor("ChartTexts");

            // If the user has not changed the color from BLACK, then use the color specified in the S52 LUP
            if( wcolor == Qt::black )
                glColor3ub( ptext->pcol->R, ptext->pcol->G, ptext->pcol->B );
            else
                glColor3ub( wcolor.red(), wcolor.green(), wcolor.blue() );

            glEnable( GL_BLEND );
            glEnable( GL_TEXTURE_2D );
            glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

            glPushMatrix();
            glTranslatef(xp, yp, 0);

            /* undo previous rotation to make text level */
            glRotatef(vp->rotation()*180/PI, 0, 0, -1);

            f_cache->RenderString(ptext->frmtd);
            glPopMatrix();

            glDisable( GL_TEXTURE_2D );
            glDisable( GL_BLEND );
        }
    }
    return bdraw;
    
    
#ifdef FIXIT
#undef FIXIT
#define DrawText DrawTextA
#endif
    
}



//    Return true if test_rect overlaps any rect in the current text rectangle list, except itself
bool s52plib::CheckTextRectList( const QRect &test_rect, S52_TextC *ptext )
{
    //    Iterate over the current object list, looking at rText
    for(int i=0; i<m_textObjList.size(); i++)
    {
        S52_TextC *data = m_textObjList[i];
        if( data->rText.intersects(test_rect ) ) {
            if( data != ptext )
                return true;
        }
    }
    return false;
}

bool s52plib::TextRenderCheck( ObjRazRules *rzRules )
{
    if( !m_bShowS57Text ) return false;

    if(rzRules->obj->bIsAton ){

        if( !strncmp( rzRules->obj->FeatureName, "LIGHTS", 6 ) ) {
            if( !m_bShowLdisText )
                return false;
        }
        else{
            if( !m_bShowAtonText )
                return false;
        }
    }



    // Declutter LIGHTS descriptions
    if( ( rzRules->obj->bIsAton ) && ( !strncmp( rzRules->obj->FeatureName, "LIGHTS", 6 ) ) ){
        if( lastLightLat == rzRules->obj->m_lat && lastLightLon == rzRules->obj->m_lon ){
            return false;       // only render text for the first object at this lat/lon
        }
        else{
            lastLightLat = rzRules->obj->m_lat;
            lastLightLon = rzRules->obj->m_lon;
        }
    }
    
    //    An optimization for CM93 charts.
    //    Don't show the text associated with some objects, since CM93 database includes _texto objects aplenty
    if( ( (int)rzRules->obj->auxParm3 == (int)CHART_TYPE_CM93 )
            || ( (int)rzRules->obj->auxParm3 == (int)CHART_TYPE_CM93COMP ) ) {
        if( !strncmp( rzRules->obj->FeatureName, "BUAARE", 6 ) )
            return false;
        else if( !strncmp( rzRules->obj->FeatureName, "SEAARE", 6 ) )
            return false;
        else if( !strncmp( rzRules->obj->FeatureName, "LNDRGN", 6 ) )
            return false;
        else if( !strncmp( rzRules->obj->FeatureName, "LNDARE", 6 ) )
            return false;
    }

    return true;
}

int s52plib::RenderT_All( ObjRazRules *rzRules, Rules *rules, ViewPort *vp, bool bTX )
{
    if( !TextRenderCheck( rzRules ) ) return 0;

    S52_TextC *text = NULL;
    bool b_free_text = false;

    //  The first Ftext object is cached in the S57Obj.
    //  If not present, create it on demand
    if( !rzRules->obj->bFText_Added ) {
        if( bTX ) text = S52_PL_parseTX( rzRules, rules, NULL );
        else
            text = S52_PL_parseTE( rzRules, rules, NULL );

        if( text ) {
            rzRules->obj->bFText_Added = true;
            rzRules->obj->FText = text;
            rzRules->obj->FText->rul_seq_creator = rules->n_sequence;
        }
    }

    //    S57Obj already contains a cached text object
    //    If it was created by this Rule earlier, then render it
    //    Otherwise, create a new text object, render it, and delete when done
    //    This will be slower, obviously, but happens infrequently enough?
    else {
        if( rules->n_sequence == rzRules->obj->FText->rul_seq_creator ) text = rzRules->obj->FText;
        else {
            if( bTX ) text = S52_PL_parseTX( rzRules, rules, NULL );
            else
                text = S52_PL_parseTE( rzRules, rules, NULL );

            b_free_text = true;
        }

    }

    if( text ) {
        if( m_bShowS57ImportantTextOnly && ( text->dis >= 20 ) ) {
            if( b_free_text ) delete text;
            return 0;
        }

        //    Establish a font
        if( !text->pFont ) {
            
            // Process the font specifications from the LUP symbolizatio rule
            int spec_weight = text->weight - 0x30;
            QFont::Weight fontweight;
            if( spec_weight < 5 )
                fontweight = QFont::Weight::Light;
            else{
                if( spec_weight == 5 )
                    fontweight = QFont::Weight::Normal;
                else
                    fontweight = QFont::Weight::Bold;
            }

            QFont *specFont = new QFont("Microsoft YaHei",text->bsize, fontweight );
            specFont->setStyle(  QFont::Style::StyleNormal);
            
            //Get the width of a single average character in the spec font
            QFontMetrics mecs(*specFont);
            QSize tsz(mecs.width("X"), mecs.height());
            text->avgCharWidth = tsz.width();
            
            //    If we have loaded a legacy S52 compliant PLIB,
            //    then we should use the formal font selection as required by S52 specifications.
            //    Otherwise, we have our own plan...
            
            if( useLegacyRaster )
            {
                text->pFont = specFont;
            } else {
                int spec_weight = text->weight - 0x30;
                QFont::Weight fontweight;
                if( spec_weight < 5 )
                {
                    fontweight = QFont::Weight::Light;
                } else
                {
                    if( spec_weight == 5 )
                    {
                        fontweight = QFont::Weight::Normal;
                    } else
                    {
                        fontweight = QFont::Weight::Bold;
                    }
                }

                QFont sys_font = qApp->font();
                int default_size = sys_font.pointSize();

#ifdef __WXOSX__
                default_size += 1;     // default to 1pt larger than system UI font
#else
                default_size += 2;     // default to 2pt larger than system UI font
#endif
                QFont templateFont = FontMgr::Get().getSacledFontDefaultSize("ChartTexts", default_size );
                
                // NOAA ENC fles requests font size up to 20 points, which looks very
                // disproportioned. Let's scale those sizes down to more reasonable values.
                int fontSize = text->bsize;

                if( fontSize > 18 ) fontSize -= 8;
                else
                    if( fontSize > 13 ) fontSize -= 3;

                // Now factor in the users selected font size.
                fontSize += templateFont.pointSize() - 10;
                
                // In no case should font size be less than 10, since it becomes unreadable
                fontSize = qMax(10, fontSize);

                text->pFont = new QFont(FontMgr::Get().FindOrCreateFont( fontSize, qApp->font().family(), templateFont.style(), fontweight, false));
            }
        }

        //  Render text at declared x/y of object
        zchxPoint r;
        GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

        QRect rect;
        bool bwas_drawn = RenderText(text, r.x, r.y, &rect, rzRules->obj, m_bDeClutterText, vp );

        //  If this is an un-cached text render, it probably means that a single object has two or more
        //  text renders in its rule set.  RDOCAL is one example.  There are others
        //  We need to cache only the first text structure, but should update the render rectangle
        //  to reflect all texts rendered for this object,  in order to process the declutter logic.
        bool b_dupok = false;
        if( b_free_text ) {
            delete text;

            if(!bwas_drawn){
                return 1;
            }
            else{                               // object was drawn
                text = rzRules->obj->FText;
                
                QRect r0 = text->rText;
                r0 = r0.united(rect);
                text->rText = r0;

                b_dupok = true;                 // OK to add a duplicate text structure to the declutter list, just for this case.
            }
        }
        else
            text->rText = rect;
        
        
        //      If this text was actually drawn, add a pointer to its rect to the de-clutter list if it doesn't already exist
        if( m_bDeClutterText ) {
            if( bwas_drawn ) {
                bool b_found = false;
                for(int i=0; i<m_textObjList.size(); i++)
                {
                    S52_TextC* oc = m_textObjList[i];
                    if( oc == text ) {
                        if(!b_dupok)
                            b_found = true;
                        break;
                    }
                }
                if( !b_found )
                    m_textObjList.append( text );
            }
        }

        //  Update the object Bounding box
        //  so that subsequent drawing operations will redraw the item fully
        //  and so that cursor hit testing includes both the text and the object

        //            if ( rzRules->obj->Primitive_type == GEO_POINT )
        {
            double latmin, lonmin, latmax, lonmax, extent = 0;

            GetPixPointSingleNoRotate( rect.x(), rect.y() + rect.height(), &latmin, &lonmin, vp );
            GetPixPointSingleNoRotate( rect.x() + rect.width(), rect.y(), &latmax, &lonmax, vp );
            LLBBox bbtext;
            bbtext.Set( latmin, lonmin, latmax, lonmax );

            rzRules->obj->BBObj.Expand( bbtext );
        }
    }

    return 1;
}

// Text
int s52plib::RenderTX( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    return RenderT_All( rzRules, rules, vp, true );
}

// Text formatted
int s52plib::RenderTE( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    return RenderT_All( rzRules, rules, vp, false );
}

bool s52plib::RenderHPGL( ObjRazRules *rzRules, Rule *prule, zchxPoint &r, ViewPort *vp, float rot_angle )
{
    float fsf = 100 / canvas_pix_per_mm;


    float xscale = 1.0;
    
    if( (!strncmp(rzRules->obj->FeatureName, "TSSLPT", 6))
            || (!strncmp(rzRules->obj->FeatureName, "DWRTPT", 6))
            || (!strncmp(rzRules->obj->FeatureName, "TWRTPT", 6))
            || (!strncmp(rzRules->obj->FeatureName, "RCTLPT", 6))
            ){
        // assume the symbol length
        float sym_length = 30;
        float scaled_length = sym_length / vp->viewScalePPM();
        
        double fac1 = scaled_length / fsf;
        
        
        float target_length = 1852;
        
        xscale = target_length / scaled_length;
        xscale = qMin(xscale, 1.0f);
        xscale = qMax(0.4f, xscale);
        
        //printf("scaled length: %g   xscale: %g\n", scaled_length, xscale);
        
        
        fsf *= xscale;
    }
    
    xscale *= g_ChartScaleFactorExp;
    
    //  Special case for GEO_AREA objects with centred symbols
    if( rzRules->obj->Primitive_type == GEO_AREA ) {
        zchxPoint r;
        GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );
        
        double latdraw, londraw;                // position of the drawn symbol with pivot applied
        GetPixPointSingleNoRotate( r.x + ((prule->pos.symb.pivot_x.SYCL - prule->pos.symb.bnbox_x.SBXC) / fsf),
                                   r.y + ((prule->pos.symb.pivot_y.SYRW - prule->pos.symb.bnbox_y.SBXR) / fsf),
                                   &latdraw, &londraw, vp );
        
        if( !rzRules->obj->BBObj.Contains( latdraw, londraw ) ) // Symbol reference point is outside base area object
            return 1;
    }
    
    double render_angle = rot_angle;
    
    //  Very special case for ATON flare lights at 135 degrees, the standard render angle.
    //  We don't want them to rotate with the viewport.
    if(rzRules->obj->bIsAton && (!strncmp(rzRules->obj->FeatureName, "LIGHTS", 6))  && (fabs(rot_angle - 135.0) < 1.) ){
        render_angle -= vp->rotation() * 180./PI;
        
        //  And, due to popular request, we make the flare lights a little bit smaller than S52 specifications
        xscale = xscale * 6. / 7.;
    }
    
    int width = prule->pos.symb.bnbox_x.SBXC + prule->pos.symb.bnbox_w.SYHL;
    width *= 4; // Grow the drawing bitmap to allow for rotation of symbols with highly offset pivot points
    width = (int) ( width / fsf );

    int height = prule->pos.symb.bnbox_y.SBXR + prule->pos.symb.bnbox_h.SYVL;
    height *= 4;
    height = (int) ( height / fsf );

    int origin_x = prule->pos.symb.bnbox_x.SBXC;
    int origin_y = prule->pos.symb.bnbox_y.SBXR;
    zchxPoint origin(origin_x, origin_y);
    
    int pivot_x = prule->pos.symb.pivot_x.SYCL;
    int pivot_y = prule->pos.symb.pivot_y.SYRW;
    zchxPoint pivot( pivot_x, pivot_y );
    
    char *str = prule->vector.LVCT;
    char *col = prule->colRef.LCRF;
    zchxPoint r0( (int) ( pivot_x / fsf ), (int) ( pivot_y / fsf ) );
    HPGL->SetTargetOpenGl();
    HPGL->Render( str, col, r, pivot, origin, xscale, render_angle, true );

    //  Update the object Bounding box
    //  so that subsequent drawing operations will redraw the item fully

    int r_width = prule->pos.symb.bnbox_w.SYHL;
    r_width = (int) ( r_width / fsf );
    int r_height = prule->pos.symb.bnbox_h.SYVL;
    r_height = (int) ( r_height / fsf );
    int maxDim = qMax(r_height, r_width);

    double latmin, lonmin, latmax, lonmax;
    GetPixPointSingleNoRotate( r.x - maxDim, r.y + maxDim, &latmin, &lonmin, vp );
    GetPixPointSingleNoRotate( r.x + maxDim, r.y - maxDim, &latmax,  &lonmax, vp );
    LLBBox symbox;
    symbox.Set( latmin, lonmin, latmax, lonmax );

    rzRules->obj->BBObj.Expand( symbox );
    return true;
}

//-----------------------------------------------------------------------------------------
//      Instantiate a Symbol or Pattern stored as XBM ascii in a rule
//      Producing a wxImage
//-----------------------------------------------------------------------------------------
QImage s52plib::RuleXBMToImage( Rule *prule )
{
    //      Decode the color definitions
    wxArrayPtrVoid *pColorArray = new wxArrayPtrVoid;

    int i = 0;
    char *cstr = prule->colRef.SCRF;

    char colname[6];
    int nl = strlen( cstr );

    while( i < nl ) {
        i++;

        strncpy( colname, &cstr[i], 5 );
        colname[5] = 0;
        S52color *pColor = getColor( colname );

        pColorArray->append( (void *) pColor );

        i += 5;
    }

    //      Get geometry
    int width = prule->pos.line.bnbox_w.SYHL;
    int height = prule->pos.line.bnbox_h.SYVL;

    QString gstr( *prule->bitmap.SBTM ); // the bit array

    QImage Image( width, height, QImage::Format_RGB32 );

    for( int iy = 0; iy < height; iy++ ) {
        QString thisrow = gstr.mid(iy * width, width ); // extract a row

        for( int ix = 0; ix < width; ix++ ) {
            int cref = (int) ( thisrow[ix].unicode() - 'A' ); // make an index
            S52color *now = &m_unused_color;
            if( cref >= 0 ) {
                now = (S52color *) ( pColorArray->at( cref ) );
            }
            //RGB转pixel
            int color = (now->R << 16) + (now->G << 8) + now->B;
            Image.setPixel(ix, iy, color);
        }
    }

    pColorArray->clear();
    delete pColorArray;
    return Image;
}

//
//      Render Raster Symbol
//      Symbol is instantiated as a bitmap the first time it is needed
//      and re-built on color scheme change
//
bool s52plib::RenderRasterSymbol( ObjRazRules *rzRules, Rule *prule, zchxPoint &r, ViewPort *vp,
                                  float rot_angle )
{
    double scale_factor = 1.0;

    scale_factor *=  g_ChartScaleFactorExp;
    scale_factor *= g_scaminScale;
    
    if(g_oz_vector_scale && vp->quilt()){
        double sfactor = vp->refScale()/vp->chartScale();
        scale_factor = fmax((sfactor - g_overzoom_emphasis_base)  / 4., scale_factor);
        scale_factor = fmin(scale_factor, 20);
    }

    // a few special cases here
    if( !strncmp(rzRules->obj->FeatureName, "notmrk", 6 )
            || !strncmp(rzRules->obj->FeatureName, "NOTMRK", 6)
            || !strncmp(prule->name.SYNM, "ADDMRK", 6)
            )
    {
        // get the symbol size
        QRect trect;
        ChartSymbols::GetGLTextureRect( trect, prule->name.SYNM );
        
        int scale_dim = qMax(trect.width(), trect.height());
        
        double scaled_size = scale_dim / vp->viewScalePPM();
        
        double target_size = 100;               // roughly, meters maximum scaled size for these inland signs
        
        double xscale = target_size / scaled_size;
        xscale = qMin(xscale, 1.0);
        xscale = qMax(.2, xscale);
        
        scale_factor *= xscale;
    }
    
    int pivot_x = prule->pos.line.pivot_x.SYCL;
    int pivot_y = prule->pos.line.pivot_y.SYRW;

    pivot_x *= scale_factor;
    pivot_y *= scale_factor;
    
    // For opengl, hopefully the symbols are loaded in a texture
    unsigned int texture = 0;
    QRect texrect;
    texture = ChartSymbols::GetGLTextureRect(texrect, prule->name.SYNM);
    if(texture) {
        prule->parm2 = texrect.width() * scale_factor;
        prule->parm3 = texrect.height() * scale_factor;
    }
    
    if(!texture ) {

        //    Check to see if any cached data is valid
        bool b_dump_cache = false;
        if( prule->pixelPtr ) {
            if( prule->parm0 != ID_RGBA ) b_dump_cache = true;
        }

        // If the requested scaled symbol size is not the same as is currently cached,
        // we have to dump the cache
        QRect trect;
        ChartSymbols::GetGLTextureRect( trect, prule->name.SYNM );
        if(prule->parm2 != trect.width() * scale_factor)  b_dump_cache = true;
        
        wxBitmap *pbm = NULL;
        QImage Image;

        //Instantiate the symbol if necessary
        if( ( prule->pixelPtr == NULL ) || ( prule->parm1 != m_colortable_index ) || b_dump_cache ) {
            Image = useLegacyRaster ? RuleXBMToImage( prule ) : ChartSymbols::GetImage( prule->name.SYNM );

            // delete any old private data
            ClearRulesCache( prule );

            // always display something, TMARDEF1 as width of 2
            int w0 = qMax(1, int(Image.width() * scale_factor));
            int h0 = qMax(1, int(Image.height() * scale_factor));
            Image.scaled(w0 , h0 , Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            
            int w = Image.width();
            int h = Image.height();
            //    Get the glRGBA format data from the wxImage
            unsigned char *d = Image.bits();
            unsigned char *a = 0;
            if(Image.hasAlphaChannel()) a = Image.alphaChannel().bits();

            //                Image.SetMaskColour( m_unused_QColor.red(), m_unused_QColor.green(),
            //                        m_unused_QColor.blue() );
            unsigned char mr, mg, mb;
            mr = m_unused_color.R;
            mg = m_unused_color.G;
            mb = m_unused_color.B;
            //                if( !a && !Image.GetOrFindMaskColour( &mr, &mg, &mb ) )
            //                    printf( "trying to use mask to draw a bitmap without alpha or mask\n" );

            unsigned char *e = (unsigned char *) malloc( w * h * 4 );
            // XXX FIXME a or e ?
            if( d /*&& a*/){
                for( int y = 0; y < h; y++ ) {
                    for( int x = 0; x < w; x++ ) {
                        unsigned char r, g, b;
                        int off = ( y * w + x );
//                        r = d[off * 3 + 0];
//                        g = d[off * 3 + 1];
//                        b = d[off * 3 + 2];
                        QColor color = Image.pixelColor(x, y);
                        r = color.red();
                        g = color.green();
                        b = color.blue();
                        int alpha = color.alpha();

                        e[off * 4 + 0] = r;
                        e[off * 4 + 1] = g;
                        e[off * 4 + 2] = b;

                        e[off * 4 + 3] = alpha;
//                                a ? a[off] : ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                    }
                }
            }
#if 0
        QImage temp(e, w, h, QImage::Format_RGBA8888);
        static int index = 1;
        bool sts = temp.save(QString("%1.png").arg(index++), "PNG");
        qDebug()<<"save temp image:"<<sts;

#endif

            //      Save the bitmap ptr and aux parms in the rule
            prule->pixelPtr = e;
            prule->parm0 = ID_RGBA;
            prule->parm1 = m_colortable_index;
            prule->parm2 = w;
            prule->parm3 = h;
        }               // instantiation
    }

    //        Get the bounding box for the to-be-drawn symbol
    int b_width, b_height;
    b_width = prule->parm2;
    b_height = prule->parm3;

    LLBBox symbox;
    double latmin, lonmin, latmax, lonmax;

    if(fabs( vp->rotation() ) > .01)          // opengl
    {
        float cx = vp->pixWidth()/2.;
        float cy = vp->pixHeight()/2.;
        float c = cosf(vp->rotation() );
        float s = sinf(vp->rotation() );
        float x = r.x - pivot_x -cx;
        float y = r.y - pivot_y + b_height -cy;
        GetPixPointSingle( x*c - y*s +cx, x*s + y*c +cy, &latmin, &lonmin, vp );

        x = r.x - pivot_x + b_width -cx;
        y = r.y - pivot_y -cy;
        GetPixPointSingle( x*c - y*s +cx, x*s + y*c +cy, &latmax, &lonmax, vp );
    }
    symbox.Set( latmin, lonmin, latmax, lonmax );

    //  Special case for GEO_AREA objects with centred symbols
    if( rzRules->obj->Primitive_type == GEO_AREA ) {
        if( !rzRules->obj->BBObj.IntersectIn( symbox ) ) // Symbol is wholly outside base object
            return true;
    }

    //      Now render the symbol
    glEnable( GL_BLEND );

    if(texture) {
        extern GLenum       g_texture_rectangle_format;

        glEnable(g_texture_rectangle_format);
        glBindTexture(g_texture_rectangle_format, texture);

        int w = texrect.width(), h = texrect.height();

        float tx1 = texrect.x(), ty1 = texrect.y();
        float tx2 = tx1 + w, ty2 = ty1 + h;

        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
        if(g_texture_rectangle_format == GL_TEXTURE_2D) {
            QSize size = ChartSymbols::GLTextureSize();
            tx1 /= size.width(), tx2 /= size.width();
            ty1 /= size.height(), ty2 /= size.height();
        }

        if(fabs( vp->rotation() ) > .01){
            glPushMatrix();

            glTranslatef(r.x, r.y, 0);
            glRotatef(vp->rotation() * 180/PI, 0, 0, -1);
            glTranslatef(-pivot_x, -pivot_y, 0);
            glScalef(scale_factor, scale_factor, 1);

            glBegin(GL_QUADS);
            glTexCoord2f(tx1, ty1);    glVertex2i( 0, 0);
            glTexCoord2f(tx2, ty1);    glVertex2i( w, 0);
            glTexCoord2f(tx2, ty2);    glVertex2i( w, h);
            glTexCoord2f(tx1, ty2);    glVertex2i( 0, h);
            glEnd();

            glPopMatrix();
        }
        else {

            if(1/*scale_factor > 1.0*/){
                glPushMatrix();

                glTranslatef(r.x, r.y, 0);
                glTranslatef(-pivot_x, -pivot_y, 0);
                glScalef(scale_factor, scale_factor, 1);

                glBegin(GL_QUADS);
                glTexCoord2f(tx1, ty1);    glVertex2i( 0, 0);
                glTexCoord2f(tx2, ty1);    glVertex2i( w, 0);
                glTexCoord2f(tx2, ty2);    glVertex2i( w, h);
                glTexCoord2f(tx1, ty2);    glVertex2i( 0, h);
                glEnd();

                glPopMatrix();
            }
            else {
                float ddx = pivot_x;
                float ddy = pivot_y;

                glBegin(GL_QUADS);
                glTexCoord2f(tx1, ty1);    glVertex2i(  r.x - ddx, r.y - ddy );
                glTexCoord2f(tx2, ty1);    glVertex2i(  r.x - ddx + w, r.y - ddy );
                glTexCoord2f(tx2, ty2);    glVertex2i(  r.x - ddx + w, r.y - ddy + h );
                glTexCoord2f(tx1, ty2);    glVertex2i(  r.x - ddx, r.y - ddy + h);
                glEnd();
            }
        }

        glDisable(g_texture_rectangle_format);
    } else { /* this is only for legacy mode, or systems without NPOT textures */
        if(prule->pixelPtr != NULL)
        {
            float cr = cosf( vp->rotation() );
            float sr = sinf( vp->rotation() );
            float ddx = pivot_x * cr + pivot_y * sr;
            float ddy = pivot_y * cr - pivot_x * sr;

            glColor4f( 1, 1, 1, 1 );

            //  Since draw pixels is so slow, lets not draw anything we don't have to
            QRect sym_rect(r.x - ddx, r.y - ddy, b_width, b_height);
            if(vp->rvRect().intersects(sym_rect) ) {

                glPushAttrib( GL_SCISSOR_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                glDisable( GL_SCISSOR_TEST );
                glDisable( GL_STENCIL_TEST );
                glDisable( GL_DEPTH_TEST );

                glRasterPos2f( r.x - ddx, r.y - ddy );
                glPixelZoom( 1, -1 );
                glDrawPixels( b_width, b_height, GL_RGBA, GL_UNSIGNED_BYTE, prule->pixelPtr );
                glPixelZoom( 1, 1 );

                glPopAttrib();
            }
        }
    }

    glDisable( GL_BLEND );

    //  Update the object Bounding box
    //  so that subsequent drawing operations will redraw the item fully
    //  We expand the object's BBox to account for objects rendered by multiple symbols, such as SOUNGD.
    //  so that expansions are cumulative.
    if( rzRules->obj->Primitive_type == GEO_POINT )
        rzRules->obj->BBObj.Expand( symbox );

    //  Dump the cache for next time
    if(g_oz_vector_scale && (scale_factor > 1.0))
        ClearRulesCache( prule );
    
    return true;
}

// SYmbol
int s52plib::RenderSY( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    float angle = 0;
    double orient;

    if( rules->razRule != NULL ) {
        if( rules->INSTstr[8] == ',' ) // supplementary parameter assumed to be angle, seen in LIGHTSXX
        {
            char sangle[10];
            int cp = 0;
            while( rules->INSTstr[cp + 9] && ( rules->INSTstr[cp + 9] != ')' ) ) {
                sangle[cp] = rules->INSTstr[cp + 9];
                cp++;
            }
            sangle[cp] = 0;
            int angle_i = atoi( sangle );
            angle = angle_i;
        }

        if( GetDoubleAttr( rzRules->obj, "ORIENT", orient ) ) // overriding any LIGHTSXX angle, probably TSSLPT
        {
            angle = orient;
            if( strncmp( rzRules->obj->FeatureName, "LIGHTS", 6 ) == 0 ) {
                angle += 180;
                if( angle > 360 ) angle -= 360;
            }
        }

        //  Render symbol at object's x/y
        zchxPoint r, r1;
        GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

        //  Render a raster or vector symbol, as specified by LUP rules
        if( rules->razRule->definition.SYDF == 'V' ){
            RenderHPGL( rzRules, rules->razRule, r, vp, angle );
        }
        else{
            if( rules->razRule->definition.SYDF == 'R' )
                RenderRasterSymbol( rzRules, rules->razRule, r, vp, angle );
        }
    }

    return 0;

}

// Line Simple Style, OpenGL
int s52plib::RenderGLLS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    // for now don't use vbo model in non-mercator
    if(vp->projectType() != PROJECTION_MERCATOR)
        return RenderLS(rzRules, rules, vp);

    if( !m_benableGLLS )                        // root chart cannot support VBO model, for whatever reason
        return RenderLS(rzRules, rules, vp);

    double scale_factor = vp->refScale()/vp->chartScale();
    if(scale_factor > 10.0)
        return RenderLS(rzRules, rules, vp);

    if( !rzRules->obj->m_chart_context->chart )
        return RenderLS(rzRules, rules, vp);    // this is where S63 PlugIn gets caught
    
    if(( vp->getBBox().GetMaxLon() >= 180.) || (vp->getBBox().GetMinLon() <= -180.))
        return RenderLS(rzRules, rules, vp);    // cm03 has trouble at IDL
    
    bool b_useVBO = false;
    float *vertex_buffer = 0;
    
    if(rzRules->obj->auxParm2 > 0)             // Has VBO been defined and uploaded?
        b_useVBO = true;

    if( !b_useVBO ){
#if 0        
        if( rzRules->obj->m_chart_context->chart ){
            vertex_buffer = rzRules->obj->m_chart_context->chart->GetLineVertexBuffer();
        }
        else {
            vertex_buffer = rzRules->obj->m_chart_context->vertex_buffer;
        }
        
        
        if(!vertex_buffer)
            return RenderLS(rzRules, rules, vp);    // this is where cm93 gets caught
#else
        vertex_buffer = rzRules->obj->m_chart_context->vertex_buffer;

#endif            
    }

    
#ifdef ocpnUSE_GL

    char *str = (char*) rules->INSTstr;
    LLBBox BBView = vp->getBBox();

    //  Allow a little slop in calculating whether a segment
    //  is within the requested Viewport
    double margin = BBView.GetLonRange() * .05;
    BBView.EnLarge( margin );

    //  Try to determine if the feature needs to be drawn in the most efficient way
    //  We need to look at priority and visibility of each segment
    int bdraw = 0;
    
    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    
    line_segment_element *ls_list = rzRules->obj->m_ls_list;
    
    S52color *c = getColor( str + 7 ); // Colour
    int w = atoi( str + 5 ); // Width
    
    glColor3ub( c->R, c->G, c->B );
    
    //    Set drawing width
    float lineWidth = w;
    
    if( w > 1 ) {
        GLint parms[2];
        glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
        if( w > parms[1] )
            lineWidth = fmax(g_GLMinCartographicLineWidth, parms[1]);
        else
            lineWidth = fmax(g_GLMinCartographicLineWidth, w);
    } else
        lineWidth = fmax(g_GLMinCartographicLineWidth, 1);

    // Manage super high density displays
    float target_w_mm = 0.5 * w;
    if(GetPPMM() > 7){               // arbitrary
        target_w_mm = ((float)w) / 6.0;  // Target width in mm
        //  The value "w" comes from S52 library CNSY procedures, in "nominal" pixels
        // the value "6" comes from semi-standard LCD display densities
        // or something like 0.18 mm pitch, or 6 pix per mm.
        lineWidth =  qMax(g_GLMinCartographicLineWidth, target_w_mm * GetPPMM());
    }

    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );
    glLineWidth(lineWidth);
    if(lineWidth > 4.0 && m_GLLineSmoothing){
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
    }
    
#ifndef ocpnUSE_GLES // linestipple is emulated poorly
    if( !strncmp( str, "DASH", 4 ) ) {
        glLineStipple( 1, 0x3F3F );
        glEnable( GL_LINE_STIPPLE );
    }
    else if( !strncmp( str, "DOTT", 4 ) ) {
        glLineStipple( 1, 0x3333 );
        glEnable( GL_LINE_STIPPLE );
    }
    else
        glDisable( GL_LINE_STIPPLE );
#endif    

    glPushMatrix();
    
    // Set up the OpenGL transform matrix for this object
    //  Transform from Simple Mercator (relative to chart reference point) to screen coordinates.
    
    //  First, the VP transform
    glTranslatef( vp->pixWidth() / 2, vp->pixHeight()/2, 0 );
    glScalef( vp->viewScalePPM(), -vp->viewScalePPM(), 0 );
    glTranslatef( -rzRules->sm_transform_parms->easting_vp_center, -rzRules->sm_transform_parms->northing_vp_center, 0 );
    
    //  Next, the per-object transform
    if( rzRules->obj->m_chart_context->chart ){
        glTranslatef( rzRules->obj->x_origin, rzRules->obj->y_origin, 0);
        glScalef( rzRules->obj->x_rate, rzRules->obj->y_rate, 0 );
    }

    glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array

    //   Has line segment PBO been allocated for this chart?
    if(b_useVBO){
        (s_glBindBuffer)(GL_ARRAY_BUFFER, rzRules->obj->auxParm2);
    }

    


    // from above ls_list is the first drawable segment
    while( ls_list){
        
        if( ls_list->priority == priority_current  )
        {
            size_t seg_vbo_offset = 0;
            size_t point_count = 0;
            
            //  Check visibility of the segment
            bool b_drawit = false;
            if( (ls_list->ls_type == TYPE_EE) || (ls_list->ls_type == TYPE_EE_REV) ){
                //                 if((BBView.GetMinLat() < ls_list->pedge->edgeBBox.GetMaxLat() && BBView.GetMaxLat() > ls_list->pedge->edgeBBox.GetMinLat()) &&
                //                     ((BBView.GetMinLon() <= ls_list->pedge->edgeBBox.GetMaxLon() && BBView.GetMaxLon() >= ls_list->pedge->edgeBBox.GetMinLon()) ||
                //                     (BBView.GetMaxLon() >=  180 && BBView.GetMaxLon() - 360 > ls_list->pedge->edgeBBox.GetMinLon()) ||
                //                     (BBView.GetMinLon() <= -180 && BBView.GetMinLon() + 360 < ls_list->pedge->edgeBBox.GetMaxLon())))
                {
                    // render the segment
                    b_drawit = true;
                    seg_vbo_offset = ls_list->pedge->vbo_offset;
                    point_count = ls_list->pedge->nCount;
                }

            }
            else{
                //                 if((BBView.GetMinLat() < ls_list->pcs->cs_lat_avg && BBView.GetMaxLat() > ls_list->pcs->cs_lat_avg) &&
                //                     ((BBView.GetMinLon() <= ls_list->pcs->cs_lon_avg && BBView.GetMaxLon() >= ls_list->pcs->cs_lon_avg) ||
                //                     (BBView.GetMaxLon() >=  180 && BBView.GetMaxLon() - 360 > ls_list->pcs->cs_lon_avg) ||
                //                     (BBView.GetMinLon() <= -180 && BBView.GetMinLon() + 360 < ls_list->pcs->cs_lon_avg)))
                {
                    // render the segment
                    b_drawit = true;
                    seg_vbo_offset = ls_list->pcs->vbo_offset;
                    point_count = 2;
                }
            }
            
            
            
            
            
            if( b_drawit) {
                // render the segment
                
                if(b_useVBO){
                    glVertexPointer(2, GL_FLOAT, 2 * sizeof(float), (GLvoid *)(seg_vbo_offset));
                    glDrawArrays(GL_LINE_STRIP, 0, point_count);
                }
                else{
                    glVertexPointer(2, GL_FLOAT, 2 * sizeof(float), (unsigned char *)vertex_buffer + seg_vbo_offset);
                    glDrawArrays(GL_LINE_STRIP, 0, point_count);
                }
            }
        }
        ls_list = ls_list->next;
    }
    
    if(b_useVBO)
        (s_glBindBuffer)(GL_ARRAY_BUFFER_ARB, 0);
    
    glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array

    glPopMatrix();

    glDisable( GL_LINE_STIPPLE );
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );
    
#endif                  // OpenGL
    
    return 1;
}


// Line Simple Style
int s52plib::RenderLS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    // catch legacy PlugIns (e.g.s63_pi)
    if( rzRules->obj->m_n_lsindex  && !rzRules->obj->m_ls_list)
        return RenderLSLegacy(rzRules, rules, vp);

    // catch improperly coded edge arrays, usually seen on cm93
    if( !rzRules->obj->m_n_lsindex  && !rzRules->obj->m_ls_list)
        return 0;
    
    S52color *c;
    int w;

    char *str = (char*) rules->INSTstr;
    c = getColor( str + 7 ); // Colour
    QColor color( c->R, c->G, c->B );
    w = atoi( str + 5 ); // Width

    double scale_factor = vp->refScale()/vp->chartScale();
    double scaled_line_width = fmax((scale_factor - g_overzoom_emphasis_base), 1);
    bool b_wide_line = g_oz_vector_scale && vp->quilt() && (scale_factor > g_overzoom_emphasis_base);
    
    QPen wide_pen(Qt::black);
    QVector<qreal> dashw(2);
    dashw[0] = 3;
    dashw[1] = 1;
    
    if( b_wide_line)
    {
        int w = qMax((int)scaled_line_width, 2);            // looks better
        w = qMin(w, 50);                               // upper bound
        wide_pen.setWidth( w );
        wide_pen.setColor(color);
        
        if( !strncmp( str, "DOTT", 4 ) ) {
            dashw[0] = 1;
            wide_pen.setStyle(Qt::CustomDashLine);
            wide_pen.setDashPattern(dashw);
        }
        else if( !strncmp( str, "DASH", 4 ) ){
            wide_pen.setStyle(Qt::CustomDashLine);
            wide_pen.setDashPattern(dashw);
        }
    }

    QPen thispen(color, w, Qt::SolidLine);
    QVector<qreal> dash1(2);
    glColor3ub( c->R, c->G, c->B );

    //    Set drawing width
    if( w > 1 ) {
        GLint parms[2];
        glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
        if( w > parms[1] )
            glLineWidth( fmax(g_GLMinCartographicLineWidth, parms[1]) );
        else
            glLineWidth( fmax(g_GLMinCartographicLineWidth, w) );
    } else
    {
        glLineWidth( fmax(g_GLMinCartographicLineWidth, 1) );
    }

#ifndef ocpnUSE_GLES // linestipple is emulated poorly
    if( !strncmp( str, "DASH", 4 ) ) {
        glLineStipple( 1, 0x3F3F );
        glEnable( GL_LINE_STIPPLE );
    }
    else if( !strncmp( str, "DOTT", 4 ) ) {
        glLineStipple( 1, 0x3333 );
        glEnable( GL_LINE_STIPPLE );
    }
    else {
        glDisable( GL_LINE_STIPPLE );
    }
#endif
    if(w >= 2 && m_GLLineSmoothing){
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
    }


    //    Get a true pixel clipping/bounding box from the vp
    zchxPoint pbb = vp->GetPixFromLL( vp->lat(), vp->lon() );
    int xmin_ = pbb.x - (vp->rvRect().width() / 2) - (4 * scaled_line_width);
    int xmax_ = xmin_ + vp->rvRect().width() + (8 * scaled_line_width);
    int ymin_ = pbb.y - (vp->rvRect().height() / 2) - (4 * scaled_line_width) ;
    int ymax_ = ymin_ + vp->rvRect().height() + (8 * scaled_line_width);

    int x0, y0, x1, y1;

    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;

    if( rzRules->obj->m_ls_list )
    {
        float *ppt;

        unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
        line_segment_element *ls = rzRules->obj->m_ls_list;
        if(!b_wide_line)
            glBegin( GL_LINES );
        while(ls){
            if( ls->priority == priority_current  ) {
                int nPoints;
                // fetch the first point
                if( (ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV) ){
                    ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
                    nPoints = ls->pedge->nCount;
                }
                else{
                    ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
                    nPoints = 2;
                }

                zchxPoint l;
                GetPointPixSingle( rzRules, ppt[1], ppt[0], &l, vp );
                ppt += 2;

                for(int ip=0 ; ip < nPoints - 1 ; ip++){
                    zchxPoint r;
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &r, vp );
                    //        Draw the edge as point-to-point
                    x0 = l.x, y0 = l.y;
                    x1 = r.x, y1 = r.y;

                    // Do not draw null segments
                    if( ( x0 != x1 ) || ( y0 != y1 ) ){
                        // simplified faster test, let opengl do the rest
                        if((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                                (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {
                            if(!b_wide_line) {
                                glVertex2i( x0, y0 );
                                glVertex2i( x1, y1 );
                            } else
                                PLIBDrawGLThickLine( x0, y0, x1, y1, wide_pen, true );
                        }

                    }

                    l = r;
                    ppt += 2;
                }
            }

            ls = ls->next;
        }
        if(!b_wide_line)
            glEnd();
    }
    glDisable( GL_LINE_STIPPLE );
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );
    return 1;
}

// Line Simple Style
int s52plib::RenderLSLegacy( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    if( !rzRules->obj->m_chart_context->chart )
        return RenderLSPlugIn( rzRules, rules, vp );

    // Must be cm93
    S52color *c;
    int w;

    char *str = (char*) rules->INSTstr;
    c = getColor( str + 7 ); // Colour
    QColor color( c->R, c->G, c->B );
    w = atoi( str + 5 ); // Width

    double scale_factor = vp->refScale()/vp->chartScale();
    double scaled_line_width = fmax((scale_factor - g_overzoom_emphasis_base), 1);
    bool b_wide_line = g_oz_vector_scale && vp->quilt() && (scale_factor > g_overzoom_emphasis_base);
    
    QPen wide_pen(Qt::black);
    QVector<qreal> dashw(2);
    dashw[0] = 3;
    dashw[1] = 1;
    
    if( b_wide_line)
    {
        int w = fmax(scaled_line_width, 2);            // looks better
        w = qMin(w, 50);                               // upper bound
        wide_pen.setWidth( w );
        wide_pen.setColor(color);
        
        if( !strncmp( str, "DOTT", 4 ) ) {
            dashw[0] = 1;
            wide_pen.setStyle(Qt::CustomDashLine);
            wide_pen.setDashPattern(dashw);
        }
        else if( !strncmp( str, "DASH", 4 ) ){
            wide_pen.setStyle(Qt::CustomDashLine);
            wide_pen.setDashPattern(dashw);
        }
    }
    glColor3ub( c->R, c->G, c->B );

    //    Set drawing width
    if( w > 1 ) {
        GLint parms[2];
        glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
        if( w > parms[1] )
            glLineWidth( fmax(g_GLMinCartographicLineWidth, parms[1]) );
        else
            glLineWidth( fmax(g_GLMinCartographicLineWidth, w) );
    } else
        glLineWidth( fmax(g_GLMinCartographicLineWidth, 1) );

#ifndef ocpnUSE_GLES // linestipple is emulated poorly
    if( !strncmp( str, "DASH", 4 ) ) {
        glLineStipple( 1, 0x3F3F );
        glEnable( GL_LINE_STIPPLE );
    }
    else if( !strncmp( str, "DOTT", 4 ) ) {
        glLineStipple( 1, 0x3333 );
        glEnable( GL_LINE_STIPPLE );
    }
    else
        glDisable( GL_LINE_STIPPLE );
#endif
    if(w >= 2 && m_GLLineSmoothing){
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
    }

    //    Get a true pixel clipping/bounding box from the vp
    zchxPoint pbb = vp->GetPixFromLL( vp->lat(), vp->lon() );
    int xmin_ = pbb.x - (vp->rvRect().width() / 2) - (4 * scaled_line_width);
    int xmax_ = xmin_ + vp->rvRect().width() + (8 * scaled_line_width);
    int ymin_ = pbb.y - (vp->rvRect().height() / 2) - (4 * scaled_line_width) ;
    int ymax_ = ymin_ + vp->rvRect().height() + (8 * scaled_line_width);

    int x0, y0, x1, y1;

    
    if( rzRules->obj->m_n_lsindex ) {
        VE_Hash *ve_hash;
        VC_Hash *vc_hash;
        ve_hash = (VE_Hash *)rzRules->obj->m_chart_context->m_pve_hash;             // This is cm93
        vc_hash = (VC_Hash *)rzRules->obj->m_chart_context->m_pvc_hash;


        //  Get the current display priority
        //  Default comes from the LUP, unless overridden
        int priority_current = rzRules->LUP->DPRI - '0';
        if(rzRules->obj->m_DPRI >= 0)
            priority_current = rzRules->obj->m_DPRI;
        
        int *index_run;
        float *ppt;

        VC_Element *pnode;
        if(!b_wide_line)
            glBegin( GL_LINES );
        for( int iseg = 0; iseg < rzRules->obj->m_n_lsindex; iseg++ ) {
            int seg_index = iseg * 3;
            index_run = &rzRules->obj->m_lsindex_array[seg_index];

            //  Get first connected node
            unsigned int inode = *index_run++;

            //  Get the edge
            unsigned int enode = *index_run++;
            VE_Element *pedge = 0;
            if(enode)
                pedge = (*ve_hash)[enode];

            //  Get last connected node
            unsigned int jnode = *index_run++;

            int nls;
            if(pedge) {
                //  Here we decide to draw or not based on the highest priority seen for this segment
                //  That is, if this segment is going to be drawn at a higher priority later, then "continue", and don't draw it here.

                // This logic is not perfectly right for one case:
                // If the segment has only two end connected nodes, and no intermediate edge,
                // then we have no good way to evaluate the priority.
                // This is due to the fact that priority is only precalculated for edge segments, not connector nodes.
                // Only thing to do is take the conservative approach and draw the segment, in this case.
                if( pedge->nCount && pedge->max_priority != priority_current )
                    continue;
                nls = pedge->nCount + 1;
            } else
                nls = 1;

            zchxPoint l;
            bool lastvalid = false;
            for( int ipc = 0; ipc < nls + 1; ipc++ ) {
                ppt = 0;
                if( ipc == 0 ) {
                    if( inode ) {
                        pnode = (*vc_hash)[inode];
                        if( pnode )
                            ppt = pnode->pPoint;
                    }
                } else if(ipc == nls) {
                    if( ( jnode ) ) {
                        pnode = (*vc_hash)[jnode];
                        if( pnode )
                            ppt = pnode->pPoint;
                    }
                } else if(pedge)
                    ppt = pedge->pPoints + 2*(ipc-1);
                
                if(ppt) {
                    zchxPoint r;
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &r, vp );

                    if(r.x != INVALID_COORD) {
                        if(lastvalid) {
                            //        Draw the edge as point-to-point
                            x0 = l.x, y0 = l.y;
                            x1 = r.x, y1 = r.y;

                            // Do not draw null segments
                            if( ( x0 == x1 ) && ( y0 == y1 ) ) continue;
                            // simplified faster test, let opengl do the rest
                            if((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                                    (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {
                                if(!b_wide_line) {
                                    glVertex2i( x0, y0 );
                                    glVertex2i( x1, y1 );
                                } else
                                    PLIBDrawGLThickLine( x0, y0, x1, y1, wide_pen, true );
                            }
                        }

                        l = r;
                        lastvalid = true;
                    } else
                        lastvalid = false;
                } else
                    lastvalid = false;
            }
        }
        if(!b_wide_line)
            glEnd();
    }
    glDisable( GL_LINE_STIPPLE );
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );
    return 1;
}

class PI_connector_segment              // This was extracted verbatim from S63_pi private definition
{
public:
    void *start;
    void *end;
    SegmentType type;
    int vbo_offset;
    int max_priority;
};

int s52plib::RenderLSPlugIn( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    S52color *c;
    int w;
    
    char *str = (char*) rules->INSTstr;
    c = getColor( str + 7 ); // Colour
    QColor color( c->R, c->G, c->B );
    w = atoi( str + 5 ); // Width
    
    double scale_factor = vp->refScale()/vp->chartScale();
    double scaled_line_width = fmax((scale_factor - g_overzoom_emphasis_base), 1);
    bool b_wide_line = g_oz_vector_scale && vp->quilt() && (scale_factor > g_overzoom_emphasis_base);
    
    QPen wide_pen(Qt::black);
    QVector<qreal> dashw(2);
    dashw[0] = 3;
    dashw[1] = 1;
    
    if( b_wide_line)
    {
        int w = fmax(scaled_line_width, 2);            // looks better
        w = qMin(w, 50);                               // upper bound
        wide_pen.setWidth( w );
        wide_pen.setColor(color);
        
        if( !strncmp( str, "DOTT", 4 ) ) {
            dashw[0] = 1;
            wide_pen.setStyle(Qt::CustomDashLine);
            wide_pen.setDashPattern(dashw);
        }
        else if( !strncmp( str, "DASH", 4 ) ){
            wide_pen.setStyle(Qt::CustomDashLine);
            wide_pen.setDashPattern(dashw);
        }
    }

    glColor3ub( c->R, c->G, c->B );

    //    Set drawing width
    if( w > 1 ) {
        GLint parms[2];
        glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, &parms[0] );
        if( w > parms[1] )
            glLineWidth( fmax(g_GLMinCartographicLineWidth, parms[1]) );
        else
            glLineWidth( fmax(g_GLMinCartographicLineWidth, w) );
    } else
        glLineWidth( fmax(g_GLMinCartographicLineWidth, 1) );

#ifndef ocpnUSE_GLES // linestipple is emulated poorly
    if( !strncmp( str, "DASH", 4 ) ) {
        glLineStipple( 1, 0x3F3F );
        glEnable( GL_LINE_STIPPLE );
    }
    else if( !strncmp( str, "DOTT", 4 ) ) {
        glLineStipple( 1, 0x3333 );
        glEnable( GL_LINE_STIPPLE );
    }
    else
        glDisable( GL_LINE_STIPPLE );
#endif
    
    
    //    Get a true pixel clipping/bounding box from the vp
    zchxPoint pbb = vp->GetPixFromLL( vp->lat(), vp->lon() );
    int xmin_ = pbb.x - (vp->rvRect().width() / 2) - (4 * scaled_line_width);
    int xmax_ = xmin_ + vp->rvRect().width() + (8 * scaled_line_width);
    int ymin_ = pbb.y - (vp->rvRect().height() / 2) - (4 * scaled_line_width) ;
    int ymax_ = ymin_ + vp->rvRect().height() + (8 * scaled_line_width);
    
    int x0, y0, x1, y1;
    
    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    
    if( rzRules->obj->m_ls_list_legacy )
    {
        float *ppt;

        VE_Element *pedge;


        PI_connector_segment *pcs;

        unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
        PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;
        if(!b_wide_line)
            glBegin( GL_LINES );
        while(ls){
            if( ls->priority == priority_current  ) {

                int nPoints;
                // fetch the first point
                if(ls->type == TYPE_EE){
                    pedge = (VE_Element *)ls->private0;
                    ppt = (float *)(vbo_point + pedge->vbo_offset);
                    nPoints = pedge->nCount;
                }
                else{
                    pcs = (PI_connector_segment *)ls->private0;
                    ppt = (float *)(vbo_point + pcs->vbo_offset);
                    nPoints = 2;
                }

                zchxPoint l;
                GetPointPixSingle( rzRules, ppt[1], ppt[0], &l, vp );
                ppt += 2;

                for(int ip=0 ; ip < nPoints - 1 ; ip++){
                    zchxPoint r;
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &r, vp );
                    //        Draw the edge as point-to-point
                    x0 = l.x, y0 = l.y;
                    x1 = r.x, y1 = r.y;

                    // Do not draw null segments
                    if( ( x0 != x1 ) || ( y0 != y1 ) ){

                        // simplified faster test, let opengl do the rest
                        if((x0 > xmin_ || x1 > xmin_) && (x0 < xmax_ || x1 < xmax_) &&
                                (y0 > ymin_ || y1 > ymin_) && (y0 < ymax_ || y1 < ymax_)) {
                            if(!b_wide_line) {
                                glVertex2i( x0, y0 );
                                glVertex2i( x1, y1 );
                            } else
                                PLIBDrawGLThickLine( x0, y0, x1, y1, wide_pen, true );
                        }

                    }

                    l = r;
                    ppt += 2;
                }
            }

            ls = ls->next;
        }
        if(!b_wide_line)
            glEnd();
    }

    glDisable( GL_LINE_STIPPLE );
    return 1;
}

// Line Complex
int s52plib::RenderLC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    //     if(rzRules->obj->Index != 7574)
    //         return 0;
    
    // catch cm93 and legacy PlugIns (e.g.s63_pi)
    if( rzRules->obj->m_n_lsindex  && !rzRules->obj->m_ls_list)
        return RenderLCLegacy(rzRules, rules, vp);
    
    zchxPoint r;
    
    
    int isym_len = rules->razRule->pos.line.bnbox_w.SYHL + (rules->razRule->pos.line.bnbox_x.LBXC - rules->razRule->pos.line.pivot_x.LICL);
    float sym_len = isym_len * canvas_pix_per_mm / 100;
    float sym_factor = 1.0; ///1.50;                        // gives nicer effect
    
    //      Create a color for drawing adjustments outside of HPGL renderer
    char *tcolptr = rules->razRule->colRef.LCRF;
    S52color *c = getColor( tcolptr + 1 ); // +1 skips "n" in HPGL SPn format
    int w = 1; // arbitrary width
    QColor color( c->R, c->G, c->B );
    double LOD = 2.0 / vp->viewScalePPM();              // empirical value, by experiment
    LOD = 0; //qMin(LOD, 10.0);
    
    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    
    if( rzRules->obj->m_n_lsindex ) {
        
        
        // Calculate the size of a work buffer
        int max_points = 0;
        if( rzRules->obj->m_n_edge_max_points > 0 )
            max_points = rzRules->obj->m_n_edge_max_points;
        else{
            line_segment_element *lsa = rzRules->obj->m_ls_list;
            
            while(lsa){
                
                if( (lsa->ls_type == TYPE_EE) || (lsa->ls_type == TYPE_EE_REV) )
                    max_points += lsa->pedge->nCount;
                else
                    max_points += 2;
                
                lsa = lsa->next;
            }
        }
        
        
        //  Allocate some storage for converted points
        zchxPoint *ptp = (zchxPoint *) malloc( ( max_points ) * sizeof(zchxPoint) );
        double *pdp = (double *)malloc( 2 * ( max_points ) * sizeof(double) );
        
        unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer; //chart->GetLineVertexBuffer();
        line_segment_element *ls = rzRules->obj->m_ls_list;
        
        unsigned int index = 0;
        unsigned int idouble = 0;
        int nls = 0;
        zchxPoint lp;
        float *ppt;
        
        int direction = 1;
        int ndraw = 0;
        while(ls){
            if( ls->priority == priority_current  ) {

                
                //transcribe the segment in the proper order into the output buffer
                int nPoints;
                int idir = 1;
                // fetch the first point
                if( (ls->ls_type == TYPE_EE) || (ls->ls_type == TYPE_EE_REV) ){
                    ppt = (float *)(vbo_point + ls->pedge->vbo_offset);
                    nPoints = ls->pedge->nCount;
                    if(ls->ls_type == TYPE_EE_REV)
                        idir = -1;
                    
                }
                else{
                    ppt = (float *)(vbo_point + ls->pcs->vbo_offset);
                    nPoints = 2;
                }
                
                
                int vbo_index = 0;
                int vbo_inc = 2;
                if(idir == -1){
                    vbo_index = (nPoints-1) * 2;
                    vbo_inc = -2;
                }
                for(int ip=0 ; ip < nPoints ; ip++){
                    zchxPoint r;
                    GetPointPixSingle( rzRules, ppt[vbo_index + 1], ppt[vbo_index], &r, vp );
                    if( (r.x != lp.x) || (r.y != lp.y) ){
                        ptp[index++] = r;
                        pdp[idouble++] = ppt[vbo_index];
                        pdp[idouble++] = ppt[vbo_index + 1];
                        
                        nls++;
                    }
                    else{               // sKipping point
                    }
                    
                    lp = r;
                    vbo_index += vbo_inc;
                }
                
            }  // priority
            
            // inspect the next segment to see if it can be connected, or if the chain breaks
            int idir = 1;
            if(ls->next){
                
                int nPoints_next;
                line_segment_element *lsn = ls->next;
                // fetch the first point
                if( (lsn->ls_type == TYPE_EE) || (lsn->ls_type == TYPE_EE_REV) ){
                    ppt = (float *)(vbo_point + lsn->pedge->vbo_offset);
                    nPoints_next = lsn->pedge->nCount;
                    if(lsn->ls_type == TYPE_EE_REV)
                        idir = -1;
                    
                }
                else{
                    ppt = (float *)(vbo_point + lsn->pcs->vbo_offset);
                    nPoints_next = 2;
                }
                
                zchxPoint ptest;
                if(idir == 1)
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &ptest, vp );

                else{
                    // fetch the last point
                    int index_last_next = (nPoints_next-1) * 2;
                    GetPointPixSingle( rzRules, ppt[index_last_next +1], ppt[index_last_next], &ptest, vp );
                }
                
                // try to match the correct point in this segment with the last point in the previous segment

                if(lp != ptest)         // not connectable?
                {
                    
                    if(nls){
                        zchxPointF *pReduced = 0;
                        int nPointReduced = reduceLOD( LOD, nls, pdp, &pReduced);

                        zchxPoint *ptestp = (zchxPoint *) malloc( ( max_points ) * sizeof(zchxPoint) );
                        GetPointPixArray( rzRules, pReduced, ptestp, nPointReduced, vp );
                        free(pReduced);

                        draw_lc_poly(color, w, ptestp, nPointReduced, sym_len, sym_factor, rules->razRule, vp );
                        free(ptestp);

                        ndraw++;
                    }
                    
                    nls = 0;
                    index = 0;
                    idouble = 0;
                    lp = zchxPoint(0,0);
                    direction = 1;
                }
                
                
            }
            else{
                // no more segments, so render what is available
                if(nls){
                    zchxPointF *pReduced = 0;
                    int nPointReduced = reduceLOD( LOD, nls, pdp, &pReduced);
                    
                    zchxPoint *ptestp = (zchxPoint *) malloc( ( max_points ) * sizeof(zchxPoint) );
                    GetPointPixArray( rzRules, pReduced, ptestp, nPointReduced, vp );
                    free(pReduced);
                    
                    draw_lc_poly(color, w, ptestp, nPointReduced, sym_len, sym_factor, rules->razRule, vp );
                    free( ptestp );
                }
            }
            
            ls = ls->next;
        }
        
        
        free( ptp );
        free(pdp);
    }
    
    return 1;
}


int s52plib::reduceLOD( double LOD_meters, int nPoints, double *source, zchxPointF **dest)
{
    //      Reduce the LOD of this linestring
    std::vector<int> index_keep;
    if(nPoints > 5 && (LOD_meters > .01)){
        index_keep.push_back(0);
        index_keep.push_back(nPoints-1);
        index_keep.push_back(nPoints-2);
        
        DouglasPeucker(source, 1, nPoints-2, LOD_meters, &index_keep);
        
    }
    else {
        index_keep.resize(nPoints);
        // Consider using std::iota here when there is C++11 support.
        for(int i = 0 ; i < nPoints ; i++)
            index_keep[i] = i;
    }
    
    zchxPointF *pReduced = (zchxPointF *)malloc( ( index_keep.size() ) * sizeof(zchxPointF) );
    *dest = pReduced;
    
    double *ppr = source;
    int ir = 0;
    for(int ip = 0 ; ip < nPoints ; ip++)
    {
        double x = *ppr++;
        double y = *ppr++;
        
        for(unsigned int j=0 ; j < index_keep.size() ; j++){
            if(index_keep[j] == ip){
                pReduced[ir++] = zchxPointF(x, y);
                break;
            }
        }
    }
    
    return index_keep.size();
}


// Line Complex
int s52plib::RenderLCLegacy( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    if( !rzRules->obj->m_chart_context->chart )
        return RenderLCPlugIn( rzRules, rules, vp );
    
    //  Must be cm93

    zchxPoint r;

    int isym_len = rules->razRule->pos.line.bnbox_w.SYHL;
    float sym_len = isym_len * canvas_pix_per_mm / 100;
    float sym_factor = 1.0; ///1.50;                        // gives nicer effect

    //      Create a color for drawing adjustments outside of HPGL renderer
    char *tcolptr = rules->razRule->colRef.LCRF;
    S52color *c = getColor( tcolptr + 1 ); // +1 skips "n" in HPGL SPn format
    int w = 1; // arbitrary width
    QColor color( c->R, c->G, c->B );

    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;

    if( rzRules->obj->m_n_lsindex ) {
        
        VE_Hash *ve_hash = (VE_Hash *)rzRules->obj->m_chart_context->m_pve_hash;
        VC_Hash *vc_hash = (VC_Hash *)rzRules->obj->m_chart_context->m_pvc_hash;
        
        unsigned int nls_max;
        if( rzRules->obj->m_n_edge_max_points > 0 ) // size has been precalculated on SENC load
            nls_max = rzRules->obj->m_n_edge_max_points;
        else {
            //  Calculate max malloc size required
            nls_max = 0;
            int *index_run_x = rzRules->obj->m_lsindex_array;
            for( int imseg = 0; imseg < rzRules->obj->m_n_lsindex; imseg++ ) {
                index_run_x++; //Skip cNode
                //  Get the edge
                unsigned int enode = *index_run_x;
                if( enode ){
                    VE_Element *pedge = (*ve_hash)[enode];
                    if(pedge){
                        if( pedge->nCount > nls_max )
                            nls_max = pedge->nCount;
                    }
                }
                index_run_x += 2;
            }
            rzRules->obj->m_n_edge_max_points = nls_max; // Got it, cache for next time
        }

        //  Allocate some storage for converted points
        zchxPoint *ptp = (zchxPoint *) malloc( ( nls_max + 2 ) * sizeof(zchxPoint) ); // + 2 allows for end nodes

        int *index_run;
        float *ppt;
        double easting, northing;
        zchxPoint pra( 0, 0 );
        VC_Element *pnode;

        for( int iseg = 0; iseg < rzRules->obj->m_n_lsindex; iseg++ ) {
            int seg_index = iseg * 3;
            index_run = &rzRules->obj->m_lsindex_array[seg_index];

            //  Get first connected node
            unsigned int inode = *index_run++;
            if( inode ) {
                pnode = (*vc_hash)[inode];
                if( pnode ) {
                    ppt = pnode->pPoint;
                    easting = *ppt++;
                    northing = *ppt;
                    GetPointPixSingle( rzRules, (float) northing, (float) easting, &pra, vp );
                }
                ptp[0] = pra; // insert beginning node
            }

            //  Get the edge
            unsigned int enode = *index_run++;
            VE_Element *pedge = 0;
            if(enode)
                pedge = (*ve_hash)[enode];

            int nls = 0;
            if(pedge){
                //  Here we decide to draw or not based on the highest priority seen for this segment
                //  That is, if this segment is going to be drawn at a higher priority later, then don't draw it here.

                // This logic is not perfectly right for one case:
                // If the segment has only two end connected nodes, and no intermediate edge,
                // then we have no good way to evaluate the priority.
                // This is due to the fact that priority is only precalculated for edge segments, not connector nodes.
                // Only thing to do is take the conservative approach and draw the segment, in this case.
                if( pedge->nCount ){
                    if( pedge->max_priority != priority_current ) continue;
                }

                //                if( pedge->max_priority != priority_current ) continue;

                nls = pedge->nCount;

                ppt = pedge->pPoints;
                for( int ip = 0; ip < nls; ip++ ) {
                    easting = *ppt++;
                    northing = *ppt++;
                    GetPointPixSingle( rzRules, (float) northing, (float) easting, &ptp[ip + 1], vp );
                }
            }

            //  Get last connected node
            unsigned int jnode = *index_run++;
            if( jnode ) {
                pnode = (*vc_hash)[jnode];
                if( pnode ) {
                    ppt = pnode->pPoint;
                    easting = *ppt++;
                    northing = *ppt;
                    GetPointPixSingle( rzRules, (float) northing, (float) easting, &pra, vp );
                }
                ptp[nls + 1] = pra; // insert ending node
            }

            if( ( inode ) && ( jnode ) )
                draw_lc_poly(color, w, ptp, nls + 2, sym_len, sym_factor, rules->razRule, vp );
            else if(nls)
                draw_lc_poly(color, w, &ptp[1], nls, sym_len, sym_factor, rules->razRule, vp );

        }
        free( ptp );
    }

    else
        if( rzRules->obj->pPolyTessGeo ) {
            if( !rzRules->obj->pPolyTessGeo->IsOk() ){ // perform deferred tesselation
                rzRules->obj->pPolyTessGeo->BuildDeferredTess();
            }

            PolyTriGroup *pptg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
            float *ppolygeo = pptg->pgroup_geom;
            if(ppolygeo){
                int ctr_offset = 0;
                for( int ic = 0; ic < pptg->nContours; ic++ ) {

                    int npt = pptg->pn_vertex[ic];
                    zchxPoint *ptp = (zchxPoint *) malloc( ( npt + 1 ) * sizeof(zchxPoint) );
                    zchxPoint *pr = ptp;
                    for( int ip = 0; ip < npt; ip++ ) {
                        float plon = ppolygeo[( 2 * ip ) + ctr_offset];
                        float plat = ppolygeo[( 2 * ip ) + ctr_offset + 1];

                        GetPointPixSingle( rzRules, plat, plon, pr, vp );
                        pr++;
                    }
                    float plon = ppolygeo[ctr_offset]; // close the polyline
                    float plat = ppolygeo[ctr_offset + 1];
                    GetPointPixSingle( rzRules, plat, plon, pr, vp );

                    draw_lc_poly(color, w, ptp, npt + 1, sym_len, sym_factor, rules->razRule, vp );

                    free( ptp );

                    ctr_offset += npt * 2;
                }
            }
        }

    return 1;
}

int s52plib::RenderLCPlugIn( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    zchxPoint r;
    
    int isym_len = rules->razRule->pos.line.bnbox_w.SYHL;
    float sym_len = isym_len * canvas_pix_per_mm / 100;
    float sym_factor = 1.0; ///1.50;                        // gives nicer effect
    
    //      Create a color for drawing adjustments outside of HPGL renderer
    char *tcolptr = rules->razRule->colRef.LCRF;
    S52color *c = getColor( tcolptr + 1 ); // +1 skips "n" in HPGL SPn format
    int w = 1; // arbitrary width
    QColor color( c->R, c->G, c->B );
    
    //  Get the current display priority
    //  Default comes from the LUP, unless overridden
    int priority_current = rzRules->LUP->DPRI - '0';
    if(rzRules->obj->m_DPRI >= 0)
        priority_current = rzRules->obj->m_DPRI;
    

    //  Calculate max malloc size required
    unsigned int nls_max = 0;

    if( rzRules->obj->m_ls_list_legacy )
    {
        float *ppt;
        
        VE_Element *pedge;
        PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;
        

        while(ls){
            uint nPoints;
            // fetch the first point
            if(ls->type == TYPE_EE){
                pedge = (VE_Element *)ls->private0;
                nPoints = pedge->nCount;
            }
            else{
                nPoints = 2;
            }

            nls_max = qMax(nls_max, nPoints);

            ls = ls->next;
        }

        //  Allocate some storage for converted points
        zchxPoint *ptp = (zchxPoint *) malloc( ( nls_max + 2 ) * sizeof(zchxPoint) ); // + 2 allows for end nodes

        PI_connector_segment *pcs;
        
        unsigned char *vbo_point = (unsigned char *)rzRules->obj->m_chart_context->vertex_buffer;
        ls = rzRules->obj->m_ls_list_legacy;
        
        while(ls){
            if( ls->priority == priority_current  ) {

                int nPoints;
                // fetch the first point
                if(ls->type == TYPE_EE){
                    pedge = (VE_Element *)ls->private0;
                    ppt = (float *)(vbo_point + pedge->vbo_offset);
                    nPoints = pedge->nCount;
                }
                else{
                    pcs = (PI_connector_segment *)ls->private0;
                    ppt = (float *)(vbo_point + pcs->vbo_offset);
                    nPoints = 2;
                }

                for(int ip=0 ; ip < nPoints ; ip++){
                    zchxPoint r;
                    GetPointPixSingle( rzRules, ppt[1], ppt[0], &ptp[ip], vp );

                    ppt += 2;
                }

                if(nPoints)
                    draw_lc_poly(color, w, &ptp[0], nPoints, sym_len, sym_factor, rules->razRule, vp );
            }

            ls = ls->next;
        }
        
        free(ptp);
    }
    
    return 1;
}

//      Render Line Complex Polyline

void s52plib::draw_lc_poly(QColor &color, int width, zchxPoint *ptp, int npt,
                           float sym_len, float sym_factor, Rule *draw_rule, ViewPort *vp )
{
    if(npt < 2)
        return;
    
    zchxPoint r;

    //  We calculate the winding direction of the poly
    //  in order to know which side to draw symbol on
    double dfSum = 0.0;
    
    for( int iseg = 0; iseg < npt - 1; iseg++ ) {
        dfSum += ptp[iseg].x * ptp[iseg+1].y - ptp[iseg].y * ptp[iseg+1].x;
    }
    dfSum += ptp[npt-1].x * ptp[0].y - ptp[npt-1].y * ptp[0].x;
    
    bool cw = dfSum < 0.;
    
    //    Get a true pixel clipping/bounding box from the vp
    zchxPoint pbb = vp->GetPixFromLL( vp->lat(), vp->lon() );
    int xmin_ = pbb.x - vp->rvRect().width() / 2;
    int xmax_ = xmin_ + vp->rvRect().width();
    int ymin_ = pbb.y - vp->rvRect().height() / 2;
    int ymax_ = ymin_ + vp->rvRect().height();

    int x0, y0, x1, y1;
    //    Set up the color
    glColor4ub( color.red(), color.green(), color.blue(), color.alpha() );

    // Adjust line width up a bit, to improve render quality for GL_BLEND/GL_LINE_SMOOTH
    float awidth = fmax(g_GLMinCartographicLineWidth, (float)(width * 0.7));
    awidth = fmax(awidth, 1.5);
    glLineWidth( awidth );

    int start_seg = 0;
    int end_seg = npt - 1;
    int inc = 1;

    if( cw ){
        start_seg = npt - 1;
        end_seg = 0;
        inc = -1;
    }

    float dx, dy, seg_len, theta;

    bool done = false;
    int iseg = start_seg;
    while( !done ){
        // Do not bother with segments that are invisible

        x0 = ptp[iseg].x;
        y0 = ptp[iseg].y;
        x1 = ptp[iseg + inc].x;
        y1 = ptp[iseg + inc].y;

        ClipResult res = cohen_sutherland_line_clip_i( &x0, &y0, &x1, &y1, xmin_, xmax_, ymin_,
                                                       ymax_ );

        if( res == Invisible )
            goto next_seg;

        dx = ptp[iseg + inc].x - ptp[iseg].x;
        dy = ptp[iseg + inc].y - ptp[iseg].y;
        seg_len = sqrt( dx * dx + dy * dy );

        if( seg_len >= 1.0 ) {
            if( seg_len <= sym_len * sym_factor ) {
                int xst1 = ptp[iseg].x;
                int yst1 = ptp[iseg].y;
                float xst2, yst2;

                if( seg_len >= sym_len ) {
                    xst2 = xst1 + ( sym_len * dx / seg_len );
                    yst2 = yst1 + ( sym_len * dy / seg_len );
                } else {
                    xst2 = ptp[iseg + inc].x;
                    yst2 = ptp[iseg + inc].y;
                }

                //      Enable anti-aliased lines, at best quality
#ifndef __OCPN__ANDROID__
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable (GL_BLEND);

                if( m_GLLineSmoothing )
                {
                    glEnable (GL_LINE_SMOOTH);
                    glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
                }
#endif
                {
                    glBegin( GL_LINES );
                    glVertex2i( xst1, yst1 );
                    glVertex2i( (int) floor( xst2 ), (int) floor( yst2 ) );
                    glEnd();
                }

                glDisable( GL_LINE_SMOOTH );
                glDisable( GL_BLEND );
            } else {
                float s = 0;
                float xs = ptp[iseg].x;
                float ys = ptp[iseg].y;

                while( s + ( sym_len * sym_factor ) < seg_len ) {
                    r.x = (int) xs;
                    r.y = (int) ys;
                    char *str = draw_rule->vector.LVCT;
                    char *col = draw_rule->colRef.LCRF;
                    zchxPoint pivot( draw_rule->pos.line.pivot_x.LICL,
                                     draw_rule->pos.line.pivot_y.LIRW );

                    HPGL->SetTargetOpenGl();
                    theta = atan2f( dy, dx );
                    HPGL->Render( str, col, r, pivot, pivot, 1.0, theta * 180. / PI, false );

                    xs += sym_len * dx / seg_len * sym_factor;
                    ys += sym_len * dy / seg_len * sym_factor;
                    s += sym_len * sym_factor;
                }
#ifndef __OCPN__ANDROID__
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable (GL_BLEND);

                if( m_GLLineSmoothing ) {
                    glEnable (GL_LINE_SMOOTH);
                    glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
                }

#endif
                {
                    glBegin( GL_LINES );
                    glVertex2i( xs, ys );
                    glVertex2i( ptp[iseg + inc].x, ptp[iseg + inc].y );
                    glEnd();
                }
                glDisable( GL_LINE_SMOOTH );
                glDisable( GL_BLEND );
            }
        }
next_seg:            
        iseg += inc;
        if(iseg == end_seg)
            done = true;
    } // while
}

// Multipoint Sounding
int s52plib::RenderMPS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    if( !m_bShowSoundg )
        return 0;

    if( m_bUseSCAMIN ) {
        if( vp->chartScale() > rzRules->obj->Scamin )
            return 0;
    }
    
    
    int npt = rzRules->obj->npt;

    // this should never happen
    // But it seems that some PlugIns clear the mps rules without resetting the CS state machine
    // So fix it
    if( rzRules->obj->bCS_Added  && !rzRules->mps)
        rzRules->obj->bCS_Added = false;

    //  Build the cached rules list if necessary
    if( !rzRules->obj->bCS_Added ) {

        ObjRazRules point_rzRules;
        point_rzRules = *rzRules; // take a copy of attributes, etc
        
        S57Obj point_obj;
        point_obj = *( rzRules->obj );
        point_obj.bIsClone = true;
        point_rzRules.obj = &point_obj;
        
        Rules *ru_cs = StringToRules("CS(SOUNDG03;");

        zchxPoint p;
        double *pd = rzRules->obj->geoPtz; // the SM points
        double *pdl = rzRules->obj->geoPtMulti; // and corresponding lat/lon
        

        mps_container *pmps = (mps_container *)calloc( sizeof(mps_container), 1);
        pmps->cs_rules = new ArrayOfRules;
        rzRules->mps = pmps;
        
        for( int ip = 0; ip < npt; ip++ ) {
            double east = *pd++;
            double nort = *pd++;
            double depth = *pd++;
            
            point_obj.x = east;
            point_obj.y = nort;
            point_obj.z = depth;
            
            double lon = *pdl++;
            double lat = *pdl++;
            point_obj.BBObj.Set( lat, lon, lat, lon );
            point_obj.BBObj.Invalidate();
            
            char *rule_str1 = RenderCS( &point_rzRules, ru_cs );
            QString cs_string = QString::fromUtf8(rule_str1 );
            free( rule_str1 );

            Rules *rule_chain = StringToRules( cs_string );
            
            rzRules->mps->cs_rules->append( rule_chain );
            
        }

        DestroyRulesChain( ru_cs );
        rzRules->obj->bCS_Added = 1; // mark the object
    }

    

    double *pdl = rzRules->obj->geoPtMulti; // and corresponding lat/lon

    //  We need a private unrotated copy of the Viewport
    ViewPort vp_local = *vp;
    vp_local.setRotation( 0. );

    //  We may be rendering the soundings symbols scaled up, so
    //  adjust the inclusion test bounding box
    
    double scale_factor = vp->refScale()/vp->chartScale();
    double box_mult = fmax((scale_factor - g_overzoom_emphasis_base), 1);
    int box_dim = 32 * box_mult;
    
    // We need a pixel bounding rectangle of the passed ViewPort.
    // Very important for partial screen renders, as with dc mode pans or OpenGL FBO operation.

    zchxPoint cr0 = vp_local.GetPixFromLL( vp_local.getBBox().GetMaxLat(), vp_local.getBBox().GetMinLon());
    zchxPoint cr1 = vp_local.GetPixFromLL( vp_local.getBBox().GetMinLat(), vp_local.getBBox().GetMaxLon());
    QRect clip_rect(cr0.toPoint(), cr1.toPoint());
    
    for( int ip = 0; ip < npt; ip++ ) {
        
        double lon = *pdl++;
        double lat = *pdl++;

        zchxPoint r = vp_local.GetPixFromLL( lat, lon );
        //      Use estimated symbol size
        QRect rr(r.x-(box_dim/2), r.y-(box_dim/2), box_dim, box_dim);
        
        //      After all the setup, the render inclusion test is trivial....
        if(!clip_rect.intersects(rr))
            continue;
        
        double angle = 0;
        
        Rules *rules =  rzRules->mps->cs_rules->at(ip);
        while( rules ){
            
            //  Render a raster or vector symbol, as specified by LUP rules
            if( rules->razRule->definition.SYDF == 'V' )
                RenderHPGL( rzRules, rules->razRule, r, vp, angle );
            
            else if( rules->razRule->definition.SYDF == 'R' )
                RenderRasterSymbol( rzRules, rules->razRule, r, vp, angle );
            
            rules = rules->next;
        }
    }
    
    return 1;
}

int s52plib::RenderCARC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    return RenderCARC_VBO(rzRules, rules, vp);
}

int s52plib::RenderCARC_VBO( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
    char *str = (char*) rules->INSTstr;
    //    extract the parameters from the string
    //    And creating a unique string hash as we go
    QString inst = QString::fromUtf8(str );
    QString carc_hash;

    QStringList tkz = inst.split(",;");

    //    outline color
    int i = 0;
    QString outline_color = tkz[i++];
    carc_hash += outline_color;
    carc_hash += ".";

    //    outline width
    QString slong = tkz[i++];
    long outline_width = slong.toLong();
    carc_hash += slong;
    carc_hash += ".";

    //    arc color
    QString arc_color = tkz[i++];
    carc_hash += arc_color;
    carc_hash += ".";

    //    arc width
    slong = tkz[i++];
    long arc_width = slong.toLong();
    carc_hash += slong;
    carc_hash += ".";

    //    sectr1
    slong = tkz[i++];
    double sectr1 = slong.toDouble();
    carc_hash += slong;
    carc_hash += ".";

    //    sectr2
    slong = tkz[i++];
    double sectr2 = slong.toDouble();
    carc_hash += slong;
    carc_hash += ".";

    //    arc radius
    slong = tkz[i++];
    long radius = slong.toLong();
    carc_hash += slong;
    carc_hash += ".";

    //    sector radius
    slong = tkz[i++];
    long sector_radius = slong.toLong();
    carc_hash += slong;
    carc_hash += ".";

    slong.sprintf("%d", m_colortable_index );
    carc_hash += slong;

    int width;
    int height;
    int rad;
    int bm_width;
    int bm_height;
    int bm_orgx;
    int bm_orgy;

    Rule *prule = rules->razRule;

    float scale_factor = 1.0;
    
    // The dimensions of the light are presented here as pixels on-screen.
    // We must scale the rendered size based on the device pixel density
    // Let us declare that the width of the arc should be no less than X mm
    float wx = 1.0;
    
    float pd_scale = 1.0;
    float nominal_arc_width_pix = qMax(1.0, floor(GetPPMM() * wx));             // { wx } mm nominal, but not less than 1 pixel
    pd_scale = nominal_arc_width_pix / arc_width;
    
    //scale_factor *= pd_scale;
    //qDebug() << GetPPMM() << arc_width << nominal_arc_width_pix << pd_scale;
    
    
    // Adjust size
    //  Some plain lights have no SCAMIN attribute.
    //  This causes display congestion at small viewing scales, since the objects are rendered at fixed pixel dimensions from the LUP rules.
    //  As a correction, the idea is to not allow the rendered symbol to be larger than "X" meters on the chart.
    //   and scale it down when rendered if necessary.

    float xscale = 1.0;
    if(rzRules->obj->Scamin > 10000000){                        // huge (unset) SCAMIN)
        float radius_meters_target = 200;

        float radius_meters = ( radius * canvas_pix_per_mm ) / vp->viewScalePPM();

        xscale = radius_meters_target / radius_meters;
        xscale = fmin(xscale, 1.0);
        xscale = fmax(.4, xscale);

        radius *= xscale;
        sector_radius *= xscale;
    }

    ///scale_factor *= xscale;
    
    carc_hash += ".";
    QString xs;
    xs.sprintf("%5g", xscale );
    carc_hash += xs;

    CARC_Buffer buffer;
    //    Is there not already an generated vbo the CARC_hashmap for this object?
    rad = (int) ( radius * canvas_pix_per_mm );
    if( m_CARC_hashmap.find( carc_hash ) == m_CARC_hashmap.end() ) {

        if( sectr1 > sectr2 ) sectr2 += 360;

        /* to ensure that the final segment lands exactly on sectr2 */

        //    Draw wide outline arc
        QColor colorb = getQColor( outline_color );
        buffer.color[0][0] = colorb.red();
        buffer.color[0][1] = colorb.green();
        buffer.color[0][2] = colorb.blue();
        buffer.color[0][3] = 150;
        buffer.line_width[0] = qMax(g_GLMinSymbolLineWidth, outline_width * scale_factor);

        int steps = ceil((sectr2 - sectr1) / 12) + 1; // max of 12 degree step
        float step = (sectr2 - sectr1) / (steps - 1);

        buffer.steps = steps;
        buffer.size = 2*(steps + 4);
        buffer.data = new float[buffer.size];

        int s = 0;
        for(int i = 0; i < steps; i++) {
            float a = (sectr1 + i * step) * M_PI / 180.0;
            buffer.data[s++] = rad * sinf( a );
            buffer.data[s++] = -rad * cosf( a );
        }

        //    Draw narrower color arc, overlaying the drawn outline.
        colorb = getQColor( arc_color );
        buffer.color[1][0] = colorb.red();
        buffer.color[1][1] = colorb.green();
        buffer.color[1][2] = colorb.blue();
        buffer.color[1][3] = 150;
        buffer.line_width[1] = qMax(double(g_GLMinSymbolLineWidth), (arc_width  * scale_factor) + .8);

        //    Draw the sector legs
        if( sector_radius > 0 ) {
            int leg_len = (int) ( sector_radius * canvas_pix_per_mm );

            //QColor c = GetGlobalColor( _T ( "CHBLK" ) );
            QColor c = GetGlobalColor( "CHBLK");

            buffer.color[2][0] = c.red();
            buffer.color[2][1] = c.green();
            buffer.color[2][2] = c.blue();
            buffer.color[2][3] = c.alpha();
            //buffer.line_width[2] = qMax(g_GLMinSymbolLineWidth, (float)0.5) * scale_factor;
            buffer.line_width[2] = qMax(1.0, floor(GetPPMM() * 0.2));             //0.4 mm nominal, but not less than 1 pixel

            float a = ( sectr1 - 90 ) * PI / 180.;
            buffer.data[s++] = 0;
            buffer.data[s++] = 0;
            buffer.data[s++] = leg_len * cosf( a );
            buffer.data[s++] = leg_len * sinf( a );

            a = ( sectr2 - 90 ) * PI / 180.;
            buffer.data[s++] = 0;
            buffer.data[s++] = 0;
            buffer.data[s++] = leg_len * cosf( a );
            buffer.data[s++] = leg_len * sinf( a );
        } else
            buffer.line_width[2] = 0;

        m_CARC_hashmap[carc_hash] = buffer;

    } else
        buffer = m_CARC_hashmap[carc_hash];

    int border_fluff = 4; // by how much should the blit bitmap be "fluffed"
    bm_width = rad * 2 + ( border_fluff * 2 );
    bm_height = rad * 2 + ( border_fluff * 2 );
    bm_orgx = -bm_width / 2;
    bm_orgy = -bm_height / 2;

    prule->parm2 = bm_orgx;
    prule->parm3 = bm_orgy;
    prule->parm5 = bm_width;
    prule->parm6 = bm_height;
    prule->parm7 = xscale;


    //  Render arcs at object's x/y
    zchxPoint r;
    GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

    //      Now render the symbol
    glPushMatrix();
    glTranslatef( r.x, r.y, 0 );

    //        glScalef(scale_factor, scale_factor, 1);
    glVertexPointer(2, GL_FLOAT, 2 * sizeof(float), buffer.data);

#ifndef __OCPN__ANDROID__
    glEnable( GL_BLEND );
    if( m_GLLineSmoothing )
        glEnable( GL_LINE_SMOOTH );
#endif        
    glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array

    glColor3ubv(buffer.color[0]);
    glLineWidth(buffer.line_width[0]);
    glDrawArrays(GL_LINE_STRIP, 0, buffer.steps);


    glColor3ubv(buffer.color[1]);
    glLineWidth(buffer.line_width[1]);
    glDrawArrays(GL_LINE_STRIP, 0, buffer.steps);

    //qDebug() << buffer.line_width[0] << buffer.line_width[1] << buffer.line_width[2];

    if(buffer.line_width[2]) {
#ifndef ocpnUSE_GLES // linestipple is emulated poorly
        glLineStipple( 1, 0x3F3F );
        glEnable( GL_LINE_STIPPLE );
#endif
        glColor3ubv(buffer.color[2]);
        glLineWidth(buffer.line_width[2]);
        glDrawArrays(GL_LINES, buffer.steps, 4);
#ifndef ocpnUSE_GLES
        glDisable( GL_LINE_STIPPLE );
#endif
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable( GL_LINE_SMOOTH );
    glDisable( GL_BLEND );

    // Debug the symbol bounding box.....
    /*
        {
            int x0 = rules->razRule->parm2;
            int y0 = rules->razRule->parm3;

            glLineWidth( 2 );
            glColor4f( 0,0,0,0 );
            
            glBegin( GL_LINE_STRIP );
            glVertex2i( x0, y0 );
            glVertex2i( x0 + b_width, y0 );
            glVertex2i( x0 + b_width, y0 + b_height );
            glVertex2i( x0, y0 + b_height);
            glVertex2i( x0, y0 );
            glEnd();
        }
        */

    //        glTranslatef( -r.x, -r.y, 0 );
    glPopMatrix();
    //  Update the object Bounding box,
    //  so that subsequent drawing operations will redraw the item fully

    double latmin, lonmin, latmax, lonmax;

    GetPixPointSingleNoRotate( r.x + prule->parm2,                r.y + prule->parm3 + prule->parm6, &latmin, &lonmin, vp );
    GetPixPointSingleNoRotate( r.x + prule->parm2 + prule->parm5, r.y + prule->parm3,                &latmax, &lonmax, vp );
    LLBBox symbox;
    symbox.Set( latmin, lonmin, latmax, lonmax );
    rzRules->obj->BBObj.Expand( symbox );

    return 1;
}

// Conditional Symbology
char *s52plib::RenderCS( ObjRazRules *rzRules, Rules *rules )
{
    void *ret;
    void* (*f)( void* );

    static int f05;

    if( rules->razRule == NULL ) {
        if( !f05 )
            //                  CPLError ( ( CPLErr ) 0, 0,"S52plib:_renderCS(): ERROR no conditional symbology for: %s\n", rules->INSTstr );
            f05++;
        return 0;
    }

    void *g = (void *) rules->razRule;

#ifdef FIX_FOR_MSVC  //__WXMSW__
    //#warning Fix this cast, somehow...
    //      dsr             sigh... can't get the cast right
    _asm
    {
        mov eax,[dword ptr g]
                mov [dword ptr f],eax
    }
    ret = f ( ( void * ) rzRules ); // call cond symb
#else

    f = (void * (*)( void * ) ) g;ret
            = f( (void *) rzRules );

#endif

    return (char *) ret;
}

int s52plib::RenderObjectToDC(ObjRazRules *rzRules, ViewPort *vp )
{
    return DoRenderObject(rzRules, vp );
}


int s52plib::RenderObjectToGL(QGLContext* glcc, ObjRazRules *rzRules, ViewPort *vp )
{
    m_glcc = glcc;
    return DoRenderObject(rzRules, vp );
}

int s52plib::RenderObjectToDCText(ObjRazRules *rzRules, ViewPort *vp )
{
    return DoRenderObjectTextOnly(rzRules, vp );
}

int s52plib::RenderObjectToGLText(QGLContext *glcc, ObjRazRules *rzRules, ViewPort *vp )
{
    m_glcc = glcc;
    return DoRenderObjectTextOnly(rzRules, vp );
}



int s52plib::DoRenderObject(ObjRazRules *rzRules, ViewPort *vp )
{
    //TODO  Debugging
    //      if(rzRules->obj->Index != 1103)
    //          return 0; //int yyp = 0;

    //        if(!strncmp(rzRules->obj->FeatureName, "berths", 6))
    //            int yyp = 0;

    if( !ObjectRenderCheckRules( rzRules, vp, true ) )
        return 0;

    Rules *rules = rzRules->LUP->ruleList;

    while( rules != NULL ) {
        switch( rules->ruleType ){
        case RUL_TXT_TX:
            RenderTX( rzRules, rules, vp );
            break; // TX
        case RUL_TXT_TE:
            RenderTE( rzRules, rules, vp );
            break; // TE
        case RUL_SYM_PT:
            RenderSY( rzRules, rules, vp );
            break; // SY
        case RUL_SIM_LN:
            RenderGLLS( rzRules, rules, vp );
            break; // LS
        case RUL_COM_LN:
            RenderLC( rzRules, rules, vp );
            break; // LC
        case RUL_MUL_SG:
            RenderMPS( rzRules, rules, vp );
            break; // MultiPoint Sounding
        case RUL_ARC_2C:
            RenderCARC( rzRules, rules, vp );
            break; // Circular Arc, 2 colors

        case RUL_CND_SY: {
            if( !rzRules->obj->bCS_Added ) {
                rzRules->obj->CSrules = NULL;
                GetAndAddCSRules( rzRules, rules );
                if(strncmp(rzRules->obj->FeatureName, "SOUNDG", 6))
                    rzRules->obj->bCS_Added = 1; // mark the object
            }

            Rules *rules_last = rules;
            rules = rzRules->obj->CSrules;

            while( NULL != rules ) {
                switch( rules->ruleType ){
                case RUL_TXT_TX:
                    RenderTX( rzRules, rules, vp );
                    break;
                case RUL_TXT_TE:
                    RenderTE( rzRules, rules, vp );
                    break;
                case RUL_SYM_PT:
                    RenderSY( rzRules, rules, vp );
                    break;
                case RUL_SIM_LN:
                    RenderGLLS( rzRules, rules, vp );
                    break; // LS
                case RUL_COM_LN:
                    RenderLC( rzRules, rules, vp );
                    break;
                case RUL_MUL_SG:
                    RenderMPS( rzRules, rules, vp );
                    break; // MultiPoint Sounding
                case RUL_ARC_2C:
                    RenderCARC( rzRules, rules, vp );
                    break; // Circular Arc, 2 colors
                case RUL_NONE:
                default:
                    break; // no rule type (init)
                }
                rules_last = rules;
                rules = rules->next;
            }

            rules = rules_last;
            break;
        }

        case RUL_NONE:
        default:
            break; // no rule type (init)
        } // switch

        rules = rules->next;
    }

    return 1;
}

int s52plib::DoRenderObjectTextOnly(ObjRazRules *rzRules, ViewPort *vp )
{
    //    if(strncmp(rzRules->obj->FeatureName, "RDOCAL", 6))
    //        return 0;

    //    if(rzRules->obj->Index == 2766)
    //        int yyp = 4;
    
    if( !ObjectRenderCheckRules( rzRules, vp, true ) )
        return 0;

    Rules *rules = rzRules->LUP->ruleList;
    
    while( rules != NULL ) {
        switch( rules->ruleType ){
        case RUL_TXT_TX:
            RenderTX( rzRules, rules, vp );
            break; // TX
        case RUL_TXT_TE:
            RenderTE( rzRules, rules, vp );
            break; // TE
        case RUL_CND_SY: {
            if( !rzRules->obj->bCS_Added ) {
                rzRules->obj->CSrules = NULL;
                GetAndAddCSRules( rzRules, rules );
                if(strncmp(rzRules->obj->FeatureName, "SOUNDG", 6))
                    rzRules->obj->bCS_Added = 1; // mark the object
            }

            Rules *rules_last = rules;
            rules = rzRules->obj->CSrules;

            while( NULL != rules ) {
                switch( rules->ruleType ){
                case RUL_TXT_TX:
                    RenderTX( rzRules, rules, vp );
                    break;
                case RUL_TXT_TE:
                    RenderTE( rzRules, rules, vp );
                    break;
                default:
                    break; // no rule type (init)
                }
                rules_last = rules;
                rules = rules->next;
            }

            rules = rules_last;
            break;
        }
            
        case RUL_NONE:
        default:
            break; // no rule type (init)
        } // switch
        
        rules = rules->next;
    }
    
    return 1;
}

bool s52plib::PreloadOBJLFromCSV(const QString &csv_file)
{
    QFile file( csv_file );
    if( !file.exists() ) return false;

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QString str = file.readLine();
    QChar quote[] = { '\"', 0 };
    QString description;
    QString token;

    while( !file.atEnd() ) {
        str = file.readLine();
        QStringList tkz = str.split(",");
        if(tkz.size() < 2 )continue;
        int i = 0;
        token = tkz[i++]; // code
        description = tkz[i++]; // May contain comma
        if( !description.endsWith('\"') ) description.append(tkz[i++]);
        description.replace("\"", "");
        if(i<tkz.length())
        {
            token = tkz[i++]; // Acronym
        } else
        {
            token = "";
        }

        if( token.size() > 0 ) {
            //    Filter out any duplicates, in a case insensitive way
            //    i.e. only the first of "DEPARE" and "depare" is added
            bool bdup = false;
            for( unsigned int iPtr = 0; iPtr < pOBJLArray->count(); iPtr++ ) {
                OBJLElement *pOLEt = (OBJLElement *) ( pOBJLArray->at( iPtr ) );
                if( !token.compare(QString::fromUtf8(pOLEt->OBJLName), Qt::CaseInsensitive) ) {
                    bdup = true;
                    break;
                }
            }

            if( !bdup ) {
                QByteArray buffer = token.toUtf8();
                if(buffer.data()) {
                    OBJLElement *pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
                    memcpy( pOLE->OBJLName, buffer.data(), 6 );
                    pOLE->nViz = 0;

                    pOBJLArray->append( (void *) pOLE );

                    OBJLDescriptions.push_back( description );
                }
            }
        }
    }
    return true;
}

void s52plib::UpdateOBJLArray( S57Obj *obj )
{
    //    Search the array for this object class

    bool bNeedNew = true;
    OBJLElement *pOLE;

    for( unsigned int iPtr = 0; iPtr < pOBJLArray->count(); iPtr++ ) {
        pOLE = (OBJLElement *) ( pOBJLArray->at( iPtr ) );
        if( !strncmp( pOLE->OBJLName, obj->FeatureName, 6 ) ) {
            obj->iOBJL = iPtr;
            bNeedNew = false;
            break;
        }
    }

    //    Not found yet, so add an element
    if( bNeedNew ) {
        pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
        memcpy( pOLE->OBJLName, obj->FeatureName, OBJL_NAME_LEN );
        pOLE->nViz = 1;

        pOBJLArray->append( (void *) pOLE );
        obj->iOBJL = pOBJLArray->count() - 1;
    }

}

int s52plib::SetLineFeaturePriority( ObjRazRules *rzRules, int npriority )
{

    int priority_set = npriority; // may be adjusted

    Rules *rules = rzRules->LUP->ruleList;

    //      Do Object Type Filtering
    //    If the object s not currently visible (i.e. classed as a not-currently visible category),
    //    then do not set the line segment priorities at all
    //

    bool b_catfilter = true;

    // DEPCNT is mutable
    if( m_nDisplayCategory == STANDARD ) {
        if( ( DISPLAYBASE != rzRules->LUP->DISC ) && ( STANDARD != rzRules->LUP->DISC ) ) {
            b_catfilter = rzRules->obj->m_bcategory_mutable;
        }
    } else if( m_nDisplayCategory == DISPLAYBASE ) {
        if( DISPLAYBASE != rzRules->LUP->DISC ) {
            b_catfilter = rzRules->obj->m_bcategory_mutable;
        }
    }

    if( IsObjNoshow( rzRules->LUP->OBCL) )
        b_catfilter = false;
    
    if(!b_catfilter)            // No chance this object is visible
        return 0;


    while( rules != NULL ) {
        switch( rules->ruleType ){

        case RUL_SIM_LN:
        case RUL_COM_LN:
            PrioritizeLineFeature( rzRules, priority_set );
            break; // LC

        case RUL_CND_SY: {
            if( !rzRules->obj->bCS_Added ) {
                rzRules->obj->CSrules = NULL;
                GetAndAddCSRules( rzRules, rules );
                rzRules->obj->bCS_Added = 1; // mark the object
            }
            Rules *rules_last = rules;
            rules = rzRules->obj->CSrules;

            while( NULL != rules ) {
                switch( rules->ruleType ){
                case RUL_SIM_LN:
                case RUL_COM_LN:
                    PrioritizeLineFeature( rzRules, priority_set );
                    break;
                case RUL_NONE:
                default:
                    break; // no rule type (init)
                }
                rules_last = rules;
                rules = rules->next;
            }

            rules = rules_last;
            break;
        }

        case RUL_NONE:
        default:
            break; // no rule type (init)
        } // switch

        rules = rules->next;
    }

    return 1;
}

int s52plib::PrioritizeLineFeature( ObjRazRules *rzRules, int npriority )
{
    if(rzRules->obj->m_ls_list){
        
        VE_Element *pedge;
        connector_segment *pcs;
        line_segment_element *ls = rzRules->obj->m_ls_list;
        while( ls ){
            switch (ls->ls_type){
            case TYPE_EE:
            case TYPE_EE_REV:

                pedge = ls->pedge; //(VE_Element *)ls->private0;
                if(pedge)
                    pedge->max_priority = npriority;// qMax(pedge->max_priority, npriority);
                break;

            default:
                pcs = ls->pcs; //(connector_segment *)ls->private0;
                if(pcs)
                    pcs->max_priority_cs = npriority; //qMax(pcs->max_priority, npriority);
                break;
            }
            
            ls = ls->next;
        }
    }

    else if(rzRules->obj->m_ls_list_legacy){            // PlugIn (S63)
        
        PI_connector_segment *pcs;
        VE_Element *pedge;
        
        PI_line_segment_element *ls = rzRules->obj->m_ls_list_legacy;
        while( ls ){
            switch (ls->type){
            case TYPE_EE:

                pedge = (VE_Element *)ls->private0;
                if(pedge)
                    pedge->max_priority = npriority;// qMax(pedge->max_priority, npriority);
                break;

            default:
                pcs = (PI_connector_segment *)ls->private0;
                if(pcs)
                    pcs->max_priority = npriority; //qMax(pcs->max_priority, npriority);
                break;
            }
            
            ls = ls->next;
        }
    }
#if 0    
    else if( rzRules->obj->m_n_lsindex && rzRules->obj->m_lsindex_array) {
        VE_Hash *edge_hash;
        
        if( rzRules->obj->m_chart_context->chart ){
            edge_hash = &rzRules->obj->m_chart_context->chart->Get_ve_hash();
        }
        else {
            edge_hash = (VE_Hash *)rzRules->obj->m_chart_context->m_pve_hash;
        }
        
        int *index_run = rzRules->obj->m_lsindex_array;

        for( int iseg = 0; iseg < rzRules->obj->m_n_lsindex; iseg++ ) {
            //  Get first connected node
            int inode = *index_run++;

            VE_Element *pedge = 0;
            //  Get the edge
            int enode = *index_run++;
            if(enode)
                pedge = (*edge_hash)[enode];

            //    Set priority
            if(pedge){
                pedge->max_priority = npriority;
            }

            //  Get last connected node
            inode = *index_run++;

        }
    }
#endif

    return 1;
}

class XPOINT {
public:
    float x, y;
};

class XLINE {
public:
    XPOINT o, p;
    float m;
    float c;
};

bool TestLinesIntersection( XLINE &a, XLINE &b )
{
    XPOINT i;

    if( ( a.p.x == a.o.x ) && ( b.p.x == b.o.x ) ) // both vertical
    {
        return ( a.p.x == b.p.x );
    }

    if( a.p.x == a.o.x ) // a line a is vertical
    {
        // calculate b gradient
        b.m = ( b.p.y - b.o.y ) / ( b.p.x - b.o.x );
        // calculate axis intersect values
        b.c = b.o.y - ( b.m * b.o.x );
        // calculate y point of intercept
        i.y = b.o.y + ( ( a.o.x - b.o.x ) * b.m );
        if( i.y < qMin(a.o.y, a.p.y) || i.y > qMax(a.o.y, a.p.y) ) return false;
        return true;
    }

    if( b.p.x == b.o.x ) // line b is vertical
    {
        // calculate b gradient
        a.m = ( a.p.y - a.o.y ) / ( a.p.x - a.o.x );
        // calculate axis intersect values
        a.c = a.o.y - ( a.m * a.o.x );
        // calculate y point of intercept
        i.y = a.o.y + ( ( b.o.x - a.o.x ) * a.m );
        if( i.y < qMin(b.o.y, b.p.y) || i.y > qMax(b.o.y, b.p.y) ) return false;
        return true;
    }

    // calculate gradients
    a.m = ( a.p.y - a.o.y ) / ( a.p.x - a.o.x );
    b.m = ( b.p.y - b.o.y ) / ( b.p.x - b.o.x );
    // parallel lines can't intercept
    if( a.m == b.m ) {
        return false;
    }
    // calculate axis intersect values
    a.c = a.o.y - ( a.m * a.o.x );
    b.c = b.o.y - ( b.m * b.o.x );
    // calculate x point of intercept
    i.x = ( b.c - a.c ) / ( a.m - b.m );
    // is intersection point in segment
    if( i.x < qMin(a.o.x, a.p.x) || i.x > qMax(a.o.x, a.p.x) ) {
        return false;
    }
    if( i.x < qMin(b.o.x, b.p.x) || i.x > qMax(b.o.x, b.p.x) ) {
        return false;
    }
    // points intercept
    return true;
}

//-----------------------------------------------------------------------
//    Check a triangle described by point array, and rectangle described by render_canvas_parms
//    for intersection
//    Return false if no intersection
//-----------------------------------------------------------------------
bool s52plib::inter_tri_rect( zchxPoint *ptp, render_canvas_parms *pb_spec )
{
    //    First stage
    //    Check all three points of triangle to see it any are within the render rectangle

    wxBoundingBox rect( pb_spec->lclip, pb_spec->y, pb_spec->rclip, pb_spec->y + pb_spec->height );

    for( int i = 0; i < 3; i++ ) {
        if( rect.PointInBox( ptp[i].x, ptp[i].y ) ) return true;
    }

    //    Next stage
    //    Check all four points of rectangle to see it any are within the render triangle

    double p[6];
    MyPoint *pmp = (MyPoint *) p;

    for( int i = 0; i < 3; i++ ) {
        pmp[i].x = ptp[i].x;
        pmp[i].y = ptp[i].y;
    }

    if( G_PtInPolygon( pmp, 3, pb_spec->lclip, pb_spec->y ) ) return true;

    if( G_PtInPolygon( pmp, 3, pb_spec->lclip, pb_spec->y + pb_spec->height ) ) return true;

    if( G_PtInPolygon( pmp, 3, pb_spec->rclip, pb_spec->y ) ) return true;

    if( G_PtInPolygon( pmp, 3, pb_spec->rclip, pb_spec->y + pb_spec->height ) ) return true;

    //    last step
    //    Check triangle lines against rect lines for line intersect

    for( int i = 0; i < 3; i++ ) {
        XLINE a;
        a.o.x = ptp[i].x;
        a.o.y = ptp[i].y;
        if( i == 2 ) {
            a.p.x = ptp[0].x;
            a.p.y = ptp[0].y;
        } else {
            a.p.x = ptp[i + 1].x;
            a.p.y = ptp[i + 1].y;
        }

        XLINE b;

        //    top line
        b.o.x = pb_spec->lclip;
        b.o.y = pb_spec->y;
        b.p.x = pb_spec->rclip;
        b.p.y = pb_spec->y;

        if( TestLinesIntersection( a, b ) ) return true;

        //    right line
        b.o.x = pb_spec->rclip;
        b.o.y = pb_spec->y;
        b.p.x = pb_spec->rclip;
        b.p.y = pb_spec->y + pb_spec->height;

        if( TestLinesIntersection( a, b ) ) return true;

        //    bottom line
        b.o.x = pb_spec->rclip;
        b.o.y = pb_spec->y + pb_spec->height;
        b.p.x = pb_spec->lclip;
        b.p.y = pb_spec->y + pb_spec->height;

        if( TestLinesIntersection( a, b ) ) return true;

        //    left line
        b.o.x = pb_spec->lclip;
        b.o.y = pb_spec->y + pb_spec->height;
        b.p.x = pb_spec->lclip;
        b.p.y = pb_spec->y;

        if( TestLinesIntersection( a, b ) ) return true;
    }

    return false; // no Intersection

}

//----------------------------------------------------------------------------------
//
//              Fast Basic Canvas Rendering
//              Render triangle
//
//----------------------------------------------------------------------------------
int s52plib::dda_tri( zchxPoint *ptp, S52color *c, render_canvas_parms *pb_spec,
                      render_canvas_parms *pPatt_spec )
{
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;

    if( !inter_tri_rect( ptp, pb_spec ) ) return 0;

    if( NULL != c ) {
        if(pb_spec->b_revrgb) {
            r = c->R;
            g = c->G;
            b = c->B;
        }
        else {
            b = c->R;
            g = c->G;
            r = c->B;
        }
    }

    //      Color Debug
    /*    int fc = rand();
     b = fc & 0xff;
     g = fc & 0xff;
     r = fc & 0xff;
     */

    int color_int = 0;
    if( NULL != c ) color_int = ( ( r ) << 16 ) + ( ( g ) << 8 ) + ( b );

    //      Determine ymin and ymax indices

    int ymax = ptp[0].y;
    int ymin = ymax;
    int xmin, xmax, xmid, ymid;
    int imin = 0;
    int imax = 0;
    int imid;

    for( int ip = 1; ip < 3; ip++ ) {
        if( ptp[ip].y > ymax ) {
            imax = ip;
            ymax = ptp[ip].y;
        }
        if( ptp[ip].y <= ymin ) {
            imin = ip;
            ymin = ptp[ip].y;
        }
    }

    imid = 3 - ( imin + imax ); // do the math...

    xmax = ptp[imax].x;
    xmin = ptp[imin].x;
    xmid = ptp[imid].x;
    ymid = ptp[imid].y;

    //      Create edge arrays using fast integer DDA
    int m, x, dy, count;
    bool dda8 = false;
    bool cw;

    if( ( abs( xmax - xmin ) > 32768 ) || ( abs( xmid - xmin ) > 32768 )
            || ( abs( xmax - xmid ) > 32768 ) || ( abs( ymax - ymin ) > 32768 )
            || ( abs( ymid - ymin ) > 32768 ) || ( abs( ymax - ymid ) > 32768 ) || ( xmin > 32768 )
            || ( xmid > 32768 ) ) {
        dda8 = true;

        dy = ( ymax - ymin );
        if( dy ) {
            m = ( xmax - xmin ) << 8;
            m /= dy;

            x = xmin << 8;

            for( count = ymin; count <= ymax; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) ledge[count] = x >> 8;
                x += m;
            }
        }

        dy = ( ymid - ymin );
        if( dy ) {
            m = ( xmid - xmin ) << 8;
            m /= dy;

            x = xmin << 8;

            for( count = ymin; count <= ymid; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) redge[count] = x >> 8;
                x += m;
            }
        }

        dy = ( ymax - ymid );
        if( dy ) {
            m = ( xmax - xmid ) << 8;
            m /= dy;

            x = xmid << 8;

            for( count = ymid; count <= ymax; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) redge[count] = x >> 8;
                x += m;
            }
        }

        double ddfSum = 0;
        //      Check the triangle edge winding direction
        ddfSum += ( xmin / 1 ) * ( ymax / 1 ) - ( ymin / 1 ) * ( xmax / 1 );
        ddfSum += ( xmax / 1 ) * ( ymid / 1 ) - ( ymax / 1 ) * ( xmid / 1 );
        ddfSum += ( xmid / 1 ) * ( ymin / 1 ) - ( ymid / 1 ) * ( xmin / 1 );
        cw = ddfSum < 0;

    } else {

        dy = ( ymax - ymin );
        if( dy ) {
            m = ( xmax - xmin ) << 16;
            m /= dy;

            x = xmin << 16;

            for( count = ymin; count <= ymax; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) ledge[count] = x >> 16;
                x += m;
            }
        }

        dy = ( ymid - ymin );
        if( dy ) {
            m = ( xmid - xmin ) << 16;
            m /= dy;

            x = xmin << 16;

            for( count = ymin; count <= ymid; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) redge[count] = x >> 16;
                x += m;
            }
        }

        dy = ( ymax - ymid );
        if( dy ) {
            m = ( xmax - xmid ) << 16;
            m /= dy;

            x = xmid << 16;

            for( count = ymid; count <= ymax; count++ ) {
                if( ( count >= 0 ) && ( count < 1500 ) ) redge[count] = x >> 16;
                x += m;
            }
        }

        //      Check the triangle edge winding direction
        long dfSum = 0;
        dfSum += xmin * ymax - ymin * xmax;
        dfSum += xmax * ymid - ymax * xmid;
        dfSum += xmid * ymin - ymid * xmin;

        cw = dfSum < 0;

    } // else

    //      if cw is true, redge is actually on the right

    int y1 = ymax;
    int y2 = ymin;

    int ybt = pb_spec->y;
    int yt = pb_spec->y + pb_spec->height;

    if( y1 > yt ) y1 = yt;
    if( y1 < ybt ) y1 = ybt;

    if( y2 > yt ) y2 = yt;
    if( y2 < ybt ) y2 = ybt;

    int lclip = pb_spec->lclip;
    int rclip = pb_spec->rclip;
    if (y1 == y2 )
        return 0;

    //              Clip the triangle
    if( cw ) {
        for( int iy = y2; iy <= y1; iy++ ) {
            if( ledge[iy] < lclip ) {
                if( redge[iy] < lclip ) ledge[iy] = -1;
                else
                    ledge[iy] = lclip;
            }

            if( redge[iy] > rclip ) {
                if( ledge[iy] > rclip ) ledge[iy] = -1;
                else
                    redge[iy] = rclip;
            }
        }
    } else {
        for( int iy = y2; iy <= y1; iy++ ) {
            if( redge[iy] < lclip ) {
                if( ledge[iy] < lclip ) ledge[iy] = -1;
                else
                    redge[iy] = lclip;
            }

            if( ledge[iy] > rclip ) {
                if( redge[iy] > rclip ) ledge[iy] = -1;
                else
                    ledge[iy] = rclip;
            }
        }
    }

    //              Fill the triangle

    int ya = y2;
    int yb = y1;

    unsigned char *pix_buff = pb_spec->pix_buff;

    int patt_size_x = 0, patt_size_y = 0, patt_pitch = 0;
    unsigned char *patt_s0 = NULL;
    if( pPatt_spec ) {
        patt_size_y = pPatt_spec->height;
        patt_size_x = pPatt_spec->width;
        patt_pitch = pPatt_spec->pb_pitch;
        patt_s0 = pPatt_spec->pix_buff;

        if(patt_size_y == 0) /* integer division by this value below */
            return false;
    }

    if( pb_spec->depth == 24 ) {
        for( int iyp = ya; iyp < yb; iyp++ ) {
            if( ( iyp >= ybt ) && ( iyp < yt ) ) {
                int yoff = ( iyp - pb_spec->y ) * pb_spec->pb_pitch;

                unsigned char *py = pix_buff + yoff;

                int ix, ixm;
                if( cw ) {
                    ix = ledge[iyp];
                    ixm = redge[iyp];
                } else {
                    ixm = ledge[iyp];
                    ix = redge[iyp];
                }

                if( ledge[iyp] != -1 ) {

                    //    This would be considered a failure of the dda algorithm
                    //    Happens on very high zoom, with very large triangles.
                    //    The integers of the dda algorithm don't have enough bits...
                    //    Anyway, just ignore this triangle if it happens
                    if( ix > ixm ) continue;

                    int xoff = ( ix - pb_spec->x ) * 3;

                    unsigned char *px = py + xoff;

                    if( pPatt_spec  ) // Pattern
                    {
                        int y_stagger = ( iyp - pPatt_spec->y ) / patt_size_y;
                        int x_stagger_off = 0;
                        if( ( y_stagger & 1 ) && pPatt_spec->b_stagger ) x_stagger_off =
                                pPatt_spec->width / 2;

                        int patt_y = abs( ( iyp - pPatt_spec->y ) ) % patt_size_y;

                        unsigned char *pp0 = patt_s0 + ( patt_y * patt_pitch );

                        while( ix <= ixm ) {
                            int patt_x = abs( ( ( ix - pPatt_spec->x ) + x_stagger_off ) % patt_size_x );

                            unsigned char *pp = pp0 + ( patt_x * 4 );
                            unsigned char alpha = pp[3];
                            double da = (double) alpha / 256.;

                            unsigned char r = (unsigned char) ( *px*(1.0-da) + pp[0] * da );
                            unsigned char g = (unsigned char) ( *(px+1)*(1.0-da) + pp[1] * da );
                            unsigned char b = (unsigned char) ( *(px+2)*(1.0-da) + pp[2] * da );

                            *px++ = r;
                            *px++ = g;
                            *px++ = b;
                            ix++;
                        }
                    }

                    else // No Pattern
                    {
#if defined( __WXGTK__) && defined(__INTEL__)
#define memset3(dest, value, count) \
    __asm__ __volatile__ ( \
    "cmp $0,%2\n\t" \
    "jg 2f\n\t" \
    "je 3f\n\t" \
    "jmp 4f\n\t" \
    "2:\n\t" \
    "movl  %0,(%1)\n\t" \
    "add $3,%1\n\t" \
    "dec %2\n\t" \
    "jnz 2b\n\t" \
    "3:\n\t" \
    "movb %b0,(%1)\n\t" \
    "inc %1\n\t" \
    "movb %h0,(%1)\n\t" \
    "inc %1\n\t" \
    "shr $16,%0\n\t" \
    "movb %b0,(%1)\n\t" \
    "4:\n\t" \
    : : "a"(value), "D"(dest), "r"(count) :  );

                        int count = ixm-ix;
                        memset3 ( px, color_int, count )
        #else

                        while( ix <= ixm ) {
                            *px++ = b;
                            *px++ = g;
                            *px++ = r;

                            ix++;
                        }
#endif
                    }
                }
            }
        }
    }

    if( pb_spec->depth == 32 ) {

        assert( ya <= yb );

        for( int iyp = ya; iyp < yb; iyp++ ) {
            if( ( iyp >= ybt ) && ( iyp < yt ) ) {
                int yoff = ( iyp - pb_spec->y ) * pb_spec->pb_pitch;

                unsigned char *py = pix_buff + yoff;

                int ix, ixm;
                if( cw ) {
                    ix = ledge[iyp];
                    ixm = redge[iyp];
                } else {
                    ixm = ledge[iyp];
                    ix = redge[iyp];
                }

                if( ledge[iyp] != -1 ) {
                    //    This would be considered a failure of the dda algorithm
                    //    Happens on very high zoom, with very large triangles.
                    //    The integers of the dda algorithm don't have enough bits...
                    //    Anyway, just ignore this triangle if it happens
                    if( ix > ixm ) continue;

                    int xoff = ( ix - pb_spec->x ) * pb_spec->depth / 8;

                    unsigned char *px = py + xoff;

                    if( pPatt_spec ) // Pattern
                    {
                        int y_stagger = ( iyp - pPatt_spec->y ) / patt_size_y;

                        int x_stagger_off = 0;
                        if( ( y_stagger & 1 ) && pPatt_spec->b_stagger ) x_stagger_off =
                                pPatt_spec->width / 2;

                        int patt_y = abs( ( iyp - pPatt_spec->y ) ) % patt_size_y;

                        unsigned char *pp0 = patt_s0 + ( patt_y * patt_pitch );

                        while( ix <= ixm ) {
                            int patt_x = abs(
                                        ( ( ix - pPatt_spec->x ) + x_stagger_off ) % patt_size_x );
                            /*
                             if(pPatt_spec->depth == 24)
                             {
                             unsigned char *pp = pp0 + (patt_x * 3);

                             //  Todo    This line assumes unused_color is always 0,0,0
                             if( pp[0] && pp[1] && pp[2] ) {
                             *px++ = *pp++;
                             *px++ = *pp++;
                             *px++ = *pp++;
                             px++;
                             } else {
                             px += 4;
                             //                                                      pp += 4;
                             }
                             }
                             else
                             */
                            {
                                unsigned char *pp = pp0 + ( patt_x * 4 );
                                unsigned char alpha = pp[3];
                                if( alpha > 128 ) {
                                    double da = (double) alpha / 256.;

                                    unsigned char r = (unsigned char) ( pp[0] * da );
                                    unsigned char g = (unsigned char) ( pp[1] * da );
                                    unsigned char b = (unsigned char) ( pp[2] * da );

                                    *px++ = r;
                                    *px++ = g;
                                    *px++ = b;
                                    px++;
                                } else
                                    px += 4;
                            }
                            ix++;
                        }
                    }

                    else // No Pattern
                    {
                        int *pxi = (int *) px;
                        while( ix <= ixm ) {
                            *pxi++ = color_int;
                            ix++;
                        }
                    }
                }
            }
        }
    }

    return true;
}

//----------------------------------------------------------------------------------
//
//              Render Trapezoid
//
//----------------------------------------------------------------------------------
inline int s52plib::dda_trap( zchxPoint *segs, int lseg, int rseg, int ytop, int ybot, S52color *c,
                              render_canvas_parms *pb_spec, render_canvas_parms *pPatt_spec )
{
    unsigned char r = 0, g = 0, b = 0;

    if( NULL != c ) {
        if(pb_spec->b_revrgb) {
            r = c->R;
            g = c->G;
            b = c->B;
        }
        else {
            b = c->R;
            g = c->G;
            r = c->B;
        }
    }

    //      Color Debug
    /*    int fc = rand();
     b = fc & 0xff;
     g = fc & 0xff;
     r = fc & 0xff;
     */

    //      int debug = 0;
    int ret_val = 0;

    int color_int = 0;
    if( NULL != c ) color_int = ( ( r ) << 16 ) + ( ( g ) << 8 ) + ( b );

    //      Create edge arrays using fast integer DDA

    int lclip = pb_spec->lclip;
    int rclip = pb_spec->rclip;

    int m, x, dy, count;

    //    Left edge
    int xmax = segs[lseg].x;
    int xmin = segs[lseg + 1].x;
    int ymax = segs[lseg].y;
    int ymin = segs[lseg + 1].y;

    if( ymax < ymin ) {
        int a = ymax;
        ymax = ymin;
        ymin = a;

        a = xmax;
        xmax = xmin;
        xmin = a;
    }

    int y_dda_limit = qMin ( ybot, ymax );
    y_dda_limit = qMin ( y_dda_limit, 1499 ); // don't overrun edge array

    //    Some peephole optimization:
    //    if xmax and xmin are both < 0, arrange to simply fill the ledge array with 0
    if( ( xmax < 0 ) && ( xmin < 0 ) ) {
        xmax = -2;
        xmin = -2;
    }
    //    if xmax and xmin are both > rclip, arrange to simply fill the ledge array with rclip + 1
    //    This may induce special clip case below, and cause trap not to be rendered
    else
        if( ( xmax > rclip ) && ( xmin > rclip ) ) {
            xmax = rclip + 1;
            xmin = rclip + 1;
        }

    dy = ( ymax - ymin );
    if( dy ) {
        m = ( xmax - xmin ) << 16;
        m /= dy;

        x = xmin << 16;

        //TODO implement this logic in dda_tri also
        count = ymin;
        while( count < 0 ) {
            x += m;
            count++;
        }

        while( count < y_dda_limit ) {
            ledge[count] = x >> 16;
            x += m;
            count++;
        }
    }

    if( ( ytop < ymin ) || ( ybot > ymax ) ) {
        //            printf ( "### ledge out of range\n" );
        ret_val = 1;
        //            r=255;
        //            g=0;
        //            b=0;
    }

    //    Right edge
    xmax = segs[rseg].x;
    xmin = segs[rseg + 1].x;
    ymax = segs[rseg].y;
    ymin = segs[rseg + 1].y;

    //Note this never gets hit???
    if( ymax < ymin ) {
        int a = ymax;
        ymax = ymin;
        ymin = a;

        a = xmax;
        xmax = xmin;
        xmin = a;
    }

    //    Some peephole optimization:
    //    if xmax and xmin are both < 0, arrange to simply fill the redge array with -1
    //    This may induce special clip case below, and cause trap not to be rendered
    if( ( xmax < 0 ) && ( xmin < 0 ) ) {
        xmax = -1;
        xmin = -1;
    }

    //    if xmax and xmin are both > rclip, arrange to simply fill the redge array with rclip + 1
    //    This may induce special clip case below, and cause trap not to be rendered
    else
        if( ( xmax > rclip ) && ( xmin > rclip ) ) {
            xmax = rclip + 1;
            xmin = rclip + 1;
        }

    y_dda_limit = qMin ( ybot, ymax );
    y_dda_limit = qMin ( y_dda_limit, 1499 ); // don't overrun edge array

    dy = ( ymax - ymin );
    if( dy ) {
        m = ( xmax - xmin ) << 16;
        m /= dy;

        x = xmin << 16;

        count = ymin;
        while( count < 0 ) {
            x += m;
            count++;
        }

        while( count < y_dda_limit ) {
            redge[count] = x >> 16;
            x += m;
            count++;
        }
    }

    if( ( ytop < ymin ) || ( ybot > ymax ) ) {
        //            printf ( "### redge out of range\n" );
        ret_val = 1;
        //            r=255;
        //            g=0;
        //            b=0;
    }

    //    Clip trapezoid to height spec
    int y1 = ybot;
    int y2 = ytop;

    int ybt = pb_spec->y;
    int yt = pb_spec->y + pb_spec->height;

    if( y1 > yt ) y1 = yt;
    if( y1 < ybt ) y1 = ybt;

    if( y2 > yt ) y2 = yt;
    if( y2 < ybt ) y2 = ybt;

    //   Clip the trapezoid to width
    for( int iy = y2; iy <= y1; iy++ ) {
        if( ledge[iy] < lclip ) {
            if( redge[iy] < lclip ) ledge[iy] = -1;
            else
                ledge[iy] = lclip;
        }

        if( redge[iy] > rclip ) {
            if( ledge[iy] > rclip ) ledge[iy] = -1;
            else
                redge[iy] = rclip;
        }
    }

    //    Fill the trapezoid

    int ya = y2;
    int yb = y1;

    unsigned char *pix_buff = pb_spec->pix_buff;

    int patt_size_x = 0;
    int patt_size_y = 0;
    int patt_pitch = 0;
    unsigned char *patt_s0 = NULL;
    if( pPatt_spec ) {
        patt_size_y = pPatt_spec->height;
        patt_size_x = pPatt_spec->width;
        patt_pitch = pPatt_spec->pb_pitch;
        patt_s0 = pPatt_spec->pix_buff;
    }

    if( pb_spec->depth == 24 ) {
        for( int iyp = ya; iyp < yb; iyp++ ) {
            if( ( iyp >= ybt ) && ( iyp < yt ) ) {
                int yoff = ( iyp - pb_spec->y ) * pb_spec->pb_pitch;

                unsigned char *py = pix_buff + yoff;

                int ix, ixm;
                ix = ledge[iyp];
                ixm = redge[iyp];

                //                        if(debug) printf("iyp %d, ix %d, ixm %d\n", iyp, ix, ixm);
                //                           int ix = ledge[iyp];
                //                            if(ix != -1)                    // special clip case
                if( ledge[iyp] != -1 ) {
                    int xoff = ( ix - pb_spec->x ) * 3;

                    unsigned char *px = py + xoff;

                    if( pPatt_spec ) // Pattern
                    {
                        int y_stagger = ( iyp - pPatt_spec->y ) / patt_size_y;
                        int x_stagger_off = 0;
                        if( ( y_stagger & 1 ) && pPatt_spec->b_stagger ) x_stagger_off =
                                pPatt_spec->width / 2;

                        int patt_y = abs( ( iyp - pPatt_spec->y ) ) % patt_size_y;
                        unsigned char *pp0 = patt_s0 + ( patt_y * patt_pitch );

                        while( ix <= ixm ) {
                            int patt_x = abs(
                                        ( ( ix - pPatt_spec->x ) + x_stagger_off ) % patt_size_x );
                            /*
                             if(pPatt_spec->depth == 24)
                             {
                             unsigned char *pp = pp0 + (patt_x * 3);

                             //  Todo    This line assumes unused_color is always 0,0,0
                             if( pp[0] && pp[1] && pp[2] ) {
                             *px++ = *pp++;
                             *px++ = *pp++;
                             *px++ = *pp++;
                             } else {
                             px += 3;
                             pp += 3;
                             }
                             }
                             else
                             */
                            {
                                unsigned char *pp = pp0 + ( patt_x * 4 );
                                unsigned char alpha = pp[3];
                                if( alpha > 128 ) {
                                    double da = (double) alpha / 256.;

                                    unsigned char r = (unsigned char) ( pp[0] * da );
                                    unsigned char g = (unsigned char) ( pp[1] * da );
                                    unsigned char b = (unsigned char) ( pp[2] * da );

                                    *px++ = r;
                                    *px++ = g;
                                    *px++ = b;
                                } else
                                    px += 3;
                            }

                            ix++;
                        }
                    }

                    else // No Pattern
                    {
#if defined(__WXGTK__WITH_OPTIMIZE_0) && defined(__INTEL__)
#define memset3d(dest, value, count) \
    __asm__ __volatile__ ( \
    "cmp $0,%2\n\t" \
    "jg ld0\n\t" \
    "je ld1\n\t" \
    "jmp ld2\n\t" \
    "ld0:\n\t" \
    "movl  %0,(%1)\n\t" \
    "add $3,%1\n\t" \
    "dec %2\n\t" \
    "jnz ld0\n\t" \
    "ld1:\n\t" \
    "movb %b0,(%1)\n\t" \
    "inc %1\n\t" \
    "movb %h0,(%1)\n\t" \
    "inc %1\n\t" \
    "shr $16,%0\n\t" \
    "movb %b0,(%1)\n\t" \
    "ld2:\n\t" \
    : : "a"(value), "D"(dest), "r"(count) :  );
                        int count = ixm-ix;
                        memset3d ( px, color_int, count )
        #else

                        while( ix <= ixm ) {
                            *px++ = b;
                            *px++ = g;
                            *px++ = r;

                            ix++;
                        }
#endif
                    }
                }
            }
        }
    }

    if( pb_spec->depth == 32 ) {

        assert( ya <= yb );

        for( int iyp = ya; iyp < yb; iyp++ ) {
            if( ( iyp >= ybt ) && ( iyp < yt ) ) {
                int yoff = ( iyp - pb_spec->y ) * pb_spec->pb_pitch;

                unsigned char *py = pix_buff + yoff;

                int ix, ixm;
                ix = ledge[iyp];
                ixm = redge[iyp];

                if( ledge[iyp] != -1 ) {
                    int xoff = ( ix - pb_spec->x ) * pb_spec->depth / 8;

                    unsigned char *px = py + xoff;

                    if( pPatt_spec ) // Pattern
                    {
                        int y_stagger = ( iyp - pPatt_spec->y ) / patt_size_y;
                        int x_stagger_off = 0;
                        if( ( y_stagger & 1 ) && pPatt_spec->b_stagger ) x_stagger_off =
                                pPatt_spec->width / 2;

                        int patt_y = abs( ( iyp - pPatt_spec->y ) ) % patt_size_y;
                        unsigned char *pp0 = patt_s0 + ( patt_y * patt_pitch );

                        while( ix <= ixm ) {
                            int patt_x = abs(
                                        ( ( ix - pPatt_spec->x ) + x_stagger_off ) % patt_size_x );
                            /*
                             if(pPatt_spec->depth == 24)
                             {
                             unsigned char *pp = pp0 + (patt_x * 3);

                             //  Todo    This line assumes unused_color is always 0,0,0
                             if( pp[0] && pp[1] && pp[2] ) {
                             *px++ = *pp++;
                             *px++ = *pp++;
                             *px++ = *pp++;
                             px++;
                             } else {
                             px += 4;
                             //                                                      pp += 3;
                             }
                             }
                             else
                             */
                            {
                                unsigned char *pp = pp0 + ( patt_x * 4 );
                                unsigned char alpha = pp[3];
                                if( alpha > 128 ) {
                                    double da = (double) alpha / 256.;

                                    unsigned char r = (unsigned char) ( pp[0] * da );
                                    unsigned char g = (unsigned char) ( pp[1] * da );
                                    unsigned char b = (unsigned char) ( pp[2] * da );

                                    *px++ = r;
                                    *px++ = g;
                                    *px++ = b;
                                    px++;
                                } else
                                    px += 4;
                            }
                            ix++;
                        }
                    }

                    else // No Pattern
                    {
                        int *pxi = (int *) px;
                        while( ix <= ixm ) {
                            *pxi++ = color_int;
                            ix++;
                        }
                    }

                }
            }
        }
    }

    return ret_val;
}

void s52plib::RenderToBufferFilledPolygon( ObjRazRules *rzRules, S57Obj *obj, S52color *c,
                                           render_canvas_parms *pb_spec, render_canvas_parms *pPatt_spec, ViewPort *vp )
{
    //    LLBBox BBView = vp->getBBox();
    LLBBox BBView = vp->getBBox();
    // please untangle this logic with the logic below
    if(BBView.GetMaxLon()+180 < vp->lon())
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() + 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() + 360);
    else if(BBView.GetMinLon()-180 > vp->lon())
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() - 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() - 360);


    S52color cp;
    if( NULL != c ) {
        cp.R = c->R;
        cp.G = c->G;
        cp.B = c->B;
    }

    if( obj->pPolyTessGeo ) {
        if( !rzRules->obj->pPolyTessGeo->IsOk() ){ // perform deferred tesselation
            rzRules->obj->pPolyTessGeo->BuildDeferredTess();
        }

        zchxPoint *pp3 = (zchxPoint *) malloc( 3 * sizeof(zchxPoint) );
        zchxPoint *ptp = (zchxPoint *) malloc(
                    ( obj->pPolyTessGeo->GetnVertexMax() + 1 ) * sizeof(zchxPoint) );

        //  Allow a little slop in calculating whether a triangle
        //  is within the requested Viewport
        double margin = BBView.GetLonRange() * .05;

        PolyTriGroup *ppg = obj->pPolyTessGeo->Get_PolyTriGroup_head();

        TriPrim *p_tp = ppg->tri_prim_head;
        while( p_tp ) {
            LLBBox box;
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
            }
            else
                box = p_tp->tri_box;

            if(!BBView.IntersectOut(box)) {
                //      Get and convert the points
                zchxPoint *pr = ptp;

                if(ppg->data_type == DATA_TYPE_DOUBLE){
                    double *pvert_list = p_tp->p_vertex;

                    for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                        double lon = *pvert_list++;
                        double lat = *pvert_list++;
                        GetPointPixSingle( rzRules, lat, lon, pr, vp );

                        pr++;
                    }
                }
                else {
                    float *pvert_list = (float *)p_tp->p_vertex;
                    
                    for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                        double lon = *pvert_list++;
                        double lat = *pvert_list++;
                        GetPointPixSingle( rzRules, lat, lon, pr, vp );
                        
                        pr++;
                    }
                }
                
                switch( p_tp->type ){
                case PTG_TRIANGLE_FAN: {
                    for( int it = 0; it < p_tp->nVert - 2; it++ ) {
                        pp3[0].x = ptp[0].x;
                        pp3[0].y = ptp[0].y;

                        pp3[1].x = ptp[it + 1].x;
                        pp3[1].y = ptp[it + 1].y;

                        pp3[2].x = ptp[it + 2].x;
                        pp3[2].y = ptp[it + 2].y;

                        dda_tri( pp3, &cp, pb_spec, pPatt_spec );
                    }
                    break;
                }
                case PTG_TRIANGLE_STRIP: {
                    for( int it = 0; it < p_tp->nVert - 2; it++ ) {
                        pp3[0].x = ptp[it].x;
                        pp3[0].y = ptp[it].y;

                        pp3[1].x = ptp[it + 1].x;
                        pp3[1].y = ptp[it + 1].y;

                        pp3[2].x = ptp[it + 2].x;
                        pp3[2].y = ptp[it + 2].y;

                        dda_tri( pp3, &cp, pb_spec, pPatt_spec );
                    }
                    break;
                }
                case PTG_TRIANGLES: {

                    for( int it = 0; it < p_tp->nVert; it += 3 ) {
                        pp3[0].x = ptp[it].x;
                        pp3[0].y = ptp[it].y;

                        pp3[1].x = ptp[it + 1].x;
                        pp3[1].y = ptp[it + 1].y;

                        pp3[2].x = ptp[it + 2].x;
                        pp3[2].y = ptp[it + 2].y;

                        dda_tri( pp3, &cp, pb_spec, pPatt_spec );
                    }
                    break;

                }
                }
            } // if bbox
            
            // pick up the next in chain
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                p_tp = (TriPrim *)p_ltp->p_next;
            }
            else
                p_tp = p_tp->p_next;

        } // while
        
        free( ptp );
        free( pp3 );
    } // if pPolyTessGeo
}

int s52plib::RenderToGLAC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifdef ocpnUSE_GL    
    S52color *c;
    char *str = (char*) rules->INSTstr;

    c = getColor( str );

    glColor3ub( c->R, c->G, c->B );

    LLBBox BBView = vp->getBBox();
    // please untangle this logic with the logic below
    if(BBView.GetMaxLon()+180 < vp->lon())
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() + 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() + 360);
    else if(BBView.GetMinLon()-180 > vp->lon())
        BBView.Set(BBView.GetMinLat(), BBView.GetMinLon() - 360,
                   BBView.GetMaxLat(), BBView.GetMaxLon() - 360);

    //  Allow a little slop in calculating whether a triangle
    //  is within the requested Viewport
    double margin = BBView.GetLonRange() * .05;
    BBView.EnLarge( margin );

    bool b_useVBO = m_useVBO && !rzRules->obj->auxParm1 && vp->projectType() == PROJECTION_MERCATOR;
    
    if( rzRules->obj->pPolyTessGeo ) {
        
        bool b_temp_vbo = false;
        bool b_transform = false;
        
        // Set up the OpenGL transform matrix for this object
        // We transform from SENC SM vertex data to screen.

        //  First, the VP transform
        if(b_useVBO || vp->projectType() == PROJECTION_MERCATOR) {
            b_transform = true;
            glPushMatrix();

            glTranslatef( vp->pixWidth() / 2, vp->pixHeight()/2, 0 );
            glScalef( vp->viewScalePPM(), -vp->viewScalePPM(), 0 );
            glTranslatef( -rzRules->sm_transform_parms->easting_vp_center, -rzRules->sm_transform_parms->northing_vp_center, 0 );
            //  Next, the per-object transform

            float x_origin = rzRules->obj->x_origin;
            
            if(rzRules->obj->m_chart_context->chart) {          // not a PlugIn Chart
                if( ( (int)rzRules->obj->auxParm3 == (int)CHART_TYPE_CM93 )
                        || ( (int)rzRules->obj->auxParm3 == (int)CHART_TYPE_CM93COMP ) )
                {
                    //      We may need to translate object coordinates by 360 degrees to conform.
                    if( BBView.GetMaxLon() >= 180. ) {
                        if(rzRules->obj->BBObj.GetMinLon() < BBView.GetMaxLon() - 360.)
                            x_origin += (float)(mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI);
                    }
                    else
                        if( (BBView.GetMinLon() <= -180. && rzRules->obj->BBObj.GetMaxLon() > BBView.GetMinLon() + 360.)
                                || (rzRules->obj->BBObj.GetMaxLon() > 180 && BBView.GetMinLon() + 360 < rzRules->obj->BBObj.GetMaxLon() )
                                )
                            x_origin -= (float)(mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI);
                }
            }
            
            glTranslatef( x_origin, rzRules->obj->y_origin, 0);
            glScalef( rzRules->obj->x_rate, rzRules->obj->y_rate, 0 );
        }
        
        // perform deferred tesselation
        if( !rzRules->obj->pPolyTessGeo->IsOk() ){
            rzRules->obj->pPolyTessGeo->BuildDeferredTess();
        }

        //  Get the vertex data
        PolyTriGroup *ppg_vbo = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

        //  Has the input vertex buffer been converted to "single_alloc float" model?
        //  and is it allowed?
        if(!ppg_vbo->bsingle_alloc && (rzRules->obj->auxParm1 >= 0) ){

            int data_size = sizeof(float);

            //  First calculate the required total byte size
            int total_byte_size = 0;
            TriPrim *p_tp = ppg_vbo->tri_prim_head;
            while( p_tp ) {
                total_byte_size += p_tp->nVert * 2 * data_size;
                p_tp = p_tp->p_next; // pick up the next in chain
            }

            float *vbuf = (float *)malloc(total_byte_size);
            p_tp = ppg_vbo->tri_prim_head;

            if( ppg_vbo->data_type == DATA_TYPE_DOUBLE){  //DOUBLE to FLOAT
                float *p_run = vbuf;
                while( p_tp ) {
                    float *pfbuf = p_run;
                    for( int i=0 ; i < p_tp->nVert * 2 ; ++i){
                        float x = (float)(p_tp->p_vertex[i]);
                        *p_run++ = x;
                    }

                    free(p_tp->p_vertex);
                    p_tp->p_vertex = (double *)pfbuf;

                    p_tp = p_tp->p_next; // pick up the next in chain
                }
            }
            else {          // FLOAT to FLOAT
                float *p_run = vbuf;
                while( p_tp ) {
                    memcpy( p_run, p_tp->p_vertex, p_tp->nVert * 2 * sizeof(float) );

                    free(p_tp->p_vertex);
                    p_tp->p_vertex = (double *)p_run;

                    p_run += p_tp->nVert * 2;

                    p_tp = p_tp->p_next; // pick up the next in chain
                }
            }


            ppg_vbo->bsingle_alloc = true;
            ppg_vbo->single_buffer = (unsigned char *)vbuf;
            ppg_vbo->single_buffer_size = total_byte_size;
            ppg_vbo->data_type = DATA_TYPE_FLOAT;

        }


        if( b_useVBO ){
            //  Has a VBO been built for this object?
            if( 1 ) {

                if(rzRules->obj->auxParm0 <= 0) {
                    b_temp_vbo = (rzRules->obj->auxParm0 == -5);   // Must we use a temporary VBO?  Probably slower than simple glDrawArrays

                    GLuint vboId;
                    // generate a new VBO and get the associated ID
                    (s_glGenBuffers)(1, &vboId);
                    
                    rzRules->obj->auxParm0 = vboId;
                    
                    // bind VBO in order to use
                    (s_glBindBuffer)(GL_ARRAY_BUFFER, vboId);
                    GLenum err = glGetError();
                    if(err){
                        qDebug("VBO Error A: %d", err);
                        return 0;
                    }
                    
                    // upload data to VBO
                    glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
                    (s_glBufferData)(GL_ARRAY_BUFFER,
                                     ppg_vbo->single_buffer_size, ppg_vbo->single_buffer, GL_STATIC_DRAW);
                    err = glGetError();
                    if(err){
                        qDebug("VBO Error B: %d", err);
                        return 0;
                    }
                    
                }
                else {
                    (s_glBindBuffer)(GL_ARRAY_BUFFER, rzRules->obj->auxParm0);
                    GLenum err = glGetError();
                    if(err){
                        qDebug("VBO Error C: %d", err);
                        return 0;
                    }
                    
                    glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
                }
            }
        }

        

        PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

        TriPrim *p_tp = ppg->tri_prim_head;
        GLintptr vbo_offset = 0;
        
        glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array

        //      Set up the stride sizes for the array
        int array_data_size = sizeof(float);
        GLint array_gl_type = GL_FLOAT;
        
        if(ppg->data_type == DATA_TYPE_DOUBLE){
            array_data_size = sizeof(double);
            array_gl_type = GL_DOUBLE;
        }

        while( p_tp ) {
            LLBBox box;
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
            }
            else
                box = p_tp->tri_box;
            
            if(!BBView.IntersectOut(box)) {
                if(b_useVBO) {
                    glVertexPointer(2, array_gl_type, 2 * array_data_size, (GLvoid *)(vbo_offset));
                    glDrawArrays(p_tp->type, 0, p_tp->nVert);
                }
                else {
                    if(vp->projectType() == PROJECTION_MERCATOR) {
                        glVertexPointer(2, array_gl_type, 2 * array_data_size, p_tp->p_vertex);
                        glDrawArrays(p_tp->type, 0, p_tp->nVert);
                    } else {
                        // temporary slow hack
                        glDisableClientState(GL_VERTEX_ARRAY);

                        glBegin(p_tp->type);
                        float *pvert_list = (float *)p_tp->p_vertex;
                        for(int i=0; i<p_tp->nVert; i++) {
                            float lon = *pvert_list++;
                            float lat = *pvert_list++;
                            zchxPoint r;
                            GetPointPixSingle(rzRules, lat, lon, &r, vp);

                            if(r.x != INVALID_COORD)
                                glVertex2i(r.x, r.y);
                            else if(p_tp->type != GL_TRIANGLE_FAN) {
                                glEnd();
                                glBegin(p_tp->type);
                                if(p_tp->type == GL_TRIANGLES)
                                    while(i%3 < 2) i++;
                            }
                        }
                        glEnd();

                    }
                }
            }
            
            vbo_offset += p_tp->nVert * 2 * array_data_size;
            
            // pick up the next in chain
            if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
                LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
                p_tp = (TriPrim *)p_ltp->p_next;
            }
            else
                p_tp = p_tp->p_next;
            
        } // while
        
        if(b_useVBO)
            (s_glBindBuffer)(GL_ARRAY_BUFFER_ARB, 0);
        
        glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
        
        if(b_transform)
            glPopMatrix();
        
        if( b_useVBO && b_temp_vbo){
            (s_glBufferData)(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
            s_glDeleteBuffers(1, (unsigned int *)&rzRules->obj->auxParm0);
            rzRules->obj->auxParm0 = 0;
        }
    } // if pPolyTessGeo


#endif          //#ifdef ocpnUSE_GL

    return 1;
}

void s52plib::SetGLClipRect(const ViewPort &vp, const QRect &rect)
{
    bool b_clear = false;
    bool s_b_useStencil = m_useStencil;
    
#if 0
    /* for some reason this causes an occasional bug in depth mode, I cannot
     *       seem to solve it yet, so for now: */
    if(s_b_useStencil && s_b_useScissorTest) {
        QRect vp_rect(0, 0, vp.pix_width, vp.pix_height);
        if(rect != vp_rect) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(rect.x, cc1->m_canvas_height-rect.height-rect.y, rect.width, rect.height);
        }

        if(b_clear) {
            glBegin(GL_QUADS);
            glVertex2i( rect.x, rect.y );
            glVertex2i( rect.x + rect.width, rect.y );
            glVertex2i( rect.x + rect.width, rect.y + rect.height );
            glVertex2i( rect.x, rect.y + rect.height );
            glEnd();
        }

        /* the code in s52plib depends on the depth buffer being
 *           initialized to this value, this code should go there instead and
 *           only a flag set here. */
        if(!s_b_useStencil) {
            glClearDepth( 0.25 );
            glDepthMask( GL_TRUE );    // to allow writes to the depth buffer
            glClear( GL_DEPTH_BUFFER_BIT );
            glDepthMask( GL_FALSE );
            glClearDepth( 1 ); // set back to default of 1
            glDepthFunc( GL_GREATER );                          // Set the test value
        }
        return;
    }
#endif
    // slower way if there is no scissor support
    if(!b_clear)
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );   // disable color buffer
    
    if( s_b_useStencil ) {
        //    Create a stencil buffer for clipping to the region
        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x1 );                 // write only into bit 0 of the stencil buffer
        glClear( GL_STENCIL_BUFFER_BIT );
        
        //    We are going to write "1" into the stencil buffer wherever the region is valid
        glStencilFunc( GL_ALWAYS, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
    } else              //  Use depth buffer for clipping
    {
        glEnable( GL_DEPTH_TEST ); // to enable writing to the depth buffer
        glDepthFunc( GL_ALWAYS );  // to ensure everything you draw passes
        glDepthMask( GL_TRUE );    // to allow writes to the depth buffer
        
        glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start
        
        //    Decompose the region into rectangles, and draw as quads
        //    With z = 1
        // dep buffer clear = 1
        // 1 makes 0 in dep buffer, works
        // 0 make .5 in depth buffer
        // -1 makes 1 in dep buffer
        
        //    Depth buffer runs from 0 at z = 1 to 1 at z = -1
        //    Draw the clip geometry at z = 0.5, giving a depth buffer value of 0.25
        //    Subsequent drawing at z=0 (depth = 0.5) will pass if using glDepthFunc(GL_GREATER);
        glTranslatef( 0, 0, .5 );
    }
    
    glBegin(GL_QUADS);
    glVertex2i( rect.x(), rect.y() );
    glVertex2i( rect.x() + rect.width(), rect.y() );
    glVertex2i( rect.x() + rect.width(), rect.y() + rect.height() );
    glVertex2i( rect.x(), rect.y() + rect.height() );
    glEnd();
    
    if( s_b_useStencil ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "1"
        glStencilFunc( GL_EQUAL, 1, 1 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_GREATER );                          // Set the test value
        glDepthMask( GL_FALSE );                            // disable depth buffer
        glTranslatef( 0, 0, -.5 ); // reset translation
    }
    
    if(!b_clear)
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );  // re-enable color buffer
}

void RotateToViewPort(const ViewPort &vp)
{
    bool g_bskew_comp = true;
    
    float angle = vp.rotation();
    if(g_bskew_comp)
        angle -= vp.skew();
    
    if( fabs( angle ) > 0.0001 )
    {
        //    Rotations occur around 0,0, so translate to rotate around screen center
        float xt = vp.pixWidth() / 2.0, yt = vp.pixHeight() / 2.0;
        
        glTranslatef( xt, yt, 0 );
        glRotatef( angle * 180. / PI, 0, 0, 1 );
        glTranslatef( -xt, -yt, 0 );
    }
}



int s52plib::RenderToGLAP( ObjRazRules *rzRules, Rules *rules, ViewPort *vp )
{
#ifdef ocpnUSE_GL
    if( rules->razRule == NULL )
        return 0;

    int obj_xmin = 10000;
    int obj_xmax = -10000;
    int obj_ymin = 10000;
    int obj_ymax = -10000;

    double z_clip_geom = 1.0;
    double z_tex_geom = 0.;

    GLuint clip_list = 0;

    LLBBox BBView = vp->getBBox();

    zchxPoint *ptp;
    if( rzRules->obj->pPolyTessGeo ) {
        if( !rzRules->obj->pPolyTessGeo->IsOk() ){ // perform deferred tesselation
            rzRules->obj->pPolyTessGeo->BuildDeferredTess();
        }

        ptp = (zchxPoint *) malloc(
                    ( rzRules->obj->pPolyTessGeo->GetnVertexMax() + 1 ) * sizeof(zchxPoint) );
    } else
        return 0;

    if( m_useStencilAP ) {
        glPushAttrib( GL_STENCIL_BUFFER_BIT );          // See comment below
        //    Use masked bit "1" of the stencil buffer to create a stencil for the area of interest

        glEnable( GL_STENCIL_TEST );
        glStencilMask( 0x2 ); // write only into bit 1 of the stencil buffer
        glColorMask( false, false, false, false ); // Disable writing to the color buffer
        glClear( GL_STENCIL_BUFFER_BIT );

        //    We are going to write "2" into the stencil buffer wherever the object is valid
        glStencilFunc( GL_ALWAYS, 2, 2 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

    } else {
        glEnable( GL_DEPTH_TEST ); // to use the depth test
        glDepthFunc( GL_GREATER ); // Respect global render mask in depth buffer
        glDepthMask( GL_TRUE ); // to allow writes to the depth buffer
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE ); // disable color buffer

        glColor3f( 1, 1, 0 );

        //  If we are using stencil for overall clipping, then we are only
        //  using depth buffer for AreaPattern rendering
        //  So, each AP render can start with a clear depth buffer
        
        if(m_useStencil){
            glClearDepth(.26);
            glClear( GL_DEPTH_BUFFER_BIT ); // for a fresh start
        }
        //    Overall chart clip buffer was set at z=0.5
        //    Draw this clip geometry at z = .25, so still respecting the previously established clip region
        //    Subsequent drawing to this area at z=.25  will pass only this area if using glDepthFunc(GL_EQUAL);

        z_clip_geom = .25;
        z_tex_geom = .25;
    }

    PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();

    TriPrim *p_tp = ppg->tri_prim_head;
    while( p_tp ) {
        LLBBox box;
        if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
            LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
            box.Set(p_ltp->miny, p_ltp->minx, p_ltp->maxy, p_ltp->maxx);
        }
        else
            box = p_tp->tri_box;

        if(!BBView.IntersectOut(box)) {
            //      Get and convert the points

            zchxPoint *pr = ptp;
            if( ppg->data_type == DATA_TYPE_FLOAT ){
                float *pvert_list = (float *)p_tp->p_vertex;

                for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                    float lon = *pvert_list++;
                    float lat = *pvert_list++;
                    GetPointPixSingle(rzRules, lat, lon, pr, vp );

                    obj_xmin = qMin(obj_xmin, pr->x);
                    obj_xmax = qMax(obj_xmax, pr->x);
                    obj_ymin = qMin(obj_ymin, pr->y);
                    obj_ymax = qMax(obj_ymax, pr->y);

                    pr++;
                }
            }
            else {
                double *pvert_list = p_tp->p_vertex;

                for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                    double lon = *pvert_list++;
                    double lat = *pvert_list++;
                    GetPointPixSingle(rzRules, lat, lon, pr, vp );

                    obj_xmin = qMin(obj_xmin, pr->x);
                    obj_xmax = qMax(obj_xmax, pr->x);
                    obj_ymin = qMin(obj_ymin, pr->y);
                    obj_ymax = qMax(obj_ymax, pr->y);

                    pr++;
                }
            }


            switch( p_tp->type ){
            case PTG_TRIANGLE_FAN: {
                glBegin( GL_TRIANGLE_FAN );
                for( int it = 0; it < p_tp->nVert; it++ )
                    glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                glEnd();
                break;
            }

            case PTG_TRIANGLE_STRIP: {
                glBegin( GL_TRIANGLE_STRIP );
                for( int it = 0; it < p_tp->nVert; it++ )
                    glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                glEnd();
                break;
            }
            case PTG_TRIANGLES: {
                glBegin( GL_TRIANGLES );
                for( int it = 0; it < p_tp->nVert; it += 3 ) {
                    int xmin = qMin(ptp[it].x, qMin(ptp[it+1].x, ptp[it+2].x));
                    int xmax = qMax(ptp[it].x, qMax(ptp[it+1].x, ptp[it+2].x));
                    int ymin = qMin(ptp[it].y, qMin(ptp[it+1].y, ptp[it+2].y));
                    int ymax = qMax(ptp[it].y, qMax(ptp[it+1].y, ptp[it+2].y));

                    QRect rect( xmin, ymin, xmax - xmin, ymax - ymin );
                    //if( rect.Intersects( m_render_rect ) )
                    {
                        glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                        glVertex3f( ptp[it + 1].x, ptp[it + 1].y, z_clip_geom );
                        glVertex3f( ptp[it + 2].x, ptp[it + 2].y, z_clip_geom );
                    }
                }
                glEnd();
                break;
            }
            }
        } // if bbox

        // pick up the next in chain
        if(!rzRules->obj->m_chart_context->chart) {          // This is a PlugIn Chart
            LegacyTriPrim *p_ltp = (LegacyTriPrim *)p_tp;
            p_tp = (TriPrim *)p_ltp->p_next;
        }
        else
            p_tp = p_tp->p_next;

    } // while

    //        obj_xmin = 0;
    //        obj_xmax = 2000;

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ); // re-enable color buffer

    if( m_useStencilAP ) {
        //    Now set the stencil ops to subsequently render only where the stencil bit is "2"
        glStencilFunc( GL_EQUAL, 2, 2 );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    } else {
        glDepthFunc( GL_EQUAL ); // Set the test value
        glDepthMask( GL_FALSE ); // disable depth buffer
    }
    //    Get the pattern definition
    if( ( rules->razRule->pixelPtr == NULL ) || ( rules->razRule->parm1 != m_colortable_index )
            || ( rules->razRule->parm0 != ID_GL_PATT_SPEC ) ) {
        render_canvas_parms *patt_spec = CreatePatternBufferSpec( rzRules, rules, vp, false,
                                                                  true );

        ClearRulesCache( rules->razRule ); //  Clear out any existing cached symbology

        rules->razRule->pixelPtr = patt_spec;
        rules->razRule->parm1 = m_colortable_index;
        rules->razRule->parm0 = ID_GL_PATT_SPEC;
    }

    //  Render the Area using the pattern spec stored in the rules
    render_canvas_parms *ppatt_spec = (render_canvas_parms *) rules->razRule->pixelPtr;

    //    Has the pattern been uploaded as a texture?
    if( !ppatt_spec->OGL_tex_name ) {
        GLuint tex_name;
        glGenTextures( 1, &tex_name );
        ppatt_spec->OGL_tex_name = tex_name;

        glBindTexture( GL_TEXTURE_2D, tex_name );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ppatt_spec->w_pot, ppatt_spec->h_pot, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, ppatt_spec->pix_buff );
    }

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, ppatt_spec->OGL_tex_name );

    glEnable( GL_BLEND );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    int h = ppatt_spec->height;
    int w = ppatt_spec->width;
    int xr = obj_xmin;
    int yr = obj_ymin;

    float ww = (float) ppatt_spec->width / (float) ppatt_spec->w_pot;
    float hh = (float) ppatt_spec->height / (float) ppatt_spec->h_pot;
    float x_stagger_off = 0;
    if( ppatt_spec->b_stagger ) x_stagger_off = (float) ppatt_spec->width / 2;
    int yc = 0;


    if(w>0 && h>0) {
        while( yr < vp->pixHeight() ) {
            if( ( (yr + h) >= 0 ) && ( yr <= obj_ymax ) )  {
                xr = obj_xmin;   //reset
                while( xr < vp->pixWidth() ) {
                    
                    int xp = xr;
                    if( yc & 1 ) xp += x_stagger_off;
                    
                    //    Render a quad.
                    if( ( (xr + w) >= 0 ) && ( xr <= obj_xmax ) ){
                        glBegin( GL_QUADS );
                        glTexCoord2f( 0, 0 );
                        glVertex3f( xp, yr, z_tex_geom );
                        glTexCoord2f( ww, 0 );
                        glVertex3f( xp + w, yr, z_tex_geom );
                        glTexCoord2f( ww, hh );
                        glVertex3f( xp + w, yr + h, z_tex_geom );
                        glTexCoord2f( 0, hh );
                        glVertex3f( xp, yr + h, z_tex_geom );
                        glEnd();
                    }
                    xr += ppatt_spec->width;
                }
            }
            yr += ppatt_spec->height;
            yc++;
        }
    }

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );

    //    Restore the previous state

    if( m_useStencilAP ){
        //  Theoretically, it should be sufficient to simply reset the StencilFunc()...
        //  But I found one platform where this does not work, and we need to save and restore
        //  the entire STENCIL state.  I suspect bad GL drivers here, but we do what must needs...
        //glStencilFunc( GL_EQUAL, 1, 1 );

        glPopAttrib();
    }
    else {
        // restore clipping region
        glPopMatrix();
        SetGLClipRect( *vp, m_last_clip_rect);

        glPushMatrix();
        RotateToViewPort(*vp);
        glDisable( GL_DEPTH_TEST );

    }


    free( ptp );
#endif                  //#ifdef ocpnUSE_GL
    
    return 1;
}

void s52plib::RenderPolytessGL(ObjRazRules *rzRules, ViewPort *vp, double z_clip_geom, zchxPoint *ptp)
{
#ifdef ocpnUSE_GL

    LLBBox BBView = vp->getBBox();

    //  Allow a little slop in calculating whether a triangle
    //  is within the requested Viewport
    double margin = BBView.GetLonRange() * .05;

    int obj_xmin = 10000;
    int obj_xmax = -10000;
    int obj_ymin = 10000;
    int obj_ymax = -10000;
    
    PolyTriGroup *ppg = rzRules->obj->pPolyTessGeo->Get_PolyTriGroup_head();
    
    TriPrim *p_tp = ppg->tri_prim_head;
    while( p_tp ) {
        if(!BBView.IntersectOut( p_tp->tri_box)) {
            //      Get and convert the points
            
            zchxPoint *pr = ptp;
            if( ppg->data_type == DATA_TYPE_FLOAT ){
                float *pvert_list = (float *)p_tp->p_vertex;
                
                for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                    float lon = *pvert_list++;
                    float lat = *pvert_list++;
                    GetPointPixSingle(rzRules, lat, lon, pr, vp );
                    
                    obj_xmin = qMin(obj_xmin, pr->x);
                    obj_xmax = qMax(obj_xmax, pr->x);
                    obj_ymin = qMin(obj_ymin, pr->y);
                    obj_ymax = qMax(obj_ymax, pr->y);
                    
                    pr++;
                }
            }
            else {
                double *pvert_list = p_tp->p_vertex;
                
                for( int iv = 0; iv < p_tp->nVert; iv++ ) {
                    double lon = *pvert_list++;
                    double lat = *pvert_list++;
                    GetPointPixSingle(rzRules, lat, lon, pr, vp );
                    
                    obj_xmin = qMin(obj_xmin, pr->x);
                    obj_xmax = qMax(obj_xmax, pr->x);
                    obj_ymin = qMin(obj_ymin, pr->y);
                    obj_ymax = qMax(obj_ymax, pr->y);
                    
                    pr++;
                }
            }
            
            
            
            switch( p_tp->type ){
            case PTG_TRIANGLE_FAN: {
                glBegin( GL_TRIANGLE_FAN );
                for( int it = 0; it < p_tp->nVert; it++ )
                    glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                glEnd();
                break;
            }
                
            case PTG_TRIANGLE_STRIP: {
                glBegin( GL_TRIANGLE_STRIP );
                for( int it = 0; it < p_tp->nVert; it++ )
                    glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                glEnd();
                break;
            }
            case PTG_TRIANGLES: {
                glBegin( GL_TRIANGLES );
                for( int it = 0; it < p_tp->nVert; it += 3 ) {
                    int xmin = qMin(ptp[it].x, qMin(ptp[it+1].x, ptp[it+2].x));
                    int xmax = qMax(ptp[it].x, qMax(ptp[it+1].x, ptp[it+2].x));
                    int ymin = qMin(ptp[it].y, qMin(ptp[it+1].y, ptp[it+2].y));
                    int ymax = qMax(ptp[it].y, qMax(ptp[it+1].y, ptp[it+2].y));

                    QRect rect( xmin, ymin, xmax - xmin, ymax - ymin );
                    //                        if( rect.Intersects( m_render_rect ) )
                    {
                        glVertex3f( ptp[it].x, ptp[it].y, z_clip_geom );
                        glVertex3f( ptp[it + 1].x, ptp[it + 1].y, z_clip_geom );
                        glVertex3f( ptp[it + 2].x, ptp[it + 2].y, z_clip_geom );
                    }
                }
                glEnd();
                break;
            }
            }
        } // if bbox
        p_tp = p_tp->p_next; // pick up the next in chain
    } // while
    
#endif    
}

#ifdef ocpnUSE_GL

int s52plib::RenderAreaToGL(QGLContext *glcc, ObjRazRules *rzRules, ViewPort *vp )
{
    if( !ObjectRenderCheckRules( rzRules, vp, true ) )
        return 0;

    Rules *rules = rzRules->LUP->ruleList;

    while( rules != NULL ) {
        switch( rules->ruleType ){
        case RUL_ARE_CO:
            RenderToGLAC( rzRules, rules, vp );
            break; // AC

        case RUL_ARE_PA:
            RenderToGLAP( rzRules, rules, vp );
            break; // AP

        case RUL_CND_SY: {
            if( !rzRules->obj->bCS_Added ) {
                rzRules->obj->CSrules = NULL;
                GetAndAddCSRules( rzRules, rules );
                rzRules->obj->bCS_Added = 1; // mark the object
            }
            Rules *rules_last = rules;
            rules = rzRules->obj->CSrules;

            while( NULL != rules ) {
                switch( rules->ruleType ){
                case RUL_ARE_CO:
                    RenderToGLAC( rzRules, rules, vp );
                    break;
                case RUL_ARE_PA:
                    RenderToGLAP( rzRules, rules, vp );
                    break;
                case RUL_NONE:
                default:
                    break; // no rule type (init)
                }
                rules_last = rules;
                rules = rules->next;
            }

            rules = rules_last;
            break;
        }

        case RUL_NONE:
        default:
            break; // no rule type (init)
        } // switch

        rules = rules->next;
    }

    return 1;

}
#endif

render_canvas_parms* s52plib::CreatePatternBufferSpec( ObjRazRules *rzRules, Rules *rules,
                                                       ViewPort *vp, bool b_revrgb, bool b_pot )
{
    QImage Image;
    
    Rule *prule = rules->razRule;
    
    bool bstagger_pattern = ( prule->fillType.PATP == 'S' );
    
    QColor local_unused_QColor = m_unused_QColor;
    
    //      Create a wxImage of the pattern drawn on an "unused_color" field
    if( prule->definition.SYDF == 'R' ) {
        Image = useLegacyRaster ?
                    RuleXBMToImage( prule ) : ChartSymbols::GetImage( prule->name.PANM );
    }
    
    else          // Vector
    {
        float fsf = 100 / canvas_pix_per_mm;
        
        // Base bounding box
        wxBoundingBox box( prule->pos.patt.bnbox_x.PBXC, prule->pos.patt.bnbox_y.PBXR,
                           prule->pos.patt.bnbox_x.PBXC + prule->pos.patt.bnbox_w.PAHL,
                           prule->pos.patt.bnbox_y.PBXR + prule->pos.patt.bnbox_h.PAVL );
        
        // Expand to include pivot
        box.Expand( prule->pos.patt.pivot_x.PACL, prule->pos.patt.pivot_y.PARW );
        
        //    Pattern bounding boxes may be offset from origin, to preset the spacing
        //    So, the bitmap must be delta based.
        double dwidth = box.GetWidth();
        double dheight = box.GetHeight();
        
        //  Add in the pattern spacing parameters
        dwidth += prule->pos.patt.minDist.PAMI;
        dheight += prule->pos.patt.minDist.PAMI;
        
        //  Prescale
        dwidth /= fsf;
        dheight /= fsf;
        
        int width = (int) dwidth + 1;
        int height = (int) dheight + 1;
        
        //      Instantiate the vector pattern to a wxBitmap
        QPainter mdc;
        
        //  TODO
        // This ought to work for wxOSX, but DOES NOT.
        // We we do not want anti-aliased lines drawn in the pattern spec, since we used the solid primary color
        // as a mask for manual blitting from the dc to memory buffer.
        
        // Best we can do is set the background color very dark, and hope for the best

        QBitmap *pbm = NULL;

        if( ( 0 != width ) && ( 0 != height ) )
        {
            pbm = new QBitmap( width, height );

            mdc.begin(pbm );
            mdc.setBackground( QBrush( local_unused_QColor ) );
            //            mdc.Clear();

            int pivot_x = prule->pos.patt.pivot_x.PACL;
            int pivot_y = prule->pos.patt.pivot_y.PARW;

            char *str = prule->vector.LVCT;
            char *col = prule->colRef.LCRF;
            zchxPoint pivot( pivot_x, pivot_y );

            int origin_x = prule->pos.patt.bnbox_x.PBXC;
            int origin_y = prule->pos.patt.bnbox_y.PBXR;
            zchxPoint origin(origin_x, origin_y);

            zchxPoint r0( (int) ( ( pivot_x - box.GetMinX() ) / fsf ) + 1,
                          (int) ( ( pivot_y - box.GetMinY() ) / fsf ) + 1 );

            HPGL->SetTargetDC( &mdc );
            HPGL->Render( str, col, r0, pivot, origin, 1.0, 0, false);
        } else
        {
            pbm = new QBitmap( 2, 2 );       // substitute small, blank pattern
            mdc.begin(pbm );
            mdc.setBackground( QBrush( local_unused_QColor ) );
            //            mdc.Clear();
        }

        //        mdc.SelectObject( wxNullBitmap );
        mdc.end();

        //    Build a wxImage from the wxBitmap
        Image = pbm->toImage();

        delete pbm;
    }
    
    //  Convert the wxImage to a populated render_canvas_parms struct
    
    int sizey = Image.width();
    int sizex = Image.height();
    
    render_canvas_parms *patt_spec = new render_canvas_parms;
    patt_spec->OGL_tex_name = 0;
    
    if( b_pot ) {
        int xp = sizex;
        int a = 0;
        while( xp ) {
            xp = xp >> 1;
            a++;
        }
        patt_spec->w_pot = 1 << a;
        
        xp = sizey;
        a = 0;
        while( xp ) {
            xp = xp >> 1;
            a++;
        }
        patt_spec->h_pot = 1 << a;
        
    } else {
        patt_spec->w_pot = sizex;
        patt_spec->h_pot = sizey;
    }
    
    patt_spec->depth = 32;             // set the depth, always 32 bit
    
    patt_spec->pb_pitch = ( ( patt_spec->w_pot * patt_spec->depth / 8 ) );
    patt_spec->lclip = 0;
    patt_spec->rclip = patt_spec->w_pot - 1;
    patt_spec->pix_buff = (unsigned char *) malloc( patt_spec->h_pot * patt_spec->pb_pitch );
    
    // Preset background
    memset( patt_spec->pix_buff, 0, sizey * patt_spec->pb_pitch );
    patt_spec->width = sizex;
    patt_spec->height = sizey;
    patt_spec->x = 0;
    patt_spec->y = 0;
    patt_spec->b_stagger = bstagger_pattern;
    
    unsigned char *pd0 = patt_spec->pix_buff;
    unsigned char *pd;
    unsigned char *ps0 = Image.bits();
    unsigned char *imgAlpha = NULL;
    bool b_use_alpha = false;
    if( Image.hasAlphaChannel() ) {
        imgAlpha = Image.alphaChannel().bits();
        b_use_alpha = true;
    }
    
#if defined(__WXMAC__) || defined(__WXQT__)
    
    if( prule->definition.SYDF == 'V' ) {
        b_use_alpha = true;
        imgAlpha = NULL;
    }
#endif
    
    unsigned char primary_r = 0;
    unsigned char primary_g = 0;
    unsigned char primary_b = 0;
    double reference_value = 0.5;
    
    bool b_filter = false;
#if defined(__WXMAC__)
    S52color *primary_color = 0;
    if( prule->definition.SYDF == 'V' ){
        b_filter = true;
        char *col = prule->colRef.LCRF;
        primary_color = getColor( col+1);
        if(primary_color){
            primary_r = primary_color->R;
            primary_g = primary_color->G;
            primary_b = primary_color->B;
            wxImage::RGBValue rgb(primary_r,primary_g,primary_b);
            wxImage::HSVValue hsv = wxImage::RGBtoHSV( rgb );
            reference_value = hsv.value;
        }
    }
#endif
    
    unsigned char *ps;
    
    {
        unsigned char mr = local_unused_QColor.red();
        unsigned char mg = local_unused_QColor.green();
        unsigned char mb = local_unused_QColor.blue();
        
        if( pd0 && ps0 ){
            for( int iy = 0; iy < sizey; iy++ ) {
                pd = pd0 + ( iy * patt_spec->pb_pitch );
                ps = ps0 + ( iy * sizex * 3 );
                for( int ix = 0; ix < sizex; ix++ ) {
                    if( ix < sizex ) {
                        unsigned char r = *ps++;
                        unsigned char g = *ps++;
                        unsigned char b = *ps++;
                        
                        if(b_filter){
                            QColor rgb(r,g,b);
                            int h, s, v;
                            rgb.getHsv(&h, &s, &v);
                            double ratio = v/reference_value;
                            
                            if(ratio > 0.5){
                                *pd++ = primary_r;
                                *pd++ = primary_g;
                                *pd++ = primary_b;
                                *pd++ = 255;
                            }
                            else{
                                *pd++ = 0;
                                *pd++ = 0;
                                *pd++ = 0;
                                *pd++ = 0;
                            }
                        }
                        else{
#ifdef ocpnUSE_ocpnBitmap
                            if( b_revrgb ) {
                                *pd++ = b;
                                *pd++ = g;
                                *pd++ = r;
                            } else {
                                *pd++ = r;
                                *pd++ = g;
                                *pd++ = b;
                            }
                            
#else
                            *pd++ = r;
                            *pd++ = g;
                            *pd++ = b;
#endif
                            if( b_use_alpha && imgAlpha ) {
                                *pd++ = *imgAlpha++;
                            } else {
                                *pd++ = ( ( r == mr ) && ( g == mg ) && ( b == mb ) ? 0 : 255 );
                            }
                        }
                    }
                }
            }
        }
    }
    
    return patt_spec;
    
}


int s52plib::RenderToBufferAP( ObjRazRules *rzRules, Rules *rules, ViewPort *vp,
                               render_canvas_parms *pb_spec )
{
    if(vp->projectType() != PROJECTION_MERCATOR)
        return 1;

    QImage Image;

    if( rules->razRule == NULL )
        return 0;
    
    if( ( rules->razRule->pixelPtr == NULL ) || ( rules->razRule->parm1 != m_colortable_index )
            || ( rules->razRule->parm0 != ID_RGB_PATT_SPEC ) ) {
        render_canvas_parms *patt_spec = CreatePatternBufferSpec( rzRules, rules, vp, true );

        ClearRulesCache( rules->razRule ); //  Clear out any existing cached symbology

        rules->razRule->pixelPtr = patt_spec;
        rules->razRule->parm1 = m_colortable_index;
        rules->razRule->parm0 = ID_RGB_PATT_SPEC;

    } // Instantiation done

    //  Render the Area using the pattern spec stored in the rules
    render_canvas_parms *ppatt_spec = (render_canvas_parms *) rules->razRule->pixelPtr;

    //  Set the pattern reference point

    zchxPoint r;
    GetPointPixSingle( rzRules, rzRules->obj->y, rzRules->obj->x, &r, vp );

    ppatt_spec->x = r.x - 2000000; // bias way down to avoid zero-crossing logic in dda
    ppatt_spec->y = r.y - 2000000;

    RenderToBufferFilledPolygon( rzRules, rzRules->obj, NULL, pb_spec, ppatt_spec, vp );

    return 1;
}

int s52plib::RenderToBufferAC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp,
                               render_canvas_parms *pb_spec )
{
    if(vp->projectType() != PROJECTION_MERCATOR)
        return 1;

    S52color *c;
    char *str = (char*) rules->INSTstr;

    c = getColor( str );

    RenderToBufferFilledPolygon( rzRules, rzRules->obj, c, pb_spec, NULL, vp );

    //    At very small scales, the object could be visible on both the left and right sides of the screen.
    //    Identify this case......
    if( vp->chartScale() > 5e7 ) {
        //    Does the object hang out over the left side of the VP?
        if( ( rzRules->obj->BBObj.GetMaxLon() > vp->getBBox().GetMinLon() )
                && ( rzRules->obj->BBObj.GetMinLon() < vp->getBBox().GetMinLon() ) ) {
            //    If we add 360 to the objects lons, does it intersect the the right side of the VP?
            if( ( ( rzRules->obj->BBObj.GetMaxLon() + 360. ) > vp->getBBox().GetMaxLon() )
                    && ( ( rzRules->obj->BBObj.GetMinLon() + 360. ) < vp->getBBox().GetMaxLon() ) ) {
                //  If so, this area oject should be drawn again, this time for the left side
                //    Do this by temporarily adjusting the objects rendering offset
                rzRules->obj->x_origin -= mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
                RenderToBufferFilledPolygon( rzRules, rzRules->obj, c, pb_spec,
                                             NULL, vp );
                rzRules->obj->x_origin += mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;

            }
        }
    }

    return 1;
}

int s52plib::RenderAreaToDC(ObjRazRules *rzRules, ViewPort *vp,
                             render_canvas_parms *pb_spec )
{

    if( !ObjectRenderCheckRules( rzRules, vp, true ) )
        return 0;
    Rules *rules = rzRules->LUP->ruleList;

    while( rules != NULL ) {
        switch( rules->ruleType ){
        case RUL_ARE_CO:
            RenderToBufferAC( rzRules, rules, vp, pb_spec );
            break; // AC
        case RUL_ARE_PA:
            RenderToBufferAP( rzRules, rules, vp, pb_spec );
            break; // AP

        case RUL_CND_SY: {
            if( !rzRules->obj->bCS_Added ) {
                rzRules->obj->CSrules = NULL;
                GetAndAddCSRules( rzRules, rules );
                rzRules->obj->bCS_Added = 1; // mark the object
            }
            Rules *rules_last = rules;
            rules = rzRules->obj->CSrules;

            //    The CS procedure may have changed the Display Category of the Object, need to check again for visibility
            if( ObjectRenderCheckCat( rzRules, vp ) ) {
                while( NULL != rules ) {
                    //Hve seen drgare fault here, need to code area query to debug
                    //possible that RENDERtoBUFFERAP/AC is blowing obj->CSRules
                    //    When it faults here, look at new debug field obj->CSLUP
                    switch( rules->ruleType ){
                    case RUL_ARE_CO:
                        RenderToBufferAC( rzRules, rules, vp, pb_spec );
                        break;
                    case RUL_ARE_PA:
                        RenderToBufferAP( rzRules, rules, vp, pb_spec );
                        break;
                    case RUL_NONE:
                    default:
                        break; // no rule type (init)
                    }
                    rules_last = rules;
                    rules = rules->next;
                }
            }

            rules = rules_last;
            break;
        }

        case RUL_NONE:
        default:
            break; // no rule type (init)
        } // switch

        rules = rules->next;
    }

    return 1;

}

void s52plib::GetAndAddCSRules( ObjRazRules *rzRules, Rules *rules )
{

    LUPrec *NewLUP;
    LUPrec *LUP;
    LUPrec *LUPCandidate;

    char *rule_str1 = RenderCS( rzRules, rules );
    QString cs_string = QString::fromUtf8(rule_str1);
    free( rule_str1 ); //delete rule_str1;

    //  Try to find a match for this object/attribute set in dynamic CS LUP Table

    //  Do this by checking each LUP in the CS LUPARRAY and checking....
    //  a) is Object Name the same? and
    //  b) was LUP created earlier by exactly the same INSTruction string?
    //  c) does LUP have same Display Category and Priority?

    wxArrayOfLUPrec *la = condSymbolLUPArray;
    int index = 0;
    int index_max = la->count();
    LUP = NULL;

    while( ( index < index_max ) ) {
        LUPCandidate = la->at( index );
        if( !strcmp( rzRules->LUP->OBCL, LUPCandidate->OBCL ) ) {
            if( *(LUPCandidate->INST) ==  cs_string  ) {
                if( LUPCandidate->DISC == rzRules->LUP->DISC ) {
                    LUP = LUPCandidate;
                    break;
                }
            }
        }
        index++;
    }

    //  If not found, need to create a dynamic LUP and add to CS LUP Table

    if( NULL == LUP ) // Not found
    {

        NewLUP = (LUPrec*) calloc( 1, sizeof(LUPrec) );
        pAlloc->append( NewLUP );

        NewLUP->DISC = rzRules->LUP->DISC; // as a default

        //sscanf(pBuf+11, "%d", &LUP->RCID);

        memcpy( NewLUP->OBCL, rzRules->LUP->OBCL, 6 ); // the object class name

        //      Add the complete CS string to the LUP
        QString *pINST = new QString( cs_string );
        NewLUP->INST = pINST;

        _LUP2rules( NewLUP, rzRules->obj );

        // Add LUP to array
        wxArrayOfLUPrec *pLUPARRAYtyped = condSymbolLUPArray;

        pLUPARRAYtyped->append( NewLUP );

        LUP = NewLUP;

    } // if (LUP = NULL)

    Rules *top = LUP->ruleList;

    rzRules->obj->CSrules = top; // patch in a new rule set

}

bool s52plib::ObjectRenderCheck( ObjRazRules *rzRules, ViewPort *vp )
{
    if( !ObjectRenderCheckPos( rzRules, vp ) ) return false;

    if( !ObjectRenderCheckCat( rzRules, vp ) ) return false;

    return true;
}

bool s52plib::ObjectRenderCheckCS( ObjRazRules *rzRules, ViewPort *vp )
{
    //  We need to do this test since some CS procedures change the display category
    //  So we need to tentatively process all objects with CS LUPs
    Rules *rules = rzRules->LUP->ruleList;
    while( rules != NULL ) {
        if( RUL_CND_SY == rules->ruleType ) return true;

        rules = rules->next;
    }

    return false;
}

bool s52plib::ObjectRenderCheckPos( ObjRazRules *rzRules, ViewPort *vp )
{
    if( rzRules->obj == NULL )
        return false;

    // Of course, the object must be at least partly visible in the viewport
    const LLBBox &vpBox = vp->getBBox(), &testBox = rzRules->obj->BBObj;

    if(vpBox.GetMaxLat() < testBox.GetMinLat() || vpBox.GetMinLat() > testBox.GetMaxLat())
        return false;

    if(vpBox.GetMaxLon() >= testBox.GetMinLon() && vpBox.GetMinLon() <= testBox.GetMaxLon())
        return true;

    if(vpBox.GetMaxLon() >= testBox.GetMinLon()+360 && vpBox.GetMinLon() <= testBox.GetMaxLon()+360)
        return true;

    if(vpBox.GetMaxLon() >= testBox.GetMinLon()-360 && vpBox.GetMinLon() <= testBox.GetMaxLon()-360)
        return true;

    return false;
}

bool s52plib::ObjectRenderCheckCat( ObjRazRules *rzRules, ViewPort *vp )
{
    g_scaminScale = 1.0;
    
    if( rzRules->obj == NULL ) return false;

    bool b_catfilter = true;
    bool b_visible = false;

    //      Do Object Type Filtering
    DisCat obj_cat = rzRules->obj->m_DisplayCat;
    
    //  Meta object filter.
    // Applied when showing display category OTHER, and
    // only for objects whose decoded S52 display category (by LUP) is also OTHER
    if( m_nDisplayCategory == OTHER ){
        if(OTHER == obj_cat){
            if( !strncmp( rzRules->LUP->OBCL, "M_", 2 ) )
                if( !m_bShowMeta &&  strncmp( rzRules->LUP->OBCL, "M_QUAL", 6 ))
                    return false;
        }
    }
    else{
        // We want to filter out M_NSYS objects everywhere except "OTHER" category
        if( !strncmp( rzRules->LUP->OBCL, "M_", 2 ) )
            if( !m_bShowMeta )
                return false;
    }


    if( m_nDisplayCategory == MARINERS_STANDARD ) {
        if( -1 == rzRules->obj->iOBJL ) UpdateOBJLArray( rzRules->obj );

        if( DISPLAYBASE == obj_cat ){        // always display individual objects that were moved to DISPLAYBASE by CS Procedures
            b_visible = true;
            b_catfilter = false;
        }
        else if( !( (OBJLElement *) ( pOBJLArray->at( rzRules->obj->iOBJL ) ) )->nViz ){
            b_catfilter = false;
        }
    }

    else
        if( m_nDisplayCategory == OTHER ) {
            if( ( DISPLAYBASE != obj_cat ) && ( STANDARD != obj_cat ) && ( OTHER != obj_cat ) ) {
                b_catfilter = false;
            }
        }

        else
            if( m_nDisplayCategory == STANDARD ) {
                if( ( DISPLAYBASE != obj_cat ) && ( STANDARD != obj_cat ) ) {
                    b_catfilter = false;
                }
            } else
                if( m_nDisplayCategory == DISPLAYBASE ) {
                    if( DISPLAYBASE != obj_cat ) {
                        b_catfilter = false;
                    }
                }

    //  Soundings override
    if( !strncmp( rzRules->LUP->OBCL, "SOUNDG", 6 ) )
        b_catfilter = m_bShowSoundg;
    
    if( b_catfilter ) {
        b_visible = true;

        //      SCAMIN Filtering
        //      Implementation note:
        //      According to S52 specs, SCAMIN must not apply to GROUP1 objects, Meta Objects
        //      or DisplayCategoryBase objects.
        //      Occasionally, an ENC will encode a spurious SCAMIN value for one of these objects.
        //      see, for example, US5VA18M, in OpenCPN SENC as Feature 350(DEPARE), LNAM = 022608187ED20ACC.
        //      We shall explicitly ignore SCAMIN filtering for these types of objects.

        if( m_bUseSCAMIN ) {


            if( ( DISPLAYBASE == rzRules->LUP->DISC ) || ( PRIO_GROUP1 == rzRules->LUP->DPRI ) )
                b_visible = true;
            else{
                //                if( vp->chartScale() > rzRules->obj->Scamin ) b_visible = false;


                double zoom_mod = (double)g_chart_zoom_modifier_vector;

                double modf = zoom_mod/5.;  // -1->1
                double mod = pow(8., modf);
                mod = qMax(mod, .2);
                mod = qMin(mod, 8.0);

                if(mod > 1){
                    if( vp->chartScale()  > rzRules->obj->Scamin * mod )
                        b_visible = false;                              // definitely invisible
                    else{
                        //  Theoretically invisible, however...
                        //  In the "zoom modified" scale region,
                        //  we render the symbol at reduced size, scaling down to no less than half normal size.
                        
                        if(vp->chartScale()  > rzRules->obj->Scamin){
                            double xs = vp->chartScale() - rzRules->obj->Scamin;
                            double xl = (rzRules->obj->Scamin * mod) - rzRules->obj->Scamin;
                            g_scaminScale = 1.0 - (0.5 * xs / xl);
                            
                        }
                    }
                }
                else{
                    if(vp->chartScale()  > rzRules->obj->Scamin)
                        b_visible = false;
                }
            }

            //      On the other hand, $TEXTS features need not really be displayed at all scales, always
            //      To do so makes a very cluttered display
            if( ( !strncmp( rzRules->LUP->OBCL, "$TEXTS", 6 ) )
                    && ( vp->chartScale() > rzRules->obj->Scamin ) ) b_visible = false;
        }

        return b_visible;
    }

    return b_visible;
}

bool s52plib::ObjectRenderCheckRules( ObjRazRules *rzRules, ViewPort *vp, bool check_noshow )
{
    if( !ObjectRenderCheckPos( rzRules, vp ) )
        return false;

    if( check_noshow && IsObjNoshow( rzRules->LUP->OBCL) )
        return false;

    if( ObjectRenderCheckCat( rzRules, vp ) )
        return true;

    //  If this object cannot be moved to a higher category by CS procedures,
    //  then we are done here
    if(!rzRules->obj->m_bcategory_mutable)
        return false;

    // already added, nothing below can change its display category
    if(rzRules->obj->bCS_Added )
        return false;

    //  Otherwise, make sure the CS, if present, has been evaluated,
    //  and then check the category again
    //  no rules
    if( !ObjectRenderCheckCS( rzRules, vp ) )
        return false;

    rzRules->obj->CSrules = NULL;
    Rules *rules = rzRules->LUP->ruleList;
    while( rules != NULL ) {
        if( RUL_CND_SY ==  rules->ruleType ){
            GetAndAddCSRules( rzRules, rules );
            rzRules->obj->bCS_Added = 1; // mark the object
            break;
        }
        rules = rules->next;
    }
    
    // still not displayable
    if( !ObjectRenderCheckCat( rzRules, vp ) )
        return false;

    return true;
}


void s52plib::SetDisplayCategory(enum _DisCat cat)
{
    enum _DisCat old = m_nDisplayCategory;
    m_nDisplayCategory = cat;
    
    if(old != cat){
        ClearNoshow();
    }
    GenerateStateHash();
}


bool s52plib::IsObjNoshow( const char *objcl )
{
    for(unsigned int i=0 ; i < m_noshow_array.count() ; i++){
        if(!strncmp(m_noshow_array[i].obj, objcl, 6) )
            return true;
    }
    return false;
}

void s52plib::AddObjNoshow( const char *objcl )
{
    if( !IsObjNoshow( objcl ) ){
        noshow_element element;
        memcpy(element.obj, objcl, 6);
        m_noshow_array.append( element );
    }
}

void s52plib::RemoveObjNoshow( const char *objcl )
{
    for(unsigned int i=0 ; i < m_noshow_array.count() ; i++){
        if(!strncmp(m_noshow_array[i].obj, objcl, 6) ){
            m_noshow_array.removeAt(i);
            return;
        }
    }
}

void s52plib::ClearNoshow(void)
{
    m_noshow_array.clear();
}

void s52plib::PLIB_LoadS57Config()
{
    //    Get a pointer to the opencpn configuration object
    zchxConfig *pconfig = ZCHX_CFG_INS;
    
    int read_int;
    double dval;
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bShowS57Text", 0 ).toInt();
    SetShowS57Text( !( read_int == 0 ) );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bShowS57ImportantTextOnly", 0 ).toInt();
    SetShowS57ImportantTextOnly( !( read_int == 0 ) );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bShowLightDescription", 0 ).toInt();
    SetShowLdisText( !( read_int == 0 ) );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bExtendLightSectors", 0 ).toInt();
    SetExtendLightSectors( !( read_int == 0 ) );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "nDisplayCategory", 0 ).toInt();
    SetDisplayCategory((enum _DisCat) read_int );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "nSymbolStyle", (enum _LUPname) PAPER_CHART).toInt();
    m_nSymbolStyle = (LUPname) read_int;
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "nBoundaryStyle", PLAIN_BOUNDARIES ).toInt();
    m_nBoundaryStyle = (LUPname) read_int;
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bShowSoundg", 1 ).toInt();
    m_bShowSoundg = !( read_int == 0 );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bShowMeta", 0 ).toInt();
    m_bShowMeta = !( read_int == 0 );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bUseSCAMIN", 1 ).toInt();
    m_bUseSCAMIN = !( read_int == 0 );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bShowAtonText", 1 ).toInt();
    m_bShowAtonText = !( read_int == 0 );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bDeClutterText", 0 ).toInt();
    m_bDeClutterText = !( read_int == 0 );
    
    read_int = pconfig->getCustomValue("Settings/GlobalState", "bShowNationalText", 0 ).toInt();
    m_bShowNationalTexts = !( read_int == 0 );
    
    dval = pconfig->getCustomValue("Settings/GlobalState", "S52_MAR_SAFETY_CONTOUR", 5.0).toDouble();
    S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval );
    S52_setMarinerParam( S52_MAR_SAFETY_DEPTH, dval ); // Set safety_contour and safety_depth the same
    
    dval = pconfig->getCustomValue("Settings/GlobalState", "S52_MAR_SHALLOW_CONTOUR", 3.0).toDouble();
    S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval );
    
    dval = pconfig->getCustomValue("Settings/GlobalState", "S52_MAR_DEEP_CONTOUR", 10.0).toDouble();
    S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval );
    
    dval = pconfig->getCustomValue("Settings/GlobalState", "S52_MAR_TWO_SHADES", 0.0).toDouble();
    S52_setMarinerParam(S52_MAR_TWO_SHADES, dval );
    
    UpdateMarinerParams();

    read_int = pconfig->getCustomValue("Settings/GlobalState", "S52_DEPTH_UNIT_SHOW", 1 ).toInt();   // default is metres
    read_int = qMax(read_int, 0);                      // qualify value
    read_int = qMin(read_int, 2);
    m_nDepthUnitDisplay = read_int;
    
    //    S57 Object Class Visibility
    
    OBJLElement *pOLE;
    QStringList keys = pconfig->getChildKeys("Settings/ObjectFilter");
    int iOBJMax = keys.size();
    for(int i=0; i<iOBJMax; i++)
    {
        QString str = keys[i];
        long val = pconfig->getCustomValue("Settings/ObjectFilter", str).toLongLong();
        bool bNeedNew = true;

        if( str.startsWith("viz") ) {
            QString sObj = str.mid(3);
            for( unsigned int iPtr = 0; iPtr < pOBJLArray->count(); iPtr++ ) {
                pOLE = (OBJLElement *) ( pOBJLArray->at( iPtr ) );
                if( !strncmp( pOLE->OBJLName, sObj.toUtf8().data(), 6 ) ) {
                    pOLE->nViz = val;
                    bNeedNew = false;
                    break;
                }
            }

            if( bNeedNew ) {
                pOLE = (OBJLElement *) calloc( sizeof(OBJLElement), 1 );
                memcpy( pOLE->OBJLName, sObj.toUtf8().data(), OBJL_NAME_LEN );
                pOLE->nViz = 1;
                pOBJLArray->append((void *) pOLE );
            }
        }
    }
}

//    Do all those things necessary to prepare for a new rendering
void s52plib::PrepareForRender( void )
{
    PrepareForRender(NULL);
}

void s52plib::PrepareForRender(ViewPort *vp)
{
    m_benableGLLS = true;               // default is to always use RenderToGLLS (VBO support)

#ifdef USE_ANDROID_GLES2
    void PrepareS52ShaderUniforms(ViewPort *vp);
    if(vp)
        PrepareS52ShaderUniforms( vp );
#endif

#ifdef BUILDING_PLUGIN    
    // Has the core S52PLIB configuration changed?
    //  If it has, reload from global preferences file, and other dynamic status information.
    //  This additional step is only necessary for Plugin chart rendering, as core directly sets
    //  options and updates State Hash as needed.

    int core_config = PI_GetPLIBStateHash();
    if(core_config != m_myConfig){
        
        g_ChartScaleFactorExp = GetOCPNChartScaleFactor_Plugin();
        
        //  If a modern (> OCPN 4.4) version of the core is active,
        //  we may rely upon having been updated on S52PLIB state by means of PlugIn messaging scheme.
        if( ((m_coreVersionMajor == 4) && (m_coreVersionMinor >= 5)) || m_coreVersionMajor > 4 ){
            
            // Retain compatibility with O4.8.x
            if( (m_coreVersionMajor == 4) && (m_coreVersionMinor < 9)){
                // First, we capture some temporary values that were set by messaging, but would be overwritten by config read
                bool bTextOn = m_bShowS57Text;
                bool bSoundingsOn = m_bShowSoundg;
                enum _DisCat old = m_nDisplayCategory;

                PLIB_LoadS57Config();

                //  And then reset the temp values that were overwritten by config load
                m_bShowS57Text = bTextOn;
                m_bShowSoundg = bSoundingsOn;
                m_nDisplayCategory = old;
            }
            else
                PLIB_LoadS57GlobalConfig();
            
            
            // Pick up any changes in Mariner's Standard object list
            PLIB_LoadS57ObjectConfig();

            // Detect and manage "LIGHTS" toggle
            bool bshow_lights = !m_lightsOff;
            if(!bshow_lights)                     // On, going off
                AddObjNoshow("LIGHTS");
            else{                                   // Off, going on
                RemoveObjNoshow("LIGHTS");
            }

            const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL", "SBDARE" };
            unsigned int num = sizeof(categories) / sizeof(categories[0]);
            
            // Handle Anchor area toggle
            if( (m_nDisplayCategory == OTHER) || (m_nDisplayCategory == MARINERS_STANDARD) ){
                bool bAnchor = m_anchorOn;
                
                
                if(!bAnchor){
                    for( unsigned int c = 0; c < num; c++ )
                        AddObjNoshow(categories[c]);
                }
                else{
                    for( unsigned int c = 0; c < num; c++ )
                        RemoveObjNoshow(categories[c]);

                    //  Force the USER STANDARD object list anchor detail items ON
                    unsigned int cnt = 0;
                    for( unsigned int iPtr = 0; iPtr < pOBJLArray->count(); iPtr++ ) {
                        OBJLElement *pOLE = (OBJLElement *) ( pOBJLArray->at( iPtr ) );
                        for( unsigned int c = 0; c < num; c++ ) {
                            if( !strncmp( pOLE->OBJLName, categories[c], 6 ) ) {
                                pOLE->nViz = 1;         // force on
                                cnt++;
                                break;
                            }
                        }
                        if( cnt == num ) break;
                    }
                }
            }
        }
        m_myConfig = PI_GetPLIBStateHash();
    }

#endif          //BUILDING_PLUGIN

    // Reset the LIGHTS declutter machine
    lastLightLat = 0;
    lastLightLon = 0;
    
}

void s52plib::SetAnchorOn(bool val)
{
    OBJLElement *pOLE = NULL;

    const char * categories[] = { "ACHBRT", "ACHARE", "CBLSUB", "PIPARE", "PIPSOL", "TUNNEL", "SBDARE" };
    unsigned int num = sizeof(categories) / sizeof(categories[0]);

    if( (m_nDisplayCategory == OTHER) || (m_nDisplayCategory == MARINERS_STANDARD) ){
        bool bAnchor = val;

        if(!bAnchor){
            for( unsigned int c = 0; c < num; c++ ) {
                AddObjNoshow(categories[c]);
            }
        }
        else{
            for( unsigned int c = 0; c < num; c++ ) {
                RemoveObjNoshow(categories[c]);
            }
        }
    }
    else{                               // if not category OTHER, then anchor-related features are always shown.
        for( unsigned int c = 0; c < num; c++ ) {
            RemoveObjNoshow(categories[c]);
        }
    }
    
    m_anchorOn = val;
}

void s52plib::SetQualityOfData(bool val)
{
    int old_vis = GetQualityOfData();
    if(old_vis == val)
        return;
    
    if(old_vis && !val){                            // On, going off
        AddObjNoshow("M_QUAL");
    }
    else if(!old_vis && val){                                   // Off, going on
        RemoveObjNoshow("M_QUAL");

        for( unsigned int iPtr = 0; iPtr < pOBJLArray->count(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( pOBJLArray->at( iPtr ) );
            if( !strncmp( pOLE->OBJLName, "M_QUAL", 6 ) ) {
                pOLE->nViz = 1;         // force on
                break;
            }
        }
    }

    m_qualityOfDataOn = val;
    
}

void s52plib::ClearTextList( void )
{
    //      Clear the current text rectangle list
    m_textObjList.clear();

}

bool s52plib::EnableGLLS(bool b_enable)
{
    bool return_val = m_benableGLLS;
    m_benableGLLS = b_enable;
    return return_val;
}

void s52plib::AdjustTextList( int dx, int dy, int screenw, int screenh )
{
    return;
    QRect rScreen( 0, 0, screenw, screenh );
    //    Iterate over the text rectangle list
    //        1.  Apply the specified offset to the list elements
    //        2.. Remove any list elements that are off screen after applied offset

    for(int i=0; i < m_textObjList.size(); ) {
        S52_TextC* data = m_textObjList[i];
        data->rText.translate(dx, dy );
        if( !data->rText.intersects(rScreen ) ) {
            m_textObjList.removeAt(i);
            delete data;
            continue;
        }
        i++;
    }
}

bool s52plib::GetPointPixArray( ObjRazRules *rzRules, zchxPointF* pd, zchxPoint *pp, int nv, ViewPort *vp )
{
    for( int i = 0; i < nv; i++ ) {
        GetPointPixSingle(rzRules, pd[i].y, pd[i].x, pp + i, vp);
    }
    
    return true;
}

bool s52plib::GetPointPixSingle( ObjRazRules *rzRules, float north, float east, zchxPoint *r, ViewPort *vp )
{
    if(vp->projectType() == PROJECTION_MERCATOR) {

        double xr =  rzRules->obj->x_rate;
        double xo =  rzRules->obj->x_origin;
        double yr =  rzRules->obj->y_rate;
        double yo =  rzRules->obj->y_origin;

        if(fabs(xo) > 1){                           // cm93 hits this
            if ( vp->getBBox().GetMaxLon() >= 180. && rzRules->obj->BBObj.GetMaxLon() < vp->getBBox().GetMinLon() )
                xo += mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
            else if( (vp->getBBox().GetMinLon() <= -180. &&
                      rzRules->obj->BBObj.GetMinLon() > vp->getBBox().GetMaxLon()) ||
                     (rzRules->obj->BBObj.GetMaxLon() >= 180 && vp->getBBox().GetMinLon() <= 0.))
                xo -= mercator_k0 * WGS84_semimajor_axis_meters * 2.0 * PI;
        }

        double valx = ( east * xr ) + xo;
        double valy = ( north * yr ) + yo;

        r->x = roundint(((valx - rzRules->sm_transform_parms->easting_vp_center) * vp->viewScalePPM()) + (vp->pixWidth() / 2) );
        r->y = roundint((vp->pixHeight()/2) - ((valy - rzRules->sm_transform_parms->northing_vp_center) * vp->viewScalePPM()));
    } else {
        double lat, lon;
        fromSM(east - rzRules->sm_transform_parms->easting_vp_center,
               north - rzRules->sm_transform_parms->northing_vp_center,
               vp->lat(), vp->lon(), &lat, &lon);

        *r = vp->GetPixFromLL(north, east);
    }
    
    return true;
}

void s52plib::GetPixPointSingle( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
#if 1
    vpt->GetLLFromPix(zchxPoint(pixx, pixy), plat, plon);
    //    if(*plon < 0 && vpt->lon() > 180)
    //      *plon += 360;
#else
    //    Use Mercator estimator
    int dx = pixx - ( vpt->pix_width / 2 );
    int dy = ( vpt->pix_height / 2 ) - pixy;
    
    double xp = ( dx * cos( vpt->skew ) ) - ( dy * sin( vpt->skew ) );
    double yp = ( dy * cos( vpt->skew ) ) + ( dx * sin( vpt->skew ) );
    
    double d_east = xp / vpt->view_scale_ppm;
    double d_north = yp / vpt->view_scale_ppm;
    
    double slat, slon;
    fromSM( d_east, d_north, vpt->lat(), vpt->lon(), &slat, &slon );
    
    *plat = slat;
    *plon = slon;
#endif    
}

void s52plib::GetPixPointSingleNoRotate( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt )
{
    if(vpt){
        double rotation = vpt->rotation();
        vpt->setRotation(0);
        vpt->GetLLFromPix(zchxPoint(pixx, pixy), plat, plon);
        vpt->setRotation(rotation);
    }
}    


void DrawAALine( QPainter *pDC, int x0, int y0, int x1, int y1, QColor clrLine, int dash, int space )
{
    int width = 1 + abs( x0 - x1 );
    int height = 1 + abs( y0 - y1 );
    zchxPoint upperLeft( qMin ( x0, x1 ), qMin ( y0, y1 ) );
    pDC->save();
    QPen pen( clrLine, 1, Qt::CustomDashLine );
    QVector<qreal> dashes;
    dashes.append(dash);
    dashes.append(space);
    pen.setDashPattern(dashes);
    pDC->setPen(pen);
    pDC->translate(upperLeft.toPoint());
    pDC->drawLine( x0 - upperLeft.x, y0 - upperLeft.y, x1 - upperLeft.x, y1 - upperLeft.y );
    pDC->restore();
    return;
}

RenderFromHPGL::RenderFromHPGL( s52plib* plibarg )
{
    plib = plibarg;
    renderToDC = false;
    renderToOpenGl = false;
    renderToGCDC = false;
    transparency = 255;
}

RenderFromHPGL::~RenderFromHPGL( )
{
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        glDisable (GL_BLEND );
    }
#endif    
}

void RenderFromHPGL::SetTargetDC( QPainter* pdc )
{
    targetDC = pdc;
    renderToDC = true;
    renderToOpenGl = false;
    renderToGCDC = false;
}

void RenderFromHPGL::SetTargetOpenGl()
{
    renderToOpenGl = true;
    renderToDC = false;
    renderToGCDC = false;
}

#if wxUSE_GRAPHICS_CONTEXT
void RenderFromHPGL::SetTargetGCDC( wxGCDC* gdc )
{
    targetGCDC = gdc;
    renderToGCDC = true;
    renderToDC = false;
    renderToOpenGl = false;
}
#endif

const char* RenderFromHPGL::findColorNameInRef( char colorCode, char* col )
{
    int noColors = strlen( col ) / 6;
    for( int i = 0, j=0; i < noColors; i++, j += 6 ) {
        if( *(col + j) == colorCode ) return col + j + 1;
    }
    return col + 1; // Default to first color if not found.
}

zchxPoint RenderFromHPGL::ParsePoint( QString& argument )
{
    int colon = argument.indexOf(',');
    long x = argument.left(colon ).toLong();
    long y = argument.mid(colon + 1).toLong();
    return zchxPoint( x, y );
}

void RenderFromHPGL::SetPen()
{

    if( renderToDC ) {
        mPen = new QPen(penColor, penWidth, Qt::SolidLine );
        mBrush = new QBrush(penColor);
        mPen->setBrush(*mBrush);
        targetDC->setPen(*mPen);
        //        targetDC->SetBrush( *brush );
    }
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        if( plib->GetGLPolygonSmoothing() )
            glEnable( GL_POLYGON_SMOOTH );
        
        glColor4ub( penColor.red(), penColor.green(), penColor.blue(), transparency );
        int line_width = qMax(g_GLMinSymbolLineWidth, (float) (penWidth * 0.7));
        glLineWidth( line_width );
        
#ifdef __OCPN__ANDROID__
        //  Scale the pen width dependent on the platform display resolution
        float nominal_line_width_pix = qMax(1.0, floor(ps52plib->GetPPMM() / 5.0));             //0.2 mm nominal, but not less than 1 pixel
        //qDebug() << nominal_line_width_pix;
        line_width =  qMax(g_GLMinSymbolLineWidth, (float) penWidth * nominal_line_width_pix);
        glLineWidth( line_width );
#endif
        
#ifndef __OCPN__ANDROID__
        if( line_width >= 2 && plib->GetGLLineSmoothing() )
            glEnable( GL_LINE_SMOOTH );
        else
            glDisable( GL_LINE_SMOOTH );
        glEnable( GL_BLEND );
#endif        
    }
#endif    
#if wxUSE_GRAPHICS_CONTEXT
    if( renderToGCDC ) {
        pen = wxThePenList->FindOrCreatePen( penColor, penWidth, QPenSTYLE_SOLID );
        brush = wxTheBrushList->FindOrCreateBrush( penColor, wxBRUSHSTYLE_SOLID );
        targetGCDC->SetPen( *pen );
        targetGCDC->SetBrush( *brush );
    }
#endif
}

void RenderFromHPGL::Line( zchxPoint from, zchxPoint to )
{
    if( renderToDC ) {
        targetDC->drawLine(from.toPoint(), to.toPoint() );
    }
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        glBegin( GL_LINES );
        glVertex2i( from.x, from.y );
        glVertex2i( to.x, to.y );
        glEnd();
    }
#endif
#if wxUSE_GRAPHICS_CONTEXT
    if( renderToGCDC ) {
        targetGCDC->DrawLine( from, to );
    }
#endif
}

void RenderFromHPGL::Circle( zchxPoint center, int radius, bool filled )
{
    if( renderToDC ) {
        QPen pen = targetDC->pen();
        pen.setBrush( filled == true? (*mBrush) : Qt::transparent);
        targetDC->drawEllipse(center.x, center.y, radius, radius );
    }
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        int noSegments = 2 + ( radius * 4 );
        if( noSegments > 200 ) noSegments = 200;
        glBegin( GL_LINE_STRIP );
        for( float a = 0; a <= 2 * M_PI; a += 2 * M_PI / noSegments )
            glVertex2f( center.x + radius * sinf( a ),
                        center.y + radius * cosf( a ) );
        glEnd();
    }
#endif    
#if wxUSE_GRAPHICS_CONTEXT
    if( renderToGCDC ) {
        if( filled ) targetGCDC->SetBrush( *brush );
        else
            targetGCDC->SetBrush( *wxTRANSPARENT_BRUSH );

        targetGCDC->DrawCircle( center, radius );

        // wxGCDC doesn't update min/max X/Y properly for DrawCircle.
        targetGCDC->SetPen( *wxTRANSPARENT_PEN );
        targetGCDC->DrawPoint( center.x - radius, center.y );
        targetGCDC->DrawPoint( center.x + radius, center.y );
        targetGCDC->DrawPoint( center.x, center.y - radius );
        targetGCDC->DrawPoint( center.x, center.y + radius );
        targetGCDC->SetPen( *pen );
    }
#endif
}

void RenderFromHPGL::Polygon()
{
    if( renderToDC ) {
        targetDC->drawPolygon(zchxPoint::makePoiygon(polygon, noPoints));
    }
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        glColor4ub( penColor.red(), penColor.green(), penColor.blue(), transparency );
        
        glBegin( GL_POLYGON );
        for( int ip = 1; ip < noPoints; ip++ )
            glVertex2i( polygon[ip].x, polygon[ip].y );
        glEnd();
    }
#endif    
#if wxUSE_GRAPHICS_CONTEXT
    if( renderToGCDC ) {
        targetGCDC->DrawPolygon( noPoints, polygon );
    }
#endif
}

void RenderFromHPGL::RotatePoint( zchxPoint& point, zchxPoint origin, double angle )
{
    if( angle == 0. ) return;
    double sin_rot = sin( angle * PI / 180. );
    double cos_rot = cos( angle * PI / 180. );

    double xp = ( (point.x - origin.x) * cos_rot ) - ( (point.y - origin.y) * sin_rot );
    double yp = ( (point.x - origin.x) * sin_rot ) + ( (point.y - origin.y) * cos_rot );

    point.x = (int) xp + origin.x;
    point.y = (int) yp + origin.y;
}

bool RenderFromHPGL::Render( char *str, char *col, zchxPoint &r, zchxPoint &pivot, zchxPoint origin, float scale, double rot_angle, bool bSymbol )
{
#ifdef ocpnUSE_GL
    if( renderToOpenGl )
        glGetFloatv(GL_CURRENT_COLOR,m_currentColor);
#endif        
    
    zchxPoint lineStart;
    zchxPoint lineEnd;

    scaleFactor = 100.0 / plib->GetPPMM();
    scaleFactor /= scale;
    scaleFactor /= g_scaminScale;
    
    if(bSymbol)
        scaleFactor /= plib->GetRVScaleFactor();
    
    // SW is not always defined, cf. US/US4CA17M/US4CA17M.000
    penWidth = 1;

    QStringList commands = QString::fromUtf8(str).split(";");
    int i = 0;
    while( i < commands.size() ) {
        QString command = commands[i++];
        QString arguments = command.mid(2 );
        command = command.left( 2 );

        if( command == "SP" ) {
            S52color* color = plib->getColor( findColorNameInRef( arguments[0].unicode(), col ) );
            penColor = QColor( color->R, color->G, color->B );
            brushColor = penColor;
            continue;
        }
        if( command == "SW" ) {
            penWidth = arguments.toLong();
            continue;
        }
        if( command == "ST" ) {
            long transIndex = arguments.toLong();
            transparency = (4 - transIndex) * 64;
            transparency = qMin(transparency, 255);
            transparency = qMax(0, transparency);
            continue;
        }
        if( command == "PU" ) {
            SetPen();
            lineStart = ParsePoint( arguments );
            RotatePoint( lineStart, origin, rot_angle );
            lineStart -= pivot;
            lineStart.x /= scaleFactor;
            lineStart.y /= scaleFactor;
            lineStart += r;
            continue;
        }
        if( command == "PD" ) {
            if( arguments.length() == 0 ) {
                lineEnd = lineStart;
                lineEnd.x++;
            } else {
                lineEnd = ParsePoint( arguments );
                RotatePoint( lineEnd, origin, rot_angle );
                lineEnd -= pivot;
                lineEnd.x /= scaleFactor;
                lineEnd.y /= scaleFactor;
                lineEnd += r;
            }
            Line( lineStart, lineEnd );
            lineStart = lineEnd; // For next line.
            continue;
        }
        if( command == "CI" ) {
            long radius = arguments.toLong();
            radius = (int) radius / scaleFactor;
            Circle( lineStart, radius );
            continue;
        }
        if( command == "PM" ) {
            noPoints = 1;
            polygon[0] = lineStart;

            if( arguments == "0" ) {
                do {
                    command = commands[i++];
                    arguments = command.mid( 2 );
                    command = command.left( 2 );

                    if( command == "AA" ) {
                        qDebug( "RenderHPGL: AA instruction not implemented." );
                    }
                    if( command == "CI" ) {
                        long radius = arguments.toLong(  );
                        radius = (int) radius / scaleFactor;
                        Circle( lineStart, radius, HPGL_FILLED );
                    }
                    if( command == "PD" ) {
                        QStringList points = arguments.split(",");
                        int k = 0;
                        while( k < points.size()) {
                            long x = points[k++].toLong();
                            long y = points[k++].toLong();
                            lineEnd = zchxPoint( x, y );
                            RotatePoint( lineEnd, origin, rot_angle );
                            lineEnd -= pivot;
                            lineEnd.x /= scaleFactor;
                            lineEnd.y /= scaleFactor;
                            lineEnd += r;
                            polygon[noPoints++] = lineEnd;
                        }
                    }
                } while( command != "PM" );
            }
            continue;
        }
        if( command == "FP" ) {
            SetPen();
            Polygon();
            continue;
        }

        // Only get here if non of the other cases did a continue.
        //        QString msg( _T("RenderHPGL: The '%s' instruction is not implemented.") );
        //        msg += QString( command );
        //        wxLogWarning( msg );
    }
    
    transparency = 255;
    
#ifdef ocpnUSE_GL
    if( renderToOpenGl ) {
        glDisable (GL_BLEND );
        glColor4fv( m_currentColor );
    }
#endif    

    return true;
}


#ifdef ocpnUSE_GL
/* draw a half circle using triangles */
void PLIBDrawEndCap(float x1, float y1, float t1, float angle)
{
    const int steps = 16;
    float xa, ya;
    bool first = true;
    for(int i = 0; i <= steps; i++) {
        float a = angle + M_PI/2 + M_PI/steps*i;
        
        float xb = x1 + t1 / 2 * cos( a );
        float yb = y1 + t1 / 2 * sin( a );
        if(first)
            first = false;
        else {
            glVertex2f( x1, y1 );
            glVertex2f( xa, ya );
            glVertex2f( xb, yb );
        }
        xa = xb, ya = yb;
    }
}
#endif

// Draws a line between (x1,y1) - (x2,y2) with a start thickness of t1
void PLIBDrawGLThickLine( float x1, float y1, float x2, float y2, QPen pen, bool b_hiqual )
{
#ifdef ocpnUSE_GL
    
    float angle = atan2f( y2 - y1, x2 - x1 );
    float t1 = pen.widthF();
    float t2sina1 = t1 / 2 * sinf( angle );
    float t2cosa1 = t1 / 2 * cosf( angle );
    
    glBegin( GL_TRIANGLES );
    
    //    n.b.  The dwxDash interpretation for GL only allows for 2 elements in the dash table.
    //    The first is assumed drawn, second is assumed space
    //    wxDash *dashes;
    //    int n_dashes = pen.GetDashes( &dashes );
    QVector<qreal> dashes = pen.dashPattern();
    int n_dashes = dashes.size();
    if( n_dashes ) {
        float lpix = sqrtf( powf( (float) (x1 - x2), 2) + powf( (float) (y1 - y2), 2) );
        float lrun = 0.;
        float xa = x1;
        float ya = y1;
        //        float ldraw = t1 * (unsigned char)dashes[0];
        //        float lspace = t1 * (unsigned char)dashes[1];
        float ldraw = t1 * dashes[0];
        float lspace = t1 * dashes[1];
        
        if((ldraw < 0) || (lspace < 0)){
            glEnd();
            return;
        }
        
        while( lrun < lpix ) {
            //    Dash
            float xb = xa + ldraw * cosf( angle );
            float yb = ya + ldraw * sinf( angle );
            
            if( ( lrun + ldraw ) >= lpix )         // last segment is partial draw
            {
                xb = x2;
                yb = y2;
            }
            
            glVertex2f( xa + t2sina1, ya - t2cosa1 );
            glVertex2f( xb + t2sina1, yb - t2cosa1 );
            glVertex2f( xb - t2sina1, yb + t2cosa1 );
            
            glVertex2f( xb - t2sina1, yb + t2cosa1 );
            glVertex2f( xa - t2sina1, ya + t2cosa1 );
            glVertex2f( xa + t2sina1, ya - t2cosa1 );
            
            xa = xb;
            ya = yb;
            lrun += ldraw;
            
            //    Space
            xb = xa + lspace * cos( angle );
            yb = ya + lspace * sin( angle );
            
            xa = xb;
            ya = yb;
            lrun += lspace;
        }
    } else {
        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );
        glVertex2f( x2 + t2sina1, y2 - t2cosa1 );
        glVertex2f( x2 - t2sina1, y2 + t2cosa1 );
        
        glVertex2f( x2 - t2sina1, y2 + t2cosa1 );
        glVertex2f( x1 - t2sina1, y1 + t2cosa1 );
        glVertex2f( x1 + t2sina1, y1 - t2cosa1 );
        
        /* wx draws a nice rounded end in dc mode, so replicate
         *           this for opengl mode, should this be done for the dashed mode case? */
        if(pen.capStyle() == Qt::RoundCap) {
            PLIBDrawEndCap( x1, y1, t1, angle);
            PLIBDrawEndCap( x2, y2, t1, angle + M_PI);
        }
        
    }
    
    glEnd();
#endif    
}

