﻿/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Samuel R. Blackburn, David S Register           *
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
 *   S Blackburn's original source license:                                *
 *         "You can use it any way you like."                              *
 *   More recent (2010) license statement:                                 *
 *         "It is BSD license, do with it what you will"                   *
 */


#include "nmea0183.h"

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/


RMB::RMB()
{
    Mnemonic = ("RMB");
   Empty();
}

RMB::~RMB()
{
   Mnemonic.clear();
   Empty();
}

void RMB::Empty( void )
{

   IsDataValid                     = Unknown0183;
   CrossTrackError                 = 0.0;
   DirectionToSteer                = LR_Unknown;
   To.clear();
   From.clear();
   DestinationPosition.Empty();
   RangeToDestinationNauticalMiles = 0.0;
   BearingToDestinationDegreestrue = 0.0;
   DestinationClosingVelocityKnots = 0.0;
   IsArrivalCircleEntered          = Unknown0183;
}

bool RMB::Parse( const SENTENCE& sentence )
{

   /*
   ** RMB - Recommended Minimum Navigation Information
   **                                                             14
   **        1 2   3 4    5    6       7 8        9 10  11  12  13|
   **        | |   | |    |    |       | |        | |   |   |   | |
   ** $--RMB,A,x.x,a,c--c,c--c,llll.ll,a,yyyyy.yy,a,x.x,x.x,x.x,A*hh<CR><LF>
   **
   ** Field Number:
   **  1) Status, V = Navigation receiver warning
   **  2) Cross Track error - nautical miles
   **  3) Direction to Steer, Left or Right
   **  4) TO Waypoint ID
   **  5) FROM Waypoint ID
   **  6) Destination Waypoint Latitude
   **  7) N or S
   **  8) Destination Waypoint Longitude
   **  9) E or W
   ** 10) Range to destination in nautical miles
   ** 11) Bearing to destination in degrees true
   ** 12) Destination closing velocity in knots
   ** 13) Arrival Status, A = Arrival Circle Entered
   ** Version 2.0
   ** 14) Checksum

   Version 2.3
   ** 14) FAA Mode Indicatior, The value can be A=autonomous, D=differential, E=Estimated, N=not valid, S=Simulator, optional, may be NULL
   ** 15) Checksum
   */

   /*
   ** First we check the checksum...
   */
   int nFields = sentence.GetNumberOfDataFields();
   NMEA0183_BOOLEAN check = sentence.IsChecksumBad(nFields + 1);

   if ( check == Ntrue )
   {
       SetErrorMessage( ("Invalid Checksum") );
      return( false );
   }

   // If sentence is at least Version 2.3, check the extra FAA mode indicator field
   bool mode_valid = true;
   if (nFields >= 14) {
     QString mode_string = sentence.Field(14);
     if (!mode_string.startsWith(("*"))) {
       if ((mode_string == ("N")) || (mode_string == ("S")))     // Not valid, or simulator mode
         mode_valid = false;
     }
   }

/*
   if ( check == Unknown0183 )
   {
       SetErrorMessage( ("Missing Checksum") );
      return( false );
   }
*/
   IsDataValid = mode_valid ? sentence.Boolean( 1 ) : NFalse;
   CrossTrackError                 = sentence.Double( 2 );
   DirectionToSteer                = sentence.LeftOrRight( 3 );
   From                            = sentence.Field( 4 );
   To                              = sentence.Field( 5 );
   DestinationPosition.Parse( 6, 7, 8, 9, sentence );
   RangeToDestinationNauticalMiles = sentence.Double( 10 );
   BearingToDestinationDegreestrue = sentence.Double( 11 );
   DestinationClosingVelocityKnots = sentence.Double( 12 );
   IsArrivalCircleEntered          = sentence.Boolean( 13 );

   return( true );
}

bool RMB::Write( SENTENCE& sentence )
{

   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += IsDataValid;
   sentence += CrossTrackError;
   if(DirectionToSteer == Left)
       sentence += ("L");
   else
       sentence += ("R");

   sentence += From;
   sentence += To;
   sentence += DestinationPosition;
   sentence += RangeToDestinationNauticalMiles;
   sentence += BearingToDestinationDegreestrue;
   sentence += DestinationClosingVelocityKnots;
   sentence += IsArrivalCircleEntered;
   sentence += FAAModeIndicator;

   sentence.Finish();

//   NMEA0183_BOOLEAN check = sentence.IsChecksumBad( 14 );

   return( true );
}

const RMB& RMB::operator = ( const RMB& source )
{

   IsDataValid                     = source.IsDataValid;
   CrossTrackError                 = source.CrossTrackError;
   DirectionToSteer                = source.DirectionToSteer;
   From                            = source.From;
   To                              = source.To;
   DestinationPosition             = source.DestinationPosition;
   RangeToDestinationNauticalMiles = source.RangeToDestinationNauticalMiles;
   BearingToDestinationDegreestrue = source.BearingToDestinationDegreestrue;
   DestinationClosingVelocityKnots = source.DestinationClosingVelocityKnots;
   IsArrivalCircleEntered          = source.IsArrivalCircleEntered;
   FAAModeIndicator                = source.FAAModeIndicator;

  return( *this );
}
