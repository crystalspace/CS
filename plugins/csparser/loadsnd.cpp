/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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

/* This file contains functions to load sounds from files. They do
 * not handle parsing of sound statements in any way.
 */

#include "cssysdef.h"

#include "cstool/vfsdirchange.h"
#include "csutil/stringquote.h"
#include "iengine/engine.h"
#include "isndsys/ss_loader.h"
#include "isndsys/ss_manager.h"
#include "isndsys/ss_data.h"
#include "isndsys/ss_stream.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#include "csthreadedloader.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  THREADED_CALLABLE_IMPL3(csThreadedLoader, LoadSoundSysData, const char* cwd, const char* filename,
    bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    if (!SndSysLoader)
    {
      return false;
    }

    // read the file data
    csRef<iDataBuffer> buf = vfs->ReadFile (filename);
    if (!buf || !buf->GetSize ())
    {
      ReportError (
        "crystalspace.maploader.parse.sound",
        "Cannot open sound file %s from VFS!", CS::Quote::Single (filename));
      return false;
    }

    // load the sound
    csRef<iSndSysData> Sound = SndSysLoader->LoadSound (buf, filename);

    // check for valid sound data
    if (!Sound)
    {
      ReportError (
        "crystalspace.maploader.parse.sound",
        "Cannot create sound data from file %s!", CS::Quote::Single (filename));
      return false;
    }

    ret->SetResult(csRef<iBase>(Sound));

    if(sync)
    {
      Engine->SyncEngineListsWait(this);
    }

    return true;
  }

  THREADED_CALLABLE_IMPL4(csThreadedLoader, LoadSoundStream, const char* cwd, const char* fname, int mode3d,
    bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    if (!SndSysRenderer)
    {
      return false;
    }

    csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
    if (!LoadSoundSysDataTC (itr, false, cwd, fname, do_verbose))
    {
      return false;
    }

    /* register the sound */
    csRef<iSndSysData> sound = scfQueryInterface<iSndSysData>(itr->GetResultRefPtr());
    csRef<iSndSysStream> stream = SndSysRenderer->CreateStream (sound, mode3d);
    if (!stream)
    {
      ReportError (
        "crystalspace.maploader.parse.sound",
        "Cannot register sound %s!", CS::Quote::Single (fname));
      return false;
    }

    ret->SetResult(csRef<iBase>(stream));

    if(sync)
    {
      Engine->SyncEngineListsWait(this);
    }

    return true;
  }

  THREADED_CALLABLE_IMPL4(csThreadedLoader, LoadSoundWrapper, const char* cwd, const char* name,
    const char* fname, bool do_verbose)
  {
    csVfsDirectoryChanger dirChange(vfs);
    dirChange.ChangeToFull(cwd);

    if (!SndSysManager)
    {
      return false;
    }

    // load the sound handle
    csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
    if (!LoadSoundSysDataTC (itr, false, vfs->GetCwd(), fname, do_verbose))
    {
      return false;
    }

    // build wrapper object
    iSndSysWrapper* wrapper = SndSysManager->CreateSound (name);
    csRef<iSndSysData> data = scfQueryInterface<iSndSysData>(itr->GetResultRefPtr());
    wrapper->SetData(data);
    ret->SetResult(csRef<iBase>(wrapper));

    if(sync)
    {
      Engine->SyncEngineListsWait(this);
    }

    return true;
  }
}
CS_PLUGIN_NAMESPACE_END(csparser)
