/*
  Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_IVARIA_PROFILE_H__
#define __CS_IVARIA_PROFILE_H__

/**\file
 * Interface and helper macros for performance profiler
 */

#include "csutil/array.h"
#include "csutil/scf_interface.h"
#include "csutil/sysfunc.h"
#include "csutil/threading/atomicops.h"

#define CS_USE_PROFILER

namespace CS
{
namespace Debug
{
  class ProfileZone
  {
  public:
    // Methods
    ProfileZone ()
      : zoneName (0), parentZone (0), totalTime (0), enterCount (0)
    {}

    ~ProfileZone ()
    {
      delete[] zoneName;
    }

    // Data
    const char* zoneName;
    ProfileZone* parentZone;
    uint64 totalTime;
    uint32 enterCount;
  };


  class ProfileCounter
  {
  public:
    // Methods
    ProfileCounter ()
      : counterName (0), counterValue (0)
    {
    }

    ~ProfileCounter ()
    {
      delete[] counterName;
    }

    // Data
    const char* counterName;
    uint64 counterValue;
  };

  class ProfilerZoneScope
  {
  public:
    ProfilerZoneScope (ProfileZone* zone)
    {
      this->zone = zone;
      startTime = csGetMicroTicks ();
    }

    ~ProfilerZoneScope ()
    {
      int64 stopTime = csGetMicroTicks ();

      zone->enterCount++;
      zone->totalTime += (stopTime - startTime);
    }


  private:
    int64 startTime;
    ProfileZone* zone;
  };

  inline void ProfilerCounterAdd (ProfileCounter* counter)
  {
    counter->counterValue++;
  }
}
}


/**
 * Interface to profiler
 */
struct iProfiler : public virtual iBase
{
  SCF_INTERFACE (iProfiler, 3,0,0);
  
  /**
   * Get a pointer to a profiler zone.
   * Will register a new zone if it doesn't exist. The pointer is guaranteed to
   * be valid until the profiler object is destroyed.
   */
  virtual CS::Debug::ProfileZone* GetProfileZone (const char* zonename) = 0;

  /**
   * Get a pointer to a profiler counter.
   * Will register a new counter if it doesn't exist. The pointer is guaranteed to
   * be valid until the profiler object is destroyed.
   */
  virtual CS::Debug::ProfileCounter* GetProfileCounter (const char* countername) = 0;

  /**
   * Reset all zones and counters.
   */
  virtual void Reset () = 0;

  /**
   * Get all profiler zones.
   */
  virtual const csArray<CS::Debug::ProfileZone*>& GetProfileZones () = 0;
  
  /**
   * Get all profiler counters.
   */
  virtual const csArray<CS::Debug::ProfileCounter*>& GetProfileCounters () = 0;
};

/**
 * Interface to profile factory.
 */
struct iProfilerFactory : public virtual iBase
{
  SCF_INTERFACE (iProfilerFactory, 1,0,0);

  /**
   * Get a profiler object to use.
   */
  virtual iProfiler* GetProfiler () = 0;
};

#ifdef CS_USE_PROFILER
#define CS_DECLARE_PROFILER \
static iProfiler* CS_DEBUG_Profiler_staticProfilerPtr = 0; \
static inline iProfiler* CS_DEBUG_Profiler_GetProfiler () \
{ \
  if (!CS_DEBUG_Profiler_staticProfilerPtr)  \
  { \
    csRef<iProfilerFactory> fact = \
      scfCreateInstance<iProfilerFactory> ("crystalspace.utilities.profiler"); \
    CS_DEBUG_Profiler_staticProfilerPtr = fact->GetProfiler (); \
  } \
  CS_ASSERT (CS_DEBUG_Profiler_staticProfilerPtr); \
  return CS_DEBUG_Profiler_staticProfilerPtr; \
}
#define CS_DECLARE_PROFILER_ZONE(name) \
static CS::Debug::ProfileZone* CS_DEBUG_Profiler_staticProfileZone ## name = 0; \
static inline CS::Debug::ProfileZone* CS_DEBUG_Profiler_GetProfileZone ## name () \
{\
  if (!CS_DEBUG_Profiler_staticProfileZone ## name) \
  {\
    CS_DEBUG_Profiler_staticProfileZone ## name = CS_DEBUG_Profiler_GetProfiler ()->GetProfileZone (#name); \
  }\
  return CS_DEBUG_Profiler_staticProfileZone ## name; \
}
#define CS_DECLARE_PROFILER_COUNTER(name) \
static CS::Debug::ProfileCounter* CS_DEBUG_Profiler_staticProfileCounter ## name = 0; \
static inline CS::Debug::ProfileCounter* CS_DEBUG_Profiler_GetProfileCounter ## name () \
{\
  if (!CS_DEBUG_Profiler_staticProfileCounter ## name) \
  {\
    CS_DEBUG_Profiler_staticProfileCounter ## name = CS_DEBUG_Profiler_GetProfiler ()->GetProfileZone (#name); \
  }\
  return CS_DEBUG_Profiler_staticProfileCounter ## name; \
}
#define CS_PROFILER_GET_PROFILER \
  CS_DEBUG_Profiler_GetProfiler ()
#define CS_PROFILER_ZONE(name) \
CS::Debug::ProfilerZoneScope CS_DEBUG_Profiler_zone ## name ## __LINE__ \
  (CS_DEBUG_Profiler_GetProfileZone ## name());
#define CS_PROFILER_COUNTER(name) \
CS::Debug::ProfilerCounterAdd (CS_DEBUG_Profiler_GetProfileCounter ## name());
#else

#define CS_DECLARE_PROFILER 
#define CS_DECLARE_PROFILER_ZONE(name)
#define CS_DECLARE_PROFILER_COUNTER(name)
#define CS_PROFILER_GET_PROFILER (0)
#define CS_PROFILER_ZONE(name)
#define CS_PROFILER_COUNTER(name)

#endif


#endif