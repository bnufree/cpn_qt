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
#include "config.h"


#include <stdlib.h>

#include "chartsymbols.h"
#include <QPixmap>
//#ifdef ocpnUSE_GL
//#include <wx/glcanvas.h>
//#endif

#ifdef ocpnUSE_GL
extern GLenum       g_texture_rectangle_format;
#endif

//--------------------------------------------------------------------------------------
// The below data is global since there will ever only be one ChartSymbols instance,
// and some methods+data of class S52plib are needed inside ChartSymbol, and s2plib
// needs some methods from ChartSymbol. So s52plib only calls static methods in
// order to resolve circular include file dependencies.

wxArrayPtrVoid* colorTables = 0;
unsigned int rasterSymbolsTexture;
QSize rasterSymbolsTextureSize;
QPixmap *rasterSymbols = 0;
int rasterSymbolsLoadedColorMapNumber;
QString configFileDirectory;
int ColorTableIndex;

typedef QMap<QString, QRect> symbolGraphicsHashMap;

symbolGraphicsHashMap* symbolGraphicLocations = 0;

//--------------------------------------------------------------------------------------


ChartSymbols::ChartSymbols( void )
{
}

ChartSymbols::~ChartSymbols( void )
{
}

void ChartSymbols::InitializeGlobals( void )
{
    if( !colorTables ) colorTables = new wxArrayPtrVoid;
    if( !symbolGraphicLocations ) symbolGraphicLocations = new symbolGraphicsHashMap;
    rasterSymbolsLoadedColorMapNumber = -1;
    ColorTableIndex = 0;
    rasterSymbols = new QPixmap;
}

void ChartSymbols::DeleteGlobals( void )
{

    ( *symbolGraphicLocations ).clear();
    delete symbolGraphicLocations;
    symbolGraphicLocations = NULL;

    for( unsigned int i = 0; i < colorTables->count(); i++ ) {
        colTable *ct = (colTable *) colorTables->at( i );
//        delete ct->tableName;
        ct->S52Colors.clear();
        ct->QColors.clear();
        delete ct;
    }

    colorTables->clear();
    delete colorTables;
    colorTables = NULL;
}

void ChartSymbols::ProcessColorTables( pugi::xml_node &node )
{
    for( pugi::xml_node child = node.first_child(); child != 0; child = child.next_sibling() ) {
        const char *pcn = child.name();
        
        if( !strcmp( pcn, "color-table" ) ) {
            colTable *colortable = new colTable;
            colortable->tableName = QString::fromUtf8(child.first_attribute().value());
            
            pugi::xml_node colorNode =child.first_child();
            while(colorNode){
                if(!strcmp(colorNode.name(), "graphics-file")){
                    colortable->rasterFileName = QString::fromUtf8(colorNode.first_attribute().value());
                }

                if(!strcmp(colorNode.name(), "color")){
                    QString key;
                    S52color color;
                    
                    for ( pugi::xml_attribute attr = colorNode.first_attribute(); attr; attr = attr.next_attribute() ) {
                        const char *pca = attr.name();
                        if(!strcmp(pca, "name")){
                            strncpy(color.colName, attr.value(), 5);
                            color.colName[5] = 0;
                            key = QString::fromUtf8(attr.value());
                            
                        }                        
                        else if(!strcmp(pca, "r")){
                            color.R = attr.as_int();
                        }
                        else if(!strcmp(pca, "g")){
                            color.G = attr.as_int();
                        }
                        else if(!strcmp(pca, "b")){
                            color.B = attr.as_int();
                        }
                        
                    }
                    
                    colortable->S52Colors[key] = color;
                    QColor qcolor( color.R, color.G, color.B );
                    colortable->QColors[key] = qcolor;
                    
                }
            
	            colorNode = colorNode.next_sibling();
            }
            
            colorTables->append( (void *) colortable );
        }
    }
}









void TGET_INT_PROPERTY_VALUE( TiXmlElement* node, char* name, int &r)
{
    QString propVal = QString::fromUtf8(node->Attribute(name));
    r =  propVal.toInt();
}

void ChartSymbols::ProcessColorTables( TiXmlElement* colortableNodes )
{

    for( TiXmlNode *childNode = colortableNodes->FirstChild(); childNode;
            childNode = childNode->NextSibling() ) {
        TiXmlElement *child = childNode->ToElement();
        colTable *colortable = new colTable;

        const char *pName = child->Attribute( "name" );
        colortable->tableName = QString::fromUtf8(pName);

        TiXmlElement* colorNode = child->FirstChild()->ToElement();

        while( colorNode ) {
            S52color color;
            QString propVal;
            long numVal;

            if( QString::fromUtf8(colorNode->Value()) == "graphics-file" ) {
                colortable->rasterFileName = QString::fromUtf8(colorNode->Attribute( "name" ));
                goto next;
            } else {
                int r, g, b;
                TGET_INT_PROPERTY_VALUE( colorNode, "r", r);
                TGET_INT_PROPERTY_VALUE( colorNode, "g", g);
                TGET_INT_PROPERTY_VALUE( colorNode, "b", b);
                color.R = r;
                color.G = g;
                color.B = b;

                QString key = QString::fromUtf8(colorNode->Attribute( "name" ));
                strncpy( color.colName, key.toUtf8().data(), 5 );
                color.colName[5] = 0;

                colortable->S52Colors[key] = color;

                QColor qcolor( color.R, color.G, color.B );
                colortable->QColors[key] = qcolor;
            }

            next: colorNode = colorNode->NextSiblingElement();
        }

        colorTables->append((void *) colortable );

    }
}

