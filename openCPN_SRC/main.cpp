#include <QApplication>
#include <QSharedMemory>
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <iostream>
#include <QTextCodec>
#include <QMessageBox>
#include <QDebug>
#include "_def.h"
#include "zchxmapmainwindow.h"
#include <QPixmapCache>


#undef QT_MESSAGELOG_FILE
#undef QT_MESSAGELOG_LINE
#undef QT_MESSAGELOG_FUNC
#undef qDebug
#undef qInfo
#undef qWarning
#undef qCritical
#undef qFatal

#define QT_MESSAGELOG_FILE __FILE__
#define QT_MESSAGELOG_LINE __LINE__
#define QT_MESSAGELOG_FUNC Q_FUNC_INFO
#define qDebug QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug
#define qInfo QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).info
#define qWarning QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).warning
#define qCritical QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).critical
#define qFatal QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).fatal

void logMessageOutputQt5(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QString log_file_name = QApplication::applicationDirPath() + QString("/log/opencpn.log");
    static qint64 max = 10485760;//10 * 1024 * 1024;
    static QMutex mutex;
    static qint64 log_file_lenth = 0;
    mutex.lock();
    QString text;
    switch (type) {
    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
        abort();
    default:
        break;
    }
    QString message = QString("[%1] %2 [%3] [%4] [%5] %6")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(text)
            .arg(context.file)
            .arg(context.function)
            .arg(context.line)
            .arg(msg);

    QDir dir(QApplication::applicationDirPath() + QString("/log"));
    if(!dir.exists())
    {
        dir.mkpath(dir.path());
    }
    QFile file(log_file_name);
    if(log_file_lenth > max)
    {
        if(file.exists()) file.remove();
    }

    if(file.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        QTextStream text_stream(&file);
        text_stream << message << endl;
        file.flush();
        file.close();
        log_file_lenth = file.size();
    } else
    {
        std::cout <<"open file failed...........";
    }
    mutex.unlock();

    message = QString("[%1] %2 [%3] [%4] ").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(text).arg(context.file).arg(context.line);
    std::cout << "\033[31m" << message.toStdString();
    std::cout << "\033[32m" << msg.toUtf8().toStdString() <<std::endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));

    qInstallMessageHandler(logMessageOutputQt5);
    QSharedMemory mem("zchxCPN");
    if (!mem.create(1))
    {
        QMessageBox msgBox(QMessageBox::Warning, QObject::tr("提示"),QObject::tr("提示，您已开启一个zchxCPN程序，该程序不能同时开启多个。\n"));
        msgBox.addButton(QObject::tr("确定"), QMessageBox::AcceptRole);
        msgBox.exec();
        a.exit(1);
        return 0;
    }


    //make size as 9:16
//    gFrame = new zchxMapMainWindow();
//    gFrame->showMaximized();
    QPixmapCache::setCacheLimit(1);
    zchxMapMainWindow w;
    w.showMaximized();

    return a.exec();
}
