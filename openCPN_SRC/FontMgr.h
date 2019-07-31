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

class OCPNwxFontList;

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
        wxFont *GetFont(const QString &TextElement, int default_size = 0);
        wxColour GetFontColor( const QString &TextElement ) const;
        bool SetFontColor( const QString &TextElement, const wxColour color ) const;
    
        int GetNumFonts(void) const;
        const QString & GetConfigString(int i) const;
        const QString & GetDialogString(int i) const;
        const QString & GetNativeDesc(int i) const;
        QString GetFullConfigDesc( int i ) const;
        static QString GetFontConfigKey( const QString &description );
        
        wxArrayString &GetAuxKeyArray(){ return m_AuxKeyArray; }
        bool AddAuxKey( QString key );
        
        void LoadFontNative(QString *pConfigString, QString *pNativeDesc);
        bool SetFont(const QString &TextElement, wxFont *pFont, wxColour color);
        void ScrubList( );
        MyFontDesc *FindFontByConfigString( QString pConfigString );
        
        wxFont* FindOrCreateFont( int point_size, wxFontFamily family, 
                    wxFontStyle style, wxFontWeight weight, bool underline = false,
                    const QString &facename = wxEmptyString,
                    wxFontEncoding encoding = wxFONTENCODING_DEFAULT );
        // For wxWidgets 2.8 compatability
        wxFont *FindOrCreateFont(int pointSize, int family, int style, int weight,
                                 bool underline = false,
                                 const QString& face = wxEmptyString,
                                 wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
            { return FindOrCreateFont(pointSize, (wxFontFamily)family, (wxFontStyle)style,
                (wxFontWeight)weight, underline, face, encoding); }
        
        static void Shutdown();
        
    private: // private for singleton
        FontMgr();
        ~FontMgr();
        FontMgr(const FontMgr &) {}
        FontMgr & operator=(const FontMgr &) { return *this; }
        
    private:
        QString GetSimpleNativeFont(int size, QString face);
    
        static FontMgr * instance;

        OCPNwxFontList  *m_wxFontCache;
        FontList *m_fontlist;
        wxFont   *pDefFont;
        wxArrayString m_AuxKeyArray;
};

#endif
