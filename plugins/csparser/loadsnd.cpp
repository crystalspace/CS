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

csPtr<iSoundData> csLoader::LoadSoundData(const char* filename)
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
  }

  return csPtr<iSoundData> (Sound);
}

csPtr<iSoundHandle> csLoader::LoadSound(const char* filename)
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
  }

  return csPtr<iSoundHandle> (hdl);
}

csPtr<iSoundWrapper> csLoader::LoadSound (const char* name, const char* fname)
{
  // load the sound handle
  csRef<iSoundHandle> Sound (LoadSound(fname));
  if (!Sound) return 0;

  // build wrapper object
  iSoundWrapper* Wrapper = &(new csSoundWrapper (Sound))->scfiSoundWrapper;
  Wrapper->QueryObject ()->SetName (name);
  if (Engine) Engine->QueryObject ()->ObjAdd(Wrapper->QueryObject ());

  return csPtr<iSoundWrapper> (Wrapper);
}

