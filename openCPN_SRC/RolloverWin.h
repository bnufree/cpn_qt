/***************************************************************************
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
 **************************************************************************/

#ifndef __ROLLOVERWIN_H__
#define __ROLLOVERWIN_H__


//constants for rollovers fonts
enum
{
    AIS_ROLLOVER =1,
    LEG_ROLLOVER =2,
    TC_ROLLOVER  =3
};

#include <QWidget>

class RolloverWin: public QWidget
{
public:
    RolloverWin( QWidget *parent, int timeout = -1, bool maincanvas = true );
    ~RolloverWin();


    void Draw(ocpnDC &dc);

    void SetColorScheme( ColorScheme cs );
    void SetString(const QString &s) { m_string = s; }
    void SetPosition( QPoint pt ) { m_position = pt; }
    void SetBitmap( int rollover );
    QBitmap* GetBitmap() { return m_pbm; }
    void SetBestPosition( int x, int y, int off_x, int off_y, int rollover, QSize parent_size );
    void SetMousePropogation( int level ) { m_mmouse_propogate = level; }
    bool IsActive() { return isActive; }
    void IsActive( bool state ) { isActive = state; }
public slots:
    void OnTimer();
protected:
    void paintEvent( QPaintEvent* event );
    //void mouseEvent( QMouseEvent* event );

private:
    QString m_string;
    QSize m_size;
    QPoint m_position;
    QBitmap *m_pbm;
    QTimer  *m_timer_timeout;
    int m_timeout_sec;
    int m_mmouse_propogate;
    unsigned int m_texture;
    bool isActive;
    QFont *m_plabelFont;
    bool m_bmaincanvas;
};


#endif
