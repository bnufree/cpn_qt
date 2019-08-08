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
#include <math.h>

#if !defined(NAN)

//static const long long lNaN = 0x7fffffffffffffff;

//#define NaN (*(double*)&lNaN)
//#else
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)

#endif



/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/


SENTENCE::SENTENCE()
{
   Sentence.clear();
}

SENTENCE::~SENTENCE()
{
   Sentence.clear();
}

NMEA0183_BOOLEAN SENTENCE::Boolean( int field_number ) const
{
//   ASSERT_VALID( this );

   QString field_data;

   field_data = Field( field_number );

   if ( field_data.startsWith(("A")) )
   {
      return( Ntrue );
   }
   else if ( field_data.startsWith(("V")) )
   {
      return( NFalse );
   }
   else
   {
      return( Unknown0183 );
   }
}

COMMUNICATIONS_MODE SENTENCE::CommunicationsMode( int field_number ) const
{
//   ASSERT_VALID( this );

   QString field_data;

   field_data = Field( field_number );

   if ( field_data == ("d") )
   {
      return( F3E_G3E_SimplexTelephone );
   }
   else if ( field_data == ("e") )
   {
      return( F3E_G3E_DuplexTelephone );
   }
   else if ( field_data == ("m") )
   {
      return( J3Eelephone );
   }
   else if ( field_data == ("o") )
   {
      return( H3Eelephone );
   }
   else if ( field_data == ("q") )
   {
      return( F1B_J2B_FEC_NBDPelexTeleprinter );
   }
   else if ( field_data == ("s") )
   {
      return( F1B_J2B_ARQ_NBDPelexTeleprinter );
   }
   else if ( field_data == ("w") )
   {
      return( F1B_J2B_ReceiveOnlyTeleprinterDSC );
   }
   else if ( field_data == ("x") )
   {
      return( A1A_MorseTapeRecorder );
   }
   else if ( field_data == ("{") )
   {
      return( A1A_MorseKeyHeadset );
   }
   else if ( field_data == ("|") )
   {
      return( F1C_F2C_F3C_FaxMachine );
   }
   else
   {
      return( CommunicationsModeUnknown );
   }
}

unsigned char SENTENCE::ComputeChecksum( void ) const
{
   unsigned char checksum_value = 0;

   int string_length = Sentence.length();
   int index = 1; // Skip over the $ at the begining of the sentence

   while( index < string_length    &&
       Sentence[ index ] != '*' &&
       Sentence[ index ] != CARRIAGE_RETURN &&
       Sentence[ index ] != LINE_FEED )
   {
       checksum_value ^= (char)(Sentence[ index ].toLatin1());
         index++;
   }

   return( checksum_value );
}

double SENTENCE::Double( int field_number ) const
{
 //  ASSERT_VALID( this );
      if(Field( field_number ).length() == 0)
            return (NAN);

      QByteArray abuf = Field( field_number).toUtf8();
      if( !abuf.data() )                            // badly formed sentence?
        return (NAN);
      
      return( ::atof( abuf.data() ));
      
}


EASTWEST SENTENCE::EastOrWest( int field_number ) const
{
//   ASSERT_VALID( this );

   QString field_data;

   field_data = Field( field_number );

   if ( field_data == ("E") )
   {
      return( East );
   }
   else if ( field_data == ("W") )
   {
      return( West );
   }
   else
   {
      return( EW_Unknown );
   }
}

const QString& SENTENCE::Field( int desired_field_number ) const
{
//   ASSERT_VALID( this );

   static QString return_string;
   return_string.clear();

   int index                = 1; // Skip over the $ at the begining of the sentence
   int current_field_number = 0;
   int string_length        = 0;

   string_length = Sentence.length();

   while( current_field_number < desired_field_number && index < string_length )
   {
      if ( Sentence[ index ] == ',' || Sentence[ index ] == '*' )
      {
         current_field_number++;
      }

      if( Sentence[ index ] == '*')
          return_string += Sentence[ index ];
      index++;
   }

   if ( current_field_number == desired_field_number )
   {
      while( index < string_length    &&
             Sentence[ index ] != ',' &&
             Sentence[ index ] != '*' &&
             Sentence[ index ] != 0x00 )
      {
         return_string += Sentence[ index ];
         index++;
      }
   }


   return( return_string );
}