void ChartSymbols::ProcessLookups( pugi::xml_node &node )
{
    Lookup lookup;
    
    for( pugi::xml_node child = node.first_child(); child != 0; child = child.next_sibling() ) {
        const char *pcn = child.name();
        
        
        if( !strcmp( pcn, "lookup" ) ) {
            for ( pugi::xml_attribute attr = child.first_attribute(); attr; attr = attr.next_attribute() ) {
                const char *pca = attr.name();
                if(!strcmp(pca, "name")){
                    lookup.name = QString::fromUtf8(attr.value());
                }
                else if(!strcmp(pca, "RCID")){
                    lookup.RCID = attr.as_int();
                }
                else if(!strcmp(pca, "id")){
                    lookup.id = attr.as_int();
                }
            }
        }
        
        pugi::xml_node lookupNode = child.first_child();
        while(lookupNode){
            const char *nodeText = lookupNode.first_child().value();
 
            if(!strcmp( lookupNode.name(), "type")){
                if(!strcmp(nodeText, "Area")) lookup.type = AREAS_T;
                else if(!strcmp(nodeText, "Line")) lookup.type = LINES_T;
                else lookup.type = POINT_T;
            }
            else if( !strcmp( lookupNode.name(), "disp-prio") ) {
                if( !strcmp(nodeText,"Group 1") ) lookup.displayPrio = PRIO_GROUP1;
                else if( !strcmp(nodeText,"Area 1") ) lookup.displayPrio = PRIO_AREA_1;
                else if( !strcmp(nodeText,"Area 2") ) lookup.displayPrio = PRIO_AREA_2;
                else if( !strcmp(nodeText,"Point Symbol") ) lookup.displayPrio = PRIO_SYMB_POINT;
                else if( !strcmp(nodeText,"Line Symbol") ) lookup.displayPrio = PRIO_SYMB_LINE;
                else if( !strcmp(nodeText,"Area Symbol") ) lookup.displayPrio = PRIO_SYMB_AREA;
                else if( !strcmp(nodeText,"Routing") ) lookup.displayPrio = PRIO_ROUTEING;
                else if( !strcmp(nodeText,"Hazards") ) lookup.displayPrio = PRIO_HAZARDS;
                else if( !strcmp(nodeText,"Mariners") ) lookup.displayPrio = PRIO_MARINERS;
                else lookup.displayPrio = PRIO_NODATA;
                
            }

            else if(!strcmp( lookupNode.name(), "radar-prio")){
                if( !strcmp(nodeText,"On Top") ) lookup.radarPrio = RAD_OVER;
                else lookup.radarPrio = RAD_SUPP;
            }                
            
            else if( !strcmp( lookupNode.name(), "table-name") ) {
                if( !strcmp(nodeText, "Simplified") ) lookup.tableName = SIMPLIFIED;
                else if( !strcmp(nodeText, "Lines") ) lookup.tableName = LINES;
                else if( !strcmp(nodeText,"Plain") ) lookup.tableName = PLAIN_BOUNDARIES;
                else if( !strcmp(nodeText,"Symbolized") ) lookup.tableName = SYMBOLIZED_BOUNDARIES;
                else  lookup.tableName = PAPER_CHART;
            }
            
            else if( !strcmp( lookupNode.name(), "display-cat") ) {
                if( !strcmp( nodeText,"Displaybase") ) lookup.displayCat = DISPLAYBASE;
                else  if( !strcmp( nodeText,"Standard") ) lookup.displayCat = STANDARD;
                else  if( !strcmp( nodeText,"Other") ) lookup.displayCat = OTHER;
                else  if( !strcmp( nodeText,"Mariners") ) lookup.displayCat = MARINERS_STANDARD;
                else  lookup.displayCat = OTHER;
            }
            
            else if( !strcmp( lookupNode.name(), "comment") ) {
                lookup.comment = lookupNode.first_child().text().as_int();
            }
            
            else if( !strcmp( lookupNode.name(), "instruction") ) {
                QString inst = QString::fromUtf8(nodeText);
                lookup.instruction = inst;
                lookup.instruction.append('\037' );
                
            }
            
            else if( !strcmp( lookupNode.name(), "attrib-code") ) {
                int nc = strlen(nodeText);
                if(nc >= 6){                            //  ignore spurious short fields
                    char *attVal = (char *)calloc(nc+2, sizeof(char));
                    memcpy(attVal, nodeText, nc);
                
                    if( attVal[6] == '\0')
                        attVal[6] = ' ';
                    lookup.attributeCodeArray.push_back(attVal);
                }
                
            }
        
            lookupNode = lookupNode.next_sibling();
        }
        
        BuildLookup( lookup );
        lookup.attributeCodeArray.clear();
    }
            
}

void ChartSymbols::ProcessVectorTag( pugi::xml_node &vectorNode, SymbolSizeInfo_t &vectorSize )
{
    vectorSize.size.width = vectorNode.attribute("width").as_int();
    vectorSize.size.height = vectorNode.attribute("height").as_int();
    
    
    for( pugi::xml_node child = vectorNode.first_child(); child != 0; child = child.next_sibling() ) {
        const char *nodeType = child.name();
        
        if( !strcmp(nodeType,"distance") ){
            vectorSize.minDistance = child.attribute("min").as_int();
            vectorSize.maxDistance = child.attribute("max").as_int();
        }

        else if( !strcmp(nodeType,"origin") ){
            vectorSize.origin.x = child.attribute("x").as_int();
            vectorSize.origin.y = child.attribute("y").as_int();
        }
    
        else if( !strcmp(nodeType,"pivot") ){
            vectorSize.pivot.x = child.attribute("x").as_int();
            vectorSize.pivot.y = child.attribute("y").as_int();
        }
    }
}

void ChartSymbols::ProcessLinestyles( pugi::xml_node &node )
{
    LineStyle lineStyle;
    
    for( pugi::xml_node child = node.first_child(); child != 0; child = child.next_sibling() ) {
        lineStyle.RCID = child.attribute("RCID").as_int();
        
        pugi::xml_node lineNode = child.first_child();
        while(lineNode){
            const char *nodeText = lineNode.first_child().value();
            const char *nodeType = lineNode.name();
            
            if( !strcmp(nodeType,"description") ) lineStyle.description = nodeText;
            else if( !strcmp(nodeType,"name") ) lineStyle.name = nodeText;
            else if( !strcmp(nodeType,"color-ref") ) lineStyle.colorRef = nodeText;
            else if( !strcmp(nodeType,"HPGL") ) lineStyle.HPGL = nodeText;
            else if( !strcmp(nodeType,"vector") ) ProcessVectorTag( lineNode, lineStyle.vectorSize );
        
            lineNode = lineNode.next_sibling();
        }
        BuildLineStyle( lineStyle );
    }            
}


