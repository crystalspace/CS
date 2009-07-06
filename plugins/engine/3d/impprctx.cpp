/*
    Copyright (C) 2002 by Keith Fulton and Jorrit Tyberghein
    Rewritten during Sommer of Code 2006 by Christoph "Fossi" Mewes

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
#include "csgfx/imagememory.h"
#include "cstool/csview.h"
#include "csgeom/polyclip.h"
#include "csgeom/math.h"
#include "csutil/cscolor.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "ivideo/material.h"
#include "engine.h"
#include "material.h"
#include "impmesh.h"
#include "meshobj.h"
#include "impprctx.h"

//@@@ debugging
#include "cstool/debugimagewriter.h"
#include "csgfx/imagememory.h"

csStringID csImposterProcTex::stringid_standard = csInvalidStringID;
CS::ShaderVarStringID csImposterProcTex::stringid_light_ambient = CS::InvalidShaderVarStringID;

csImposterProcTex::csImposterProcTex  (csEngine* engine,  
  csImposterMesh *parent) : scfImplementationType(this)
{
  mesh = parent;

  //@@@ make dynamic
  w = h = 256;

  int texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;

  csRef<iImage> thisImage = new csImageMemory (w, h,
    CS_IMGFMT_ALPHA && CS_IMGFMT_TRUECOLOR );
  tex = engine->GetTextureList ()->NewTexture (thisImage);
  tex->SetFlags (tex->GetFlags() | texFlags);
  tex->Register (engine->G3D->GetTextureManager());
  tex->GetTextureHandle ()->SetAlphaType (csAlphaMode::alphaBinary);
  thisImage = 0;

  svStringSet = csQueryRegistryTagInterface<iShaderVarStringSet> (
    engine->objectRegistry, "crystalspace.shader.variablenameset");
  if (stringid_standard == csInvalidStringID)
  {
    csRef<iStringSet> stringSet = csQueryRegistryTagInterface<iStringSet> (
      engine->objectRegistry, "crystalspace.stringset.shared");
    stringid_standard = stringSet->Request("standard");
    stringid_light_ambient = svStringSet->Request("light ambient");
  }

  clip = new csBoxClipper (0, 0, w, h);

  updating = false;
}

csImposterProcTex::~csImposterProcTex ()
{
  delete clip;
}

void csImposterProcTex::Update (iRenderView* rview)
{
  // Check if we're not already updating.
  // @@@ Note! In the future we might want to support imposters
  // different for various views. i.e. the same object visible at
  // different distances through different portals or views.
  // In that case it might make sense to also add the imposter to
  // other rview update queues in the engine.
  if (!updating && rview)
  {
    csEngine* engine = static_cast<csEngine*> (rview->GetEngine ());
    engine->AddImposterToUpdateQueue (this, rview);
    //engine->imposterUpdateList.Push (csWeakRef<csImposterProcTex> (this));
    updating = true;
  }
}


void csImposterProcTex::RenderToTexture (iRenderView *rview, iSector *s)
{/*
  if (!mesh) return;

  csEngine* engine = static_cast<csEngine*> (rview->GetEngine ());
  iGraphics3D* g3d = engine->G3D;
  iShaderManager* shadermanager = engine->shaderManager;

  //start r2t
  csRef<iTextureHandle> handle = tex->GetTextureHandle ();
  g3d->SetRenderTarget (handle);

  g3d->BeginDraw (CSDRAW_3DGRAPHICS | engine->GetBeginDrawFlags ()
    | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN);
 
  //get imposted mesh
  csRef<iMeshWrapper> originalmesh = mesh->instances[0]->mesh;
  csRef<iMeshObject> meshobj = originalmesh->GetMeshObject ();
  csVector3 mesh_pos = originalmesh->GetWorldBoundingBox ().GetCenter ();

  //update imposter billbord
  iCamera* cam = rview->GetCamera ();
  csOrthoTransform old_cam_transform = cam->GetTransform ();
  mesh->FindImposterRectangle (cam);

  //save camerastate for later
  int persx, persy;
  g3d->GetPerspectiveCenter ( persx, persy );

  //Calculate camera position for imposter rendering
  const csVector3& cam_pos = cam->GetTransform ().GetOrigin ();
  csVector3 camdir = mesh_pos-cam_pos;

  csOrthoTransform transform = cam->GetTransform();;

  csVector3 col3 = transform.GetT2O ().Col3 ();
//printf("transform col3: %f %f %f\n", col3.x, col3.y, col3.z);

  //look at the mesh
  cam->GetTransform ().LookAt (camdir, cam->GetTransform ().GetT2O ().Col2 ());

  col3 = transform.GetT2O ().Col3 ();
//printf("transform col3: %f %f %f\n", col3.x, col3.y, col3.z);


  //the distance to the mesh has the same ratio as
  //the billbordsize to the screen.
  //@@@ this is only roughly correct
  float maxratio = csMax (
    mesh->width/engine->frameWidth,
    mesh->height/engine->frameHeight);
  
  csVector3 new_cam_pos = mesh_pos - maxratio * camdir;
  cam->GetTransform ().SetOrigin (new_cam_pos);

//printf("maxr: %f\n", maxratio);
//printf("camdir: %f %f %f\n", camdir.x, camdir.y, camdir.z);
//printf("newpos: %f %f %f\n", new_cam_pos.x, new_cam_pos.y, new_cam_pos.z);

  //Setup rendering
  g3d->SetPerspectiveCenter (w/2, w/2);
  g3d->SetClipper (clip, CS_CLIPPER_TOPLEVEL);
  cam->SetFOV (w, h);
  g3d->SetPerspectiveAspect (cam->GetFOV ());
  g3d->SetWorldToCamera (cam->GetTransform ().GetInverse ());

  //get the original rendermeshes
  int num;
  csRenderMesh** rendermeshes = meshobj->GetRenderMeshes (num, rview, 
    originalmesh->GetMovable (), ~0);

  csShaderVariableStack& sva = shadermanager->GetShaderVariableStack ();
  sva.Setup (svStringSet->GetSize ());

  //draw them, as the view, engine and renderloops do
  for (int i = 0; i < num; i++)
  {
    csRenderMesh* rendermesh = rendermeshes[i];
    csRenderMeshModes mode (*rendermesh);
    sva.Clear ();

    iMaterial* hdl = rendermesh->material->GetMaterial ();
    iShader* meshShader = hdl->GetShader (stringid_standard);

    if (meshShader == 0)
    {
      static bool defaultshaderused = 0;
      if (!defaultshaderused)
      {
        defaultshaderused = 1;
        engine->Warn("No 'standard' Shader! Using default.");
      }
      meshShader = engine->defaultShader;
    }

    iShaderVariableContext *svc = new csShaderVariableContext();

    //add ambient shadervariable
    csRef<csShaderVariable> sv;
    sv = svc->GetVariableAdd(stringid_light_ambient);
    csColor ambient;
    engine->GetAmbientLight (ambient);
    if (s) sv->SetValue (ambient + s->GetDynamicAmbientLight());
    svc->PushVariables (sva);

    if (rendermesh->variablecontext)
      rendermesh->variablecontext->PushVariables (sva);
    meshShader->PushVariables (sva);
    if (hdl) hdl->PushVariables (sva);

    size_t shaderTicket = meshShader->GetTicket (mode, sva);
    size_t passCount = meshShader->GetNumberOfPasses (shaderTicket);

    for (size_t p = 0; p < passCount; p++)
    {
      meshShader->ActivatePass (shaderTicket, p);
      meshShader->SetupPass (shaderTicket, rendermesh, mode, sva);
      g3d->DrawMesh(rendermesh, mode, sva);
      meshShader->TeardownPass (shaderTicket);
      meshShader->DeactivatePass (shaderTicket);
    }
  }

  //restore old camera values
  g3d->SetPerspectiveCenter (persx, persy);
  g3d->SetClipper (0, CS_CLIPPER_NONE);
  g3d->FinishDraw ();
  cam->SetTransform (old_cam_transform);

  updating = false;*/
}

