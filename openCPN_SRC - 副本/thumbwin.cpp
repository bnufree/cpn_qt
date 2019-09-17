/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Thumbnail Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *
 */

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "dychart.h"

#include "thumbwin.h"
//#include "chart1.h"
#include "chartdb.h"
#include "wx28compat.h"
#include <QPainter>

//------------------------------------------------------------------------------
//    Thumbwin Implementation
//------------------------------------------------------------------------------

// Define a constructor

ThumbWin::ThumbWin( QWidget *parent ) :
        QWidget( parent),
        m_max_size(100, 100)
{
    pThumbChart = NULL;
    setVisible(false);
}

ThumbWin::~ThumbWin()
{
}

void ThumbWin::Resize( void )
{
    if( pThumbChart ) {
        if( pThumbChart->GetThumbData()->pDIBThumb ) {
            int newheight = std::min(m_max_size.height(), pThumbChart->GetThumbData()->pDIBThumb->height());
            int newwidth = std::min(m_max_size.width(), pThumbChart->GetThumbData()->pDIBThumb->width());
            resize(newwidth, newheight );
        }
    }
}

void ThumbWin::SetMaxSize( QSize const &max_size )
{
    m_max_size = max_size;
}


void ThumbWin::paintEvent(QPaintEvent *event)
{
    QPainter dc( this );
    if(!pThumbChart )  return;
    if(!(pThumbChart->GetThumbData()) ) return;
    if( pThumbChart->GetThumbData()->pDIBThumb )
    {
        dc.drawPixmap(QPoint(0, 0), *( pThumbChart->GetThumbData()->pDIBThumb->GetHandle() ));
    }

    QPen ppPen( GetGlobalColor("CHBLK"), 1, Qt::SolidLine );
    dc.setPen( ppPen );
    QBrush yBrush( GetGlobalColor("CHYLW" ), Qt::SolidPattern );
    dc.setBrush( yBrush );
    dc.drawEllipse(pThumbChart->GetThumbData()->ShipX, pThumbChart->GetThumbData()->ShipY, 6, 6 );
}

const wxBitmap &ThumbWin::GetBitmap(void)
{
    if( pThumbChart ) {
        if( pThumbChart->GetThumbData() ) {
            if( pThumbChart->GetThumbData()->pDIBThumb )
                m_bitmap =  *( pThumbChart->GetThumbData()->pDIBThumb );
        }
    }
    
    return m_bitmap;
}
     
