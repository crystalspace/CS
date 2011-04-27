/*
    Copyright (C) 2007 by Frank Richter

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

#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "csutil/stringarray.h"

#include "cstool/smartfileopen.h"

namespace CS
{
  namespace Utility
  {
    csPtr<iFile> SmartFileOpen (iVFS* vfs, const char* path, 
                                const char* defaultFilename, 
                                const char** actualFilename)
    {
      csRef<iFile> file;

      const char* fileNameToOpen;
      if (SmartChDir (vfs, path, defaultFilename, &fileNameToOpen))
      {
	file = vfs->Open (fileNameToOpen, VFS_FILE_READ);
      }

      if (actualFilename) *actualFilename = fileNameToOpen;
      return csPtr<iFile> (file);
    }
    
    bool SmartChDir (iVFS* vfs, const char* path, const char* defaultFilename, 
		     const char** actualFilename)
    {
      csString filename (path);
      csStringArray paths;
      paths.Push ("/lev/");

      /* Check if the given path can be auto-mounted. 
       * (Done first so ZIPs get handled properly.) 
       * This is also takes care of the case that "path" identifies a directory. */
      /* @@@ However, prefers maps in /lev/ over files in current dirs.
       * Desired? */
      if (defaultFilename 
        && vfs->ChDirAuto (path, &paths, 0, defaultFilename))
      {
	if (actualFilename) *actualFilename = defaultFilename;
	return true;
      }

      /* Now check if there's a path separator. If so split into directory
       * and filename part ...*/
      bool dirSet = false;
      size_t slashPos = filename.FindLast ('/');
      const char* fileNameToOpen;
      if (slashPos != (size_t)-1)
      {
        csString dir, base;
        filename.SubString (dir, 0, slashPos);
        fileNameToOpen = path + slashPos + 1;
        dirSet = vfs->ChDirAuto (dir, &paths, 0, fileNameToOpen);
      }
      else
      {
        // ... else assume path is a filename relative to current directory.
        fileNameToOpen = path;
        dirSet = vfs->ChDirAuto (".", &paths, 0, fileNameToOpen);
      }

      if (actualFilename) *actualFilename = fileNameToOpen;
      return dirSet;
    }
  } // namespace Utility
} // namespace CS
