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

#ifndef __CS_IUTIL_VERBOSITYMANAGER_H__
#define __CS_IUTIL_VERBOSITYMANAGER_H__

/**\file
 * Verbosity management helper
 */

#include "csutil/scf.h"

SCF_VERSION (iVerbosityManager, 0, 0, 1);

/**
 * Helper interface plugins can use to determine whether they should be 
 * "verbose". At startup, the "-verbose" command line flag is parsed, and
 * plugins can subsequently check whether their verbose flag is set.
 */
struct iVerbosityManager : public iBase
{
  /// Check for the verbosity of a class and subclass pair.
  virtual bool CheckFlag (const char* msgClass, 
    const char* msgSubclass = 0) = 0;
};

#endif // __CS_IUTIL_VERBOSITYMANAGER_H__
