/*
    Copyright (C) 1998,1999,2000 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_CMDLINE_H__
#define __CS_IUTIL_CMDLINE_H__

/**\file
 * Command line parsing utility
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf.h"


SCF_VERSION (iCommandLineParser, 0, 0, 3);

/**
 * A utility class that makes it easier to parse the command line.
 *
 * Main creators of instances implementing this interface:
 * - csInitializer::CreateEnvironment()
 * - csInitializer::CreateCommandLineParser()
 *
 * Main ways to get pointers to this interface:
 * - csQueryRegistry()
 */
struct iCommandLineParser : public iBase
{
  /**
   * Initialize for the given command line.  Options from command line are
   * added to any options already present --- i.e. those added via AddName()
   * or AddOption().
   */
  virtual void Initialize (int argc, const char* const argv[]) = 0;
  /// Clear all options and names.
  virtual void Reset () = 0;
  /// Query specific commandline option (you can query second etc. such option)
  virtual const char *GetOption (const char *iName, size_t iIndex = 0)
  	const = 0;
  /// Query filename specified on commandline (that is, without leading '-')
  virtual const char *GetName (size_t iIndex = 0) const = 0;
  /// Add a command-line option to the command-line option array
  virtual void AddOption (const char *iName, const char *iValue) = 0;
  /// Add a command-line name to the command-line names array
  virtual void AddName (const char *iName) = 0;
  /// Replace the Nth command-line option with a new value
  virtual bool ReplaceOption (const char *iName, const char *iValue,
    size_t iIndex = 0) = 0;
  /// Replace the Nth command-line name with a new value
  virtual bool ReplaceName (const char *iValue, size_t iIndex = 0) = 0;
  /**
   * Check for a -[no]option toggle. The difference to using GetOption() to
   * check for the two possibilities is that this function respects the
   * argument order.<br> 
   * Example: the result of evaluating the arguments 
   * <tt>-option -nooption</tt> would depend on if you either check for
   * "option" or "nooption" using GetOption(), while GetBoolOption() returns
   * false because it looks for the <em>last</em> toggle argument.
   * \param iName The name of the positive toggle argument.  The negative
   *	argument is created by inserting "no" in front of it.
   * \param defaultValue The default value, if neither of the toggle arguments
   *	is found.
   */
  virtual bool GetBoolOption (const char *iName, 
    bool defaultValue = false) = 0;
    
  /**
   * Returns the directory in which the application's resources resides.  On
   * many platforms, this may be the same as the directory returned by
   * GetAppDir(); however, on MacOS/X, it is the "Resources" directory within
   * the Cocoa application wrapper.
   */
  virtual const char* GetResourceDir () = 0;

  /**
   * Returns the directory in which the application executable resides; or the
   * directory in which the Cocoa application wrapper resides on MacOS/X.
   */
  virtual const char* GetAppDir () = 0;

  /**
   * Returns the full path to the application executable.
   */
  virtual const char* GetAppPath () = 0;
};

/** @} */

#endif // __CS_IUTIL_CMDLINE_H__
