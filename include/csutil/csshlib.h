/*
    Copyright (C) 1999 by Andrew Zabolotny
    Crystal Space cross-platform shared library management

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

#ifndef __CS_CSSHLIB_H__
#define __CS_CSSHLIB_H__

/**\file
 * Crystal Space cross-platform shared library management
 */

/** 
 * \addtogroup util
 * @{ */

/**\name Low-level shared library support
 * @{ */

#include "csextern.h"

#include "csutil/ref.h"
#include "csutil/stringarray.h"

#include "iutil/string.h"

typedef void* csLibraryHandle;

/**
 * Load a shared library and return a library handle,
 * which is used later to query and unload the library.
 * iName is the FULL path to the library.
 */
CS_CRYSTALSPACE_EXPORT csLibraryHandle csLoadLibrary (char const* iName);

/**
 * Return a pointer to a symbol within given shared library.
 * Note that generally Crystal Space needs just one exported symbol
 * from every shared library; the symbol is called {library}_scfInitialize.
 * If your OS is short on features, you may implement querying of just
 * this symbol.
 */
CS_CRYSTALSPACE_EXPORT void* csGetLibrarySymbol (csLibraryHandle Handle,
					   char const* iName);

/**
 * Unload a shared library given its handle.
 * The function returns false on error.
 */
CS_CRYSTALSPACE_EXPORT bool csUnloadLibrary (csLibraryHandle Handle);

/**
 * Print out the latest dynamic loader error.
 * This is not strictly required (and on some platforms its just a empty
 * routine) but sometimes it helps to find problems.
 */
CS_CRYSTALSPACE_EXPORT void csPrintLibraryError (char const* iModule);

/**
 * Control whether dynamic library loading messages are verbose or terse.
 * When verbose, and a library fails to load, csPrintLibraryError() is invoked
 * to emit detailed diagnostic information regarding the failure.  If terse,
 * then a simple message is emitted stating that the library failed to load
 * and instructing the user to use the -verbose command-line option for more
 * details.  Verbose messages are enabled by default for debug builds; terse
 * messages for optimized builds.
 */
CS_CRYSTALSPACE_EXPORT void csSetLoadLibraryVerbose(bool);

/**
 * Query if failed dynamic library loads generate verbose messages.
 */
CS_CRYSTALSPACE_EXPORT bool csGetLoadLibraryVerbose();

/**
 * Scan a given directory for plugins and return a list of the plugin
 * native file names and their respective metadata.
 * \param dir Directory to scan.
 * \param plugins Native file names.
 * \param recursive Recursively scan all subdirectories.
 * \remark It is the responsibility of the caller to do any cleaning
 *   of \p metadata and \p plugins, if desired.
 * \remark \p plugins can be 0, a string vector will be created in this case.
 * \return If any errors occured, a vector of error descriptions.
 */
CS_CRYSTALSPACE_EXPORT csRef<iStringArray> csScanPluginDir (const char* dir, 
  csRef<iStringArray>& plugins, bool recursive = true);

/**
 * Scan some given directories for plugins.
 * Accepts the same parameters as csScanPluginDir(), with the exception of
 * \p dirs.
 */				
CS_CRYSTALSPACE_EXPORT csRef<iStringArray> csScanPluginDirs (csPathsList* dirs,   
  csRef<iStringArray>& plugins);

/**
 * Retrive a plugin's metadata.
 * \remark \p `fullPath' should be either a string returned from
 * csScanPluginDir() or csScanPluginDirs(), or a fully qualified native path of
 * the plugin module.  The path suffix should be either .csplugin if the plugin
 * metadata is stored in an external .csplugin file, or it should be the
 * standard suffix of shared libraries or bundles for the platform (for
 * example, .so for Unix; .dll for Windows).  If metadata is retrieved
 * successfully for the specified plugin module, metadata.IsValid() will return
 * true, and the `metadata' argument will reference an iDocument containing the
 * data.  If no metadata is located or an error occurrs while attempting to
 * retrieve the metadata, metadata.IsValid() return false.  If some type of
 * reportable error or warning is encountered, it will be returned from the
 * function as an iString.  It is possible for a warning to be issued even if
 * metadata was successfully retrieved.  Finally, if metadata.IsValid() returns
 * false and the returned iString is empty, then that indicates simply that the
 * specified path does not correspond to a Crystal Space plugin module.  This
 * is a valid condition.
 */
CS_CRYSTALSPACE_EXPORT csRef<iString> csGetPluginMetadata (const char* fullPath, 
				    csRef<iDocument>& metadata);

/** @} */

/** @} */

#endif // __CS_CSSHLIB_H__
