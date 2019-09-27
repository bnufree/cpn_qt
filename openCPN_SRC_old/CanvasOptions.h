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

#include <QScrollArea>
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
class QCheckBox;
class QLineEdit;
class QSlider;
class QRadioButton;
class QComboBox;

//----------------------------------------------------------------------------
// CanvasOptions
//----------------------------------------------------------------------------

class  CanvasOptions: public QWidget
{
    Q_OBJECT
public:
    CanvasOptions(QWidget *parent = 0);
    void RefreshControlValues( void );
    void UpdateCanvasOptions( void );
    void SetENCAvailable( bool avail );
public slots:
    void OnClose();
    void OnOptionChange();
    
private:
    int m_style;
    bool m_ENCAvail;
    QScrollArea *m_sWindow;
    
    QCheckBox *pShowStatusBar, *pShowMenuBar, *pShowChartBar, *pShowCompassWin;
    QCheckBox *pPrintShowIcon, *pCDOOutlines, *pSDepthUnits, *pSDisplayGrid;
    QCheckBox *pAutoAnchorMark, *pCDOQuilting, *pCBRaster, *pCBVector;
    QCheckBox *pCBCM93, *pCBLookAhead, *pSkewComp, *pOpenGL, *pSmoothPanZoom;
    QCheckBox *pFullScreenQuilt, *pMobile, *pResponsive, *pOverzoomEmphasis;
    QCheckBox *pOZScaleVector, *pToolbarAutoHideCB, *pInlandEcdis, *pDarkDecorations;
    QLineEdit *pCOGUPUpdateSecs, *m_pText_OSCOG_Predictor, *pScreenMM;
    QLineEdit *pToolbarHideSecs, *m_pText_OSHDT_Predictor;
    QComboBox *m_pShipIconType, *m_pcTCDatasets;
    QSlider *m_pSlider_Zoom, *m_pSlider_GUI_Factor, *m_pSlider_Chart_Factor, *m_pSlider_Ship_Factor;
    QSlider *m_pSlider_Zoom_Vector;
    QRadioButton *pCBCourseUp, *pCBNorthUp, *pRBSizeAuto, *pRBSizeManual;
    QCheckBox *pEnableZoomToCursor, *pPreserveScale;
    
    QCheckBox *pCDOTides, *pCDOCurrents;
    QCheckBox *pCDOENCText, *pCBToolbar;
    QComboBox *m_pDispCat;
    QCheckBox *pCBENCDepth, *pCBENCLightDesc, *pCBENCBuoyLabels, *pCBENCLights, *pCBENCAnchorDetails;
    QCheckBox *pCBShowAIS, *pCBAttenAIS;

};

#endif //guard
