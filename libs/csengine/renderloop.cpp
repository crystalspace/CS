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

#include "cssysdef.h"

#include "iutil/objreg.h"

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

SCF_IMPLEMENT_IBASE(csAmbientRenderStep)
  SCF_IMPLEMENTS_INTERFACE(iRenderStep)
SCF_IMPLEMENT_IBASE_END

csAmbientRenderStep::csAmbientRenderStep (csRenderLoop* rl)
{
  SCF_CONSTRUCT_IBASE (NULL);

  csAmbientRenderStep::rl = rl;
}

void csAmbientRenderStep::Perform (csRenderView* rview, 
				   csRenderMeshList* meshes)
{
  rl->engine->G3D->EnableZOffset ();
  for (int i = 0; i < meshes->num; i++)
  {
    iMaterialHandle *matsave;
    matsave = meshes->meshes[i]->mathandle;
    meshes->meshes[i]->mathandle = NULL;
    uint mixsave = meshes->meshes[i]->mixmode;
    meshes->meshes[i]->mixmode = CS_FX_COPY;
    rl->engine->G3D->DrawMesh (meshes->meshes[i]);
    meshes->meshes[i]->mathandle = matsave;
    meshes->meshes[i]->mixmode = mixsave;
  }
  rl->engine->G3D->DisableZOffset ();
};

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csLightingRenderStep)
  SCF_IMPLEMENTS_INTERFACE(iRenderStep)
SCF_IMPLEMENT_IBASE_END

csLightingRenderStep::csLightingRenderStep (csRenderLoop* rl)
{
  SCF_CONSTRUCT_IBASE (NULL);

  csLightingRenderStep::rl = rl;
}

void csLightingRenderStep::Perform (csRenderView* rview, 
				    csRenderMeshList* meshes)
{
  csHashMap shader_sort;
  int i;
  for (i = 0; i < meshes->num; i ++) 
  {
    shader_sort.Put ((csHashKey)meshes->meshes[i]->mathandle->GetShader(), (csHashObject)(i + 1));
  }

  csReversibleTransform camTransR = 
    rview->GetCamera()->GetTransform();
  rl->engine->G3D->SetObjectToCamera (&camTransR);

  csRef<iShaderManager> shmgr = 
    CS_QUERY_REGISTRY (rl->engine->object_reg, iShaderManager);  
    
  const csBasicVector &shader_list = shmgr->GetShaders ();
  for (i = 0; i < shader_list.Length(); i ++) 
  {
    if (shader_sort.Get ((csHashKey)shader_list[i]) == NULL) 
    { 
      continue; 
    }
    iShaderTechnique *tech = ((iShader *)shader_list[i])->GetBestTechnique ();

    for (int p=0; p<tech->GetPassCount (); p++)
    {
      iShaderPass *pass = tech->GetPass (p);
      pass->Activate (NULL);

      for (int j = 0; j < meshes->lightnum; j++)
      {
	iLight* light = meshes->lights[j];
	rl->engine->G3D->SetLightParameter (0, CS_LIGHTPARAM_POSITION,
  	      light->GetCenter());
	rl->engine->G3D->SetLightParameter (0, CS_LIGHTPARAM_ATTENUATION,
  	      light->GetAttenuationVector());
	csVector3 color (light->GetColor().red, light->GetColor().blue, light->GetColor().green);

	csHashIterator iter (&shader_sort, (csHashKey)shader_list[i]);
	while (iter.HasNext ()) 
	{
	  int meshidx = (int)iter.Next() - 1;
	  if (!meshes->lightflags[j][meshidx]) 
	  {
	    continue;
	  }
	  csRenderMesh *mesh = meshes->meshes[meshidx];
	  /*
 	  if (mesh->mathandle->GetShader() != shader_list[i]) 
	  { 
	    continue; 
	  } */
      
	  float diffuse, specular, ambient;
	  csRGBpixel matcolor;
	  mesh->mathandle->GetReflection (diffuse, specular, ambient);
	  mesh->mathandle->GetFlatColor (matcolor);
	  rl->engine->G3D->SetLightParameter (0, CS_LIGHTPARAM_DIFFUSE, color * diffuse);
	  rl->engine->G3D->SetLightParameter (0, CS_LIGHTPARAM_SPECULAR, color * specular);

	  pass->SetupState (mesh);
	  csZBufMode zsave = mesh->z_buf_mode;
	  uint mixsave = mesh->mixmode;
          
	  uint mixmode = pass->GetMixmodeOverride ();
	  if (mixmode != 0)
	    mesh->mixmode = mixmode;
          
	  mesh->z_buf_mode = CS_ZBUF_TEST;
	  rl->engine->G3D->DrawMesh (mesh);
	  mesh->z_buf_mode = zsave;
	  mesh->mixmode = mixsave;

	  pass->ResetState ();
	}
      }
      pass->Deactivate ();
    }
  }
};

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csRenderLoop)
  SCF_IMPLEMENTS_INTERFACE(iRenderLoop)
SCF_IMPLEMENT_IBASE_END

csRenderLoop::csRenderLoop (csEngine* engine)
{
  SCF_CONSTRUCT_IBASE (NULL);

  csRenderLoop::engine = engine;

  csRef<iRenderStep> tmp;
  tmp.AttachNew (new csAmbientRenderStep (this));
  steps.Push (tmp);
  tmp.AttachNew (new csLightingRenderStep (this));
  steps.Push (tmp);
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
    rview.SetThisSector (s);

    csRenderMeshList meshes;
    s->CollectMeshes (&rview, meshes);

    if (meshes.num) 
    {
      int i;
      for (i = 0; i < steps.Length(); i++)
      {
	steps[i]->Perform (&rview, &meshes);
      }
    }
  }

  // draw all halos on the screen
/*  if (halos.Length () > 0)
  {
    csTicks elapsed = virtual_clock->GetElapsedTicks ();
    for (int halo = halos.Length () - 1; halo >= 0; halo--)
      if (!halos[halo]->Process (elapsed, *this)) halos.Delete (halo);
  }*/

  engine->G3D->SetClipper (NULL, CS_CLIPPER_NONE);
}

#endif
