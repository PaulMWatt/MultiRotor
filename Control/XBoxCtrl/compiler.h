/* compiler.h *****************************************************************
Author:		Paul Watt
Date:			10/22/2003
Purpose:	This file contains any macros that can be used to improve the 
					clarity of code or ease for debugging purposes.
******************************************************************************/
#ifndef __COMPILER_H
#define __COMPILER_H

/* CxxUnit Testing ***********************************************************/
// Change the mock namespace from T to cxx to reduce confusion with template semantics. 
#define CXXTEST_MOCK_NAMESPACE cxx

/* PRAGMA Define for use in MACROS *******************************************/
#ifdef _MSC_VER
# define DO_PRAGMA(x) __pragma(x)
#elif defined(__GCC__)
# define DO_PRAGMA(x) _Pragma(x)
#else
# define DO_PRAGMA(x)
#endif

/* Stringize Helper **********************************************************/
#define STR2_(x) #x
#define STR1_(x) STR2_(x)

#define WSTR2_(x) L ## x
#define WSTR1_(x) _T(x)

/* Notice And TODO Macro *****************************************************/
// The notice macro set will display a message in the compiler
// output window that you may click to take you immediately to 
// the line in the file where the message is located.
// 
// These messages will only be printed for DEBUG builds.
// 

#ifdef _DEBUG
// This message will be printed to the debug window.
//		NOTICE: This is a notice!
#define NOTICE(str) DO_PRAGMA(message(__FILE__"("STR1_(__LINE__)"): NOTICE: " str))

// This message will be printed to the debug window.
//		TODO: This is a notice!
#define TODO(str) DO_PRAGMA(message(__FILE__"("STR1_(__LINE__)"): TODO: " str))

#else
#define NOTICE(str)
#define TODO(str)
#endif

/* Determine Processor *******************************************************/
#define BL_CPU_ARM        1
#define BL_CPU_X86        2
// Definition to make comparisons simpler.  
// This value can increase as more l-endian CPUs are added.
#define BL_CPU_BIG_ENDIAN 3   

// Determine the processor type based on compiler flags.  This code has been 
// adapted from the POSH portable framework from: http://www.poshlib.org
// New definitions will be required for CPUs that are not defined here.
#if defined ARM || defined __arm__ || defined _ARM
#  define BL_CPU        BL_CPU_ARM
#  define BL_CPU_STRING "ARM"
#endif

#if defined __X86__ || defined __i386__ || defined i386 || defined _M_IX86 || defined __386__ || defined __x86_64__ || defined _M_X64
#  define BL_CPU        BL_CPU_X86
#  define BL_CPU_STRING "Intel 386+"
#endif

#ifndef BL_CPU
#  error BL_CPU not defined.  Please add definition
#endif

/* Determine Support for 64-bit **********************************************/



/* Determine Endianess *******************************************************/
#define BL_BIG_ENDIAN     1
#define BL_LITTLE_ENDIAN  2
// Attempt to infer endianess by comparing the processor type and/or OS.
// Some processors are bi-endian, such as MIPS, this may cause trouble in the future.
#if (BL_CPU < BL_CPU_BIG_ENDIAN) || defined(_WIN32)
#  define BL_ENDIANESS BL_LITTLE_ENDIAN
#  define ORDER_DCBA
#else
#  define BL_ENDIANESS BL_BIG_ENDIAN
#endif

/* Compiler Dependant Type Definitions ***************************************/
#if defined(_WIN32)
#define _64(x) x ## i64
 typedef __int64 int64;
 typedef unsigned __int64 uint64;

#elif defined(__GNUC__)
#define _64(x) x ## LL
 typedef long long int int64;
 typedef unsigned long long int uint64;

#else
#  error Requires definition of _64() for this compiler!
#endif

#ifdef __cplusplus

// Declare this macro at the top of the class definition to prevent
// the automatic creation of the default constructor.
#define NO_DEFAULT_CONSTRUCTOR(TypeName) \
  TypeName()        

