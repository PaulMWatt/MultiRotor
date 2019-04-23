/// @file BLTypes.h
///
/// Common header file for all client software in order to
/// create a portable set of types that will reliably map to different
/// platforms.
/// 
/// There are two categories for types:
/// 1) Variable size: common use counters and general data.
/// 2) Guaranteed size: for use in protocols and data transfer.
/// 
/// Because of the variety of 32-bit and 64-bit machines and compilers,
/// it is necessary to explicitly define the size of variables in 
/// certain situations.  Where the size of the buffer is not crucial,
/// use a common use definition of sufficient size for the task.
///
/// @copyright Paul Watt 2009
//  ****************************************************************************
#ifndef BLTYPES_H_INCLUDED
#define BLTYPES_H_INCLUDED

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>

#else

typedef void *HANDLE;
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name

DECLARE_HANDLE(HMODULE);


// Define TCHAR macros
#undef TCHAR
#ifdef  _UNICODE
# define TCHAR               wchar_t
#else
# define TCHAR               char
#endif // _UNICODE

#endif // !_WIN32

// preferred types for maximum clarity and portability.

typedef long long           int64_t;
typedef unsigned long long  uint64_t;

typedef unsigned int        uint32_t;

typedef short               int16_t;
typedef unsigned short      uint16_t;

typedef unsigned char       byte_t;

// Base character type that can change based on default character size.
#ifdef _UNICODE
typedef wchar_t             tchar_t;
#else
typedef char                tchar_t;
#endif

// Macros to abstract the declaration of strings for wide-char vs multi-byte.
#undef __T
#ifdef  _UNICODE
#define __T(x)      L ## x
#else
#define __T(x)      x
#endif

#undef _T
#define _T(x)       __T(x)

#undef _TEXT
#define _TEXT(x)    __T(x)

#endif // BLTYPES_H_INCLUDED
