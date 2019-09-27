﻿/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __QUIT_H__
#define __QUIT_H__

#include "LLRegion.h"
#include "OCPNRegion.h"
#include "chcanv.h"

struct ChartTableEntry;

class QuiltPatch
{
public:
    QuiltPatch()
    {
        b_Valid = false;
        b_eclipsed = false;
        b_overlay = false;
    }
    int dbIndex;
    LLRegion ActiveRegion;
    int ProjType;
    bool b_Valid;
    bool b_eclipsed;
    bool b_overlay;
    LLRegion quilt_region;
};

class QuiltCandidate
{
public:
    QuiltCandidate()
    {
        b_include = false;
        b_eclipsed = false;
        b_locked = false;
        last_factor = -1;
    }

    const LLRegion &GetCandidateRegion();
    LLRegion &GetReducedCandidateRegion(double factor);
    void SetScale(int scale);
    bool Scale_eq( int b ) const { return abs ( ChartScale - b) <= rounding; }
    bool Scale_ge( int b ) const { return  Scale_eq( b ) || ChartScale > b; }
    
    int dbIndex;
    int ChartScale;
    int rounding;
    bool b_include;
    bool b_eclipsed;
    bool b_locked;

private:
    double last_factor;
    LLRegion reduced_candidate_region;

};

typedef QList<QuiltPatch*>   PatchList;
typedef QList< QuiltCandidate*> ArrayOfSortedQuiltCandidates;
typedef QuiltPatch*        PatchListNode;

class Quilt
{
public:

    Quilt( ChartFrameWork *parent);
    ~Quilt();

    void SetQuiltParameters( double CanvasScaleFactor, int CanvasWidth )
    {
        m_canvas_scale_factor = CanvasScaleFactor;
        m_canvas_width = CanvasWidth;
    }

    void EnableHighDefinitionZoom( bool value ) { m_b_hidef = value;}
    
    void UnlockQuilt();
    bool Compose( const ViewPort &vp );
    bool IsComposed() {
        return m_bcomposed;
    }
    ChartBase *GetFirstChart();
    ChartBase *GetNextChart();
    ChartBase *GetLargestScaleChart();
    ChartBase *GetNextSmallerScaleChart();
    
    std::vector<int> GetQuiltIndexArray( void );
    bool IsQuiltDelta( ViewPort &vp );
    bool IsChartQuiltableRef( int db_index );
    ViewPort &GetQuiltVP() {
        return m_vp_quilt;
    }
    QString GetQuiltDepthUnit() {
        return m_quilt_depth_unit;
    }
    void SetRenderedVP( ViewPort &vp ) {
        m_vp_rendered = vp;
    }
    bool HasOverlays( void ) {
        return m_bquilt_has_overlays;
    }

    int GetExtendedStackCount(void) {
        return m_extended_stack_array.size();
    }

    int GetnCharts() {
        return m_PatchList.count();
    }
    double GetBestStartScale(int dbi_ref_hint, const ViewPort &vp_in);
    

    void ComputeRenderRegion( ViewPort &vp, OCPNRegion &chart_region );
    
    bool IsVPBlittable( ViewPort &VPoint, int dx, int dy, bool b_allow_vector = false );
    ChartBase *GetChartAtPix( ViewPort &VPoint, const zchxPoint& p );
    ChartBase *GetOverlayChartAtPix( ViewPort &VPoint, const zchxPoint& p );
    int GetChartdbIndexAtPix( ViewPort &VPoint, zchxPoint p );
    void InvalidateAllQuiltPatchs( void );
    void Invalidate( void )
    {
        m_bcomposed = false;
        m_vp_quilt.invalidate();
        m_zout_dbindex = -1;

        //  Quilting of skewed raster charts is allowed for OpenGL only
        m_bquiltskew = true;
        //  Quilting of different projections is allowed for OpenGL only
        m_bquiltanyproj = true;
    }
    void AdjustQuiltVP( ViewPort &vp_last, ViewPort &vp_proposed );

