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

#ifndef __CS_CSENGINE_RENDERLOOP_H__
#define __CS_CSENGINE_RENDERLOOP_H__

#ifdef CS_USE_NEW_RENDERER
#define CS_NR_ALTERNATE_RENDERLOOP

#ifdef CS_NR_ALTERNATE_RENDERLOOP

#include "iengine/renderloop.h"
#include "csengine/engine.h"

class csRenderView;
class csRenderLoop;

class csAmbientRenderStep : public iRenderStep
{
private:
  csRenderLoop* rl;
public:
  SCF_DECLARE_IBASE;

  csAmbientRenderStep (csRenderLoop* rl);

  virtual void Perform (csRenderView* rview, iSector* sector);
};

class csLightingRenderStep : public iRenderStep
{
private:
  csRenderLoop* rl;
public:
  SCF_DECLARE_IBASE;

  csLightingRenderStep (csRenderLoop* rl);

  inline void RenderMeshes (iRender3D* r3d, iShader* shader, 
    csRenderMesh** meshes, int num);
  virtual void Perform (csRenderView* rview, iSector* sector);
};

class csRenderLoop : public iRenderLoop
{
protected:
  friend class csAmbientRenderStep;
  friend class csLightingRenderStep;

  csEngine* engine;

  csRefArray<iRenderStep> steps;
public:
  SCF_DECLARE_IBASE;

  csRenderLoop (csEngine* engine);

  void StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview);
  virtual void Draw (iCamera* c, iClipper2D* clipper);
};

#endif
#endif

#endif
