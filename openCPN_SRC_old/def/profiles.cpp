#include "profiles.h"
#include <QSettings>
#include <QTextCodec>
#include <QApplication>

namespace qt {

Profiles* Profiles::minstance = 0;
Profiles::MGarbage Profiles::Garbage;

Profiles::Profiles(QObject *parent) :
    QObject(parent)
{
    configSettings = new QSettings("mapdata/settings.ini", QSettings::IniFormat);
    configSettings->setIniCodec(QTextCodec::codecForName("GB18030"));
    initDefaultValues();;
}

Profiles::~Profiles()
{
}
/*-------------------------------------------
 * 实例化
---------------------------------------------*/
Profiles *Profiles::instance()
{
    if ( minstance == 0)
    {
        minstance = new Profiles();
    }
    return minstance;
}

void Profiles::initDefaultValues()
{
    //地图配置文件相关的设定
    setDefault(MAP_INDEX, MAX_LINE_LENGTH, 100);
    setDefault(MAP_INDEX, WARN_FLAH_COLOR_ALPHA, 100);
    setDefault(MAP_INDEX, OPEN_MEET, false);
    setDefault(MAP_INDEX, MAP_UNIT, MapUnit::MapUnitNmi);
    setDefault(MAP_INDEX, MAP_STYLE_AUTO_CHANGE, true);
    setDefault(MAP_INDEX, MAP_DAY_TIME, "090000");
    setDefault(MAP_INDEX, MAP_DUSK_TIME, "170000");
    setDefault(MAP_INDEX, MAP_DUSK_TIME, "190000");
    setDefault(MAP_INDEX, MAP_DEFAULT_LAT, 20.12345678);
    setDefault(MAP_INDEX, MAP_DEFAULT_LON, 110.12345678);
    setDefault(MAP_INDEX, MAP_DEFAULT_ZOOM, 13);
    setDefault(MAP_INDEX, MAP_MIN_ZOOM, 13);
    setDefault(MAP_INDEX, MAP_MAX_ZOOM, 17);
    setDefault(MAP_INDEX, MAP_DEFAULT_TARGET_ZOOM, 15);
    setDefault(MAP_INDEX, MAP_UPDATE_INTERVAL, 100);
    setDefault(MAP_INDEX, MAP_SOURCE, 0);
    setDefault(MAP_INDEX, MAP_DISPLAY_MENU, true);
    setDefault(MAP_INDEX, MAP_FILE_DIR, QApplication::applicationDirPath() + "/mapdata/ecdis/");
    setDefault(MAP_INDEX, IMG_FILE_DIR, QApplication::applicationDirPath() + "/mapdata/image/");
    setDefault(MAP_INDEX, MAP_BACK_GROUND, "rgb(203,199,131)");
    //Ais显示配置
    setDefault(AIS_DISPLAY_SETTING, AIS_FILL_COLOR, QColor(Qt::green).name());
    setDefault(AIS_DISPLAY_SETTING, AIS_TEXT_COLOR, QColor(Qt::black).name());
    setDefault(AIS_DISPLAY_SETTING, AIS_BORDER_COLOR, QColor(Qt::green).name());
    setDefault(AIS_DISPLAY_SETTING, AIS_CONCERN_COLOR, QColor(Qt::red).name());
    setDefault(AIS_DISPLAY_SETTING, AIS_FORCED_IMAGE, false);
    setDefault(AIS_DISPLAY_SETTING, AIS_CONCERN_NUM, 10);
    setDefault(AIS_DISPLAY_SETTING, AIS_REPLACE_CONCERN, true);
    setDefault(AIS_DISPLAY_SETTING, AIS_TAIL_TRACK_NUM, 10);
    setDefault(AIS_DISPLAY_SETTING, AIS_REPLACE_TAIL_TRACK, true);
    setDefault(AIS_DISPLAY_SETTING, AIS_HISTORY_TRACK_NUM, 10);
    setDefault(AIS_DISPLAY_SETTING, AIS_REPLACE_HISTORY_TRACK, true);
    setDefault(AIS_DISPLAY_SETTING, AIS_EXTRAPOLATE_NUM, 10);
    setDefault(AIS_DISPLAY_SETTING, AIS_REPLACE_EXTRAPOLATE, true);

    //雷达显示配置
    setDefault(RADAR_DISPLAY_SETTING, RADAR_FILL_COLOR, QColor(Qt::green).name());
    setDefault(RADAR_DISPLAY_SETTING, RADAR_TEXT_COLOR, QColor(Qt::black).name());
    setDefault(RADAR_DISPLAY_SETTING, RADAR_CONCERN_COLOR, QColor(Qt::red).name());
    setDefault(RADAR_DISPLAY_SETTING, RADAR_BORDER_COLOR, QColor(Qt::black).name());
    setDefault(RADAR_DISPLAY_SETTING, RADAR_SHAPE_RECT, true);
    setDefault(RADAR_DISPLAY_SETTING, RADAR_FORCED_AIS, false);
    setDefault(RADAR_DISPLAY_SETTING, RADAR_CONCERN_NUM, 10);
    setDefault(RADAR_DISPLAY_SETTING, RADAR_REPLACE_CONCERN, true);
    setDefault(RADAR_DISPLAY_SETTING, RADAR_TAIL_TRACK_NUM, 10);
    setDefault(RADAR_DISPLAY_SETTING, RADAR_REPLACE_TAIL_TRACK, true);
    setDefault(RADAR_DISPLAY_SETTING, RADAR_HISTORY_TRACK_NUM, 10);
    setDefault(RADAR_DISPLAY_SETTING, RADAR_REPLACE_HISTORY_TRACK, true);

}

/*-------------------------------------------
 * 设置默认值
---------------------------------------------*/
void Profiles::setDefault(const QString & prefix,const QString &key, const QVariant &value)
{
    configSettings->beginGroup(prefix);
    if(configSettings->value(key).toString().isEmpty())
    {
        configSettings->setValue(key, value);
    }
    configSettings->endGroup();
}
/*-------------------------------------------
 * 设置配置文件值
---------------------------------------------*/
void Profiles::setValue(const QString & prefix,const QString & key, const QVariant & value)
{
    configSettings->beginGroup(prefix);
    {
        configSettings->setValue(key, value);
    }
    configSettings->endGroup();
}
/*-------------------------------------------
 * 返回值
---------------------------------------------*/
QVariant Profiles::value(const QString & prefix,const QString &keys, const QVariant &defaultValue)
{
//    QVariant values;
//    configSettings->beginGroup(prefix);
//    {
//        values =  configSettings->value(keys, defaultValue);
//    }
//    configSettings->endGroup();
    return configSettings->value(prefix+"/"+keys,defaultValue);;
}
}

