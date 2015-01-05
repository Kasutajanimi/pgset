// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//prevent error_code redefinition
#define _ERRCODE_DEFINED 
typedef int errno_t;


#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// 32-64 bit detection
// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENV64
#else
#define ENV32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENV64
#else
#define ENV32
#endif
#endif



extern "C" {

#define PG_PORT_H

#include <postgres.h>
#include <utils/array.h>
#include <catalog/pg_type.h>

#include <fmgr.h>
#include <libpq/pqformat.h>		/* needed for send/recv functions */

}