void ChartSymbols::ProcessPatterns( pugi::xml_node &node )
{
    OCPNPattern pattern;
 
    for( pugi::xml_node child = node.first_child(); child != 0; child = child.next_sibling() ) {
        pattern.RCID = child.attribute("RCID").as_int();
    
        pattern.hasVector = false;
        pattern.hasBitmap = false;
        pattern.preferBitmap = true;
        
        pugi::xml_node pattNode = child.first_child();
        while(pattNode){
            const char *nodeText = pattNode.first_child().value();
            const char *nodeType = pattNode.name();
            
            if( !strcmp(nodeType,"description") ) pattern.description = nodeText;
            else if( !strcmp(nodeType,"name") ) pattern.name = nodeText;
            else if( !strcmp(nodeType,"filltype") ) pattern.fillType = nodeText[0];
            else if( !strcmp(nodeType,"spacing") ) pattern.spacing = nodeText[0];
            else if( !strcmp(nodeType,"definition") ) pattern.hasVector = !strcmp(nodeText, "V");
            else if( !strcmp(nodeType,"color-ref") ) pattern.colorRef = nodeText;
            else if( !strcmp(nodeType,"HPGL") ) { pattern.HPGL = nodeText; pattern.hasVector = true; }
            
            else if( !strcmp(nodeType,"prefer-bitmap") ){
                if(!strcmp(nodeText, "no")) pattern.preferBitmap = false;
                else if(!strcmp(nodeText, "false")) pattern.preferBitmap = false;
            }
                
            else if( !strcmp(nodeType,"bitmap") ){
                pattern.bitmapSize.size.width = pattNode.attribute("width").as_int();
                pattern.bitmapSize.size.height = pattNode.attribute("height").as_int();
                
                for( pugi::xml_node child = pattNode.first_child(); child != 0; child = child.next_sibling() ) {
                    const char *nodeType = child.name();
                    
                    if( !strcmp(nodeType,"distance") ){
                        pattern.bitmapSize.minDistance = child.attribute("min").as_int();
                        pattern.bitmapSize.maxDistance = child.attribute("max").as_int();
                    }
                    else if( !strcmp(nodeType,"origin") ){
                        pattern.bitmapSize.origin.x = child.attribute("x").as_int();
                        pattern.bitmapSize.origin.y = child.attribute("y").as_int();
                    }
                    else if( !strcmp(nodeType,"pivot") ){
                        pattern.bitmapSize.pivot.x = child.attribute("x").as_int();
                        pattern.bitmapSize.pivot.y = child.attribute("y").as_int();
                    }
                    else if( !strcmp(nodeType,"graphics-location") ){
                        pattern.bitmapSize.graphics.x = child.attribute("x").as_int();
                        pattern.bitmapSize.graphics.y = child.attribute("y").as_int();
                    }
                }
            }
            
            else if( !strcmp(nodeType,"vector") )
                ProcessVectorTag( pattNode, pattern.vectorSize );
                
            
            pattNode = pattNode.next_sibling();
        }
        
        
        BuildPattern( pattern );
    }
}



void ChartSymbols::ProcessSymbols( pugi::xml_node &node )
{
    ChartSymbol symbol;
    
    for( pugi::xml_node child = node.first_child(); child != 0; child = child.next_sibling() ) {
        symbol.RCID = child.attribute("RCID").as_int();
 
        symbol.hasVector = false;
        symbol.hasBitmap = false;
        symbol.preferBitmap = true;
        
        pugi::xml_node symbolNode = child.first_child();
        while(symbolNode){
            const char *nodeText = symbolNode.first_child().value();
            const char *nodeType = symbolNode.name();
            
            if( !strcmp(nodeType,"description") ) symbol.description = nodeText;
            else if( !strcmp(nodeType,"name") ) symbol.name = nodeText;
            else if( !strcmp(nodeType,"definition") ) symbol.hasVector = !strcmp(nodeText, "V");
            else if( !strcmp(nodeType,"color-ref") ) symbol.colorRef = nodeText;
            
            else if( !strcmp(nodeType,"prefer-bitmap") ){
                if(!strcmp(nodeText, "no")) symbol.preferBitmap = false;
                else if(!strcmp(nodeText, "false")) symbol.preferBitmap = false;
            }
            
            else if( !strcmp(nodeType,"bitmap") ){
                symbol.bitmapSize.size.width = symbolNode.attribute("width").as_int();
                symbol.bitmapSize.size.height = symbolNode.attribute("height").as_int();
                symbol.hasBitmap = true;
                
                for( pugi::xml_node child = symbolNode.first_child(); child != 0; child = child.next_sibling() ) {
                    const char *nodeType = child.name();
                    
                    if( !strcmp(nodeType,"distance") ){
                        symbol.bitmapSize.minDistance = child.attribute("min").as_int();
                        symbol.bitmapSize.maxDistance = child.attribute("max").as_int();
                    }
                    else if( !strcmp(nodeType,"origin") ){
                        symbol.bitmapSize.origin.x = child.attribute("x").as_int();
                        symbol.bitmapSize.origin.y = child.attribute("y").as_int();
                    }
                    else if( !strcmp(nodeType,"pivot") ){
                        symbol.bitmapSize.pivot.x = child.attribute("x").as_int();
                        symbol.bitmapSize.pivot.y = child.attribute("y").as_int();
                    }
                    else if( !strcmp(nodeType,"graphics-location") ){
                        symbol.bitmapSize.graphics.x = child.attribute("x").as_int();
                        symbol.bitmapSize.graphics.y = child.attribute("y").as_int();
                    }
                }
            }
            
            else if( !strcmp(nodeType,"vector") ){
                symbol.vectorSize.size.width = symbolNode.attribute("width").as_int();
                symbol.vectorSize.size.height = symbolNode.attribute("height").as_int();
                symbol.hasVector = true;
                
                for( pugi::xml_node child = symbolNode.first_child(); child != 0; child = child.next_sibling() ) {
                    const char *nodeType = child.name();
                    
                    if( !strcmp(nodeType,"distance") ){
                        symbol.vectorSize.minDistance = child.attribute("min").as_int();
                        symbol.vectorSize.maxDistance = child.attribute("max").as_int();
                    }
                    else if( !strcmp(nodeType,"origin") ){
                        symbol.vectorSize.origin.x = child.attribute("x").as_int();
                        symbol.vectorSize.origin.y = child.attribute("y").as_int();
                    }
                    else if( !strcmp(nodeType,"pivot") ){
                        symbol.vectorSize.pivot.x = child.attribute("x").as_int();
                        symbol.vectorSize.pivot.y = child.attribute("y").as_int();
                    }
                    else if( !strcmp(nodeType,"HPGL") ){
                        symbol.HPGL = QString::fromUtf8(child.first_child().value());
                    }
                }
            }
            
            
            symbolNode = symbolNode.next_sibling();
        }
        
        BuildSymbol( symbol );
    }
    
}

