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

#include "iutil/verbositymanager.h"

#include "csutil/csstring.h"
#include "csutil/hashhandlers.h"

class csVerbosityParser
{
  struct VerbosityFlag
  {
    csString msgClass;
    bool defaultFlag;
    csHash<bool, csStrKey, csConstCharHashKeyHandler> msgSubclasses;
    
    VerbosityFlag() : msgClass ("") {}
  };
  csArray<VerbosityFlag> verbosityFlags;
  enum
  {
    ForceResult = 1,
    ForceTrue = 2,
    ForceFalse = 0
  };
  uint all;

  static int VfKeyCompare (const VerbosityFlag& vf, const char* const& K);
  static int VfCompare (const VerbosityFlag& vf1, const VerbosityFlag& vf2);
public:
  /**
   * Construct the verbose flag parser.
   * \param flags Verbosity flags. <b>Note:</b> 0 and "" have different meanings:
   *  0 means "always return false", "" means "always return true".
   */
  csVerbosityParser (const char* flags);

  bool CheckFlag (const char* msgClass, const char* msgSubclass = 0);
};

extern CS_CSUTIL_EXPORT bool csCheckVerbosity (int argc,
  const char* const argv[], const char* msgClass,
  const char* msgSubclass = 0);
  
class csVerbosityManager : public iVerbosityManager
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
