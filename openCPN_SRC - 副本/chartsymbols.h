/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Symbols
 * Author:   Jesper Weissglas
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

#pragma once

#include "s52plib.h"
#include "tinyxml/tinyxml.h"
#include "pugixml.hpp"
#include <QImage>
#include <QBitmap>


class Lookup {
public:
	int	RCID;
	int id;
	QString       name;
	Object_t       type;             // 'A' Area, 'L' Line, 'P' Point
	DisPrio        displayPrio;             // Display Priority
	RadPrio        radarPrio;             // 'O' or 'S', Radar Priority
	LUPname        tableName;             // FTYP:  areas, points, lines
	std::vector<char *> attributeCodeArray;  // ArrayString of LUP Attributes
	QString       instruction;            // Instruction Field (rules)
	DisCat         displayCat;             // Display Categorie: D/S/O, DisplayBase, Standard, Other
	int            comment;             // Look-Up Comment (PLib3.x put 'groupes' here,
};

typedef struct _SymbolSizeInfo {
    zchxSize size;
    zchxPoint origin;
    zchxPoint pivot;
    zchxPoint graphics;
	int minDistance;
	int maxDistance;
    _SymbolSizeInfo()
    {
        minDistance = 0;
        maxDistance = 0;
    }
} SymbolSizeInfo_t;


class OCPNPattern {
public:
	int	RCID;
	QString name;
	QString description;
	QString colorRef;
	bool hasVector;
	bool hasBitmap;
	bool preferBitmap;
	char fillType;
	char spacing;
	SymbolSizeInfo_t bitmapSize;
	SymbolSizeInfo_t vectorSize;
    QBitmap bitmap;
	QString HPGL;

    OCPNPattern()
    {
        RCID = -1;
        hasVector = false;
        hasBitmap = false;
        preferBitmap = false;
        fillType = 0;
        spacing = 0;
    }
};

class LineStyle {
public:
	int	RCID;
	QString name;
	QString description;
	QString colorRef;
	SymbolSizeInfo_t vectorSize;
    QBitmap bitmap;
	QString HPGL;
};


class ChartSymbol
{
public:
	QString name;
	int	RCID;
	bool hasVector;
	bool hasBitmap;
	bool preferBitmap;
	QString description;
	QString colorRef;
	SymbolSizeInfo_t bitmapSize;
	SymbolSizeInfo_t vectorSize;
	QString HPGL;
};


class ChartSymbols
{
public:
	ChartSymbols(void);
	~ChartSymbols(void);
	bool LoadConfigFile(s52plib* plibArg, const QString & path);


	static void InitializeGlobals( void );
	static void DeleteGlobals( void );
	static int LoadRasterFileForColorTable( int tableNo, bool flush=false );
	static wxArrayPtrVoid * GetColorTables();
	static int FindColorTable(const QString & tableName);
	static S52color* GetColor( const char *colorName, int fromTable );
    static QColor GetQColor( const QString &colorName, int fromTable );
    static QColor GetQColor( const char *colorName, int fromTable );
    static QString HashKey( const char* symbolName );
    static QImage GetImage( const char* symbolName );
    static unsigned int GetGLTextureRect( QRect &rect, const char* symbolName );
    static QSize GLTextureSize();
    static void SetColorTableIndex( int index );

private:
      void ProcessVectorTag( TiXmlElement* subNodes, SymbolSizeInfo_t &vectorSize );
      void ProcessColorTables( TiXmlElement* colortableodes );
      void ProcessLookups( TiXmlElement* lookupNodes );
      void ProcessLinestyles( TiXmlElement* linestyleNodes );
      void ProcessPatterns( TiXmlElement* patternNodes );
      void ProcessSymbols( TiXmlElement* symbolNodes );
      void BuildLineStyle( LineStyle &lineStyle );
      void BuildLookup( Lookup &lookup );
      void BuildPattern( OCPNPattern &pattern );
      void BuildSymbol( ChartSymbol &symol );
       
      void ProcessColorTables( pugi::xml_node &node );
      void ProcessLookups( pugi::xml_node &node );
      void ProcessLinestyles( pugi::xml_node &node );
      void ProcessPatterns( pugi::xml_node &node );
      void ProcessSymbols( pugi::xml_node &node );
      void ProcessVectorTag( pugi::xml_node &vectorNode, SymbolSizeInfo_t &vectorSize );
      
      pugi::xml_document m_symbolsDoc;

      s52plib* plib;
};

