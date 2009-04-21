/*
    Copyright (C) 2007 by Jorrit Tyberghein

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

#ifndef __CS_IVARIA_BULLET_H__
#define __CS_IVARIA_BULLET_H__

/**\file
 * Bullet-specific interfaces
 */

#include "csutil/scf_interface.h"

struct iView;

/**
 * The Bullet implementation of iDynamicSystem also implements this
 * interface.
 */
struct iBulletDynamicSystem : public virtual iBase
{
  SCF_INTERFACE(iBulletDynamicSystem, 2, 0, 0);

  /**
   * Draw debug information for all colliders managed by bullet.
   */
  virtual void DebugDraw (iView* rview) = 0;
};

#endif // __CS_IVARIA_BULLET_H__

