/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_UTIL_REFTRACKERACCESS_H__
#define __CS_UTIL_REFTRACKERACCESS_H__

#include "csextern.h"

#ifndef CS_REF_TRACKER
  // @@@ HACK: to allow enabled and disabled versions to coexist
  #define csRefTrackerAccess	csRefTrackerAccess_nada
  #define CS_CSUTIL_REFTRACK_EXPORT
#else
  #define CS_CSUTIL_REFTRACK_EXPORT	CS_CRYSTALSPACE_EXPORT
#endif

/**
 * Helper to facilitate access to the global reference tracker. See the 
 * iRefTracker documentation for an explanation of the available 
 * methods.
 */
class CS_CSUTIL_REFTRACK_EXPORT csRefTrackerAccess
{
public:
#ifndef CS_REF_TRACKER
  static void TrackIncRef (void*, int) {}
  static void TrackDecRef (void*, int) {}
  static void TrackConstruction (void*) {}
  static void TrackDestruction (void*, int) {}

  static void MatchIncRef (void*, int, void*) {}
  static void MatchDecRef (void*, int, void*) {}

  static void AddAlias (void*, void*) {}
  static void RemoveAlias (void*, void*) {}

  static void SetDescription (void*, const char*) {}
#else
  static void TrackIncRef (void* object, int refCount);
  static void TrackDecRef (void* object, int refCount);
  static void TrackConstruction (void* object);
  static void TrackDestruction (void* object, int refCount);

  static void MatchIncRef (void* object, int refCount, void* tag);
  static void MatchDecRef (void* object, int refCount, void* tag);

  static void AddAlias (void* obj, void* mapTo);
  static void RemoveAlias (void* obj, void* mapTo);

  static void SetDescription (void* obj, const char* description);
#endif
};

#endif // __CS_UTIL_REFTRACKERACCESS_H__
