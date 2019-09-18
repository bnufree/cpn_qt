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

#include <locale>

//#include <wx/gdicmn.h>
//#include <wx/tokenzr.h>

#include "FontMgr.h"
#include "OCPNPlatform.h"
#include <QGuiApplication>
#include "_def.h"


extern QString g_locale;
extern bool g_bresponsive;

extern QString s_locale;
extern int g_default_font_size;

FontMgr * FontMgr::instance = NULL;

FontMgr & FontMgr::Get()
{
    if (!instance)
        instance = new FontMgr;
    return *instance;
}

void FontMgr::Shutdown()
{
    if (instance)
    {
        delete instance;
        instance = NULL;
    }
}

FontMgr::FontMgr()
{
    //    Create the list of fonts
    s_locale = g_locale;
    
    //    Get a nice generic font as default
    pDefFont = FindOrCreateFont( 12, "Micorosoft Yahei", QFont::StyleNormal, QFont::Weight::Normal, false);

}

FontMgr::~FontMgr()
{
    m_fontlist.clear();
}

void FontMgr::SetLocale( QString& newLocale)
{
    s_locale = newLocale;
}

QColor  FontMgr::GetFontColor( const QString &TextElement ) const
{
    //    Look thru the font list for a match
    for(int i=0; i<m_fontlist.size(); i++)
    {
        MyFontDesc pmfd = m_fontlist[i];
        if( pmfd.m_dialogstring == TextElement )
        {
            if(pmfd.m_configstring.left(pmfd.m_configstring.indexOf('-')) == s_locale)
            {
                return pmfd.m_color;
            }
        }
    }

    return QColor ( 0, 0, 0 );
}

bool FontMgr::SetFontColor( const QString &TextElement, const QColor  color )
{
    //    Look thru the font list for a match
    for(int i=0; i<m_fontlist.size(); i++)
    {
        MyFontDesc &pmfd = m_fontlist[i];
        if( pmfd.m_dialogstring == TextElement )
        {
            if(pmfd.m_configstring.left(pmfd.m_configstring.indexOf('-')) == s_locale)
            {
                pmfd.m_color = color;
                return true;
            }
        }
    }


    return false;
}

QString FontMgr::GetFontConfigKey( const QString &description )
{
    // Create the configstring by combining the locale with
    // a hash of the font description. Hash is used because the i18n
    // description can contain characters that mess up the config file.

    QString configkey = s_locale;
    configkey.append("-");

    using namespace std;
    locale loc;
    const collate<char>& coll = use_facet<collate<char> >( loc );
    //    char cFontDesc[101];
    //    wcstombs( cFontDesc, description.c_str(), 100 );
    //    cFontDesc[100] = 0;

    QByteArray abuf = description.toUtf8();
    
    int fdLen = strlen( abuf.data() );

    configkey.append(QString("").sprintf("%08lx",coll.hash( abuf.data(), abuf.data() + fdLen ) ) );
    return configkey;
}

QFont FontMgr::GetFont( const QString &TextElement, int user_default_size )
{
    //    Look thru the font list for a match
    for(int i=0; i<m_fontlist.size(); i++)
    {
        MyFontDesc pmfd = m_fontlist[i];
        if( pmfd.m_dialogstring == TextElement )
        {
            if(pmfd.m_configstring.left(pmfd.m_configstring.indexOf('-')) == s_locale)
            {
                return pmfd.m_font;
            }
        }
    }

    // Found no font, so create a nice one and add to the list
    QString configkey = GetFontConfigKey( TextElement );

    //    Now create a benign, always present native font
    //    with optional user requested default size
    
    //    Get the system default font.
    QFont sys_font = QGuiApplication::font();
    int sys_font_size = sys_font.pointSize();
    QString family = sys_font.family();

    int new_size;
    if( 0 == user_default_size )
        new_size = sys_font_size;
    else
        new_size = user_default_size;

    QFont nf = GetSimpleNativeFont(new_size, family);
    QColor color = Qt::black;
    m_fontlist.append( MyFontDesc( TextElement, configkey, nf, color ) );

    return nf;
}

QFont FontMgr::GetSimpleNativeFont( int size, QString family )
{
    QFont font(family, size, QFont::Weight::Normal);
    font.setStyle(QFont::StyleNormal);
    font.setUnderline(false);
    return font;
}

