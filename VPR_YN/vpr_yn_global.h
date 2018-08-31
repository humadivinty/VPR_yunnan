#ifndef VPR_YN_GLOBAL_H
#define VPR_YN_GLOBAL_H

#include <QtCore/qglobal.h>

//extern "C"

//#    define Q_DECL_EXPORT     __declspec(dllexport)
//#    define Q_DECL_IMPORT     __declspec(dllimport)

#if defined(VPR_YN_LIBRARY)
#  define VPR_YNSHARED_EXPORT Q_DECL_EXPORT
//#  define VPR_YNSHARED_EXPORT __declspec(dllexport)
#else
#  define VPR_YNSHARED_EXPORT Q_DECL_IMPORT extern "C"
#endif

#endif // VPR_YN_GLOBAL_H