void ChartSymbols::ProcessLookups( TiXmlElement* lookupNodes )
{
    Lookup lookup;
    QString propVal;
    long numVal;

    for( TiXmlNode *childNode = lookupNodes->FirstChild(); childNode;
            childNode = childNode->NextSibling() ) {
        TiXmlElement *child = childNode->ToElement();

        TGET_INT_PROPERTY_VALUE( child, "id", lookup.id);
        TGET_INT_PROPERTY_VALUE( child, "RCID", lookup.RCID);
        lookup.name = QString::fromUtf8(child->Attribute( "name" ) );

        TiXmlElement* subNode = child->FirstChild()->ToElement();

        while( subNode ) {
            QString nodeType = QString::fromUtf8(subNode->Value());
            QString nodeText = QString::fromUtf8(subNode->GetText());

            if( nodeType == ("type") ) {

                if( nodeText == ("Area") ) lookup.type = AREAS_T;
                else
                    if( nodeText == ("Line") ) lookup.type = LINES_T;
                    else
                        lookup.type = POINT_T;

                goto nextNode;
            }

            if( nodeType == ("disp-prio") ) {
                lookup.displayPrio = PRIO_NODATA;
                if( nodeText == ("Group 1") ) lookup.displayPrio = PRIO_GROUP1;
                else
                if( nodeText == ("Area 1") ) lookup.displayPrio = PRIO_AREA_1;
                else
                if( nodeText == ("Area 2") ) lookup.displayPrio = PRIO_AREA_2;
                else
                if( nodeText == ("Point Symbol") ) lookup.displayPrio = PRIO_SYMB_POINT;
                else
                if( nodeText == ("Line Symbol") ) lookup.displayPrio = PRIO_SYMB_LINE;
                else
                if( nodeText == ("Area Symbol") ) lookup.displayPrio = PRIO_SYMB_AREA;
                else
                if( nodeText == ("Routing") ) lookup.displayPrio = PRIO_ROUTEING;
                else
                if( nodeText == ("Hazards") ) lookup.displayPrio = PRIO_HAZARDS;
                else
                if( nodeText == ("Mariners") ) lookup.displayPrio = PRIO_MARINERS;
                goto nextNode;
            }
            if( nodeType == ("radar-prio") ) {
                if( nodeText == ("On Top") ) lookup.radarPrio = RAD_OVER;
                else
                    lookup.radarPrio = RAD_SUPP;
                goto nextNode;
            }
            if( nodeType == ("table-name") ) {
                if( nodeText == ("Simplified") ) lookup.tableName = SIMPLIFIED;
                else
                if( nodeText == ("Lines") ) lookup.tableName = LINES;
                else
                if( nodeText == ("Plain") ) lookup.tableName = PLAIN_BOUNDARIES;
                else
                if( nodeText == ("Symbolized") ) lookup.tableName = SYMBOLIZED_BOUNDARIES;
                else
                lookup.tableName = PAPER_CHART;
                goto nextNode;
            }
            if( nodeType == ("display-cat") ) {
                if( nodeText == ("Displaybase") ) lookup.displayCat = DISPLAYBASE;
                else
                if( nodeText == ("Standard") ) lookup.displayCat = STANDARD;
                else
                if( nodeText == ("Other") ) lookup.displayCat = OTHER;
                else
                if( nodeText == ("Mariners") ) lookup.displayCat = MARINERS_STANDARD;
                else
                lookup.displayCat = OTHER;
                goto nextNode;
            }
            if( nodeType == ("comment") ) {
                QString comment = QString::fromUtf8(subNode->GetText());
                long value = comment.toLong();
                lookup.comment = value;
                goto nextNode;
            }

            if( nodeType == ("instruction") ) {
                lookup.instruction = nodeText;
                lookup.instruction.append('\037' );
                goto nextNode;
            }
            if( nodeType == ("attrib-code") ) {
                char *attVal = (char *)calloc(8, sizeof(char));
                strncpy(attVal, nodeText.toUtf8().data(), 7);
                if( attVal[6] == '\0')
                    attVal[6] = ' ';
                lookup.attributeCodeArray.push_back(attVal);

                goto nextNode;
            }

            nextNode: subNode = subNode->NextSiblingElement();
        }

        BuildLookup( lookup );
    }
}

void ChartSymbols::BuildLookup( Lookup &lookup )
{

    LUPrec *LUP = (LUPrec*) calloc( 1, sizeof(LUPrec) );
    plib->pAlloc->append(LUP );

    LUP->RCID = lookup.RCID;
    LUP->nSequence = lookup.id;
    LUP->DISC = lookup.displayCat;
    LUP->FTYP = lookup.type;
    LUP->DPRI = lookup.displayPrio;
    LUP->RPRI = lookup.radarPrio;
    LUP->TNAM = lookup.tableName;
    LUP->OBCL[6] = 0;
    memcpy( LUP->OBCL, lookup.name.toUtf8().data(), 7 );

    LUP->ATTArray = lookup.attributeCodeArray;

    LUP->INST = new QString( lookup.instruction );
    LUP->LUCM = lookup.comment;

    // Add LUP to array
    // Search the LUPArray to see if there is already a LUP with this RCID
    // If found, replace it with the new LUP
    // This provides a facility for updating the LUP tables after loading a basic set
    unsigned int index = 0;
    wxArrayOfLUPrec *pLUPARRAYtyped = plib->SelectLUPARRAY( LUP->TNAM );

    while( index < pLUPARRAYtyped->count() ) {
        LUPrec *pLUPCandidate = pLUPARRAYtyped->at( index );
        if( LUP->RCID == pLUPCandidate->RCID ) {
            pLUPARRAYtyped->removeAt(index);
            plib->DestroyLUP(pLUPCandidate); // empties the LUP
            break;
        }
        index++;
    }

    pLUPARRAYtyped->append( LUP );

}

