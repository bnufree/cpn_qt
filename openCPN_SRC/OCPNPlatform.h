/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Platform specific support utilities
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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

#ifndef OCPNPLATFORM_H
#define OCPNPLATFORM_H

#include <stdio.h>
#include <QFileDialog>

class zchxConfig;

typedef struct {
    char    tsdk[20];
    char    hn[20];
    char    msdk[20];
} PlatSpec;

//--------------------------------------------------------------------------
//      Per-Platform Utility support
//--------------------------------------------------------------------------

#ifdef __WXQT__
extern bool LoadQtStyleSheet(QString &sheet_file);
extern QString getQtStyleSheet( void );
#endif


class OCPNPlatform
{
private:
    OCPNPlatform();
public:
    static OCPNPlatform* instance();
    ~OCPNPlatform();

    //获取核心数
    int getCpuCorNum() const {return mCpuCoreNum;}

    void Initialize_2( void );

    void Initialize_3( void );

    //  Called from MyApp() just before end of MyApp::OnInit()
    static void Initialize_4( void );
    
    static void OnExit_1( void );
    static void OnExit_2( void );
    

    void SetDefaultOptions( void );
    
//--------------------------------------------------------------------------
//      Platform Display Support
//--------------------------------------------------------------------------
    QCursor ShowBusySpinner( Qt::CursorShape old = Qt::ArrowCursor );
    QCursor HideBusySpinner( void );
    double getFontPointsperPixel( void );
    QSize getDisplaySize();
    double GetDisplaySizeMM();
    void SetDisplaySizeMM( double size );
    double GetDisplayDPmm();
    unsigned int GetSelectRadiusPix();
    double GetToolbarScaleFactor( int GUIScaleFactor );
    double GetCompassScaleFactor( int GUIScaleFactor );
    void onStagedResizeFinal();
    float getChartScaleFactorExp( float scale_linear );
    int GetStatusBarFieldCount();
    bool GetFullscreen();
    bool SetFullscreen( bool bFull );
    double GetDisplayDensityFactor();
    
    double m_pt_per_pixel;
//--------------------------------------------------------------------------
//      Per-Platform file/directory support
//----------------------
    QString GetConfigFileName();
    QString GetSupplementalLicenseString();
    QString NormalizePath(const QString &full_path); //Adapt for portable use
    
    bool DoFileSelectorDialog( QWidget *parent, QString *file_spec, QString Title, QString initDir, QString suggestedName, QString wildcard);
    int DoDirSelectorDialog( QWidget *parent, QString *file_spec, QString Title, QString initDir);
    
//--------------------------------------------------------------------------
//      Per-Platform OpenGL support
//--------------------------------------------------------------------------
    bool BuildGLCaps( void *pbuf );
    bool IsGLCapable();
private:
    void initSystemInfo();

private:
    static OCPNPlatform     *minstance;

    class MGarbage
    {
    public:
        ~MGarbage()
        {
            if (OCPNPlatform::minstance)
                delete OCPNPlatform::minstance;
        }
    };
    static MGarbage Garbage;

private:
    bool        GetWindowsMonitorSize( int& width, int& height);
    QSize      m_displaySize;
    QSize      m_displaySizeMM;
    int         m_displaySizeMMOverride;
    bool        m_bdisableWindowsDisplayEnum;
    int         mCpuCoreNum;
    Qt::CursorShape         mOldShape;
};


#endif          //guard
