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

#include "csutil/array.h"
#include "csutil/plugldr.h"
#include "csutil/plugmgr.h"
#include "csutil/scf_implementation.h"
#include "csutil/scfstringarray.h"
#include "csutil/stringconv.h"
#include "csutil/stringquote.h"
#include "csutil/util.h"

#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/cmdline.h"
#include "iutil/cfgmgr.h"
#include "iutil/verbositymanager.h"
#include "ivaria/reporter.h"

//------------------------------------------------------ csPlugin class -----//
csPluginManager::csPlugin::csPlugin ()
{
}

csPluginManager::csPlugin::csPlugin (iComponent *obj, const char *classID)
  : Plugin (obj), ClassID (classID) { }

//------------------------------------------------------------------------
/**
 * Implementation of iPluginIterator.
 */
class csPluginIterator : public scfImplementation1<csPluginIterator,
                                                   iPluginIterator>
{
public:
  csArray<iComponent*> pointers;
  size_t idx;

public:
  csPluginIterator ()
    : scfImplementationType (this), idx (0)
  {
  }
  virtual ~csPluginIterator ()
  {
  }


  virtual bool HasNext ()
  {
    return idx < pointers.GetSize ();
  }
  virtual iComponent* Next ()
  {
    iComponent* p = pointers[idx];
    idx++;
    return p;
  }
};


//------------------------------------------------------------------------


csPluginManager::csPluginManager (iObjectRegistry* object_reg) 
  : scfImplementationType (this), do_verbose (false), object_reg (object_reg),
  Plugins (8), OptionList (16)
{
  csRef<iVerbosityManager> verbosity = csQueryRegistry<iVerbosityManager> (
    object_reg);
  if (verbosity.IsValid())
    do_verbose = verbosity->Enabled ("plugins");
}

csPluginManager::~csPluginManager ()
{
  Clear ();
}

void csPluginManager::Report (int severity, const char* subMsgID,
                              const char* message, ...)
{
  va_list args;
  va_start (args, message);
  ReportV (severity, subMsgID, message, args);
  va_end (args);
}

void csPluginManager::ReportV (int severity, const char* subMsgID,
                               const char* message, va_list args)
{
  csStringFast<64> msgId ("crystalspace.pluginmgr.");
  msgId.Append (subMsgID);
  csReportV (object_reg, severity, msgId, message, args);
}

void csPluginManager::ReportInLock (int severity, const char* subMsgID,
				    const char* message, ...)
{
  va_list args;
  va_start (args, message);
  csStringFast<64> msgId ("crystalspace.pluginmgr.");
  msgId.Append (subMsgID);
  csReportV (0/*deliberate, force printf*/, severity, msgId, message, args);
  va_end (args);
}

void csPluginManager::Clear ()
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
 
  OptionList.DeleteAll ();

  // Free all plugins.
  for (size_t i = Plugins.GetSize () ; i > 0 ; i--)
  {
    csRef<iComponent> plugin (Plugins.Get(i - 1).Plugin);
    if (!plugin)
    {
      Plugins.DeleteIndexFast (i);
      continue;
    }
    UnloadPluginInstance (plugin);
  }
}

