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
#include "zchxconfig.h"
#include <stdlib.h>
#include "OCPNPlatform.h"

#include "styles.h"
//#include "chart1.h"
//#include "wx28compat.h"
#include <QtSvg>
#include <QPainter>
#include <QPixmap>

extern QColor GetGlobalColor(const QString& colorName);

using namespace ocpnStyle;

void bmdump(wxBitmap bm, QString name)
{
    QImage img = bm.ConvertToImage();
    img.save(name, "PNG" );
}

static wxBitmap LoadSVG( const QString filename, unsigned int width, unsigned int height )
{
#ifdef ocpnUSE_SVG
    wxBitmap bitmap(width, height);
    QPainter painter;
    painter.begin(bitmap.GetHandle());
    QSvgRenderer renderObj(filename);
    if(renderObj.isValid())
    {
        renderObj.render(&painter);
    }
    painter.end();
    return bitmap;
#else
    return wxBitmap(width, height);
#endif // ocpnUSE_SVG
}

// This function can be used to create custom bitmap blending for all platforms
// where 32 bit bitmap ops are broken. Can hopefully be removed for wxWidgets 3.0...

wxBitmap MergeBitmaps( wxBitmap back, wxBitmap front, QSize offset )
{
    //  If the front bitmap has no alpha channel, then merging will accomplish nothing
    //  So, simply return the bitmap intact
    //  However, if the bitmaps are different sizes, do the render anyway.
    QImage im_front = front.ConvertToImage();
    if(!im_front.hasAlphaChannel() && (front.width() == back.width()) )
        return front;

#ifdef __WXMSW__
    //  WxWidgets still has some trouble overlaying bitmaps with transparency.
    //  This is true on wx2.8 as well as wx3.0
    //  In the specific case where the back bitmap has alpha, but the front does not,
    //  we obviously mean for the front to be drawn over the back, with 100% opacity.
    //  To do this, we need to convert the back bitmap to simple no-alpha model.    
    if(!im_front.HasAlpha()){
        QImage im_back = back.ConvertToImage();
        back = wxBitmap(im_back);
    }
#endif    
    
    wxBitmap merged( back.GetWidth(), back.GetHeight(), back.GetDepth() );
    
    // Manual alpha blending for broken wxWidgets alpha bitmap support, pervasive in wx2.8.
    // And also in wx3, at least on Windows...
#if 1 //!wxCHECK_VERSION(2,9,4) 

//#if !wxCHECK_VERSION(2,9,4)
//    merged.UseAlpha();
//    back.UseAlpha();
//    front.UseAlpha();
//#endif
    
    QImage im_back = back.ConvertToImage();
    QImage im_result = back.ConvertToImage();// Only way to make result have alpha channel in wxW 2.8.

    unsigned char *presult = im_result.bits();
    unsigned char *pback = im_back.bits();
    unsigned char *pfront = im_front.bits();

    unsigned char *afront = NULL;
    if( im_front.hasAlphaChannel() )
        afront = im_front.alphaChannel().bits();

    unsigned char *aback = NULL;
    if( im_back.hasAlphaChannel() )
        aback = im_back.alphaChannel().bits();

    unsigned char *aresult = NULL;
    if( im_result.hasAlphaChannel() )
        aresult = im_result.alphaChannel().bits();

    // Do alpha blending, associative version of "over" operator.
    if(presult && pback && pfront){ 
        for( int i = 0; i < back.height(); i++ ) {
            for( int j = 0; j < back.width(); j++ ) {

                int fX = j - offset.width();
                int fY = i - offset.height();

                bool inFront = true;
                if( fX < 0 || fY < 0 ) inFront = false;
                if( fX >= front.width() ) inFront = false;
                if( fY >= front.height() ) inFront = false;

                if( inFront ) {
                    double alphaF = 1.0;
                    if (afront) alphaF = (double) ( *afront++ ) / 255.0;
                    double alphaB = 1.0;
                    if (aback) alphaB = (double) ( *aback++ ) / 255.0;
                    double alphaRes = alphaF + alphaB * ( 1.0 - alphaF );
                    if (aresult) {
                        unsigned char a = alphaRes * 255;
                        *aresult++ = a;
                    }
                    unsigned char r = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
                    *presult++ = r;
                    unsigned char g = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
                    *presult++ = g;
                    unsigned char b = (*pfront++ * alphaF + *pback++ * alphaB * ( 1.0 - alphaF )) / alphaRes;
                    *presult++ = b;
                } else {
                    if (aresult && aback) *aresult++ = *aback++;
                    *presult++ = *pback++;
                    *presult++ = *pback++;
                    *presult++ = *pback++;
                }
            }
        }
    }
    merged = wxBitmap( im_result );
#else
    wxMemoryDC mdc( merged );
    mdc.Clear();
    mdc.DrawBitmap( back, 0, 0, true );
    mdc.DrawBitmap( front, offset.x, offset.y, true );
    mdc.SelectObject( wxNullBitmap );
#endif

    return merged;
}

// The purpouse of ConvertTo24Bit is to take an icon with 32 bit depth and alpha
// channel and put it in a 24 bit deep bitmap with no alpha, that can be safely
// drawn in the crappy wxWindows implementations.

