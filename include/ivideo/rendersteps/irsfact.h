/*
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

#ifndef __CS_IVIDEO_RENDERSTEPS_ITEXFACT_H__
#define __CS_IVIDEO_RENDERSTEPS_ITEXFACT_H__

/**\file
 * Render step factory.
 */

/**
 * \addtogroup gfx3d
 * @{ */
 
#include "csutil/scf.h"

struct iRenderStep;

SCF_VERSION (iRenderStepFactory, 0, 0, 1);

/**
 * Interface to a render step factory.
 */
struct iRenderStepFactory : public iBase
{
  /**
   * Generate a new texture with the selected parameters.
   */
  virtual csPtr<iRenderStep> Create () = 0;
};

SCF_VERSION (iRenderStepType, 0, 0, 1);

/**
 * Texture type.
 * Interface used to create instances of iTextureFactory.
 */
struct iRenderStepType : public iBase
{
  /**
   * Create a new instance of a texture factory.
   */
  virtual csPtr<iRenderStepFactory> NewFactory() = 0;
};

/** @} */

#endif
