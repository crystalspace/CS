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

#include "cssysdef.h"

#include "streamloader_fg.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"

#define CS_REPORT(severity,message) csReport (obj_reg, CS_REPORTER_SEVERITY_##severity, "cel.networklayer.client", message);


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csForegroundStreamingLoader)


/// Constructor
csForegroundStreamingLoader::csForegroundStreamingLoader (iBase* iParent) : scfImplementationType (this, iParent)
{
  basePath = "";
}

/// Destructor
csForegroundStreamingLoader::~csForegroundStreamingLoader ()
{
}

/// Initialize the plugin.
bool csForegroundStreamingLoader::Initialize (iObjectRegistry* object_reg)
{
  obj_reg = object_reg;

  vfs = csQueryRegistry<iVFS> (object_reg);
  if (!vfs.IsValid ())
  {
    CS_REPORT (ERROR,"Can't find the VFS plugin!")
    return false;
  }

  return true;
}

/**
 * Set the base VFS path from which to retrieve all buffers.
 */
void csForegroundStreamingLoader::SetBasePath (const char* base)
{
  basePath = base;
  if (basePath[basePath.Length () + 1] != '/')
  {
    basePath.Append ("/");
  }
}

/**
 * Load a buffer given an id. This will fire the callback as soon as
 * the buffer is ready. Note that some implementations that don't support
 * asynchronous loading may call the callback immediatelly from within
 * this function.
 */
void csForegroundStreamingLoader::QueryBuffer (const char* id, iStreamDataCallback* callback, RequestPriority priority, iProgressMeter* indicator)
{
  CS_ASSERT (callback != 0);
  csString filename = id;
  if (!filename.StartsWith ("/"))
  {
    filename = basePath;
    filename.Append (id);
  }
  csRef<iFile> inputfile = vfs->Open (filename, VFS_FILE_READ);
  if (inputfile.IsValid ())
  {
    csRef<iDataBuffer> buffer = inputfile->GetAllData ();
    if (buffer.IsValid ())
    {
      callback->StreamingFinished (id, buffer, iStreamDataCallback::RT_LOAD);
    }
    else
    {
      callback->StreamingError (id, iStreamDataCallback::RT_LOAD);
    }
  }
  else
  {
    callback->StreamingError (id, iStreamDataCallback::RT_LOAD);
  }
}

/**
 * Save a buffer with some id.
 */
void csForegroundStreamingLoader::SaveBuffer (const char* id, iDataBuffer* buffer, iStreamDataCallback* callback, RequestPriority priority, iProgressMeter* indicator)
{
  csString filename = id;
  if (!filename.StartsWith ("/"))
  {
    filename = basePath;
    filename.Append (id);
  }
  bool success = vfs->WriteFile (filename, buffer->GetData (), buffer->GetSize ());
  if (callback != 0)
  {
    if (success)
    {
      callback->StreamingFinished (id, buffer, iStreamDataCallback::RT_SAVE);
    }
    else
    {
      callback->StreamingError (id, iStreamDataCallback::RT_SAVE);
    }
  }
}

