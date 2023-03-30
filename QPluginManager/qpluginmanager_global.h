#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(QPLUGINMANAGER_LIB)
#  define QPLUGINMANAGER_EXPORT Q_DECL_EXPORT
# else
#  define QPLUGINMANAGER_EXPORT Q_DECL_IMPORT
# endif
#else
# define QPLUGINMANAGER_EXPORT
#endif