void ChartSymbols::ProcessVectorTag( TiXmlElement* vectorNode, SymbolSizeInfo_t &vectorSize )
{
    QString propVal;
    long numVal;
    TGET_INT_PROPERTY_VALUE( vectorNode, "width", vectorSize.size.width);
    TGET_INT_PROPERTY_VALUE( vectorNode, "height", vectorSize.size.height);

    TiXmlElement* vectorNodes = vectorNode->FirstChild()->ToElement();

    while( vectorNodes ) {
        QString nodeType = QString::fromUtf8(vectorNodes->Value());

        if( nodeType == ("distance") ) {
            TGET_INT_PROPERTY_VALUE( vectorNodes, "min", vectorSize.minDistance);
            TGET_INT_PROPERTY_VALUE( vectorNodes, "max", vectorSize.maxDistance);
            goto nextVector;
        }
        if( nodeType == ("origin") ) {
            TGET_INT_PROPERTY_VALUE( vectorNodes, "x", vectorSize.origin.x);
            TGET_INT_PROPERTY_VALUE( vectorNodes, "y", vectorSize.origin.y);
            goto nextVector;
        }
        if( nodeType == ("pivot") ) {
            TGET_INT_PROPERTY_VALUE( vectorNodes, "x", vectorSize.pivot.x);
            TGET_INT_PROPERTY_VALUE( vectorNodes, "y", vectorSize.pivot.y);
            goto nextVector;
        }
        nextVector: vectorNodes = vectorNodes->NextSiblingElement();
    }
}


void ChartSymbols::ProcessLinestyles( TiXmlElement* linestyleNodes )
{

    LineStyle lineStyle;
    QString propVal;
    long numVal;

    for( TiXmlNode *childNode = linestyleNodes->FirstChild(); childNode;
            childNode = childNode->NextSibling() ) {
        TiXmlElement *child = childNode->ToElement();

        TGET_INT_PROPERTY_VALUE( child, "RCID", lineStyle.RCID);
        TiXmlElement* subNode = child->FirstChild()->ToElement();

        while( subNode ) {
            QString nodeType = QString::fromUtf8(subNode->Value());
            QString nodeText = QString::fromUtf8(subNode->GetText());

            if( nodeType == ("description") ) {
                lineStyle.description = nodeText;
                goto nextNode;
            }
            if( nodeType == ("name") ) {
                lineStyle.name = nodeText;
                goto nextNode;
            }
            if( nodeType == ("color-ref") ) {
                lineStyle.colorRef = nodeText;
                goto nextNode;
            }
            if( nodeType == ("HPGL") ) {
                lineStyle.HPGL = nodeText;
                goto nextNode;
            }
            if( nodeType == ("vector") ) {
                ProcessVectorTag( subNode, lineStyle.vectorSize );
            }
            nextNode: subNode = subNode->NextSiblingElement();
        }

        BuildLineStyle( lineStyle );
    }
}

void ChartSymbols::BuildLineStyle( LineStyle &lineStyle )
{
    Rule *lnstmp = NULL;
    Rule *lnst = (Rule*) calloc( 1, sizeof(Rule) );
    plib->pAlloc->append( lnst );

    lnst->RCID = lineStyle.RCID;
    memcpy( lnst->name.PANM, lineStyle.name.toUtf8().data(), 8 );
    lnst->bitmap.PBTM = NULL;

    lnst->vector.LVCT = (char *) malloc( lineStyle.HPGL.length() + 1 );
    strcpy( lnst->vector.LVCT, lineStyle.HPGL.toUtf8().data() );

    lnst->colRef.LCRF = (char *) malloc( lineStyle.colorRef.length() + 1 );
    strcpy( lnst->colRef.LCRF, lineStyle.colorRef.toUtf8().data() );

    lnst->pos.line.minDist.PAMI = lineStyle.vectorSize.minDistance;
    lnst->pos.line.maxDist.PAMA = lineStyle.vectorSize.maxDistance;

    lnst->pos.line.pivot_x.PACL = lineStyle.vectorSize.pivot.x;
    lnst->pos.line.pivot_y.PARW = lineStyle.vectorSize.pivot.y;

    lnst->pos.line.bnbox_w.PAHL = lineStyle.vectorSize.size.width;
    lnst->pos.line.bnbox_h.PAVL = lineStyle.vectorSize.size.height;

    lnst->pos.line.bnbox_x.SBXC = lineStyle.vectorSize.origin.x;
    lnst->pos.line.bnbox_y.SBXR = lineStyle.vectorSize.origin.y;

    lnstmp = ( *plib->_line_sym )[lineStyle.name];

    if( NULL == lnstmp ) ( *plib->_line_sym )[lineStyle.name] = lnst;
    else
        if( lnst->name.LINM != lnstmp->name.LINM ) ( *plib->_line_sym )[lineStyle.name] = lnst;
}

void ChartSymbols::ProcessPatterns( TiXmlElement* patternNodes )
{

    OCPNPattern pattern;
    QString propVal;
    long numVal;

    for( TiXmlNode *childNode = patternNodes->FirstChild(); childNode;
            childNode = childNode->NextSibling() ) {
        TiXmlElement *child = childNode->ToElement();

        TGET_INT_PROPERTY_VALUE( child, "RCID", pattern.RCID);
        pattern.hasVector = false;
        pattern.hasBitmap = false;
        pattern.preferBitmap = true;

        TiXmlElement* subNodes = child->FirstChild()->ToElement();

        while( subNodes ) {
            QString nodeType = QString::fromUtf8(subNodes->Value());
            QString nodeText = QString::fromUtf8(subNodes->GetText());

            if( nodeType == ("description") ) {
                pattern.description = nodeText;
                goto nextNode;
            }
            if( nodeType == ("name") ) {
                pattern.name = nodeText;
                goto nextNode;
            }
            if( nodeType == ("filltype") ) {
                pattern.fillType = ( subNodes->GetText() )[0];
                goto nextNode;
            }
            if( nodeType == ("spacing") ) {
                pattern.spacing = ( subNodes->GetText() )[0];
                goto nextNode;
            }
            if( nodeType == ("color-ref") ) {
                pattern.colorRef = nodeText;
                goto nextNode;
            }
            if( nodeType == ("definition") ) {
                if( !strcmp( subNodes->GetText(), "V" ) ) pattern.hasVector = true;
                goto nextNode;
            }
            if( nodeType == ("prefer-bitmap") ) {
                if( nodeText.toLower() == ("no") ) pattern.preferBitmap = false;
                if( nodeText.toLower() == ("false") ) pattern.preferBitmap = false;
                goto nextNode;
            }
            if( nodeType == ("bitmap") ) {
                TGET_INT_PROPERTY_VALUE( subNodes, "width", pattern.bitmapSize.size.width);
                TGET_INT_PROPERTY_VALUE( subNodes, "height", pattern.bitmapSize.size.height);
                pattern.hasBitmap = true;

                TiXmlElement* bitmapNodes = subNodes->FirstChild()->ToElement();
                while( bitmapNodes ) {
                    QString bitmapnodeType = QString::fromUtf8(bitmapNodes->Value() );

                    if( bitmapnodeType == ("distance") ) {
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "min", pattern.bitmapSize.minDistance);
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "max", pattern.bitmapSize.maxDistance);
                        goto nextBitmap;
                    }
                    if( bitmapnodeType == ("origin") ) {
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "x", pattern.bitmapSize.origin.x);
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "y", pattern.bitmapSize.origin.y);
                        goto nextBitmap;
                    }
                    if( bitmapnodeType == ("pivot") ) {
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "x", pattern.bitmapSize.pivot.x );
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "y", pattern.bitmapSize.pivot.y);
                        goto nextBitmap;
                    }
                    if( bitmapnodeType == ("graphics-location") ) {
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "x", pattern.bitmapSize.graphics.x);
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "y", pattern.bitmapSize.graphics.y);
                    }
                    nextBitmap: bitmapNodes = bitmapNodes->NextSiblingElement();
                }
                goto nextNode;
            }
            if( nodeType == ("HPGL") ) {
                pattern.hasVector = true;
                pattern.HPGL = nodeText;
                goto nextNode;
            }
            if( nodeType == ("vector") ) {
                ProcessVectorTag( subNodes, pattern.vectorSize );
            }
            nextNode: subNodes = subNodes->NextSiblingElement();
        }

        BuildPattern( pattern );

    }

}

