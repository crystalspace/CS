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
   * This function should be called first. It will create the object
   * registry and return a pointer to it. If there is a problem it will
   * return NULL.
   */
  static iObjectRegistry* CreateObjectRegistry ();

  /**
   * Request a few widely used standard plugins and also read
   * the config file/command line for potential other plugins.
   * This routine must be called before Initialize().
   */
  static bool RequestPlugins (iObjectRegistry* object_reg,
	const char* config_name,
	int argc, const char* const argv[],
	bool want_3d = true, bool want_engine = true,
	bool want_imgldr = true, bool want_lvlldr = true,
	bool want_fontsvr = true);

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

