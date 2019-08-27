#include "_def.h"
#include "zchxmapmainwindow.h"
#include "GL/gl.h"

extern int g_iDistanceFormat;
extern int g_iSpeedFormat;
extern bool g_bShowMag;
extern bool g_bShowTrue;
extern zchxMapMainWindow *gFrame;
extern int g_iSDMMFormat;


bool zchxFuncUtil::isDirExist(const QString& name)
{
    QDir dir(name);
    return dir.exists();
}

bool zchxFuncUtil::isFileExist(const QString& name)
{
    return QFile::exists(name);
}

QTextCodec* zchxFuncUtil::codesOfName(const QString& name)
{
    return QTextCodec::codecForName(name.toLatin1().data());
}

QString zchxFuncUtil::convertCodesStringToUtf8(const char* str, const QString& codes)
{
    QTextCodec *src_codes = codesOfName(codes);
    QTextCodec *utf8 = codesOfName("UTF-8");
    QString unicode = src_codes->toUnicode(str);
    return QString::fromUtf8(utf8->fromUnicode(unicode));
}

bool zchxFuncUtil::isSameAs(const QString& p1, const QString& p2, bool caseSensitive )
{
    Qt::CaseSensitivity val = (caseSensitive == true ?  Qt::CaseSensitive : Qt::CaseInsensitive);
    return QString::compare(p1, p2, val) == 0;
}

bool zchxFuncUtil::renameFileExt(QString& newPath, const QString& oldFile, const QString& newExt)
{
    QFile file(oldFile);
    if(file.exists())
    {
        int last_index = oldFile.lastIndexOf(".");
        if(last_index >= 0)
        {
            QString ext = oldFile.mid(last_index+1);
            QString newname = oldFile;
            newname.replace(last_index+1, ext.size(), newExt);
            if(file.rename(newname))
            {
                newPath = newname;
                return true;
            }
        }
    }

    return false;
}

QString zchxFuncUtil::getNewFileNameWithExt(const QString& oldName, const QString& newExt)
{
    int last_index = oldName.lastIndexOf(".");
    if(last_index >= 0)
    {
        QString ext = oldName.mid(last_index+1);
        QString newname = oldName;
        newname.replace(last_index+1, ext.size(), newExt);
        return newname;
    }
    QString temp = oldName;
    return temp.append(".").append(newExt);
}


QString zchxFuncUtil::getFileName(const QString& fullName)
{
    int index = fullName.lastIndexOf(".");
    return fullName.left(index);
}

QString zchxFuncUtil::getFileExt(const QString& fullname)
{
    int index = fullname.lastIndexOf(".");
    return fullname.mid(index+1);
}

QString zchxFuncUtil::getTempDir()
{
    QString path = QApplication::applicationDirPath();
    if(path.right(1) != QDir::separator()) path.append(QDir::separator());
    QString temp_path = QString("%1__temp").arg(path);
    QDir dir(temp_path);
    if(!dir.exists()) dir.mkpath(temp_path);
    return temp_path;
}

bool zchxFuncUtil::isImageTransparent(const QImage& img, int x, int y, int alpha)
{
    QColor color = img.pixelColor(x, y);
    if(color.alpha() <= alpha) return true;
    return false;
}

QColor zchxFuncUtil::getNewRGBColor(const QColor& rgb, double level)
{
    QColor hsv = rgb.toHsv();
    int h = hsv.hsvHue();
    int s = hsv.hsvSaturation();
    int v = hsv.value();
    int new_v = (int)( v * level);
    hsv.setHsv(h, s, new_v);
    return hsv.toRgb();
}

bool zchxFuncUtil::IsLeftMouseDown(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonPress) return false;
    if(e->button() != Qt::LeftButton) return false;
    return true;
}

bool zchxFuncUtil::IsLeftMouseUp(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonRelease) return false;
    if(e->button() != Qt::LeftButton) return false;
    return true;
}

bool zchxFuncUtil::IsRightMouseDown(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonPress) return false;
    if(e->button() != Qt::RightButton) return false;
    return true;
}

bool zchxFuncUtil::IsRightMouseUp(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonRelease) return false;
    if(e->button() != Qt::RightButton) return false;
    return true;
}

