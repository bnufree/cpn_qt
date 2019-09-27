#include "zchxmaploadworker.h"

zchxMapLoadWorker::zchxMapLoadWorker(QObject *parent) : QObject(parent)
{
    this->moveToThread(&mWorkThread);
    mWorkThread.start();
}
