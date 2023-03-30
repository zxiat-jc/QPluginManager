#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(QLOGPLUGINTEST_LIB)
#  define QLOGPLUGINTEST_EXPORT Q_DECL_EXPORT
# else
#  define QLOGPLUGINTEST_EXPORT Q_DECL_IMPORT
# endif
#else
# define QLOGPLUGINTEST_EXPORT
#endif