int SENTENCE::GetNumberOfDataFields( void ) const
{
//   ASSERT_VALID( this );

   int index                = 1; // Skip over the $ at the begining of the sentence
   int current_field_number = 0;
   int string_length        = 0;

   string_length = Sentence.length();

   while( index < string_length )
   {
      if ( Sentence[ index ] == '*' )
      {
         return( (int) current_field_number );
      }

      if ( Sentence[ index ] == ',' )
      {
         current_field_number++;
      }

      index++;
   }

   return( (int) current_field_number );
}

void SENTENCE::Finish( void )
{
//   ASSERT_VALID( this );

   unsigned char checksum = ComputeChecksum();

   QString temp_string;

   temp_string.sprintf(("*%02X%c%c"), (int) checksum, CARRIAGE_RETURN, LINE_FEED );
   Sentence += temp_string;
}

int SENTENCE::Integer( int field_number ) const
{
//   ASSERT_VALID( this );
    QByteArray abuf = Field( field_number).toUtf8();
    if( !abuf.data() )                            // badly formed sentence?
        return 0;

    return( ::atoi( abuf.data() ));
}

NMEA0183_BOOLEAN SENTENCE::IsChecksumBad( int checksum_field_number ) const
{
//   ASSERT_VALID( this );

   /*
   ** Checksums are optional, return true if an existing checksum is known to be bad
   */

   QString checksum_in_sentence = Field( checksum_field_number );

   if ( checksum_in_sentence == ("") )
   {
      return( Unknown0183 );
   }

   QString check = checksum_in_sentence.mid( 1 );
   if ( ComputeChecksum() != HexValue( check ) )
   {
      return( Ntrue );
   }

   return( NFalse );
}

LEFTRIGHT SENTENCE::LeftOrRight( int field_number ) const
{
//   ASSERT_VALID( this );

   QString field_data;

   field_data = Field( field_number );

   if ( field_data == ("L") )
   {
      return( Left );
   }
   else if ( field_data == ("R") )
   {
      return( Right );
   }
   else
   {
      return( LR_Unknown );
   }
}

NORTHSOUTH SENTENCE::NorthOrSouth( int field_number ) const
{
//   ASSERT_VALID( this );

   QString field_data;

   field_data = Field( field_number );

   if ( field_data == ("N") )
   {
      return( North );
   }
   else if ( field_data == ("S") )
   {
      return( South );
   }
   else
   {
      return( NS_Unknown );
   }
}

REFERENCE SENTENCE::Reference( int field_number ) const
{
//   ASSERT_VALID( this );

   QString field_data;

   field_data = Field( field_number );

   if ( field_data == ("B") )
   {
      return( BottomTrackingLog );
   }
   else if ( field_data == ("M") )
   {
      return( ManuallyEntered );
   }
   else if ( field_data == ("W") )
   {
      return( WaterReferenced );
   }
   else if ( field_data == ("R") )
   {
      return( RadarTrackingOfFixedTarget );
   }
   else if ( field_data == ("P") )
   {
      return( PositioningSystemGroundReference );
   }
   else
   {
      return( ReferenceUnknown );
   }
}

TRANSDUCERYPE SENTENCE::TransducerType( int field_number ) const
{
//   ASSERT_VALID( this );

   QString field_data;

   field_data = Field( field_number );

   if ( field_data == ("A") )
   {
      return( AngularDisplacementTransducer );
   }
   else if ( field_data == ("D") )
   {
      return( LinearDisplacementTransducer );
   }
   else if ( field_data == ("C") )
   {
      return( TemperatureTransducer );
   }
   else if ( field_data == ("F") )
   {
      return( FrequencyTransducer );
   }
   else if ( field_data == ("N") )
   {
      return( ForceTransducer );
   }
   else if ( field_data == ("P") )
   {
      return( PressureTransducer );
   }
   else if ( field_data == ("R") )
   {
      return( FlowRateTransducer );
   }
   else if ( field_data == ("T") )
   {
      return( TachometerTransducer );
   }
   else if ( field_data == ("H") )
   {
      return( HumidityTransducer );
   }
   else if ( field_data == ("V") )
   {
      return( VolumeTransducer );
   }
   else
   {
      return( TransducerUnknown );
   }
}

