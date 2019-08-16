#ifndef ZCHXPAINTER_H
#define ZCHXPAINTER_H

#include <QPainter>

class zchxPainter : public QPainter
{
    Q_OBJECT
public:
    zchxPainter();
    zchxPainter(QPaintDevice* dev);
    void Clear();
    void resetBoundingBox();
    void calculateBoundingBox(int x, int y);
    int  MaxX() {return mMaxX;}
    int  MinX() {return mMinX;}
    int  MaxY() {return mMaxY;}
    int  MinY() {return mMinY;}

signals:

public slots:
private:
    int     mMinX;
    int     mMaxX;
    int     mMinY;
    int     mMaxY;
};

class zchxMemoryPainter : public zchxPainter
{
public:
    zchxMemoryPainter() : zchxPainter() {}
    zchxMemoryPainter(QPaintDevice* dev) : zchxPainter(dev) {}
};

#endif // ZCHXPAINTER_H
