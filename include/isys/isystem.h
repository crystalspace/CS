/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein

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

#ifndef __CS_ISYSTEM_H__
#define __CS_ISYSTEM_H__

#include "csutil/scf.h"
#include "iutil/icfgmgr.h"

/*
 * Several types of messages.
 * The type of message is indicated when you call iSystem::Printf().
 * Depending on message type, the message will be put into debug log,
 * will be output to the console and so on.
 */
/// Internal error, this is a serious bug in CS
#define MSG_INTERNAL_ERROR	1
/// Error which prevents proper further execution
#define MSG_FATAL_ERROR		2
/// Error which doesn't prevent proper execution
#define MSG_WARNING		3
/// Initialization message
#define MSG_INITIALIZATION	4
/// Message intended for display on the console
#define MSG_CONSOLE		5
/// Message intended for display on stdout
#define MSG_STDOUT		6
/// Show message if debug level is 0, 1, or 2 (no debug)
#define MSG_DEBUG_0		7
/// Show message if debug level is 1 or 2 (debug mode)
#define MSG_DEBUG_1		8
/// Show message if debug level 2 (verbose mode)
#define MSG_DEBUG_2		9
/// Show message if debug level is 0, 1, or 2 (no debug) and flush
#define MSG_DEBUG_0F		10
/// Show message if debug level is 1 or 2 (debug mode) and flush
#define MSG_DEBUG_1F		11
/// Show message if debug level 2 (verbose mode) and flush
#define MSG_DEBUG_2F		12

/*
 * Plugins have an additional characteristic called "functionality ID".
 * This identifier is used by other plugins/engine/system driver to
 * find some plugin that user assigned to perform some function.
 * For example, the "VideoDriver" functionality identifier is used to
 * separate the main 3D graphics driver from other possibly loaded
 * driver that also implements the iGraphics3D interface (it could be
 * the secondary video driver for example).
 *<p>
 * The functionality ID is given in the system config file as left
 * side of assignment in "PlugIns" section. For example, in the
 * following line:
 * <pre>
 * [PlugIns]
 * VideoDriver = crystal.graphics3d.software
 * </pre>
 * "VideoDriver" is functionality identifier, and "crystal.graphics3d.software"
 * is SCF class ID. No two plugins can have same functionality ID. When you
 * load a plugin with System::RequestPlugin() you can define functionality ID
 * after a double colon:
 * <pre>
 * System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
 * </pre>
 * If you load a plugin via the ::LoadPlugIn method you just specify the
 * functionality ID as one of arguments.
 */

/// VFS plugin functionality identifier
#define CS_FUNCID_VFS		"VFS"
/// Functionality ID for the video driver
#define CS_FUNCID_VIDEO		"VideoDriver"
/// Canvas plugin funcID (AKA 2D graphics driver)
#define CS_FUNCID_CANVAS	"VideoCanvas"
/// Sound renderer
#define CS_FUNCID_SOUND		"SoundRender"
/// Font server
#define CS_FUNCID_FONTSERVER	"FontServer"
/// Network driver
#define CS_FUNCID_NETDRV	"NetDriver"
/// Console
#define CS_FUNCID_CONSOLE	"Console"
/// 3D engine
#define CS_FUNCID_ENGINE	"Engine"
/// Skeleton Animation
#define CS_FUNCID_MOTION	"MotionManager"

/**
 * Query a pointer to some plugin through the System interface.
 * `Object' is a object that implements iSystem interface.
 * `Interface' is a interface name (iGraphics2D, iVFS and so on).
 */