void ChartSymbols::BuildPattern( OCPNPattern &pattern )
{
    Rule *pattmp = NULL;

    Rule *patt = (Rule*) calloc( 1, sizeof(Rule) );
    plib->pAlloc->append(patt );

    patt->RCID = pattern.RCID;
    patt->exposition.PXPO = new QString( pattern.description );
    memcpy( patt->name.PANM, pattern.name.toUtf8().data(), 8 );
    patt->bitmap.PBTM = NULL;
    patt->fillType.PATP = pattern.fillType;
    patt->spacing.PASP = pattern.spacing;

    patt->vector.PVCT = (char *) malloc( pattern.HPGL.length() + 1 );
    strcpy( patt->vector.PVCT, pattern.HPGL.toUtf8().data() );

    patt->colRef.PCRF = (char *) malloc( pattern.colorRef.length() + 1 );
    strcpy( patt->colRef.PCRF, pattern.colorRef.toUtf8().data() );

    SymbolSizeInfo_t patternSize;

    if( pattern.hasVector && !( pattern.preferBitmap && pattern.hasBitmap ) ) {
        patt->definition.PADF = 'V';
        patternSize = pattern.vectorSize;
    } else {
        patt->definition.PADF = 'R';
        patternSize = pattern.bitmapSize;
    }

    patt->pos.patt.minDist.PAMI = patternSize.minDistance;
    patt->pos.patt.maxDist.PAMA = patternSize.maxDistance;

    patt->pos.patt.pivot_x.PACL = patternSize.pivot.x;
    patt->pos.patt.pivot_y.PARW = patternSize.pivot.y;

    patt->pos.patt.bnbox_w.PAHL = patternSize.size.width;
    patt->pos.patt.bnbox_h.PAVL = patternSize.size.height;

    patt->pos.patt.bnbox_x.SBXC = patternSize.origin.x;
    patt->pos.patt.bnbox_y.SBXR = patternSize.origin.y;

    QRect graphicsLocation( pattern.bitmapSize.graphics.toPoint(), pattern.bitmapSize.size.toSize() );
    ( *symbolGraphicLocations )[pattern.name] = graphicsLocation;
//    qDebug()<<"symbol graphiclocation:"<<pattern.name<<graphicsLocation;
    // check if key already there
    pattmp = ( *plib->_patt_sym )[pattern.name];

    if( NULL == pattmp ) {
        ( *plib->_patt_sym )[pattern.name] = patt; // insert in hash table
    } else // already something here with same key...
    { // if the pattern names are not identical
        if( patt->name.PANM != pattmp->name.PANM ) {
            ( *plib->_patt_sym )[pattern.name] = patt; // replace the pattern
            plib->DestroyPatternRuleNode( pattmp ); // remember to free to replaced node
            // the node itself is destroyed as part of pAlloc
        }
    }
}

