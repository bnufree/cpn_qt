/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/bitmap.cpp
// Author:      Peter Most, Javier Torres, Mariano Reingart, Sean D'Epagnier
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include <QtGui/QPixmap>
#include <QtGui/QBitmap>
#include <QtWidgets/QLabel>
#include "qt/"


static wxImage ConvertImage( QImage qtImage )
{
    bool hasAlpha = qtImage.hasAlphaChannel();
    
    int numPixels = qtImage.height() * qtImage.width();

    //Convert to ARGB32 for scanLine
    qtImage = qtImage.convertToFormat(QImage::Format_ARGB32);
    
    unsigned char *data = (unsigned char *)malloc(sizeof(char) * 3 * numPixels);
    unsigned char *startData = data;
    
    unsigned char *alpha = NULL;
    if (hasAlpha)
        alpha = (unsigned char *)malloc(sizeof(char) * numPixels);

    unsigned char *startAlpha = alpha;
    
    for (int y = 0; y < qtImage.height(); y++)
    {
        QRgb *line = (QRgb*)qtImage.scanLine(y);
        
        for (int x = 0; x < qtImage.width(); x++)
        {
            QRgb colour = line[x];
            
            data[0] = qRed(colour);
            data[1] = qGreen(colour);
            data[2] = qBlue(colour);
            
            if (hasAlpha)
            {
                alpha[0] = qAlpha(colour);
                alpha++;
            }
            data += 3;
        }
    }
    if (hasAlpha)
        return wxImage(wxQtConvertSize(qtImage.size()), startData, startAlpha);
    else
        return wxImage(wxQtConvertSize(qtImage.size()), startData);
}

static QImage ConvertImage( const wxImage &image )
{
    bool hasAlpha = image.HasAlpha();
    bool hasMask = image.HasMask();
    QImage qtImage( wxQtConvertSize( image.GetSize() ),
                   ( (hasAlpha || hasMask ) ? QImage::Format_ARGB32 : QImage::Format_RGB32 ) );
    
    unsigned char *data = image.GetData();
    unsigned char *alpha = hasAlpha ? image.GetAlpha() : NULL;
    QRgb colour;

    QRgb maskedColour;
    if ( hasMask )
    {
        unsigned char r, g, b;
        image.GetOrFindMaskColour( &r, &g, &b );
        maskedColour = ( r << 16 ) + ( g << 8 ) + b;
    }
    
    for (int y = 0; y < image.GetHeight(); y++)
    {
        for (int x = 0; x < image.GetWidth(); x++)
        {
            if (hasAlpha)
            {
                colour = alpha[0] << 24;
                alpha++;
            }
            else
                colour = 0;
            
            colour += (data[0] << 16) + (data[1] << 8) + data[2];

            if ( hasMask && colour != maskedColour )
                colour += 0xFF000000; // 255 << 24
            
            qtImage.setPixel(x, y, colour);
            
            data += 3;
        }
    }
    return qtImage;
}


wxBitmap::wxBitmap()
{
    m_mask = NULL;
    m_qtPixmap = NULL;
}

wxBitmap::~wxBitmap()
{
    if(m_mask) delete m_mask;
    if(m_qtPixmap) delete m_qtPixmap;
}


wxBitmap::wxBitmap(QPixmap pix)
{
    m_qtPixmap = new QPixmap(pix);
    m_mask = NULL;
}

wxBitmap::wxBitmap(const wxBitmap& bmp)
{
    m_mask = NULL;
    m_qtPixmap = NULL;
    if(bmp.m_qtPixmap)      m_qtPixmap = new QPixmap(*(bmp.m_qtPixmap));
    if(bmp.m_mask)          m_mask = new wxMask(*(bmp.m_mask));
}

wxBitmap::wxBitmap(const char bits[], int width, int height, int depth )
{
    Q_ASSERT(depth == 1);
    m_mask = NULL;
    m_qtPixmap = NULL;
    if (width > 0 && height > 0 && depth == 1)
    {
        m_qtPixmap = new QBitmap(QBitmap::fromData(QSize(width, height), (const uchar*)bits));
    }
}

wxBitmap::wxBitmap(int width, int height, int depth)
{
    Create(width, height, depth);
}

wxBitmap::wxBitmap(const QSize& sz, int depth )
{
    Create(sz, depth);
}

// Create a wxBitmap from xpm data
wxBitmap::wxBitmap(const char* const* bits)
{
    m_mask = NULL;
    m_qtPixmap = new QPixmap( bits );
}

wxBitmap::wxBitmap(const QString &filename, wxBitmapType type )
{
    m_mask = NULL;
    m_qtPixmap = NULL;
    LoadFile(filename);
}

wxBitmap::wxBitmap(const wxImage& image, int depth, double WXUNUSED(scale) )
{
    m_mask = NULL;
    Qt::ImageConversionFlags flags = 0;
    if (depth == 1)  flags = Qt::MonoOnly;
    m_qtPixmap = new QPixmap(QPixmap::fromImage(ConvertImage(image), flags));
}

wxBitmap::wxBitmap(const QCursor& cursor)
{
    // note that pixmap could be invalid if is not a pixmap cursor
    m_mask = NULL;;
    QPixmap pix = cursor.pixmap();
    m_qtPixmap = new QPixmap(pix);
}

bool wxBitmap::Create(int width, int height, int depth )
{
    Q_UNUSED(depth)
    m_mask = NULL;
    m_qtPixmap = QBitmap( width, height );
    return true;
}

bool wxBitmap::Create(const QSize& sz, int depth )
{
    return Create(sz.width(), sz.height(), depth);
}

int wxBitmap::GetHeight() const
{
    Q_ASSERT(m_qtPixmap != NULL);
    return m_qtPixmap->height();
}

