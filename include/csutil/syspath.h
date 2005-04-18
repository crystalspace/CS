/*
    Copyright (C) 2003 by Frank Richter

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

#include "csextern.h"
#include "array.h"
#include "csstring.h"
#include "ref.h"
#include "util.h"
#include "iutil/stringarray.h"

/**\file
 * Paths helpers.
 */
/**\addtogroup util
 * @{
 */
/**\name Helpers to deal with native paths
 * @{
 */

class csPluginPaths;

/**
 * Get the list of root directories.
 * For instance in Unix it simply returns '/' but for Windows it may return a
 * list of available drive letters.
 */
CS_CRYSTALSPACE_EXPORT csRef<iStringArray> csFindSystemRoots();

/**
 * Get the installation path.
 * This returns the path where the system has been installed to.  It has a
 * limited use because mostly everything should be done through VFS which is
 * installation directory - independent; but some initialization tasks still
 * need this.  May return the empty string if unable to determine the
 * installation path.
 */
CS_CRYSTALSPACE_EXPORT csString csGetConfigPath ();

/** 
 * Get a list of directories where plugins are installed.
 * \remark Caller is responsible to free the list with delete after using it.
 */
CS_CRYSTALSPACE_EXPORT csPluginPaths* csGetPluginPaths (const char* argv0);

/**
 * Expand a native path relative to the current directory.
 * \remark The specified path must refer to a directory, rather than a file.
 * \remark Caller is responsible to free the returend string with delete[] 
 *   after using it.
 */
CS_CRYSTALSPACE_EXPORT char* csExpandPath (const char* path);

/**
 * Return the absolute path of the executable.  For MacOS/X, returns the
 * absolute path of the executable within the Cocoa application wrapper.
 * \remark May return the empty string if some problem prevents determination
 *   of the application's path.
 * \remark This function is primarily intended for very low-level use before 
 *   or during the initialization of CS core components. After initialization,
 *   it is often more convenient to invoke iCommandLineParser::GetAppPath().
 * \param argv0 The first element of the argv[] array passed to main().  On
 *   many platforms, this is the only way to determine the actual location of
 *   the executable.
 */
CS_CRYSTALSPACE_EXPORT csString csGetAppPath (const char* argv0);

/**
 * Return the directory in which the application executable resides.  For
 * MacOS/X, returns the directory in which the Cocoa application wrapper
 * resides.
 * \remark May return the empty string if some problem prevents determination
 *   of the application's directory.
 * \remark This function is primarily intended for very low-level use before 
 *   or during the initialization of CS core components. After initialization,
 *   it is often more convenient to invoke iCommandLineParser::GetAppDir().
 * \param argv0 The first element of the argv[] array passed to main().  On
 *   many platforms, this is the only way to determine the actual location of
 *   the executable.
 */
CS_CRYSTALSPACE_EXPORT csString csGetAppDir (const char* argv0);

/**
 * Return the directory in which the application's resources reside.  On
 * many platforms, resources (such as plugin modules) reside in the same
 * directory as the application itself.  The default implementation
 * returns the same value as csGetAppPath(), however platforms may want to
 * override the default implementation if this behavior is unsuitable.  For
 * example, on MacOS/X, for GUI applications, resources reside in the
 * "Resources" directory within the Cocoa application wrapper.
 * \remark May return the empty string if some problem prevents determination
 *   of the resource path.
 * \remark This function is primarily intended for very low-level use before 
 *   or during the initialization of CS core components. After initialization,
 *   it is often more convenient to invoke
 *   iCommandLineParser::GetResourceDir().
 * \param argv0 The first element of the argv[] array passed to main().  On
 *   many platforms, this is the only way to determine the actual location of
 *   the resources.
 */
CS_CRYSTALSPACE_EXPORT csString csGetResourceDir (const char* argv0);

/**
 * Check whether two native paths actually point to the same location.
 * Use this instead of strcmp() or the like, as it may not suffice in all 
 * cases (e.g. on Windows paths names are case-insensitive, but on Unix
 * they aren't).
 * \remark Expects the paths to be fully qualified. Use csExpandPath() to 
 *   ensure this.
 */
CS_CRYSTALSPACE_EXPORT bool csPathsIdentical (const char* path1,
					      const char* path2);

/**
 * This structure contains information about a plugin path.
 */
struct CS_CRYSTALSPACE_EXPORT csPluginPath
{
  /**
   * The actual path.
   * Has to be allocated with csStrNew() or new[].
   */
  char* path;
  /// "Type" of the directory (e.g. app, crystal ...)
  char* type;
  /// Whether this path should be recursively scanned for plugins.
  bool scanRecursive;
  
  csPluginPath () : path (0), type(0), scanRecursive (false) {}
  csPluginPath (const char* path, const char* type, bool recursive = false)
  {
    csPluginPath::path = csStrNew (path);
    csPluginPath::type = csStrNew (type);
    scanRecursive = recursive;
  };
  csPluginPath (char* path, char* type, bool recursive = false)
  {
    csPluginPath::path = path;
    csPluginPath::type = type;
    scanRecursive = recursive;
  };
  csPluginPath (const csPluginPath& src)
  {
    path = csStrNew (src.path);
    type = csStrNew (src.type);
    scanRecursive = src.scanRecursive;
  };
  ~csPluginPath () { delete[] path; delete[] type; }
};

/**
 * Class to manage a list of plugin paths.
 */
class CS_CRYSTALSPACE_EXPORT csPluginPaths
{
private:
  csArray<csPluginPath> paths;
public:
  /// Constructor.
  csPluginPaths () : paths (4, 4) {}
  /// Copy constructor.
  csPluginPaths(csPluginPaths const& o) : paths(o.paths) {}
  /// Destructor.
  ~csPluginPaths() {}
  /// Assignment operator.
  csPluginPaths& operator=(csPluginPaths const& o)
  { paths = o.paths; return *this; }

  /**
   * Add a path, but only if it isn't in the list already.
   * \param path Path to add to the list. 
   * \param scanRecursive Mark the path to be scanned recursively.
   * \param type An arbitrary string assigning a type to the directory
   *  (i.e. "app", "crystal", etc.).
   * \param overrideRecursive If the path is already in the list, just set
   *  the 'scan recursive' flag.
   * \return Index of path in the list.
   * \remark Saves full native paths and uses csExpandPath() for this.
   * \remark Uses csPathsIdentical() to compare paths.
   */
  size_t AddOnce (const char* path, bool scanRecursive = false, 
    const char* type = 0, bool overrideRecursive = true)
  {
    if (path == 0) return (size_t)-1;
    char* pathExpanded = csExpandPath (path);
    if (pathExpanded == 0) return (size_t)-1;
  
    size_t i;
    for (i = 0; i < paths.Length(); i++)
    {
      if (csPathsIdentical (pathExpanded, paths[i].path))
      {
	if (overrideRecursive)
	{
	  paths[i].scanRecursive = scanRecursive;
	}
	delete[] paths[i].type;
	paths[i].type = csStrNew (type);
	delete[] pathExpanded;
	return i;
      }
    }
  
    csPluginPath pluginPath (pathExpanded, csStrNew (type), scanRecursive);
    return (paths.Push (pluginPath));
  }
  
  /// Return number of contained paths.
  size_t GetCount () const { return paths.Length(); }
  /// Retrieve the n'th path record.
  csPluginPath const& operator [] (size_t n) const { return paths[n]; }
};

/** @} */
/** @} */

#endif
