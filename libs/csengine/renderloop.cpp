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
#include "csengine/engine.h"
#include "csengine/renderloop.h"

#ifdef CS_NR_ALTERNATE_RENDERLOOP

void csEngine::StartDraw (iCamera *c, iClipper2D *view, csRenderView &rview)
{
  current_camera = c;
  rview.SetEngine (this);
  rview.SetOriginalCamera (c);

/*  iEngineSequenceManager* eseqmgr = GetEngineSequenceManager ();
  if (eseqmgr)
  {
    eseqmgr->SetCamera (c);
  }*/

  // This flag is set in HandleEvent on a cscmdContextResize event
  if (resize)
  {
    resize = false;
    Resize ();
  }

  top_clipper = view;

  rview.GetClipPlane ().Set (0, 0, 1, -1);      //@@@CHECK!!!

  // Calculate frustum for screen dimensions (at z=1).
  float leftx = -c->GetShiftX () * c->GetInvFOV ();
  float rightx = (frame_width - c->GetShiftX ()) * c->GetInvFOV ();
  float topy = -c->GetShiftY () * c->GetInvFOV ();
  float boty = (frame_height - c->GetShiftY ()) * c->GetInvFOV ();
  rview.SetFrustum (leftx, rightx, topy, boty);

  cur_process_polygons = 0;
}

void csEngine::Draw (iCamera *c, iClipper2D *view)
{
  ControlMeshes ();

  csRenderView rview (c, view, G3D, G2D);
  StartDraw (c, view, rview);

  // First initialize G3D with the right clipper.
  G3D->SetClipper (view, CS_CLIPPER_TOPLEVEL);  // We are at top-level.
  G3D->ResetNearPlane ();
  G3D->SetPerspectiveAspect (c->GetFOV ());

  iSector *s = c->GetSector ();
  //if (s) s->Draw (&rview);
  if (s)
  {
    rview.SetThisSector (s);

    csRef<iShaderManager> shmgr = 
      CS_QUERY_REGISTRY (csEngine::object_reg, iShaderManager);
    csRenderMeshList meshes;
    s->CollectMeshes (&rview, meshes);

    if (meshes.num) 
    {
      int i;

      G3D->EnableZOffset ();
      for (i = 0; i < meshes.num; i++)
      {
	iMaterialHandle *matsave;
	matsave = meshes.meshes[i]->mathandle;
	meshes.meshes[i]->mathandle = NULL;
	uint mixsave = meshes.meshes[i]->mixmode;
	meshes.meshes[i]->mixmode = CS_FX_COPY;
	G3D->DrawMesh (meshes.meshes[i]);
	meshes.meshes[i]->mathandle = matsave;
	meshes.meshes[i]->mixmode = mixsave;
      }
      G3D->DisableZOffset ();

      csHashMap shader_sort;
      for (i = 0; i < meshes.num; i ++) {
	shader_sort.Put ((csHashKey)meshes.meshes[i]->mathandle->GetShader(), (csHashObject)(i + 1));
      }

      csReversibleTransform camTransR = 
	rview.GetCamera()->GetTransform();
      G3D->SetObjectToCamera (&camTransR);

      const csBasicVector &shader_list = shmgr->GetShaders ();
      for (i = 0; i < shader_list.Length(); i ++) {
	if (shader_sort.Get ((csHashKey)shader_list[i]) == NULL) { continue; }
	iShaderTechnique *tech = ((iShader *)shader_list[i])->GetBestTechnique ();

	for (int p=0; p<tech->GetPassCount (); p++)
	{
	  iShaderPass *pass = tech->GetPass (p);
	  pass->Activate (NULL);

	  for (int j = 0; j < meshes.lightnum; j++)
	  {
	    iLight* light = meshes.lights[j];
	    G3D->SetLightParameter (0, CS_LIGHTPARAM_POSITION,
  		  light->GetCenter());
	    G3D->SetLightParameter (0, CS_LIGHTPARAM_ATTENUATION,
  		  light->GetAttenuationVector());
	    csVector3 color (light->GetColor().red, light->GetColor().blue, light->GetColor().green);

	    csHashIterator iter (&shader_sort, (csHashKey)shader_list[i]);
	    while (iter.HasNext ()) {
	      int meshidx = (int)iter.Next() - 1;
	      if (!meshes.lightflags[j][meshidx]) 
	      {
		continue;
	      }
	      csRenderMesh *mesh = meshes.meshes[meshidx];
	      // if (mesh->mathandle->GetShader() != shader_list[i]) { continue; }
	      pass->SetupState (mesh);
    	  
	      float diffuse, specular, ambient;
	      csRGBpixel matcolor;
	      mesh->mathandle->GetReflection (diffuse, specular, ambient);
	      mesh->mathandle->GetFlatColor (matcolor);
	      G3D->SetLightParameter (0, CS_LIGHTPARAM_DIFFUSE, color * diffuse);
	      G3D->SetLightParameter (0, CS_LIGHTPARAM_SPECULAR, color * specular);
	      csZBufMode zsave = mesh->z_buf_mode;
	      uint mixsave = mesh->mixmode;
              
	      uint mixmode = pass->GetMixmodeOverride ();
	      if (mixmode != 0)
		mesh->mixmode = mixmode;
              
	      mesh->z_buf_mode = CS_ZBUF_TEST;
	      G3D->DrawMesh (mesh);
	      mesh->z_buf_mode = zsave;
	      mesh->mixmode = mixsave;

	      pass->ResetState ();
	    }
	  }
	  pass->Deactivate ();
	}
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

  G3D->SetClipper (NULL, CS_CLIPPER_NONE);
}

#endif
