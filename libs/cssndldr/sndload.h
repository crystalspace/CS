/*
    Copyright (C) 1998 by Jorrit Tyberghein    

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

#ifndef __SOUNDBUFFERLOADER_H
#define __SOUNDBUFFERLOADER_H

#include <stdio.h>
#include "cscom/com.h"
#include "types.h"

class csSoundBuffer;

interface ISystem;

///
class csSoundBufferLoader  
{
public:
  /// Return the name of the wave type supported by this loader.
  virtual const char* get_name() const = 0;

  /// Return a descriptive line about this wave format.
  virtual const char* get_desc() const = 0;

  /**
   * Load an wave given the filename.
   * This routine will open the file named filename, try to recognize its
   * type, and return an Wave of the appropriate type.  Returns a
   * pointer to the Wave on success, or NULL on failure.
   */
  static csSoundBuffer* load (ISystem* system, const char* filename);

  /**
   * Load an wave given a file pointer.
   * This routine will read from the given file, try to recognize its
   * type, and return an Wave of the appropriate type.  Returns a
   * pointer to the Wave on success, or NULL on failure.
   */
  static csSoundBuffer* load (FILE* fp);

  /**
   * Load an wave from a buffer.
   * This routine will read from the buffer buf of length size, try to 
   * recognize the type of wave contained within, and return an Wave
   * of the appropriate type.  Returns a pointer to the Wave on 
   * success, or NULL on failure.
   */
  static csSoundBuffer* load (UByte* buf, ULong size);

  /**
   * Register a loader for a given type of sound files.
   * Adds 'loader' to the list of image formats to be checked during an
   * ImageLoader::load(...) call.
   */
  static bool Register (csSoundBufferLoader* loader);

protected:
  ///
  virtual ~csSoundBufferLoader() {}

  /**
   * Load an image from the given buffer.
   * Attempts to read an image from the buffer 'buf' of length 'size'.
   * If successful, returns a pointer to the resulting ImageFile.  Otherwise
   * returns NULL.
   */
  virtual csSoundBuffer* loadsound(UByte* buf, ULong size) = 0;

private:
  static csVector *loaderlist;
};

#endif // __SOUNDBUFFERLOADER_H
