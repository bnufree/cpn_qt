#include "zchxpainter.h"

zchxPainter::zchxPainter(QPaintDevice *dev) : QPainter(dev)
{
    resetBoundingBox();
}

zchxPainter::zchxPainter() : QPainter()
{
    resetBoundingBox();
}

void zchxPainter::Clear()
{
    if(!this->device()) return;
    int width = this->device()->width();
    int height = this->device()->height();
    eraseRect(QRect(0, 0, width, height));
}

void zchxPainter::resetBoundingBox()
{
    mMinX = mMaxX = mMaxY = mMinY = 0;
}

void zchxPainter::calculateBoundingBox(int x, int y)
{
    if(mMinX  > x) mMinX = x;
    if(mMaxX  < x) mMaxX = x;
    if(mMinY  > y) mMinY = y;
    if(mMaxY  < y) mMaxY = y;

}


