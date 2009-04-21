/*
    Crystal Space 3D engine
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_IENGINE_RENDERSTEPS_ICONTAINER_H__
#define __CS_IENGINE_RENDERSTEPS_ICONTAINER_H__

#include "csutil/scf.h"

/**\file
 * Render step cointainer
 */

/**\addtogroup engine3d_rloop
 * @{ */

struct iRenderStep;


/**
 * Container for render steps.
 */
struct iRenderStepContainer : public virtual iBase
{
  SCF_INTERFACE(iRenderStepContainer, 2,0,0);
  /// Add a step
  virtual size_t AddStep (iRenderStep* step) = 0;
  /// Remove a given step
  virtual bool DeleteStep (iRenderStep* step) = 0;
  /// Obtain a step
  virtual iRenderStep* GetStep (size_t n) const = 0;
  /// Find the index of a step
  virtual size_t Find (iRenderStep* step) const = 0;
  /// Get the number of steps
  virtual size_t GetStepCount () const = 0;
};

/** @} */

#endif
