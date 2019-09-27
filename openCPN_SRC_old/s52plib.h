/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S52 Presentation Library
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

#ifndef _S52PLIB_H_
#define _S52PLIB_H_

#include <vector>

#include "s52s57.h"                 //types

class QGLContext;

#include "LLRegion.h"
#include "_def.h"

typedef QHash<QString, Rule*> RuleHash;
typedef QHash<int, QString>  MyNatsurHash;

typedef QList<LUPrec *> wxArrayOfLUPrec;

typedef QList<S52_TextC*> TextObjList;

struct CARC_Buffer {
    unsigned char color[3][4];
    float line_width[3];
    int steps;

    int size;
    float *data;
};
typedef QHash<QString, CARC_Buffer> CARC_Hash;
typedef QHash<QString, int> CARC_DL_Hash;

class ViewPort;
class PixelCache;

class RenderFromHPGL;
class TexFont;

class noshow_element
{
public:
    char obj[7];
};

typedef QList<noshow_element> ArrayOfNoshow;

//-----------------------------------------------------------------------------
//      LUP Array container, and friends
//-----------------------------------------------------------------------------
typedef struct _LUPHashIndex {
    int n_start;
    int count;
} LUPHashIndex;

typedef QHash<QString, LUPHashIndex*> LUPArrayIndexHash;

class LUPArrayContainer {
public:
    LUPArrayContainer();
    ~LUPArrayContainer();
    
    wxArrayOfLUPrec     *GetLUPArray(void){ return LUPArray; }
    LUPHashIndex        *GetArrayIndexHelper( const char *objectName );
    
private:
    wxArrayOfLUPrec             *LUPArray;          // Sorted Array
    LUPArrayIndexHash           IndexHash;
};

    
//-----------------------------------------------------------------------------
//    s52plib definition
//-----------------------------------------------------------------------------

class s52plib {
public:
     s52plib( const QString& PLib, bool b_forceLegacy = false );
    ~s52plib();

    void SetPPMM( float ppmm );
    float GetPPMM() { return canvas_pix_per_mm; }

    void SetOCPNVersion(int major, int minor, int patch);
    
    double GetRVScaleFactor() { return m_rv_scale_factor; }
    
    LUPrec *S52_LUPLookup( LUPname LUP_name, const char * objectName,
        S57Obj *pObj, bool bStrict = 0 );
    int _LUP2rules( LUPrec *LUP, S57Obj *pObj );
    S52color* getColor( const char *colorName );
    QColor getQColor( const QString &colorName );

    void UpdateMarinerParams( void );
    void ClearCNSYLUPArray( void );

    void GenerateStateHash();
    long GetStateHash() { return m_state_hash;  }

    void SetPLIBColorScheme( QString scheme );
    void SetPLIBColorScheme( ColorScheme cs );
    QString GetPLIBColorScheme( void ) { return m_ColorScheme; }

    void SetGLRendererString(const QString &renderer);
    void SetGLOptions(bool b_useStencil,
                      bool b_useStencilAP,
                      bool b_useScissors,
                      bool b_useFBO,
                      bool b_useVBO,
                      int  nTextureFormat);
    
    bool ObjectRenderCheck( ObjRazRules *rzRules, ViewPort *vp );
    bool ObjectRenderCheckRules( ObjRazRules *rzRules, ViewPort *vp, bool check_noshow = false );
    bool ObjectRenderCheckPos( ObjRazRules *rzRules, ViewPort *vp );
    bool ObjectRenderCheckCat( ObjRazRules *rzRules, ViewPort *vp );
    bool ObjectRenderCheckCS( ObjRazRules *rzRules, ViewPort *vp );

    static void DestroyLUP( LUPrec *pLUP );
    static void ClearRulesCache( Rule *pR );
    DisCat findLUPDisCat(const char *objectName, LUPname TNAM);
    
//    Temporarily save/restore the current colortable index
//    Useful for Thumbnail rendering
    void SaveColorScheme( void ) { m_colortable_index_save = m_colortable_index;}
    void RestoreColorScheme( void ) {}

//    Rendering stuff
    void PrepareForRender( ViewPort *vp );
    void PrepareForRender( void );
    void AdjustTextList( int dx, int dy, int screenw, int screenh );
    void ClearTextList( void );
    int SetLineFeaturePriority( ObjRazRules *rzRules, int npriority );
    void FlushSymbolCaches();