bool zchxFuncUtil::IsMouseUp(QMouseEvent* e)
{
    if(e->type() != QEvent::MouseButtonRelease) return false;
    return true;
}

double zchxFuncUtil::toUsrDistance( double nm_distance, int unit  )
{
    double ret = NAN;
    if ( unit == -1 )
        unit = g_iDistanceFormat;
    switch( unit ){
    case DISTANCE_NMI: //Nautical miles
        ret = nm_distance;
        break;
    case DISTANCE_MI: //Statute miles
        ret = nm_distance * 1.15078;
        break;
    case DISTANCE_KM:
        ret = nm_distance * 1.852;
        break;
    case DISTANCE_M:
        ret = nm_distance * 1852;
        break;
    case DISTANCE_FT:
        ret = nm_distance * 6076.12;
        break;
    case DISTANCE_FA:
        ret = nm_distance * 1012.68591;
        break;
    case DISTANCE_IN:
        ret = nm_distance * 72913.4;
        break;
    case DISTANCE_CM:
        ret = nm_distance * 185200;
        break;
    }
    return ret;
}

QString zchxFuncUtil::getUsrDistanceUnit( int unit )
{
    QString ret;
    if ( unit == -1 )
        unit = g_iDistanceFormat;
    switch( unit ){
        case DISTANCE_NMI: //Nautical miles
            ret = ("NMi");
            break;
        case DISTANCE_MI: //Statute miles
            ret = ("mi");
            break;
        case DISTANCE_KM:
            ret = ("km");
            break;
        case DISTANCE_M:
            ret = ("m");
            break;
        case DISTANCE_FT:
            ret = ("ft");
            break;
        case DISTANCE_FA:
            ret = ("fa");
            break;
        case DISTANCE_IN:
            ret = ("in");
        break;
        case DISTANCE_CM:
            ret = ("cm");
            break;
    }
    return ret;
}

double zchxFuncUtil::fromUsrDistance( double usr_distance, int unit )
{
    double ret = NAN;
    if ( unit == -1 )
        unit = g_iDistanceFormat;
    switch( unit ){
        case DISTANCE_NMI: //Nautical miles
            ret = usr_distance;
            break;
        case DISTANCE_MI: //Statute miles
            ret = usr_distance / 1.15078;
            break;
        case DISTANCE_KM:
            ret = usr_distance / 1.852;
            break;
        case DISTANCE_M:
            ret = usr_distance / 1852;
            break;
        case DISTANCE_FT:
            ret = usr_distance / 6076.12;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the speed to the units selected by user              */
/**************************************************************************/
double zchxFuncUtil::toUsrSpeed( double kts_speed, int unit )
{
    double ret = NAN;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit )
    {
        case SPEED_KTS: //kts
            ret = kts_speed;
            break;
        case SPEED_MPH: //mph
            ret = kts_speed * 1.15078;
            break;
        case SPEED_KMH: //km/h
            ret = kts_speed * 1.852;
            break;
        case SPEED_MS: //m/s
            ret = kts_speed * 0.514444444;
            break;
    }
    return ret;
}

/**************************************************************************/
/*          Converts the speed from the units selected by user to knots   */
/**************************************************************************/
double fromUsrSpeed( double usr_speed, int unit )
{
    double ret = NAN;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit )
    {
        case SPEED_KTS: //kts
            ret = usr_speed;
            break;
        case SPEED_MPH: //mph
            ret = usr_speed / 1.15078;
            break;
        case SPEED_KMH: //km/h
            ret = usr_speed / 1.852;
            break;
        case SPEED_MS: //m/s
            ret = usr_speed / 0.514444444;
            break;
    }
    return ret;
}

QString zchxFuncUtil::getUsrSpeedUnit( int unit )
{
    QString ret;
    if ( unit == -1 )
        unit = g_iSpeedFormat;
    switch( unit ){
        case SPEED_KTS: //kts
            ret = ("kts");
            break;
        case SPEED_MPH: //mph
            ret = ("mph");
            break;
        case SPEED_KMH:
            ret = ("km/h");
            break;
        case SPEED_MS:
            ret = ("m/s");
            break;
    }
    return ret;
}


