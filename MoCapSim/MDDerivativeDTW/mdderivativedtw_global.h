#ifndef MDDERIVATIVEDTW_GLOBAL_H
#define MDDERIVATIVEDTW_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(MDDERIVATIVEDTW_LIBRARY)
#  define MDDERIVATIVEDTWSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MDDERIVATIVEDTWSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MDDERIVATIVEDTW_GLOBAL_H
