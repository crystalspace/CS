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

#ifndef __IUTIL_CMDLINE_H__
#define __IUTIL_CMDLINE_H__

#include "csutil/scf.h"

SCF_VERSION (iCommandLineParser, 0, 0, 1);

/// A utility class that makes it easier to parse the command line.
struct iCommandLineParser : public iBase
{
  /**
   * Initialize for the given command line (clears any information from
   * previously used command lines).
   */
  virtual void Initialize (int argc, const char* const argv[]) = 0;

  /// Query a specific commandline option (you can query second etc such option)
  virtual const char *GetOption (const char *iName, int iIndex = 0) const = 0;
  /// Query a filename specified on the commandline (that is, without leading '-')
  virtual const char *GetName (int iIndex = 0) const = 0;
  /// Add a command-line option to the command-line option array
  virtual void AddOption (const char *iName, const char *iValue) = 0;
  /// Add a command-line name to the command-line names array
  virtual void AddName (const char *iName) = 0;
  /// Replace the Nth command-line option with a new value
  virtual bool ReplaceOption (const char *iName, const char *iValue, int iIndex = 0) = 0;
  /// Replace the Nth command-line name with a new value
  virtual bool ReplaceName (const char *iValue, int iIndex = 0) = 0;
};

#endif // __IUTIL_CMDLINE_H__
