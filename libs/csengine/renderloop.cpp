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
#include "cssys/sysfunc.h"

#include "iutil/objreg.h"

#include "iengine/material.h"
#include "csgfx/rgbpixel.h"
#include "csengine/renderloop.h"
#include "csengine/engine.h"

#ifdef CS_NR_ALTERNATE_RENDERLOOP

void csEngine::StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview)
{
}

void csEngine::Draw (iCamera *c, iClipper2D *view)
{
  DefaultRenderLoop->Draw (c, view);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csLightIteratorRenderStep)
SCF_IMPLEMENTS_INTERFACE(iRenderStep)
SCF_IMPLEMENT_IBASE_END

csLightIteratorRenderStep::csLightIteratorRenderStep (csRenderLoop* rl)
{
  SCF_CONSTRUCT_IBASE (0);

  csLightIteratorRenderStep::rl = rl;
}

void csLightIteratorRenderStep::Perform (csRenderView* rview, iSector* sector)
{
  iRender3D* r3d = rl->engine->G3D;

  r3d->SetLightParameter (0, CS_LIGHTPARAM_SPECULAR, 
    csVector3 (0, 0, 0));

  iLightList* lights = sector->GetLights();

  int nlights = lights->GetCount();

  while (nlights-- > 0)
  {
    iLight* light = lights->Get (nlights);
    const csVector3 lightPos = light->GetCenter ();

    /* 
    @@@ material specific diffuse/specular/ambient.
    Realized as shader variables maybe?
    */
    csReversibleTransform camTransR = 
      rview->GetCamera()->GetTransform();
    r3d->SetObjectToCamera (&camTransR);

    const csColor& color = light->GetColor ();
    r3d->SetLightParameter (0, CS_LIGHTPARAM_DIFFUSE, 
    csVector3 (color.red, color.green, color.blue));

    r3d->SetLightParameter (0, CS_LIGHTPARAM_ATTENUATION,
    light->GetAttenuationVector ());
    r3d->SetLightParameter (0, CS_LIGHTPARAM_POSITION,
    lightPos);

    csSphere lightSphere (lightPos, light->GetInfluenceRadius ());
    if (rview->TestBSphere (camTransR, lightSphere))
    {
      int i;
      for (i = 0; i < steps.Length(); i++)
      {
        steps[i]->Perform (rview, sector);
      }
    }
  }
}

void csLightIteratorRenderStep::AddStep (iRenderStep* step)
{
  steps.Push (step);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csGenericRenderStep)
SCF_IMPLEMENTS_INTERFACE(iRenderStep)
SCF_IMPLEMENT_IBASE_END

csGenericRenderStep::csGenericRenderStep (csRenderLoop* rl, 
  csStringID shadertype, bool firstpass, csZBufMode zmode)
{
  SCF_CONSTRUCT_IBASE (0);

  csGenericRenderStep::rl = rl;

  csGenericRenderStep::shadertype = shadertype;
  csGenericRenderStep::firstpass = firstpass;
  csGenericRenderStep::zmode = zmode;
}

void csGenericRenderStep::RenderMeshes (iRender3D* r3d,
                                         iShader* shader, 
                                         csRenderMesh** meshes, 
                                         int num)
{
  if (num == 0) return;

  iShaderTechnique *tech = shader->GetBestTechnique ();

  for (int p=0; p<tech->GetPassCount (); p++)
  {
    iShaderPass *pass = tech->GetPass (p);
    pass->Activate (0);

    int j;
    for (j = 0; j < num; j++)
    {
      csRenderMesh* mesh = meshes[j];

      pass->SetupState (mesh);

      uint mixsave = mesh->mixmode;
      uint mixmode = pass->GetMixmodeOverride ();
      if (mixmode != 0)
        mesh->mixmode = mixmode;

      rl->engine->G3D->DrawMesh (mesh);
      mesh->mixmode = mixsave;

      pass->ResetState ();
    }
    pass->Deactivate ();
  }
}

void csGenericRenderStep::Perform (csRenderView* rview, iSector* sector)
{
  iRender3D* r3d = rl->engine->G3D;

  if (firstpass)
    r3d->EnableZOffset ();

  r3d->SetZMode (zmode);

  iSectorRenderMeshList* meshes = sector->GetRenderMeshes ();
  CS_ALLOC_STACK_ARRAY (csRenderMesh*, sameShaderMeshes, meshes->GetCount());
  int numSSM = 0;
  iShader* shader = 0;
  iLightList* lights = sector->GetLights();

  int i = 0;

  while (true)
  {
    iMeshWrapper* mw;
    iVisibilityObject* visobj;
    csRenderMesh* mesh;
    // @@@Objects outside the light's influence radius are 'lit' as well!
    if (!meshes->GetVisible (i, mw, visobj, mesh)) break;
    /*
    @@@!!! That should of course NOT be necessary,
    but otherwise there's some corruption (at least w/ genmesh).
    Seems that it has some side effect we have to find out.
    */
    int n;
    mw->GetRenderMeshes(n);

    if (!mesh) continue;

    mesh->material->Visit(); // @@@ here?
    iShader* meshShader = mesh->material->GetMaterialHandle()->GetShader(shadertype);
    if (meshShader != shader)
    {
      RenderMeshes (r3d, shader, sameShaderMeshes, numSSM);

      shader = meshShader;
      numSSM = 0;
    }
    sameShaderMeshes[numSSM++] = mesh;
  }
  if (numSSM != 0)
  {
    RenderMeshes (r3d, shader, sameShaderMeshes, numSSM);
  }
  if (firstpass)
    r3d->DisableZOffset ();
};

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csRenderLoop)
  SCF_IMPLEMENTS_INTERFACE(iRenderLoop)
SCF_IMPLEMENT_IBASE_END

csRenderLoop::csRenderLoop (csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (0);

  csRenderLoop::engine = engine;

  csRef<iStringSet> strings = 
    CS_QUERY_REGISTRY_TAG_INTERFACE (engine->object_reg, 
    "crystalspace.renderer.stringset", iStringSet);

  csRef<iRenderStep> tmp;
  tmp.AttachNew (new csGenericRenderStep (this, 
    strings->Request("ambient"), true, CS_ZBUF_USE));
  steps.Push (tmp);
  csRef<csLightIteratorRenderStep> itstep = 
    new csLightIteratorRenderStep (this);
  steps.Push (itstep);
  tmp.AttachNew (new csGenericRenderStep (this,
    strings->Request("diffuse"), false, CS_ZBUF_TEST));
  itstep->AddStep (tmp);
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

#endif
