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

#ifndef __CSINITAPP_H__
#define __CSINITAPP_H__

#include "cstypes.h"

struct iObjectRegistry;
struct iEvent;
struct iEventHandler;
struct iEventQueue;
struct iPluginManager;
struct iVirtualClock;
struct iCommandLineParser;
struct iConfigManager;

// Defines to select what plugins you want to have.
#define CS_PLUGIN_NONE 0
#define CS_PLUGIN_3D 1
#define CS_PLUGIN_ENGINE 2
#define CS_PLUGIN_LEVELLOADER 4
#define CS_PLUGIN_IMAGELOADER 8
#define CS_PLUGIN_FONTSERVER 16
#define CS_PLUGIN_DEFAULT (CS_PLUGIN_3D|CS_PLUGIN_ENGINE|CS_PLUGIN_FONTSERVER)
#define CS_PLUGIN_ALL (~0)

/**
 * Function to handle events for apps.
 */
typedef bool (csEventHandlerFunc) (iEvent&);

/**
 * This is a very general function that does a lot of the application
 * setup for you. It has to be called after system->Initialize() and will
 * setup various objects in the object registry.
 * returns true if everything went fine
 @@@ SOON OBSOLETE, USE THE NEW csInitializer CLASS.
 */
extern bool csInitializeApplication (iObjectRegistry* object_reg,
	bool use_reporter = true,
	bool use_reporter_listener = true);

/**
 * This class contains several static member functions that can help
 * setup an application. It is possible to do all the setup on your own
 * but using the functions below will help considerably.
 */
class csInitializer
{
public:
  /**
   * Create everything needed to get a CS application operational.
   * This function is completely equivalent to calling:
   * <ul>
   * <li>InitializeSCF()
   * <li>CreateObjectRegistry()
   * <li>CreatePluginManager()
   * <li>CreateEventQueue()
   * <li>CreateVirtualClock()
   * <li>CreateCommandLineParser()
   * <li>CreateConfigManager()
   * </ul>
   * This function will return the pointer to the object registry where
   * all the created objects will be registered.
   */
  static iObjectRegistry* CreateEnvironment ();

  /**
   * This very important function initializes the SCF sub-system.
   * Without this you can do almost nothing in CS.
   */
  static bool InitializeSCF ();

  /**
   * This function should be called second. It will create the object
   * registry and return a pointer to it. If there is a problem it will
   * return NULL.
   */
  static iObjectRegistry* CreateObjectRegistry ();

  /**
   * You will almost certainly want to call this function. It will
   * create the plugin manager which is essential for nearly everything.
   * The created plugin manager will be registered with the object registry
   * as the default plugin manager (using NULL tag).
   */
  static iPluginManager* CreatePluginManager (iObjectRegistry* object_reg);

  /**
   * This essential function creates the event queue which is the main
   * driving force between the event-driven CS model. In addition this function
   * will register the created event queue with the object registry as
   * the default event queue (using NULL tag).
   */
  static iEventQueue* CreateEventQueue (iObjectRegistry* object_reg);

  /**
   * Create the virtual clock. This clock is responsible for keeping
   * track of virtual time in the game system. This function will
   * register the created virtual clock with the object registry as the
   * default virtual clock (using NULL tag).
   */
  static iVirtualClock* CreateVirtualClock (iObjectRegistry* object_reg);

  /**
   * Create the commandline parser. This function will register the created
   * commandline parser with the object registry as the default
   * parser (using NULL tag).
   */
  static iCommandLineParser* CreateCommandLineParser (
  	iObjectRegistry* object_reg);

  /**
   * Create the config manager. This function will register the created
   * config manager with the object registry as the default config manager
   * (using NULL tag).
   */
  static iConfigManager* CreateConfigManager (iObjectRegistry* object_reg);

  /**
   * Request a few widely used standard plugins and also read
   * the config file/command line for potential other plugins.
   * This routine must be called before Initialize().
   */
  static bool RequestPlugins (iObjectRegistry* object_reg,
	const char* config_name,
	int argc, const char* const argv[],
	unsigned long want_plugins = CS_PLUGIN_DEFAULT);

  /**
   * Really initialize the application. This will initialize all loaded
   * plugins.
   */
  static bool Initialize (iObjectRegistry* object_reg);

  /**
   * Optionally load the reporter and the reporter listener.
   */
  static bool LoadReporter (iObjectRegistry* object_reg,
	bool use_reporter_listener = true);

  /**
   * Initialize the registry with all common plugins. The following
   * plugins are queried from the plugin manager and added as default
   * plugins (with NULL tag) to the object registry:
   * <ul>
   * <li>iVFS
   * <li>iGraphics3D
   * <li>iGraphics2D
   * <li>iFontServer
   * <li>iEngine
   * <li>iConsoleOutput
   * <li>iLoader
   * <li>iImageIO
   * <li>iReporter
   * <li>iStandardReporterListener
   * </ul>
   * Note that the plugin manager (iPluginManager) is already put
   * in the object registry earlier.
   */
  static bool SetupObjectRegistry (iObjectRegistry* object_reg);

  /**
   * Send the cscmdOpen command to all loaded plugins.
   * This should be done after initializing them (Initialize()).
   */
  static bool OpenApplication (iObjectRegistry* object_reg);

  /**
   * Send the cscmdClose command to all loaded plugins.
   */
  static void CloseApplication (iObjectRegistry* object_reg);

  /**
   * Initialize an event handler for the application. This is the
   * most general routine. This event handler will receive all events
   * that are sent through the event manager. Use this function to know
   * about keyboard, mouse and other events. Note that you also have to
   * use this function to be able to render something as rendering
   * happens as a result of one event (cscmdProcess).
   */
  static bool SetupEventHandler (iObjectRegistry* object_reg,
	iEventHandler* evhdlr, unsigned int eventmask);

  /**
   * Initialize an event handler function. This is an easier version
   * of SetupEventHandler() that takes a function and will register an
   * event handler to call that function for all relevant events.
   */
  static bool SetupEventHandler (iObjectRegistry* object_reg,
	csEventHandlerFunc* evhdlr_func);

  /**
   * Start the main event loop.
   */
  static bool MainLoop (iObjectRegistry* object_reg);

  /**
   * Destroy the application.
   */
  static void DestroyApplication ();
};


#endif // __CSINITAPP_H__

