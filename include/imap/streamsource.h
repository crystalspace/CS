/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

/**\file
 * Streaming data source
 */
/**\addtogroup loadsave
 * @{ */

/**
 * This callback will be fired when the data is ready.
 */
struct iStreamDataCallback : public virtual iBase
{
  SCF_INTERFACE (iStreamDataCallback, 0, 0, 1);

  /**
   * The buffer is ready. The buffer will be endian correct for this system.
   */
  virtual void DataReady (iDataBuffer* data) = 0;
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
  SCF_INTERFACE (iStreamSource, 0, 0, 1);

  /**
   * Load a buffer given an id. This will fire the callback as soon as
   * the buffer is ready. Note that some implementations that don't support
   * asynchronious loading may call the callback immediatelly from within
   * this function.
   * \return false if we can't find the buffer (early error). The error
   * should be placed on the reporter.
   */
  virtual bool QueryBuffer (const char* id, iStreamDataCallback* callback) = 0;

  /**
   * Save a buffer with some id. Returns false if the buffer couldn't be
   * saved for some reason. The error should be reported on the reporter
   * by this function.
   */
  virtual bool SaveBuffer (const char* id, iDataBuffer* buffer) = 0;
};

/** @} */

#endif // __CS_IMAP_STREAMSOURCE_H__

