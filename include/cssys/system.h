/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#include <stdio.h>

#include "csutil/util.h"
#include "csutil/csstrvec.h"
#include "csutil/csobjvec.h"
#include "csutil/typedvec.h"
#include "cssys/csinput.h"
#include "cssys/csevcord.h"
#include "isys/system.h"
#include "isys/vfs.h"
#include "isys/event.h"
#include "iutil/config.h"

class csKeyboardDriver;
class csMouseDriver;

struct iGraphics3D;
struct iGraphics2D;
struct iConfig;
struct iConsoleOutput;
struct iConfigManager;

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
  /*
   * This is a private structure used to keep the list of plugins.
   */
  class csPlugin
  {
  public:
    // The plugin itself
    iPlugin *Plugin;
    // The class ID of the plugin, and their functionality ID
    char *ClassID, *FuncID;
    // The mask of events this plugin wants to see
    unsigned int EventMask;

    // Construct the object that represents a plugin
    csPlugin (iPlugin *iObject, const char *iClassID, const char *iFuncID);
    // Free storage
    virtual ~csPlugin ();
  };

  /*
   * This is a superset of csVector that can find by pointer a plugin.
   */
  class csPlugInsVector : public csVector
  {
  public:
    // Create the vector
    csPlugInsVector (int iLimit, int iDelta) : csVector (iLimit, iDelta) {}
    // Find a plugin either by its address or by his function ID
    virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const
    {
      if (Mode == 0)
        return ((csPlugin *)Item)->Plugin == Key ? 0 : 1;
      else
        return ((csPlugin *)Item)->FuncID ? strcmp (((csPlugin *)Item)->FuncID, (char *)Key)
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

    csPluginOption (const char *iName, csVariantType iType, int iID, bool iValue, iConfig* iConfig)
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

  /*
   * A private class which implements the iEventOutlet interface.
   */
  class csEventOutlet : public iEventOutlet
  {
    // The mask of events to allow from this plug
    unsigned EnableMask;
    // The event plug object
    iEventPlug *Plug;
    // The system driver
    csSystemDriver *System;
  public:
    SCF_DECLARE_IBASE;

    // Initialize the outlet
    csEventOutlet (iEventPlug *iPlug, csSystemDriver *iSys);
    // Destroy the outlet
    virtual ~csEventOutlet ();

    // Create a event object on behalf of the system driver.
    virtual iEvent *CreateEvent ();
    // Put a previously created event into system event queue.
    virtual void PutEvent (iEvent *Event);
    // Put a keyboard event into event queue.
    virtual void Key (int iKey, int iChar, bool iDown);
    // Put a mouse event into event queue.
    virtual void Mouse (int iButton, bool iDown, int x, int y);
    // Put a joystick event into event queue.
    virtual void Joystick (int iNumber, int iButton, bool iDown, int x, int y);
    // Put a broadcast event into event queue.
    virtual void Broadcast (int iCode, void *iInfo);
    // Broadcast a event to all plugins
    virtual void ImmediateBroadcast (int iCode, void *iInfo);
  };
  friend class csEventOutlet;

  /*
   * The array of all allocated event outlets.
   */
  class csEventOutletsVector : public csVector
  {
  public:
    csEventOutletsVector () : csVector (16, 16)
    { }
    virtual ~csEventOutletsVector ()
    { DeleteAll (); }
    virtual bool FreeItem (csSome Item)
    { delete (csEventOutlet *)Item; return true; }
    csEventOutlet *Get (int idx)
    { return (csEventOutlet *)csVector::Get (idx); }
  } EventOutlets;

  /*
   * The array of all allocated event cords.
   */
  class csEventCordsVector : public csVector
  {
  public:
    csEventCordsVector () : csVector (16, 16)
    { }
    virtual ~csEventCordsVector ()
    { DeleteAll (); }
    virtual bool FreeItem (csSome Item)
    { delete (csEventCord *)Item; return true; }
    csEventCord *Get (int idx)
    { return (csEventCord *)csVector::Get (idx); }
    int Find (int Category, int SubCategory);
  } EventCords;

  // Query all options supported by given plugin and place into OptionList
  void QueryOptions (iPlugin *iObject);

  // Elapsed time between last two frames and absolute time in milliseconds
  csTime ElapsedTime, CurrentTime;
  
protected:
  /// The command line parser
  iCommandLineParser *CommandLine;
  /// The configuration manager
  iConfigManager *Config;
  /// The Virtual File System object
  iVFS *VFS;
  /// 3D Graphics context
  iGraphics3D* G3D;
  /// 2D Graphics context
  iGraphics2D* G2D;
  /// System console
  iConsoleOutput *Console;

public:
  /// -------------------------- plug-ins --------------------------

  /// The list of all plug-ins
  csPlugInsVector Plugins;

  /// the width of this frame
  int FrameWidth;
  /// the height of this frame
  int FrameHeight;
  /// whether this device is full-screen or not.
  bool FullScreen;
  /// the bits-per-pixel of the display.
  int Depth;
  /// The event queue
  csEventQueue EventQueue;
  /// Keyboard driver
  csKeyboardDriver Keyboard;
  /// Mouse driver
  csMouseDriver Mouse;
  /// Joystick driver
  csJoystickDriver Joystick;
  /// Set to non-zero to exit csSystemDriver::Loop()
  bool Shutdown;
  /// Debugging level (0 = no debug, 1 = normal debug, 2 = verbose debug)
  int debug_level;
  /// List of all options for all plug-in modules.
  CS_DECLARE_TYPED_VECTOR (csOptionVector, csPluginOption) OptionList;

  /// Initialize system-dependent data
  csSystemDriver ();
  /// Deinitialize system-dependent parts
  virtual ~csSystemDriver ();

  /// This is usually called right after object creation.
  virtual bool Initialize (int argc, const char* const argv[], const char *iConfigName);

  /**
   * Open the graphics context (with optional title on titlebar),
   * mouse and keyboard.
   */
  virtual bool Open (const char *Title);
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

  /// Pass a single event to all plugins until one eats it
  virtual bool HandleEvent (iEvent &Event);

  /// Sleep for given number of 1/1000 seconds (very inacurate)
  virtual void Sleep (int /*SleepTime*/) {}

  /**
   * This is a function that prints the commandline help text.
   * If system driver has system-dependent switches, it should override
   * this method and type its own text (possibly after invoking
   * csSystemDriver::Help() first).
   */
  virtual void Help ();

  /**
   * Return time in milliseconds (if not supported by a system
   * just return the time in seconds multiplied by 1000).
   */
  static csTime Time ();

  /// Get the installation path.
  static bool InstallPath (char *oInstallPath, size_t iBufferSize);

  /// Print a string into debug.txt
  static void DebugTextOut (bool flush, const char* str);

  /**
   * Printf version that works on all systems.
   * Default implementation is in 'libs/cssys/general/'.
   */
  static void ConsoleOut  (const char *str);

  /// A shortcut for requesting to load a plugin (before Initialize())
  void RequestPlugin (const char *iPluginName);

protected:
  /**
   * Print help for an iConfig interface.
   */
  void Help (iConfig* Config);

  /**
   * Show an alert. This function is called by CsPrintf and
   * should show an alert in case of an error.
   * The default implementation writes the message in debug.txt
   * and on standard output.
   */
  virtual void Alert (const char* msg);

  /**
   * Show a warning. This function is called by CsPrintf and
   * should show an alert in case of a warning.
   * The default implementation writes a message in debug.txt
   * and on standard output.
   */
  virtual void Warn (const char* msg);

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
  virtual iConfigFile *OpenUserConfig(const char *ApplicationID, const char *Alias);

public:
  SCF_DECLARE_IBASE;

  /**************************** iSystem interface ****************************/

  /// Returns the configuration.
  virtual void GetSettings (int &oWidth, int &oHeight, int &oDepth, bool &oFullScreen);
  /// Get the time in milliseconds.
  virtual csTime GetTime ();
  /**
   * Print a string to the specified device.  This is implemented as a thin
   * wrapper over PrintfV().
   */
  virtual void Printf (int mode, char const* format, ...);
  /**
   * Print a string to the specified device.  Since Printf() is just a thin
   * wrapper over this method, most subclasses which need to provide special
   * extensions should override PrintfV() rather than Printf()
   */
  virtual void PrintfV (int mode, char const* format, va_list);
  /**
   * Execute a system-dependent extension command.  This is implemented as a
   * thin wrapper over PerformExtensionV().
   */
  virtual bool PerformExtension (char const* command, ...);
  /**
   * Execute a system-dependent extension command.  Since PerformExtension() is
   * just a thin wrapper over this method, most subclasses which need to
   * provide special extensions should override PerformExtensionV() rather than
   * PerformExtension().
   */
  virtual bool PerformExtensionV (char const* command, va_list);
  /// Suspend the engine's virtual-time clock.
  virtual void SuspendVirtualTimeClock() {}
  /// Resume the engine's virtual-time clock.
  virtual void ResumeVirtualTimeClock() { CurrentTime = csTime(-1); }
  /// Query the elapsed time between last frames and absolute time.
  virtual void GetElapsedTime (csTime &oElapsedTime, csTime &oCurrentTime)
  { oElapsedTime = ElapsedTime; oCurrentTime = CurrentTime; }
  /// Get the installation path.
  virtual bool GetInstallPath (char *oInstallPath, size_t iBufferSize);

  /// Load a plugin and initialize it
  virtual iBase *LoadPlugin (const char *iClassID, const char *iFuncID,
    const char *iInterface, int iVersion);
  /// Get first of the loaded plugins that supports given interface ID
  virtual iBase *QueryPlugin (const char *iInterface, int iVersion);
  /// Find a plugin given his functionality ID
  virtual iBase *QueryPlugin (const char *iFuncID, const char *iInterface, int iVersion);
  /// Find a plugin given his class ID and functionality ID
  virtual iBase *QueryPlugin (const char* iClassID, const char *iFuncID, const char *iInterface, int iVersion);
  /// Remove a plugin from system driver's plugin list
  virtual bool UnloadPlugin (iPlugin *iObject);
  /// Register a object that implements the iPlugin interface as a plugin
  virtual bool RegisterPlugin (const char *iClassID, const char *iFuncID, iPlugin *iObject);
  /// Get the number of loaded plugins in the plugin manager.
  virtual int GetPluginCount ();
  /// Get the specified plugin from the plugin manager.
  virtual iBase* GetPlugin (int idx);

  /// Get the system configuration file: this does NOT IncRef the object
  virtual iConfigManager *GetConfig ();
  /**
   * Add a config file to the global config manager (convenience method).
   * The returned config file is the newly loaded file. You must keep the
   * pointer to call RemoveConfig() later.
   */
  virtual iConfigFile *AddConfig(const char *iFileName,
    bool iVFS = true, int Priority = ConfigPriorityPlugIn);
  /// Remove a config file that was added with AddConfig()
  virtual void RemoveConfig(iConfigFile *ConfigFile);
  /**
   * Create a new configuration file object which resides on VFS without
   * adding it to the config manager.
   */
  virtual iConfigFile *CreateSeparateConfig (const char *iFileName, bool iVFS = true);
  /// Save system configuration file if it was changed
  virtual bool SaveConfig ();

  /// Register the plugin to receive specific events
  virtual bool CallOnEvents (iPlugin *iObject, unsigned int iEventMask);
  /// Query current state for given key
  virtual bool GetKeyState (int key);
  /// Query current state for given mouse button (0..9)
  virtual bool GetMouseButton (int button);
  /// Query current (last known) mouse position
  virtual void GetMousePosition (int &x, int &y);
  /// Query current state for given joystick button (1..CS_MAX_JOYSTICK_BUTTONS)
  virtual bool GetJoystickButton (int number, int button);
  /// Query last known joystick position
  virtual void GetJoystickPosition (int number, int &x, int &y);
  /// Register an event plug and return a new outlet
  virtual iEventOutlet *CreateEventOutlet (iEventPlug *iObject);
  /// Get an event cord for the given category/subcategory.
  virtual iEventCord *GetEventCord(int Category, int Subcategory);
  /// Get a public event outlet for posting just a single event and such.
  virtual iEventOutlet *GetSystemEventOutlet ();

  /// Return the command line parser
  virtual iCommandLineParser *GetCommandLine ();
};

// Fatal exit routine (which can be replaced if neccessary)
extern void (*fatal_exit) (int errorcode, bool canreturn);

#endif // __CS_SYSTEM_H__
