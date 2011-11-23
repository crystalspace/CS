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
#include "csutil/platformfile.h"
#include "csutil/scf.h"
#include "iutil/vfs.h"
#include "csutil/objreg.h"
#include "ivaria/profile.h"
#include "cstool/vfsdirchange.h"



template<>
class csComparator<const char*, csString> :
  public csComparatorString<const char*> {};

CS_PLUGIN_NAMESPACE_BEGIN(Profiler)
{
  using namespace CS::Debug;

  SCF_IMPLEMENT_FACTORY(ProfilerFactory)

  CS_IMPLEMENT_STATIC_VAR(GetGlobalProfiler, Profiler, ())


  ProfilerFactory::ProfilerFactory (iBase* parent)
    : scfImplementationType (this, parent)
  {
    // Force us to be kept
    parent->IncRef ();
  }
  
  ProfilerFactory::~ProfilerFactory ()
  {
  }

  iProfiler* ProfilerFactory::GetProfiler ()
  {
    return GetGlobalProfiler ();
  }


  Profiler::Profiler ()
    : scfImplementationType (this), nativeLogfile (0),
    logfileNameHelper ("profile_log0000.csv"), isLogging (false)
  {    
  }

  Profiler::~Profiler ()
  {

  }

  static int ZoneFindFun (ProfileZone* const& zone, csString const& name)
  {
    return csComparator<const char*, csString>::Compare (zone->zoneName, name);
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
      allZones.Push (zone);
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

  CS::Debug::ProfileCounter* Profiler::GetProfileCounter (const char* countername)
  {
    ProfileCounter* counter = 0;
    size_t index = allCounters.FindKey (csArrayCmp<ProfileCounter* , csString> (countername, CounterFindFun));

    if (index == csArrayItemNotFound)
    {
      //Allocate a new one
      counter = counterAllocator.Alloc ();
      counter->counterName = csStrNew (countername);
      allCounters.Push (counter);
    }
    else
    {
      counter = allCounters[index];
    }

    return counter;
  }

  void Profiler::Reset ()
  {
    // Dump to file if we have one
    if (isLogging && allZones.GetSize () > 0)
    {
      csString data, data2;
      for (size_t i = 0; i < allZones.GetSize (); ++i)
      {
        data2.Format ("%" PRIu64 ", %u, ", allZones[i]->totalTime, allZones[i]->enterCount);
        data.Append (data2);
      }
      data.Append ("\n");
      WriteLogEntry (data);
    }

    // Reset
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

  void Profiler::StartLogging (const char* filenamebase, iObjectRegistry* objectreg)
  {
    // Get a vfs pointer
    csRef<iVFS> vfs;
    if (objectreg) 
      vfs = csQueryRegistry<iVFS> (objectreg);
    
    if (filenamebase && *filenamebase && strlen (filenamebase) > 0)
    {
      logfileNameHelper.SetMask (filenamebase);
    }
    else
    {
      logfileNameHelper.SetMask ("profilelog_0000.csv");
    }

    if (vfs)
    {
      csVfsDirectoryChanger dirCh (vfs);
      dirCh.ChangeTo ("/tmp");
      logfile = vfs->Open (logfileNameHelper.FindNextFilename (vfs), VFS_FILE_WRITE);

      if (logfile)
        isLogging = true;
    }
    {
      // Use native logging
      nativeLogfile = CS::Platform::File::Open (logfileNameHelper.FindNextFilename (), "w");
      if (nativeLogfile)
        isLogging = true;
    }


  }

  void Profiler::StopLogging ()
  {
    if (!isLogging)
      return;

    // Write column headers at the end. This isn't really csv format, but we do
    // this to cope with any added columns during profiling
    csString data, data2;
    for (size_t i = 0; i < allZones.GetSize (); ++i)
    {
      data2.Format ("%s_Time, %s_Count, ", allZones[i]->zoneName, allZones[i]->zoneName);
      data.Append (data2);
    }
    data.Append ("\n");
    WriteLogEntry (data);

    if (logfile)
    {
      logfile->Flush ();
      logfile = 0;
    }

    if (nativeLogfile)
    {
      fflush (nativeLogfile);
      fclose (nativeLogfile);
      nativeLogfile = 0;
    }

    isLogging = false;
  }

  void Profiler::WriteLogEntry (const csString& entry, bool flush /* = false */)
  {
    if (logfile)
    {
      logfile->Write (entry.GetDataSafe (), entry.Length ());

      if (flush)
        logfile->Flush ();
    }

    if (nativeLogfile)
    {
      fprintf (nativeLogfile, "%s", entry.GetDataSafe ());

      if (flush)
        fflush (nativeLogfile);
    }
  }
}
CS_PLUGIN_NAMESPACE_END(Profiler)
