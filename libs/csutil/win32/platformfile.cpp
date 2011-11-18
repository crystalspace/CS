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

#include "csutil/csuctransform.h"
#include "csutil/platformfile.h"

namespace CS
{
  namespace Platform
  {
    FILE* File::Open (const char* filename, const char* mode)
    {
      // Convert file name + mode to wide strings.
      size_t filenameLen (strlen (filename));
      size_t filenameWSize (filenameLen + 1);
      CS_ALLOC_STACK_ARRAY(wchar_t, filenameW, filenameWSize);
      csUnicodeTransform::UTF8toWC (filenameW, filenameWSize,
                                    (utf8_char*)filename, filenameLen);

      // Convert file name + mode to wide strings.
      size_t modeLen (strlen (mode));
      size_t modeWSize (modeLen + 1);
      CS_ALLOC_STACK_ARRAY(wchar_t, modeW, modeWSize);
      csUnicodeTransform::UTF8toWC (modeW, modeWSize,
                                    (utf8_char*)mode, modeLen);

      return _wfopen (filenameW, modeW);
    }
  } // namespace Platform
} // namespace CS
