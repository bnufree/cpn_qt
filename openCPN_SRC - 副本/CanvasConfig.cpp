/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Canvas Configuration
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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

#include "CanvasConfig.h"
#include "ocpn_plugin.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

//------------------------------------------------------------------------------
// canvasConfig Implementation
//------------------------------------------------------------------------------
canvasConfig::canvasConfig( )
{
    configIndex = -1;
    canvas = NULL;
    GroupID = 0;
    iLat = 0.;
    iLon = 0.;
    iScale = .0003;        // decent initial value
    iRotation = 0.;
}

canvasConfig::canvasConfig( int index )
{
    configIndex = index;
    canvas = NULL;
    GroupID = 0;
    iLat = 0.;
    iLon = 0.;
    iScale = .0003;        // decent initial value
    iRotation = 0.;
}

canvasConfig::~canvasConfig(){}


void canvasConfig::Reset( void){
        bFollow = false;
        bShowTides = false;
        bShowCurrents = false;
        bCourseUp = false;
        bLookahead = false;
        bShowAIS = true;
        bAttenAIS = false;
        bQuilt = true;
        nENCDisplayCategory = (int)(enum _DisCat) STANDARD;

}

void canvasConfig::LoadFromLegacyConfig( QSettings *conf )
{
    if(!conf)       return;

    bFollow = false;
    bShowAIS = true;
    
    //S52 stuff
    conf->beginGroup("Settings/GlobalState" );
    bShowENCText = conf->value("bShowS57Text", 0).toBool();
    bShowENCLightDescriptions = conf->value("bShowLightDescription", 0).toBool();
    nENCDisplayCategory = conf->value("nDisplayCategory", (enum _DisCat) STANDARD ).toInt();
    bShowENCDepths = conf->value("bShowSoundg", 1 ).toBool();
    bShowENCBuoyLabels = conf->value("bShowAtonText", 1).toBool();
    bShowENCLights = true;
    conf->endGroup();

    conf->beginGroup("Settings/AIS");
    bAttenAIS = conf->value("bShowScaledTargets" , 0 ).toBool();
    conf->endGroup();

    conf->beginGroup("Settings" );
    bShowTides = conf->value("ShowTide", 0 ).toBool();
    bShowCurrents = conf->value("ShowCurrent",0 ).toBool();
    bCourseUp = conf->value("CourseUpMode",0 ).toBool();
    bLookahead = conf->value("LookAheadMode",0 ).toBool();
    bShowGrid = conf->value("ShowGrid",0 ).toBool();
    bShowOutlines = conf->value("ShowChartOutlines",1 ).toBool();
    bShowDepthUnits = conf->value("ShowDepthUnits",1 ).toBool();
    bQuilt = conf->value("ChartQuilting",1 ).toBool();
    GroupID = conf->value("ActiveChartGroup",0 ).toInt();
    DBindex = conf->value("InitialdBIndex", -1 ).toInt();
    conf->endGroup();

    conf->beginGroup("settings/GlobalState" );
    if(conf->contains("VPScale"))
    {
        double st_view_scale = conf->value("VPScale").toDouble();
        st_view_scale = fmax ( st_view_scale, .001/32 );
        st_view_scale = fmin ( st_view_scale, 4 );
        iScale = st_view_scale;
    }
    if(conf->contains("VPRotation"))
    {
        double st_rotation = conf->value("VPRotation").toDouble();
        st_rotation = fmin ( st_rotation, 360 );
        st_rotation = fmax ( st_rotation, 0 );
        iRotation = st_rotation * PI / 180.;
    }
    if(conf->contains("VPLatLon"))
    {
        double lat, lon;
        QStringList list = conf->value("VPLatLon").toString().split(",", QString::SkipEmptyParts);
        if(list.size() >= 2)
        {
            lat = list[0].toDouble();
            lon = list[1].toDouble();

            //    Sanity check the lat/lon...both have to be reasonable.
            if( fabs( lon ) < 360. ) {
                while( lon < -180. )
                    lon += 360.;

                while( lon > 180. )
                    lon -= 360.;

                iLon = lon;
            }

            if( fabs( lat ) < 90.0 )
                iLat = lat;
        }
    }
    conf->endGroup();

}



