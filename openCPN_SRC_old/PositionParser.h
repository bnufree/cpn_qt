/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifndef __positionparser_h__
#define __positionparser_h__

#include <wx/string.h>

class PositionParser
{
public:
    PositionParser(const QString & src);
    const QString & GetSeparator() const { return separator; }
    const QString & GetLatitudeString() const { return latitudeString; }
    const QString & GetLongitudeString() const { return longitudeString; }
    double GetLatitude() const { return latitude; }
    double GetLongitude() const { return longitude; }
    bool FindSeparator(const QString & src);
    bool IsOk() const { return parsedOk; }

private:
    QString source;
    QString separator;
    QString latitudeString;
    QString longitudeString;
    double latitude;
    double longitude;
    bool parsedOk;
};

#endif