void ChartSymbols::ProcessSymbols( TiXmlElement* symbolNodes )
{

    ChartSymbol symbol;
    QString propVal;
    long numVal;

    for( TiXmlNode *childNode = symbolNodes->FirstChild(); childNode;
            childNode = childNode->NextSibling() ) {
        TiXmlElement *child = childNode->ToElement();

        TGET_INT_PROPERTY_VALUE( child, "RCID", symbol.RCID);

        symbol.hasVector = false;
        symbol.hasBitmap = false;
        symbol.preferBitmap = true;

        TiXmlElement* subNodes = child->FirstChild()->ToElement();

        while( subNodes ) {
            QString nodeType = QString::fromUtf8(subNodes->Value());
            QString nodeText = QString::fromUtf8(subNodes->GetText());

            if( nodeType == ("description") ) {
                symbol.description = nodeText;
                goto nextNode;
            }
            if( nodeType == ("name") ) {
                symbol.name = nodeText;
                goto nextNode;
            }
            if( nodeType == ("color-ref") ) {
                symbol.colorRef = nodeText;
                goto nextNode;
            }
            if( nodeType == ("definition") ) {
                if( !strcmp( subNodes->GetText(), "V" ) ) symbol.hasVector = true;
                goto nextNode;
            }
            if( nodeType == ("HPGL") ) {
                symbol.HPGL = nodeText;
                goto nextNode;
            }
            if( nodeType == ("prefer-bitmap") ) {
                if( nodeText.toLower() == ("no") ) symbol.preferBitmap = false;
                if( nodeText.toLower() == ("false") ) symbol.preferBitmap = false;
                goto nextNode;
            }
            if( nodeType == ("bitmap") ) {
                TGET_INT_PROPERTY_VALUE( subNodes, "width", symbol.bitmapSize.size.width );
                TGET_INT_PROPERTY_VALUE( subNodes, "height", symbol.bitmapSize.size.height );
                symbol.hasBitmap = true;

                TiXmlElement* bitmapNodes = subNodes->FirstChild()->ToElement();
                while( bitmapNodes ) {
                    QString bitmapnodeType( bitmapNodes->Value());
                    if( bitmapnodeType == ("distance") ) {
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "min", symbol.bitmapSize.minDistance );
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "max", symbol.bitmapSize.maxDistance );
                        goto nextBitmap;
                    }
                    if( bitmapnodeType == ("origin") ) {
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "x", symbol.bitmapSize.origin.x );
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "y", symbol.bitmapSize.origin.y );
                        goto nextBitmap;
                    }
                    if( bitmapnodeType == ("pivot") ) {
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "x", symbol.bitmapSize.pivot.x );
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "y", symbol.bitmapSize.pivot.y );
                        goto nextBitmap;
                    }
                    if( bitmapnodeType == ("graphics-location") ) {
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "x", symbol.bitmapSize.graphics.x );
                        TGET_INT_PROPERTY_VALUE( bitmapNodes, "y", symbol.bitmapSize.graphics.y );
                    }
                    nextBitmap: bitmapNodes = bitmapNodes->NextSiblingElement();
                }
                goto nextNode;
            }
            if( nodeType == ("vector") ) {
                TGET_INT_PROPERTY_VALUE( subNodes, "width", symbol.vectorSize.size.width );
                TGET_INT_PROPERTY_VALUE( subNodes, "height", symbol.vectorSize.size.height );
                symbol.hasVector = true;

                TiXmlElement* vectorNodes = subNodes->FirstChild()->ToElement();
                while( vectorNodes ) {
                    QString vectornodeType( vectorNodes->Value());
                    if( vectornodeType == ("distance") ) {
                        TGET_INT_PROPERTY_VALUE( vectorNodes, "min", symbol.vectorSize.minDistance );
                        TGET_INT_PROPERTY_VALUE( vectorNodes, "max", symbol.vectorSize.maxDistance );
                        goto nextVector;
                    }
                    if( vectornodeType == ("origin") ) {
                        TGET_INT_PROPERTY_VALUE( vectorNodes, "x", symbol.vectorSize.origin.x );
                        TGET_INT_PROPERTY_VALUE( vectorNodes, "y", symbol.vectorSize.origin.y );
                        goto nextVector;
                    }
                    if( vectornodeType == ("pivot") ) {
                        TGET_INT_PROPERTY_VALUE( vectorNodes, "x", symbol.vectorSize.pivot.x );
                        TGET_INT_PROPERTY_VALUE( vectorNodes, "y", symbol.vectorSize.pivot.y );
                        goto nextVector;
                    }
                    if( vectornodeType == ("HPGL") ) {
                        symbol.HPGL = QString( vectorNodes->GetText());
                    }
                    nextVector: vectorNodes = vectorNodes->NextSiblingElement();
                }
            }
            nextNode: subNodes = subNodes->NextSiblingElement();
        }

        BuildSymbol( symbol );
    }

}

void ChartSymbols::BuildSymbol( ChartSymbol& symbol )
{
    Rule *symb = (Rule*) calloc( 1, sizeof(Rule) );
    plib->pAlloc->append( symb );

    QString SVCT;
    QString SCRF;

    symb->RCID = symbol.RCID;
    memcpy( symb->name.SYNM, symbol.name.toUtf8().data(), 8 );

    symb->exposition.SXPO = new QString( symbol.description );

    symb->vector.SVCT = (char *) malloc( symbol.HPGL.length() + 1 );
    strcpy( symb->vector.SVCT, symbol.HPGL.toUtf8().data() );

    symb->colRef.SCRF = (char *) malloc( symbol.colorRef.length() + 1 );
    strcpy( symb->colRef.SCRF, symbol.colorRef.toUtf8().data() );

    symb->bitmap.SBTM = NULL;

    SymbolSizeInfo_t symbolSize;

    if( symbol.hasVector && !( symbol.preferBitmap && symbol.hasBitmap ) ) {
        symb->definition.SYDF = 'V';
        symbolSize = symbol.vectorSize;
    } else {
        symb->definition.SYDF = 'R';
        symbolSize = symbol.bitmapSize;
    }

    symb->pos.symb.minDist.PAMI = symbolSize.minDistance;
    symb->pos.symb.maxDist.PAMA = symbolSize.maxDistance;

    symb->pos.symb.pivot_x.SYCL = symbolSize.pivot.x;
    symb->pos.symb.pivot_y.SYRW = symbolSize.pivot.y;

    symb->pos.symb.bnbox_w.SYHL = symbolSize.size.width;
    symb->pos.symb.bnbox_h.SYVL = symbolSize.size.height;

    symb->pos.symb.bnbox_x.SBXC = symbolSize.origin.x;
    symb->pos.symb.bnbox_y.SBXR = symbolSize.origin.y;

    QRect graphicsLocation( symbol.bitmapSize.graphics.toPoint(), symbol.bitmapSize.size.toSize() );
    ( *symbolGraphicLocations )[symbol.name] = graphicsLocation;
//    qDebug()<<"symbol graphiclocation:"<<symbol.name<<graphicsLocation;
    // Already something here with same key? Then free its strings, otherwise they leak.
    Rule* symbtmp = ( *plib->_symb_sym )[symbol.name];
    if( symbtmp ) {
        free( symbtmp->colRef.SCRF );
        free( symbtmp->vector.SVCT );
        delete symbtmp->exposition.SXPO;
    }

    ( *plib->_symb_sym )[symbol.name] = symb;

}

