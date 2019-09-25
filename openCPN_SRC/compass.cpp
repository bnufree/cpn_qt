/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 ***************************************************************************
 *
 */
#include "zchxconfig.h"
#include "compass.h"
#include "chcanv.h"
#include "styles.h"

#include "dychart.h"


extern bool bGPSValid;
bool g_bSatValid;
int g_SatsInView;
extern zchxMapMainWindow *gFrame;

extern QColor GetGlobalColor(const QString& str);

ocpnCompass::ocpnCompass( ChartCanvas *parent, bool bShowGPS)
{
    m_parent = parent;
    m_bshowGPS = bShowGPS;
    
     ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
     _img_compass = style->GetIcon(("CompassRose") );
     _img_gpsRed = style->GetIcon(("gpsRed") );

    m_rose_angle = -999;  // force a refresh when first used

     m_rect = QRect(style->GetCompassXOffset(), style->GetCompassYOffset(),
             _img_compass.GetWidth() + _img_gpsRed.GetWidth() + style->GetCompassLeftMargin() * 2
                     + style->GetToolSeparation(),
                     _img_compass.GetHeight() + style->GetCompassTopMargin() + style->GetCompassBottomMargin() );
    
#ifdef ocpnUSE_GL
    texobj = 0;
#endif

    m_scale = 1.0;
    m_cs = GLOBAL_COLOR_SCHEME_RGB;
    
}

ocpnCompass::~ocpnCompass()
{
#ifdef ocpnUSE_GL
    if(texobj){
        glDeleteTextures(1, &texobj);
        texobj = 0;
    }
#endif
}

void ocpnCompass::Paint( ocpnDC& dc )
{
    if(m_shown && m_StatBmp.isOK()){
# if defined(ocpnUSE_GLES) || defined(ocpnUSE_GL)  // GLES does not do ocpnDC::DrawBitmap(), so use texture
        if(texobj){
            glBindTexture( GL_TEXTURE_2D, texobj );
            glEnable( GL_TEXTURE_2D );
            
            glBegin( GL_QUADS );
            
            glTexCoord2f( 0, 0 );  glVertex2i( m_rect.x(), m_rect.y() );
            glTexCoord2f( 1, 0 );  glVertex2i( m_rect.x() + m_rect.width(), m_rect.y() );
            glTexCoord2f( 1, 1 );  glVertex2i( m_rect.x() + m_rect.width(), m_rect.y() + m_rect.height() );
            glTexCoord2f( 0, 1 );  glVertex2i( m_rect.x(), m_rect.y() + m_rect.height() );
            
            glEnd();
            glDisable( GL_TEXTURE_2D );
            
        }
        else {
         dc.DrawBitmap(m_StatBmp, m_rect.x(), m_rect.y(), true );
        }
    
#else
    dc.DrawBitmap( m_StatBmp, m_rect.x, m_rect.y, true );
#endif        
    }
        
}

bool ocpnCompass::MouseEvent( QMouseEvent& event )
{
    if(!m_shown || !m_rect.contains(event.pos()))
        return false;

    return true;
}

void ocpnCompass::SetColorScheme( ColorScheme cs )
{
    UpdateStatus( true );
    m_cs = cs;
}

void ocpnCompass::UpdateStatus( bool bnew )
{
    if( bnew ){
        m_lastgpsIconName.clear();        // force an update to occur
        
        //  We clear the texture so that any onPaint method will not use a stale texture
#ifdef ocpnUSE_GLES  
        if(1){
             if(texobj){
                glDeleteTextures(1, &texobj);
                texobj = 0;
             }
        }
#endif
    }
                

    CreateBmp( bnew );
}

void ocpnCompass::SetScaleFactor( float factor)
{
    ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();
    
    if(factor > 0.1)
        m_scale = factor;
    else
        m_scale = 1.0;
    
    //  Precalculate the background sizes to get m_rect width/height
    wxBitmap compassBg, gpsBg;
    int orient = style->GetOrientation();
    style->SetOrientation( ocpnStyle::Style::Hortizontal );
    if( style->HasBackground() ) {
        compassBg = style->GetNormalBG();
        style->DrawToolbarLineStart( compassBg );
        compassBg = style->SetBitmapBrightness( compassBg, m_cs );
        gpsBg = style->GetNormalBG();
        style->DrawToolbarLineEnd( gpsBg );
        gpsBg = style->SetBitmapBrightness( gpsBg, m_cs );
    }

    if(fabs(m_scale-1.0) > 0.1){
        QImage bg_img = compassBg.ConvertToImage();
        bg_img = bg_img.scaled(compassBg.GetWidth() * m_scale, compassBg.GetHeight() *m_scale);
        compassBg = wxBitmap( bg_img );
            
        bg_img = gpsBg.ConvertToImage();
        bg_img = bg_img.scaled(gpsBg.GetWidth() * m_scale, gpsBg.GetHeight() *m_scale);
        gpsBg = wxBitmap( bg_img );
     }

     int width = compassBg.GetWidth() + gpsBg.GetWidth() + style->GetCompassLeftMargin();
     if( !style->marginsInvisible ) 
         width += style->GetCompassLeftMargin() + style->GetToolSeparation();
     
     m_rect = QRect(style->GetCompassXOffset(), style->GetCompassYOffset(),
                    width,
                     compassBg.GetHeight() + style->GetCompassTopMargin() + style->GetCompassBottomMargin());

}


