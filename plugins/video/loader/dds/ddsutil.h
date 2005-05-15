/*
    DSS image file format support for CrystalSpace 3D library
    Copyright (C) 2004-2005 by Frank Richter

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

#include "csutil/csendian.h"

static void CopyLEUI32s (void* dest, const void* source, size_t count)
{
  uint32* d = (uint32*)dest; uint32* s = (uint32*)source;
  while (count-- > 0)
  {
    *(d++) = csGetLittleEndianLong (s++);
  }
}

