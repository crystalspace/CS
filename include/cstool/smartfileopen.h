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

#ifndef __CS_CSTOOL_SMARTFILEOPEN_H__
#define __CS_CSTOOL_SMARTFILEOPEN_H__

#include "csutil/ref.h"

struct iFile;
struct iVFS;

namespace CS
{
  namespace Utility
  {
    /**
     * "Smartly" try to locate a file given a path that could be: a plain file
     * name, directory, name of a level in /lev/, or .zip file.
     *
     * \a path is tried to be interpreted in a number of different ways:
     * - A VFS directory. Succeeds if a file named \a defaultFilename exists
     *   in the directory.
     * - A level name in VFS <tt>/lev/</tt>. Succeeds if a file named 
     *   \a defaultFilename exists below that directory.
     * - A <tt>.zip</tt> file name. Succeeds if a file named \a defaultFilename
     *   exists in the archive.
     * - A plain file name. Succeeds if the file exists.
     *
     * This is for example useful for tool applications which want to let the
     * user specify paths and file locations in a variety of ways.
     *
     * \param vfs The VFS interface to use to open the file.
     * \param path The path that is attempted to be opened.
     * \param defaultFilename Default filename, used when the provided path
     *   is a directory or .zip file.
     * \param actualFilename Can return the actual filename opened. (Note:
     *   is either \a defaultFilename or a pointer into \a path).
     * \return The file if opening succeeded.
     */
    csPtr<iFile> CS_CRYSTALSPACE_EXPORT SmartFileOpen (iVFS* vfs, 
      const char* path, const char* defaultFilename = 0, 
      const char** actualFilename = 0);
      
    /**
     * "Smartly" change to the directory with some file given a path that could
     * be: a plain file name, directory, name of a level in /lev/, or .zip file.
     *
     * \a path is tried to be interpreted in a number of different ways:
     * - A VFS directory. Succeeds if a file named \a defaultFilename exists
     *   in the directory.
     * - A level name in VFS <tt>/lev/</tt>. Succeeds if a file named 
     *   \a defaultFilename exists below that directory.
     * - A <tt>.zip</tt> file name. Succeeds if a file named \a defaultFilename
     *   exists in the archive.
     * - A plain file name. Succeeds if the file exists.
     *
     * This is for example useful for tool applications which want to let the
     * user specify paths and file locations in a variety of ways.
     *
     * \param vfs The VFS interface to use to open the file.
     * \param path The path that is attempted to be opened.
     * \param defaultFilename Default filename, used to check whether the
     *   provided path is a directory or .zip file.
     * \param actualFilename Can return the filename used to determine the
     *   directory to change to. (Note: is either \a defaultFilename or a
     *   pointer into \a path).
     * \return Whether the changing the directory succeeded.
     */
    bool CS_CRYSTALSPACE_EXPORT SmartChDir (iVFS* vfs, 
      const char* path, const char* defaultFilename = 0, 
      const char** actualFilename = 0);
  } // namespace Utility
} // namespace CS

#endif // __CS_CSTOOL_SMARTFILEOPEN_H__
