/*
    Copyright (C) 1998-2003 by Jorrit Tyberghein

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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "csutil/plugmgr.h"
#include "csutil/util.h"
#include "csutil/array.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/cmdline.h"
#include "iutil/cfgmgr.h"
#include "ivaria/reporter.h"

//------------------------------------------------------ csPlugin class -----//

csPluginManager::csPlugin::csPlugin (iComponent *obj, const char *classID)
{
  Plugin = obj;
  ClassID = csStrNew (classID);
}

csPluginManager::csPlugin::~csPlugin ()
{
//csPrintf ("DecRef %08p/'%s' ref=%d\n", Plugin, ClassID, Plugin->GetRefCount ()); fflush (stdout);
  delete [] ClassID;
  Plugin->DecRef ();
}

//------------------------------------------------------------------------
/**
 * Implementation of iPluginIterator.
 */
class csPluginIterator : public iPluginIterator
{
public:
  csArray<iBase*> pointers;
  size_t idx;

public:
  csPluginIterator ()
  {
    SCF_CONSTRUCT_IBASE (0);
    idx = 0;
  }
  virtual ~csPluginIterator ()
  {
    SCF_DESTRUCT_IBASE ();
  }

  SCF_DECLARE_IBASE;

  virtual bool HasNext ()
  {
    return idx < pointers.Length ();
  }
  virtual iBase* Next ()
  {
    iBase* p = pointers[idx];
    idx++;
    return p;
  }
};

SCF_IMPLEMENT_IBASE (csPluginIterator)
  SCF_IMPLEMENTS_INTERFACE (iPluginIterator)
SCF_IMPLEMENT_IBASE_END

//------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csPluginManager)
  SCF_IMPLEMENTS_INTERFACE (iPluginManager)
SCF_IMPLEMENT_IBASE_END

csPluginManager::csPluginManager (iObjectRegistry* object_reg) :
  Plugins (8, 8), OptionList (16, 16)
{
  SCF_CONSTRUCT_IBASE (0);
  csPluginManager::object_reg = object_reg;
  // We need a recursive mutex.
  mutex = csMutex::Create (true);
}

csPluginManager::~csPluginManager ()
{
  Clear ();
  SCF_DESTRUCT_IBASE ();
}

void csPluginManager::Clear ()
{
  csScopedMutexLock lock (mutex);
  OptionList.DeleteAll ();

  // Free all plugins.
  for (size_t i = Plugins.Length() ; i > 0 ; i--)
    UnloadPlugin ((iComponent *)Plugins.Get(i - 1)->Plugin);
}

void csPluginManager::QueryOptions (iComponent *obj)
{
  csRef<iCommandLineParser> CommandLine (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));

  csRef<iPluginConfig> Config (SCF_QUERY_INTERFACE (obj, iPluginConfig));
  if (Config)
  {
    size_t on = OptionList.Length ();
    for (int i = 0 ; ; i++)
    {
      csOptionDescription option;
      if (!Config->GetOptionDescription (i, &option))
        break;
      OptionList.Push (new csPluginOption (option.name, option.type, option.id,
        (option.type == CSVAR_BOOL) || (option.type == CSVAR_CMD), Config));
      if (option.type == CSVAR_BOOL)
      {
        char buf[100];
        strcpy (buf, "no");
        strcpy (buf + 2, option.name);
        OptionList.Push (new csPluginOption (buf, option.type, option.id,
          false, Config));
      }
    }

    // Parse the command line for plugin options and pass them to plugin
    for (; on < OptionList.Length (); on++)
    {
      csPluginOption *pio = (csPluginOption *)OptionList.Get (on);
      const char *val = CommandLine->GetOption (pio->Name);

      if (val)
      {
        csVariant optval;
        switch (pio->Type)
        {
          case CSVAR_CMD:
	    optval.SetCommand ();
	    break;
          case CSVAR_BOOL:
            optval.SetBool (pio->Value);
            break;
          case CSVAR_LONG:
            if (!val) continue;
            optval.SetLong (atol (val));
            break;
          case CSVAR_FLOAT:
            if (!val) continue;
            optval.SetFloat (atof (val));
            break;
	  case CSVAR_STRING:
	    if (!val) continue;
	    optval.SetString (val);
	    break;
        }
        pio->Config->SetOption (pio->ID, &optval);
      }
    }
  }
}

