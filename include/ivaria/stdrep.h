/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_IVARIA_STDREP_H__
#define __CS_IVARIA_STDREP_H__

/**\file
 * Standard reporter listener interface
 */

#include "csutil/scf.h"

struct iConsoleOutput;
struct iNativeWindowManager;
struct iReporter;

SCF_VERSION (iStandardReporterListener, 0, 0, 3);

/**
 * Interface to control the settings of the reporter listener plugin.
 * 
 * Main creators of instances implementing this interface:
 * - Standard Reporter Listener plugin (crystalspace.utilities.stdrep)
 *   
 * Main ways to get pointers to this interface:
 * - csQueryRegistry()
 */
struct iStandardReporterListener : public iBase
{
  /// Set the output console to use.
  virtual void SetOutputConsole (iConsoleOutput* console) = 0;
  /// Set the native window manager to use.
  virtual void SetNativeWindowManager (iNativeWindowManager* wm) = 0;
  /// Set the reporter to use.
  virtual void SetReporter (iReporter* rep) = 0;
  /** 
    * Set the debug file to use (standard filename).
    * If append is true the debug file name is appended to instead of a new 
    * one created.
    */  
  virtual void SetDebugFile (const char* filename, bool append=false) = 0;
  /**
   * Set useful defaults for output console, native window manager,
   * reporter (will use iObjectRegistry to query for those). The
   * debug file will be 'debug.txt'
   */
  virtual void SetDefaults () = 0;

  /**
   * Control where some type of message (severity level from the
   * reporter plugin: CS_REPORTER_...) will go to. Several of these
   * flags can be on at the same time or none if you just want to ignore
   * some message.
   */
  virtual void SetMessageDestination (int severity,
  	bool do_stdout, bool do_stderr, bool do_console,
	bool do_alert, bool do_debug, bool do_popup = false) = 0;

  /**
   * Control if this reporter listener should remove messages of a certain
   * severity. By default all messages are removed.
   */
  virtual void RemoveMessages (int severity, bool remove) = 0;

  /**
   * Control if the reporter should show message id as well. By default
   * this is only done for fatal, bug, and debug severity levels.
   */
  virtual void ShowMessageID (int severity, bool showid) = 0;

  /// Get the debug file name (or null if a debug file is not used).
  virtual const char* GetDebugFile () = 0;
};

#endif // __CS_IVARIA_STDREP_H__

