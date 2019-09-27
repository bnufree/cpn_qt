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

#ifndef __S57QUERYDIALOG_H__
#define __S57QUERYDIALOG_H__

#include <QDialog>
class QPushButton;
class QWebView;

class S57QueryDialog: public QDialog
{
    Q_OBJECT
public:

    /// Constructors

    S57QueryDialog( );
    S57QueryDialog( QWidget* parent,
                    const QPoint& pos,
                    const QSize& size,
                    const QString& caption = ("Object Query"),
                    Qt::WindowFlags f = Qt::WindowFlags() );

    ~S57QueryDialog( );
    void Init();

    bool Create( QWidget* parent,
                 const QPoint& pos,
                 const QSize& size,
                 const QString& caption = ("Object Query"),
                 Qt::WindowFlags f = Qt::WindowFlags() );

    void SetColorScheme(void);

    void CreateControls();
    void RecalculateSize( void );


    void SetHTMLPage(QString& page);

    //    Data
    QWebView      *m_phtml;
    QSize            m_createsize;
    QPushButton          *m_btnOK;

public slots:
    void OnHtmlLinkClicked();
    void OnOKClick() {close();}

protected:
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *);
    //    Overrides
    void paintEvent(QPaintEvent* e );

};

class S57ExtraQueryInfoDlg: public S57QueryDialog
{
    Q_OBJECT
public:

    /// Constructors
    S57ExtraQueryInfoDlg( );
    S57ExtraQueryInfoDlg( QWidget* parent,
                          const QPoint& pos,
                          const QSize& size,
                          const QString& caption = ("Extra Object Info"),
                          Qt::WindowFlags f = Qt::WindowFlags());
    bool Create( QWidget* parent,
                 const QPoint& pos,
                 const QSize& size,
                 const QString& caption = ("Extra Object Info"),
                 Qt::WindowFlags f = Qt::WindowFlags() );

    ~S57ExtraQueryInfoDlg( );    
    void RecalculateSize( void );

protected:
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *);
private:
    QPushButton          *m_btnOK;
};

#endif
