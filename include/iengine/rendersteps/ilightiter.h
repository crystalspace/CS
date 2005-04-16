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

#ifndef __CS_IENGINE_RENDERSTEPS_ILIGHTITER_H__
#define __CS_IENGINE_RENDERSTEPS_ILIGHTITER_H__

#include "csutil/scf.h"

/**\file
 * Generic render step.
 */

/**\addtogroup engine3d_rloop
 * @{ */

SCF_VERSION (iLightIterRenderStep, 0, 0, 1);

/// Document me!@@@
struct iLightIterRenderStep : public iBase
{
};

SCF_VERSION (iLightRenderStep, 0, 0, 1);

/// Document me!@@@
struct iLightRenderStep : public iBase
{
  virtual void Perform (iRenderView* rview, iSector* sector,
    iLight* light, csShaderVarStack &stacks) = 0;
};

/** @} */

#endif
