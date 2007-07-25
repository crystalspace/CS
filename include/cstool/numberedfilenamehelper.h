/*
  Copyright (C) 2006 by Frank Richter

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

#ifndef __CSTOOL_NUMBEREDFILENAMEHELPER_H__
#define __CSTOOL_NUMBEREDFILENAMEHELPER_H__

#include "csextern.h"
#include "csutil/csstring.h"
#include "iutil/vfs.h"

namespace CS
{
  /**
   * Helper to deal with numbered filename.
   * Takes a mask for a filename, which can contain digits, as input and 
   * returns filenames with subsequently increasing numbers. Also can
   * automatically pick an unused filename.
   */
  class CS_CRYSTALSPACE_EXPORT NumberedFilenameHelper
  {
    csString format;
    uint counter;
  public:
    /**
     * Initialize helper. See SetMask() for the meaning of \p mask.
     */
    NumberedFilenameHelper (const char* mask = 0)
    {
      Reset ();
      SetMask (mask);
    }
    /**
     * Resets the counter to 0.
     */
    void Reset() { counter = 0; }
    /**
     * Set the mask for the filename.
     * The rightmost cluster of digits in the filename is replaced with the
     * value of the counter. The format is chosen such that the digits are
     * filled up with leading zeroes. If the filename does not contain any
     * digits, the counter is inserted before the last dot or appended to the
     * end of the name.
     */
    void SetMask (const char* mask);
    /**
     * Returns the filename with the next higher number.
     */
    csString NextFilename ()
    {
      return csString().Format (format, counter++);
    }
    /**
     * Returns the next unused filename.
     * \param vfs Pointer to VFS interface which is used to test for file
     *   existance. If 0, the real filesystem is used.
     */
    csString FindNextFilename (iVFS* vfs = 0);
  };
}

#endif // __CSTOOL_NUMBEREDFILENAMEHELPER_H__
