/*
    Copyright (C) 1997, 1998, 1999 by Alex Pfaffe
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

#ifdef DDGSTREAM
#include <iostream.h>
#endif

#ifdef WIN32
// Windows defines
#if defined (COMP_VC) || defined (COMP_BC)
#pragma warning (disable:4244)	// Disable bogus conversion warnings. 
#pragma warning (disable:4305)  // VC++ 5.0 version of above warning. 
#pragma warning (disable:4097)	// typedef-name 'super' used as synonym for class-name 'ddgSubclass::ddgSuperClass'. 
#pragma warning (disable:4100)	// unreferenced formal parameter. 
#pragma warning (disable:4706)	// assignment within conditional expression 
#include "strstrea.h"
#include "stdlib.h"			// For exit() COMP_VC/COMP_BC
#endif
//
#ifdef DDG
#define WEXP	__declspec(dllexport)
#define WFEXP	__cdecl
#endif
//
#if defined (COMP_GCC)
#include <stdlib.h>			// For exit() COMP_GCC
#endif
//
#else
// Linux defines
#ifdef DDGSTREAM
#include <strstream.h> 
#endif
#include <stdlib.h>
#endif

#ifdef __CRYSTAL_SPACE__
// A rare case where cssysdef.h is included from a header file :-)
#include "cssysdef.h"
#include "cstypes.h"
#endif

// Define these if they are not yet defined
#ifndef WEXP
#define WEXP
#define WFEXP
#endif
// Various assert macros.
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
