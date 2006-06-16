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
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "csutil/databuf.h"

#define CS_REPORT(severity,message) csReport (obj_reg, CS_REPORTER_SEVERITY_##severity, "cel.networklayer.client", message);


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csForegroundStreamingLoader)


/// Constructor
csSLRequestRecord::csSLRequestRecord (iStreamDataCallback::RequestType type, const char* bufferID, iStreamDataCallback* callback, iDataBuffer* buffer, unsigned int priority, iFile* file)
{
  this->type = type;
  this->bufferID = bufferID;
  this->callback = callback;
  targetTime = csGetTicks () + priority;
  this->buffer = buffer;
  currentPosition = buffer->GetData ();
  bytesLeft = buffer->GetSize ();
  this->file = file;
}

/// Destructor
csSLRequestRecord::~csSLRequestRecord ()
{
}


/// Constructor
csForegroundStreamingLoader::csForegroundStreamingLoader (iBase* iParent) : scfImplementationType (this, iParent)
{
  blockSize = 16384; // 16KB block size by default.
  timeLimit = 20; // Number of msec to allow for loading per frame.
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
    CS_REPORT (ERROR, "Can't find the VFS plugin!")
    return false;
  }

  return true;
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
  csRef<iFile> inputfile = vfs->Open (id, VFS_FILE_READ);
  if (inputfile.IsValid ())
  {
    csSLRequestRecord record (iStreamDataCallback::RT_LOAD, id, callback, new csDataBuffer (inputfile->GetSize ()), priority, inputfile);
    csList<csSLRequestRecord>::Iterator iter (requests);
    while (iter.HasNext () && iter.Next ().targetTime < record.targetTime);
    requests.InsertBefore (iter, record);
  }
  else
  {
    CS_REPORT (ERROR, "Couldn't find the requested file!")
    callback->StreamingError (id, iStreamDataCallback::RT_LOAD);
  }
}

/**
 * Save a buffer with some id.
 */
void csForegroundStreamingLoader::SaveBuffer (const char* id, iDataBuffer* buffer, iStreamDataCallback* callback, RequestPriority priority, iProgressMeter* indicator)
{
  csRef<iFile> outputfile = vfs->Open (id, VFS_FILE_READ);
  if (outputfile.IsValid ())
  {
    csSLRequestRecord record (iStreamDataCallback::RT_SAVE, id, callback, buffer, priority, outputfile);
    csList<csSLRequestRecord>::Iterator iter (requests);
    while (iter.HasNext () && iter.Next ().targetTime < record.targetTime);
    requests.InsertBefore (iter, record);
  }
  else
  {
    CS_REPORT (ERROR, "Couldn't find the requested file!")
    callback->StreamingError (id, iStreamDataCallback::RT_SAVE);
  }
}

/**
 * Event Handler
 */
bool csForegroundStreamingLoader::HandleEvent (iEvent &event)
{
  if (event.Name == csevPostProcess (obj_reg))
  {
    csTicks starttime = csGetTicks();
    csTicks currenttime = starttime;

    csSLRequestRecord record = requests.Front ();

    while (currenttime - starttime < timeLimit)
    {
      if (record.type == iStreamDataCallback::RT_LOAD)
      {
        // Read a block of data.
        size_t bytes_read = record.file->Read (record.currentPosition, blockSize);
        record.bytesLeft -= bytes_read;
        record.currentPosition += bytes_read;

        if (record.bytesLeft == 0)
        {
          // Completed the request; call the callback.
          record.callback->StreamingFinished (record.bufferID, record.buffer, iStreamDataCallback::RT_LOAD);

          // Remove the record from the list.
          csList<csSLRequestRecord>::Iterator iter (requests);
          requests.Delete (iter);
        }
        else if (bytes_read < blockSize)
        {
          // An error occurred!
          record.callback->StreamingError (record.bufferID, iStreamDataCallback::RT_LOAD);

          // Remove the record from the list.
          csList<csSLRequestRecord>::Iterator iter (requests);
          requests.Delete (iter);
        }
      }
      else
      {
        // Write a block of data.
        size_t bytes_read = record.file->Write (record.currentPosition, blockSize);
        record.bytesLeft -= bytes_read;
        record.currentPosition += bytes_read;

        if (record.bytesLeft == 0)
        {
          // Completed the request; call the callback.
          record.callback->StreamingFinished (record.bufferID, record.buffer, iStreamDataCallback::RT_SAVE);

          // Remove the record from the list.
          csList<csSLRequestRecord>::Iterator iter (requests);
          requests.Delete (iter);
        }
        else if (bytes_read < blockSize)
        {
          // An error occurred!
          record.callback->StreamingError (record.bufferID, iStreamDataCallback::RT_SAVE);

          // Remove the record from the list.
          csList<csSLRequestRecord>::Iterator iter (requests);
          requests.Delete (iter);
        }
      }
      currenttime = csGetTicks ();
    }
    return true;
  }
  return false;
}