#define QUERY_PLUGIN(Object,Interface)					\
  (Interface *)(Object)->QueryPlugIn (#Interface, VERSION_##Interface)

/**
 * Find a plugin by his functionality ID. First the plugin with requested
 * functionality identifier is found, and after this it is queried for the
 * respective interface; if it does not implement the requested interface,
 * NULL is returned. NULL is also returned if the plugin with respective
 * functionality is simply not found. If you need the plugin with given
 * functionality identifier no matter which interface it implements, ask
 * for some basic interface, say iBase or iPlugIn.
 */
#define QUERY_PLUGIN_ID(Object,FuncID,Interface)			\
  (Interface *)(Object)->QueryPlugIn (FuncID, #Interface, VERSION_##Interface)

/**
 * Find a plugin by his class ID and functionality ID. First the plugin
 * with requested class identifier and functionality identifier is found,
 * and after this it is queried for the respective interface; if it does
 * not implement the requested interface, NULL is returned. NULL is also
 * returned if the plugin with respective functionality is simply not
 * found. If you need the plugin with given functionality identifier no
 * matter which interface it implements, ask for some basic interface,
 * say iBase or iPlugIn.
 */
#define QUERY_PLUGIN_CLASS(Object,ClassID,FuncID,Interface)			\
  (Interface *)(Object)->QueryPlugIn (ClassID, FuncID, #Interface, VERSION_##Interface)

/**
 * Tell system driver to load a plugin.
 * Since SCF kernel is hidden behind the iSystem driver, this is the only
 * way to load a plugin from another plugin.
 * `Object' is a object that implements iSystem interface.
 * `ClassID' is the class ID (`crystalspace.graphics3d.software').
 * `Interface' is a interface name (iGraphics2D, iVFS and so on).
 */
#define LOAD_PLUGIN(Object,ClassID,FuncID,Interface)			\
  (Interface *)(Object)->LoadPlugIn (ClassID, FuncID, #Interface, VERSION_##Interface)

/**
 * Same as LOAD_PLUGIN but don't bother asking for a interface.
 * This is useful for unconditionally loading plugins.
 */
#define _LOAD_PLUGIN(Object,ClassID,FuncID)				\
  (Object)->LoadPlugIn (ClassID, FuncID, NULL, 0)

struct iPlugIn;
struct iVFS;
struct iEventOutlet;
struct iEventPlug;
struct iEventCord;
struct iStrVector;
struct iConfigFile;
struct iConfigManager;

SCF_VERSION (iSystem, 4, 0, 1);

/**
 * This interface serves as a way for plug-ins to query Crystal Space about
 * miscelaneous settings. It also serves as a way for plug-ins to print
 * through Crystal Space's printing interfaces.
 *<p>
 * Notes on plugin support: the list of plugins is queried from the [PlugIns]
 * section in the config file. The plugins are loaded in the order they come
 * in the .cfg file.
 *<p>
 * The plugin can insert itself into the event processing chain and perform
 * some actions depending on events. It also can supply an private independent
 * API but in this case client program should know this in advance.
 */
struct iSystem : public iBase
{
  //---------------------------- Initialization ------------------------------//

  /**
   * Initialize the system. Sort all plugins with respect to their
   * dependencies. Then load all plugins and initialize them.
   */
  virtual bool Initialize (int argc, const char* const argv[], const char *iConfigName) = 0;

  /**
   * Open the graphics context (with optional title on titlebar),
   * mouse and keyboard.
   */
  virtual bool Open (const char *Title) = 0;
  /// Close the system
  virtual void Close () = 0;

  /**
   * System loop.
   * This function returns only when an cscmdQuit or an cscmdQuitLoop
   * broadcast is encountered.
   */
  virtual void Loop () = 0;

  /**
   * The ::Loop method calls this method once per frame.
   * This method can be called manually as well if you don't use the
   * Loop method.
   */
  virtual void NextFrame () = 0;

  //------------------------------ Miscellaneous -----------------------------//

  /// Returns the basic configuration parameters.
  virtual void GetSettings (int &oWidth, int &oHeight, int &oDepth, bool &oFullScreen) = 0;
  /// Get the time in milliseconds.
  virtual cs_time GetTime () = 0;
  /// Print a string to the specified device.
  virtual void Printf (int mode, const char *format, ...) = 0;

  /**
   * Execute a system-dependent extension.<p>
   * Sometimes we need just one extra function in system-dependent system
   * driver, which is called, say, from canvas driver (such as EnablePrintf
   * in DJGPP port of CS). In such cases it doesn't have much sense to create
   * a new SCF interface and so on, better just override this function and
   * use it.
   */
  virtual bool SystemExtension (const char *iCommand, ...) = 0;

  /**
   * Suspend the engine's virtual-time clock.<p>
   * Call this function when the client application will fail to invoking
   * NextFrame() for an extended period of time.  Suspending the virtual-time
   * clock prevents a temporal anomaly from occurring the next time
   * GetElapsedTime() is called after the application resumes invoking
   * NextFrame().
   */
  virtual void SuspendVirtualTimeClock() = 0;

  /**
   * Resume the engine's virtual-time clock.<p>
   * Call this function when the client application begins invoking NextFrame()
   * again after extended down-time.  This function is the complement of
   * SuspendVirtualTimeClock().
   */
  virtual void ResumeVirtualTimeClock() = 0;

  /**
   * Query the time elapsed between previous call to NextFrame and last
   * call to NextFrame(). Also returns the absolute time of the last call
   * to NextFrame(). The time is updated once at the beginning of every
   * NextFrame(), thus you may call this function as much as you wish.
   */
  virtual void GetElapsedTime (cs_time &oElapsedTime, cs_time &oCurrentTime) = 0;

  /**
   * This function will freeze your application for given number of 1/1000
   * seconds. The function is very inaccurate, so don't use it for accurate
   * timing. It may be useful when the application is idle, to explicitly
   * release CPU for other tasks in multi-tasking operating systems.
   */
  virtual void Sleep (int iSleepTime) = 0;

  /**
   * Get the installation path.<p>
   * This returns the path where the system has been installed to.
   * It has a limited use because mostly everything should be done
   * through VFS which is installation directory - independent; but
   * some initialization tasks still need this.
   */
  virtual bool GetInstallPath (char *oInstallPath, size_t iBufferSize) = 0;

  //---------------------------- Plug-in manager -----------------------------//

  /// Load a plugin and initialize it
  virtual iBase *LoadPlugIn (const char *iClassID, const char *iFuncID,
    const char *iInterface = NULL, int iVersion = 0) = 0;
  /// Get first of the loaded plugins that supports given interface ID
  virtual iBase *QueryPlugIn (const char *iInterface, int iVersion) = 0;
  /// Find a plugin given his functionality ID
  virtual iBase *QueryPlugIn (const char *iFuncID, const char *iInterface, int iVersion) = 0;
  /// Find a plugin given his class ID and functionality ID
  virtual iBase *QueryPlugIn (const char* iClassID, const char *iFuncID, const char *iInterface, int iVersion) = 0;
  /// Remove a plugin from system driver's plugin list
  virtual bool UnloadPlugIn (iPlugIn *iObject) = 0;
  /// Register a object that implements the iPlugIn interface as a plugin
  virtual bool RegisterPlugIn (const char *iClassID, const char *iFuncID,
    iPlugIn *iObject) = 0;
  /// Get the number of loaded plugins in the plugin manager.
  virtual int GetNumPlugIns () = 0;
  /// Get the specified plugin from the plugin manager.
  virtual iBase* GetPlugIn (int idx) = 0;

  //----------------------- Configuration file interface ---------------------//

  /// Default priority values (you may use other values if you want)
  enum
  {
    // plug-in priority
    ConfigPriorityPlugIn        = iConfigManager::PriorityVeryLow,
    // application priority
    ConfigPriorityApplication   = iConfigManager::PriorityLow,
    // user priority (application-neutral)
    ConfigPriorityUserGlobal    = iConfigManager::PriorityMedium,
    // user priority (application-specific)
    ConfigPriorityUserApp       = iConfigManager::PriorityHigh,
    // command-line priority
    ConfigPriorityCmdLine       = iConfigManager::PriorityVeryHigh
  };
  /// Get the system configuration file: this does NOT IncRef the object
  virtual iConfigManager *GetConfig () = 0;
  /**
   * Add a config file to the global config manager (convenience method).
   * The returned config file is the newly loaded file. You must keep the
   * pointer to call RemoveConfig() later.
   */
  virtual iConfigFile *AddConfig(const char *iFileName,
    bool iVFS = true, int Priority = ConfigPriorityPlugIn) = 0;
  /// Remove a config file that was added with AddConfig()
  virtual void RemoveConfig(iConfigFile *ConfigFile) = 0;
  /**
   * Create a new configuration file object which resides on VFS without
   * adding it to the config manager.
   */
  virtual iConfigFile *CreateSeparateConfig (const char *iFileName, bool iVFS = true) = 0;
  /// Save system configuration file if it was changed
  virtual bool SaveConfig () = 0;

  //------------------------------ Event manager -----------------------------//

  /// Register the plugin to receive specific events
  virtual bool CallOnEvents (iPlugIn *iObject, unsigned int iEventMask) = 0;
  /// Query current state for given key
  virtual bool GetKeyState (int key) = 0;
  /// Query current state for given mouse button (1..CS_MAX_MOUSE_BUTTONS)
  virtual bool GetMouseButton (int button) = 0;
  /// Query current (last known) mouse position
  virtual void GetMousePosition (int &x, int &y) = 0;
  /// Query current state for given joystick button (1..CS_MAX_JOYSTICK_BUTTONS)
  virtual bool GetJoystickButton (int number, int button) = 0;
  /// Query last known joystick position
  virtual void GetJoystickPosition (int number, int &x, int &y) = 0;

  /**
   * Register an event plug and return a new outlet.<p>
   * Any plugin which generates events should consider using this interface
   * for doing it. The plugin should implement the iEventPlug interface,
   * then register that interface with the system driver. You will get an
   * iEventOutlet object which you should use to put any events into the
   * system event queue.
   */
  virtual iEventOutlet *CreateEventOutlet (iEventPlug *iObject) = 0;

  /**
   * Get the event cord for a given category and subcategory.<p>
   * This allows events to be delivered immediately to a chain
   * of plugins that register with the implementation of iEventCord
   * returned by this function.
   */
  virtual iEventCord *GetEventCord (int Category, int Subcategory) = 0;

  /**
   * Get a public event outlet for posting just a single event and such.
   *<p>
   * In general it is not advisory to post events through this public outlet;
   * instead it is advised you to create your own private outlet (through
   * CreateEventOutlet) and register as a normal event plug. However, there are
   * cases when you just need to post one event from time to time; in these
   * cases it is easier to post it without the bulk of creating your own
   * iEventPlug interface.
   *<p>
   * Note that the returned object is NOT IncRef'd, thus you should NOT
   * DecRef it after usage.
   */
  virtual iEventOutlet *GetSystemEventOutlet () = 0;

  //--------------------------- Command-line access --------------------------//

  /// Query a specific commandline option (you can query second etc such option)
  virtual const char *GetOptionCL (const char *iName, int iIndex = 0) = 0;
  /// Query a filename specified on the commandline (that is, without leading '-')
  virtual const char *GetNameCL (int iIndex = 0) = 0;
  /// Add a command-line option to the command-line option array
  virtual void AddOptionCL (const char *iName, const char *iValue) = 0;
  /// Add a command-line name to the command-line names array
  virtual void AddNameCL (const char *iName) = 0;
  /// Replace the Nth command-line option with a new value
  virtual bool ReplaceOptionCL (const char *iName, const char *iValue, int iIndex = 0) = 0;
  /// Replace the Nth command-line name with a new value
  virtual bool ReplaceNameCL (const char *iValue, int iIndex = 0) = 0;
};

#endif // __CS_ISYSTEM_H__
