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
//#include <iostream.h> 

#ifdef WIN32
// Windows defines
#pragma warning (disable:4244)	// Disable bogus conversion warnings. 
#pragma warning (disable:4305)  // VC++ 5.0 version of above warning. 
#include "strstrea.h"
#include "stdlib.h"			// For exit()

//I undefined the following defines, because they cause every application, 
//that gets linked against the cspace lib to export all the ddg symbols.
//you probably only need these, if you intend to make the ddg stuff
//into its own dll.

//#define WEXP	__declspec(dllexport) //Only needed for a standalone version...
//#define WFEXP	__cdecl               //Only needed for a standalone version...  
#define WEXP	
#define WFEXP	

#else
// Linux defines
//#include <strstream.h> 
#include <stdlib.h>

#define WEXP
#define WFEXP
#endif
// Various assert macros.
//
#define ddgAssert(a)
#define ddgAsserts(a,b)
#define ddgMemorySet(a,b)
#ifndef WIN32
#ifndef OPTIM
#define _DEBUG
#endif
#endif

#endif
