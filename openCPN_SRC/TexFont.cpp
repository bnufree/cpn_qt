/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  texture OpenGL text rendering built from QFont
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 Sean D'Epagnier                                    *
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

#ifdef ocpnUSE_GL

#include <GL/gl.h>
#include <GL/glu.h>

#include "TexFont.h"
#include <QFontMetrics>
#include "bitmap.h"
#include <QPainter>
#include <QDebug>
#include <QDateTime>

typedef struct {
    QFont  *key;
    TexFont cache;
} TexFontCache;

#define TXF_CACHE 8
static TexFontCache s_txf[TXF_CACHE];

TexFont *GetTexFont(QFont *pFont)
{
    // rebuild font if needed
    TexFont *f_cache;
    unsigned int i;
    for (i = 0; i < TXF_CACHE && s_txf[i].key != nullptr; i++)
    {
        if (s_txf[i].key == pFont) {
            return &s_txf[i].cache;
        }
    }
    if (i == TXF_CACHE) {
        i = rand() & (TXF_CACHE -1);
    }
    s_txf[i].key = pFont;
    f_cache = &s_txf[i].cache;
    f_cache->Build(*pFont);
    return f_cache;
}

TexFont::TexFont( )
{
    texobj = 0;
    m_blur = false;
    m_built = false;
}

TexFont::~TexFont( )
{
    Delete( );
}


void TexFont::Build( QFont &font, bool blur )
{
    /* avoid rebuilding if the parameters are the same */
    if(m_built && (font == m_font) && (blur == m_blur))
        return;
    
    m_font = font;
    m_blur = blur;

    m_maxglyphw = 0;
    m_maxglyphh = 0;

    QFontMetrics mcs(m_font);
    for( int i = MIN_GLYPH; i < MAX_GLYPH; i++ ) {
        int gw, gh;
        QString text;
        if(i == DEGREE_GLYPH)
            text.sprintf("%c", 0x00B0); //_T("°");
        else
            text.sprintf("%c", i);
        gw = mcs.width(text);
        gh = mcs.height();

        tgi[i].width = gw;
        tgi[i].height = gh;

        tgi[i].advance = gw;
        
        
        m_maxglyphw = qMax(tgi[i].width,  m_maxglyphw);
        m_maxglyphh = qMax(tgi[i].height, m_maxglyphh);
    }

    /* add extra pixel to give a border between rows of characters
       without this, in some cases a faint line can be see on the edge
       from the character above */
    m_maxglyphh++;

    int w = COLS_GLYPHS * m_maxglyphw;
    int h = ROWS_GLYPHS * m_maxglyphh;

    Q_ASSERT(w < 2048 && h < 2048);

    /* make power of 2 */
    for(tex_w = 1; tex_w < w; tex_w *= 2);
    for(tex_h = 1; tex_h < h; tex_h *= 2);

    wxBitmap tbmp(tex_w, tex_h);
    QPainter dc;
    if(!dc.begin(tbmp.GetHandle())) return;
    dc.setFont( font );
    dc.setBackground( QBrush( QColor( 0, 0, 0 ) ) );
    dc.eraseRect(0, 0, dc.device()->width(), dc.device()->height());
        
    /* draw the text white */
    QPen pen = dc.pen();
    pen.setColor(QColor( 255, 255, 255 ) );
    dc.setPen(pen);
    int row = 0, col = 0;
    for( int i = MIN_GLYPH; i < MAX_GLYPH; i++ )
    {
        if(col == COLS_GLYPHS) {
            col = 0;
            row++;
        }

        tgi[i].x = col * m_maxglyphw;
        tgi[i].y = row * m_maxglyphh;

        QString text;
        if(i == DEGREE_GLYPH)
            text.sprintf("%c", 0x00B0); //_T("°");
        else
            text.sprintf("%c", i);

        dc.drawText(QRect(tgi[i].x, tgi[i].y, tgi[i].width, tgi[i].height), Qt::AlignLeft, text );
        col++;
    }
    dc.end();
    
    QImage image = tbmp.ConvertToImage();
    if(image.format() != QImage::Format::Format_RGB888)
    {
        image = image.convertToFormat(QImage::Format::Format_RGB888);
    }
#if 0
    image.save("font.png", "PNG");
#endif
    GLuint format, internalformat;
    int stride;

    format = GL_ALPHA;
    internalformat = format;
    stride = 1;

//    if( m_blur ) image = image.Blur(1);

    unsigned char *imgdata = image.bits();
    if(imgdata)
    {
        QByteArray teximage;
        teximage.reserve(stride * tex_w * tex_h);
        for(int i = 0; i <tex_w * tex_h; i++ )
        {
            for( int k = 0; k < stride; k++ )
            {
                teximage[stride + k + i] = imgdata[3*i];
            }
        }

        Delete();

        glGenTextures( 1, &texobj );
        glBindTexture( GL_TEXTURE_2D, texobj );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST/*GL_LINEAR*/ );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glTexImage2D( GL_TEXTURE_2D, 0, internalformat, tex_w, tex_h, 0, format, GL_UNSIGNED_BYTE, teximage.data() );
    }
    
    m_built = true;
}

