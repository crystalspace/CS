/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef WIN32_H
#define WIN32_H

#include <windows.h>

#if defined(COMP_WCC) || defined(COMP_BC)
// The WATCOM C++ compiler does not accept a 'main' routine
// in a program which already contains WinMain. This is a 'fix'.
#define main csMain
#endif

#ifdef COMP_BC
// The Borland C++ equivalent of strcasecmp and strncasecmp are
// stricmp and strnicmp
#define strcasecmp(s1,s2)    stricmp(s1,s2)
#define strncasecmp(s1,s2,n) strnicmp(s1,s2,n)
#endif

//#define PixelAt(x,y) ...
#include "cscom/com.h"
#include "cssys/win32/win32itf.h"
#include <objbase.h>

#include "def.h"
#if defined(COMP_VC) || defined(COMP_WCC)
#include "csinput/csinput.h"
#include "igraph2D.h"
#endif

#include "cssys/common/system.h"

// Maximal path length
// Maximal path length
#ifndef MAXPATHLEN
#  ifdef _MAX_FNAME
#    define MAXPATHLEN _MAX_FNAME
#  else
#    define MAXPATHLEN 256
#  endif
#endif

// Directory read functions
#ifndef COMP_BC
#define __NEED_OPENDIR_PROTOTYPE

#include <io.h>

/// Directory entry
struct dirent
{
	char d_name [MAXPATHLEN + 1];	/* File name, 0 terminated */
	long d_size;				/* File size (bytes)       */
	unsigned d_attr;			/* File attributes (Windows-specific) */
};

/// Directory handle
struct DIR
{
  bool valid;
  long handle;
  dirent de;
  _finddata_t fd;
};

#define __NO_GENERIC_ISDIR
static inline bool isdir (char *path, dirent *de)
{
  (void)path;
  return !!(de->d_attr & _A_SUBDIR);
}
#endif

/// Windows version.
class SysSystemDriver : public csSystemDriver
{
public:
  SysSystemDriver ();
  
  virtual void Close ();
  virtual void Loop ();
  virtual void SetSystemDefaults ();
  virtual void Alert (const char* s);
  virtual void Warn (const char* s);

  /// Implementation of IWin32SystemDriver interface.
  class XWin32SystemDriver : public IWin32SystemDriver
  {
	/// Returns the HINSTANCE of the program
	STDMETHODIMP GetInstance(HINSTANCE* retval);
	/// Returns S_OK if the program is 'active', S_FALSE otherwise.
	STDMETHODIMP GetIsActive();
	/// Gets the nCmdShow of the WinMain().
	STDMETHODIMP GetCmdShow(int* retval);

	DECLARE_IUNKNOWN()
  };

  // COM stuff
  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(SysSystemDriver)  
  DECLARE_COMPOSITE_INTERFACE_EMBEDDED (Win32SystemDriver);
};

/// Windows version.
class SysKeyboardDriver : public csKeyboardDriver
{
public:
  SysKeyboardDriver();
  ~SysKeyboardDriver(void);
  
  virtual bool Open (csEventQueue *EvQueue);
  virtual void Close ();
  
};

/// Windows version.
class SysMouseDriver : public csMouseDriver
{
private:
  int MouseOpened;
  
public:
  SysMouseDriver();
  ~SysMouseDriver(void);
  
  virtual bool Open (csEventQueue *EvQueue);
  virtual void Close ();
  
  void Move(int *x, int *y, int *button);
};

#endif // WIN32_H

