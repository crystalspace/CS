/*
    Copyright (C) 1999 by Andrew Zabolotny <bit@eltech.ru>
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __IENGINE_DYNLIGHT_H__
#define __IENGINE_DYNLIGHT_H__

#include "csutil/scf.h"

class csDynLight;
struct iObject;
struct iLight;

SCF_VERSION (iDynLight, 0, 0, 1);

/**
 * The iDynLight interface represents a dynamic light.
 */
struct iDynLight : public iBase
{
  /// Get the private pointer to csDynLight (ugly).
  virtual csDynLight* GetPrivateObject () = 0;

  /// Get the iObject for this light.
  virtual iObject *QueryObject () = 0;
  /// Get the iLight for this light.
  virtual iLight *QueryLight () = 0;

  /// Setup the light (i.e. do the lighting calculations).
  virtual void Setup () = 0;

  /// Get the next dynamic light in the list.
  virtual iDynLight* GetNext () = 0;
};

#endif // __IENGINE_DYNLIGHT_H__

