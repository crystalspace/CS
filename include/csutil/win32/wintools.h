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

#include "csutil/csunicode.h"
#include "csutil/util.h"
#include <winnls.h> // contains MultiByteToWideChar()/WideCharToMultiByte()

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

struct cswinWtoA
{
private:
  char* s;
public:
  cswinWtoA (const wchar_t* ws)
  { s = cswinWideToAnsi (ws); }
  ~cswinWtoA ()
  { delete[] s; }
  operator const char* () const
  { return s; }
};

/// Convert UTF-8 to ANSI
struct cswinCtoA
{
private:
  char* s;
public:
  cswinCtoA (const char* ws, UINT codePage = CP_ACP)
  { 
    s = cswinWideToAnsi (csCtoW (ws), codePage); 
  }
  ~cswinCtoA ()
  { delete[] s; }
  operator const char* () const
  { return s; }
};

extern char* cswinGetErrorMessage (HRESULT code);
extern wchar_t* cswinGetErrorMessageW (HRESULT code);
extern bool cswinIsWinNT ();

#endif