bool FontMgr::SetFont(const QString &TextElement, QFont pFont, QColor  color)
{
    //    Look thru the font list for a match
    for(int i=0; i<m_fontlist.size(); i++)
    {
        MyFontDesc &pmfd = m_fontlist[i];
        if( pmfd.m_dialogstring == TextElement )
        {
            if(pmfd.m_configstring.left(pmfd.m_configstring.indexOf('-')) == s_locale)
            {
                pmfd.m_font = pFont;
                pmfd.m_nativeInfo = pFont.toString();
                pmfd.m_color = color;

                return true;
            }
        }
    }

    return false;
}

int FontMgr::GetNumFonts( void ) const
{
    return m_fontlist.size();
}

const QString & FontMgr::GetConfigString( int i ) const
{
    return m_fontlist[i].m_configstring;
}

const QString & FontMgr::GetDialogString( int i ) const
{
    return m_fontlist[i].m_dialogstring;
}

const QString & FontMgr::GetNativeDesc( int i ) const
{
    return m_fontlist[i].m_nativeInfo;
}

QString FontMgr::GetFullConfigDesc( int i ) const
{
    MyFontDesc pfd = m_fontlist[i];
    QString ret = pfd.m_dialogstring;
    ret.append( ( ":" ) );
    ret.append( pfd.m_nativeInfo );
    ret.append( ( ":" ) );

    QString cols( ("#000000") );
    if( pfd.m_color.isValid() ) cols = pfd.m_color.name();

    ret.append( cols );
    return ret;
}

MyFontDesc* FontMgr::FindFontByConfigString( QString pConfigString )
{
    //    Search for a match in the list
    for(int i=0; i<m_fontlist.size(); i++)
    {
        MyFontDesc pmfd = m_fontlist[i];
        if( pmfd.m_configstring == pConfigString ) {
            return &(m_fontlist[i]);
        }
    }
    
    return NULL;
}


void FontMgr::LoadFontNative(const QString& pConfigString, const QString &pNativeDesc )
{
    //    Parse the descriptor string

    QStringList tk = pNativeDesc.split( ":"  );
    int i = 0;
    QString dialogstring = tk[i++];
    QString nativefont = tk[i++];

    QString c = tk[i++];
    QColor  color;
    color.setNamedColor(c);// from string description

    //    Search for a match in the list
    bool found = false;

    for(int i=0; i<m_fontlist.size(); i++)
    {
        MyFontDesc &pmfd = m_fontlist[i];
        if( pmfd.m_configstring == pConfigString )
        {
            if(pmfd.m_configstring.left(pmfd.m_configstring.indexOf('-')) == s_locale)
            {
                pmfd.m_font.fromString(nativefont);
                pmfd.m_nativeInfo = nativefont;
                pmfd.m_color = color;
                found = true;
                break;
            }
        }
    }


    //    Create and add the font to the list
    if( !found ) {
        QFont f;
        f.fromString(nativefont);
        m_fontlist.append(MyFontDesc( dialogstring, pConfigString, f, color ) );

    }
}

QFont FontMgr::FindOrCreateFont( int point_size,  QString family, QFont::Style style, int weight, bool underline)
{
    return m_QFontCache.FindOrCreateFont( point_size, family, style, weight, underline);
}        

bool OCPNQFontList::isSame(const QFont& font, int pointSize, const QString& family, QFont::Style style, int weight, bool underline)
{
    return font.pointSize() == pointSize && font.family() == family && font.style() == style && font.weight() == weight && font.underline() == underline;
}

QFont OCPNQFontList::FindOrCreateFont(int point_size,  const QString& family, QFont::Style style, int weight, bool underline)
{
    QFont font(family, point_size, weight);
    font.setStyle(style);
    font.setUnderline(underline);
#if 0
    bool found = false;
    for(int i=0; i<list.size(); i++)
    {
        QFont f = list[i];
        if(isSame(f, point_size, family, style, weight, underline))
        {
            found = true;
            break;
        }
    }
    if(!found) list.append(font);
#endif
    return font;
}

void OCPNQFontList::FreeAll( void )
{
    list.clear();
}