    //    For DC's
    int RenderObjectToDC(ObjRazRules *rzRules, ViewPort *vp );
    int RenderObjectToDCText(ObjRazRules *rzRules, ViewPort *vp );
    int RenderAreaToDC(ObjRazRules *rzRules, ViewPort *vp, render_canvas_parms *pb_spec );

    // Accessors
    bool GetShowSoundings() { return m_bShowSoundg; }
    void SetShowSoundings( bool f ) { m_bShowSoundg = f; GenerateStateHash(); }

    bool GetShowS57Text() { return m_bShowS57Text;  }
    void SetShowS57Text( bool f ) { m_bShowS57Text = f;  GenerateStateHash(); }

    bool GetShowS57ImportantTextOnly() { return m_bShowS57ImportantTextOnly; }
    void SetShowS57ImportantTextOnly( bool f ) { m_bShowS57ImportantTextOnly = f; GenerateStateHash(); }

    void SetLightsOff(bool val){ m_lightsOff = val; }
    bool GetLightsOff(){ return m_lightsOff; }
    
    void SetAnchorOn(bool val);
    bool GetAnchorOn();

    void SetQualityOfData(bool val);
    bool GetQualityOfData();
    
    int GetMajorVersion( void ) { return m_VersionMajor; }
    int GetMinorVersion( void ) { return m_VersionMinor; }

    void SetTextOverlapAvoid( bool f ) { m_bDeClutterText = f; }
    void SetShowNationalText( bool f ) { m_bShowNationalTexts = f; }
    void SetShowAtonText( bool f ) { m_bShowAtonText = f; }
    void SetShowLdisText( bool f ) { m_bShowLdisText = f; }
    void SetExtendLightSectors( bool f ) { m_bExtendLightSectors = f; }

    void SetDisplayCategory( enum _DisCat cat );
    DisCat GetDisplayCategory(){ return m_nDisplayCategory; }

    void SetGLPolygonSmoothing( bool bset ){ m_GLPolygonSmoothing = bset;}
    bool GetGLPolygonSmoothing( ){ return m_GLPolygonSmoothing; }
    void SetGLLineSmoothing( bool bset ){ m_GLLineSmoothing = bset;}
    bool GetGLLineSmoothing( ){ return m_GLLineSmoothing; }

    wxArrayOfLUPrec* SelectLUPARRAY( LUPname TNAM );
    LUPArrayContainer *SelectLUPArrayContainer( LUPname TNAM );
        
    void DestroyPatternRuleNode( Rule *pR );
    void DestroyRuleNode( Rule *pR );
    static void DestroyRulesChain( Rules *top );
    
    //    For OpenGL
    int RenderObjectToGL(QGLContext *glcc, ObjRazRules *rzRules, ViewPort *vp );
    int RenderAreaToGL(QGLContext *glcc, ObjRazRules *rzRules, ViewPort *vp );
    int RenderObjectToGLText(QGLContext *glcc, ObjRazRules *rzRules, ViewPort *vp );
    
    void RenderPolytessGL( ObjRazRules *rzRules, ViewPort *vp,double z_clip_geom, zchxPoint *ptp );
    
    bool EnableGLLS(bool benable);

    bool IsObjNoshow( const char *objcl);
    void AddObjNoshow( const char *objcl);
    void RemoveObjNoshow( const char *objcl);
    void ClearNoshow(void);
    void SaveObjNoshow() { m_saved_noshow = m_noshow_array; }
    void RestoreObjNoshow() { m_noshow_array = m_saved_noshow; }
    
    //Todo accessors
    LUPname m_nSymbolStyle;
    LUPname m_nBoundaryStyle;
    bool m_bOK;

    bool m_bShowSoundg;
    bool m_bShowMeta;
    bool m_bShowS57Text;
    bool m_bUseSCAMIN;
    bool m_bShowAtonText;
    bool m_bShowLdisText;
    bool m_bExtendLightSectors;
    bool m_bShowS57ImportantTextOnly;
    bool m_bDeClutterText;
    bool m_bShowNationalTexts;

    int m_VersionMajor;
    int m_VersionMinor;

    int m_nDepthUnitDisplay;

    //    Library data
    wxArrayPtrVoid *pAlloc;

    RuleHash *_line_sym; // line symbolisation rules
    RuleHash *_patt_sym; // pattern symbolisation rules
    RuleHash *_cond_sym; // conditional symbolisation rules
    RuleHash *_symb_symR; // symbol symbolisation rules, Raster

