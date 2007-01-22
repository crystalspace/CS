/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_UTIL_PROFILE_H__
#define __CS_UTIL_PROFILE_H__

/**\file
 * Profiling utilities.
 */

#include "csextern.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/scf_implementation.h"
#include "ivaria/profile.h"

struct CS_DEPRECATED_TYPE_MSG("Old profiling discontinued; check docs for new API")
csProfileInfo
{
  const char* token;
  const char* file;
  int line;
  uint32* ptr_count;
  uint32* ptr_time;
  uint32* ptr_timemin;
  uint32* ptr_timemax;
};

class CS_DEPRECATED_TYPE_MSG("Old profiling discontinued; check docs for new API")
csProfiler : public scfImplementation0<csProfiler>
{
public:
  csArray<csProfileInfo> profile_info;
  csArray<CS::Debug::ProfileZone*> profile_zones;
  csArray<CS::Debug::ProfileCounter*> profile_counters;

public:
  csProfiler () : scfImplementationType (this) {}
  virtual ~csProfiler () {}

  // Dummies to keep class compiling
  void Reset () {}
  CS::Debug::ProfileZone* GetProfileZone (const char* zonename)
  { return 0; }
  CS::Debug::ProfileCounter* GetProfileCounter (const char* countername)
  { return 0; }
  const csArray<CS::Debug::ProfileZone*>& GetProfileZones ()
  { return profile_zones; }
  const csArray<CS::Debug::ProfileCounter*>& GetProfileCounters ()
  { return profile_counters; }
};

namespace CS
{
  namespace Macros
  {
    CS_DEPRECATED_TYPE_MSG("Old profiling discontinued; check docs for new API")
    inline void CS_PROFTIME() {}
    CS_DEPRECATED_TYPE_MSG("Old profiling discontinued; check docs for new API")
    inline void CS_PROFRESET() {}
    CS_DEPRECATED_TYPE_MSG("Old profiling discontinued; check docs for new API")
    inline void CS_PROFDUMP() {}
    CS_DEPRECATED_TYPE_MSG("Old profiling discontinued; check docs for new API")
    inline void CS_PROFSTART() {}
    CS_DEPRECATED_TYPE_MSG("Old profiling discontinued; check docs for new API")
    inline void CS_PROFSTOP() {}
  } // namespace Macros
} // namespace CS

#define CS_PROFTIME(v) CS::Macros::CS_PROFTIME(); v = 0
#define CS_PROFRESET(a) CS::Macros::CS_PROFRESET()
#define CS_PROFDUMP(a) CS::Macros::CS_PROFDUMP()
#define CS_PROFSTART(a,b) CS::Macros::CS_PROFSTART()
#define CS_PROFSTOP(a) CS::Macros::CS_PROFSTOP()

#endif //__CS_UTIL_PROFILE_H__

