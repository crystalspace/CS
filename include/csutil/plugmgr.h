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

#include "csextern.h"
#include "scf.h"
#include "scopedmutexlock.h"
#include "parray.h"
#include "iutil/config.h"
#include "iutil/plugin.h"

struct iComponent;
struct iObjectRegistry;

/**
 * This is the standard implementation of the plugin manager.
 * The plugin manager is thread-safe.
 */
class CS_CRYSTALSPACE_EXPORT csPluginManager : public iPluginManager
{
private:
  /// Mutex to make the plugin manager thread-safe.
  csRef<csMutex> mutex;

  /**
   * This is a private structure used to keep the list of plugins.
   */
  class CS_CRYSTALSPACE_EXPORT csPlugin
  {
  public:
    /// The plugin itself
    iComponent *Plugin;
    /// The class ID of the plugin
    char *ClassID;

    /// Construct the object that represents a plugin
    csPlugin (iComponent *iObject, const char *iClassID);
    /// Free storage
    virtual ~csPlugin ();
  };

  /**
   * This is a superset of csPDelArray that can find by pointer a plugin.
   */
  class CS_CRYSTALSPACE_EXPORT csPluginsVector : public csPDelArray<csPlugin>
  {
  public:
    /// Create the vector
    csPluginsVector (int l, int d) : csPDelArray<csPlugin> (l, d) {}
    /// Find a plugin by its address
    static int CompareAddress (csPlugin* const& Item, iComponent* const& Key)
    { return Item->Plugin == Key ? 0 : 1; }
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
    iConfig *Config;

    csPluginOption (const char *iName, csVariantType iType, int iID,
      bool iValue, iConfig* iConfig)
    {
      Name = csStrNew (iName);
      Type = iType;
      ID = iID;
      Value = iValue;
      (Config = iConfig)->IncRef ();
    }
    virtual ~csPluginOption ()
    {
      Config->DecRef ();
      delete [] Name;
    }
  };

  /// The object registry.
  iObjectRegistry* object_reg;

  /// The list of all plug-ins
  csPluginsVector Plugins;

  // List of all options for all plug-in modules.
  csPDelArray<csPluginOption> OptionList;

public:
  /// Initialize plugin manager.
  csPluginManager (iObjectRegistry* object_reg);
  /// Destruct.
  virtual ~csPluginManager ();

  SCF_DECLARE_IBASE;

  /// Load a plugin and (optionally) initialize it.
  virtual iBase *LoadPlugin (const char *iClassID,
        const char *iInterface = 0, int iVersion = 0,
	bool init = true);

  /**
   * Get first of the loaded plugins that supports given interface ID.
   */
  virtual iBase *QueryPlugin (const char *iInterface, int iVersion);
  /// Find a plugin given his class ID.
  virtual iBase *QueryPlugin (const char* iClassID,
  	  const char *iInterface, int iVersion);
  /// Remove a plugin from system driver's plugin list.
  virtual bool UnloadPlugin (iComponent *iObject);
  /// Register a object that implements the iComponent interface as a plugin.
  virtual bool RegisterPlugin (const char *iClassID,
          iComponent *iObject);
  /// Get an iterator to iterate over all plugins.
  virtual csPtr<iPluginIterator> GetPlugins ();
  /// Unload all plugins from this plugin manager.
  virtual void Clear ();

  /// Query all options supported by given plugin and place into OptionList
  virtual void QueryOptions (iComponent *iObject);
};

#endif // __CS_PLUGMGR_H__

