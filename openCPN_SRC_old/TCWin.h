/******************************************************************************
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

#ifndef __TCWIN_H__
#define __TCWIN_H__

#include <QWidget>

class IDX_entry;
class ChartCanvas;
class RolloverWin;
class QLineEdit;
class QTimer;
class QPushButton;
class QListWidget;

typedef QList<QPoint> SplineList;           // for spline curve points

class TCWin: public QWidget
{
public:
      TCWin(ChartCanvas *parent, int x, int y, void *pvIDX);
      ~TCWin();
      void RePosition(void);
      void RecalculateSize();

protected:
      void resizeEvent(QResizeEvent* event);
      void paintEvent(QPaintEvent* event);
      void mousePressEvent(QMouseEvent *);
      void mouseReleaseEvent(QMouseEvent *);
      void mouseDoubleClickEvent(QMouseEvent *);
      void mouseMoveEvent(QMouseEvent *);
      void closeEvent(QCloseEvent *);
public slots:
      void MouseEvent(QMouseEvent* event);
      void OnTCWinPopupTimerEvent();
      void OKEvent();
      void NXEvent();
      void PREvent();
      void OnCloseWindow(QCloseEvent* e);



private:
    QLineEdit   *m_ptextctrl;
    QTimer      *m_TCWinPopupTimer;
    RolloverWin  *m_pTCRolloverWin;
    int           curs_x;
    int           curs_y;
    int           m_plot_type;
    QSize        m_tc_size;
    QFont        m_position; // window ULC in screen coordinates
    int           m_x;        // x coord of mouse click that launched window
    int           m_y;        // y coord of mouse click that launched window
    bool          m_created;
    int           m_tsx;      // test button width
    int           m_tsy;      // test button height
    float         m_tcwin_scaler; // factor to scale TCWin and contents by
    
      IDX_entry   *pIDX;
      QPushButton    *OK_button;
      QPushButton    *NX_button;
      QPushButton    *PR_button;

      int         im;  // span of values to graph
      int         ib;  // minimum value to graph
      int         it;  // maximum value to graph
      int         val_off; // offset
      int         i_skip; // vertical stride in graph
      QRect    m_graph_rect;


      float       tcv[26];
      time_t      tt_tcv[26];
      
      QListWidget  *m_tList ;
      bool        btc_valid;
      ChartCanvas    *pParent;
      int         m_corr_mins;
      QString    m_stz;
      int         m_t_graphday_00_at_station;
      QDateTime  m_graphday;
      int         m_plot_y_offset;

      SplineList  m_sList;

      QFont *pSFont;
      QFont *pSMFont;
      QFont *pMFont;
      QFont *pLFont;

      QPen *pblack_1;
      QPen *pblack_2;
      QPen *pblack_3;
      QPen *pred_2;
      QBrush *pltgray;
      QBrush *pltgray2;

      int         m_button_height;
      
      int xSpot;
      int ySpot;
};


#endif
