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

#ifndef __CHINFOWIN_H__
#define __CHINFOWIN_H__

#include <QWidget>
#include "_def.h"
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>

class ChInfoWin: public QWidget
{
    Q_OBJECT
public:
    ChInfoWin( QWidget *parent = 0 );
    ~ChInfoWin();

    void SetString(const QString &s){ m_string = s; }
    const QString& GetString(void) { return m_string; }
    
    void SetPosition( zchxPoint pt )
    {
        m_position = pt;
    }
    void SetWinSize( QSize sz )
    {
        m_size = sz;
    }
    void SetBitmap( void );
    void FitToChars( int char_width, int char_height );
    QSize GetWinSize( void )
    {
        return m_size;
    }

    QLabel *m_pInfoTextCtl;
    int dbIndex;

protected:
    void mousePressEvent(QMouseEvent* event );
    void paintEvent( QPaintEvent* event );


private:

    QString m_string;
    QSize m_size;
    zchxPoint m_position;
};


#endif
