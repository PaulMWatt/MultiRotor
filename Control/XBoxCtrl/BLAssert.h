/* BLAssert.h *****************************************************************
Copyright:  2009 Paul Watt
Author:     Paul Watt
Date:       4/25/2009
Purpose:    Assertion and verification macros designed for portibility
            and flexility.

            For now the assertion macros will default to acceptable MACRO
            definitions for each platform.  In the future, the MACRO will be
            augmented with features that incorporate debug and testing methods.
******************************************************************************/
#ifndef BLASSERT_H_INCLUDED
#define BLASSERT_H_INCLUDED

#ifdef _WIN32
//#include <atldef.h>
//#define BLASSERT(expr)  ATLASSERT(expr)
//#define BLASSERTMSG(expr, msg)  ATLASSERT(((msg), (expr)))
//#else
#include <assert.h>
#define BLASSERT(expr)  assert(expr)
#define BLASSERTMSG(expr, msg)  assert(expr)
#endif

#endif
