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
#include "csloader.h"
#include "csthreadedloader.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "iengine/engine.h"
#include "isndsys/ss_loader.h"
#include "isndsys/ss_manager.h"
#include "isndsys/ss_data.h"
#include "isndsys/ss_stream.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{
  THREADED_CALLABLE_IMPL2(csThreadedLoader, LoadSoundSysData, const char* filename,
    bool do_verbose)
  {
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
        "Cannot open sound file '%s' from VFS!", filename);
      return false;
    }

    // load the sound
    csRef<iSndSysData> Sound = SndSysLoader->LoadSound (buf, filename);

    // check for valid sound data
    if (!Sound)
    {
      ReportError (
        "crystalspace.maploader.parse.sound",
        "Cannot create sound data from file '%s'!", filename);
      return false;
    }

    ret->SetResult(csRef<iBase>(Sound));
    return true;
  }

  THREADED_CALLABLE_IMPL3(csThreadedLoader, LoadSoundStream, const char* fname, int mode3d,
    bool do_verbose)
  {
    if (!SndSysRenderer)
    {
      return false;
    }

    csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
    if (!LoadSoundSysDataTC (itr, fname, do_verbose))
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
        "Cannot register sound '%s'!", fname);
      return false;
    }

    ret->SetResult(csRef<iBase>(stream));
    return true;
  }

  THREADED_CALLABLE_IMPL3(csThreadedLoader, LoadSoundWrapper, const char* name,
    const char* fname, bool do_verbose)
  {
    if (!SndSysManager)
    {
      return false;
    }

    // load the sound handle
    csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
    if (!LoadSoundSysDataTC (itr, fname, do_verbose))
    {
      return false;
    }

    // build wrapper object
    iSndSysWrapper* wrapper = SndSysManager->CreateSound (name);
    csRef<iSndSysData> data = scfQueryInterface<iSndSysData>(itr->GetResultRefPtr());
    wrapper->SetData(data);
    ret->SetResult(csRef<iBase>(wrapper));
    return true;
  }

  // ----------------------------------------------------------------- //

csPtr<iSndSysData> csLoader::LoadSoundSysData (const char* filename)
{
  if (!VFS || !SndSysLoader)
    return 0;

  // read the file data
  csRef<iDataBuffer> buf = VFS->ReadFile (filename);
  if (!buf || !buf->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot open sound file '%s' from VFS!", filename);
    return 0;
  }

  // load the sound
  csRef<iSndSysData> Sound = SndSysLoader->LoadSound (buf, filename);

  // check for valid sound data
  if (!Sound)
  {
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot create sound data from file '%s'!", filename);
    return 0;
  }

  return csPtr<iSndSysData> (Sound);
}

csPtr<iSndSysStream> csLoader::LoadSoundStream (const char* filename,
	int mode3d)
{
  if (!SndSysRender)
    return 0;

  csRef<iSndSysData> Sound = LoadSoundSysData (filename);
  if (!Sound) return 0;

  /* register the sound */
  csRef<iSndSysStream> stream = SndSysRender->CreateStream (Sound, mode3d);
  if (!stream)
  {
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot register sound '%s'!", filename);
    return 0;
  }

  return csPtr<iSndSysStream> (stream);
}

iSndSysWrapper* csLoader::LoadSoundWrapper (const char* name, const char* fname)
{
  if (!SndSysManager) return 0;

  // load the sound handle
  csRef<iSndSysData> data = LoadSoundSysData (fname);
  if (!data) return 0;

  // build wrapper object
  iSndSysWrapper* wrapper = SndSysManager->CreateSound (name);
  wrapper->SetData (data);
  return wrapper;
}

}
CS_PLUGIN_NAMESPACE_END(csparser)