    LLRegion &GetFullQuiltRegion( void ) {
        return m_covered_region;
    }
    OCPNRegion &GetFullQuiltRenderedRegion( void ) {
        return m_rendered_region;
    }
    bool IsChartSmallestScale( int dbIndex );

    int AdjustRefOnZoomOut( double proposed_scale_onscreen );
    int AdjustRefOnZoomIn( double proposed_scale_onscreen );
    
    void SetHiliteIndex( int index ) {
        m_nHiLiteIndex = index;
    }
    void SetReferenceChart( int dbIndex ) {
        m_refchart_dbIndex = dbIndex;
        if (dbIndex >= 0) {
            m_zout_family = -1;
        }
    }
    int GetRefChartdbIndex( void ) {
        return m_refchart_dbIndex;
    }
    
    ChartBase *GetRefChart();
    
    int GetQuiltProj( void )
    {
        return m_quilt_proj;
    }
    double GetMaxErrorFactor()
    {
        return m_max_error_factor;
    }
    double GetRefScale()
    {
        return m_reference_scale;
    }
    double GetRefNativeScale();

    std::vector<int> GetCandidatedbIndexArray( bool from_ref_chart, bool exclude_user_hidden );
    std::vector<int> GetExtendedStackIndexArray()
    {
        return m_extended_stack_array;
    }
    std::vector<int> GetEclipsedStackIndexArray()
    {
        return m_eclipsed_stack_array;
    }

    unsigned long GetXStackHash() {
        return m_xa_hash;
    }

    bool IsBusy()
    {
        return m_bbusy;
    }
    QuiltPatch *GetCurrentPatch();
    bool IsChartInQuilt( ChartBase *pc );
    bool IsChartInQuilt( QString &full_path);
    
    bool IsQuiltVector( void );
    bool DoesQuiltContainPlugins( void );
    
    LLRegion GetHiliteRegion( );
    static LLRegion GetChartQuiltRegion( const ChartTableEntry &cte, ViewPort &vp );

    int GetNomScaleMin(int scale, ChartTypeEnum type, ChartFamilyEnum family);
    int GetNomScaleMax(int scale, ChartTypeEnum type, ChartFamilyEnum family);
    
private:
    bool BuildExtendedChartStackAndCandidateArray(int ref_db_index, ViewPort &vp_in);
    int AdjustRefOnZoom( bool b_zin, ChartFamilyEnum family, ChartTypeEnum type, double proposed_scale_onscreen );
    
    void EmptyCandidateArray( void );
    void SubstituteClearDC( QPainter *dc, ViewPort &vp );
    int GetNewRefChart( void );

    
    bool IsChartS57Overlay( int db_index );
    
    LLRegion m_covered_region;
    OCPNRegion m_rendered_region; // used only in dc mode

    PatchList m_PatchList;
    QBitmap *m_pBM;

    bool m_bcomposed;
    PatchListNode cnode;
    int           cnode_index;
    bool m_bbusy;
    int m_quilt_proj;

    ArrayOfSortedQuiltCandidates *m_pcandidate_array;
    std::vector<int> m_last_index_array;
    std::vector<int> m_index_array;
    std::vector<int> m_extended_stack_array;
    std::vector<int> m_eclipsed_stack_array;

    ViewPort m_vp_quilt;
    ViewPort m_vp_rendered;          // last VP rendered

    int m_nHiLiteIndex;
    int m_refchart_dbIndex;
    int m_reference_scale;
    int m_reference_type;
    int m_reference_family;
    bool m_bneed_clear;
    LLRegion m_back_region;
    QString m_quilt_depth_unit;
    double m_max_error_factor;
    double m_canvas_scale_factor;
    int m_canvas_width;
    bool m_bquilt_has_overlays;
    unsigned long m_xa_hash;
    int m_zout_dbindex;
    int m_zout_family;
    int m_zout_type;
    
    int m_lost_refchart_dbIndex;
    bool m_b_hidef;
    
    bool m_bquiltskew;
    bool m_bquiltanyproj;
    ChartFrameWork *m_parent;
    
};

#endif