void csPluginManager::QueryOptions (iComponent *obj)
{
  csRef<iCommandLineParser> CommandLine (
  	csQueryRegistry<iCommandLineParser> (object_reg));

  csRef<iPluginConfig> Config (scfQueryInterface<iPluginConfig> (obj));
  if (Config)
  {
    size_t on = OptionList.GetSize ();
    for (int i = 0 ; ; i++)
    {
      csOptionDescription option;
      if (!Config->GetOptionDescription (i, &option))
        break;
      OptionList.Push (new csPluginOption (option.name.GetData (), option.type, option.id,
        (option.type == CSVAR_BOOL) || (option.type == CSVAR_CMD), Config));
      if (option.type == CSVAR_BOOL)
      {
        char buf[100];
        strcpy (buf, "no");
        strcpy (buf + 2, option.name.GetData ());
        OptionList.Push (new csPluginOption (buf, option.type, option.id,
          false, Config));
      }
    }

    // Parse the command line for plugin options and pass them to plugin
    for (; on < OptionList.GetSize (); on++)
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
            optval.SetFloat (CS::Utility::strtof (val));
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

using namespace CS::Utility;
using namespace CS::Threading;
csPtr<iComponent> csPluginManager::LoadPluginInstance (const char *classID,
                                                       uint flags)
{
  csRef<PluginLoadCondition> loading;
  {
    bool isAlreadyLoading = true;
    MutexScopedLock lock (loadingLock);

    // Check if this plugin is already loading.
    loading = alreadyLoading.Get(classID, csRef<PluginLoadCondition>());

    // If not...
    if(!loading.IsValid())
    {
      // If we wish to return any loaded instance of this plugin..
      if(flags & lpiReturnLoadedInstance)
      {
        // Check if this plugin is already loaded and return it if so.
	CS::Threading::RecursiveMutexScopedLock lock (mutex);
	csRef<iComponent> comp;
	csPlugin* pl = FindPluginByClassID (classID);
	if (pl)
	  comp = pl->Plugin;
        if(comp)
          return csPtr<iComponent>(comp);
      }

      // Create a new loading condition and mark this plugin as loading.
      csRef<PluginLoadCondition> cond = csPtr<PluginLoadCondition>(new PluginLoadCondition());
      loading = alreadyLoading.Put (classID, cond);
      isAlreadyLoading = false;
    }

    // If another thread is currently loading this plugin..
    if(isAlreadyLoading)
    {
      // Wait until it's finished.
      loading->Wait(loadingLock);

      // If we wish to return any loaded instance of this plugin then do so.
      if(flags & lpiReturnLoadedInstance)
      {
        // The plugin should have been loaded now.
	CS::Threading::RecursiveMutexScopedLock lock (mutex);
	csPlugin* pl = FindPluginByClassID (classID);
	csRef<iComponent> comp (pl->Plugin);
        return csPtr<iComponent> (comp);
      }
    }
  }

  if (do_verbose)
    /* LoadPluginInstance() may be called recursively and the lock
    * actually held here */
    ReportInLock (CS_REPORTER_SEVERITY_NOTIFY,
    "verbose",
    "loading plugin instance for %s", classID);

  csRef<iComponent> p (scfCreateInstance<iComponent> (classID));

  if (!p)
  {
    if (flags & lpiReportErrors)
      Report (CS_REPORTER_SEVERITY_WARNING,
      "loadplugin",
      "could not load plugin %s", CS::Quote::Single (classID));
  }
  else
  {
    mutex.Lock();
    // See if the plugin is already in our plugin list.
    csPlugin* pl = FindPluginByClassID (classID);
    size_t index = pl ? Plugins.GetIndex (pl) : csArrayItemNotFound;

    if (index == csArrayItemNotFound)
    {
      // The plugin wasn't in our plugin list yet. Add it here.
      index = Plugins.Push (csPlugin (p, classID));
    }
    else if (!Plugins[index].Plugin)
      // Plugin has been unloaded, store in previous slot
      Plugins[index].Plugin = p;

    if (flags & lpiLoadDependencies)
    {
      // Grab dependencies of plugin to load.
      const char *dep = iSCF::SCF->GetClassDependencies (classID);

      csStringFast<128> tmp;
      // For each dependency:
      while (dep && *dep)
      {
        const char *comma = strchr (dep, ',');
        if (!comma)
          comma = strchr (dep, 0);
        size_t sl = comma - dep;
        tmp.Replace (dep, sl);

        dep = comma;
        while (*dep == ',' || *dep == ' ' || *dep == '\t')
          dep++;

        tmp.Trim();
        if (tmp.IsEmpty())
          continue;

        if (do_verbose)
          ReportInLock (CS_REPORTER_SEVERITY_NOTIFY,
          "verbose",
          "found dependency on %s", tmp.GetData());

        // Check if tags are associated with class IDs.
        csStringArray tags (GetClassIDTagsLocal (tmp));
        if (tags.GetSize() > 0)
        {
          // If tags are associated check if object reg has something registered
          //  with the(se) tag(s).
          for (size_t t = 0; t < tags.GetSize(); t++)
          {
            if (do_verbose)
              ReportInLock (CS_REPORTER_SEVERITY_NOTIFY,
              "verbose",
              "  found tag for dependency: %s", tags[t]);
            csRef<iBase> b = csPtr<iBase> (object_reg->Get (tags[t]));
            if (b == 0)
            {
              const char* classForTag =
                GetTagClassIDMapping (tags[t]); // tmp might be wildcard
              csPlugin* pl = FindPluginByClassID (classForTag);
              if (pl != 0) continue; // Plugin is currently being loaded
              csRef<iComponent> p = LoadPluginInstance (classForTag, flags);
              if (p.IsValid())
              {
                object_reg->Register (p, tags[t]);
              }
              else
              {
                /* Ignore load error (like csPluginLoader did) */
              }
            }
          }
        }
        else
        {
          /* If no tags, ignore dependency (like csPluginLoader did) */
        }
      }
    }

    mutex.Unlock();
    if ((!(flags & lpiInitialize)) || p->Initialize (object_reg))
    {
      if (flags & lpiInitialize) QueryOptions (p);
      CS::Threading::MutexScopedLock lock (loadingLock);
      alreadyLoading.Delete(classID, loading);
      loading->NotifyAll();
      return csPtr<iComponent> (p);
    }
    mutex.Lock();

    // If we added this plugin in this call then we remove it here as well.
    if (index != csArrayItemNotFound)
      Plugins.DeleteIndex (index);

    mutex.Unlock();
    if (flags & lpiReportErrors)
    {
      Report (CS_REPORTER_SEVERITY_WARNING,
        "loadplugin",
        "failed to initialize plugin %s", CS::Quote::Single (classID));
    }
  }

  CS::Threading::MutexScopedLock lock (loadingLock);
  alreadyLoading.Delete(classID, loading);
  loading->NotifyAll();
  return 0;
}

bool csPluginManager::RegisterPluginInstance (const char *classID,
  iComponent *obj)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t index = Plugins.Push (csPlugin (obj, classID));
  if (obj->Initialize (object_reg))
  {
    QueryOptions (obj);
    return true;
  }
  else
  {
    Plugins.DeleteIndex (index);
    mutex.Unlock(); // Lock is not needed here any more ...
    Report (CS_REPORTER_SEVERITY_WARNING,
      "registerplugin",
      "failed to initialize plugin %s", CS::Quote::Single (classID));
    // ... but must be retaken to make 'lock' destruction work
    mutex.Lock();
    return false;
  }
}

csPtr<iPluginIterator> csPluginManager::GetPluginInstances ()
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  csPluginIterator* it = new csPluginIterator ();
  size_t i;
  for (i = 0 ; i < Plugins.GetSize () ; i++)
  {
    if (!Plugins[i].Plugin) continue;
    it->pointers.Push (Plugins[i].Plugin);
  }
  return csPtr<iPluginIterator> (it);
}

