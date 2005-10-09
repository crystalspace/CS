/*
    Copyright (C) 2004 by Frank Richter
    Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_IUTIL_VERBOSITYMANAGER_H__
#define __CS_IUTIL_VERBOSITYMANAGER_H__

/**\file
 * Verbosity management interface
 */

#include "csutil/scf_interface.h"

/**
 * Helper interface which applications and plugins can use to determine whether
 * they should be <em>verbose</em>. At startup, <tt>--verbose=flags</tt>
 * command-line options are parsed. The results are made available via an
 * instance of this interface in the global iObjectRegitry once
 * csInitializer::CreateEnvironment() or
 * csInitializer::CreateVerbosityManager() has been invoked.  If you need to
 * check verbosity during very early initialization, before an
 * iVerbosityManager is available globally, you can instead use
 * csParseVerbosity(), csCheckVerbosity(), or csVerbosityParser.
 * <p>
 * See the csVerbosityParser class description in <csutil/verbosity.h> for a
 * highly detailed disucssion of the <tt>--verbose=flags</tt> option syntax.
 * \sa csVerbosityParser
 * \sa csVerbosityManager
 * \sa csCheckVerbosity
 * \sa csParseVerbosity
 */
struct iVerbosityManager : public virtual iBase
{
  SCF_INTERFACE(iVerbosityManager, 2,0,0);
  /**
   * Parse additional verbosity flags.
   * \remarks See csVerbosityParser::Parse() for detailed information
   *   regarding the interpretation of \a flags.
   */
  void Parse(char const* flags);

  /**
   * Check if verbosity should be enabled for a particular flag.
   * \param flag The flag for which verboseness should be queried.
   * \param fuzzy Whether the search should match \a flag exactly (\a fuzzy =
   *   false) or if it can traverse the inheritance chain when searching for a
   *   match (\a fuzzy = true).
   * <p>
   * \remarks See the csVerbosityParser class description and
   *   csVerbosityParser::Enabled() for detailed information regarding the
   *   interpretation of \a flag and \a fuzzy.
   */
  virtual bool Enabled(char const* flag = 0, bool fuzzy = true) const = 0;

  /**
   * Given major and minor components, check if the verbosity class
   * "major.minor" is enabled.
   * \deprecated Use instead the more generic Enabled() method, which accepts
   *   any granularity of class breakdown; not just major and minor components.
   */
  CS_DEPRECATED_METHOD
  virtual bool CheckFlag(char const* major, char const* minor) const = 0;
};

#endif // __CS_IUTIL_VERBOSITYMANAGER_H__
