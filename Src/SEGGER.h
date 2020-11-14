#ifndef _SEGGER_DEBUG_H_
#define _SEGGER_DEBUG_H_

#include "SEGGER_RTT.h"

#define DEBUG 0

#if DEBUG

#define printf(fmt,...)					SEGGER_RTT_printf(0,fmt,##__VA_ARGS__)

#else
#define printF(fmt,...)
#endif

#endif
