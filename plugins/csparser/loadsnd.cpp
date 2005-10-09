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
#include "cstool/sndwrap.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "iengine/engine.h"
#include "isound/data.h"
#include "isound/loader.h"
#include "isound/renderer.h"
#include "isound/handle.h"
#include "isndsys/ss_loader.h"
#include "isndsys/ss_manager.h"
#include "isndsys/ss_data.h"
#include "isndsys/ss_stream.h"

csPtr<iSoundData> csLoader::LoadSoundData (const char* filename)
{
  if (!VFS || !SoundLoader)
    return 0;

  // read the file data
  csRef<iDataBuffer> buf (VFS->ReadFile (filename));
  if (!buf || !buf->GetSize ())
  {
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot open sound file '%s' from VFS!", filename);
    return 0;
  }

  // load the sound
  csRef<iSoundData> Sound (
    SoundLoader->LoadSound (buf->GetUint8 (), buf->GetSize ()));

  // check for valid sound data
  if (!Sound)
  {
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot create sound data from file '%s'!", filename);
    return 0;
  }

  return csPtr<iSoundData> (Sound);
}

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
  csRef<iSndSysData> Sound = SndSysLoader->LoadSound (buf);

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

csPtr<iSoundHandle> csLoader::LoadSound (const char* filename)
{
  if (!SoundRender)
    return 0;

  csRef<iSoundData> Sound (LoadSoundData(filename));
  if (!Sound) return 0;

  /* register the sound */
  csRef<iSoundHandle> hdl (SoundRender->RegisterSound(Sound));
  if (!hdl)
  {
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot register sound '%s'!", filename);
    return 0;
  }

  return csPtr<iSoundHandle> (hdl);
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

csPtr<iSoundWrapper> csLoader::LoadSound (const char* name, const char* fname)
{
  // load the sound handle
  csRef<iSoundHandle> Sound (LoadSound(fname));
  if (!Sound) return 0;

  // build wrapper object
  iSoundWrapper* Wrapper = new csSoundWrapper (Sound);
  Wrapper->QueryObject ()->SetName (name);
  if (Engine) Engine->QueryObject ()->ObjAdd(Wrapper->QueryObject ());

  return csPtr<iSoundWrapper> (Wrapper);
}

iSndSysWrapper* csLoader::LoadSoundWrapper (const char* name, const char* fname,
	int mode3d)
{
  if (!SndSysManager) return 0;

  // load the sound handle
  csRef<iSndSysStream> Sound = LoadSoundStream (fname, mode3d);
  if (!Sound) return 0;

  // build wrapper object
  iSndSysWrapper* wrapper = SndSysManager->CreateSound (name);
  wrapper->SetStream (Sound);
  return wrapper;
}

