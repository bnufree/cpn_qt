#ifndef _DEF_H
#define _DEF_H

#include <QApplication>
#include <QDir>
#include <QTextCodec>

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
    Depth_Feet = 0,           //英尺 1 foot 英尺(呎) = 0.00018939393939394miles(英里）=12 inches 英寸(吋) = 30.48 centimetres(厘米)=0.3048meters(米) = 0.333333333333333333yard(码)
    Depth_Meters,             //米
    Depth_Fathoms,            //英寻(测量水深单位，合6英尺或1.8米)
};

enum LatlonFormat{
    Degrees_Decimal_Minutes = 0,    //度分
    Decimal_Degrees,                //度
    Degrees_Minutes_Seconds,        //度分秒
};


class zchxGLOptions
{
public:
    bool m_bUseAcceleratedPanning;
    bool m_bUseCanvasPanning;

    bool m_bTextureCompression;
    bool m_bTextureCompressionCaching;

    int m_iTextureDimension;
    int m_iTextureMemorySize;

    bool m_GLPolygonSmoothing;
    bool m_GLLineSmoothing;
};

#include <QHash>
#include <QColor>
#include <QList>

typedef struct _S52color{
    char colName[20];
    unsigned char  R;
    unsigned char  G;
    unsigned char  B;
}S52color;

typedef         QHash<QString, QColor>          QColorHashMap;
typedef         QHash<QString, S52color>        S52ColorHashMap;
typedef         QList<void*>                    wxArrayPtrVoid;

typedef struct _colTable {
    QString tableName;
    QString rasterFileName;
//    wxArrayPtrVoid *color;
    S52ColorHashMap S52Colors;
    QColorHashMap QColors;
} colTable;

struct zchxPoint{
    int x;
    int y;

    QPoint toPoint() {return QPoint(x, y);}
};

struct zchxPointF{
    double x;
    double y;
    QPointF toPointF() {return QPointF(x, y);}
};

struct zchxSize{
    int width;
    int height;

    QSize toSize() {return QSize(width, height);}
};


//    ChartType constants
typedef enum ChartTypeEnum
{
      CHART_TYPE_UNKNOWN = 0,
      CHART_TYPE_DUMMY,
      CHART_TYPE_DONTCARE,
      CHART_TYPE_KAP,
      CHART_TYPE_GEO,
      CHART_TYPE_S57,
      CHART_TYPE_CM93,
      CHART_TYPE_CM93COMP,
      CHART_TYPE_PLUGIN,
      CHART_TYPE_MBTILES
}_ChartTypeEnum;

//    ChartFamily constants
typedef enum ChartFamilyEnum
{
      CHART_FAMILY_UNKNOWN = 0,
      CHART_FAMILY_RASTER,
      CHART_FAMILY_VECTOR,
      CHART_FAMILY_DONTCARE
}_ChartFamilyEnum;

typedef enum ColorScheme
{
      GLOBAL_COLOR_SCHEME_RGB,
      GLOBAL_COLOR_SCHEME_DAY,
      GLOBAL_COLOR_SCHEME_DUSK,
      GLOBAL_COLOR_SCHEME_NIGHT,
      N_COLOR_SCHEMES
}_ColorScheme;

// display category type
typedef enum _DisCat{
   DISPLAYBASE          = 'D',            //
   STANDARD             = 'S',            //
   OTHER                = 'O',            // O for OTHER
   MARINERS_STANDARD    = 'M',            // Mariner specified
   MARINERS_OTHER,                        // value not defined
   DISP_CAT_NUM,                          // value not defined
}DisCat;




////----------------------------------------------------------------------------
//// ocpn Toolbar stuff
////----------------------------------------------------------------------------
//class ChartBase;
//class wxSocketEvent;
//class ocpnToolBarSimple;


//    A generic Position Data structure
typedef struct {
    double kLat;
    double kLon;
    double kCog;
    double kSog;
    double kVar;            // Variation, typically from RMC message
    double kHdm;            // Magnetic heading
    double kHdt;            // true heading
    time_t FixTime;
    int    nSats;
} GenericPosDatEx;

class ChartDirInfo
{
public:
    QString    fullpath;
    QString    magic_number;

    ChartDirInfo() {}
    ChartDirInfo(const QString& path, const QString& num = QString())
    {
        fullpath = path;
        magic_number = num;
    }

    bool operator ==(const ChartDirInfo& other)
    {
        return this->fullpath == other.fullpath;
    }
};

typedef QList<ChartDirInfo> ArrayOfCDI;



class FileReadWrite
{
public:
    enum FileOptMode{
        E_READ = 0,
        E_Write,
    };

    enum FileSeekMode{
        SEEK_FROM_START = 0,
        SEEK_FROM_CUR,
        SEEK_FROM_END,
    };

    FileReadWrite() : mName(""), mode(E_READ), fp(0), isError(0), mLastRead(0)
    {}
    FileReadWrite(const QString& fileName, int opt = E_READ) : mName(fileName), mode(opt), fp(0), isError(0), mLastRead(0)
    { fp = fopen(mName.toUtf8().data(), mode == E_READ? "rb" : "wb");}

    FileReadWrite(const FileReadWrite& other)
    {
        mName = other.mName;
        mode = other.mode;
        isError = 0;
        mLastRead = 0;
        fp = fopen(mName.toUtf8().data(), mode == E_READ? "rb" : "wb");
    }

    ~FileReadWrite() {if(fp) fclose(fp);}
    virtual bool IsOK() const
    {
        return fp != NULL;
    }

    bool HasError() {return isError;}

    int Read(void* buf, int size)
    {
        if(!fp)
        {
            isError = true;
            return 0;
        }
        int res = fread(buf, size, 1, fp);
        mLastRead += res;
        isError = (res == size);
        return res;
    }

    int Write(void* buf, int size)
    {
        if(!fp)
        {
            isError = true;
            return 0;
        }
        int res = fwrite(buf, size, 1, fp);
        isError = (res == size);;
        return res;
    }

    int GetC()
    {
        if(!fp)
        {
            isError = true;
            return 0;
        }
        return fgetc(fp);
    }

    int Ungetch(char a)
    {
        return ungetc(a, fp);
    }


    bool IsEof()
    {
        if(!fp)
        {
            isError = true;
            return false;
        }
        return feof(fp);
    }

    int Seek(uint64_t pos, int mode)
    {
        return fseek(fp, pos, mode);
    }

    uint64_t TellI()
    {
        return ftell(fp);
    }

    uint64_t LastRead() const {return mLastRead;}

    void Close()
    {
        if(fp)
        {
            fclose(fp);
            fp = 0;
        }
    }

private:
    FILE* fp;
    QString mName;
    int mode;
    bool    isError;
    uint64_t mLastRead;
};

class zchxFuncUtil
{
public:
    zchxFuncUtil() {}
    static bool isDirExist(const QString& name)
    {
        QDir dir(name);
        return dir.exists();
    }

    static bool isFileExist(const QString& name)
    {
        return QFile::exists(name);
    }

    QTextCodec* codesOfName(const QString& name)
    {
        return QTextCodec::codecForName(name.toLatin1().data());
    }
};




#endif // _DEF_H
