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

#include "syspath.h"
#include "csutil/ref.h"
#include "csutil/refarr.h"
#include "iutil/string.h"
#include "iutil/strvec.h"

typedef void* csLibraryHandle;

/**
 * Load a shared library and return a library handle,
 * which is used later to query and unload the library.
 * iName is the FULL path to the library.
 */
csLibraryHandle csLoadLibrary (char const* iName);

/**
 * Return a pointer to a symbol within given shared library.
 * Note that generally Crystal Space needs just one exported symbol
 * from every shared library; the symbol is called <library>_scfInitialize.
 * If your OS is short on features, you may implement querying of just
 * this symbol.
 */
void* csGetLibrarySymbol (csLibraryHandle Handle, char const* iName);

/**
 * Unload a shared library given its handle.
 * The function returns false on error.
 */
bool csUnloadLibrary (csLibraryHandle Handle);

/**
 * Print out the latest dynamic loader error.
 * This is not strictly required (and on some platforms its just a empty
 * routine) but sometimes it helps to find problems.
 */
void csPrintLibraryError (char const* iModule);

/**
 * Control whether dynamic library loading messages are verbose or terse.
 * When verbose, and a library fails to load, csPrintLibraryError() is invoked
 * to emit detailed diagnostic information regarding the failure.  If terse,
 * then a simple message is emitted stating that the library failed to load
 * and instructing the user to use the -verbose command-line option for more
 * details.  Verbose messages are enabled by default for debug builds; terse
 * messages for optimized builds.
 */
void csSetLoadLibraryVerbose(bool);

/**
 * Query if failed dynamic library loads generate verbose messages.
 */
bool csGetLoadLibraryVerbose();

/**
 * Scan a given directory for plugins and return a list of the plugin
 * native file names and their respective metadata.
 * \param dir Directory to scan.
 * \param plugins Native file names.
 * \param metadata Metadata.
 * \param recursive Recursively scan all subdirectories.
 * \remark It is the responsibility of the caller to do any cleaning
 *   of \p metadata and \p plugins, if desired.
 * \remark \p plugins can be 0, a string vector will be created in this case.
 * \return If any errors occured, a vector of error descriptions.
 */
csRef<iStrVector> csScanPluginDir (const char* dir, 
				   csRef<iStrVector>& plugins,
				   bool recursive = true);

/**
 * Scan some given directories for plugins.
 * Accepts the same parameters as csScanPluginDir(), with the exception of
 * \p dirs.
 */				
csRef<iStrVector> csScanPluginDirs (csPluginPaths* dirs, 
				    csRef<iStrVector>& plugins);

/**
 * Query plugin metadata.
 * \remark 
 */
csRef<iString> csGetPluginMetadata (const char* fullPath, 
				    csRef<iDocument>& metadata);

/** @} */

/** @} */

#endif // __CS_CSSHLIB_H__
