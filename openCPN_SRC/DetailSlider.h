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

#ifndef __DETAILSLIDE_H__
#define __DETAILSLIDE_H__

//#include "chcanv.h"
#include "chart1.h"

class QSlider;
class PopUpDSlide: public QWidget
{
public:
    PopUpDSlide( QWidget* parent,  ChartTypeEnum ChartType, ChartFamilyEnum ChartF,
            const QPoint& pos = QPoint(-1, -1), const QSize& size = QSize(-1, -1),
            long style = Qt::Dialog, const QString& title = ("") );

    ~PopUpDSlide( void );

    void Init( void );
    bool Create(QWidget* parent,  ChartTypeEnum ChartType, ChartFamilyEnum ChartF,
            const QPoint& pos, const QSize& size, long style, const QString& title );
    QSlider *m_p_DetailSlider;
    QWidget *m_pparent;

public slots:
    void OnCancelClick();
    void OnChangeValue();
protected:
    void keyPressEvent(QKeyEvent *);
    void mouseMoveEvent(QMoveEvent* event );
    void closeEvent( QCloseEvent* event );


private:
    
    ChartTypeEnum ChartType;
    ChartFamilyEnum ChartFam;

};

#endif //__DETAILSLIDE_H__