iBase *csPluginManager::LoadPlugin (const char *classID,
  const char *iInterface, int iVersion, bool init)
{
  iComponent *p = 0;

  { 
    // The reference must be held beyond the scope of this block.
    csRef<iComponent> dummy (SCF_CREATE_INSTANCE (classID, iComponent));
    if (dummy) {
      p = dummy;
      p->IncRef();
    }
  }
  
  if (!p)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    	"crystalspace.pluginmgr.loadplugin",
    	"WARNING: could not load plugin '%s'", classID);
  }
  else
  {
    csScopedMutexLock lock (mutex);
    size_t index = (size_t)-1;
    // See if the plugin is already in our plugin list.
    for (size_t i = 0 ; i < Plugins.Length () ; i++)
    {
      csPlugin* pl = Plugins.Get (i);
      if (pl->ClassID)
        if (pl->ClassID == classID || !strcmp (pl->ClassID, classID))
	{
	  index = i;
	  break;
	}
    }

    bool added_here = false;
    if (index == (size_t)-1)
    {
      // The plugin wasn't in our plugin list yet. Add it here.
      index = Plugins.Push (new csPlugin (p, classID));
      added_here = true;
    }

    if ((!init) || p->Initialize (object_reg))
    {
      iBase *ret;
      if (iInterface)
        ret = (iBase *)p->QueryInterface (
	  iSCF::SCF->GetInterfaceID (iInterface), iVersion);
      else
        (ret = p)->IncRef();
      if (ret)
      {
        if (!added_here)
	{
	  // If we didn't add the plugin (i.e. this is not the first time
	  // we called LoadPlugin() for this plugin) then we need to
	  // DecRef() the component to avoid memory leaks.
	  p->DecRef ();
	}

        if (init) QueryOptions (p);
        return ret;
      }
      else
      {
        if (!added_here)
	{
	  // If we didn't add the plugin (i.e. this is not the first time
	  // we called LoadPlugin() for this plugin) then we need to
	  // DecRef() the component to avoid memory leaks.
	  p->DecRef ();
	}
      }
    }
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    	"crystalspace.pluginmgr.loadplugin",
    	"WARNING: failed to initialize plugin '%s'", classID);
    // If we added this plugin in this call then we remove it here as well.
    if (added_here)
      Plugins.DeleteIndex (index);
  }
  return 0;
}

bool csPluginManager::RegisterPlugin (const char *classID,
  iComponent *obj)
{
  csScopedMutexLock lock (mutex);
  size_t index = Plugins.Push (new csPlugin (obj, classID));
  if (obj->Initialize (object_reg))
  {
    QueryOptions (obj);
    obj->IncRef ();
    return true;
  }
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
    	"crystalspace.pluginmgr.registerplugin",
    	"WARNING: failed to initialize plugin '%s'", classID);
    Plugins.DeleteIndex (index);
    return false;
  }
}

csPtr<iPluginIterator> csPluginManager::GetPlugins ()
{
  csScopedMutexLock lock (mutex);
  csPluginIterator* it = new csPluginIterator ();
  size_t i;
  for (i = 0 ; i < Plugins.Length () ; i++)
  {
    it->pointers.Push (Plugins.Get (i)->Plugin);
  }
  return csPtr<iPluginIterator> (it);
}

iBase *csPluginManager::QueryPlugin (const char *iInterface, int iVersion)
{
  scfInterfaceID ifID = iSCF::SCF->GetInterfaceID (iInterface);
  csScopedMutexLock lock (mutex);
  for (size_t i = 0; i < Plugins.Length (); i++)
  {
    iBase *ret =
      (iBase *)Plugins.Get (i)->Plugin->QueryInterface (ifID, iVersion);
    if (ret)
      return ret;
  }
  return 0;
}

iBase *csPluginManager::QueryPlugin (const char* classID,
				    const char *iInterface, int iVersion)
{
  scfInterfaceID ifID = iSCF::SCF->GetInterfaceID (iInterface);
  csScopedMutexLock lock (mutex);
  for (size_t i = 0 ; i < Plugins.Length () ; i++)
  {
    csPlugin* pl = Plugins.Get (i);
    if (pl->ClassID)
      if (pl->ClassID == classID || !strcmp (pl->ClassID, classID))
      {
	return (iBase*)Plugins.Get(i)->Plugin->QueryInterface(ifID,iVersion);
      }
  }
  return 0;
}

bool csPluginManager::UnloadPlugin (iComponent* obj)
{
  csScopedMutexLock lock (mutex);
  size_t idx = Plugins.FindKey (
    csArrayCmp<csPlugin*,iComponent*>(obj, csPluginsVector::CompareAddress));
  if (idx == csArrayItemNotFound)
    return false;

  csRef<iPluginConfig> config (SCF_QUERY_INTERFACE (obj, iPluginConfig));
  if (config)
  {
    for (size_t i = OptionList.Length (); i > 0; i--)
    {
      csPluginOption *pio = (csPluginOption *)OptionList.Get (i - 1);
      if (pio->Config == config)
        OptionList.DeleteIndex (i - 1);
    }
  }

  object_reg->Unregister ((iBase *)obj, 0);
  return Plugins.DeleteIndex (idx);
}