csPluginManager::csPlugin* csPluginManager::FindPluginByClassID (
  const char* classID, csPlugin* startAfter)
{
  size_t i;
  if (startAfter)
    i = Plugins.GetIndex (startAfter) + 1;
  else
    i = 0;
    
  size_t classIDLen = strlen (classID);
  bool wildcard = classID[classIDLen-1] == '.';
  if (wildcard)
  {
    for (; i < Plugins.GetSize () ; i++)
    {
      csPlugin& pl = Plugins.Get (i);
      if (strncmp (pl.ClassID, classID, classIDLen))
      {
	return &pl;
      }
    }
  }
  else
  {
    for (; i < Plugins.GetSize () ; i++)
    {
      csPlugin& pl = Plugins.Get (i);
      if (pl.ClassID == classID)
      {
	return &pl;
      }
    }
  }
  return 0;
}

void csPluginManager::WaitForPluginLoad (const char* classID)
{
  // Check if this plugin is already loading.
  csRef<PluginLoadCondition> loading = alreadyLoading.Get(classID, csRef<PluginLoadCondition>());
  if (loading.IsValid())
  {
    loading->Wait(loadingLock);
  }
}

csPtr<iComponent> csPluginManager::QueryPluginInstance (const char* classID)
{
  loadingLock.Lock();
  /* Acquire main lock _before_ unlocking the loading lock in order to make
     sure the alreadyLoading hash isn't modified between waiting for the
     plugin load and getting the lock - that would mean a missed loading-
     in-progress */
  WaitForPluginLoad (classID);
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  loadingLock.Unlock();
  csPlugin* pl = FindPluginByClassID (classID);
  if (pl) return csPtr<iComponent> (pl->Plugin);
  return 0;
}

