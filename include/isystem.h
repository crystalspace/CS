/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Dan Ogles

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

#ifndef __ISYSTEM_H__
#define __ISYSTEM_H__

#include "csutil/scf.h"

/// Several types of messages.
#define MSG_INTERNAL_ERROR 1    // Internal error, this is a serious bug in CS
#define MSG_FATAL_ERROR 2       // Error which prevents proper further execution
#define MSG_WARNING 3           // Error which doesn't prevent proper execution
#define MSG_INITIALIZATION 4    // Initialization message
#define MSG_CONSOLE 5           // Message intended for display on the console
#define MSG_STDOUT 6            // Message intended for display on stdout
#define MSG_DEBUG_0 7           // Show message if debug level is 0, 1, or 2 (no debug)
#define MSG_DEBUG_1 8           // Show message if debug level is 1 or 2 (debug mode)
#define MSG_DEBUG_2 9           // Show message if debug level 2 (verbose mode)
#define MSG_DEBUG_0F 10         // Show message if debug level is 0, 1, or 2 (no debug) and flush
#define MSG_DEBUG_1F 11         // Show message if debug level is 1 or 2 (debug mode) and flush
#define MSG_DEBUG_2F 12         // Show message if debug level 2 (verbose mode) and flush

#define QUERY_PLUGIN(Object,Interface)					\
  (Interface *)Object->QueryPlugIn (#Interface, VERSION_##Interface)
#define LOAD_PLUGIN(Object,ClassID,Interface)				\
  (Interface *)Object->LoadPlugIn (ClassID, #Interface, VERSION_##Interface)

scfInterface iPlugIn;

/**
 * This interface serves as a way for plug-ins to query Crystal Space about settings,
 * It also serves as a way for plug-ins to print through Crystal Space's printing
 * interfaces.<p>
 * Notes on plugin support: the list of plugins is queried from the [PlugIns]
 * section in the config file. The plugins are loaded in the order they come
 * in the .cfg file. If some plugin wants to be registered as a video/sound/etc
 * driver, it should call the RegisterDriver method (see below) with the
 * "iGraphics3D", "iNetworkDriver" and so on strings as first parameter.
 * If registration fails, this mean that another such driver has already
 * been registered with the system driver. The plugin should not complain,
 * just quietely fail the Initialize() call. Upon this system driver will
 * quietly unload the plugin as unuseful.<p>
 * If the plugin won't register as a basical driver, it won't be called by
 * the engine otherwise than for the cases where plugin registered himself.
 */
SCF_INTERFACE (iSystem, 0, 0, 1) : public iBase
{
  /// returns the configuration.
  virtual void GetSettings (int &oWidth, int &oHeight, int &oDepth, bool &oFullScreen) = 0;
  /// Set one of basical drivers (plugins)
  virtual bool RegisterDriver (const char *iInterface, iPlugIn *iObject) = 0;
  /// Unload a driver
  virtual bool DeregisterDriver (const char *iInterface, iPlugIn *iObject) = 0;
  /// Load a plugin and initialize it
  virtual iBase *LoadPlugIn (const char *iClassID, const char *iInterface, int iVersion) = 0;
  /// Get first of the loaded plugins that supports given interface ID
  virtual iBase *QueryPlugIn (const char *iInterface, int iVersion) = 0;
  /// Remove a plugin from system driver's plugin list
  virtual bool UnloadPlugIn (iPlugIn *iObject) = 0;
  /// print a string to the specified device.
  virtual void Print (int mode, const char *string) = 0;
  /// get the time in milliseconds.
  virtual time_t GetTime () = 0;
  /// quit the system.
  virtual void StartShutdown () = 0;
  /// check if system is shutting down
  virtual bool GetShutdown () = 0;
  /// Get a integer configuration value
  virtual int ConfigGetInt (char *Section, char *Key, int Default = 0) = 0;
  /// Get a string configuration value
  virtual char *ConfigGetStr (char *Section, char *Key, char *Default = NULL) = 0;
  /// Get a string configuration value
  virtual bool ConfigGetYesNo (char *Section, char *Key, bool Default = false) = 0;
  /// Get a float configuration value
  virtual float ConfigGetFloat (char *Section, char *Key, float Default = 0) = 0;
  /// Set an integer configuration value
  virtual bool ConfigSetInt (char *Section, char *Key, int Value) = 0;
  /// Set an string configuration value
  virtual bool ConfigSetStr (char *Section, char *Key, char *Value) = 0;
  /// Set an float configuration value
  virtual bool ConfigSetFloat (char *Section, char *Key, float Value) = 0;
  /// Save system configuration file
  virtual bool ConfigSave () = 0;
  /// Put a keyboard event into event queue 
  virtual void QueueKeyEvent (int KeyCode, bool Down) = 0;
  /// Put a mouse event into event queue 
  virtual void QueueMouseEvent (int Button, int Down, int x, int y, int ShiftFlags) = 0;
  /// Put a focus event into event queue 
  virtual void QueueFocusEvent (bool Enable) = 0;
  /// Register the plugin to receive specific events
  virtual bool CallOnEvents (iPlugIn *iObject, unsigned int iEventMask) = 0;
  /// Query current state for given key
  virtual bool GetKeyState (int key) = 0;
  /// Query current state for given mouse button (0..9)
  virtual bool GetMouseButton (int button) = 0;
  /// Query current (last known) mouse position
  virtual void GetMousePosition (int &x, int &y) = 0;
};

#endif
