/*
  Copyright (C) 2006 by Jorrit Tyberghein
            (C) 2006 by Frank Richter

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

#ifndef __CS_CSUTIL_DLMALLOC_SETTINGS_H__
#define __CS_CSUTIL_DLMALLOC_SETTINGS_H__

#define MSPACES 1
#define ONLY_MSPACES 1
#define USE_LOCKS 0
/*#define FOOTERS 1*/
/* Footers...
 * Pro: You can free memory allocated from other modules; if libcrystalspace
 *      is a static library, malloc()/free() behave more like it would be a 
 *      dynamic lib.
 * Con: Overhead.
 */

#ifdef _MSC_VER
/* silence "conversion from 'size_t' to 'bindex_t', possible loss of data" */
#pragma warning(disable:4267)
/* We want speed here */
#pragma optimize("gty", on)
#endif

#ifdef __CYGWIN__
/* Cygwin has funny issues with atexit() that ptmalloc seems to tickle.
 * So within ptmalloc we use own own single-use implementation of atexit()
 * when on Cygwin.  See cs_atexit in libs/csutil/ptmalloc_wrap.cpp.
 */
extern int cs_atexit(void(*func)(void));

#define atexit cs_atexit
#endif

#endif // __CS_CSUTIL_DLMALLOC_SETTINGS_H__
