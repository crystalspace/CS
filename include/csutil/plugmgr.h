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

#include "csutil/scf.h"
#include "csutil/csvector.h"
#include "csutil/typedvec.h"
#include "iutil/config.h"
#include "iutil/plugin.h"

struct iComponent;
struct iObjectRegistry;

/**
 * This is the standard implementation of the plugin manager.
 */
class csPluginManager : public iPluginManager
{
private:
  /*
   * This is a private structure used to keep the list of plugins.
   */
  class csPlugin
  {
  public:
    // The plugin itself
    iComponent *Plugin;
    // The class ID of the plugin
    char *ClassID;

    // Construct the object that represents a plugin
    csPlugin (iComponent *iObject, const char *iClassID);
    // Free storage
    virtual ~csPlugin ();
  };

  /*
   * This is a superset of csVector that can find by pointer a plugin.
   */
  class csPluginsVector : public csVector
  {
  public:
    // Create the vector
    csPluginsVector (int iLimit, int iDelta) : csVector (iLimit, iDelta) {}
    // Destroy the vector.
    virtual ~csPluginsVector () { DeleteAll (); }
    // Find a plugin by its address
    virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
    {
      return ((csPlugin *)Item)->Plugin == Key ? 0 : 1;
    }
    // Overrided Get() to avoid typecasts
    csPlugin *Get (int idx)
    { return (csPlugin *)csVector::Get (idx); }
    
    virtual bool FreeItem (csSome Item)
    { delete (csPlugin*)Item; return true; }
  };

  /*
   * Class to collect all options for all plug-in modules in the system.
   */
  class csPluginOption
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

  // Query all options supported by given plugin and place into OptionList
  void QueryOptions (iComponent *iObject);

  // The object registry.
  iObjectRegistry* object_reg;

  /// The list of all plug-ins
  csPluginsVector Plugins;

  /// List of all options for all plug-in modules.
  CS_DECLARE_TYPED_VECTOR (csOptionVector, csPluginOption) OptionList;

public:
  /// Initialize plugin manager.
  csPluginManager (iObjectRegistry* object_reg);
  /// Destruct.
  virtual ~csPluginManager ();

  SCF_DECLARE_IBASE;

  virtual iBase *LoadPlugin (const char *iClassID,
        const char *iInterface = NULL, int iVersion = 0);
  virtual iBase *QueryPlugin (const char *iInterface, int iVersion);
  virtual iBase *QueryPlugin (const char* iClassID,
  	  const char *iInterface, int iVersion);
  virtual bool UnloadPlugin (iComponent *iObject);
  virtual bool RegisterPlugin (const char *iClassID,
          iComponent *iObject);
  virtual int GetPluginCount ();
  virtual iBase* GetPlugin (int idx);
  virtual void Clear ();
};

#endif // __CS_PLUGMGR_H__