wxBitmap ConvertTo24Bit( QColor bgColor, wxBitmap front ) {
    if( front.GetDepth() == 24 ) return front;

//#if !wxCHECK_VERSION(2,9,4)
//    front.UseAlpha();
//#endif

    QImage im_front = front.ConvertToImage();
    unsigned char *pfront = im_front.bits();
    if(!pfront)
        return wxBitmap();
    
    unsigned char *presult = (unsigned char *)malloc(front.width() * front.height() * 3);
    if(!presult)
        return wxBitmap();
    
    unsigned char *po_result = presult;

    
    unsigned char *afront = NULL;
    if( im_front.hasAlphaChannel() )
    afront = im_front.alphaChannel().bits();

    for( int i = 0; i < front.width(); i++ ) {
        for( int j = 0; j < front.height(); j++ ) {

            double alphaF = 1.0;
            if(afront)
                alphaF = (double) ( *afront++ ) / 256.0;
             unsigned char r = *pfront++ * alphaF + bgColor.red() * ( 1.0 - alphaF );
             *presult++ = r;
             unsigned char g = *pfront++ * alphaF + bgColor.green() * ( 1.0 - alphaF );
             *presult++ = g;
             unsigned char b = *pfront++ * alphaF + bgColor.blue() * ( 1.0 - alphaF );
             *presult++ = b;
        }
    }

    QImage::Format format = afront == NULL ? QImage::Format_RGB32 : QImage::Format_ARGB32;
    QImage im_result(po_result, front.width(), front.height(), format);
    
    wxBitmap result = wxBitmap( im_result );
    return result;
}


bool Style::NativeToolIconExists(const QString & name)
{
    if( toolIndex.find( name ) == toolIndex.end() )
        return false;
    else
        return true;
}
    

// Tools and Icons perform on-demand loading and dimming of bitmaps.
// Changing color scheme invalidatres all loaded bitmaps.

