/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_CSUTIL_VERBOSITY_H__
#define __CS_CSUTIL_VERBOSITY_H__

/**\file
 * Verbosity management helpers
 */
#include "csextern.h"
#include "iutil/verbositymanager.h"

#include "csutil/csstring.h"
#include "csutil/hashhandlers.h"

/**
 * Class to parse verbosity flags and allow checking of them.
 * This class is also internally used by csCheckVerbosity() and
 * csVerbosityManager.
 * <p>
 * The syntax is <tt>[+|-]class{:[+|-]subclass}{,[+|-]class{:[+|-]subclass}}</tt>.
 * <tt>class</tt> and <tt>subclass</tt> specify the verbosity class and, for 
 * more fine-grained control, the verbosity subclass names; they match the 
 * strings passed to CheckFlag() at runtime. The "<tt>+</tt>" and "<tt>-</tt>"
 * flags specify the actual verbosity; "<tt>+</tt>" enables, "<tt>-</tt>"
 * disables verbosity for a specific class or subclass. There are the special
 * class and subclass names "<tt>*</tt>" which control the default verbosity
 * for all classes or subclasses of a class; they allow to enable or disable 
 * only specific classes or subclasses, e.g. <tt>--verbose=*,-scf</tt> will
 * enable verbosity for everything except the SCF diagnostic information.
 */
class CS_CRYSTALSPACE_EXPORT csVerbosityParser
{
  struct VerbosityFlag
  {
    csString msgClass;
    /**
     * The default verbosity flag. Used when a subclass was requested,
     * but no specific flag was set for it, or when no subclass is requested.
     */
    bool defaultFlag;
    csHash<bool, csStrKey, csConstCharHashKeyHandler> msgSubclasses;
    
    VerbosityFlag() : defaultFlag(true) {}
  };
  csArray<VerbosityFlag> verbosityFlags;
  enum
  {
    ForceResult = 1,
    DefTrue = 2,
    DefFalse = 0,
    DefMask = DefTrue
  };
  uint defaultGlobalFlag;

  static int VfKeyCompare (const VerbosityFlag& vf, const char* const& K);
  static int VfCompare (const VerbosityFlag& vf1, const VerbosityFlag& vf2);
public:
  /**
   * Construct the verbose flag parser.
   * \param flags Verbosity flags. <b>Note:</b> 0 and "" have different meanings:
   *  0 means "always return false", "" means "always return true".
   */
  csVerbosityParser (const char* flags);
  
  /**
   * Check for the verbosity of a class and subclass pair.
   */
  bool CheckFlag (const char* msgClass, const char* msgSubclass = 0);
};

/**
 * Parse verbosity for some given command line arguments.
 * \remarks Should only be used during early initializations when the object 
 *  registry is not set up yet; otherwise, the iVerbosityManager object should
 *  be obtained and used instead.
 */
extern CS_CRYSTALSPACE_EXPORT bool csCheckVerbosity (int argc,
  const char* const argv[], const char* msgClass,
  const char* msgSubclass = 0);
  
/**
 * Default iVerbosityManager implementation. Basically a thin wrapper around 
 * csVerbosityParser.
 */
class CS_CRYSTALSPACE_EXPORT csVerbosityManager : public iVerbosityManager
{
  csVerbosityParser vp;
public:
  SCF_DECLARE_IBASE;

  csVerbosityManager (const char* flags) : vp (flags)
  {
    SCF_CONSTRUCT_IBASE(0);
  }
  virtual ~csVerbosityManager()
  {
    SCF_DESTRUCT_IBASE();
  }

  virtual bool CheckFlag (const char* msgClass, 
    const char* msgSubclass = 0)
  { return vp.CheckFlag (msgClass, msgSubclass); }
};

#endif // __CS_CSUTIL_VERBOSITY_H__
