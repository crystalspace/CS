/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    csObject library (C) 1999 by Ivan Avramovic <ivan@avramovic.com>
  
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
 * This is a very general function that does a lot of the application
 * setup for you. It has to be called after system->Initialize() and will
 * setup various objects in the object registry.
 * returns true if everything went fine
 @@@ SOON OBSOLETE, USE A FUNCTION BELOW!
 */
extern bool csInitializeApplication (iObjectRegistry* object_reg,
	bool use_reporter = true,
	bool use_reporter_listener = true);

/**
 * This function should be called first. It will create the object
 * registry and return a pointer to it. If there is a problem it will
 * return NULL.
 */
extern iObjectRegistry* csInitializeApplication ();

/**
 * Ask for a few widely used standard plugins and also read
 * the config file/command line for potential other plugins.
 * This routine must be called before csInitializeStartApp().
 */
extern bool csInitializeQueryPlugins (iObjectRegistry* object_reg,
	const char* config_name,
	int argc, const char* const argv[],
	bool want_3d = true, bool want_engine = true,
	bool want_imgldr = true, bool want_lvlldr = true,
	bool want_fontsvr = true);

/**
 * Really initialize the application. This will initialize all loaded
 * plugins.
 */
extern bool csInitializeStartApp (iObjectRegistry* object_reg);

/**
 * Optionally load the reporter and the reporter listener.
 */
extern bool csInitializeReporter (iObjectRegistry* object_reg,
	bool use_reporter_listener = true);

/**
 * Initialize the registry with all common plugins.
 */
extern bool csInitializeRegistry (iObjectRegistry* object_reg);

/**
 * Send the cscmdOpen command to all loaded plugins.
 */
extern bool csInitializeOpenApp (iObjectRegistry* object_reg);

/**
 * Function to handle events for apps.
 */
typedef bool (csEventHandlerFunc) (iEvent&);

/**
 * Initialize an event handler for the application. This is the
 * most general routine.
 */
extern bool csInitializeEventHandler (iObjectRegistry* object_reg,
	iEventHandler* evhdlr, unsigned int eventmask);

/**
 * Initialize an event handler function.
 */
extern bool csInitializeEventHandler (iObjectRegistry* object_reg,
	csEventHandlerFunc* evhdlr_func);

/**
 * Start the main event loop.
 */
extern bool csStartMainLoop (iObjectRegistry* object_reg);

/**
 * Destroy the application.
 */
extern void csDestroyApp ();


#endif // __CSINITAPP_H__

