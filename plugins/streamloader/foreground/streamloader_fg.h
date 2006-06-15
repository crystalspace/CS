/*
  Copyright (C) 2006 by David H. Bronke

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

#ifndef __CS_STREAMLOADER_H__
#define __CS_STREAMLOADER_H__

#include "imap/streamsource.h"
#include "iutil/comp.h"
#include "csutil/scf.h"
#include "csutil/scf_implementation.h"


struct iObjectRegistry;
struct iVFS;

/**
 * This plugin will listen to a reporter plugin and uses the console,
 * and other output devices to show appropriate messages based on
 * what comes from the reporter plugin.
 */
class csForegroundStreamingLoader : 
  public scfImplementation2<csForegroundStreamingLoader, 
      iStreamSource,
      iComponent>
{
public:
  /// Constructor
  csForegroundStreamingLoader (iBase* iParent);

  /// Destructor
  ~csForegroundStreamingLoader ();

  /// Initialize the plugin.
  virtual bool Initialize (iObjectRegistry* object_reg);

  /**
   * Set the base VFS path from which to retrieve all buffers.
   */
  void SetBasePath (const char* base);

  /**
   * Load a buffer given an id. This will fire the callback as soon as
   * the buffer is ready. Note that some implementations that don't support
   * asynchronous loading may call the callback immediatelly from within
   * this function.
   */
  virtual void QueryBuffer (const char* id, iStreamDataCallback* callback, RequestPriority priority = RP_NORMAL, iProgressMeter* indicator = 0);

  /**
   * Save a buffer with some id.
   */
  virtual void SaveBuffer (const char* id, iDataBuffer* buffer, iStreamDataCallback* callback, RequestPriority priority = RP_NORMAL, iProgressMeter* indicator = 0);

private:
  // The base VFS path from which to retrieve all buffers.
  csString basePath;

  /// A pointer to the object registry.
  iObjectRegistry* obj_reg;

  /// A reference to the VFS plugin
  csRef<iVFS> vfs;
};

#endif // __CS_STREAMLOADER_H__

