#ifndef _DEF_H
#define _DEF_H

#include <QApplication>

/*  menu and toolbar item kinds */
enum wxItemKind
{
    wxITEM_SEPARATOR = -1,
    wxITEM_NORMAL,
    wxITEM_CHECK,
    wxITEM_RADIO,
    wxITEM_DROPDOWN,
    wxITEM_MAX
};

const           QString APP_DIR = QApplication::applicationDirPath();
const           QString MAP_DIR = QString("%1/mapdata").arg(APP_DIR);
const           QString PLUGIN_DIR = QString("%1/plugins").arg(MAP_DIR);

enum NaviMode{
    Navi_North = 0,         //正北导航
    Navi_Course,            //航向导航
};

enum DistanceUnit{
    Nautical_Miles = 0, //海里
    Statute_Miles, //英里
    Kilometers, //千米
    Meters, //米
};

enum SpeedUnit{
    Knots = 0,      //节
    MPH,            //米每小时
    KmPH,           //千米每时
    MPS,            //米每秒
};

enum DepthUnit{
    Feet = 0,           //英尺 1 foot 英尺(呎) = 0.00018939393939394miles(英里）=12 inches 英寸(吋) = 30.48 centimetres(厘米)=0.3048meters(米) = 0.333333333333333333yard(码)
    Meters,             //米
    Fathoms,            //英寻(测量水深单位，合6英尺或1.8米)
};

enum LatlonFormat{
    Degrees_Decimal_Minutes = 0,    //度分
    Decimal_Degrees,                //度
    Degrees_Minutes_Seconds,        //度分秒
};



#endif // _DEF_H
