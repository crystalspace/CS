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


#include "csutil/blockallocator.h"
#include "csutil/array.h"
#include "ivaria/profile.h"
#include "csutil/scf_implementation.h"
#include "cstool/numberedfilenamehelper.h"
#include "csutil/csstring.h"

struct iFile;

CS_PLUGIN_NAMESPACE_BEGIN(Profiler)
{
  class ProfilerFactory : 
    public scfImplementation1<ProfilerFactory, iProfilerFactory>
  {
  public:
    ProfilerFactory (iBase* parent);

    virtual ~ProfilerFactory ();

    iProfiler* GetProfiler ();
  };


  class Profiler : 
    public scfImplementation1<Profiler, iProfiler>
  {
  public:
    Profiler ();
    ~Profiler ();

    CS::Debug::ProfileZone* GetProfileZone (const char* zonename);

    CS::Debug::ProfileCounter* GetProfileCounter (const char* countername);

    void Reset ();

    const csArray<CS::Debug::ProfileZone*>& GetProfileZones ();

    const csArray<CS::Debug::ProfileCounter*>& GetProfileCounters ();

    void StartLogging (const char* filenamebase, iObjectRegistry* objectreg);
    void StopLogging ();

  private:
    csArray<CS::Debug::ProfileZone*> allZones;
    csArray<CS::Debug::ProfileCounter*> allCounters;

    csBlockAllocator<CS::Debug::ProfileZone> zoneAllocator;
    csBlockAllocator<CS::Debug::ProfileCounter> counterAllocator;

    // Logging related
    csRef<iFile> logfile;
    FILE* nativeLogfile;
    CS::NumberedFilenameHelper logfileNameHelper;
    bool isLogging;

    // Helper function to write a string to the logfile (independent of type)
    void WriteLogEntry (const csString& entry, bool flush = false);
  };

}
CS_PLUGIN_NAMESPACE_END(Profiler)
