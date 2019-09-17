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

#ifndef __GOTOPOSITIONDIALOG_H__
#define __GOTOPOSITIONDIALOG_H__

#include <QSize>
#include <QPoint>
#include <QDialog>

class ChartCanvas;
class QLineEdit;
class QPushButton;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_GOTOPOS 8100
#define SYMBOL_GOTOPOS_STYLE (Qt::WindowTitleHint|Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint)
#define SYMBOL_GOTOPOS_TITLE "Center view"
#define SYMBOL_GOTOPOS_IDNAME ID_GOTOPOS
#define SYMBOL_GOTOPOS_SIZE (QSize(200, 300))
#define SYMBOL_GOTOPOS_POSITION (QPoint(-1, -1))
#define ID_GOTOPOS_CANCEL 8101
#define ID_GOTOPOS_OK 8102


////@end control identifiers

/*!
 * GoToPositionDialog class declaration
 */
class GoToPositionDialog: public QDialog
{
    Q_OBJECT
public:
    /// Constructors
    GoToPositionDialog( );
    GoToPositionDialog( QWidget* parent,
                        const QString& caption = SYMBOL_GOTOPOS_TITLE,
                        const QPoint& pos = SYMBOL_GOTOPOS_POSITION,
                        const QSize& size = SYMBOL_GOTOPOS_SIZE,
                        Qt::WindowFlags flag = SYMBOL_GOTOPOS_STYLE );

    ~GoToPositionDialog();

    /// Creation
    bool Create( QWidget* parent,
                 const QString& caption = SYMBOL_GOTOPOS_TITLE,
                 const QPoint& pos = SYMBOL_GOTOPOS_POSITION,
                 const QSize& size = SYMBOL_GOTOPOS_SIZE,
                 Qt::WindowFlags flag = SYMBOL_GOTOPOS_STYLE);

    void SetCanvas( ChartCanvas *canvas ){ m_hostCanvas = canvas; }
    void SetColorScheme(ColorScheme cs);

    void CreateControls();
    void CheckPasteBufferForPosition();

    /// Should we show tooltips?
    static bool ShowToolTips();

    QLineEdit*   m_MarkLatCtl;
    QLineEdit*   m_MarkLonCtl;
    QPushButton*     m_CancelButton;
    QPushButton*     m_OKButton;

    double        m_lat_save;
    double        m_lon_save;
    ChartCanvas   *m_hostCanvas;
public slots:
    void OnGoToPosCancelClick();
    void OnGoToPosOkClick();
    void OnPositionCtlUpdated();
};

#endif
