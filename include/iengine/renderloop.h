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

#ifndef __CS_IENGINE_RENDERLOOP_H__
#define __CS_IENGINE_RENDERLOOP_H__

/**\file
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "csutil/scf.h"

struct iCamera;
struct iClipper2D;
struct csRenderMeshList;
class csRenderView;

SCF_VERSION (iRenderStep, 0, 0, 1);

struct iRenderStep : public iBase
{
  virtual void Perform (csRenderView* rview, csRenderMeshList* meshes) = 0;
};
 
SCF_VERSION (iRenderLoop, 0, 0, 1);

struct iRenderLoop : public iBase
{
  virtual void Draw (iCamera* c, iClipper2D* clipper) = 0;
};
 
/**@}*/

#endif
