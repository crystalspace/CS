/*
    Copyright (C) 1998-2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_CSSYS_SYSPATH_H__
#define __CS_CSSYS_SYSPATH_H__

#include "csutil/array.h"
#include "csutil/ref.h"
#include "csutil/util.h"
#include "iutil/strvec.h"

/**\file
 * Paths helpers.
 */
/**\addtogroup util
 * @{
 */
/**\name Helpers to deal with native paths
 * @{
 */

/**
 * This structure contains information about a plugin path.
 */
struct csPluginPath
{
  /**
   * The actual path.
   * Has to be allocated with csStrNew() or new[].
   */
  char* path;
  /// Whether this path should be recursively scanned for plugins.
  bool scanRecursive;
  
  csPluginPath () : path (0), scanRecursive (false) {}
  csPluginPath (const char* path, bool recursive = false)
  {
    csPluginPath::path = csStrNew (path);
    scanRecursive = recursive;
  };
  csPluginPath (char* path, bool recursive = false)
  {
    csPluginPath::path = path;
    scanRecursive = recursive;
  };
  csPluginPath (const csPluginPath& src)
  {
    path = csStrNew (src.path);
    scanRecursive = src.scanRecursive;
  };
  ~csPluginPath () { delete[] path; }
};

/**
 * Class to manage a list of plugin paths.
 */
class csPluginPaths
{
  csArray<csPluginPath> paths;
public:
  csPluginPaths () : paths (0, 4) { }

  /**
   * Add a path, but only if it isn't in the list already.
   * \param path Path to add to the list. 
   * \param scanRecursive Mark the path to be scanned recursively.
   * \param overrideRecursive If the path is already in the list, just set
   *  the 'scan recursive' flag.
   * \return Index of path in the list.
   * \remark Saves full native paths and uses csExpandPath() for this.
   * \remark Uses csPathsIdentical() to compare paths.
   */
  int AddOnce (const char* path, bool scanRecursive = false, 
    bool overrideRecursive = true);
  
  int GetCount () { return paths.Length(); }
  csPluginPath const& operator [] (int n) const
  { return paths[n]; }
};

/**
 * Get the list of root directories.<p>
 * For instance in Unix it simply returns '/' but for Windows it may return a
 * list of available drive letters.
 */
csRef<iStrVector> csFindSystemRoots();

/**
 * Get the installation path.<p>
 * This returns the path where the system has been installed to.
 * It has a limited use because mostly everything should be done
 * through VFS which is installation directory - independent; but
 * some initialization tasks still need this.
 * You're responsible to delete[] the string after using it.
 */
char* csGetConfigPath ();

/** 
 * Get a list of directories where plugins are installed.
 * \remark Caller is responsible to free the list with delete after using it.
 */
csPluginPaths* csGetPluginPaths ();

/**
 * Expand a native path relative to the current directory.
 * \remark Caller is responsible to free the returend string with delete[] 
 *   after using it.
 */
char* csExpandPath (const char* path);

/**
 * Return the path the application was started from.
 * \remark Caller is responsible to free the returend string with delete[] 
 *   after using it.
 */
char* csGetAppPath ();

/**
 * Check whether two native paths actually point to the same location.
 * Use this instead of strcasecmp(), as it may not suffice in all cases.
 * \remark Expects the paths to be fully qualified. Use csExpandPath() to 
 *   ensure this.
 */
bool csPathsIdentical (const char* path1, const char* path2);

#endif
