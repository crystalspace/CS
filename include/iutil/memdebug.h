/*
    Copyright (C) 2000 by Eric Sunshine <sunshine@sunshineco.com>

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

#ifndef __CS_IUTIL_MEMDEBUG_H__
#define __CS_IUTIL_MEMDEBUG_H__

#include "csutil/scf.h"

/**\file
 * Memory tracker interface
 */

SCF_VERSION (iMemoryTracker, 0, 0, 1);

/**
 * This interface is used with CS_MEMORY_TRACKER. In that case there will
 * be an object with tag "crystalspace.utilities.memorytracker" in the object
 * registry that implements this interface.
 */
struct iMemoryTracker : public iBase
{
  /// Dump all information.
  virtual void Dump (bool summary_only) = 0;
};

#endif // __CS_IUTIL_MEMDEBUG_H__