static QString FontCandidates[] = {
    ("AISTargetAlert"),
    ("AISTargetQuery"),
    ("StatusBar"),
    ("AIS Target Name" ),
    ("ObjectQuery"),
    ("RouteLegInfoRollover"),
    ("ExtendedTideIcon"),
    ("CurrentValue"),
    ("Console Legend"),
    ("Console Value"),
    ("AISRollover"),
    ("TideCurrentGraphRollover"),
    ("Marks"),
    ("ChartTexts"),
    ("ToolTips"),
    ("Dialog"),
    ("Menu"),
    ("END_OF_LIST")
};


void FontMgr::ScrubList( )
{
    QString now_locale = g_locale;
    QStringList string_array;
    
    //  Build the composite candidate array
    QStringList candidateArray;
    unsigned int i = 0;
    
    // The fixed, static list
    while( true ){
        QString candidate = FontCandidates[i];
        if(candidate == ("END_OF_LIST") ) {
            break;
        }
        
        candidateArray.append(candidate);
        i++;
    }

    //  The Aux Key array
    for(unsigned int i=0 ; i <  m_AuxKeyArray.size() ; i++){
        candidateArray.append(m_AuxKeyArray[i]);
    }
    
    
    for(unsigned int i = 0; i < candidateArray.size() ; i++ ){
        QString candidate = candidateArray[i];
        
        //  For each font identifier string in the FontCandidate array...
        
        //  In the current locale, walk the loaded list looking for a translation
        //  that is correct, according to the currently load .mo file.
        //  If found, add to a temporary array
        
        QString trans = /*wxGetTranslation(candidate)*/candidate;
        
        for(int i=0; i<m_fontlist.size(); i++)
        {
            MyFontDesc pmfd = m_fontlist[i];
            QString tlocale = pmfd.m_configstring.left(pmfd.m_configstring.indexOf('-'));
            if( tlocale == now_locale) {
                if(trans == pmfd.m_dialogstring){
                    string_array.append(pmfd.m_dialogstring);
                }
            }
        }
    }

    // now we have an array of correct translations
    // Walk the loaded list again.
    // If a list item's translation is not in the "good" array, mark it for removal
    
    for(int i=0; i<m_fontlist.size(); i++)
    {
        MyFontDesc &pmfd = m_fontlist[i];
        QString tlocale = pmfd.m_configstring.left(pmfd.m_configstring.indexOf('-'));
        if( tlocale == now_locale) {
            bool bfound = false;
            for(unsigned int i=0 ; i < string_array.count() ; i++){
                if( string_array[i] == pmfd.m_dialogstring){
                    bfound = true;
                    break;
                }
            }
            if(!bfound){        // mark for removal
                pmfd.m_dialogstring = ("");
                pmfd.m_configstring = ("");
            }
        }
    }
    
    //  Remove the marked list items
    for(int i=0; i<m_fontlist.size(); i++)
    {
        MyFontDesc pmfd = m_fontlist[i];
        if( pmfd.m_dialogstring == ("") ) {
            m_fontlist.removeAt(i);
            i--;
        }
    }

    //  And finally, for good measure, make sure that everything in the candidate array has a valid entry in the list
    i = 0;
    while( true ){
        QString candidate = FontCandidates[i];
        if(candidate == ("END_OF_LIST") ) {
            break;
        }

        GetFont( /*wxGetTranslation*/(candidate), g_default_font_size );

        i++;
    }


}

bool FontMgr::AddAuxKey( QString key )
{
    for(unsigned int i=0 ; i <  m_AuxKeyArray.count() ; i++){
        if(m_AuxKeyArray[i] == key)
            return false;
    }
    m_AuxKeyArray.append(key);
    return true;
}

QFont FontMgr::getSacledFontDefaultSize(const QString &item, int default_size)
{
    QFont dFont = GetFont( item, default_size );
    int req_size = dFont.pointSize();

    if( g_bresponsive )
    {
        //      Adjust font size to be no smaller than xx mm actual size
        double scaled_font_size = dFont.pointSize();

        double points_per_mm  = zchxFuncUtil::getFontPointsperPixel() * OCPNPlatform::instance()->GetDisplayDPmm();
        double min_scaled_font_size = 3 * points_per_mm;    // smaller than 3 mm is unreadable
        int nscaled_font_size = fmax( qRound(scaled_font_size), min_scaled_font_size );

        if(req_size >= nscaled_font_size)
            return dFont;
        else{
            return FindOrCreateFont( nscaled_font_size,
                                     dFont.family(),
                                     dFont.style(),
                                     dFont.weight());
        }
    }
    return dFont;
}

