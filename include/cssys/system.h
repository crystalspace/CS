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

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>

#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/csstrvec.h"
#include "csutil/csobjvec.h"
#include "cssys/csinput.h"
#include "iconfig.h"
#include "isystem.h"
#include "ivfs.h"
#include "iproto.h"
#include "iauth.h"

class csEventQueue;
class csKeyboardDriver;
class csMouseDriver;
class csIniFile;

struct iGraphics3D;
struct iGraphics2D;
struct iNetworkDriver;
struct iNetworkManager;
struct iSoundRender;
struct iConfig;
struct iConsole;

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
  // This is a private structure used to keep the list of plugins
  class csPlugIn : public csBase
  {
  public:
    // The plugin itself
    iPlugIn *PlugIn;
    // The class ID of the plugin, and their functionality ID
    char *ClassID, *FuncID;
    // The mask of events this plugin wants to see
    unsigned int EventMask;

    // Construct the object that represents a plugin
    csPlugIn (iPlugIn *iObject, const char *iClassID, const char *iFuncID);
    // Free storage
    virtual ~csPlugIn ();
  };

  // This is a superset of csObjVector that can find by pointer a plugin
  class csPlugInsVector : public csObjVector
  {
  public:
    // Create the vector
    csPlugInsVector (int iLimit, int iDelta) : csObjVector (iLimit, iDelta) {}
    // Find a plugin either by its address or by his function ID
    virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const
    {
      if (Mode == 0)
        return ((csPlugIn *)Item)->PlugIn == Key ? 0 : 1;
      else
        return ((csPlugIn *)Item)->FuncID ? strcmp (((csPlugIn *)Item)->FuncID, (char *)Key)
             : ((csPlugIn *)Item)->FuncID == Key ? 0 : 1;
    }
    // Overrided Get() to avoid typecasts
    csPlugIn *Get (int idx)
    { return (csPlugIn *)csObjVector::Get (idx); }
  };

  /// Class to collect all options for all plug-in modules in the system.
  class csPluginOption : public csBase
  {
  public:
    char *Name;
    csVariantType Type;
    int ID;
    bool Value;				// If Type is CSVAR_BOOL
    iConfig *Config;

    csPluginOption (const char *iName, csVariantType iType, int iID, bool iValue, iConfig* iConfig)
    {
      Name = strnew (iName);
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

  struct csCommandLineOption
  {
    /// Option name
    char *Name;
    /// Option value
    char *Value;
    /// Name and Value should be in same array (optimization)
    csCommandLineOption (char *iName, char *iValue)
    { Name = iName; Value = iValue; }
    /// Destructor: assume that "delete [] Name" will remove also Value
    ~csCommandLineOption ()
    { delete [] Name; }
  };

  class csCommandLineOptions : public csVector
  {
  public:
    csCommandLineOptions (int iLength, int iDelta) : csVector (iLength, iDelta) {}
    virtual bool FreeItem (csSome Item)
    { delete (csCommandLineOption *)Item; return true; }
    virtual int CompareKey (csSome Item, csConstSome Key, int /*Mode*/) const
    { return strcmp (((csCommandLineOption *)Item)->Name, (const char*)Key); }
  };

public:
  /// -------------------------- plug-ins --------------------------
  /// The list of all plug-ins
  csPlugInsVector PlugIns;
  /// The Virtual File System object
  iVFS *VFS;
  /// 3D Graphics context
  iGraphics3D* G3D;
  /// 2D Graphics context
  iGraphics2D* G2D;
  /// Sound render
  iSoundRender* Sound;
  /// Network driver
  iNetworkDriver* NetDrv;
  /// Network manager
  iNetworkManager* NetMan;
  /// Network User Protocol Layer
  iPROTO *NetProtocol;
  /// Network Command Manager
  iCMDMGR *CmdManager;
  /// System console
  iConsole *Console;
  /// Authentication method
  iAuth *Auth;  

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
  /// The Configuration File object
  csIniFile *Config;
  /// Set to non-zero to exit csSystemDriver::Loop()
  static bool Shutdown;
  /// Same as Shutdown but set manually by windowing system
  bool ExitLoop;
  /// Enable console output (used on systems where graphics screen is shared with text console)
  static bool EnableConsoleOutput;
  /// Debugging level (0 = no debug, 1 = normal debug, 2 = verbose debug)
  int debug_level;
  /// true if demo console is ready
  bool ConsoleReady;
  /// true if CrystalSpace visual is active (focused)
  bool IsFocused;
  /// List of all options for all plug-in modules.
  csObjVector OptionList;
  /// The collection of all options specified on command line
  csCommandLineOptions CommandLine;
  /// The list of raw filenames on the command line (i.e. without any switches)
  csStrVector CommandLineNames;
  /// System configuration file name
  char *ConfigName;

  /// Initialize system-dependent data
  csSystemDriver ();
  /// Deinitialize system-dependent parts
  virtual ~csSystemDriver ();

  /// This is usually called right after object creation.
  virtual bool Initialize (int argc, const char* const argv[], const char *iConfigName);

  /// Collect all options from command line
  virtual void CollectOptions (int argc, const char* const argv[]);

  /// Query all options supported by given plugin and place into OptionList
  void QueryOptions (iPlugIn *iObject);

  /// Check if all required drivers are loaded
  virtual bool CheckDrivers ();

  /**
   * Open the graphics context (with optional title on titlebar),
   * mouse and keyboard.
   */
  virtual bool Open (const char *Title);
  /// Close the system
  virtual void Close ();

  /**
   * System loop. This should be called last since it returns
   * only on program exit
   */
  virtual void Loop () = 0;

  /**
   * SysSystemDriver::Loop should call this method once per frame.
   * 'elapsed_time' is the time elapsed since the previous call to
   * NextFrame. 'current_time' is a global time counter.
   * The time is expressed in milliseconds.
   */
  virtual void NextFrame (time_t elapsed_time, time_t current_time);

  /// Called from NextFrame() to process all events
  virtual bool ProcessEvents ();

  /// Pass a single event to all plugins until one eats it
  virtual bool HandleEvent (csEvent &Event);

  /// Sleep for given number of 1/1000 seconds (very inacurate)
  virtual void Sleep (int /*SleepTime*/) {}

  /**
   * System dependent function to set all system defaults (like
   * the default resolution, if SHM is used or not (only for X),
   * and so on...).<p>
   * This routine can also make use of the config file to get
   * the defaults from there (so instead of having to specify
   * -mode 640x480 everytime you can set this in the config file
   * and this routine should read that).<p>
   * The routine can use GetOptionCL method as well to override
   * values that can be overrided from command line.<p>
   * System driver should implement only those options that are
   * common for absolutely all drivers of one kind. For example,
   * if your system does not support running in full screen, the
   * system driver can take care of the -windowpos <x>,<y> switch.
   * If at least for one driver this option is not applicable, you 
   * should implement the option inside each driver (call GetOptionCL
   * from plugin's Initialize() method).
   */
  virtual void SetSystemDefaults (csIniFile *config);

  /**
   * This is a function that prints the commandline help text.
   * If system has system-dependent switches, it should override
   * this method and type its own text (possibly invoking
   * csSystemDriver::Help() first).
   */
  virtual void Help ();

  /**
   * Return time in milliseconds (if not supported by a system
   * just return the time in seconds multiplied by 1000).
   * Default implementation is in 'def_sys'.
   */
  static long Time ();

  /// Print a string into debug.txt
  static void debug_out (bool flush, const char* str);

  /**
   * Printf version that works on all systems.
   * Default implementation is in 'libs/cssys/general/'.
   */
  static void console_open ();
  static void console_close ();
  static void console_out (const char *str);

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
   * Write a message on the display in demo mode (used by
   * Printf with mode MSG_INITIALIZATION).
   * The default implementation just uses the console.
   */
  virtual void DemoWrite (const char* msg);

  /**
   * This is a system independent function that just initilizes
   * FrameWidth and FrameHeight from the given mode string.
   */
  void SetMode (const char* mode);

public:
  DECLARE_IBASE;

  /**************************** iSystem interface ****************************/

  /// returns the configuration.
  virtual void GetSettings (int &oWidth, int &oHeight, int &oDepth, bool &oFullScreen);
  /// Load a plugin and initialize it
  virtual iBase *LoadPlugIn (const char *iClassID, const char *iFuncID,
    const char *iInterface, int iVersion);
  /// Get first of the loaded plugins that supports given interface ID
  virtual iBase *QueryPlugIn (const char *iInterface, int iVersion);
  /// Find a plugin given his functionality ID
  virtual iBase *QueryPlugIn (const char *iFuncID, const char *iInterface, int iVersion);
  /// Remove a plugin from system driver's plugin list
  virtual bool UnloadPlugIn (iPlugIn *iObject);
  /// print a string to the specified device.
  virtual void Printf (int mode, const char *format, ...);
  /// get the time in milliseconds.
  virtual time_t GetTime ();
  /// quit the system.
  virtual void StartShutdown ();
  /// check if system is shutting down
  virtual bool GetShutdown ();
  /// Get a VFS implementation if available
  virtual iVFS* GetVFS () const;
  /// Get a integer configuration value
  virtual int ConfigGetInt (const char *Section, const char *Key, int Default = 0);
  /// Get a string configuration value
  virtual const char *ConfigGetStr (const char *Section, const char *Key, const char *Default = NULL);
  /// Get a string configuration value
  virtual bool ConfigGetYesNo (const char *Section, const char *Key, bool Default = false);
  /// Get a float configuration value
  virtual float ConfigGetFloat (const char *Section, const char *Key, float Default = 0);
  /// Set an integer configuration value
  virtual bool ConfigSetInt (const char *Section, const char *Key, int Value);
  /// Set an string configuration value
  virtual bool ConfigSetStr (const char *Section, const char *Key, const char *Value);
  /// Set an float configuration value
  virtual bool ConfigSetFloat (const char *Section, const char *Key, float Value);
  /// Save system configuration file
  virtual bool ConfigSave ();
  /// Put a keyboard event into event queue 
  virtual void QueueKeyEvent (int iKeyCode, bool iDown);
  /// Put an extended keyboard event into event queue ( keycode is not translated in any way, thus
  /// the caller must take care to calculate the right code )
  virtual void QueueExtendedKeyEvent (int iKeyCode, int iKeyCodeTranslated, bool iDown);
  /// Put a mouse event into event queue 
  virtual void QueueMouseEvent (int iButton, bool iDown, int x, int y);
  /// Put a joystick event into event queue
  virtual void QueueJoystickEvent (int iNumber, int iButton, bool iDown, int x, int y);
  /// Put a focus event into event queue 
  virtual void QueueFocusEvent (bool Enable);
  /// Register the plugin to receive specific events
  virtual bool CallOnEvents (iPlugIn *iObject, unsigned int iEventMask);
  /// Query current state for given key
  virtual bool GetKeyState (int key);
  /// Query current state for given mouse button (0..9)
  virtual bool GetMouseButton (int button);
  /// Query current (last known) mouse position
  virtual void GetMousePosition (int &x, int &y);
  /// Query a specific commandline option (you can query second etc such option)
  virtual const char *GetOptionCL (const char *iName, int iIndex = 0);
  /// Query a filename specified on the commandline (that is, without leading '-')
  virtual const char *GetNameCL (int iIndex = 0);
  /// Add a command-line option to the command-line option array
  virtual void AddOptionCL (const char *iName, const char *iValue);
  /// Add a command-line name to the command-line names array
  virtual void AddNameCL (const char *iName);
  /// Called before forced suspend / after resuming suspend
  virtual void SuspendResume (bool iSuspend);
  /// Toggle console text output (for consoles that share text/graphics mode)
  virtual void EnablePrintf (bool iEnable);

  /****************************** iSCF interface ******************************/

  class csSCF : public iSCF
  {
  public:
    DECLARE_EMBEDDED_IBASE (csSystemDriver);

    /// Wrapper for scfClassRegistered ()
    virtual bool scfClassRegistered (const char *iClassID);
    /// Wrapper for scfCreateInstance ()
    virtual void *scfCreateInstance (const char *iClassID, const char *iInterfaceID,
      int iVersion);
    /// Wrapper for scfGetClassDescription ()
    virtual const char *scfGetClassDescription (const char *iClassID);
    /// Wrapper for scfGetClassDependencies ()
    virtual const char *scfGetClassDependencies (const char *iClassID);
    /// Wrapper for scfRegisterClass ()
    virtual bool scfRegisterClass (const char *iClassID, const char *iLibraryName,
      const char *Dependencies = NULL);
    /// Wrapper for scfRegisterStaticClass ()
    virtual bool scfRegisterStaticClass (scfClassInfo *iClassInfo);
    /// Wrapper for scfRegisterClassList ()
    virtual bool scfRegisterClassList (scfClassInfo *iClassInfo);
    /// Wrapper for scfUnregisterClass ()
    virtual bool scfUnregisterClass (char *iClassID);
  } scfiSCF;
};

// Shortcuts for compatibility
#define SysGetTime	csSystemDriver::Time

// CrystalSpace global variables
extern csSystemDriver *System;

// Fatal exit routine (which can be replaced if neccessary)
extern void (*fatal_exit) (int errorcode, bool canreturn);

#endif // SYSTEM_H
