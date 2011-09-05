/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_PLUGMGR_H__
#define __CS_PLUGMGR_H__

/**\file
 * Standard iPluginManager implementation.
 */

#include "csextern.h"
#include "csutil/parray.h"
#include "csutil/hash.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"
#include "csutil/threading/mutex.h"
#include "csutil/threadmanager.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iutil/pluginconfig.h"

struct iObjectRegistry;

/**
 * This is the standard implementation of the plugin manager.
 * The plugin manager is thread-safe.
 */
class CS_CRYSTALSPACE_EXPORT csPluginManager :
  public scfImplementation1<csPluginManager, iPluginManager>
{
private:
  /// Mutex to make the plugin manager thread-safe.
  CS::Threading::RecursiveMutex mutex;
  bool do_verbose;

  /// Mutex on 'already loading' array.
  CS::Threading::Mutex loadingLock;

  /**
   * Ref counted plugin load condition.
   */
  class PluginLoadCondition : public CS::Threading::Condition,
    public CS::Utility::FastRefCount<PluginLoadCondition>
  {
  };

  /// Hash of loading plugins and their conditions.
  typedef csHash<csRef<PluginLoadCondition>, csString> AlreadyLoadingHash;
  AlreadyLoadingHash alreadyLoading;

  /**
   * This is a private structure used to keep the list of plugins.
   */
  class CS_CRYSTALSPACE_EXPORT csPlugin
  {
  public:
    /// The plugin itself
    csWeakRef<iComponent> Plugin;
    /// The class ID of the plugin
    csString ClassID;

    csPlugin ();

    /// Construct the object that represents a plugin
    csPlugin (iComponent *iObject, const char *iClassID);

    /// Comparison
    bool operator== (const csPlugin& other) const
    { return Plugin == other.Plugin && ClassID == other.ClassID; }
    bool operator< (const csPlugin& other) const
    { return Plugin < other.Plugin || 
            (Plugin == other.Plugin && ClassID < other.ClassID); }
  };

  /**
   * This is a superset of csArray that can find by pointer a plugin.
   */
  class CS_CRYSTALSPACE_EXPORT csPluginsVector :
    public csArray<csPlugin, 
                   csArraySafeCopyElementHandler<csPlugin>,
                   CS::Container::ArrayAllocDefault,
                   csArrayCapacityFixedGrow<8> >
  {
  public:
    /// Create the vector
    csPluginsVector (int l) : csArray<csPlugin, 
      csArraySafeCopyElementHandler<csPlugin>, CS::Container::ArrayAllocDefault,
      csArrayCapacityFixedGrow<8> > (l) {}
    /// Find a plugin by its address
    static int CompareAddress (csPlugin const& Item, iComponent* const& Key)
    { return Item.Plugin == Key ? 0 : 1; }
  };

  /**
   * Class to collect all options for all plug-in modules in the system.
   */
  class CS_CRYSTALSPACE_EXPORT csPluginOption
  {
  public:
    char *Name;
    csVariantType Type;
    int ID;
    bool Value;				// If Type is CSVAR_BOOL
    csRef<iPluginConfig> Config;

    csPluginOption (const char *iName, csVariantType iType, int iID,
      bool iValue, iPluginConfig* cfg)
    {
      Name = csStrNew (iName);
      Type = iType;
      ID = iID;
      Value = iValue;
      Config = cfg;
    }
    virtual ~csPluginOption ()
    {
      delete [] Name;
    }
  };

  /// The object registry.
  iObjectRegistry* object_reg;

  /// The list of all plug-ins
  csPluginsVector Plugins;
  
  csPlugin* FindPluginByClassID (const char* classID,
    csPlugin* startAfter = 0);
  void WaitForPluginLoad (const char* classID);
  csStringArray GetClassIDTagsLocal (const char* classID);

  /// List of all options for all plug-in modules.
  csPDelArray<csPluginOption, CS::Container::ArrayAllocDefault,
    csArrayCapacityFixedGrow<16> > OptionList;
    
  typedef csHash<csString, csString> TagToClassHash;
  TagToClassHash tagToClassMap;
  
  void Report (int severity, const char* subMsgID,
    const char* message, ...);
  void ReportV (int severity, const char* subMsgID,
    const char* message, va_list args);
  /* Report while the mutex is held:
   * Report() might wait for the main thread; if the mutex is held there
   * b/c a plugin is being loaded a deadlock may occur.
   * This method avoids that. */
  void ReportInLock (int severity, const char* subMsgID,
    const char* message, ...);
public:
  /// Initialize plugin manager.
  csPluginManager (iObjectRegistry* object_reg);
  /// Destruct.
  virtual ~csPluginManager ();

  virtual csPtr<iComponent> LoadPluginInstance (const char* iClassID, uint flags);

  virtual csPtr<iComponent> QueryPluginInstance (const char *iInterface, int iVersion);
  csPtr<iComponent> QueryPluginInstance (const char* classID);
  virtual csPtr<iComponent> QueryPluginInstance (const char* iClassID,
  	  const char *iInterface, int iVersion);
  virtual bool UnloadPluginInstance (iComponent *iObject);
  virtual bool RegisterPluginInstance (const char *iClassID,
          iComponent *iObject);
  virtual csPtr<iPluginIterator> GetPluginInstances ();
  virtual void Clear ();

  /// Query all options supported by given plugin and place into OptionList
  virtual void QueryOptions (iComponent *iObject);
  
  bool SetTagClassIDMapping (const char* tag, const char* classID)
  {
    CS::Threading::RecursiveMutexScopedLock lock (mutex);
    bool result = tagToClassMap.Contains (tag);
    tagToClassMap.PutUnique (tag, classID);
    return result;
  }
  
  bool UnsetTagClassIDMapping (const char* tag)
  {
    CS::Threading::RecursiveMutexScopedLock lock (mutex);
    return tagToClassMap.DeleteAll (tag);
  }
  
  const char* GetTagClassIDMapping (const char* tag)
  {
    CS::Threading::RecursiveMutexScopedLock lock (mutex);
    return tagToClassMap.Get (tag, (const char*)0);
  }
  
  csPtr<iStringArray> GetClassIDTags (const char* classID);
  
  csPtr<iComponent> LoadTagPluginInstance (const char* tag,
    uint loadFlags)
  {
    return LoadTagPluginInstance (GetTagClassIDMapping (tag), loadFlags);
  }
  
  csPtr<iComponent> QueryTagPluginInstance (const char* tag)
  {
    return QueryPluginInstance (GetTagClassIDMapping (tag));
  }
};

#endif // __CS_PLUGMGR_H__

