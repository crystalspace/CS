/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_SYSFUNC_H__
#define __CS_SYSFUNC_H__

/**\file
 * Common system functions.
 */
/**\addtogroup util
 * @{
 */
/**\name System functions
 * @{
 */

#include <stdarg.h>
#include <stdio.h>
#include "csextern.h"
#include "ref.h"
#include "csstring.h"
#include "iutil/cfgfile.h"

struct iObjectRegistry;

/**
 * Implements a default run-loop for stand-alone applications.<p>
 * This function implements a run-loop for stand-alone applications which do
 * not provide their own run-loop.  You only need to call this function if your
 * application does not otherwise implement its own run-loop.  For example, an
 * existing Microsoft Windows-based application will already have a run-loop
 * which processes the Windows event-queue, among other tasks.  Such an
 * application should not call this function, since doing so would interrupt
 * the existing run-loop and prevent it from running.  For each iteration of
 * the run-loop, this function increments the Crystal Space virtual clock (if
 * it is present in the shared-object registry at the time of the call to this
 * function) and processes the Crystal Space event queue (which must be present
 * in the shared-object registry at the time of the call to this function).
 * This function only returns after a csevBroadcast message has been received
 * in the Crystal Space event queue with command code cscmdQuit.  The return
 * value of this function indicates whether or not the run-loop was actually
 * able to run.  If there was a problem starting the run-loop, then `false' is
 * returned, otherwise `true' is returned.
 */
CS_CRYSTALSPACE_EXPORT bool csDefaultRunLoop(iObjectRegistry*);

/**
 * Platform-specific startup.<p>
 * Some platforms on which Crystal Space runs need to perform their own startup
 * processing.  For instance, on some platforms, platform-specific "helper"
 * objects may be placed in the shared-object registry for use by other
 * platform-specific components.  Typically, this function is automatically
 * called by the higher-level csInitializer::CreateEnvironment(), however if
 * you are performing application initialization manually, you should call it
 * yourself.  Returns `true' if startup initialization was successful,
 * otherwise `false'.
 */
CS_CRYSTALSPACE_EXPORT bool csPlatformStartup(iObjectRegistry*);

/**
 * Platform-specific shutdown.<p>
 * Some platforms on which Crystal Space runs need to perform their own
 * shutdown processing.  Typically, this function is automatically called by
 * the higher-level csInitializer::DestroyApplication(), however if you are
 * performing application shutdown manually, you should call it yourself.
 * Returns `true' if shutdown processing was successful, otherwise `false'.
 */
CS_CRYSTALSPACE_EXPORT bool csPlatformShutdown(iObjectRegistry*);

/**
 * CS version of printf.
 * It accepts UTF-8 strings and converts it, if required, to the platforms
 * native codepage.
 */
CS_CRYSTALSPACE_EXPORT int csPrintf (const char* str, ...) CS_GNUC_PRINTF (1, 2);
/**
 * CS version of vprintf.
 * \copydoc csPrintf()
 */
CS_CRYSTALSPACE_EXPORT int csPrintfV (const char* str, va_list arg) CS_GNUC_PRINTF (1, 0);
/**
 * CS version of fprintf (file, str, ...). 
 * \copydoc csPrintf()
 * \remarks Will not flush file.
 */
CS_CRYSTALSPACE_EXPORT int csFPrintf (FILE* file, const char* str, ...) 
  CS_GNUC_PRINTF (2, 3);
/**
 * CS version of vfprintf (stderr, str, ...). 
 * \copydoc csPrintf()
 * \remarks Will not flush file.
 */
CS_CRYSTALSPACE_EXPORT int csFPrintfV (FILE* file, const char* str, va_list arg) 
  CS_GNUC_PRINTF (2, 0);
/**
 * CS version of vfprintf (stderr, str, ...). 
 * \copydoc csPrintf()
 * \remarks Will flush stderr.
 */
CS_CRYSTALSPACE_EXPORT int csPrintfErrV (const char* str, va_list arg) 
  CS_GNUC_PRINTF (1, 0);
/**
 * CS version of fprintf (stderr, str, ...). 
 * \copydoc csPrintf()
 * \remarks Will flush stderr.
 */
CS_CRYSTALSPACE_EXPORT int csPrintfErr (const char* str, ...) CS_GNUC_PRINTF (1, 2);

/**
 * Get the current tick count. Warning! Do NOT use this function for
 * controlling game logic speed. To do that you should use the virtual
 * clock (iVirtualClock) which you can get from the object registry. By
 * using the virtual clock it will be possible to control the speed of your
 * game and also to pause it if needed.
 */
CS_CRYSTALSPACE_EXPORT csTicks csGetTicks ();

/**
 * This function will freeze your application for given number of 1/1000
 * seconds. The function is very inaccurate, so don't use it for accurate
 * timing. It may be useful when the application is idle, to explicitly
 * release CPU for other tasks in multi-tasking operating systems.
 */
CS_CRYSTALSPACE_EXPORT void csSleep (int /*SleepTime*/);

/**
 * Get the username of the account running the program.<p>
 * Returns the username of the owner of the process running the program.
 * If the username can not be determined, then an empty string is returned.
 */
CS_CRYSTALSPACE_EXPORT csString csGetUsername();

/**
 * Get a platform-specific per-user config object.
 * The data is stored in a platform-specific manner - e.g. in "Documents and
 * Settings\\Application Data" on Windows, or $HOME on Unix.
 * \param key Used to distinguish different stored configurations. Should be
 *  the application ID, e.g. "MyGames.ClickFrenzy2".
 * \return A config 'file'. Might return 0 on some platforms or in case an
 *   error occured.
 */
CS_CRYSTALSPACE_EXPORT csPtr<iConfigFile> csGetPlatformConfig (const char* key);

/**
 * Get a platform-specific path to store per-user configuration data.
 * The path is in a platform-specific place - e.g. in "Documents and
 * Settings\\Application Data" on Windows, or $HOME on Unix.
 * \param key Used to distinguish different stored configurations. Should be
 *  the application ID, e.g. "MyGames.ClickFrenzy2".
 * \return A native path  suitable to store per-user configuration data, in
 *   the form of e.g. "...\MyGames\ClickFrenzy2". The path could then be used
 *   as a directory name or a base for a file name.
 * \remarks The returned path may not exist. The caller has to ensure its
 *   existance before using it.
 */
CS_CRYSTALSPACE_EXPORT csString csGetPlatformConfigPath (const char* key);

/** @} */

/** @} */

#endif // __CS_SYSFUNC_H__
