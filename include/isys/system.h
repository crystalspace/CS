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

#ifndef __ISYS_SYSTEM_H__
#define __ISYS_SYSTEM_H__

#include <stdarg.h>
#include "csutil/scf.h"
#include "iutil/cfgmgr.h"

struct iPlugin;
struct iObjectRegistry;
struct iVFS;
struct iEventOutlet;
struct iEventPlug;
struct iEventCord;
struct iStrVector;
struct iConfigFile;
struct iConfigManager;

//@@@ This is a temporary define that MUST be used to get the iSystem
// pointer from the object registry. Normally you can also use
// CS_QUERY_REGISTRY() but since we want to eventually remove iSystem
// we need to be able to find queries to iSystem fast so we can remove them
// too. That's why we put this in a seperate define that we will later remove.
#define CS_GET_SYSTEM(object_reg) CS_QUERY_REGISTRY(object_reg, iSystem)


SCF_VERSION (iSystem, 11, 0, 0);

/**
 * This interface serves as a way for plug-ins to query Crystal Space about
 * miscelaneous settings. It also serves as a way for plug-ins to print
 * through Crystal Space's printing interfaces.
 *<p>
 * Notes on plugin support: the list of plugins is queried from the [Plugins]
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
   * The ::Loop method calls this method once per frame.
   * This method can be called manually as well if you don't use the
   * Loop method.
   */
  virtual void NextFrame () = 0;

  //------------------------------ Miscellaneous -----------------------------//

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
  virtual void GetElapsedTime (csTicks &oElapsedTime, csTicks &oCurrentTime) = 0;

  /**
   * This function will freeze your application for given number of 1/1000
   * seconds. The function is very inaccurate, so don't use it for accurate
   * timing. It may be useful when the application is idle, to explicitly
   * release CPU for other tasks in multi-tasking operating systems.
   */
  virtual void Sleep (int iSleepTime) = 0;

  //---------------------------- Object Registry -----------------------------//

  /**
   * Get the global object registry (temporary function).
   */
  virtual iObjectRegistry* GetObjectRegistry () = 0;

  //------------------------------ Event manager -----------------------------//

  /// Register the plugin to receive specific events
  virtual bool CallOnEvents (iPlugin *iObject, unsigned int iEventMask) = 0;
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
};

#endif // __ISYS_SYSTEM_H__
