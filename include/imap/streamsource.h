/*
    Copyright (C) 2005 by Jorrit Tyberghein
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

#ifndef __CS_IMAP_STREAMSOURCE_H__
#define __CS_IMAP_STREAMSOURCE_H__

#include "csutil/scf.h"
#include "csutil/scf_implementation.h"

struct iDataBuffer;
struct iProgressMeter;

/**\file
 * Streaming data source
 */
/**\addtogroup loadsave
 * @{ */

/**
 * Callback functions for when a streaming operation has finished or when an
 * error has occurred.
 */
struct iStreamDataCallback : public virtual iBase
{
  SCF_INTERFACE (iStreamDataCallback, 1, 0, 0);

  /**
   * The type of streaming operation being referenced.
   */
  enum RequestType
  {
    RT_SAVE,
    RT_LOAD
  };

  /**
   * The given streaming operation is finished. In the case of a load, the
   * buffer will be endian correct for this system.
   */
  virtual void StreamingFinished (const char* id, iDataBuffer* data, RequestType type = RT_LOAD) = 0;

  /**
   * An error has occurred during the given streaming operation. The error
   * should already be reported.
   */
  virtual void StreamingError (const char* id, RequestType type = RT_LOAD) = 0;
};

/**
 * This interface represents a stream source. This can be implemented
 * by the application to implement faster loading of data. Basically the
 * idea is to have some kind of 'id' that represents a buffer for a mesh.
 * This implementation of this interface can try to load the buffer
 * given that id.
 */
struct iStreamSource : public virtual iBase
{
  SCF_INTERFACE (iStreamSource, 1, 0, 0);

  /**
   * Available priority levels for load/save requests.
   */
  enum RequestPriority
  {
    RP_LOW           = 100000,
    RP_NORMAL        = 1000,
    RP_HIGH          = 10,
    RP_TIME_CRITICAL = 0
  };

  /**
   * Load a buffer given an id. This will fire the callback as soon as
   * the buffer is ready. Note that some implementations that don't support
   * asynchronous loading may call the callback immediatelly from within
   * this function.
   */
  virtual void QueryBuffer (const char* id, iStreamDataCallback* callback, RequestPriority priority = RP_NORMAL, iProgressMeter* indicator = 0) = 0;

  /**
   * Save a buffer with some id.
   */
  virtual void SaveBuffer (const char* id, iDataBuffer* buffer, iStreamDataCallback* callback, RequestPriority priority = RP_NORMAL, iProgressMeter* indicator = 0) = 0;
};

/** @} */

#endif // __CS_IMAP_STREAMSOURCE_H__

