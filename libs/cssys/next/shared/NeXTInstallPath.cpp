//=============================================================================
//
//	Copyright (C) 2002 by Matt Reda <mreda@mac.com>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//=============================================================================
//-----------------------------------------------------------------------------
// NeXTInstallPath.cpp
//
//	Platform-specific function to determine installation path.
//
//-----------------------------------------------------------------------------
#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "NeXTInstallPath.h"
#include <stdlib.h>

//-----------------------------------------------------------------------------
// csGetInstallPath
//-----------------------------------------------------------------------------
bool csGetInstallPath(char* buff, size_t sz)
{
  bool ok = false;
  if (buff != 0 && sz > 0)
  {
    ok = true;
    if (!NeXTGetInstallPath(buff, sz, PATH_SEPARATOR))
    {
      char const* path = getenv("CRYSTAL");
      if (path == 0 || *path == '\0')
      {
        *buff = '\0';
      }
      else
      {
        size_t n = strlen(path);
	CS_ASSERT(n > 0); // Should be caught by above case.
        // Do we have to add a final path separator to the directory?
        bool const addsep = (path[n - 1] != PATH_SEPARATOR);
        if (addsep)
          n++;
        if (n >= sz)
          n = sz - 1;
        memcpy(buff, path, n);
        if (addsep)
          buff[n - 1] = PATH_SEPARATOR;
        buff[n] = '\0';
      }
    }
  }
  return ok;
}
