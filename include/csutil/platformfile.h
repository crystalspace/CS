/*
    Copyright (C) 2011 by Frank Richter

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

#ifndef __CSUTIL_PLATFORM_FILE_H__
#define __CSUTIL_PLATFORM_FILE_H__

/**\file
 * Functions to abstract platform-specific file access.
 */

#include "cssysdef.h"
#include "csextern.h"

namespace CS
{
  namespace Platform
  {
    /**
     * Functions to abstract platform-specific file access.
     */
    struct CS_CRYSTALSPACE_EXPORT File
    {
      /**
       * Open a file from a native path, encoded in UTF-8.
       * The function takes care of translating the file name to the
       * platform-specific file name encoding.
       * \param filename Native file name, encoded in UTF-8.
       * \param mode \c fopen()-style file mode string.
       */
      static FILE* Open (const char* filename, const char* mode);
    };
  } // namespace Platform
} // namespace CS

#endif // __CSUTIL_PLATFORM_FILE_H__
