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
#include "isys/plugin.h"
#include "isys/system.h"
#include "isys/plugin.h"
#include "iutil/strvec.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "csutil/csvector.h"

#define MY_CLASSNAME "crystalspace.sound.loader.multiplexer"

CS_IMPLEMENT_PLUGIN

class csSoundLoaderMultiplexer : public iSoundLoader
{
private:
  csVector Loaders;

public:
  SCF_DECLARE_IBASE;

  // Constructor
  csSoundLoaderMultiplexer(iBase *iParent);

  // Destructor
  virtual ~csSoundLoaderMultiplexer();

  // Initialize the Sound Loader.
  virtual bool Initialize (iObjectRegistry *object_reg);

  // Load a sound file from the raw data.
  virtual iSoundData *LoadSound(void *Data, unsigned long Size) const;

  struct eiPlugin : public iPlugin
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundLoaderMultiplexer);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
    virtual bool HandleEvent (iEvent&) { return false; }
  } scfiPlugin;
};

SCF_IMPLEMENT_FACTORY(csSoundLoaderMultiplexer);

SCF_EXPORT_CLASS_TABLE (sndplex)
SCF_EXPORT_CLASS_DEP (csSoundLoaderMultiplexer,
  MY_CLASSNAME, "Sound Loader Multiplexer", "crystalspace.sound.loader.")
SCF_EXPORT_CLASS_TABLE_END;

SCF_IMPLEMENT_IBASE(csSoundLoaderMultiplexer)
  SCF_IMPLEMENTS_INTERFACE(iSoundLoader)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPlugin)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundLoaderMultiplexer::eiPlugin)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoundLoaderMultiplexer::csSoundLoaderMultiplexer(iBase *iParent)
{
  SCF_CONSTRUCT_IBASE(iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csSoundLoaderMultiplexer::~csSoundLoaderMultiplexer()
{
  for (long i=0; i<Loaders.Length(); i++)
  {
    iSoundLoader *ldr=(iSoundLoader*)(Loaders.Get(i));
    ldr->DecRef();
  }
}

bool csSoundLoaderMultiplexer::Initialize(iObjectRegistry *object_reg) 
{
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iReporter* reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  if (reporter)
    reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "crystalspace.sound.loader.mplex",
      "Initializing sound loading multiplexer...\n"
      "  Looking for sound loader modules:");

  iStrVector* list = iSCF::SCF->QueryClassList ("crystalspace.sound.loader.");
  int const nmatches = list->Length();
  if (nmatches != 0)
  {
	int i;
    for (i = 0; i < nmatches; i++)
    {
      char const* classname = list->Get(i);
      if (strcasecmp (classname, MY_CLASSNAME))
      {
	if (reporter)
          reporter->Report (CS_REPORTER_SEVERITY_NOTIFY,
      		"crystalspace.sound.loader.mplex",
	  	"  %s", classname);
        iSoundLoader *ldr = CS_LOAD_PLUGIN (plugin_mgr, classname, 0,
		iSoundLoader);
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