void ocpnCompass::CreateBmp( bool newColorScheme )
{
    if(!m_shown)
        return;

    QString gpsIconName;
    ocpnStyle::Style* style = StyleMgrIns->GetCurrentStyle();

    // In order to draw a horizontal compass window when the toolbar is vertical, we
    // need to save away the sizes and backgrounds for the two icons.

    static wxBitmap compassBg, gpsBg;
    static QSize toolsize;
    static int topmargin, leftmargin, radius;


    if( ! compassBg.isOK() || newColorScheme ) {
        int orient = style->GetOrientation();
        style->SetOrientation( ocpnStyle::Style::Hortizontal );
        if( style->HasBackground() ) {
            compassBg = style->GetNormalBG();
            style->DrawToolbarLineStart( compassBg );
            compassBg = style->SetBitmapBrightness( compassBg, m_cs );
            gpsBg = style->GetNormalBG();
            style->DrawToolbarLineEnd( gpsBg );
            gpsBg = style->SetBitmapBrightness( gpsBg, m_cs );
        }

        if(fabs(m_scale-1.0) > 0.1){
            QImage bg_img = compassBg.ConvertToImage();
            bg_img = bg_img.scaled(compassBg.GetWidth() * m_scale, compassBg.GetHeight() *m_scale);
            compassBg = wxBitmap( bg_img );
            
            bg_img = gpsBg.ConvertToImage();
            bg_img = bg_img.scaled(gpsBg.GetWidth() * m_scale, gpsBg.GetHeight() *m_scale);
            gpsBg = wxBitmap( bg_img );
        }
    
        leftmargin = style->GetCompassLeftMargin();
        topmargin = style->GetCompassTopMargin();
        radius = style->GetCompassCornerRadius();

        if( orient == ocpnStyle::Style::Vertical )
            style->SetOrientation( ocpnStyle::Style::Vertical );
    }

    bool b_need_refresh = false;

    if( bGPSValid ) {
        if( g_bSatValid ) {
            gpsIconName = ("gps3Bar");
            if( g_SatsInView <= 8 ) gpsIconName = ("gps2Bar");
            if( g_SatsInView <= 4 ) gpsIconName = ("gps1Bar");
            if( g_SatsInView < 0 ) gpsIconName = ("gpsGry");

        } else
            gpsIconName = ("gpsGrn");
    } else
        gpsIconName = ("gpsRed");

    if( m_lastgpsIconName != gpsIconName ) b_need_refresh = true;

    double rose_angle = -999.;

    if( ( fabs( m_parent->GetVPRotation() ) > .01 ) || ( fabs( m_parent->GetVPSkew() ) > .01 ) ) {
        rose_angle = -m_parent->GetVPRotation();
     } else
         rose_angle = 0.;

    if( fabs( m_rose_angle - rose_angle ) > .1 ) 
        b_need_refresh = true;

    if( !b_need_refresh )
        return;

    int width = compassBg.GetWidth();
    if(m_bshowGPS)
        width += gpsBg.GetWidth() + leftmargin;
    
    if( !style->marginsInvisible ) 
        width += leftmargin + style->GetToolSeparation();
        
    m_StatBmp.Create( width, compassBg.GetHeight() + topmargin + style->GetCompassBottomMargin() );

    m_rect.setSize(QSize(m_StatBmp.GetWidth(),m_StatBmp.GetHeight()));
    
    if( !m_StatBmp.isOK() )
        return;

    m_MaskBmp = wxBitmap( m_StatBmp.GetWidth(), m_StatBmp.GetHeight() );
    if( style->marginsInvisible ) {
        QPainter sdc;
        sdc.begin(m_MaskBmp.GetHandle() );
        sdc.setBackground(Qt::white);
        sdc.eraseRect(0, 0, sdc.device()->width(), sdc.device()->height());
        sdc.setBrush(Qt::black );
        sdc.setPen(QPen(Qt::black) );
        QSize maskSize = QSize(m_MaskBmp.GetWidth() - leftmargin,
                                 m_MaskBmp.GetHeight() - (2 * topmargin));
        sdc.drawRoundedRect( QRect(QPoint( leftmargin, topmargin ), maskSize), radius, radius );
        sdc.end();
    } else if(radius) {
        QPainter sdc;
        sdc.begin( m_MaskBmp.GetHandle() );
        sdc.setBackground( Qt::white );
        sdc.eraseRect(0, 0, sdc.device()->width(), sdc.device()->height());
        sdc.setBrush( Qt::black );
        sdc.setPen( Qt::black );
        sdc.drawRoundedRect(QRect(0, 0, m_MaskBmp.GetWidth(), m_MaskBmp.GetHeight()), radius, radius );
        sdc.end();
    }
    m_StatBmp.SetMask(new wxMask(m_MaskBmp, Qt::white));

    QPainter mdc;
    mdc.begin( m_StatBmp.GetHandle() );
    mdc.setBackground( QBrush( GetGlobalColor( ("COMP1") ), Qt::SolidPattern ) );
    mdc.eraseRect(0, 0, mdc.device()->width(), mdc.device()->height());

    mdc.setPen( QPen( GetGlobalColor( ("UITX1") ), 1 ) );
    mdc.setBrush( QBrush( GetGlobalColor( ("UITX1") ), Qt::NoBrush ) );
    
    if( !style->marginsInvisible )
        mdc.drawRoundedRect( QRect(0, 0, m_StatBmp.GetWidth(), m_StatBmp.GetHeight()),radius, radius );

    zchxPoint offset(leftmargin, topmargin);

    //    Build Compass Rose, rotated...
    wxBitmap BMPRose;
    zchxPoint after_rotate;

    int cwidth = style->GetToolSize().width() * m_scale;
    int cheight = style->GetToolSize().height() * m_scale;
    cheight = fmin(cheight, compassBg.GetHeight());
    cwidth = fmin( cwidth, cheight );
    cheight = cwidth;
    
    if( m_parent->m_bCourseUp )
        BMPRose = style->GetIcon( ("CompassRose"), cwidth, cheight );
    else
        BMPRose = style->GetIcon( ("CompassRoseBlue"), cwidth, cheight );
    if( ( fabs( m_parent->GetVPRotation() ) > .01 ) || ( fabs( m_parent->GetVPSkew() ) > .01 ) ) {
         QImage rose_img = BMPRose.ConvertToImage();
         zchxPoint rot_ctr( cwidth / 2, cheight / 2  );
//         QImage rot_image = rose_img.Rotate( rose_angle, rot_ctr, true, &after_rotate );
//         BMPRose = wxBitmap( rot_image ).GetSubBitmap( QRect( -after_rotate.x, -after_rotate.y, cwidth, cheight ));
     }

    wxBitmap iconBm;

    if( style->HasBackground() ) {
        iconBm = MergeBitmaps( compassBg, BMPRose, QSize( 0, 0 ) );
    } else {
        iconBm = BMPRose;
    }
    
    iconBm = ConvertTo24Bit( QColor(0,0,0), iconBm);
        
    mdc.drawPixmap(offset.toPoint(), *(iconBm.GetHandle()) );
    offset.x += iconBm.GetWidth();
    offset.x += style->GetToolSeparation();

    m_rose_angle = rose_angle;

    if(m_bshowGPS){
        //  GPS Icon
        int twidth = style->GetToolSize().width() * m_scale;
        int theight = style->GetToolSize().height() * m_scale;
        theight = fmin(cheight, compassBg.GetHeight());
        int swidth = fmax( twidth, theight );
        int sheight = fmin( twidth, theight );
        
        //  Sometimes, the SVG renderer gets the size wrong due to some internal rounding error.
        //  If so found, it seems to work OK by just reducing the requested size by one pixel....
        wxBitmap gicon = style->GetIcon( gpsIconName, swidth, sheight );
        if( gicon.GetHeight() != sheight )
            gicon = style->GetIcon( gpsIconName, swidth-1, sheight-1, true );

        if( style->HasBackground() ) {
            iconBm = MergeBitmaps( gpsBg, gicon, QSize( 0, 0 ) );
        } else {
            iconBm = gicon;
        }
        
        iconBm = ConvertTo24Bit( QColor(0,0,0), iconBm);
        mdc.drawPixmap(offset.toPoint(), *(iconBm.GetHandle()) );
        m_lastgpsIconName = gpsIconName;
    }

    mdc.end();


#if defined(ocpnUSE_GLES)   // GLES does not do ocpnDC::DrawBitmap(), so use texture
    if(1){
        wxImage image = m_StatBmp.ConvertToImage(); 
        unsigned char *imgdata = image.GetData();
        unsigned char *imgalpha = image.GetAlpha();
        int tex_w = image.GetWidth();
        int tex_h = image.GetHeight();
        
        GLuint format = GL_RGBA;
        GLuint internalformat = format;
        int stride = 4;
        
        if(imgdata){
            unsigned char *teximage = (unsigned char *) malloc( stride * tex_w * tex_h );
        
            for( int j = 0; j < tex_w*tex_h; j++ ){
                for( int k = 0; k < 3; k++ )
                    teximage[j * stride + k] = imgdata[3*j + k];
                teximage[j * stride + 3] = imgalpha ? imgalpha[j] : 255;      // alpha
            }
                
            if(texobj){
                glDeleteTextures(1, &texobj);
                texobj = 0;
            }
                
            glGenTextures( 1, &texobj );
            glBindTexture( GL_TEXTURE_2D, texobj );
            
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST/*GL_LINEAR*/ );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            
            glTexImage2D( GL_TEXTURE_2D, 0, internalformat, tex_w, tex_h, 0,
                        format, GL_UNSIGNED_BYTE, teximage );
                            
            free(teximage);
        }
   }
#endif
       
}
