/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg

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

#include "csutil/hashmapr.h"
#include "csutil/refarr.h"
#include "iengine/renderloop.h"
#include "iutil/strset.h"
#include "ivideo/render3d.h"
#include "ivideo/shader/shader.h"

class csEngine;
class csRenderView;
class csRenderLoop;

class csLightIteratorRenderStep : public iRenderStep
{
private:
  csRenderLoop* rl;
  csRefArray<iRenderStep> steps;
public:
  SCF_DECLARE_IBASE;

  csLightIteratorRenderStep (csRenderLoop* rl);

  virtual void Perform (csRenderView* rview, iSector* sector);

  void AddStep (iRenderStep* step);
};


class csGenericRenderStep : public iRenderStep
{
private:
  csRenderLoop* rl;
  csStringID shadertype;
  bool firstpass;
  csZBufMode zmode;
public:
  SCF_DECLARE_IBASE;

  csGenericRenderStep (csRenderLoop* rl, csStringID shadertype, 
    bool firstpass, csZBufMode zmode);

  inline void RenderMeshes (iRender3D* r3d, iShader* shader, 
    csRenderMesh** meshes, int num);
  virtual void Perform (csRenderView* rview, iSector* sector);
};

class csRenderLoop : public iRenderLoop
{
protected:
  friend class csLightIteratorRenderStep;
  friend class csGenericRenderStep;

  csEngine* engine;

  csRefArray<iRenderStep> steps;
public:
  SCF_DECLARE_IBASE;

  csRenderLoop (csEngine* engine);

  void StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview);
  virtual void Draw (iCamera* c, iClipper2D* clipper);

  virtual int AddStep (iRenderStep* step);
  virtual int GetStepCount ();
};

class csRenderLoopManager : public iRenderLoopManager
{
  csHashMapReversible loops;
  csStringSet strings;

  csEngine* engine;
public:
  SCF_DECLARE_IBASE;

  csRenderLoopManager(csEngine* engine);
  virtual ~csRenderLoopManager();

  virtual csPtr<iRenderLoop> Create ();
  
  virtual bool Register (const char* name, iRenderLoop* loop);
  virtual iRenderLoop* Retrieve (const char* name);
  virtual const char* GetName (iRenderLoop* loop);
  virtual bool Unregister (iRenderLoop* loop);
};

#endif
#endif

#endif
