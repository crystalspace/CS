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

#ifndef __CS_SYSTEM_H__
#define __CS_SYSTEM_H__

#include <stdarg.h>
#include <stdio.h>

#include "csutil/util.h"
#include "csutil/csstrvec.h"
#include "csutil/csobjvec.h"
#include "csutil/typedvec.h"
#include "iutil/vfs.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/config.h"
#include "iutil/objreg.h"

/**
 * This is the interface to operating system.<p>
 * This driver takes care of all system-dependent parts such as
 * video hardware and input hardware. Note that system-dependent
 * source code should NOT write implementations for methods of
 * csSystemDriver (they are already implemented in system.cpp),
 * but inherit a new class from csSystemDriver, overriding desired
 * methods. Note that some methods it is required to override,
 * otherwise program simply will not compile (they are marked
 * as abstract).
 * <p>
 * This is an abstract class since it does not implement the iBase
 * interface. The iBase interface is supposed to be implemented
 * in SysSystemDriver which should be derived from csSystemDriver.
 */
class csSystemDriver : public iBase
{
  friend class SysSystemDriver;

protected:
  // The object registry.
  iObjectRegistry* object_reg;

public:
  /// Initialize system-dependent data
  csSystemDriver (iObjectRegistry* object_reg);
  /// Deinitialize system-dependent parts
  virtual ~csSystemDriver () { }

  /// This is usually called right after object creation.
  virtual bool Initialize () { return true; }

  SCF_DECLARE_IBASE;
};

#endif // __CS_SYSTEM_H__