bool ChartSymbols::LoadConfigFile(s52plib* plibArg, const QString & s52ilePath)
{
    TiXmlDocument doc;

    plib = plibArg;

    // Expect to find library data XML file in same folder as other S52 data.
    // Files in CWD takes precedence.

    QString name, extension;
    QString xmlFileName = ("chartsymbols.xml");

    QFileInfo info(s52ilePath);
    configFileDirectory = info.absolutePath();
    name = info.fileName();
    extension = info.suffix();
    QString fullFilePath = configFileDirectory + zchxFuncUtil::separator() + xmlFileName;

    if( QFile::exists(xmlFileName )) {
        fullFilePath = xmlFileName;
        configFileDirectory = (".");
    }

    if( !QFile::exists( fullFilePath ) ) {
        qDebug("ChartSymbols ConfigFile not found: %s", fullFilePath.toUtf8().data() );
        return false;
    }

#if 1   
    if(m_symbolsDoc.load_file( fullFilePath.toUtf8().data() ) ){
//        qDebug("ChartSymbols loaded from %s", fullFilePath.toUtf8().data());
        pugi::xml_node elements = m_symbolsDoc.child("chartsymbols");
        
        for (pugi::xml_node element = elements.first_child(); element; element = element.next_sibling()){
            if( !strcmp(element.name(), "color-tables") ) ProcessColorTables( element );
            else if( !strcmp(element.name(), "lookups") ) ProcessLookups( element );
            else if( !strcmp(element.name(), "line-styles") ) ProcessLinestyles( element );
            else if( !strcmp(element.name(), "patterns") ) ProcessPatterns( element );
            else if( !strcmp(element.name(), "symbols") ) ProcessSymbols( element );
            
        }
        m_symbolsDoc.reset();           // purge the document to recover memory;
        
    }    
    
#else
    if( !doc.LoadFile( (const char *) fullFilePath.toUtf8().data() ) ) {
        QString msg( ("    ChartSymbols ConfigFile Failed to load ") );
        msg += fullFilePath;
        ZCHX_LOGMSG( msg );
        return false;
    }

    QString msg( ("ChartSymbols loaded from ") );
    msg += fullFilePath;
    ZCHX_LOGMSG( msg );
    
    TiXmlHandle hRoot( doc.RootElement() );
    
    QString root = QString( doc.RootElement()->Value());
    if( root != ("chartsymbols" ) ) {
        ZCHX_LOGMSG(
                ("    ChartSymbols::LoadConfigFile(): Expected XML Root <chartsymbols> not found.") );
        return false;
    }

    TiXmlElement* pElem = hRoot.FirstChild().Element();

    for( ; pElem != 0; pElem = pElem->NextSiblingElement() ) {
        QString child = QString( pElem->Value());

        if( child == ("color-tables") ) ProcessColorTables( pElem );
        if( child == ("lookups") ) ProcessLookups( pElem );
        if( child == ("line-styles") ) ProcessLinestyles( pElem );
        if( child == ("patterns") ) ProcessPatterns( pElem );
        if( child == ("symbols") ) ProcessSymbols( pElem );

    }
#endif

    return true;
}

void ChartSymbols::SetColorTableIndex( int index )
{
    ColorTableIndex = index;
    LoadRasterFileForColorTable(ColorTableIndex);
}


int ChartSymbols::LoadRasterFileForColorTable( int tableNo, bool flush )
{

    if( tableNo == rasterSymbolsLoadedColorMapNumber && !flush ){
        if(rasterSymbolsTexture) return true;
        else if( !g_texture_rectangle_format && !rasterSymbols->isNull()) return true;
        if( !rasterSymbols->isNull()) return true;
    }
        
    
    colTable* coltab = (colTable *) colorTables->at( tableNo );

    QString filename = configFileDirectory + zchxFuncUtil::separator() + coltab->rasterFileName;

    QImage rasterFileImg;
    if( rasterFileImg.load(filename, /*QBitmap_TYPE_PNG*/"PNG" ) ) {
#ifdef ocpnUSE_GL
        /* for opengl mode, load the symbols into a texture */
        if(g_texture_rectangle_format) {

            int w = rasterFileImg.width();
            int h = rasterFileImg.height();

            //    Get the glRGBA format data from the wxImage
            unsigned char *d = rasterFileImg.bits();
            unsigned char *a = rasterFileImg.alphaChannel().bits();
            QImage::Format fm = rasterFileImg.format();

            /* combine rgb with alpha */
            unsigned char *e = (unsigned char *) malloc( w * h * 4 );
            if(d /*&& a*/){
                for( int y = 0; y < h; y++ )
                    for( int x = 0; x < w; x++ ) {
                        int off = ( y * w + x );
                        QColor color = rasterFileImg.pixelColor(x, y);
                        e[off * 4 + 0] = color.red();
                        e[off * 4 + 1] = color.green();
                        e[off * 4 + 2] = color.blue();
                        e[off * 4 + 3] = color.alpha();
                    }
            }
            if(!rasterSymbolsTexture)
                glGenTextures(1, &rasterSymbolsTexture);

            glBindTexture(g_texture_rectangle_format, rasterSymbolsTexture);

            /* unfortunately this texture looks terrible with compression */
            GLuint format = GL_RGBA;
            glTexImage2D(g_texture_rectangle_format, 0, format, w, h,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, e);

            glTexParameteri( g_texture_rectangle_format, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( g_texture_rectangle_format, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            rasterSymbolsTextureSize = QSize(w, h);

            glDisable( GL_TEXTURE_2D );
            
            free(e);
        } 
#endif
        {
            *rasterSymbols = QPixmap::fromImage(rasterFileImg);
        }

        rasterSymbolsLoadedColorMapNumber = tableNo;
        return true;
    }

    qDebug("ChartSymbols...Failed to load raster symbols file %s", filename.toUtf8().data() );
    return false;
}

// Convenience method for old s52plib code.
wxArrayPtrVoid* ChartSymbols::GetColorTables()
{
    return colorTables;
}

S52color* ChartSymbols::GetColor( const char *colorName, int fromTable )
{
    colTable *colortable;
    QString key = QString::fromUtf8(colorName, 5 );
    colortable = (colTable *) colorTables->at( fromTable );
    return &( colortable->S52Colors[key] );
}

QColor ChartSymbols::GetQColor( const QString &colorName, int fromTable )
{
    colTable *colortable;
    colortable = (colTable *) colorTables->at( fromTable );
    QColor c = colortable->QColors[colorName];
    return c;
}

QColor ChartSymbols::GetQColor( const char *colorName, int fromTable )
{
    QString key = QString::fromUtf8(colorName, 5 );
    return GetQColor( key, fromTable );
}

int ChartSymbols::FindColorTable(const QString & tableName)
{
    for( unsigned int i = 0; i < colorTables->count(); i++ ) {
        colTable *ct = (colTable *) colorTables->at( i );
        if( tableName == ct->tableName  ) {
            return i;
        }
    }
    return 0;
}

QString ChartSymbols::HashKey( const char* symbolName )
{
    char key[9];
    key[8] = 0;
    strncpy( key, symbolName, 8 );
    return QString( key);
}

QImage ChartSymbols::GetImage( const char* symbolName )
{
    QRect bmArea = ( *symbolGraphicLocations )[HashKey( symbolName )];
    if(!rasterSymbols->isNull()){
        return rasterSymbols->copy(bmArea).toImage();
    }
    else
        return QImage(1,1, QImage::Format_RGB32);
}

unsigned int ChartSymbols::GetGLTextureRect( QRect &rect, const char* symbolName )
{
    rect = ( *symbolGraphicLocations )[HashKey( symbolName )];
    return rasterSymbolsTexture;
}

QSize ChartSymbols::GLTextureSize()
{
    return rasterSymbolsTextureSize;
}

