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

#include "cssysdef.h"

#include "csutil/syspath.h"

#include "csutil/csuctransform.h"

static inline int CS_mkdir (const char* path)
{
#if defined (__CYGWIN32__)
  return mkdir(path, 0755);
#else
  size_t pathLen (strlen (path));
  size_t pathWlen (pathLen + 1);
  CS_ALLOC_STACK_ARRAY(wchar_t, pathW, pathWlen);
  csUnicodeTransform::UTF8toWC (pathW, pathWlen,
                                (utf8_char*)path, pathLen);
  return _wmkdir (pathW);
#endif
}

namespace CS
{
  namespace Platform
  {
    int CreateDirectory (const char* path)
    {
      int olderrno (errno);
      int result (0);
      if (CS_mkdir (path) < 0)
      {
        result = errno;
      }
      errno = olderrno;
      return result;
    }
  } // namespace Platform
} // namespace CS
