#ifndef ZCHXMAPLOADWORKER_H
#define ZCHXMAPLOADWORKER_H

#include <QObject>
#include <QThread>

class zchxMapLoadWorker : public QObject
{
    Q_OBJECT
public:
    explicit zchxMapLoadWorker(QObject *parent = 0);

signals:

public slots:
private:
    QThread     mWorkThread;
};

#endif // ZCHXMAPLOADWORKER_H