/*
** Operators
*/

SENTENCE::operator QString() const
{
//   ASSERT_VALID( this );

   return( Sentence );
}

const SENTENCE& SENTENCE::operator = ( const SENTENCE& source )
{
//   ASSERT_VALID( this );

   Sentence = source.Sentence;

   return( *this );
}

const SENTENCE& SENTENCE::operator = ( const QString& source )
{
//   ASSERT_VALID( this );

   Sentence = source;

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( const QString& source )
{
//   ASSERT_VALID( this );

    Sentence += (",");
   Sentence += source;

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( double value )
{
//   ASSERT_VALID( this );

   QString temp_string;

   temp_string.sprintf(("%.3f"), value );

   Sentence += (",");
   Sentence += temp_string;

   return( *this );
}

SENTENCE& SENTENCE::Add ( double value, int precision )
{
//   ASSERT_VALID( this );

    QString temp_string;
    QString s_Precision;

    s_Precision.sprintf(("%c.%if"), '%', precision );
    temp_string.sprintf( s_Precision.toStdString().data(), value );

    Sentence += (",");
    Sentence += temp_string;

    return( *this );
}
const SENTENCE& SENTENCE::operator += ( COMMUNICATIONS_MODE mode )
{
//   ASSERT_VALID( this );

    Sentence += (",");

   switch( mode )
   {
      case F3E_G3E_SimplexTelephone:

          Sentence += ("d");
               break;

      case F3E_G3E_DuplexTelephone:

          Sentence += ("e");
               break;

      case J3Eelephone:

          Sentence += ("m");
               break;

      case H3Eelephone:

          Sentence += ("o");
               break;

      case F1B_J2B_FEC_NBDPelexTeleprinter:

          Sentence += ("q");
               break;

      case F1B_J2B_ARQ_NBDPelexTeleprinter:

          Sentence += ("s");
               break;

      case F1B_J2B_ReceiveOnlyTeleprinterDSC:

          Sentence += ("w");
               break;

      case A1A_MorseTapeRecorder:

          Sentence += ("x");
               break;

      case A1A_MorseKeyHeadset:

          Sentence += ("{");
               break;

       case F1C_F2C_F3C_FaxMachine:

           Sentence += ("|");
           break;

       case CommunicationsModeUnknown:

           break;
   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( TRANSDUCERYPE transducer )
{
//   ASSERT_VALID( this );

    Sentence += (",");

   switch( transducer )
   {
      case TemperatureTransducer:

          Sentence += ("C");
               break;

      case AngularDisplacementTransducer:

          Sentence += ("A");
               break;

      case LinearDisplacementTransducer:

          Sentence += ("D");
               break;

      case FrequencyTransducer:

          Sentence += ("F");
               break;

      case ForceTransducer:

          Sentence += ("N");
               break;

      case PressureTransducer:

          Sentence += ("P");
               break;

      case FlowRateTransducer:

          Sentence += ("R");
               break;

      case TachometerTransducer:

          Sentence += ("T");
               break;

      case HumidityTransducer:

          Sentence += ("H");
               break;

      case VolumeTransducer:

          Sentence += ("V");
               break;

      case TransducerUnknown:

          Sentence += ("?");
               break;

   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( NORTHSOUTH northing )
{
//   ASSERT_VALID( this );

    Sentence += (",");

   if ( northing == North )
   {
       Sentence += ("N");
   }
   else if ( northing == South )
   {
       Sentence += ("S");
   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( int value )
{
//   ASSERT_VALID( this );

   QString temp_string;

   temp_string.sprintf(("%d"), value );

   Sentence += (",");
   Sentence += temp_string;

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( EASTWEST easting )
{
//   ASSERT_VALID( this );

    Sentence += (",");

   if ( easting == East )
   {
       Sentence += ("E");
   }
   else if ( easting == West )
   {
       Sentence += ("W");
   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( NMEA0183_BOOLEAN boolean )
{
//   ASSERT_VALID( this );

    Sentence += (",");

   if ( boolean == Ntrue )
   {
       Sentence += ("A");
   }
   else if ( boolean == NFalse )
   {
       Sentence += ("V");
   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( LATLONG& source )
{
//   ASSERT_VALID( this );

   source.Write( *this );

   return( *this );
}
