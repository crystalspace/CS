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

#include "cssysdef.h"
#include "csengine/renderloop.h"

#ifdef CS_NR_ALTERNATE_RENDERLOOP

#include "cssys/sysfunc.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iengine/material.h"
#include "ivideo/rendersteps/irsfact.h"
#include "ivideo/rendersteps/igeneric.h"

#include "csgfx/rgbpixel.h"
#include "csengine/engine.h"

void csEngine::StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview)
{
}

void csEngine::Draw (iCamera *c, iClipper2D *view)
{
  defaultRenderLoop->Draw (c, view);
}

csPtr<iRenderLoop> csEngine::CreateDefaultRenderLoop ()
{
  csRef<iRenderLoop> loop = renderLoopManager->Create ();

  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));

  csRef<iRenderStepType> genType =
    CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.renderloop.step.generic.type",
      iRenderStepType);

  csRef<iRenderStepFactory> genFact = genType->NewFactory ();

  csRef<iRenderStep> step;
  csRef<iGenericRenderStep> genStep;

  step = genFact->Create ();
  loop->AddStep (step);
  genStep = SCF_QUERY_INTERFACE (step, iGenericRenderStep);
  
  genStep->SetShaderType ("ambient");
  genStep->SetZBufMode (CS_ZBUF_USE);
  genStep->SetZOffset (true);

  csRef<iRenderStepType> liType =
    CS_LOAD_PLUGIN (plugin_mgr,
      "crystalspace.renderloop.step.lightiter.type",
      iRenderStepType);

  csRef<iRenderStepFactory> liFact = liType->NewFactory ();

  step = liFact->Create ();
  loop->AddStep (step);

  csRef<iRenderStepContainer> liContainer =
    SCF_QUERY_INTERFACE (step, iRenderStepContainer);

  step = genFact->Create ();
  liContainer->AddStep (step);

  genStep = SCF_QUERY_INTERFACE (step, iGenericRenderStep);

  genStep->SetShaderType ("diffuse");
  genStep->SetZBufMode (CS_ZBUF_TEST);
  genStep->SetZOffset (false);

  return csPtr<iRenderLoop> (loop);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csRenderLoop)
  SCF_IMPLEMENTS_INTERFACE(iRenderLoop)
  SCF_IMPLEMENTS_INTERFACE(iRenderStepContainer)
SCF_IMPLEMENT_IBASE_END

csRenderLoop::csRenderLoop (csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (engine);
  csRenderLoop::engine = engine;
}

void csRenderLoop::StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview)
{
  rview.SetEngine (engine);
  rview.SetOriginalCamera (c);

/*  iEngineSequenceManager* eseqmgr = GetEngineSequenceManager ();
  if (eseqmgr)
  {
    eseqmgr->SetCamera (c);
  }*/

  // This flag is set in HandleEvent on a cscmdContextResize event
/*  if (resize)
  {
    resize = false;
    Resize ();
  }*/

  rview.GetClipPlane ().Set (0, 0, 1, -1);      //@@@CHECK!!!

  // Calculate frustum for screen dimensions (at z=1).
  float leftx = -c->GetShiftX () * c->GetInvFOV ();
  float rightx = (engine->frame_width - c->GetShiftX ()) * c->GetInvFOV ();
  float topy = -c->GetShiftY () * c->GetInvFOV ();
  float boty = (engine->frame_height - c->GetShiftY ()) * c->GetInvFOV ();
  rview.SetFrustum (leftx, rightx, topy, boty);
}

void csRenderLoop::Draw (iCamera *c, iClipper2D *view)
{
  engine->ControlMeshes ();

  csRenderView rview (c, view, engine->G3D, engine->G2D);
  StartDraw (c, view, rview);

  // First initialize G3D with the right clipper.
  engine->G3D->SetClipper (view, CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  engine->G3D->ResetNearPlane ();
  engine->G3D->SetPerspectiveAspect (c->GetFOV ());

  iSector *s = c->GetSector ();
  //if (s) s->Draw (&rview);
  if (s)
  {
    s->PrepareDraw (&rview);

    int i;
    for (i = 0; i < steps.Length(); i++)
    {
      steps[i]->Perform (&rview, s);
    }
/*    csRenderMeshList meshes;
    s->CollectMeshes (&rview, meshes);

    if (meshes.num) 
    {
      int i;
      for (i = 0; i < steps.Length(); i++)
      {
	steps[i]->Perform (&rview, &meshes);
      }
    }*/
  }

  // draw all halos on the screen
/*  if (halos.Length () > 0)
  {
    csTicks elapsed = virtual_clock->GetElapsedTicks ();
    for (int halo = halos.Length () - 1; halo >= 0; halo--)
      if (!halos[halo]->Process (elapsed, *this)) halos.Delete (halo);
  }*/

  engine->G3D->SetClipper (0, CS_CLIPPER_NONE);

  //csSleep (1000);
}

int csRenderLoop::AddStep (iRenderStep* step)
{
  return steps.Push (step);
}

int csRenderLoop::GetStepCount ()
{
  return steps.Length();
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csRenderLoopManager)
  SCF_IMPLEMENTS_INTERFACE(iRenderLoopManager)
SCF_IMPLEMENT_IBASE_END

csRenderLoopManager::csRenderLoopManager(csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (0);

  csRenderLoopManager::engine = engine;
}

csRenderLoopManager::~csRenderLoopManager()
{
  csGlobalHashIteratorReversible it (&loops);
  while (it.HasNext())
  {
    iRenderLoop* loop = (iRenderLoop*)it.Next();
    loop->DecRef ();
  }
}

csPtr<iRenderLoop> csRenderLoopManager::Create ()
{
  csRenderLoop* loop = new csRenderLoop (engine);
  return csPtr<iRenderLoop> (loop);
}
  
bool csRenderLoopManager::Register (const char* name, iRenderLoop* loop)
{
  const char* myName = strings.Request (strings.Request (name));
  if (loops.Get (myName) != 0) return false;
  loop->IncRef();
  loops.Put (myName, (csHashObject)loop);
  return true;
}
 
iRenderLoop* csRenderLoopManager::Retrieve (const char* name)
{
  return (iRenderLoop*)(loops.Get (name));
}

const char* csRenderLoopManager::GetName (iRenderLoop* loop)
{
  return loops.GetKey ((csHashObject)loop);
}

bool csRenderLoopManager::Unregister (iRenderLoop* loop)
{
  const char* key;
  if ((key = loops.GetKey ((csHashObject)loop)) == 0) return false;
  loop->DecRef();
  loops.Delete (key, (csHashObject)loop);
  return false;
}

#endif
