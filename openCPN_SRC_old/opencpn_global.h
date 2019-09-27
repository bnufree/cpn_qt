#ifndef OPENCPN_GLOBAL_H
#define OPENCPN_GLOBAL_H

#include <qglobal.h>
#include <QLoggingCategory>

#if defined(ZCHX_OPENCPN_PLUGIN)
#  define ZCHX_OPENCPN_EXPORT Q_DECL_EXPORT
#else
#  define ZCHX_OPENCPN_EXPORT Q_DECL_IMPORT
#endif

Q_DECLARE_LOGGING_CATEGORY(ecdis)

#endif // OPENCPN_GLOBAL_H
