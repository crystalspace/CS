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
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "csutil/databuf.h"
#include "csutil/eventnames.h"

#define CS_REPORT(severity,message) csReport (obj_reg, CS_REPORTER_SEVERITY_##severity, "crystalspace.streamloader.foreground", message);


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csForegroundStreamingLoader)


/// Constructor
csSLRequestRecord::csSLRequestRecord (iStreamDataCallback::RequestType type, const char* bufferID, iStreamDataCallback* callback, iDataBuffer* buffer, unsigned int priority)
{
  this->type = type;
  this->bufferID = bufferID;
  this->callback = callback;
  targetTime = csGetTicks () + priority;
  this->buffer = buffer;
  currentPosition = buffer->GetData ();
  bytesLeft = buffer->GetSize ();
  this->file = 0;
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

  // Register as an event listener.
  csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (object_reg);
  frameEventName = csevFrame (object_reg);
  if (q.IsValid ())
  {
    csEventID subEvents[] = { frameEventName, CS_EVENTLIST_END };
    q->RegisterListener (this, subEvents);
  }
  else
  {
    CS_REPORT (ERROR, "Can't find the event queue!")
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
  if (vfs->Exists (id))
  {
    size_t filesize;
    filesize = vfs->GetFileSize (id, filesize);
    csSLRequestRecord record (iStreamDataCallback::RT_LOAD, id, callback, new csDataBuffer (filesize), priority);
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
  if (vfs->Exists (id))
  {
    csSLRequestRecord record (iStreamDataCallback::RT_SAVE, id, callback, buffer, priority);
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
  if (event.Name == frameEventName)
  {
    csTicks starttime = csGetTicks();
    csTicks currenttime = starttime;

    csSLRequestRecord record = requests.Front ();

    while (currenttime - starttime < timeLimit)
    {
      if (record.type == iStreamDataCallback::RT_LOAD)
      {
        // Open the file if it hasn't been opened yet.
        if (!record.file.IsValid ())
        {
          record.file = vfs->Open (record.bufferID, VFS_FILE_READ);

          if (!record.file.IsValid ())
          {
            // An error occurred!
            record.callback->StreamingError (record.bufferID, iStreamDataCallback::RT_LOAD);

            // Remove the record from the list.
            csList<csSLRequestRecord>::Iterator iter (requests);
            requests.Delete (iter);
            record = requests.Front ();
	    continue;
          }
        }

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
          record = requests.Front ();
        }
        else if (bytes_read < blockSize)
        {
          // An error occurred!
          record.callback->StreamingError (record.bufferID, iStreamDataCallback::RT_LOAD);

          // Remove the record from the list.
          csList<csSLRequestRecord>::Iterator iter (requests);
          requests.Delete (iter);
          record = requests.Front ();
        }
      }
      else
      {
        // Open the file if it hasn't been opened yet.
        if (!record.file.IsValid ())
        {
          record.file = vfs->Open (record.bufferID, VFS_FILE_WRITE);

          if (!record.file.IsValid ())
          {
            // An error occurred!
            record.callback->StreamingError (record.bufferID, iStreamDataCallback::RT_LOAD);

            // Remove the record from the list.
            csList<csSLRequestRecord>::Iterator iter (requests);
            requests.Delete (iter);
            record = requests.Front ();
	    continue;
          }
        }

        // Write a block of data.
        size_t bytes_written = record.file->Write (record.currentPosition, blockSize);
        record.bytesLeft -= bytes_written;
        record.currentPosition += bytes_written;

        if (record.bytesLeft == 0)
        {
          // Completed the request; call the callback.
          record.callback->StreamingFinished (record.bufferID, record.buffer, iStreamDataCallback::RT_SAVE);

          // Remove the record from the list.
          csList<csSLRequestRecord>::Iterator iter (requests);
          requests.Delete (iter);
          record = requests.Front ();
        }
        else if (bytes_written < blockSize)
        {
          // An error occurred!
          record.callback->StreamingError (record.bufferID, iStreamDataCallback::RT_SAVE);

          // Remove the record from the list.
          csList<csSLRequestRecord>::Iterator iter (requests);
          requests.Delete (iter);
          record = requests.Front ();
        }
      }
      currenttime = csGetTicks ();
    }
    return true;
  }
  return false;
}