csPtr<iComponent> csPluginManager::QueryPluginInstance (const char *iInterface, int iVersion)
{
  scfInterfaceID ifID = iSCF::SCF->GetInterfaceID (iInterface);
  loadingLock.Lock();
  {
    // Make sure all pending plugins are loaded completely
    while (alreadyLoading.GetSize() > 0)
    {
      AlreadyLoadingHash::GlobalIterator it (alreadyLoading.GetIterator());
      if (it.HasNext())
      {
	csRef<PluginLoadCondition> loading (it.Next ());
	// Avoid deadlock if loading didn't commence yet
	loading->Wait (loadingLock);
      }
      loadingLock.Unlock();
      CS::Threading::Thread::Yield();
      loadingLock.Lock();
    }
  }
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  loadingLock.Unlock();
  for (size_t i = 0; i < Plugins.GetSize (); i++)
  {
    iComponent* ret = Plugins[i].Plugin;
    if (!ret) continue;
    if (ret->QueryInterface (ifID, iVersion))
      // QI does an implicit IncRef()
      return ret;
  }
  return 0;
}

csPtr<iComponent> csPluginManager::QueryPluginInstance (const char* classID,
				                        const char *iInterface, 
                                                        int iVersion)
{
  scfInterfaceID ifID = iSCF::SCF->GetInterfaceID (iInterface);

  loadingLock.Lock();
  WaitForPluginLoad (classID);
  /* Acquire main lock _before_ unlocking the loading lock in order to make
     sure the alreadyLoading hash isn't modified between waiting for the
     plugin load and getting the lock - that would mean a missed loading-
     in-progress */
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  loadingLock.Unlock();
  csPlugin* lastPlugin = 0;
  do
  {
    csPlugin* pl = FindPluginByClassID (classID, lastPlugin);
    if (pl)
    {
      iComponent* p = pl->Plugin;
      if (p && p->QueryInterface(ifID, iVersion))
        // QI does an implicit IncRef()
	return p;
    }
    lastPlugin = pl;
  }
  while (lastPlugin != 0);
  return 0;
}

bool csPluginManager::UnloadPluginInstance (iComponent* obj)
{
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  size_t idx = Plugins.FindKey (
    csArrayCmp<csPlugin,iComponent*>(obj, csPluginsVector::CompareAddress));
  if (idx == csArrayItemNotFound)
    return false;

  csRef<iPluginConfig> config (scfQueryInterface<iPluginConfig> (obj));
  if (config)
  {
    for (size_t i = OptionList.GetSize (); i > 0; i--)
    {
      csPluginOption *pio = (csPluginOption *)OptionList.Get (i - 1);
      if (pio->Config == config)
        OptionList.DeleteIndex (i - 1);
    }
  }

  object_reg->Unregister (obj, 0);
  return Plugins.DeleteIndex (idx);
}

csStringArray csPluginManager::GetClassIDTagsLocal (const char* classID)
{
  csStringArray result;
  bool wildcard = classID[strlen (classID)-1] == '.';
  
  CS::Threading::RecursiveMutexScopedLock lock (mutex);
  TagToClassHash::GlobalIterator it (tagToClassMap.GetIterator());
  while (it.HasNext())
  {
    csString tag;
    csString& id = it.Next (tag);
    bool match =
      wildcard ? id.StartsWith (classID, false) : id == classID;
    if (match) result.Push (tag);
  }
  return result;
}

csPtr<iStringArray> csPluginManager::GetClassIDTags (const char* classID)
{
  csRef<iStringArray> result;
  result.AttachNew (new scfStringArray (GetClassIDTagsLocal (classID)));
  return csPtr<iStringArray> (result);
}