QString zchxFuncUtil::FormatDistanceAdaptive( double distance ) {
    QString result;
    int unit = g_iDistanceFormat;
    double usrDistance = toUsrDistance( distance, unit );
    if( usrDistance < 0.1 &&
      ( unit == DISTANCE_KM || unit == DISTANCE_MI || unit == DISTANCE_NMI ) ) {
        unit = ( unit == DISTANCE_MI ) ? DISTANCE_FT : DISTANCE_M;
        usrDistance = toUsrDistance( distance, unit );
    }
    QString format;
    if( usrDistance < 5.0 ) {
        format = ("%1.2f ");
    } else if( usrDistance < 100.0 ) {
        format = ("%2.1f ");
    } else if( usrDistance < 1000.0 ) {
        format = ("%3.0f ");
    } else {
        format = ("%4.0f ");
    }
    result.append(QString("").sprintf(format.toUtf8().data(), usrDistance ))
            .append(getUsrDistanceUnit( unit ));
    return result;
}

/**************************************************************************/
/*          Formats the coordinates to string                             */
/**************************************************************************/
QString zchxFuncUtil::toSDMM( int NEflag, double a, bool hi_precision )
{
    QString s;
    double mpy;
    short neg = 0;
    int d;
    long m;
    double ang = a;
    char c = 'N';

    if( a < 0.0 ) {
        a = -a;
        neg = 1;
    }
    d = (int) a;
    if( neg ) d = -d;
    if( NEflag ) {
        if( NEflag == 1 ) {
            c = 'N';

            if( neg ) {
                d = -d;
                c = 'S';
            }
        } else
            if( NEflag == 2 ) {
                c = 'E';

                if( neg ) {
                    d = -d;
                    c = 'W';
                }
            }
    }

    switch( g_iSDMMFormat ){
        case 0:
            mpy = 600.0;
            if( hi_precision ) mpy = mpy * 1000;

            m = (long) qRound( ( a - (double) d ) * mpy );

            if( !NEflag || NEflag < 1 || NEflag > 2 ) //Does it EVER happen?
                    {
                if( hi_precision ) s.sprintf( ( "%d\u00B0 %02ld.%04ld'" ), d, m / 10000, m % 10000 );
                else
                    s.sprintf( ( "%d\u00B0 %02ld.%01ld'" ), d, m / 10, m % 10 );
            } else {
                if( hi_precision )
                    if (NEflag == 1)
                        s.sprintf( ( "%02d\u00B0 %02ld.%04ld' %c" ), d, m / 10000, ( m % 10000 ), c );
                    else
                        s.sprintf(( "%03d\u00B0 %02ld.%04ld' %c" ), d, m / 10000, ( m % 10000 ), c );
                else
                    if (NEflag == 1)
                        s.sprintf( ( "%02d\u00B0 %02ld.%01ld' %c" ), d, m / 10, ( m % 10 ), c );
                    else
                        s.sprintf(( "%03d\u00B0 %02ld.%01ld' %c" ), d, m / 10, ( m % 10 ), c );
            }
            break;
        case 1:
            if( hi_precision ) s.sprintf( ( "%03.6f" ), ang ); //cca 11 cm - the GPX precision is higher, but as we use hi_precision almost everywhere it would be a little too much....
            else
                s.sprintf(( "%03.4f" ), ang ); //cca 11m
            break;
        case 2:
            m = (long) ( ( a - (double) d ) * 60 );
            mpy = 10.0;
            if( hi_precision ) mpy = mpy * 100;
            long sec = (long) ( ( a - (double) d - ( ( (double) m ) / 60 ) ) * 3600 * mpy );

            if( !NEflag || NEflag < 1 || NEflag > 2 ) //Does it EVER happen?
                    {
                if( hi_precision ) s.sprintf(( "%d\u00B0 %ld'%ld.%ld\"" ), d, m, sec / 1000,
                        sec % 1000 );
                else
                    s.sprintf( ( "%d\u00B0 %ld'%ld.%ld\"" ), d, m, sec / 10, sec % 10 );
            } else {
                if( hi_precision )
                    if (NEflag == 1)
                        s.sprintf( ( "%02d\u00B0 %02ld' %02ld.%03ld\" %c" ), d, m, sec / 1000, sec % 1000, c );
                    else
                        s.sprintf(( "%03d\u00B0 %02ld' %02ld.%03ld\" %c" ), d, m, sec / 1000, sec % 1000, c );
                else
                    if (NEflag == 1)
                        s.sprintf(( "%02d\u00B0 %02ld' %02ld.%ld\" %c" ), d, m, sec / 10, sec % 10, c );
                    else
                        s.sprintf(( "%03d\u00B0 %02ld' %02ld.%ld\" %c" ), d, m, sec / 10, sec % 10, c );
            }
            break;
    }
    return s;
}