// Declare this macro at the top of the class definition to prevent
// the automatic creation of the copy constructor and operator=.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#endif // __cplusplus

#endif //__COMPILER_H

/* Macros for conditional compilation on different platforms *****************/
#ifdef UNDER_CE
  // Removes severe warnings on CE
# ifndef _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA
#   define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA
# endif

  // Set a windows version to remove certain warnings.
# ifndef WINVER
#   define WINVER 0x0400
# endif
  
# undef _WIN32_WINDOWS
# undef _WIN32_WINNT

# ifdef WIN32_PLATFORM_PSPC
#   ifndef SHELL_AYGSHELL
#     define SHELL_AYGSHELL
#   endif
# endif

#elif defined(_WIN32)
  // Set windows XP as the default version if one is not yet defined.
# ifndef WINVER
#   define WINVER 0x0501
# endif
  
# ifndef UNDER_NT
#   define UNDER_NT
# endif
  
#elif defined(linux)
# ifndef UNDER_LINUX
#   define UNDER_LINUX
# endif

#elif defined(apple_ios)
# ifndef UNDER_APPLE_IOS
#   define UNDER_APPLE_IOS
# endif

#else
# error Must add definition to indicate platform.
#endif


#if defined(__cplusplus)

/* STL Definitions ***********************************************************/
#ifdef _WIN32
# define BOOST_NO_STD_TYPEINFO
#endif

//   Add definitions to disable exception handling in the STL 
//   implementations on CE.
#ifdef UNDER_CE
  #define BOOST_NO_EXCEPTION_STD_NAMESPACE
	#if _WIN32_WCE < 0x500 && _MSC_VER > 1220 
	// only needed for WM2003 builds under VS2005 
	#pragma comment(lib, "ccrtrtti.lib") 
	#else

	#ifdef _HAS_EXCEPTIONS
		#undef _HAS_EXCEPTIONS
	#endif /* _HAS_EXCEPTIONS */

	#define _HAS_EXCEPTIONS  0
	//   These extra inclusions and redefinitions need to occur
	//   for eVC4.0 and CE.Net4.2.
	#if (_WIN32_WCE <= 0x420)

		// These macros are defined in here:
		//#define _TRY_BEGIN	try {
		//#define _CATCH(x)	} catch (x) {
		//#define _CATCH_ALL	} catch (...) {
		//#define _CATCH_END	}
		//#define _RAISE(x)	throw (x)
		//#define _RERAISE	throw
		#include <xstddef>

		// Undefine them, and redefine with macros that will do nothing.
		#ifdef _TRY_BEGIN
		#undef _TRY_BEGIN
		#endif

		#define _TRY_BEGIN {

		#ifdef _CATCH_ALL
		#undef _CATCH_ALL
		#endif

		#define _CATCH_ALL } if(0) {

		#ifdef _RAISE
		#undef _RAISE
		#endif

		#define _RAISE(x)	(x)

		#ifdef _RERAISE
		#undef _RERAISE
		#endif

		#define _RERAISE

		#ifdef _CATCH_END
		#undef _CATCH_END
		#endif

		#define _CATCH_END }

	#endif

	#endif
#endif

/* Compiler Dependent Settings ***********************************************/

// Stops the compiler from generating code to initialize the vfptr in 
// the constructor(s) and destructor of the class.  Only use on pure interfaces.
#ifdef _WIN32
# define BL_NO_VTABLE __declspec(novtable)
#else
# define BL_NO_VTABLE 
#endif

#ifdef _WIN32
// Disable windows definition of the min/max macros in favor of std templates.
# define NOMINMAX
#ifdef min
# undef min
#endif

#ifdef max
# undef max
#endif

# include <algorithm>
using std::min;
using std::max;
#endif

// Create a definition for __stdcall calling convention.
#ifndef _WIN32
# define __stdcall
#endif

#include "BLTypes.h"
#include "BLAssert.h"


#endif //defined(__cplusplus)