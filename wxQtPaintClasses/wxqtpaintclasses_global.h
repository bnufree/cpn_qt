#ifndef WXQTPAINTCLASSES_GLOBAL_H
#define WXQTPAINTCLASSES_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(WXQTPAINTCLASSES_LIBRARY)
#  define WXQTPAINTCLASSESSHARED_EXPORT Q_DECL_EXPORT
#else
#  define WXQTPAINTCLASSESSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // WXQTPAINTCLASSES_GLOBAL_H