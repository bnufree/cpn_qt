/***************************************************************************
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

extern QString galkerIdText;

RESPONSE::RESPONSE()
{
   Talker.clear();
   ErrorMessage.clear();
}

RESPONSE::~RESPONSE()
{
   Mnemonic.clear();
   Talker.clear();
   ErrorMessage.clear();
}

void RESPONSE::SetContainer( NMEA0183 *container )
{
   container_p = container;
}

void RESPONSE::SetErrorMessage( const QString& error_message )
{
   ErrorMessage  = Mnemonic;
   ErrorMessage += (", ");
   ErrorMessage += error_message;
}

bool RESPONSE::Write( SENTENCE& sentence )
{
   /*
   ** All NMEA0183 sentences begin with the mnemonic...
   */

    sentence  = ("$");

    if(NULL == container_p)
          sentence.Sentence.append(("--"));
    else {
        if ( galkerIdText.length() == 0) {
          sentence.Sentence.append(container_p->TalkerID);
        }
        else {
            sentence.Sentence.append( galkerIdText );
        }
    }
    
    sentence.Sentence.append(Mnemonic);

   return( true );
}

const QString& RESPONSE::PlainEnglish( void )
{
   static QString return_string;

   return_string.clear();

   return( return_string );
}
