/*
    Copyright (C) 1997, 1998, 1999, 2000 by Alex Pfaffe
	(Digital Dawn Graphics Inc)
  
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
#ifndef _ddg_Class_
#define _ddg_Class_

#include <math.h>
#include <stdlib.h>

#ifdef DDGSTREAM
#include <iostream.h>
#ifdef WIN32
#include <strstrea.h>
#else
#include <strstream.h>
#endif
#endif

#ifdef WIN32
// Windows defines
#if defined (COMP_VC) || defined (COMP_BC) || defined (DDG)
#pragma warning (disable:4244)	// Disable bogus conversion warnings. 
#pragma warning (disable:4305)  // VC++ 5.0 version of above warning. 
#pragma warning (disable:4097)	// typedef-name 'super' used as synonym for class-name 'ddgSubclass::ddgSuperClass'. 
#pragma warning (disable:4100)	// unreferenced formal parameter. 
#pragma warning (disable:4706)	// assignment within conditional expression 
#endif
//
#ifdef DDG
#define WEXP	__declspec(dllexport)
#define WFEXP	__cdecl
#endif // DDG
#endif // WIN32

#ifdef DDGGL
#if defined(_WIN32)

/* GLUT 3.7 now tries to avoid including <windows.h>
   to avoid name space pollution, but Win32's <GL/gl.h> 
   needs APIENTRY and WINGDIAPI defined properly. */
   /* XXX This is from Win32's <windef.h> */
#  ifndef APIENTRY
#   define GLUT_APIENTRY_DEFINED
#   if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#    define APIENTRY    __stdcall
#   else
#    define APIENTRY
#   endif
#  endif
   /* XXX This is from Win32's <winnt.h> */
#  ifndef CALLBACK
#   if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#    define CALLBACK __stdcall
#   else
#    define CALLBACK
#   endif
#  endif
   /* XXX This is from Win32's <wingdi.h> and <winnt.h> */
#  ifndef WINGDIAPI
#   define GLUT_WINGDIAPI_DEFINED
#   define WINGDIAPI __declspec(dllimport)
#  endif
   /* XXX This is from Win32's <ctype.h> */
#  ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#   define _WCHAR_T_DEFINED
#  endif
# endif

#pragma comment (lib, "winmm.lib")     /* link with Windows MultiMedia lib */
#pragma comment (lib, "opengl32.lib")  /* link with Microsoft OpenGL lib */
#pragma comment (lib, "glu32.lib")     /* link with OpenGL Utility lib */
#pragma comment (lib, "glut32.lib")    /* link with Win32 GLUT lib */

#pragma warning (disable:4244)	/* Disable bogus conversion warnings. */
#pragma warning (disable:4305)  /* VC++ 5.0 version of above warning. */

#include <gl/gl.h>
#include <gl/glu.h>
#endif

#ifdef __CRYSTAL_SPACE__
// A rare case where cssysdef.h is included from a header file. :-)
// By including cssysdef.h here, we avoid having to modify each of
// the DDG source files.  We want to avoid CrystalSpace-specific
// customization of those files since they are also part of the
// stand-alone DDG toolkit.
#ifndef __CS_CSSYSDEFS_H__
#include "cssysdef.h"
#endif
#include "cstypes.h"
#endif

// Define these if they are not yet defined
#ifndef WEXP
#define WEXP
#define WFEXP
#endif
//
#ifndef DDG
// Dummy classes
class ddgVector3;
#define _ddgPath_Class_

class ddgPath {
public:
	void add( ddgVector3 *, ddgVector3 *) {}
	void get( ddgVector3 *, ddgVector3 *, unsigned int) {}
	unsigned int frames(void) { return 0; }
};
#endif
#ifndef WIN32
#ifndef OPTIM
#define _DEBUG
#endif
#endif

#endif
