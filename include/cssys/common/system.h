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

#include "cscom/com.h"
#include "iconfig.h"
#include "isystem.h"

class csEventQueue;
class csKeyboardDriver;
class csMouseDriver;
class csConsole;

interface IGraphics3D;
interface IGraphics2D;
interface IGraphicsInfo;
interface INetworkDriver;
interface INetworkManager;
interface ISoundRender;
interface IConfig;

/// Class to collect all options for all COM modules in the system.
class csColOption
{
public:
  csColOption* next;
  csVariantType type;
  int id;
  char* option_name;
  bool option_value;	// If type is CSVAR_BOOL
  IConfig* config;

  csColOption (csColOption* next, csVariantType type, int id, char* option_name, bool option_value, IConfig* config)
  {
    csColOption::next = next;
    csColOption::type = type;
    csColOption::id = id;
    CHK (csColOption::option_name = new char [strlen (option_name)+1]);
    strcpy (csColOption::option_name, option_name);
    csColOption::option_value = option_value;
    csColOption::config = config;
  }
  ~csColOption ()
  {
    CHK (delete next);
    CHK (delete [] option_name);
  }
};

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
 */
class csSystemDriver
{
public:
  /// Sound render
  ISoundRender* piSound;
  /// Program event queue
  csEventQueue *EventQueue;
  /// Network driver
  INetworkDriver* piNetDrv;
  /// Network manager
  INetworkManager* piNetMan;
  /// 3D Graphics context
  IGraphics3D* piG3D;
  /// 2D Graphics context
  IGraphics2D* piG2D;
  /// Graphics info context
  IGraphicsInfo* piGI;
  /// the width of this frame
  int FrameWidth;
  /// the height of this frame
  int FrameHeight;
  /// whether this device is full-screen or not.
  bool FullScreen;
  /// the bits-per-pixel of the display.
  int Depth;
  /// Keyboard driver
  csKeyboardDriver *Keyboard;
  /// Mouse driver
  csMouseDriver *Mouse;
  /// Set to non-zero to exit csSystemDriver::Loop()
  static bool Shutdown;
  /// Same as Shutdown but set manually by windowing system
  static bool ExitLoop;
  /// System console
  csConsole *Console;
  /// Interface for the engine configuration values.
  IConfig* cfg_engine;
  /// Debugging level (0 = no debug, 1 = normal debug, 2 = verbose debug)
  static int debug_level;
  /// true if demo console is ready
  static bool DemoReady;
  /// true if CrystalSpace visual is active (focused)
  bool IsFocused;
  /// List of all options for all COM modules.
  csColOption* com_options;

  /// Initialize system-dependent data
  csSystemDriver ();
  /// Deinitialize system-dependent parts
  virtual ~csSystemDriver ();

  /**
   * This is usually called right after object creation.
   * 'cfg_engine' is an instance of IConfig for the engine settings.
   */
  virtual bool Initialize (int argc, char *argv[], IConfig* cfg_engine);

  /// Initialize 3D Graphics context object
  virtual bool InitGraphics ();
  /// Initialize Keyboard object
  virtual bool InitKeyboard ();
  /// Initialize Mouse object
  virtual bool InitMouse ();
  /// Initialize Sound system
  virtual bool InitSound ();
  /// Initialize Network system
  virtual bool InitNetwork ();

  /**
   * Open the graphics context (with optional title on titlebar),
   * mouse and keyboard.
   */
  virtual bool Open (char *Title);
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
  virtual void NextFrame (long elapsed_time, long current_time);

  /// Sleep for given number of 1/1000 seconds (very inacurate)
  virtual void Sleep (int /*SleepTime*/) {}

  /**
   * System dependent function to set all system defaults (like
   * the default resolution, if SHM is used or not (only for X),
   * and so on...).<p>
   * This routine can also make use of the config file to get
   * the defaults from there (so instead of having to specify
   * -mode 640x480 everytime you can set this in the config file
   * and this routine should read that).
   */
  virtual void SetSystemDefaults ();

  /**
   * This is a system-independent function used to parse a
   * commandline. It will iterate all command-line arguments
   * through system-dependent ParsArg() method.
   */
  bool ParseCmdLine (int argc, char* argv[]);

  /**
   * This version is supposed to be called first and will
   * search all driver options on the commandline. This
   * allows the system to open the drivers first and only then
   * start parsing the rest of the options.
   */
  bool ParseCmdLineDriver (int argc, char* argv[]);

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

