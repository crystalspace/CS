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

#include "cscom/com.h"

extern const IID IID_ISystem;

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

/**
 * This interface serves as a way for plug-ins to query Crystal Space about settings,
 * It also serves as a way for plug-ins to print through Crystal Space's printing
 * interfaces.
 */
interface ISystem : public IUnknown
{
  /// returns the width requested by the configuration.
  STDMETHOD (GetWidthSetting) (int& retval) PURE;
  /// returns the height requested by the configuration.
  STDMETHOD (GetHeightSetting) (int& retval) PURE;
  /// get the full screen setting
  STDMETHOD (GetFullScreenSetting) (bool& retval) PURE;
  /// get the depth setting
  STDMETHOD (GetDepthSetting) (int& retval) PURE;
  /// Get a subsystem pointer
  STDMETHOD (GetSubSystemPtr) (void **retval, int iSubSystemID) PURE;
  /// print a string to the specified device.
  STDMETHOD (Print) (int mode, const char *string) PURE;
  /// open a file in a system independent way (translating directory '/' if needed).
  STDMETHOD (FOpen) (const char* filename, const char* mode, FILE** fp) PURE;
  /// close a file in a system independent way.
  STDMETHOD (FClose) (FILE* fp) PURE;
  /// get the time in milliseconds.
  STDMETHOD (GetTime) (time_t& time) PURE;
  /// quit the system.
  STDMETHOD (Shutdown) () PURE;
  /// check if system is shutting down
  STDMETHOD (GetShutdown) (bool &Shutdown) PURE;
  /// Get a integer configuration value
  STDMETHOD (ConfigGetInt) (char *Section, char *Key, int &Value, int Default = 0) PURE;
  /// Get a string configuration value
  STDMETHOD (ConfigGetStr) (char *Section, char *Key, char *&Value, char *Default = NULL) PURE;
  /// Get a string configuration value
  STDMETHOD (ConfigGetYesNo) (char *Section, char *Key, bool &Value, bool Default = false) PURE;
};

#endif
