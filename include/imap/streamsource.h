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
 */
/**\addtogroup loadsave
 * @{ */

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
   * Load a buffer given an id. Returns 0 if the buffer couldn't be loaded
   * for some reason. The error should be reported on the reporter by this
   * function. This function should make sure the buffer is endian
   * correct for the current system.
   */
  virtual csPtr<iDataBuffer> LoadBuffer (const char* id) = 0;

  /**
   * Save a buffer with some id. Returns false if the buffer couldn't be
   * saved for some reason. The error should be reported on the reporter
   * by this function.
   */
  virtual bool SaveBuffer (const char* id, iDataBuffer* buffer) = 0;
};

/** @} */

#endif // __CS_IMAP_STREAMSOURCE_H__

