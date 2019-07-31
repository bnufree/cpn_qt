/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Canvas Options Window/Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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


#ifndef __canvasoption_H__
#define __canvasoption_H__

#include "chart1.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
enum {
    ID_SHOWDEPTHUNITSBOX1 = 31000,
    ID_OUTLINECHECKBOX1,
    ID_CHECK_DISPLAYGRID,
    ID_ZTCCHECKBOX,
    ID_SMOOTHPANZOOMBOX,
    ID_PRESERVECHECKBOX,
    ID_QUILTCHECKBOX1,
    ID_CHECK_LOOKAHEAD,
    ID_COURSEUPCHECKBOX,
    ID_TIDES_CHECKBOX,
    ID_CURRENTS_CHECKBOX,
    ID_ENCTEXT_CHECKBOX1,
    ID_TOOLBARCHECKBOX,
    ID_CODISPCAT,
    ID_ENCDEPTH_CHECKBOX1,
    ID_ENCBUOY_CHECKBOX1,
    ID_ENCBUOYLABEL_CHECKBOX1,
    ID_ENCANCHOR_CHECKBOX1,
    ID_SHOW_AIS_CHECKBOX,
    ID_ATTEN_AIS_CHECKBOX,
};

class MyFrame;
class ChartCanvas;

//----------------------------------------------------------------------------
// CanvasOptions
//----------------------------------------------------------------------------

class  CanvasOptions: public wxDialog
{

public:
    CanvasOptions(wxWindow *parent);

    void OnClose( wxCloseEvent& event );
    void OnOptionChange( wxCommandEvent &event);
    
    void RefreshControlValues( void );
    void UpdateCanvasOptions( void );
    void OnEraseBackground( wxEraseEvent& event );
    void SetENCAvailable( bool avail );
    
private:
    int m_style;
    bool m_ENCAvail;
    wxScrolledWindow *m_sWindow;
    
    wxCheckBox *pShowStatusBar, *pShowMenuBar, *pShowChartBar, *pShowCompassWin;
    wxCheckBox *pPrintShowIcon, *pCDOOutlines, *pSDepthUnits, *pSDisplayGrid;
    wxCheckBox *pAutoAnchorMark, *pCDOQuilting, *pCBRaster, *pCBVector;
    wxCheckBox *pCBCM93, *pCBLookAhead, *pSkewComp, *pOpenGL, *pSmoothPanZoom;
    wxCheckBox *pFullScreenQuilt, *pMobile, *pResponsive, *pOverzoomEmphasis;
    wxCheckBox *pOZScaleVector, *pToolbarAutoHideCB, *pInlandEcdis, *pDarkDecorations;
    wxTextCtrl *pCOGUPUpdateSecs, *m_pText_OSCOG_Predictor, *pScreenMM;
    wxTextCtrl *pToolbarHideSecs, *m_pText_OSHDT_Predictor;
    wxChoice *m_pShipIconType, *m_pcTCDatasets;
    wxSlider *m_pSlider_Zoom, *m_pSlider_GUI_Factor, *m_pSlider_Chart_Factor, *m_pSlider_Ship_Factor;
    wxSlider *m_pSlider_Zoom_Vector;
    wxRadioButton *pCBCourseUp, *pCBNorthUp, *pRBSizeAuto, *pRBSizeManual;
    wxCheckBox *pEnableZoomToCursor, *pPreserveScale;
    
    wxCheckBox *pCDOTides, *pCDOCurrents;
    wxCheckBox *pCDOENCText, *pCBToolbar;
    wxChoice *m_pDispCat;
    wxCheckBox *pCBENCDepth, *pCBENCLightDesc, *pCBENCBuoyLabels, *pCBENCLights, *pCBENCAnchorDetails;
    wxCheckBox *pCBShowAIS, *pCBAttenAIS;
    
        DECLARE_EVENT_TABLE()

};

#endif //guard
