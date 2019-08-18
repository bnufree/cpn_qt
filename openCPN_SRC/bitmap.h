/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/bitmap.h
// Author:      Peter Most, Javier Torres, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_BITMAP_H_
#define _WX_QT_BITMAP_H_

#include <QBitmap>

class QImage;
class QPixmap;
class QBitmap;
class wxMask;
class QCursor;

class wxBitmap
{
public:
    wxBitmap();
    wxBitmap(QPixmap pix);
    wxBitmap(const wxBitmap& bmp);
    wxBitmap(const char bits[], int width, int height, int depth = 1);
    wxBitmap(int width, int height, int depth = -1);
    wxBitmap(const QSize& sz, int depth = -1);
    wxBitmap(const char* const* bits);
    wxBitmap(const QString &filename);
    wxBitmap(const QImage& image, int depth = -1, double scale = 1.0);
    
    // Convert from wxIcon / wxCursor
//    wxBitmap(const wxIcon& icon) { CopyFromIcon(icon); }
    explicit wxBitmap(const QCursor& cursor);

    virtual ~wxBitmap();

    virtual bool Create(int width, int height, int depth = -1);
    virtual bool Create(const QSize& sz, int depth = -1);

    virtual int GetHeight() const;
    virtual int GetWidth() const;
    virtual int GetDepth() const;
    virtual QImage ConvertToImage() const;

    virtual wxMask *GetMask() const;
    virtual void SetMask(wxMask *mask);

    virtual wxBitmap GetSubBitmap(const QRect& rect) const;

    virtual bool SaveFile(const QString &name, const QString& type) const; // tpye: bmp, ico, jpeg, png, gif, cur, tif, xbm, pcx
    virtual bool LoadFile(const QString &name);

    // copies the contents and mask of the given (colour) icon to the bitmap
//    virtual bool CopyFromIcon(const wxIcon& icon);
    // these functions are internal and shouldn't be used, they risk to
    // disappear in the future
    bool HasAlpha() const;
    QPixmap *GetHandle() const;
    bool isOK() const {return m_qtPixmap != 0;}

private:
    QPixmap*    m_qtPixmap;
    wxMask*     m_mask;
};

class wxMask
{
public:
    wxMask();

    // Copy constructor
    wxMask(const wxMask &mask);
    wxMask& operator=(const wxMask &mask);

    // Construct a mask from a bitmap and a colour indicating the transparent
    // area
    wxMask(const wxBitmap& bitmap, const QColor& colour);
    // Construct a mask from a mono bitmap (copies the bitmap).
    wxMask(const wxBitmap& bitmap);
    virtual ~wxMask();

    // Implementation
    QBitmap *GetHandle() const;


    bool Create(const wxBitmap& bitmap, const QColor& colour);
    bool Create(const wxBitmap& bitmap);


protected:
    // this function is called from Create() to free the existing mask data
    void FreeData();
    // by the public wrappers
    bool InitFromColour(const wxBitmap& bitmap, const QColor& colour);
    bool InitFromMonoBitmap(const wxBitmap& bitmap);

    wxBitmap GetBitmap() const;

private:
    QBitmap *m_qtBitmap;
};

#endif // _WX_QT_BITMAP_H_
