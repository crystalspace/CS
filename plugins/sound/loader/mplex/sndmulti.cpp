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
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/stringarray.h"
#include "csutil/refarr.h"
#include "csutil/util.h"
#include "ivaria/reporter.h"

#define SNDPLEX_CLASSNAME "crystalspace.sound.loader.multiplexer"

CS_IMPLEMENT_PLUGIN

class csSoundLoaderMultiplexer : public iSoundLoader
{
private:
  csRefArray<iSoundLoader> Loaders;
  csRef<iStringArray> list;
  csRef<iPluginManager> plugin_mgr;

  bool LoadNextPlugin ();
public:
  SCF_DECLARE_IBASE;

  // Constructor
  csSoundLoaderMultiplexer(iBase *iParent);

  // Destructor
  virtual ~csSoundLoaderMultiplexer();

  // Initialize the Sound Loader.
  virtual bool Initialize (iObjectRegistry *object_reg);

  // Load a sound file from the raw data.
  virtual csPtr<iSoundData> LoadSound(void *Data, uint32 Size);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSoundLoaderMultiplexer);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
};

SCF_IMPLEMENT_FACTORY(csSoundLoaderMultiplexer);


SCF_IMPLEMENT_IBASE(csSoundLoaderMultiplexer)
  SCF_IMPLEMENTS_INTERFACE(iSoundLoader)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoundLoaderMultiplexer::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoundLoaderMultiplexer::csSoundLoaderMultiplexer(iBase *iParent)
{
  SCF_CONSTRUCT_IBASE(iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSoundLoaderMultiplexer::~csSoundLoaderMultiplexer()
{
  plugin_mgr = 0;
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csSoundLoaderMultiplexer::Initialize(iObjectRegistry *object_reg)
{
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  // grab the sound loader list
  list = iSCF::SCF->QueryClassList ("crystalspace.sound.loader.");
  if (list.IsValid())
  {
    size_t i = 0;
    while (i < list->Length())
    {
      char const* classname = list->Get(i);
      if (!strcasecmp (classname, SNDPLEX_CLASSNAME))
      {
	list->DeleteIndex (i);
      }
      else if (strstr (classname, "mp3") && (i < (list->Length() - 1)))
      {
	// ok the following is a bit hacky, but since the mp3 loader skips junk
	// until it finds something useful chances are high it finds some
	// "good" header the bigger the input gets, so we give all other
	// loaders a chance to look at it first
	list->Push (csStrNew (classname));
	list->DeleteIndex (i);
      }
      else
	i++;
    }
  }
  return true;
}

bool csSoundLoaderMultiplexer::LoadNextPlugin ()
{
  if (!list.IsValid())
    return false;

  csRef<iSoundLoader> plugin;
  while (list.IsValid() && !plugin.IsValid())
  {
    char const* classname = list->Get(0);
    plugin = CS_LOAD_PLUGIN (plugin_mgr, classname, iSoundLoader);
    if (plugin.IsValid())
    {
      // remember the plugin
      Loaders.Push (plugin);
    }
    list->DeleteIndex (0);
    if (list->Length() == 0)
    {
      list = 0;
      plugin_mgr = 0;
    }
  }
  return true;
}

csPtr<iSoundData> csSoundLoaderMultiplexer::LoadSound (
	void *Data, uint32 Size) 
{
  bool consecutive = false; // set to true if we searched the list completely.
  do
  {
    size_t i;
    for (i=Loaders.Length(); (i--)>0; ) 
      // i is decremented after comparison but before we use it below;
      //  hence it goes from Laoders.Length()-1 to 0
    {
      csRef<iSoundLoader> ldr = Loaders.Get(i);
      csRef<iSoundData> snd (ldr->LoadSound(Data, Size));
      if (snd)
      {
	/*
	  move used plugin to the bottom of the list.
	  the idea is that some formats are used more
	  commonly than other formats and that those
	  plugins are asked first. 
	 */
	if ((Loaders.Length()-i) > 4)
	  // keep a 'top 4'; no need to shuffle the list
	  // when a plugin is already one of the first asked
	{
	  Loaders.Push (ldr);
	  Loaders.DeleteIndex (i);
	}
	return csPtr<iSoundData> (snd);
      }
      // if we just loaded a plugin only check that.
      if (consecutive) break;
    }
    consecutive = true;
  } while (LoadNextPlugin());
  return 0;
}
