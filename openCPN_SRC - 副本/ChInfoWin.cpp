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
#include "ChInfoWin.h"
#include "OCPNPlatform.h"
#include "FontMgr.h"
#include <QPainter>
#include <QVBoxLayout>


extern bool g_btouch;

extern QColor GetGlobalColor(const QString& str);

// Define a constructor
ChInfoWin::ChInfoWin( QWidget *parent ):QWidget(parent)
{
    this->setWindowFlags(Qt::SubWindow);
    QFont dFont = FontMgr::Get().GetFont("Dialog") ;
    this->setFont(dFont);
    this->setLayout(new QVBoxLayout);
    m_pInfoTextCtl = new QLabel(this);
    m_pInfoTextCtl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->layout()->addWidget(m_pInfoTextCtl);
    dbIndex = -1;
    hide();
}

ChInfoWin::~ChInfoWin()
{
    delete m_pInfoTextCtl;
}



void ChInfoWin::mousePressEvent(QMouseEvent *event)
{
    if(g_btouch){
        if( event->button() == Qt::LeftButton) {
            hide();
        }
    }
}

    
void ChInfoWin::paintEvent(QPaintEvent* event)
{
    int width, height;
    QPainter dc( this );

    dc.setBrush(QBrush( GetGlobalColor( ( "UIBCK" ) ) ) );
    dc.setPen(QPen( GetGlobalColor( ( "UITX1" ) ) ) );
    dc.drawRect(this->rect() );
}

void ChInfoWin::SetBitmap()
{
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Window, QBrush(GetGlobalColor( "UIBCK" )));
    this->setPalette(pal);
    m_pInfoTextCtl->setStyleSheet(QString("background-color:transparent;color:%1").arg(GetGlobalColor(  "UITX1" ).name()));
    m_pInfoTextCtl->setText(m_string );

    zchxPoint top_position = m_position; //GetParent()->ClientToScreen( m_position);
    setGeometry(top_position.x, top_position.y, m_size.width(), m_size.height() );
}

// width 字符最大长度. height 行数
void ChInfoWin::FitToChars( int char_width, int char_height )
{
    QStringList list;
    for(int i=0; i<char_width; i++) list.append("A");
    QSize size;

    int adjust = 1;
    QFontMetrics mcs(this->font());
    size.setWidth(mcs.width(list.join("")));
    size.setHeight(mcs.height() * ( char_height + adjust ));
    size.setWidth(fmin(size.width(), OCPNPlatform::instance()->getDisplaySize().width()-10));
    resize(size );
    SetWinSize(size);
}

