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



#include "cssysdef.h"

#include "profiler.h"
#include "csutil/csstring.h"
#include "csutil/scf.h"
#include "ivaria/profile.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Profiler)
{
  using namespace CS::Debug;

  SCF_IMPLEMENT_FACTORY(ProfilerFactory)

  CS_IMPLEMENT_STATIC_VAR(GetGlobalProfiler, Profiler,)


  ProfilerFactory::ProfilerFactory (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }
  
  ProfilerFactory::~ProfilerFactory ()
  {
  }

  iProfiler* ProfilerFactory::GetProfiler ()
  {
    return GetGlobalProfiler ();
  }


  Profiler::Profiler ()
    : scfImplementationType (this)
  {

  }

  Profiler::~Profiler ()
  {

  }

  static int ZoneFindFun (ProfileZone* const& zone, csString const& name)
  {
    return csComparator<const char*, csString>::Compare (zone->zoneName, name);
  }

  static int ZoneCompareFun (ProfileZone* const& zone1, ProfileZone* const& zone2)
  {
    return csComparator<const char*, const char*>::Compare (zone1->zoneName, zone2->zoneName);
  }

  CS::Debug::ProfileZone* Profiler::GetProfileZone (const char* zonename)
  {
    ProfileZone* zone = 0;
    size_t index = allZones.FindKey (csArrayCmp<ProfileZone* , csString> (zonename, ZoneFindFun));

    if (index == csArrayItemNotFound)
    {
      //Allocate a new one
      zone = zoneAllocator.Alloc ();
      zone->zoneName = csStrNew (zonename);
      allZones.InsertSorted (zone, ZoneCompareFun);
    }
    else
    {
      zone = allZones[index];
    }

    return zone;
  }

  static int CounterFindFun (ProfileCounter* const& counter, csString const& name)
  {
    return csComparator<const char*, csString>::Compare (counter->counterName, name);
  }

  static int CounterCompareFun (ProfileCounter* const& counter1, ProfileCounter* const& counter2)
  {
    return csComparator<const char*, const char*>::Compare 
      (counter1->counterName, counter2->counterName);
  }

  CS::Debug::ProfileCounter* Profiler::GetProfileCounter (const char* countername)
  {
    ProfileCounter* counter = 0;
    size_t index = allCounters.FindKey (csArrayCmp<ProfileCounter* , csString> (countername, CounterFindFun));

    if (index == csArrayItemNotFound)
    {
      //Allocate a new one
      counter = counterAllocator.Alloc ();
      counter->counterName = csStrNew (countername);
      allCounters.InsertSorted (counter, CounterCompareFun);
    }
    else
    {
      counter = allCounters[index];
    }

    return counter;
  }

  void Profiler::Reset ()
  {
    for (size_t i = 0; i < allZones.GetSize (); ++i)
    {
      ProfileZone* zone = allZones[i];
      zone->totalTime = 0;
      zone->enterCount = 0;
    }

    for (size_t i = 0; i < allCounters.GetSize(); ++i)
    {
      ProfileCounter* counter = allCounters[i];
      counter->counterValue = 0;
    }
  }

  const csArray<CS::Debug::ProfileZone*>& Profiler::GetProfileZones ()
  {
    return allZones;
  }

  const csArray<CS::Debug::ProfileCounter*>& Profiler::GetProfileCounters ()
  {
    return allCounters;
  }
}
CS_PLUGIN_NAMESPACE_END(Profiler)
