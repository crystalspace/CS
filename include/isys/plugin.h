/*
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __ISYS_PLUGIN_H__
#define __ISYS_PLUGIN_H__

#include "csutil/scf.h"

struct iSystem;
struct iEvent;

SCF_VERSION (iPlugIn, 0, 0, 1);

/**
 * This is the general plug-in interface for CS.
 * All plug-ins must implement this interface.
 * During Initialize() call plug-in should do all initialization stuff,
 * such as registering with the system driver and so on.
 */
struct iPlugIn : public iBase
{
  /// Initialize the plugin, and return success status
  virtual bool Initialize (iSystem *iSys) = 0;
  /**
   * This is plugin's event handler. Plugin should register first
   * with the system driver (using iSystem::CallOnEvents method)
   * before he'll receive any events. The handler should return true
   * if the event has been handled indeed (and thus to not pass it further).
   * The default implementation of HandleEvent does nothing.
   * NOTE: do NOT return true unless you really handled the event
   * and want the event to not be passed further for processing by
   * other plugins.
   */
  virtual bool HandleEvent (iEvent &/*Event*/)
  { return false; }
};

#endif // __ISYS_PLUGIN_H__