void TexFont::Delete( )
{
    if (texobj) {
        glDeleteTextures(1, &texobj);
        texobj = 0;
    }
    m_built = false;
}

void TexFont::GetTextExtent(const char *string, int *width, int *height)
{
    int w=0, h=0;

    for(int i = 0; string[i]; i++ ) {
        unsigned char c = string[i];
        if(c == '\n') {
            h += tgi[(int)'A'].height;
            continue;
        }
        if(c == 0xc2 && (unsigned char)string[i+1] == 0xb0) {
            c = DEGREE_GLYPH;
            i++;
        }
        if( c < MIN_GLYPH || c >= MAX_GLYPH)
            continue;

        TexGlyphInfo &tgisi = tgi[c];
        w += tgisi.advance;
        if(tgisi.height > h)
            h = tgisi.height;
    }
    if(width) *width = w;
    if(height) *height = h;
}

void TexFont::GetTextExtent(const QString &string, int *width, int *height)
{
    GetTextExtent((const char*)string.toUtf8().data(), width, height);
}

void TexFont::RenderGlyph( int c )
{
    if( c < MIN_GLYPH || c >= MAX_GLYPH)
        return;

    TexGlyphInfo &tgic = tgi[c];

    int x = tgic.x, y = tgic.y;
    float w = m_maxglyphw, h = m_maxglyphh;
    float tx1 = (float)x / (float)tex_w;
    float tx2 = (float)(x + w) / (float)tex_w;
    float ty1 = (float)y / (float)tex_h;
    float ty2 = (float)(y + h) / (float)tex_h;

    glBegin( GL_QUADS );

    glTexCoord2f( tx1, ty1 );  glVertex2i( 0, 0 );
    glTexCoord2f( tx2, ty1 );  glVertex2i( w, 0 );
    glTexCoord2f( tx2, ty2 );  glVertex2i( w, h );
    glTexCoord2f( tx1, ty2 );  glVertex2i( 0, h );

    glEnd();
    glTranslatef( tgic.advance, 0.0, 0.0 );
}

void TexFont::RenderString( const char *string, int x, int y )
{
    glPushMatrix();
    glTranslatef(x, y, 0);

    glPushMatrix();
    glBindTexture( GL_TEXTURE_2D, texobj);

    for( int i = 0; string[i]; i++ ) {
        if(string[i] == '\n') {
            glPopMatrix();
            glTranslatef(0, tgi[(int)'A'].height, 0);
            glPushMatrix();
            continue;
        }
        /* degree symbol */
        if((unsigned char)string[i] == 0xc2 &&
           (unsigned char)string[i+1] == 0xb0) {
            RenderGlyph( DEGREE_GLYPH );
            i++;
            continue;
        }
        RenderGlyph( string[i] );
    }

    glPopMatrix();
    glPopMatrix();
}

void TexFont::RenderString( const QString &string, int x, int y )
{
    RenderString((const char*)string.toUtf8().data(), x, y);
}

#endif     //#ifdef ocpnUSE_GL
