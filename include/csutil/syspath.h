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

class csPathsList;

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
CS_CRYSTALSPACE_EXPORT csPathsList* csGetPluginPaths (const char* argv0);

/**
 * Class to manage a list of paths.
 * \remark Entries are ensured to not have a terminating path separator.
 * \remark Empty entries ("") are allowed.
 */
class CS_CRYSTALSPACE_EXPORT csPathsList
{
public:
  /**
  * This structure contains information about a plugin path.
  */
  struct CS_CRYSTALSPACE_EXPORT Entry
  {
    /**
    * The actual path.
    */
    csString path;
    /// "Type" of the directory (e.g. app, crystal ...)
    csString type;
    /// Whether this path should be recursively scanned for plugins.
    bool scanRecursive;
    
    void FixSeparators();

    Entry () : scanRecursive (false) {}
    Entry (const char* path, const char* type, bool recursive = false)
    {
      Entry::path = path;
      FixSeparators();
      Entry::type = type;
      scanRecursive = recursive;
    };
    Entry (const Entry& src)
    {
      path = src.path;
      type = src.type;
      scanRecursive = src.scanRecursive;
    };
  };
private:
  csArray<Entry> paths;
public:
  /// Constructor.
  csPathsList () : paths (4, 4) {}
  /// Copy constructor.
  csPathsList (csPathsList const& o) : paths(o.paths) {}
  /// Construct from a list of paths separated by CS_PATH_DELIMITER.
  csPathsList (const char* pathList);
  /**
   * Construct from a list of single paths. The list must be terminated by a
   * 0 entry.
   */
  csPathsList (const char* pathList[]);
  /// Destructor.
  ~csPathsList() {}
  /// Assignment operator.
  csPathsList& operator= (csPathsList const& o)
  { if (&o != this) paths = o.paths; return *this; }

  /**
   * Add a path, but only if it isn't in the list already.
   * \param path Path to add to the list. 
   * \param scanRecursive Mark the path to be scanned recursively.
   * \param type An arbitrary string assigning a type to the directory
   *  (i.e. "app", "crystal", etc.).
   * \param overrideRecursive If the path is already in the list, just set
   *  the 'scan recursive' flag.
   * \return Index of path in the list.
   * \remark Uses csPathsIdentical() to compare paths.
   */
  size_t AddUnique (const char* path, bool scanRecursive = false, 
    const char* type = 0, bool overrideRecursive = true);
  /**
   * \copydoc AddUnique(const char*, bool, const char*, bool)
   * \remark Saves full native paths and uses csExpandPath() for this.
   */
  size_t AddUniqueExpanded (const char* path, bool scanRecursive = false, 
    const char* type = 0, bool overrideRecursive = true);
  /**
   * Add a path, but only if it isn't in the list already.
   * \param path Path to add to the list. 
   * \param overrideRecursive If the path is already in the list, just set
   *  the 'scan recursive' flag.
   * \return Index of path in the list.
   * \remark Uses csPathsIdentical() to compare paths.
   */
  size_t AddUnique (const Entry& path, bool overrideRecursive = true);
  /**
   * \copydoc AddUnique(const Entry&, bool)
   * \remark Saves full native paths and uses csExpandPath() for this.
   */
  size_t AddUniqueExpanded (const Entry& path, bool overrideRecursive = true);
  /// Add another path list.
  void AddUnique (const csPathsList& list, bool overrideRecursive = true)
  {
    for (size_t i = 0; i < list.Length(); i++)
    {
      AddUnique (list[i], overrideRecursive);
    }
  }
  /**
   * \copydoc AddUnique(const csPathsList&, bool)
   * \remark Saves full native paths and uses csExpandPath() for this.
   */
  void AddUniqueExpanded (const csPathsList& list, 
    bool overrideRecursive = true)
  {
    for (size_t i = 0; i < list.Length(); i++)
    {
      AddUniqueExpanded (list[i], overrideRecursive);
    }
  }
  /// Remove an entry from the list.
  void DeleteIndex (size_t index)
  { paths.DeleteIndex (index); }
  
  /// Return number of contained paths.
  size_t Length () const { return paths.Length(); }
  CS_DEPRECATED_METHOD size_t GetCount () const { return Length(); }
  /// Retrieve the n'th path record.
  Entry const& operator [] (size_t n) const { return paths[n]; }
  
