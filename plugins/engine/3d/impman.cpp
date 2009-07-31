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
#include "csgeom/projections.h"
#include "csgfx/imagememory.h"
#include "cstool/csview.h"
#include "cstool/meshfilter.h"
#include "iengine/camera.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"

#include "engine.h"
#include "impman.h"
#include "impmesh.h"
#include "meshobj.h"
#include "camera.h"

using namespace CS::Math;
using namespace CS::Utility;

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

  // Allocate a texture image.
  csRef<iTextureManager> texman = g3d->GetTextureManager();
  csRef<iTextureHandle> texh = texman->CreateTexture(csIMesh->texWidth, csIMesh->texHeight,
    csimg2D, "rgba8", CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS);
  texh->SetAlphaType (csAlphaMode::alphaBinary);

  // Set up camera.
  csRef<iCustomMatrixCamera> newCamera = engine->CreateCustomMatrixCamera(csIMesh->camera);

  // Move camera to look at mesh.
  csVector3 mesh_pos = csMesh->GetWorldBoundingBox ().GetCenter ();
  const csVector3& cam_pos = newCamera->GetCamera()->GetTransform ().GetOrigin ();
  csVector3 camdir = mesh_pos-cam_pos;
  newCamera->GetCamera()->GetTransform ().LookAt (camdir, csVector3(0,1,0));

  // Set up a new projection matrix (code from heavy portal setup).
  csScreenBoxResult rbox = csMesh->GetScreenBoundingBox(newCamera->GetCamera());
  float irw = 1.0f/csIMesh->texWidth;
  float irh = 1.0f/csIMesh->texHeight;
  int screenW = g3d->GetDriver2D()->GetWidth();
  int screenH = g3d->GetDriver2D()->GetHeight();

  CS::Math::Matrix4 projShift (
      screenW*irw, 0, 0, irw * (screenW-2*rbox.sbox.MinX()) - 1,
      0, screenH*irh, 0, irh * (screenH-2*rbox.sbox.MinY()) - 1,
      0, 0, 1, 0,
      0, 0, 0, 1);

  newCamera->SetProjectionMatrix (projShift * newCamera->GetCamera()->GetProjectionMatrix());

  // Set up view.
  csRef<iView> newView = csPtr<iView>(new csView(engine, g3d));
  newView->SetCamera(newCamera->GetCamera());
  newView->GetMeshFilter().SetFilterMode(MESH_FILTER_INCLUDE);
  newView->GetMeshFilter().AddFilterMesh(csMesh);

  // Mark original mesh for r2t draw.
  csMesh->drawing_imposter = scfQueryInterface<iBase>(newCamera);

  // Add view and texture as a render target.
  csRef<iRenderManagerTargets> rmTargets = scfQueryInterface<iRenderManagerTargets>(engine->renderManager);
  rmTargets->RegisterRenderTarget(texh, newView, 0, iRenderManagerTargets::updateOnce);

  csRef<iTextureWrapper> tex = engine->GetTextureList()->CreateTexture(texh);
  csIMesh->mat = engine->CreateMaterial("impostermat", tex);

  // Move imposter mesh to correct sector.
  csIMesh->mesh->GetMovable()->SetPosition(csVector3(0.0f));
  csIMesh->mesh->GetMovable()->SetSector(csIMesh->sector);
  csIMesh->mesh->GetMovable()->UpdateMove();
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
