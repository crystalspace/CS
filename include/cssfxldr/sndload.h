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

#ifndef __SOUNDLOADER_H
#define __SOUNDLOADER_H

#include <stdio.h>
#include "csutil/scf.h"
#include "types.h"

class csSoundData;

struct iSystem;

///
class csSoundLoader  
{
public:
  /// Return the name of the wave type supported by this loader.
  virtual const char* get_name() const = 0;

  /// Return a descriptive line about this wave format.
  virtual const char* get_desc() const = 0;

  /**
   * Load an wave from a buffer.
   * This routine will read from the buffer buf of length size, try to 
   * recognize the type of wave contained within, and return an Wave
   * of the appropriate type.  Returns a pointer to the Wave on 
   * success, or NULL on failure.
   */
  static csSoundData* load (UByte* buf, ULong size);

  /**
   * Register a loader for a given type of sound files.
   * Adds 'loader' to the list of image formats to be checked during an
   * ImageLoader::load(...) call.
   */
  static bool Register (csSoundLoader* loader);

protected:
  ///
  virtual ~csSoundLoader() {}

  /**
   * Load an image from the given buffer.
   * Attempts to read an image from the buffer 'buf' of length 'size'.
   * If successful, returns a pointer to the resulting csImageFile.  Otherwise
   * returns NULL.
   */
  virtual csSoundData* loadsound(UByte* buf, ULong size) = 0;

private:
  static csVector *loaderlist;
};

#endif // __SOUNDLOADER_H
