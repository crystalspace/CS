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
 * Add one element to shared library search path;
 * the path should end in '/' or whatever the path separator is,
 * that is, it should be immediately prependable to shared library name.
 */
void csAddLibraryPath (char const* iPath);

/**
 * Find a shared library in library search path and load it.
 * Same as csLoadLibrary except that you give just the name of the
 * module, without any prefix/suffix.
 */
csLibraryHandle csFindLoadLibrary (char const* iModule);

/**
 * Same but you give the possible suffix and prefix. This is usually called
 * by the system-dependent implementation of csFindLoadLibrary, and not
 * by the user. iPrefix can be either NULL or something like "lib";
 * the routine tries both with (if it is not NULL) and without prefix.
 * Same about iSuffix - it can be something like ".dll" or ".so", but
 * not NULL (because all OSes use some suffix for shared libs).
 */
csLibraryHandle csFindLoadLibrary (char const* iPrefix,
  char const* iName, char const* iSuffix);

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

#endif // __CS_CSSHLIB_H__
