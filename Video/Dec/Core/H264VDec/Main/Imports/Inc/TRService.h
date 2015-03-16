#ifndef _TRService_H
#define _TRService_H

//#define TRSDK_VER 1009

#include "../../SDKConfigDef.h"

#if defined(TR_BUILD_TARGET)
#define TR_ENABLE_NEWMACROS
#endif

#if !defined(TRSDK_VER)
#include "TRService/OLD_LIBTR.h"
#elif TRSDK_VER >= 1000 && TRSDK_VER < 2000 && defined(TR_BUILD_TARGET)
#include "TRSDK_V1/TRSDK.h"
#elif TRSDK_VER >= 1000 && TRSDK_VER < 2000 && !defined(TR_BUILD_TARGET)
#include "TRSDK_V1/TROff/TRSDK.h"
#endif

#endif