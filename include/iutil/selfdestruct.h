/*
    Crystal Space Data Buffer interface
    Copyright (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_SELFDESTRUCT_H__
#define __CS_IUTIL_SELFDESTRUCT_H__

/**\file
 * Self destruction interface.
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf_interface.h"

/**
 * An object implementing this interface can remove itself from its
 * 'natural parent'. For example, if this is used on a mesh object then
 * the engine will be the natural parent. This is used by
 * iEngine->RemoveObject().
 */
struct iSelfDestruct : public virtual iBase
{
  SCF_INTERFACE(iSelfDestruct, 1,0,0);
  /// Remove me!
  virtual void SelfDestruct () = 0;
};
/** @} */

#endif // __CS_IUTIL_SELFDESTRUCT_H__
