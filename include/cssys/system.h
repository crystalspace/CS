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

#ifndef __CS_SYSTEM_H__
#define __CS_SYSTEM_H__

#include <stdarg.h>
#include <stdio.h>

#include "csutil/util.h"
#include "csutil/csstrvec.h"
#include "csutil/csobjvec.h"
#include "csutil/typedvec.h"
#include "csutil/objreg.h"
#include "isys/system.h"
#include "isys/vfs.h"
#include "isys/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/config.h"
#include "iutil/objreg.h"
#include "iutil/virtclk.h"

struct iGraphics3D;
struct iGraphics2D;
struct iConfig;
struct iConfigManager;
struct iEventQueue;
class csPluginList;

/**
 * This is the interface to operating system.<p>
 * This driver takes care of all system-dependent parts such as
 * video hardware and input hardware. Note that system-dependent
 * source code should NOT write implementations for methods of
 * csSystemDriver (they are already implemented in system.cpp),
 * but inherit a new class from csSystemDriver, overriding desired
 * methods. Note that some methods it is required to override,
 * otherwise program simply will not compile (they are marked
 * as abstract).
 * <p>
 * This is an abstract class since it does not implement the iBase
 * interface. The iBase interface is supposed to be implemented
 * in SysSystemDriver which should be derived from csSystemDriver.
 */
class csSystemDriver : public iSystem
{
  friend class csPluginList;

private:
  /*
   * This is a private structure used to keep the list of plugins.
   */
  class csPlugin
  {
  public:
    // The plugin itself
    iComponent *Plugin;
    // The class ID of the plugin, and their functionality ID
    char *ClassID, *FuncID;

    // Construct the object that represents a plugin
    csPlugin (iComponent *iObject, const char *iClassID, const char *iFuncID);
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
    // Find a plugin either by its address or by his function ID
    virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const
    {
      if (Mode == 0)
        return ((csPlugin *)Item)->Plugin == Key ? 0 : 1;
      else
        return ((csPlugin *)Item)->FuncID
	     ? strcmp (((csPlugin *)Item)->FuncID, (char *)Key)
             : ((csPlugin *)Item)->FuncID == Key ? 0 : 1;
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

  // Elapsed time between last two frames and absolute time in milliseconds
  csTicks ElapsedTime, CurrentTime;
  
private:
  // The Virtual File System object
  iVFS *VFS;
  // Shared event queue.
  iEventQueue* EventQueue;

protected:
  // The object registry.
  csObjectRegistry object_reg;

private: //@@@
  /// Print something to the reporter.
  void ReportSys (int severity, const char* msg, ...);

  /// -------------------------- plug-ins --------------------------

  /// The list of all plug-ins
  csPluginsVector Plugins;

  /// Set to non-zero to exit csSystemDriver::Loop()
  bool Shutdown;

  /// Debugging level (0 = no debug, 1 = normal debug, 2 = verbose debug)
  int debug_level;

  /// List of all options for all plug-in modules.
  CS_DECLARE_TYPED_VECTOR (csOptionVector, csPluginOption) OptionList;

public:
  /// Initialize system-dependent data
  csSystemDriver ();
  /// Deinitialize system-dependent parts
  virtual ~csSystemDriver ();

  /// This is usually called right after object creation.
  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);

  /**
   * Send cscmdSystemOpen message to all loaded plugins.
   */
  virtual bool Open ();
  /// Close the system
  virtual void Close ();

  /**
   * System loop. This should be called last since it returns
   * only on program exit. There are two ways for every Crystal Space
   * application to function: the simplest way is to call Loop() and
   * it will take care of everything. The second way is to call NextFrame
   * manually often enough, and use an API-dependent application loop
   * (many GUI toolkits require your application to call their own event
   * loop function rather than providing your own). You will have to handle
   * the Shutdown variable yourself then.
   */
  virtual void Loop ();

  /**
   * SysSystemDriver::Loop calls this method once per frame.
   * This method can be called manually as well if you don't use the
   * Loop method. System drivers may override this method to pump events
   * from system event queue into Crystal Space event queue.
   */
  virtual void NextFrame ();

  /// Handle an event.
  virtual bool HandleEvent(iEvent&);

  /// Sleep for given number of 1/1000 seconds (very inacurate)
  virtual void Sleep (int /*SleepTime*/) {}

  /**
   * This is a function that prints the commandline help text.
   * If system driver has system-dependent switches, it should override
   * this method and type its own text (possibly after invoking
   * csSystemDriver::Help() first).
   */
  virtual void Help ();

