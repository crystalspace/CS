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

#include "iutil/cmdline.h"
#include "csutil/typedvec.h"
#include "csutil/csstrvec.h"
#include "csutil/util.h"

struct csCommandLineOption;

/// A utility class that makes it easier to parse the command line.
class csCommandLineParser : public iCommandLineParser
{
private:
  // A vector of command line options
  CS_DECLARE_TYPED_VECTOR_NODELETE (csCommandLineOptionVectorHelper, csCommandLineOption);

  class csCommandLineOptionVector : public csCommandLineOptionVectorHelper
  {
  public:
    virtual bool FreeItem (csSome item);
    virtual int CompareKey (csSome Item, csConstSome Key, int Mode) const;
  };

  // The array of all command-line options.
  csCommandLineOptionVector Options;
  // The list of raw filenames on the command line (i.e. without any switches)
  csStrVector Names;

  // Find Nth command-line option and return a pointer to the object (or NULL)
  csCommandLineOption *FindOption (const char *iName, int iIndex) const;

public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csCommandLineParser (iBase *Parent = NULL);
  /// Construct with a given command line.
  csCommandLineParser (int argc, const char* const argv[]);
  /// Destructor.
  virtual ~csCommandLineParser() {}

  /**
   * Initialize for the given command line.  Options from command line are
   * added to any options already present --- i.e. those added via AddName()
   * or AddOption().
   */
  virtual void Initialize (int argc, const char* const argv[]);

  /// Clear all options and names.
  virtual void Reset ();
  /// Query specific commandline option (you can query second etc. such option)
  virtual const char *GetOption (const char *iName, int iIndex = 0) const;
  /// Query filename specified on commandline (that is, without leading '-')
  virtual const char *GetName (int iIndex = 0) const;
  /// Add a command-line option to the command-line option array
  virtual void AddOption (const char *iName, const char *iValue);
  /// Add a command-line name to the command-line names array
  virtual void AddName (const char *iName);
  /// Replace the Nth command-line option with a new value
  virtual bool ReplaceOption (const char *iName, const char *iValue,
    int iIndex = 0);
  /// Replace the Nth command-line name with a new value
  virtual bool ReplaceName (const char *iValue, int iIndex = 0);
};

#endif // __CS_CMDLINE_H__
