/***************************************************************************
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
 **************************************************************************/

#ifndef __FONTMGR_H__
#define __FONTMGR_H__

#include "FontDesc.h"

class  OCPNQFontList
{
public:
    QFont   FindOrCreateFont(int pointSize, const QString& family, QFont::Style style, int weight, bool underline = false);
    void    FreeAll( void );

private:
    bool isSame(const QFont& font, int pointSize, const QString& family, QFont::Style style, int weight, bool underline);

    QList<QFont>        list;
};

/**
 * Manages the font list.
 *
 * Singleton.
 */
class FontMgr
{
    public:
        static FontMgr & Get();
    
        void SetLocale( QString &newLocale );
        QFont GetFont(const QString &TextElement, int default_size = 0);
        QColor GetFontColor( const QString &TextElement ) const;
        bool SetFontColor( const QString &TextElement, const QColor color );
    
        int GetNumFonts(void) const;
        const QString & GetConfigString(int i) const;
        const QString & GetDialogString(int i) const;
        const QString & GetNativeDesc(int i) const;
        QString GetFullConfigDesc( int i ) const;
        static QString GetFontConfigKey( const QString &description );
        
        QStringList &GetAuxKeyArray(){ return m_AuxKeyArray; }
        bool AddAuxKey( QString key );
        
        void LoadFontNative(const QString& pConfigString, const QString &pNativeDesc);
        bool SetFont(const QString &TextElement, QFont pFont, QColor color);
        void ScrubList( );
        MyFontDesc* FindFontByConfigString( QString pConfigString );
        
        QFont FindOrCreateFont( int point_size, QString family, QFont::Style style, int weight, bool underline = false);
        // For wxWidgets 2.8 compatability
        QFont FindOrCreateFont(int pointSize, QString family, int style, int weight,   bool underline = false)
        { return FindOrCreateFont(pointSize, family, (QFont::Style)style, weight, underline); }
        
        static void Shutdown();
        QFont  getSacledFontDefaultSize(const QString& item, int default_szie = 0);
        
    private: // private for singleton
        FontMgr();
        ~FontMgr();
        FontMgr(const FontMgr &) {}
        FontMgr & operator=(const FontMgr &) { return *this; }
        
    private:
        QFont GetSimpleNativeFont(int size, QString face);
    
        static FontMgr * instance;

        OCPNQFontList  m_QFontCache;
        FontList m_fontlist;
        QFont   pDefFont;
        QStringList m_AuxKeyArray;
};

#endif
