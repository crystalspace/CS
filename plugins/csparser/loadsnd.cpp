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
#include "isys/vfs.h"
#include "iengine/engine.h"
#include "isound/data.h"
#include "isound/loader.h"
#include "isound/renderer.h"

iSoundData *csLoader::LoadSoundData(const char* filename) {
  if (!VFS || !SoundLoader)
    return NULL;

  // read the file data
  iDataBuffer *buf = VFS->ReadFile (filename);
  if (!buf || !buf->GetSize ()) {
    if (buf) buf->DecRef ();
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot open sound file '%s' from VFS!", filename);
    return NULL;
  }

  // load the sound
  iSoundData *Sound = SoundLoader->LoadSound(buf->GetUint8 (), buf->GetSize ());
  buf->DecRef ();

  // check for valid sound data
  if (!Sound)
  {
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot create sound data from file '%s'!", filename);
  }
  else
    Stats->sounds_loaded++;

  return Sound;
}

iSoundHandle *csLoader::LoadSound(const char* filename) {
  if (!SoundRender)
    return NULL;

  iSoundData *Sound = LoadSoundData(filename);
  if (!Sound) return NULL;

  /* register the sound */
  iSoundHandle *hdl = SoundRender->RegisterSound(Sound);
  if (!hdl)
  {
    ReportError (
	      "crystalspace.maploader.parse.sound",
	      "Cannot register sound '%s'!", filename);
  }

  return hdl;
}

iSoundWrapper *csLoader::LoadSound (const char* name, const char* fname) {
  // load the sound handle
  iSoundHandle *Sound = LoadSound(fname);
  if (!Sound) return NULL;

  // build wrapper object
  iSoundWrapper* Wrapper = &(new csSoundWrapper (Sound))->scfiSoundWrapper;
  Wrapper->QueryObject ()->SetName (name);
  if (Engine) Engine->QueryObject ()->ObjAdd(Wrapper->QueryObject ());
  Wrapper->DecRef ();

  return Wrapper;
}
