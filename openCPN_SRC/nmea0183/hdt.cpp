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


HDT::HDT()
{
   Mnemonic = ("HDT");
   Empty();
}

HDT::~HDT()
{
   Mnemonic.clear();
   Empty();
}

void HDT::Empty( void )
{
   Degreestrue = 0.0;
}

bool HDT::Parse( const SENTENCE& sentence )
{

   /*
   ** HDT - Heading - true
   **
   **        1   2 3
   **        |   | |
   ** $--HDT,x.x,T*hh<CR><LF>
   **
   ** Field Number:
   **  1) Heading Degrees, true
   **  2) T = true
   **  3) Checksum
   */

   /*
   ** First we check the checksum...
   */

   if ( sentence.IsChecksumBad( 3 ) == true )
   {
      SetErrorMessage( ("Invalid Checksum") );
      return( false );
   }

   Degreestrue = sentence.Double( 1 );

   return( true );
}

bool HDT::Write( SENTENCE& sentence )
{
   /*
   ** Let the parent do its thing
   */

   RESPONSE::Write( sentence );

   sentence += Degreestrue;
   sentence += ("T");

   sentence.Finish();

   return( true );
}

const HDT& HDT::operator = ( const HDT& source )
{
   Degreestrue = source.Degreestrue;

   return( *this );
}
