/*
    Copyright (C) 2003 by Frank Richter

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSSYS_WIN32_WINTOOLS_H__
#define __CS_CSSYS_WIN32_WINTOOLS_H__

/**\file
 * Win32 tool functions
 */

#include "csextern.h"
#include "csutil/csunicode.h"
#include "csutil/util.h"
#include <winnls.h> // contains MultiByteToWideChar()/WideCharToMultiByte()
#include "sanity.inc"

/**
 * Convert an ANSI string to a wide string.
 * \remarks Free the returned pointer with delete[].
 * \remarks This function provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the function and inclusion of the 
 *  header file should be surrounded by appropriate `#if defined(CS_PLATFORM_WIN32) ... 
 *  #endif' statements.
 */
static inline wchar_t* cswinAnsiToWide (const char* ansi, 
					 UINT codePage = CP_ACP)
{
  int bufsize;
  WCHAR* buf;

  bufsize = MultiByteToWideChar (codePage,
    MB_PRECOMPOSED, ansi, -1, 0, 0);
  buf = new WCHAR[bufsize];
  MultiByteToWideChar (codePage,
    MB_PRECOMPOSED, ansi, -1, buf, bufsize);
  
  return buf;
}

/**
 * Convert a wide string to an ANSI string.
 * \remarks Free the returned pointer with delete[].
 * \remarks This function provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the function and inclusion of the 
 *  header file should be surrounded by appropriate `#if defined(CS_PLATFORM_WIN32) ... 
 *  #endif' statements.
 */
static inline char* cswinWideToAnsi (const wchar_t* wide, 
				     UINT codePage = CP_ACP)
{
  int bufsize;
  char* buf;

  bufsize = WideCharToMultiByte (codePage,
    WC_COMPOSITECHECK, wide, -1, 0, 0, 0, 0);
  buf = new char[bufsize];
  WideCharToMultiByte (codePage,
    WC_COMPOSITECHECK, wide, -1, buf, bufsize, 0, 0);
  
  return buf;
}

/**
 * Small helper to convert a wide to an ANSI string, useful when passing
 * arguments to a function.
 * \code
 *  wchar_t test[] = L"Foo";
 *  SomeFunctionA (cswinWtoA (test));
 * \endcode
 * \remarks This class provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the class and inclusion of the 
 *  header file should be surrounded by appropriate `#if defined(CS_PLATFORM_WIN32) ... 
 *  #endif' statements.
 */
struct CS_CRYSTALSPACE_EXPORT cswinWtoA
{
private:
  char* s;
public:
  /// Construct from a wchar_t string.
  cswinWtoA (const wchar_t* ws)
  { s = cswinWideToAnsi (ws); }
  /// Destruct, free up memory.
  ~cswinWtoA ()
  { delete[] s; }
  /// Return the string passed in on construction as an ANSI string.
  operator const char* () const
  { return s; }
};

/**
 * Small helper to convert an UTF-8 to an ANSI string, useful when passing
 * arguments to a function.
 * \remarks This class provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the class and inclusion of the 
 *  header file should be surrounded by appropriate `#if defined(CS_PLATFORM_WIN32) ... 
 *  #endif' statements.
 */
struct CS_CRYSTALSPACE_EXPORT cswinCtoA
{
private:
  char* s;
public:
  /// Construct from an ANSI string.
  cswinCtoA (const char* ws, UINT codePage = CP_ACP)
  { 
    s = cswinWideToAnsi (csCtoW (ws), codePage); 
  }
  /// Destruct, free up memory.
  ~cswinCtoA ()
  { delete[] s; }
  /// Return the string passed in on construction as an ANSI string.
  operator const char* () const
  { return s; }
};

/**
 * Retrieve the system's description for an error code.
 * \param code The error code, usually retrieved through GetLastError().
 * \remarks Returns an UTF-8 encoded string.
 * \remarks Free the returned pointer with delete[].
 * \remarks This function provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the function and inclusion of the 
 *  header file should be surrounded by appropriate `#if defined(CS_PLATFORM_WIN32) ... 
 *  #endif' statements.
 */
extern CS_CRYSTALSPACE_EXPORT char* cswinGetErrorMessage (HRESULT code);
/**
 * Retrieve the system's description for an error code.
 * \param code The error code, usually retrieved through GetLastError().
 * \remarks Free the returned pointer with delete[].
 * \remarks This function provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the function and inclusion of the 
 *  header file should be surrounded by appropriate `#if defined(CS_PLATFORM_WIN32) ... 
 *  #endif' statements.
 */
extern CS_CRYSTALSPACE_EXPORT wchar_t* cswinGetErrorMessageW (HRESULT code);

/// Windows versions cswinIsWinNT() can identify.
enum cswinWindowsVersion
{
  /// Windows 95/98/ME
  cswinWin9x = 30,
  /// Windows NT 4.0
  cswinWinNT = 40,
  /// Windows 2000
  cswinWin2K = 50,
  /// Windows XP (or better)
  cswinWinXP = 51
};

/**
 * Returns 'true' if the current Windows is from the NT strain, 'false' if
 * from the 9x strain.
 * \param version Optionally returns more specifically what Windows is used.
 * \return Whether the current OS is an Windows NT derivate.
 * \remarks This function provides functionality specific to the Win32 
 *  platform. To ensure that code using this functionality compiles properly 
 *  on all other platforms, the use of the function and inclusion of the 
 *  header file should be surrounded by appropriate `#if defined(CS_PLATFORM_WIN32) ... 
 *  #endif' statements.
 */
extern CS_CRYSTALSPACE_EXPORT bool cswinIsWinNT (cswinWindowsVersion* version = 0);

#endif // __CS_CSSYS_WIN32_WINTOOLS_H__
