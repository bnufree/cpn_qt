#ifndef _DEF_H
#define _DEF_H

#include <QApplication>
#include <QDir>
#include <QTextCodec>
#include <QPolygon>
#include <QPolygonF>
#include <QImage>
#include <QMouseEvent>
#include <QDebug>
#include <QDateTime>
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
    QList<S52color*> *color;
    S52ColorHashMap S52Colors;
    QColorHashMap QColors;
} colTable;

struct zchxPoint{
    int x;
    int y;
    zchxPoint() {}
    zchxPoint(int px, int py) {x = px; y=py;}
    zchxPoint(const QPoint& pos) {x = pos.x(); y=pos.y();}

    void operator+=(const zchxPoint &p) {x+=p.x; y+=p.y;}
    void operator-=(const zchxPoint &p) {x-=p.x; y-=p.y;}
    void operator*=(qreal c) {x/=c; y/=c;}
    void operator/=(qreal c) {x*=c; y*=c;}


    QPoint toPoint() const {return QPoint(x, y);}

    static QPolygon makePoiygon(zchxPoint* pnt, int size)
    {
        QPolygon res;
        for(int i=0; i<size; i++)
        {
            zchxPoint temp = pnt[size];
            res.append(QPoint(temp.x, temp.y));
        }
    }

    bool operator ==(const zchxPoint& other) const
    {
        return other.toPoint() == this->toPoint();
    }

    bool operator !=(const zchxPoint& other) const
    {
        return !(*this == other);
    }
};

struct zchxPointF{
    double x;
    double y;
    zchxPointF() {}
    zchxPointF(double px, double py) {x = px; y=py;}
    zchxPointF(const zchxPoint& p) {x = p.x; y = p.y;}
    QPointF toPointF() const {return QPointF(x, y);}
    bool operator ==(const zchxPointF& other) const
    {
        return other.toPointF() == this->toPointF();
    }

    bool operator !=(const zchxPointF& other) const
    {
        return !(*this == other);
    }
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

Q_DECLARE_METATYPE(ChartDirInfo)

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
        if(!fp) return false;
        return !HasError();
    }

    bool HasError() const {return isError;}

    int Read(void* buf, int size)
    {
        if(!fp)
        {
            isError = true;
            return 0;
        }
        int res = fread(buf, size, 1, fp);
        mLastRead = res * size;
        isError = (res != 1);
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
        isError = (res != 1);;
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

enum
{
    DISTANCE_NMI = 0,
    DISTANCE_MI,
    DISTANCE_KM,
    DISTANCE_M,
    DISTANCE_FT,
    DISTANCE_FA,
    DISTANCE_IN,
    DISTANCE_CM
};

enum
{
    SPEED_KTS = 0,
    SPEED_MPH,
    SPEED_KMH,
    SPEED_MS
};

class zchxFuncUtil
{
public:
    zchxFuncUtil() {}
    static bool isDirExist(const QString& name);
    static bool isFileExist(const QString& name);
    static QTextCodec* codesOfName(const QString& name);
    static QString convertCodesStringToUtf8(const char* str, const QString& codes);
    static bool isSameAs(const QString& p1, const QString& p2, bool caseSensitive );
    static bool renameFileExt(QString& newPath,
                              const QString& oldFile,
                              const QString& newExt);
    static QString getNewFileNameWithExt(const QString& oldName, const QString& newExt);
    static QString getFileName(const QString& fullName);
    static QString getFileExt(const QString& fullname);
    static QString getTempDir();
    static bool isImageTransparent(const QImage& img, int x, int y, int alpha);
    static QColor getNewRGBColor(const QColor& rgb, double level);
    static bool IsLeftMouseDown(QMouseEvent* e);
    static bool IsLeftMouseUp(QMouseEvent* e);
    static bool IsRightMouseDown(QMouseEvent* e);
    static bool IsRightMouseUp(QMouseEvent* e);
    static bool IsMouseUp(QMouseEvent* e);
    static double toUsrDistance( double nm_distance, int unit = -1 );
    static QString getUsrDistanceUnit( int unit = -1);
    static double fromUsrDistance( double usr_distance, int unit = -1 );
    static double toUsrSpeed( double kts_speed, int unit = -1  );
    static double fromUsrSpeed( double usr_speed, int unit = -1  );
    static QString getUsrSpeedUnit( int unit = -1  );
    static QString toSDMM(int NEflag, double a, bool hi_precision = true);
    static QString FormatDistanceAdaptive( double distance );
    static QString formatAngle(double angle, double mag, bool show_mag, bool show_true);
    static double  fromDMM(QString sdms);
    static void AlphaBlending(int x, int y, int size_x, int size_y, float radius,
                              QColor color, unsigned char transparency );
    static  qint64 getProcessIDFromSystem();
    static  qint64 getApplicationMemoryUse();
    static  void    getMemoryStatus(int* total = 0, int* used = 0);

    static QString getAppDir();
    static QString getDataDir();
    static QString separator();
    static QString getPluginDir();
    static QString getConfigFileName();
    static float getChartScaleFactorExp( float scale_linear );
    static double getFontPointsperPixel();

public:
    static double m_pt_per_pixel;


};


typedef QList<float *> MyFloatPtrArray;

class PI_line_segment_element
{
public:
    size_t              vbo_offset;
    size_t              n_points;
    int                 priority;
    float               lat_max;                // segment bounding box
    float               lat_min;
    float               lon_max;
    float               lon_min;
    int                 type;
    void                *private0;

    PI_line_segment_element *next;
};

#define         Q_INDEX_NOT_FOUND       -1

#define MAX_COG_AVERAGE_SECONDS        60
#define MAX_COGSOG_FILTER_SECONDS      60

#endif // _DEF_H