    LUPArrayContainer   *line_LAC;
    LUPArrayContainer   *areaPlain_LAC;
    LUPArrayContainer   *areaSymbol_LAC;
    LUPArrayContainer   *pointSimple_LAC;
    LUPArrayContainer   *pointPaper_LAC;
    
    wxArrayOfLUPrec *condSymbolLUPArray; // Dynamic Conditional Symbology

    wxArrayPtrVoid *pOBJLArray; // Used for Display Filtering
    std::vector<QString> OBJLDescriptions;

    RuleHash *_symb_sym; // symbol symbolisation rules
    MyNatsurHash m_natsur_hash;     // hash table for cacheing NATSUR string values from int attributes

    QRect m_last_clip_rect;
    int m_myConfig;
    
    double lastLightLat;
    double lastLightLon;
    
private:
    int S52_load_Plib( const QString& PLib, bool b_forceLegacy );
    bool S52_flush_Plib();
    
    void PLIB_LoadS57Config();
    
    bool PreloadOBJLFromCSV(const QString &csv_file);

    int DoRenderObject(ObjRazRules *rzRules, ViewPort *vp );
    int DoRenderObjectTextOnly(ObjRazRules *rzRules, ViewPort *vp );
    
    //    Area Renderers
    int RenderToBufferAC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp,
        render_canvas_parms *pb_spec );
    int RenderToBufferAP( ObjRazRules *rzRules, Rules *rules, ViewPort *vp,
	render_canvas_parms *pb_spec );
    int RenderToGLAC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderToGLAP( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );

    //    Object Renderers
    int RenderTX( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderTE( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderSY( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderLS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderLC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderMPS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderCARC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    char *RenderCS( ObjRazRules *rzRules, Rules *rules );
    int RenderGLLS( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderGLLC( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    
    int RenderCARC_VBO( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    
    void UpdateOBJLArray( S57Obj *obj );

    int reduceLOD( double LOD_meters, int nPoints, double *source, zchxPointF **dest);
    
    int RenderLSLegacy( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderLCLegacy( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderGLLSLegacy( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderGLLCLegacy( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderLSPlugIn( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    int RenderLCPlugIn( ObjRazRules *rzRules, Rules *rules, ViewPort *vp );
    
    render_canvas_parms* CreatePatternBufferSpec( ObjRazRules *rzRules,
        Rules *rules, ViewPort *vp, bool b_revrgb, bool b_pot = false );

    void RenderToBufferFilledPolygon( ObjRazRules *rzRules, S57Obj *obj,
        S52color *c, render_canvas_parms *pb_spec,
        render_canvas_parms *patt_spec, ViewPort *vp );

    void draw_lc_poly(QColor &color, int width, zchxPoint *ptp,
        int npt, float sym_len, float sym_factor, Rule *draw_rule,
        ViewPort *vp );

    bool RenderHPGL( ObjRazRules *rzRules, Rule * rule_in, zchxPoint &r,
        ViewPort *vp, float rot_angle = 0. );
    bool RenderRasterSymbol( ObjRazRules *rzRules, Rule *prule, zchxPoint &r,
        ViewPort *vp, float rot_angle = 0. );
    QImage RuleXBMToImage( Rule *prule );

    bool RenderText(S52_TextC *ptext, int x, int y,
        QRect *pRectDrawn, S57Obj *pobj, bool bCheckOverlap, ViewPort *vp );

    bool CheckTextRectList( const QRect &test_rect, S52_TextC *ptext );
    int RenderT_All( ObjRazRules *rzRules, Rules *rules, ViewPort *vp,	bool bTX );

    int PrioritizeLineFeature( ObjRazRules *rzRules, int npriority );

    int dda_tri( zchxPoint *ptp, S52color *c, render_canvas_parms *pb_spec,
        render_canvas_parms *pPatt_spec );
    int dda_trap( zchxPoint *segs, int lseg, int rseg, int ytop, int ybot,
        S52color *c, render_canvas_parms *pb_spec, render_canvas_parms *pPatt_spec );

    LUPrec *FindBestLUP( wxArrayOfLUPrec *LUPArray, unsigned int startIndex, unsigned int count,
                              S57Obj *pObj, bool bStrict );
    
    void SetGLClipRect(const ViewPort &vp, const QRect &rect);
    
    char *_getParamVal( ObjRazRules *rzRules, char *str, char *buf, int bsz );
    S52_TextC *S52_PL_parseTX( ObjRazRules *rzRules, Rules *rules, char *cmd );
    char *_parseTEXT( ObjRazRules *rzRules, S52_TextC *text, char *str0 );
    S52_TextC *S52_PL_parseTE( ObjRazRules *rzRules, Rules *rules, char *cmd );
    
    
    Rules *StringToRules( const QString& str_in );
    void GetAndAddCSRules( ObjRazRules *rzRules, Rules *rules );

    void DestroyPattRules( RuleHash *rh );
    void DestroyRules( RuleHash *rh );
    void DestroyLUPArray( wxArrayOfLUPrec *pLUPArray );

    bool TextRenderCheck( ObjRazRules *rzRules );
    bool inter_tri_rect( zchxPoint *ptp, render_canvas_parms *pb_spec );

    bool GetPointPixArray( ObjRazRules *rzRules, zchxPointF* pd, zchxPoint *pp, int nv, ViewPort *vp );
    bool GetPointPixSingle( ObjRazRules *rzRules, float north, float east, zchxPoint *r, ViewPort *vp );
    void GetPixPointSingle( int pixx, int pixy, double *plat, double *plon, ViewPort *vp );
    void GetPixPointSingleNoRotate( int pixx, int pixy, double *plat, double *plon, ViewPort *vpt );
    
    QString m_plib_file;

    float canvas_pix_per_mm; // Set by parent, used to scale symbols/lines/patterns
    double m_rv_scale_factor;
    
    S52color m_unused_color;
    QColor m_unused_QColor;

    bool bUseRasterSym;
    bool useLegacyRaster;

//    QPainter *m_pdc; // The current DC
    
//#ifdef ocpnUSE_GL
    QGLContext *m_glcc;
//#endif

    int *ledge;
    int *redge;

    int m_colortable_index;
    int m_colortable_index_save;

    TextObjList m_textObjList;

    QString m_ColorScheme;

    bool m_lightsOff;
    bool m_anchorOn;
    bool m_qualityOfDataOn;

    long m_state_hash;

    bool m_txf_ready;
    int m_txf_avg_char_width;
    int m_txf_avg_char_height;
    CARC_Hash m_CARC_hashmap;
    CARC_DL_Hash m_CARC_DL_hashmap;
    RenderFromHPGL* HPGL;

    TexFont *m_txf;
    
    bool m_benableGLLS;
    DisCat m_nDisplayCategory;
    ArrayOfNoshow m_noshow_array;
    ArrayOfNoshow m_saved_noshow;
    
    int m_coreVersionMajor;
    int m_coreVersionMinor;
    int m_coreVersionPatch;

    // GL Options, set by core depending on hardware capability
    bool m_useStencil;
    bool m_useStencilAP;
    bool m_useScissors;
    bool m_useFBO;
    bool m_useVBO;
    int  m_TextureFormat;
    bool m_GLLineSmoothing;
    bool m_GLPolygonSmoothing;
};


#define HPGL_FILLED true

class RenderFromHPGL {
public:
    RenderFromHPGL( s52plib* plibarg );
    ~RenderFromHPGL(  );
    
    void SetTargetDC( QPainter* pdc );
    void SetTargetOpenGl();
#if wxUSE_GRAPHICS_CONTEXT
    void SetTargetGCDC( wxGCDC* gdc );
#endif
    bool Render(char *str, char *col, zchxPoint &r, zchxPoint &pivot, zchxPoint origin, float scale, double rot_angle, bool bSymbol);

private:
    const char* findColorNameInRef( char colorCode, char* col );
    void RotatePoint( zchxPoint& point, zchxPoint origin, double angle );
    zchxPoint ParsePoint( QString& argument );
    void SetPen();
    void Line( zchxPoint from, zchxPoint to );
    void Circle( zchxPoint center, int radius, bool filled = false );
    void Polygon();

    s52plib* plib;
    double scaleFactor;

    QPainter* targetDC;
#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC* targetGCDC;
#endif

    QColor penColor;
    QPen    *mPen;
    QColor brushColor;
    QBrush  *mBrush;
    long penWidth;
    int transparency;
    
    int noPoints;
    zchxPoint polygon[100];
    
    float m_currentColor[4];

    bool renderToDC;
    bool renderToOpenGl;
    bool renderToGCDC;
};

#endif //_S52PLIB_H_