  //@{
  /**
   * Create a list of paths where all paths are those on the left side
   * concatenated with those on the right (of course with a CS_PATH_SEPARATOR 
   * ensured to be in between).
   */
  friend csPathsList operator* (const csPathsList& left,
    const csPathsList& right);
  inline csPathsList operator*= (const csPathsList& right)
  { return (*this = *this * right); }
  inline friend csPathsList operator* (const Entry& left, 
    const csPathsList& right)
  {
    csPathsList newPaths;
    newPaths.paths.Push (left);
    return newPaths * right;
  }
  inline friend csPathsList operator* (const char* left, 
    const csPathsList& right)
  { return Entry (left, 0) * right; }
  inline friend csPathsList operator* (const csPathsList& left, 
    const Entry& right)
  {
    csPathsList newPaths;
    newPaths.paths.Push (right);
    return left * newPaths;
  }
  inline friend csPathsList operator* (const csPathsList& left, 
    const char* right)
  { return left * Entry (right, 0); }
  inline csPathsList operator*= (const Entry& right)
  { return (*this = *this * right); }
  inline csPathsList operator*= (const char* right)
  { return (*this = *this * right); }
  //@}
};

/**
 * A helper class with path-related utilities.
 */
class CS_CRYSTALSPACE_EXPORT csPathsUtilities
{
public:
  /**
   * Check whether two native paths actually point to the same location.
   * Use this instead of strcmp() or the like, as it may not suffice in all 
   * cases (e.g. on Windows paths names are case-insensitive, but on Unix
   * they aren't).
   * \remark Expects the paths to be fully qualified. Use csExpandPath() to 
   *   ensure this.
   */
  static bool PathsIdentical (const char* path1, const char* path2);
  /**
   * Expand a native path relative to the current directory.
   * \remark The specified path must refer to a directory, rather than a file.
   * \remark Caller is responsible to free the returend string with delete[] 
   *   after using it.
   */
  static char* ExpandPath (const char* path);
  
  /**
   * Determine which path(s) of a given set contains a given file.
   * \param paths List of (native) paths to check.
   * \param file Filename to search.
   * \param thorough Whether all paths should be checked. If \c false, at most
   *   only a single path, the first containing the file, is returned.
   */
  static csPathsList LocateFile (const csPathsList& paths, 
    const char* file, bool thorough = false);

  /// Filter all non-existant items out of a paths list.
  static void FilterInvalid (csPathsList& paths);

  /// Expands all paths in a path list.
  static csPathsList ExpandAll (const csPathsList& paths);
};

/**
 * A helper class containing a number of functions to deal with Crystal Space
 * installation paths.
 */
class CS_CRYSTALSPACE_EXPORT csInstallationPathsHelper
{
public:
  /**
   * Return one or more paths which themselves or whose subdirectories can 
   * contain CrystalSpace-related resources, plugins as well as common data and
   * configuration. There's no guarantee that any entry actually contains some
   * useable resource; it's up to the caller to check what resources are
   * available and to pick the appropriate entries for some purpose.
   * \remark Caller is responsible to free the list with delete after using it.
   */
  static csPathsList* GetPlatformInstallationPaths ();
  /**
   * Get the list of root directories.
   * For instance in Unix it simply returns '/' but for Windows it may return a
   * list of available drive letters.
   */
  static csRef<iStringArray> FindSystemRoots();
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
  static csString GetAppPath (const char* argv0);
  
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
  static csString GetAppDir (const char* argv0);
  
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
  static csString GetResourceDir (const char* argv0);
  /** 
  * Get a list of directories where plugins are installed.
  * \remark Caller is responsible to free the list with delete after using it.
  */
  static csPathsList* GetPluginPaths (const char* argv0);
};

//@{
/** \deprecated Use the equivalent from csPathsUtilities or 
 * csInstallationPathsHelper instead. */
CS_DEPRECATED_METHOD inline bool csPathsIdentical (const char* path1, 
  const char* path2)
{ return csPathsUtilities::PathsIdentical (path1, path2); }
CS_DEPRECATED_METHOD inline csRef<iStringArray> csFindSystemRoots()
{ return csInstallationPathsHelper::FindSystemRoots (); }
CS_DEPRECATED_METHOD inline char* csExpandPath (const char* path)
{ return csPathsUtilities::ExpandPath (path); }
CS_DEPRECATED_METHOD inline csString csGetAppPath (const char* argv0)
{ return csInstallationPathsHelper::GetAppPath (argv0); }
CS_DEPRECATED_METHOD inline csString csGetAppDir (const char* argv0)
{ return csInstallationPathsHelper::GetAppDir (argv0); }
CS_DEPRECATED_METHOD inline csString csGetResourceDir (const char* argv0)
{ return csInstallationPathsHelper::GetResourceDir (argv0); }
//@}

/** @} */
/** @} */

#endif
