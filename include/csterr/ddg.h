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
#ifdef DDG
#include <iostream.h>
#else
#include "csgeom/math3d.h"
#define ddgVector3	csVector3
#endif

#ifdef OS_WIN32
// Windows defines
#if defined (COMP_VC) || defined (COMP_BC)
// incorrect GCC/G++ pragmas
#pragma warning (disable:4244)	// Disable bogus conversion warnings. 
#pragma warning (disable:4305)  // VC++ 5.0 version of above warning. 
#include "strstrea.h"
#include "stdlib.h"			// For exit()
#endif
//
#ifdef DDG
#define WEXP	__declspec(dllexport)
#define WFEXP	__cdecl
#endif
//
#else
// Linux defines
#ifdef DDG
#include <strstream.h> 
#endif
#include <stdlib.h>
#endif
// Define these if they are not yet defined
#ifndef WEXP
#define WEXP
#define WFEXP
#endif
// Various assert macros.
//
#ifndef DDG
#define ddgAssert(a)
#define ddgAsserts(a,b)
#define ddgMemorySet(a,b)
#define ddgErrorSet(a,b)
#define ddgSuccess false
#define ddgFailure true
#endif
#ifndef OS_WIN32
#ifndef OPTIM
#define _DEBUG
#endif
#endif

#endif
