/*
    Copyright (C) 1998 by Jorrit Tyberghein    

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
#include "isound/data.h"
#include "isound/loader.h"
#include "isys/system.h"
#include "isys/vfs.h"
#include "iutil/strvec.h"
#include "csutil/csvector.h"

#define MY_CLASSNAME "crystalspace.sound.loader.multiplexer"

class csSoundLoaderMultiplexer : public iSoundLoader {
private:
  csVector Loaders;

public:
  DECLARE_IBASE;

  // constructor
  csSoundLoaderMultiplexer(iBase *iParent);

  // destructor
  virtual ~csSoundLoaderMultiplexer();

  // Initialize the Sound Loader.
  virtual bool Initialize (iSystem *sys);

  // Load a sound file from the VFS.
  virtual iSoundData *LoadSound(void *Data, unsigned long Size) const;
};

IMPLEMENT_FACTORY(csSoundLoaderMultiplexer);

EXPORT_CLASS_TABLE (sndplex)
EXPORT_CLASS_DEP (csSoundLoaderMultiplexer,
  MY_CLASSNAME, "Sound Loader Multiplexer", "crystalspace.sound.loader.")
EXPORT_CLASS_TABLE_END;

IMPLEMENT_IBASE(csSoundLoaderMultiplexer)
  IMPLEMENTS_INTERFACE(iSoundLoader)
  IMPLEMENTS_INTERFACE(iPlugIn)
IMPLEMENT_IBASE_END;

csSoundLoaderMultiplexer::csSoundLoaderMultiplexer(iBase *iParent) {
  CONSTRUCT_IBASE(iParent);
}

csSoundLoaderMultiplexer::~csSoundLoaderMultiplexer() {
  for (long i=0; i<Loaders.Length(); i++) {
    iSoundLoader *ldr=(iSoundLoader*)(Loaders.Get(i));
    ldr->DecRef();
  }
}

bool csSoundLoaderMultiplexer::Initialize(iSystem *sys) 
{
  sys->Printf(MSG_INITIALIZATION,
    "Initializing sound loading multiplexer...\n"
    "  Looking for sound loader modules:\n");

  iStrVector* list = iSCF::SCF->QueryClassList ("crystalspace.sound.loader.");
  int const nmatches = list->Length();
  if (nmatches != 0)
  {
    for (int i = 0; i < nmatches; i++)
    {
      char const* classname = list->Get(i);
      if (strcasecmp (classname, MY_CLASSNAME))
      {
        sys->Printf(MSG_INITIALIZATION, "  %s\n", classname);
        iSoundLoader *ldr = LOAD_PLUGIN (sys, classname, 0, iSoundLoader);
        if (ldr)
	  Loaders.Push(ldr);
      }
    }
  }
  list->DecRef();
  return true;
}

iSoundData*
csSoundLoaderMultiplexer::LoadSound(void *Data, unsigned long Size) const
{
  for (long i=0;i<Loaders.Length();i++)
  {
    iSoundLoader *Ldr=(iSoundLoader*)(Loaders.Get(i));
    iSoundData *snd=Ldr->LoadSound(Data, Size);
    if (snd) return snd;
  }
  return NULL;
}
