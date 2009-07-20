/*
  Copyright (C) 2009 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#include "csgeom/polyclip.h"
#include "csgfx/imagememory.h"
#include "cstool/procmesh.h"
#include "iengine/camera.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"

#include "engine.h"
#include "impman.h"
#include "impmesh.h"
#include "meshobj.h"

csImposterManager::csImposterManager(csEngine* engine)
: scfImplementationType(this), engine(engine)
{
  g3d = csQueryRegistry<iGraphics3D>(engine->GetObjectRegistry());
  maxWidth = g3d->GetCaps()->maxTexWidth;
  maxHeight = g3d->GetCaps()->maxTexHeight;

  // Register our event handler
  csRef<EventHandler> event_handler = csPtr<EventHandler> (new EventHandler (this));
  csEventID esub[] = 
  {
    csevFrame (engine->GetObjectRegistry()),
    CS_EVENTLIST_END
  };

  csRef<iEventQueue> queue = csQueryRegistry<iEventQueue> (engine->GetObjectRegistry());
  queue->RegisterListener(event_handler, esub);
}

csImposterManager::~csImposterManager()
{
  for(size_t i=0; i<imposterMats.GetSize(); ++i)
  {
    delete imposterMats[i];
  }
}

bool csImposterManager::HandleEvent(iEvent &ev)
{
  for(size_t i=0; i<updateQueue.GetSize(); ++i)
  {
    if(!updateQueue[i]->init)
    {
      InitialiseImposter(updateQueue[i]);
      updateQueue[i]->init = true;
    }

    if(updateQueue[i]->remove)
    {
      csImposterMesh* cmesh = static_cast<csImposterMesh*>(&*(updateQueue[i]->mesh));
      cmesh->mesh->GetMovable()->SetSector(0);
      cmesh->mesh->GetMovable()->UpdateMove();

      imposterMats.Delete(updateQueue[i]);
      delete updateQueue[i];
    }
  }

  updateQueue.Empty();

  return false;
}

void csImposterManager::InitialiseImposter(ImposterMat* imposter)
{
  csImposterMesh* csIMesh = static_cast<csImposterMesh*>(&*imposter->mesh);
  csMeshWrapper* csMesh = static_cast<csMeshWrapper*>(&*csIMesh->instances[0]->mesh);

  if(!csIMesh->camera.IsValid())
    return;

  // Move imposter mesh to correct sector.
  csIMesh->mesh->GetMovable()->SetPosition(csVector3(0.0f));
  csIMesh->mesh->GetMovable()->SetSector(csIMesh->sector);
  csIMesh->mesh->GetMovable()->UpdateMove();

  // Allocate a texture image.
  int texFlags = CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;

  csRef<iImage> thisImage = new csImageMemory (csIMesh->texWidth, csIMesh->texHeight,
    CS_IMGFMT_ALPHA && CS_IMGFMT_TRUECOLOR );
  iTextureWrapper* tex = engine->GetTextureList ()->NewTexture (thisImage);
  tex->SetFlags (tex->GetFlags() | texFlags);
  tex->Register (engine->G3D->GetTextureManager());
  tex->GetTextureHandle ()->SetAlphaType (csAlphaMode::alphaBinary);
/*
  csMesh->drawing_imposter = true;
  csMeshOnTexture r2t(engine->GetObjectRegistry());
  r2t.GetView()->GetCamera()->SetTransform(csIMesh->camera->GetTransform());
  r2t.GetView()->GetCamera()->SetSector(csIMesh->sector);
  r2t.ScaleCamera(csMesh, 8.0f);
  r2t.Render(csMesh, tex->GetTextureHandle());
  csMesh->drawing_imposter = false;

  csView view(engine, g3d);
  view.SetCamera(csIMesh->camera);

  csMesh->drawing_imposter = true;
  csRef<iRenderManagerTargets> rmTargets = scfQueryInterface<iRenderManagerTargets>(engine->renderManager);
  rmTargets->RegisterRenderTarget(tex->GetTextureHandle(), &view, 0, iRenderManagerTargets::updateOnce);
  g3d->BeginDraw (CSDRAW_3DGRAPHICS | CSDRAW_CLEARZBUFFER);
  engine->renderManager->RenderView(&view);
  g3d->FinishDraw();
  csMesh->drawing_imposter = false;
*/
  csRef<iImageIO> imageio = csQueryRegistry<iImageIO>(engine->GetObjectRegistry());
  csRef<iDataBuffer> db = imageio->Save(thisImage, "image/png");

  engine->VFS->WriteFile("/this/testimage.png", db->GetData(), db->GetSize());

  thisImage.Invalidate();

  /*
  csBoxClipper* clip = new csBoxClipper (0, 0, csMesh->texWidth, csMesh->texHeight);

  //start r2t
  csRef<iTextureHandle> handle = tex->GetTextureHandle ();
  g3d->SetRenderTarget (handle);

  g3d->BeginDraw (CSDRAW_3DGRAPHICS | engine->GetBeginDrawFlags ()
    | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN);
 
  //get imposted mesh
  csRef<iMeshWrapper> originalmesh = csMesh->instances[0]->mesh;
  csRef<iMeshObject> meshobj = originalmesh->GetMeshObject ();
  csVector3 mesh_pos = originalmesh->GetWorldBoundingBox ().GetCenter ();

  //save camerastate for later
  iCamera* cam = rview->GetCamera ();
  csOrthoTransform old_cam_transform = cam->GetTransform ();

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
    csMesh->width/engine->frameWidth,
    csMesh->height/engine->frameHeight);
  
  csVector3 new_cam_pos = mesh_pos - maxratio * camdir;
  cam->GetTransform ().SetOrigin (new_cam_pos);

//printf("maxr: %f\n", maxratio);
//printf("camdir: %f %f %f\n", camdir.x, camdir.y, camdir.z);
//printf("newpos: %f %f %f\n", new_cam_pos.x, new_cam_pos.y, new_cam_pos.z);

  //Setup rendering
  g3d->SetPerspectiveCenter (csMesh->texWidth/2, csMesh->texWidth/2);
  g3d->SetClipper (clip, CS_CLIPPER_TOPLEVEL);
  cam->SetFOV (csMesh->texWidth, csMesh->texHeight);
  g3d->SetPerspectiveAspect (cam->GetFOV ());
  g3d->SetWorldToCamera (cam->GetTransform ().GetInverse ());

  //get the original rendermeshes
  int num;
  csRenderMesh** rendermeshes = meshobj->GetRenderMeshes (num, rview, 
    originalmesh->GetMovable (), ~0);

  csShaderVariableStack& sva = engine->shaderManager->GetShaderVariableStack ();
  sva.Setup (svStringSet->GetSize ());

  for (int i = 0; i < num; i++)
  {
    csRenderMesh* rendermesh = rendermeshes[i];
    csRenderMeshModes mode (*rendermesh);
    sva.Clear ();

    iMaterial* hdl = rendermesh->material->GetMaterial ();
    iShaderVariableContext *svc = new csShaderVariableContext();

    //add ambient shadervariable
    csRef<csShaderVariable> sv;
    sv = svc->GetVariableAdd(stringid_light_ambient);
    csColor ambient;
    engine->GetAmbientLight (ambient);
    if (csMesh->sector) sv->SetValue (ambient + csMesh->sector->GetDynamicAmbientLight());
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

  // Create material.
  csMesh->mat = engine->CreateMaterial("Imposter", tex);*/
}

void csImposterManager::Register(iImposterMesh* mesh)
{
  ImposterMat* imposterMat = new ImposterMat(mesh);
  imposterMats.Push(imposterMat);
  updateQueue.Push(imposterMat);
}

void csImposterManager::Unregister(iImposterMesh* mesh)
{
  for(size_t i=0; i<imposterMats.GetSize(); ++i)
  {
    if(imposterMats[i]->mesh == mesh)
    {
      updateQueue.Push(imposterMats[i]);
      imposterMats[i]->remove = true;
      break;
    }
  }
}
