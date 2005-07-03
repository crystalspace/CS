/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

/**\file
 */

#include "iutil/vfs.h"
#include "csutil/csstring.h"

/**
 * Helper class to change the current VFS directory and restore the previous
 * directory when an instance goes out of scope.
 */
class csVfsDirectoryChanger
{
  csRef<iVFS> vfs;
  bool vfsPop;
public:
  /// Create instance
  csVfsDirectoryChanger (iVFS* vfs) : vfs(vfs), vfsPop(false) { }
  /// Destroy instance. Restores the old directory if ChangeTo() was called.
  ~csVfsDirectoryChanger()
  {
    if (vfsPop)
      vfs->PopDir();
  }
  /**
   * Change to the directory \a filename is in.
   * \remarks Only the part of the path up to the last '/' is treated as the
   * directory to change to. E.g. both "<tt>/foo/bar/baz</tt>" and 
   * "<tt>/foo/bar/</tt>" will cause a directory change to "<tt>/foo/bar</tt>".
   */
  void ChangeTo (const char* filename)
  {
    if (!vfs) return;
    const char* slash = strrchr (filename, '/');
    if (slash != 0)
    {
      csString dir;
      dir.Replace (filename, slash - filename);
      vfs->PushDir ();
      vfs->ChDir (dir);
      vfsPop = true;
    }
  }
};