  /**
   * General printf routine. This routine should eventually
   * replace all other variants of printf that exist in
   * Crystal Space. 'mode' is any of the MSG_... fields
   * as defined above.
   */
  static void Printf (int mode, const char* str, ...);

  /// Print a string into debug.txt
  static void debugprintf (bool flush, const char* str, ...);

  /**
   * Printf version that works on all systems.
   * Default implementation is in 'def_sys'.
   */
  static void printf_init ();
  static void printf_close ();
  static int printf (const char *str, ...);

  /**
   * Open a file. This function first translates the directory
   * separator ('/' is used here) to whatever the system uses
   * and after that passes control to 'fopen'.
   * Default implementation of first function is in 'def_sys'.
   */
  static FILE* fopen (const char* filename, const char* mode);

protected:
  /**
   * This is a system-dependent function which eats a single
   * command-line option (like -help, ...). If system-dependent
   * part does not recognize the option, it should pass it to
   * its parent class method. Return false if error detected.
   */
  virtual bool ParseArg (int argc, char* argv[], int& i);

  /**
   * This is a system-dependent function which eats a single
   * command-line option for drivers.
   */
  virtual bool ParseArgDriver (int argc, char* argv[], int& i);

  /**
   * Print help for an IConfig interface.
   */
  void Help (IConfig* piConf);

  /**
   * Collect all options for a IConfig interface.
   */
  csColOption* CollectOptions (IConfig* config, csColOption* already_collected);

  /**
   * Show an alert. This function is called by csPrintf and
   * should show an alert in case of an error.
   * The default implementation writes the message in debug.txt
   * and on standard output.
   */
  virtual void Alert (const char* msg);

  /**
   * Show a warning. This function is called by csPrintf and
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
   * FRAME_WIDTH and FRAME_HEIGHT from the given mode string.
   */
  void SetMode (const char* mode);

  /**
   * System-dependent code should call this to emit 'focus changed' events.
   * Initially system assumes that CrystalSpace window is active.
   */
  void do_focus (int enable);

  // the COM ISystem interface implementation.
  class XSystem : public ISystem
  {
    DECLARE_IUNKNOWN()
    STDMETHODIMP GetDepthSetting(int& retval);
    STDMETHODIMP GetFullScreenSetting(bool& retval);
    STDMETHODIMP GetHeightSetting(int& retval);
    STDMETHODIMP GetWidthSetting(int& retval);
    STDMETHODIMP GetSubSystemPtr(void **retval, int iSubSystemID);
    STDMETHODIMP Print(int mode, const char* string);
    STDMETHODIMP FOpen (const char* filename, const char* mode, FILE** fp);
    STDMETHODIMP FClose (FILE* fp);
    STDMETHODIMP GetTime (time_t& time);
    STDMETHODIMP Shutdown ();
    STDMETHODIMP GetShutdown (bool &Shutdown);
    STDMETHODIMP (ConfigGetInt) (char *Section, char *Key, int &Value, int Default = 0);
    STDMETHODIMP (ConfigGetStr) (char *Section, char *Key, char *&Value, char *Default = NULL);
    STDMETHODIMP (ConfigGetYesNo) (char *Section, char *Key, bool &Value, bool Default = false);
    STDMETHODIMP (QueueKeyEvent) (int KeyCode, bool Down);
    STDMETHODIMP (QueueMouseEvent) (int Button, int Down, int x, int y, int ShiftFlags);
  };
  
  DECLARE_IUNKNOWN()
  DECLARE_INTERFACE_TABLE(csSystemDriver)
  DECLARE_COMPOSITE_INTERFACE_EMBEDDED (System);
};

#define GetISystemFromSystem(a) (&a->m_xSystem)

// Shortcuts for compatibility
#define pprintf_init	csSystemDriver::printf_init
#define pprintf_close	csSystemDriver::printf_close
#define pprintf		csSystemDriver::printf
#define SysOpen		csSystemDriver::fopen
#define SysGetTime	csSystemDriver::Time

class csIniFile;

// CrystalSpace global variables
extern csSystemDriver *System;
extern csIniFile *config;

// Fatal exit routine (which can be replaced if neccessary)
extern void (*fatal_exit) (int errorcode, bool canreturn);

#endif // SYSTEM_H