wxBitmap Style::GetIcon(const QString & name, int width, int height, bool bforceReload)
{
    if( iconIndex.find( name ) == iconIndex.end() ) {
        QString msg("The requested icon was not found in the style: ");
        msg += name;
        qDebug()<<msg;
        return wxBitmap( GetToolSize().width(), GetToolSize().height() ); // Prevents crashing.
    }

    int index = iconIndex[name]; // FIXME: this operation is not const but should be, use 'find'


    Icon* icon = icons[index];
//    qDebug()<<"icon:"<<icon<<icon->name<<icon->iconLoc<<icon->size;

    if( icon->loaded && !bforceReload)
        return icon->icon;
    if( icon->size.width() == 0 )
        icon->size = toolSize[currentOrientation];
    
    QSize retSize = icon->size;
    if((width > 0) && (height > 0))
        retSize = QSize(width, height);
    
    wxBitmap bm;
#ifdef ocpnUSE_SVG
    QString fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + name + (".svg");
    if( QFile::exists(fullFilePath ) )
    {
        bm = LoadSVG( fullFilePath, retSize.width(), retSize.height());
    } else
    {
#endif // ocpnUSE_SVG
        QRect location( icon->iconLoc, icon->size );
//        graphics->SaveFile("zchxTest.png", "png");
//        qDebug()<<"graphics img size:"<<graphics->GetWidth()<<graphics->GetHeight();
        QPixmap res = graphics->GetHandle()->copy(/*QRect(540,100, 32,32)*/location);
//        res.save("copy.png", "png");
        bm = graphics->GetSubBitmap( location );
        if(retSize != icon->size){
            QImage scaled_image = bm.ConvertToImage();
            bm = wxBitmap(scaled_image.scaled(retSize.width(), retSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        
#ifdef ocpnUSE_SVG
    }
#endif // ocpnUSE_SVG
    icon->icon = SetBitmapBrightness( bm, colorscheme );
    icon->loaded = true;
    return icon->icon;
}

wxBitmap Style::GetToolIcon(const QString & toolname, int iconType, bool rollover, int width, int height )
{

    if( toolIndex.find( toolname ) == toolIndex.end() ) {
//  This will produce a flood of log messages for some PlugIns, notably WMM_PI, and GRADAR_PI
//        QString msg( _T("The requested tool was not found in the style: ") );
//        msg += toolname;
//        qDebug()<<( msg );
        return wxBitmap( GetToolSize().width(), GetToolSize().height(), 1 );
    }

    int index = toolIndex[toolname];

    Tool* tool = (Tool*) tools[index];
 
    QSize size = tool->customSize;
    if( size.width() == 0 )
        size = toolSize[currentOrientation];
    
    QSize retSize = size;
    if((width > 0) && (height > 0))
        retSize = QSize(width, height);
    
    switch( iconType ){
        case TOOLICON_NORMAL: {
            if( tool->iconLoaded && !rollover ) return tool->icon;
            if( tool->rolloverLoaded && rollover ) return tool->rollover;

            QRect location( tool->iconLoc, size );

            //  If rollover icon does not exist, use the defult icon
            if( rollover ) {
                if( (tool->rolloverLoc.x() != 0) || (tool->rolloverLoc.y() != 0) )
                    location = QRect( tool->rolloverLoc, size );
            }

            if( currentOrientation ) {
                location.translate(verticalIconOffset.width() * (-1), verticalIconOffset.height() * (-1));
            }

            wxBitmap bm;
#ifdef ocpnUSE_SVG
            QString fullFilePath;
            if( rollover )
            {
                fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + toolname + ("_rollover.svg");
                if( !QFile::exists( fullFilePath ) ) fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + toolname + (".svg");
            } else
            {
                fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + toolname + (".svg");
            }
            if( QFile::exists(fullFilePath ) )
                bm = LoadSVG( fullFilePath, retSize.width(), retSize.height() );
            else
            {
                ///qDebug()<<( _T("Can't find SVG: ") + fullFilePath );
#endif // ocpnUSE_SVG
                bm = graphics->GetSubBitmap( location );
                
                if( hasBackground ) {
                    bm = MergeBitmaps( GetNormalBG(), bm, QSize( 0, 0 ) );
                } else {
                    wxBitmap bg( GetToolSize().width(), GetToolSize().height() );
                    QPainter mdc;
                    mdc.begin(bg.GetHandle());
                    mdc.setBackground( QBrush( GetGlobalColor( ("GREY2") ), Qt::SolidPattern ) );
                    mdc.eraseRect(0, 0, mdc.device()->width(), mdc.device()->height());
                    mdc.end();
                    bm = MergeBitmaps( bg, bm, QSize( 0, 0 ) );
                }
                
                if(retSize != size){
                    QImage scaled_image = bm.ConvertToImage();
                    bm = wxBitmap(scaled_image.scaled(retSize.width(), retSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                }
                
#ifdef ocpnUSE_SVG
            }
#endif // ocpnUSE_SVG

            if( rollover ) {
                tool->rollover = SetBitmapBrightness( bm, colorscheme );
                tool->rolloverLoaded = true;
                return tool->rollover;
            } else {
                if( toolname == ("mob_btn") ) {
                    double dimLevel = 1.0;
                    if(colorscheme ==  GLOBAL_COLOR_SCHEME_DUSK)
                        dimLevel = 0.5;
                    else if(colorscheme ==  GLOBAL_COLOR_SCHEME_NIGHT)
                        dimLevel = 0.5;
                    tool->icon = SetBitmapBrightnessAbs( bm, dimLevel );
                }
                else {
                    tool->icon = SetBitmapBrightness( bm, colorscheme );
                }
                
                tool->iconLoaded = true;
                return tool->icon;
            }
        }
        case TOOLICON_TOGGLED: {
            if( tool->toggledLoaded && !rollover ) return tool->toggled;
            if( tool->rolloverToggledLoaded && rollover ) return tool->rolloverToggled;

            QRect location( tool->iconLoc, size );
            if( rollover ) location = QRect( tool->rolloverLoc, size );
            QSize offset( 0, 0 );
            if( GetToolSize() != GetToggledToolSize() ) {
                offset = GetToggledToolSize() - GetToolSize();
                offset /= 2;
            }
            if( currentOrientation ) {
                location.translate(verticalIconOffset.width() * (-1), verticalIconOffset.height() * (-1));
            }
            wxBitmap bm;
#ifdef ocpnUSE_SVG
            QString fullFilePath;
            if( rollover )
                fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + toolname + ("_rollover_toggled.svg");
            else
                fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + toolname + ("_toggled.svg");
            if( QFile::exists( fullFilePath ) )
                bm = LoadSVG( fullFilePath, retSize.width(), retSize.height() );
            else
            {
                // Could not find a toggled SVG, so try to make one
                if( rollover )
                    fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + toolname + ("_rollover.svg");
                else
                    fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + toolname + (".svg");

                if( QFile::exists( fullFilePath ) ){
                    bm = LoadSVG( fullFilePath, retSize.width(), retSize.height() );
                    
                    wxBitmap bmBack = GetToggledBG();
                    if( (bmBack.width() != retSize.width()) || (bmBack.height() != retSize.height()) ){
                        QImage scaled_back = bmBack.ConvertToImage();
                        bmBack = wxBitmap(scaled_back.scaled(retSize.width(), retSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                    }
                    bm = MergeBitmaps( bmBack, bm, QSize(0,0) );
                }
            }
                
#endif // ocpnUSE_SVG
            if(!bm.isOK()){
                bm = graphics->GetSubBitmap( location );
                bm = MergeBitmaps( GetToggledBG(), bm, offset );
                
                if(retSize != size){
                    QImage scaled_image = bm.ConvertToImage();
                    bm = wxBitmap(scaled_image.scaled(retSize.width(), retSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                }
            }
                
            if( rollover ) {
                tool->rolloverToggled = SetBitmapBrightness( bm, colorscheme );
                tool->rolloverToggledLoaded = true;
                return tool->rolloverToggled;
            } else {
                tool->toggled = SetBitmapBrightness( bm, colorscheme );
                tool->toggledLoaded = true;
                return tool->toggled;
            }
        }
        case TOOLICON_DISABLED: {
            if( tool->disabledLoaded ) return tool->disabled;
            QRect location( tool->disabledLoc, size );

            wxBitmap bm;
#ifdef ocpnUSE_SVG
            QString fullFilePath = myConfigFileDir + this->sysname + zchxFuncUtil::separator() + toolname + ("_disabled.svg");
            if( QFile::exists(fullFilePath ) )
                bm = LoadSVG( fullFilePath, retSize.width(), retSize.height());
            else
            {
                ///qDebug()<<( _T("Can't find SVG: ") + fullFilePath );
#endif // ocpnUSE_SVG
                bm = graphics->GetSubBitmap( location );
                
                if( hasBackground ) {
                    bm = MergeBitmaps( GetNormalBG(), bm, QSize( 0, 0 ) );
                }
                
                if(retSize != size){
                    QImage scaled_image = bm.ConvertToImage();
                    bm = wxBitmap(scaled_image.scaled(retSize.width(), retSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                }
#ifdef ocpnUSE_SVG
            }
#endif // ocpnUSE_SVG
            if( currentOrientation ) {
                location.translate(verticalIconOffset.width() * (-1), verticalIconOffset.height() * (-1));
            }
            tool->disabled = SetBitmapBrightness( bm, colorscheme );
            tool->disabledLoaded = true;
            return tool->disabled;
        }
    }
    QString msg( ("A requested icon type for this tool was not found in the style: ") );
    msg += toolname;
    qDebug()<< msg ;
    return wxBitmap( GetToolSize().width(), GetToolSize().height() ); // Prevents crashing.
}

wxBitmap Style::BuildPluginIcon( wxBitmap &bm, int iconType, double factor )
{
    if(  !bm.isOK() ) return wxBitmap();

    wxBitmap iconbm;

    switch( iconType ){
        case TOOLICON_NORMAL:
        case TOOLICON_TOGGLED:
            {
            if( hasBackground ) {
                wxBitmap bg;
                if(iconType == TOOLICON_NORMAL)
                    bg = GetNormalBG();
                else
                    bg = GetToggledBG();

                if((bg.GetWidth() >= bm.GetWidth()) && (bg.GetHeight() >= bm.GetHeight())){
                    int w = bg.GetWidth() * factor;
                    int h = bg.GetHeight() * factor;
                    QImage scaled_image = bg.ConvertToImage();
                    bg = wxBitmap(scaled_image.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                    
                    QSize offset = QSize( bg.width() - bm.width(), bg.height() - bm.height() );
                    offset /= 2;
                    iconbm = MergeBitmaps( bg, bm, offset );
                }
                else{
                    // A bit of contorted logic for non-square backgrounds...
                    double factor = ((double)bm.GetHeight()) / bg.GetHeight();
                    int nw = bg.GetWidth() * factor;
                    int nh = bm.GetHeight();
                    if(bg.GetWidth() == bg.GetHeight())
                        nw = nh;
                    QImage scaled_image = bg.ConvertToImage();
                    bg = wxBitmap(scaled_image.scaled(nw, nh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                    
                    QSize offset = QSize( bg.GetWidth() - bm.GetWidth(), bg.GetHeight() - bm.GetHeight() );
                    offset /= 2;
                    iconbm = MergeBitmaps( bg, bm, offset );
                }
                
            } else {
                wxBitmap bg( GetToolSize().width(), GetToolSize().height() );
                QPainter mdc;
                mdc.begin(bg.GetHandle());
                QSize offset = GetToolSize() - QSize( bm.GetWidth(), bm.GetHeight() );
                offset /= 2;
                mdc.setBackground( QBrush( GetGlobalColor( ("GREY2") ), Qt::SolidPattern ) );
                mdc.eraseRect(0, 0, mdc.device()->width(), mdc.device()->height());
                mdc.end();
                iconbm = MergeBitmaps( bg, bm, offset );
            }
            break;
        }
        default:
            return wxBitmap();
            break;
    }
    return SetBitmapBrightness( iconbm, colorscheme );
}

wxBitmap Style::SetBitmapBrightness( wxBitmap& bitmap, ColorScheme cs )
{
    double dimLevel;
    switch( cs ){
        case GLOBAL_COLOR_SCHEME_DUSK: {
            dimLevel = 0.8;
            break;
        }
        case GLOBAL_COLOR_SCHEME_NIGHT: {
            dimLevel = 0.5;
            break;
        }
        default: {
            return bitmap;
        }
    }
    
    return SetBitmapBrightnessAbs(bitmap, dimLevel);
}

wxBitmap Style::SetBitmapBrightnessAbs( wxBitmap& bitmap, double level )
{
    QImage image = bitmap.ConvertToImage();

    int gimg_width = image.width();
    int gimg_height = image.height();

    for( int iy = 0; iy < gimg_height; iy++ ) {
        for( int ix = 0; ix < gimg_width; ix++ ) {            
            if( !zchxFuncUtil::isImageTransparent(image, ix, iy, 30 ) ) {
                QColor old = image.pixelColor(ix, iy);
                image.setPixelColor(ix, iy, zchxFuncUtil::getNewRGBColor(old, level));
            }
        }
    }
    return wxBitmap( image );
}

wxBitmap Style::GetNormalBG()
{
    QSize size = toolSize[currentOrientation];
    return graphics->GetSubBitmap(
            QRect( normalBGlocation[currentOrientation].x(), normalBGlocation[currentOrientation].y(),
                    size.width(), size.height() ) );
}

wxBitmap Style::GetActiveBG()
{
    return graphics->GetSubBitmap(
            QRect( activeBGlocation[currentOrientation].x(), activeBGlocation[currentOrientation].y(),
                    toolSize[currentOrientation].width(), toolSize[currentOrientation].height() ) );
}

wxBitmap Style::GetToggledBG()
{
    QSize size = toolSize[currentOrientation];
    if( toggledBGSize[currentOrientation].width() ) {
        size = toggledBGSize[currentOrientation];
    }
    return graphics->GetSubBitmap( QRect( toggledBGlocation[currentOrientation], size ) );
}

wxBitmap Style::GetToolbarStart()
{
    QSize size = toolbarStartSize[currentOrientation];
    if( toolbarStartSize[currentOrientation].width() == 0 ) {
        size = toolbarStartSize[currentOrientation];
    }
    return graphics->GetSubBitmap( QRect( toolbarStartLoc[currentOrientation], size ) );
}

wxBitmap Style::GetToolbarEnd()
{
    QSize size = toolbarEndSize[currentOrientation];
    if( toolbarEndSize[currentOrientation].width() == 0 ) {
        size = toolbarEndSize[currentOrientation];
    }
    return graphics->GetSubBitmap( QRect( toolbarEndLoc[currentOrientation], size ) );
}

int Style::GetToolbarCornerRadius()
{
    return cornerRadius[currentOrientation];
}

void Style::DrawToolbarLineStart( wxBitmap& bmp, double scale )
{
    if( !HasToolbarStart() ) return;
    QPainter dc( bmp .GetHandle());
    wxBitmap sbmp = GetToolbarStart();
    if( fabs(scale - 1.0) > 0.01){
        int h = sbmp.GetHeight() * scale;
        int w = sbmp.GetWidth() * scale;
        if( (h > 0) && (w > 0)){
            QImage scaled_image = sbmp.ConvertToImage();
            sbmp = wxBitmap(scaled_image.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    dc.drawPixmap(0, 0, *(sbmp.GetHandle()));
}

void Style::DrawToolbarLineEnd( wxBitmap& bmp, double scale )
{
    if( !HasToolbarStart() ) return;
    QPainter dc(bmp.GetHandle());
    wxBitmap sbmp = GetToolbarEnd();
    if( fabs(scale - 1.0) > 0.01){
        int h = sbmp.GetHeight() * scale;
        int w = sbmp.GetWidth() * scale;
        if( (h > 0) && (w > 0)){
            QImage scaled_image = sbmp.ConvertToImage();
            sbmp = wxBitmap(scaled_image.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    
    if( currentOrientation ) {
        dc.drawPixmap(0, bmp.GetHeight() - sbmp.GetHeight(), *(sbmp.GetHandle()) );
    } else {
        dc.drawPixmap( bmp.GetWidth() - sbmp.GetWidth(), 0, *(sbmp.GetHandle()) );
    }
//    dc.SelectObject( wxNullBitmap );
}

void Style::SetOrientation( Direction orient )
{
    int newOrient = 0;
    if( orient == Vertical ) newOrient = 1;
    if( newOrient == currentOrientation ) return;
    currentOrientation = newOrient;
    Unload();
}

int Style::GetOrientation()
{
    return currentOrientation;
}

void Style::SetColorScheme( ColorScheme cs )
{
    colorscheme = cs;
    Unload();
    
    if( (consoleTextBackgroundSize.width()) && (consoleTextBackgroundSize.height())) {
        wxBitmap bm = graphics->GetSubBitmap(
            QRect( consoleTextBackgroundLoc, consoleTextBackgroundSize ) );

    // The background bitmap in the icons file may be too small, so will grow it arbitrailly
        QImage image = bm.ConvertToImage();
        image.scaled(consoleTextBackgroundSize.width() * 2, consoleTextBackgroundSize.height() * 2  );
        wxBitmap bn( image );
        consoleTextBackground = SetBitmapBrightness( bn, cs );
    }
}

void Style::Unload()
{
    for( unsigned int i = 0; i < tools.count(); i++ ) {
        Tool* tool = (Tool*) tools[i];
        tool->Unload();
    }

    for( unsigned int i = 0; i < icons.count(); i++ ) {
        Icon* icon = icons[i];
        icon->Unload();
    }
}

Style::Style( void )
{
    graphics = NULL;
    currentOrientation = 0;
    colorscheme = GLOBAL_COLOR_SCHEME_DAY;
    marginsInvisible = false;
    hasBackground = false;
    chartStatusIconWidth = 0;
    chartStatusWindowTransparent = false;
    embossHeight = 40;
    embossFont = "";

    //  Set compass window style defauilts
    compassMarginTop = 4;
    compassMarginRight = 0;
    compassMarginBottom = 4;
    compassMarginLeft = 4;
    compasscornerRadius = 3;
    compassXoffset = 0;
    compassYoffset = 0;

    for( int i = 0; i < 2; i++ ) {
        toolbarStartLoc[i] = QPoint( 0, 0 );
        toolbarEndLoc[i] = QPoint( 0, 0 );
        cornerRadius[i] = 0;
    }
}

Style::~Style( void )
{
    for( unsigned int i = 0; i < tools.count(); i++ ) {
        delete (Tool*) ( tools[i] );
    }
    tools.clear();

    for( unsigned int i = 0; i < icons.count(); i++ ) {
        delete ( icons[i] );
    }
    icons.clear();

    if( graphics ) delete graphics;

    toolIndex.clear();
    iconIndex.clear();
}
StyleManager* StyleManager::mInstance = 0;
StyleManager::MGarbage StyleManager::Garbage;

StyleManager* StyleManager::instance()
{
    if(mInstance == 0) mInstance = new StyleManager();
    return mInstance;
}

StyleManager::StyleManager(void)
{
    isOK = false;
    currentStyle = NULL;
    Init( zchxFuncUtil::getDataDir() + zchxFuncUtil::separator() + ("uidata") + zchxFuncUtil::separator() );
    Init( zchxFuncUtil::getDataDir() );
    Init( zchxFuncUtil::getDataDir() + zchxFuncUtil::separator() + ("/.opencpn") + zchxFuncUtil::separator() );
    SetStyle( ("") );
#ifdef ocpnUSE_SVG
    qDebug()<<(("Using SVG Icons"));
#else
    qDebug()<<(("Using PNG Icons"));
#endif    
}

StyleManager::StyleManager(const QString & configDir)
{
    isOK = false;
    currentStyle = NULL;
    Init( configDir );
    SetStyle(("") );
}

StyleManager::~StyleManager(void)
{
    for( unsigned int i = 0; i < styles.count(); i++ ) {
        delete (Style*) ( styles[i] );
    }
    styles.clear();
}

void StyleManager::Init(const QString & fromPath)
{
    TiXmlDocument doc;
    QDir dir(fromPath);
    if(!dir.exists())
    {
        QString msg = ("No styles found at: ");
        msg.append(fromPath);
        qDebug()<<( msg );
        return;
    }

    QFileInfoList filelist = dir.entryInfoList(QStringList()<<("style*.xml"), QDir::Files);

    // We allow any number of styles to load from files called style<something>.xml

    if( !filelist.size() ) {
        QString msg = ("No styles found at: ");
        msg.append(fromPath);
        qDebug()<<msg;
        return;
    }

    bool firstFile = true;
    for(int i=0; i<filelist.size(); i++)
    {
        QFileInfo file = filelist[i];
//        QString name, extension;

//        if( !firstFile ) more = dir.GetNext( &filename );
//        if( !more ) break;
//        firstFile = false;

        QString fullFilePath = file.absoluteFilePath();

        if( !doc.LoadFile( (const char*) fullFilePath.toUtf8().data() ) ) {
            QString msg(("Attempt to load styles from this file failed: ") );
            msg += fullFilePath;
            qDebug()<< msg ;
            continue;
        }

        QString msg(("Styles loading from ") );
        msg += fullFilePath;
        qDebug()<< msg ;

        TiXmlHandle hRoot( doc.RootElement() );

        QString root = QString::fromUtf8(doc.RootElement()->Value() );
        if( root != ("styles" ) ) {
            qDebug("    StyleManager: Expected XML Root <styles> not found.") ;
            continue;
        }

        TiXmlElement* styleElem = hRoot.FirstChild().Element();

        for( ; styleElem; styleElem = styleElem->NextSiblingElement() ) {

            if( QString::fromUtf8( styleElem->Value()) == ("style") ) {

                Style* style = new Style();
                styles.append( style );

                style->name = QString::fromUtf8( styleElem->Attribute( "name" ) );
                style->sysname = QString::fromUtf8( styleElem->Attribute( "sysname" ) );
                style->myConfigFileDir = fromPath;

                TiXmlElement* subNode = styleElem->FirstChild()->ToElement();

                for( ; subNode; subNode = subNode->NextSiblingElement() ) {
                    QString nodeType = QString::fromUtf8(subNode->Value() );

                    if( nodeType == ("description") ) {
                        style->description = QString::fromUtf8(subNode->GetText() );
                        continue;
                    }
                    if( nodeType == ("chart-status-icon") ) {
                        int w = 0;
                        subNode->QueryIntAttribute( "width", &w );
                        style->chartStatusIconWidth = w;
                        continue;
                    }
                    if( nodeType == ("chart-status-window") ) {
                        style->chartStatusWindowTransparent = QString::fromUtf8(subNode->Attribute( "transparent" )).toLower() == "true";
                        continue;
                    }
                    if( nodeType == ("embossed-indicators") ) {
                        style->embossFont = QString::fromUtf8(subNode->Attribute( "font" ));
                        subNode->QueryIntAttribute( "size", &(style->embossHeight) );
                        continue;
                    }
                    if( nodeType == ("graphics-file") ) {
                        style->graphicsFile = QString::fromUtf8(subNode->Attribute( "name" ) );
                        isOK = true; // If we got this far we are at least partially OK...
                        continue;
                    }
                    if( nodeType == ("active-route") ) {
                        TiXmlHandle handle( subNode );
                        TiXmlElement* tag = handle.Child( "font-color", 0 ).ToElement();
                        if( tag ) {
                            int r, g, b;
                            tag->QueryIntAttribute( "r", &r );
                            tag->QueryIntAttribute( "g", &g );
                            tag->QueryIntAttribute( "b", &b );
                            style->consoleFontColor = QColor( r, g, b );
                        }
                        tag = handle.Child( "text-background-location", 0 ).ToElement();
                        if( tag ) {
                            int x, y, w, h;
                            tag->QueryIntAttribute( "x", &x );
                            tag->QueryIntAttribute( "y", &y );
                            tag->QueryIntAttribute( "width", &w );
                            tag->QueryIntAttribute( "height", &h );
                            style->consoleTextBackgroundLoc = QPoint( x, y );
                            style->consoleTextBackgroundSize = QSize( w, h );
                        }
                        continue;
                    }
                    if( nodeType == ("icons") ) {
                        TiXmlElement* iconNode = subNode->FirstChild()->ToElement();

                        for( ; iconNode; iconNode = iconNode->NextSiblingElement() ) {
                            QString nodeType = QString::fromUtf8(iconNode->Value() );
                            if( nodeType == ("icon") ) {
                                Icon* icon = new Icon();
                                style->icons.append( icon );
                                icon->name = QString::fromUtf8( iconNode->Attribute( "name" ) );
                                style->iconIndex[icon->name] = style->icons.count() - 1;
                                TiXmlHandle handle( iconNode );
                                TiXmlElement* tag = handle.Child( "icon-location", 0 ).ToElement();
                                if( tag ) {
                                    int x, y;
                                    tag->QueryIntAttribute( "x", &x );
                                    tag->QueryIntAttribute( "y", &y );
                                    icon->iconLoc = QPoint( x, y );
                                }
                                tag = handle.Child( "size", 0 ).ToElement();
                                if( tag ) {
                                    int x, y;
                                    tag->QueryIntAttribute( "x", &x );
                                    tag->QueryIntAttribute( "y", &y );
                                    icon->size = QSize( x, y );
                                }
//                                qDebug()<<"load icon :"<<icon<<icon->name<<icon->iconLoc<<icon->size;
//                                int index = style->iconIndex[icon->name];
//                                Icon* test = (style->icons[index]);
//                                 qDebug()<<"load test :"<<test<<test->name<<test->iconLoc<<test->size;
                            }
                        }
                    }
                    if( nodeType == ("tools") ) {
                        TiXmlElement* toolNode = subNode->FirstChild()->ToElement();

                        for( ; toolNode; toolNode = toolNode->NextSiblingElement() ) {
                            QString nodeType = QString::fromUtf8(toolNode->Value() );

                            if( nodeType == ("horizontal") || nodeType == ("vertical") ) {
                                int orientation = 0;
                                if( nodeType == ("vertical") ) orientation = 1;

                                TiXmlElement* attrNode = toolNode->FirstChild()->ToElement();
                                for( ; attrNode; attrNode = attrNode->NextSiblingElement() ) {
                                    QString nodeType = QString::fromUtf8( attrNode->Value() );
                                    if( nodeType == ("separation") ) {
                                        attrNode->QueryIntAttribute( "distance",
                                                &style->toolSeparation[orientation] );
                                        continue;
                                    }
                                    if( nodeType == ("margin") ) {
                                        attrNode->QueryIntAttribute( "top",
                                                &style->toolMarginTop[orientation] );
                                        attrNode->QueryIntAttribute( "right",
                                                &style->toolMarginRight[orientation] );
                                        attrNode->QueryIntAttribute( "bottom",
                                                &style->toolMarginBottom[orientation] );
                                        attrNode->QueryIntAttribute( "left",
                                                &style->toolMarginLeft[orientation] );
                                        QString invis = QString::fromUtf8( attrNode->Attribute( "invisible" ) );
                                        style->marginsInvisible = ( invis.toLower() == ("true") );
                                        continue;;
                                    }
                                    if( nodeType == ("toggled-location") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->toggledBGlocation[orientation] = QPoint( x, y );
                                        x = 0;
                                        y = 0;
                                        attrNode->QueryIntAttribute( "width", &x );
                                        attrNode->QueryIntAttribute( "height", &y );
                                        style->toggledBGSize[orientation] = QSize( x, y );
                                        continue;
                                    }
                                    if( nodeType == ("toolbar-start") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->toolbarStartLoc[orientation] = QPoint( x, y );
                                        x = 0;
                                        y = 0;
                                        attrNode->QueryIntAttribute( "width", &x );
                                        attrNode->QueryIntAttribute( "height", &y );
                                        style->toolbarStartSize[orientation] = QSize( x, y );
                                        continue;
                                    }
                                    if( nodeType == ("toolbar-end") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->toolbarEndLoc[orientation] = QPoint( x, y );
                                        x = 0;
                                        y = 0;
                                        attrNode->QueryIntAttribute( "width", &x );
                                        attrNode->QueryIntAttribute( "height", &y );
                                        style->toolbarEndSize[orientation] = QSize( x, y );
                                        continue;
                                    }
                                    if( nodeType == ("toolbar-corners") ) {
                                        int r;
                                        attrNode->QueryIntAttribute( "radius", &r );
                                        style->cornerRadius[orientation] = r;
                                        continue;
                                    }
                                    if( nodeType == ("background-location") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->normalBGlocation[orientation] = QPoint( x, y );
                                        style->HasBackground( true );
                                        continue;
                                    }
                                    if( nodeType == ("active-location") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->activeBGlocation[orientation] = QPoint( x, y );
                                        continue;
                                    }
                                    if( nodeType == ("size") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->toolSize[orientation] = QSize( x, y );
                                        continue;
                                    }
                                    if( nodeType == ("icon-offset") ) {
                                        int x, y;
                                        attrNode->QueryIntAttribute( "x", &x );
                                        attrNode->QueryIntAttribute( "y", &y );
                                        style->verticalIconOffset = QSize( x, y );
                                        continue;
                                    }
                                }
                                continue;
                            }
                            if( nodeType == ("compass") ) {

                                TiXmlElement* attrNode = toolNode->FirstChild()->ToElement();
                                for( ; attrNode; attrNode = attrNode->NextSiblingElement() ) {
                                    QString nodeType = QString::fromUtf8(attrNode->Value() );
                                    if( nodeType == ("margin") ) {
                                        attrNode->QueryIntAttribute( "top",
                                                                     &style->compassMarginTop );
                                        attrNode->QueryIntAttribute( "right",
                                                                     &style->compassMarginRight );
                                        attrNode->QueryIntAttribute( "bottom",
                                                                     &style->compassMarginBottom );
                                        attrNode->QueryIntAttribute( "left",
                                                                     &style->compassMarginLeft );
                                        continue;
                                    }
                                    if( nodeType == ("compass-corners") ) {
                                            int r;
                                            attrNode->QueryIntAttribute( "radius", &r );
                                            style->compasscornerRadius = r;
                                            continue;
                                        }
                                    if( nodeType == ("offset") ) {
                                        attrNode->QueryIntAttribute( "x",
                                                                     &style->compassXoffset );
                                        attrNode->QueryIntAttribute( "y",
                                                                     &style->compassYoffset );
                                        continue;
                                    }
                                }
                             }

                             if( nodeType == ("tool") ) {
                                Tool* tool = new Tool();
                                style->tools.append( tool );
                                tool->name = QString::fromUtf8( toolNode->Attribute( "name" ) );
                                style->toolIndex[tool->name] = style->tools.count() - 1;
                                TiXmlHandle toolHandle( toolNode );
                                TiXmlElement* toolTag =
                                        toolHandle.Child( "icon-location", 0 ).ToElement();
                                if( toolTag ) {
                                    int x, y;
                                    toolTag->QueryIntAttribute( "x", &x );
                                    toolTag->QueryIntAttribute( "y", &y );
                                    tool->iconLoc = QPoint( x, y );
                                }
                                toolTag = toolHandle.Child( "rollover-location", 0 ).ToElement();
                                if( toolTag ) {
                                    int x, y;
                                    toolTag->QueryIntAttribute( "x", &x );
                                    toolTag->QueryIntAttribute( "y", &y );
                                    tool->rolloverLoc = QPoint( x, y );
                                }
                                toolTag = toolHandle.Child( "disabled-location", 0 ).ToElement();
                                if( toolTag ) {
                                    int x, y;
                                    toolTag->QueryIntAttribute( "x", &x );
                                    toolTag->QueryIntAttribute( "y", &y );
                                    tool->disabledLoc = QPoint( x, y );
                                }
                                toolTag = toolHandle.Child( "size", 0 ).ToElement();
                                if( toolTag ) {
                                    int x, y;
                                    toolTag->QueryIntAttribute( "x", &x );
                                    toolTag->QueryIntAttribute( "y", &y );
                                    tool->customSize = QSize( x, y );
                                }
                                continue;
                            }
                        }
                        continue;
                    }
                }
            }
        }
    }

//    foreach (Style* style, styles) {
//        qDebug()<<"style name:"<<style->name;
//        foreach (Icon* icon, style->icons) {
//            qDebug()<<icon<<icon->name<<icon->iconLoc<<icon->size;
//        }
//    }
}

void StyleManager::SetStyle(QString name)
{
    Style* style = NULL;
    bool ok = true;
    if( currentStyle ) currentStyle->Unload();
    else ok = false;

    bool selectFirst = false;

    // Verify the named style exists
    //  If not, just use the "first" style
    bool bstyleFound = false;

    for( unsigned int i = 0; i < styles.count(); i++ ) {
        style = (Style*) ( styles.at( i ) );
        if( style->name == name ) {
            bstyleFound = true;
            break;
        }
    }

    if( (name.length() == 0) || !bstyleFound )
        selectFirst = true;

    for( unsigned int i = 0; i < styles.count(); i++ ) {
        style = (Style*) ( styles[i] );
        if( style->name == name || selectFirst ) {
            if( style->graphics ) {
                currentStyle = style;
                ok = true;
                break;
            }

            QString fullFilePath = style->myConfigFileDir + zchxFuncUtil::separator() + style->graphicsFile;

            if( !QFile::exists( fullFilePath ) ) {
                QString msg(("Styles Graphics File not found: ") );
                msg += fullFilePath;
                qDebug()<< msg ;
                ok = false;
                if( selectFirst ) continue;
                break;
            }

            QImage img; // Only image does PNG LoadFile properly on GTK.

            if( !img.load(fullFilePath, "PNG" ) ) {
                QString msg(("Styles Graphics File failed to load: ") );
                msg += fullFilePath;
                qDebug()<< msg ;
                ok = false;
                break;
            }
            style->graphics = new wxBitmap( img );
            currentStyle = style;
            ok = true;
            break;
        }
    }

    if( !ok || !currentStyle->graphics ) {
        QString msg("The requested style was not found: ");
        msg += name;
        qDebug()<<msg;
        return;
    }

    if(currentStyle) {
        if( (currentStyle->consoleTextBackgroundSize.width()) && (currentStyle->consoleTextBackgroundSize.height())) {
            currentStyle->consoleTextBackground = currentStyle->graphics->GetSubBitmap(
                QRect( currentStyle->consoleTextBackgroundLoc, currentStyle->consoleTextBackgroundSize ) );
        }
    }

    if(currentStyle)
        nextInvocationStyle = currentStyle->name;

//    if(currentStyle)
//    {
//        qDebug()<<"style name:"<<currentStyle->name;
//        foreach (Icon* icon, currentStyle->icons) {
//            qDebug()<<icon<<icon->name<<icon->iconLoc<<icon->size;
//        }
//    }
    
    return;
}

Style* StyleManager::GetCurrentStyle()
{
    return currentStyle;
}
