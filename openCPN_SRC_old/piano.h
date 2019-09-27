/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Bar Window
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
 ***************************************************************************
 */


#ifndef __statwin_H__
#define __statwin_H__

//#include "chart1.h"
#include "bitmap.h"
#include "_def.h"
#include <QPainter>
#include "GL/gl.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

#define PIANO_EVENT_TIMER  73566
#define DEFERRED_KEY_CLICK_DOWN 1
#define DEFERRED_KEY_CLICK_UP 2
#define INFOWIN_TIMEOUT 3

// Class declarations
typedef QList<QRect> RectArray;

class zchxMapMainWindow;
class ChartCanvas;
class ocpnDC;

//----------------------------------------------------------------------------
// Piano
//----------------------------------------------------------------------------
class Piano /*: public wxEvtHandler*/:public QObject
{
    Q_OBJECT
public:
      Piano( ChartCanvas *parent );
      ~Piano();
      void DrawGL(int y);
      void FormatKeys(void);
      bool MouseEvent(QMouseEvent* event);
      void SetColorScheme(ColorScheme cs);
      void SetKeyArray(std::vector<int> piano_chart_index_array);
      void SetActiveKey(int iactive) { m_iactive = iactive; }
      void SetActiveKeyArray(std::vector<int> array);
      void SetNoshowIndexArray(std::vector<int> array);
      void SetEclipsedIndexArray(std::vector<int> array);
      void SetSkewIndexArray(std::vector<int> array);
      void SetTmercIndexArray(std::vector<int> array);
      void SetPolyIndexArray(std::vector<int> array);

      std::vector<int>  GetActiveKeyArray() { return m_active_index_array; }

      void SetVizIcon(wxBitmap *picon_bmp){ if( m_pVizIconBmp ) delete m_pVizIconBmp; m_pVizIconBmp = picon_bmp; }
      void SetInVizIcon(wxBitmap *picon_bmp){ if( m_pInVizIconBmp ) delete m_pInVizIconBmp; m_pInVizIconBmp = picon_bmp; }
      void SetSkewIcon(wxBitmap *picon_bmp){ if( m_pSkewIconBmp ) delete m_pSkewIconBmp; m_pSkewIconBmp = picon_bmp; }
      void SetTMercIcon(wxBitmap *picon_bmp){ if( m_pTmercIconBmp ) delete m_pTmercIconBmp; m_pTmercIconBmp = picon_bmp; }
      void SetPolyIcon(wxBitmap *picon_bmp){ if( m_pPolyIconBmp ) delete m_pPolyIconBmp; m_pPolyIconBmp = picon_bmp; }
      void ShowBusy( bool busy );

      
      zchxPoint GetKeyOrigin(int key_index);
      void ResetRollover(void);
      void SetRoundedRectangles(bool val){ m_brounded = val; m_hash.clear();}

      int GetHeight();
      int GetWidth();
      
      QString &GenerateAndStoreNewHash();
      QString &GetStoredHash();
      
      int GetnKeys(){ return m_nRegions; }
public slots:
      void onTimerEvent();
      
private:
      void BuildGLTexture();
      bool InArray(std::vector<int> &array, int key);

      QString GetStateHash();
      QString    m_hash;
      
      ChartCanvas *m_parentCanvas;
      
      int         m_nRegions;
      int         m_index_last;
      int         m_hover_icon_last;
      int         m_hover_last;

      QBrush     m_backBrush;
      QBrush     m_srBrush, m_rBrush;
      QBrush     m_svBrush, m_vBrush;
      QBrush     m_unavailableBrush;
      QBrush     m_utileBrush, m_tileBrush;

      QBrush     m_cBrush;
      QBrush     m_scBrush;

      std::vector<int> m_key_array;
      std::vector<int> m_noshow_index_array;
      std::vector<int> m_active_index_array;
      std::vector<int> m_eclipsed_index_array;
      std::vector<int> m_skew_index_array;
      std::vector<int> m_tmerc_index_array;
      std::vector<int> m_poly_index_array;
      bool        m_bBusy;
      QTimer     *m_eventTimer;
      int         m_click_sel_index;
      int         m_click_sel_dbindex;
      int         m_action;
      
      RectArray KeyRect;
      
      wxBitmap    *m_pVizIconBmp;
      wxBitmap    *m_pInVizIconBmp;
      wxBitmap    *m_pTmercIconBmp;
      wxBitmap    *m_pSkewIconBmp;
      wxBitmap    *m_pPolyIconBmp;

      int         m_iactive;
      bool        m_brounded;
      bool        m_bleaving;

      GLuint      m_tex, m_texw, m_texh, m_tex_piano_height;
      int         m_width;
};

////----------------------------------------------------------------------------
//// ChartBarWin
////----------------------------------------------------------------------------
//class ChartBarWin: public QWidget
//{
//    Q_OBJECT
//public:
//      ChartBarWin(wxWindow *win);
//      ~ChartBarWin();

//protected:
//      void OnSize(wxSizeEvent& event);
//      void OnPaint(wxPaintEvent& event);
//      void MouseEvent(wxMouseEvent& event);
//      int  GetFontHeight();
//      void RePosition();
//      void ReSize();
//};

#endif