int wxBitmap::GetWidth() const
{
    Q_ASSERT(m_qtPixmap != NULL);
    return m_qtPixmap->width();
}

int wxBitmap::GetDepth() const
{
    Q_ASSERT(m_qtPixmap != NULL);
    return m_qtPixmap->depth();
}

wxImage wxBitmap::ConvertToImage() const
{
    QPixmap pixmap(M_PIXDATA);
    if ( M_MASK && M_MASK->GetHandle() )
        pixmap.setMask(*M_MASK->GetHandle());
    return ConvertImage(pixmap.toImage());
}

#endif // wxUSE_IMAGE

wxMask *wxBitmap::GetMask() const
{
    return m_mask;
}

void wxBitmap::SetMask(wxMask *mask)
{
    if(m_mask) delete m_mask;
    m_mask = mask;
}

wxBitmap wxBitmap::GetSubBitmap(const QRect& rect) const
{
    Q_ASSERT(m_qtPixmap != NULL);
    return wxBitmap(m_qtPixmap->copy(rect));
}


bool wxBitmap::SaveFile(const QString &name, const QString& type ) const
{
    Q_ASSERT(m_qtPixmap != NULL);
    return m_qtPixmap->save(name, type);
}

bool wxBitmap::LoadFile(const QString &name)
{
    if(!m_qtPixmap) m_qtPixmap = new QPixmap;
    m_qtPixmap->load(name);
}


#if wxUSE_PALETTE
wxPalette *wxBitmap::GetPalette() const
{
    wxMISSING_IMPLEMENTATION( "wxBitmap palettes" );
    return 0;
}

void wxBitmap::SetPalette(const wxPalette& WXUNUSED(palette))
{
    wxMISSING_IMPLEMENTATION( "wxBitmap palettes" );
}

#endif // wxUSE_PALETTE

// copies the contents and mask of the given (colour) icon to the bitmap
bool wxBitmap::CopyFromIcon(const wxIcon& icon)
{
    *this = icon;
    return IsOk();
}

#if WXWIN_COMPATIBILITY_3_0
void wxBitmap::SetHeight(int height)
{
    M_PIXDATA = QPixmap(GetWidth(), height);
}

void wxBitmap::SetWidth(int width)
{
    M_PIXDATA = QPixmap(width, GetHeight());
}

void wxBitmap::SetDepth(int depth)
{
    if (depth == 1)
        M_PIXDATA = QBitmap(GetWidth(), GetHeight());
    else
        M_PIXDATA = QPixmap(GetWidth(), GetHeight());
}
#endif

void *wxBitmap::GetRawData(wxPixelDataBase& data, int bpp)
{
    void* bits = NULL;
    // allow access if bpp is valid and matches existence of alpha
    if ( !m_qtPixmap )
    {
        QImage qimage = m_qtPixmap->toImage();
        bool hasAlpha = m_qtPixmap->hasAlphaChannel();
        if ((bpp == 24 && !hasAlpha) || (bpp == 32 && hasAlpha))
        {
            data.m_height = qimage.height();
            data.m_width = qimage.width();
            data.m_stride = qimage.bytesPerLine();
            bits = (void*) qimage.bits();
        }
    }
    return bits;
}


QPixmap *wxBitmap::GetHandle() const
{
    return m_qtPixmap;
}



bool wxBitmap::HasAlpha() const
{
    if(!m_qtPixmap) return false;
    return m_qtPixmap->hasAlphaChannel();
}


wxMask::wxMask()
{
    m_qtBitmap = NULL;
}

wxMask::wxMask(const wxMask &mask)
{
    QBitmap *mask_bmp = mask.GetHandle();
    m_qtBitmap = mask_bmp ? new QBitmap(*mask_bmp) : NULL;
}

wxMask& wxMask::operator=(const wxMask &mask)
{
    delete m_qtBitmap;
    QBitmap *mask_bmp = mask.GetHandle();
    m_qtBitmap = mask_bmp ? new QBitmap(*mask_bmp) : NULL;
    return *this;
}    

wxMask::wxMask(const wxBitmap& bitmap, const QColor& colour)
{
    m_qtBitmap = NULL;
    Create(bitmap, colour);
}

wxMask::wxMask(const wxBitmap& bitmap)
{
    m_qtBitmap = NULL;
    Create(bitmap);
}

wxMask::~wxMask()
{
    delete m_qtBitmap;
}

// this function is called from Create() to free the existing mask data
void wxMask::FreeData()
{
    delete m_qtBitmap;
    m_qtBitmap = NULL;
}

bool wxMask::InitFromColour(const wxBitmap& bitmap, const QColor& colour)
{
    if (!bitmap.isOK()) return false;

    delete m_qtBitmap;
    m_qtBitmap = new QBitmap(bitmap.GetHandle()->createMaskFromColor(colour));

    return true;
}

bool wxMask::InitFromMonoBitmap(const wxBitmap& bitmap)
{
    //Only for mono bitmaps
    if (!bitmap.isOK() || bitmap.GetDepth() != 1)
        return false;

    delete m_qtBitmap;
    m_qtBitmap = new QBitmap(*bitmap.GetHandle());

    return true;
}

wxBitmap wxMask::GetBitmap() const
{
    return wxBitmap(*m_qtBitmap);
}

QBitmap *wxMask::GetHandle() const
{
    return m_qtBitmap;
}


bool wxMask::Create(const wxBitmap& bitmap, const QColor& colour)
{
    FreeData();

    return InitFromColour(bitmap, colour);
}


bool wxMask::Create(const wxBitmap& bitmap)
{
    FreeData();

    return InitFromMonoBitmap(bitmap);
}