double zchxFuncUtil::fromDMM( QString sdms )
{
    wchar_t buf[64];
    char narrowbuf[64];
    int i, len, top = 0;
    double stk[32], sign = 1;

    //First round of string modifications to accomodate some known strange formats
    QString replhelper = QString::fromUtf8("´·" ); //UKHO PDFs
    sdms.replace( replhelper, (".") );
    replhelper = QString::fromUtf8("\"·" ); //Don't know if used, but to make sure
    sdms.replace( replhelper, (".") );
    replhelper = QString::fromUtf8( "·" );
    sdms.replace( replhelper, (".") );

    replhelper = QString::fromUtf8("s. š." ); //Another example: cs.wikipedia.org (someone was too active translating...)
    sdms.replace( replhelper, ("N") );
    replhelper = QString::fromUtf8( "j. š." );
    sdms.replace( replhelper, ("S") );
    sdms.replace( ("v. d."), ("E") );
    sdms.replace( ("z. d."), ("W") );

    //If the string contains hemisphere specified by a letter, then '-' is for sure a separator...
    sdms = sdms.toUpper();
    if( sdms.contains(("N") ) || sdms.contains(("S") ) || sdms.contains( ("E") )
            || sdms.contains( ("W") ) ) sdms.replace( ("-"), (" ") );

    wcsncpy( buf, sdms.toStdWString().data(), 63 );
    buf[63] = 0;
    len = qMin( wcslen( buf ), sizeof(narrowbuf)-1);;

    for( i = 0; i < len; i++ ) {
        wchar_t c = buf[i];
        if( ( c >= '0' && c <= '9' ) || c == '-' || c == '.' || c == '+' ) {
            narrowbuf[i] = c;
            continue; /* Digit characters are cool as is */
        }
        if( c == ',' ) {
            narrowbuf[i] = '.'; /* convert to decimal dot */
            continue;
        }
        if( ( c | 32 ) == 'w' || ( c | 32 ) == 's' ) sign = -1; /* These mean "negate" (note case insensitivity) */
        narrowbuf[i] = 0; /* Replace everything else with nuls */
    }

    /* Build a stack of doubles */
    stk[0] = stk[1] = stk[2] = 0;
    for( i = 0; i < len; i++ ) {
        while( i < len && narrowbuf[i] == 0 )
            i++;
        if( i != len ) {
            stk[top++] = atof( narrowbuf + i );
            i += strlen( narrowbuf + i );
        }
    }

    return sign * ( stk[0] + ( stk[1] + stk[2] / 60 ) / 60 );
}

QString zchxFuncUtil::formatAngle(double angle)
{
    QString out;
    if( g_bShowMag && g_bShowTrue ) {
        out.sprintf("%.0f \u00B0T (%.0f \u00B0M)", angle, gFrame->GetMag(angle));
    } else if( g_bShowTrue ) {
        out.sprintf("%.0f \u00B0T", angle);
    } else {
        out.sprintf("%.0f \u00B0M", gFrame->GetMag(angle));
    }
    return out;
}

/* render a rectangle at a given color and transparency */
void zchxFuncUtil::AlphaBlending(int x, int y, int size_x, int size_y,
                                 float radius, QColor color,
                                    unsigned char transparency )
{

    glEnable( GL_BLEND );
    glColor4ub( color.red(), color.green(), color.blue(), transparency );
    glBegin( GL_QUADS );
    glVertex2i( x, y );
    glVertex2i( x + size_x, y );
    glVertex2i( x + size_x, y + size_y );
    glVertex2i( x, y + size_y );
    glEnd();
    glDisable( GL_BLEND );
}