  /// A shortcut for requesting to load a plugin (before Initialize())
  void RequestPlugin (const char *iPluginName);

// @@@ The following (and some of the above) should all move to the
// specific implementation of the plugin manager when we have that.

private:
  /// Load a plugin and initialize it.
  iBase *LoadPlugin (const char *iClassID, const char *iFuncID,
    const char *iInterface, int iVersion);
  /// Get first of the loaded plugins that supports given interface ID.
  iBase *QueryPlugin (const char *iInterface, int iVersion);
  /// Find a plugin given his functionality ID.
  iBase *QueryPlugin (const char *iFuncID, const char *iInterface,
  	int iVersion);
  /// Find a plugin given his class ID and functionality ID.
  iBase *QueryPlugin (const char* iClassID, const char *iFuncID,
  	const char *iInterface, int iVersion);
  /// Remove a plugin from system driver's plugin list.
  bool UnloadPlugin (iComponent *iObject);
  /// Register a object that implements the iComponent interface as a plugin.
  bool RegisterPlugin (const char *iClassID, const char *iFuncID,
  	iComponent *iObject);
  /// Get the number of loaded plugins in the plugin manager.
  int GetPluginCount ();
  /// Get the specified plugin from the plugin manager.
  iBase* GetPlugin (int idx);

protected:
  /// Advance the engine's virtual-time clock.
  void AdvanceVirtualTimeClock ();

  /// Suspend the engine's virtual-time clock.
  void SuspendVirtualTimeClock() {}
  /// Resume the engine's virtual-time clock.
  void ResumeVirtualTimeClock() { CurrentTime = csTicks(-1); }
  /// Query the elapsed time between last frames and absolute time.
  void GetElapsedTime (csTicks &oElapsedTime, csTicks &oCurrentTime) const
  { oElapsedTime = ElapsedTime; oCurrentTime = CurrentTime; }

  /**
   * Print help for an iConfig interface.
   */
  void Help (iConfig* Config);

  /**
   * Query default width/height/depth from config file
   * and from command-line parameters.
   */
  virtual void SetSystemDefaults (iConfigManager *config);

  /**
   * Open the user-specific configuration domain. The default is the file
   * CS/data/config/user.cfg. This function is called at least twice, with
   * different ID strings. This *must* be supported!
   */
  virtual iConfigFile *OpenUserConfig (const char *ApplicationID,
  	const char *Alias);

public:
  SCF_DECLARE_IBASE;

  /**************************** iSystem interface ****************************/

  /// Get the object registry.
  virtual iObjectRegistry* GetObjectRegistry () { return &object_reg; }

  //------------------------------------------------------------------

  class PluginManager : public iPluginManager
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSystemDriver);
    virtual iBase *LoadPlugin (const char *iClassID, const char *iFuncID,
          const char *iInterface = NULL, int iVersion = 0)
    {
      return scfParent->LoadPlugin (iClassID, iFuncID, iInterface,
        iVersion);
    }
    virtual iBase *QueryPlugin (const char *iInterface, int iVersion)
    {
      return scfParent->QueryPlugin (iInterface, iVersion);
    }
    virtual iBase *QueryPlugin (const char *iFuncID, const char *iInterface,
  	  int iVersion)
    {
      return scfParent->QueryPlugin (iFuncID, iInterface, iVersion);
    }
    virtual iBase *QueryPlugin (const char* iClassID, const char *iFuncID,
  	  const char *iInterface, int iVersion)
    {
      return scfParent->QueryPlugin (iClassID, iFuncID, iInterface, iVersion);
    }
    virtual bool UnloadPlugin (iComponent *iObject)
    {
      return scfParent->UnloadPlugin (iObject);
    }
    virtual bool RegisterPlugin (const char *iClassID, const char *iFuncID,
          iComponent *iObject)
    {
      return scfParent->RegisterPlugin (iClassID, iFuncID, iObject);
    }
    virtual int GetPluginCount ()
    {
      return scfParent->GetPluginCount ();
    }
    virtual iBase* GetPlugin (int idx)
    {
      return scfParent->GetPlugin (idx);
    }
  } scfiPluginManager;
  friend class PluginManager;

  //------------------------------------------------------------------

  class VirtualClock : public iVirtualClock
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSystemDriver);
    virtual void Advance () { scfParent->AdvanceVirtualTimeClock (); }
    virtual void Suspend () { scfParent->SuspendVirtualTimeClock (); }
    virtual void Resume () { scfParent->ResumeVirtualTimeClock (); }
    virtual csTicks GetElapsedTicks () const
    {
      csTicks oElapsedTime, oCurrentTime;
      scfParent->GetElapsedTime (oElapsedTime, oCurrentTime);
      return oElapsedTime;
    }
    virtual csTicks GetCurrentTicks () const
    {
      csTicks oElapsedTime, oCurrentTime;
      scfParent->GetElapsedTime (oElapsedTime, oCurrentTime);
      return oCurrentTime;
    }
  } scfiVirtualClock;
  friend class VirtualClock;

  //------------------------------------------------------------------

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSystemDriver);
    virtual bool Initialize (iObjectRegistry*) { return true; }
  } scfiComponent;
  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(csSystemDriver);
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiEventHandler;
};

/// CS version of printf
extern int csPrintf (const char* str, ...);
/// CS version of vprintf
extern int csPrintfV (const char* str, va_list arg);

/// Get the current tick count.
extern csTicks csGetTicks ();

/**
 * Get the installation path.<p>
 * This returns the path where the system has been installed to.
 * It has a limited use because mostly everything should be done
 * through VFS which is installation directory - independent; but
 * some initialization tasks still need this.
 */
extern bool csGetInstallPath (char *oInstallPath, size_t iBufferSize);

#endif // __CS_SYSTEM_H__
