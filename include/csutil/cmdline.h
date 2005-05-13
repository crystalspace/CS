/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_CMDLINE_H__
#define __CS_CMDLINE_H__

/**\file
 * iCommandLineParser implementation
 */

#include "csextern.h"
#include "iutil/cmdline.h"
#include "parray.h"
#include "stringarray.h"
#include "util.h"

/// Representation of a commandline option.
struct CS_CRYSTALSPACE_EXPORT csCommandLineOption
{
  /// Option name
  char *Name;
  /// Option value
  char *Value;
  /// Name and Value should be already allocated
  csCommandLineOption (char *iName, char *iValue)
  {
    Name = iName;
    Value = iValue;
  }
  /// Destructor
  ~csCommandLineOption ()
  { delete [] Name; delete [] Value; }
};


/// Utility class that makes it easier to parse the command line.
class CS_CRYSTALSPACE_EXPORT csCommandLineParser : public iCommandLineParser
{
private:
  /// A vector of command line options
  typedef csPDelArray<csCommandLineOption> csCommandLineOptionVector;

  /// The array of all command-line options.
  csCommandLineOptionVector Options;
  /// The list of raw filenames on the command line (i.e. without any switches)
  csStringArray Names;

  /// Find Nth command-line option and return a pointer to the object (or 0)
  csCommandLineOption *FindOption (const char *iName, size_t iIndex) const;

  /// Directory of application resources.
  csString resDir;
  /// Directory of application executable.
  csString appDir;
  /// Full path of application executable.
  csString appPath;
public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCommandLineParser (iBase *Parent = 0);
  /// Construct with a given command line.
  csCommandLineParser (int argc, const char* const argv[]);
  /// Destructor.
  virtual ~csCommandLineParser();

  /**
   * Initialize for the given command line.  Options from command line are
   * added to any options already present --- i.e. those added via AddName()
   * or AddOption().
   */
  virtual void Initialize (int argc, const char* const argv[]);

  /// Clear all options and names.
  virtual void Reset ();
  /// Query specific commandline option (you can query second etc. such option)
  virtual const char *GetOption (const char *iName, size_t iIndex = 0) const;
  /// Query filename specified on commandline (that is, without leading '-')
  virtual const char *GetName (size_t iIndex = 0) const;
  /// Add a command-line option to the command-line option array
  virtual void AddOption (const char *iName, const char *iValue);
  /// Add a command-line name to the command-line names array
  virtual void AddName (const char *iName);
  /// Replace the Nth command-line option with a new value
  virtual bool ReplaceOption (const char *iName, const char *iValue,
    size_t iIndex = 0);
  /// Replace the Nth command-line name with a new value
  virtual bool ReplaceName (const char *iValue, size_t iIndex = 0);
  /**
   * Check for a -[no]option toggle. 
   */
  virtual bool GetBoolOption (const char *iName, 
    bool defaultValue = false);

  /**
   * Returns the directory in which the application's resources resides.  On
   * many platforms, this may be the same as the directory returned by
   * GetAppDir(); however, on MacOS/X, it is the "Resources" directory within
   * the Cocoa application wrapper.
   */
  virtual const char* GetResourceDir ();

  /**
   * Returns the directory in which the application executable resides; or the
   * directory in which the Cocoa application wrapper resides on MacOS/X.
   */
  virtual const char* GetAppDir ();

  /**
   * Returns the full path to the application executable.
   */
  virtual const char* GetAppPath ();
};

#endif // __CS_CMDLINE_H__
