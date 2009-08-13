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
: scfImplementationType(this), engine(engine), shaderLoaded(false)
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
    else if(updateQueue[i]->update)
    {
      UpdateImposter(updateQueue[i]);
      updateQueue[i]->update = false;
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

csImposterManager::TextureSpace::TextureSpace(size_t width, size_t height, TextureSpace* parent)
: parent(parent), full(false)
{
  if(!parent)
  {
    minX = 0;
    minY = 0;
  }
  else if(!parent->firstSpace)
  {
    if(width < height)
    {
      minX = parent->minX;
      minY = parent->minY;
    }
    else
    {
      minX = parent->minX;
      minY = parent->minY + height;
    }
  }
  else
  {
    if(width < height)
    {
      minX = parent->minX + width;
      minY = parent->minY;
    }
    else
    {
      minX = parent->minX;
      minY = parent->minY;
    }
  }

  if(width > 64 || height > 64)
  {
    if(width < height)
    {
      childWidth = width;
      childHeight = height/2;
    }
    else
    {
      childWidth = width/2;
      childHeight = height;
    }

    firstSpace.AttachNew(new TextureSpace(childWidth, childHeight, this));
    secondSpace.AttachNew(new TextureSpace(childWidth, childHeight, this));
  }
}

csImposterManager::TextureSpace* csImposterManager::TextureSpace::Allocate(size_t& width,
                                                                           size_t& height,
                                                                           csBox2& texCoords)
{
  if(childWidth < width || childHeight < height)
  {
    texCoords.Set(minX, minY, minX+width, minY+height);

    full = true;
    return this;
  }
  
  if(!firstSpace->IsFull())
  {
    TextureSpace* space = firstSpace->Allocate(width, height, texCoords);
    if(space)
    {
      return space;
    }
  }

  TextureSpace* space = secondSpace->Allocate(width, height, texCoords);
  if(space->IsFull())
  {
    full = true;
  }

  return space;
}

void csImposterManager::TextureSpace::Free()
{
  full = false;

  if(parent)
    parent->Free();
}

iMaterialWrapper* csImposterManager::AllocateTexture(ImposterMat* imposter,
                                                     csBox2& texCoords)
{
  // Check whether to free existing space.
  if(imposter->allocatedSpace)
  {
     imposter->allocatedSpace->Free();
  }

  // Check for space in existing textures.
  for(size_t i=0; i<textureSpace.GetSize(); ++i)
  {
    if(!textureSpace[i]->IsFull())
    {
      imposter->allocatedSpace = textureSpace[i]->Allocate(imposter->texWidth,
        imposter->texHeight, texCoords);
      return textureSpace[i]->material;
    }
  }

  // Need to recalc in case screen size changes.
  size_t rTexWidth = csFindNearestPowerOf2(g3d->GetWidth());
  size_t rTexHeight = csFindNearestPowerOf2(g3d->GetHeight());

  // Else allocate a new texture.
  csRef<TextureSpace> newSpace;
  newSpace.AttachNew(new TextureSpace(rTexWidth, rTexHeight));
  textureSpace.Push(newSpace);

  // Create texture handle. Size is the current screen size (to nearest pow2)
  // as that's the maximum texture size we should have to handle.
  csRef<iTextureManager> texman = g3d->GetTextureManager();
  csRef<iTextureHandle> texh = texman->CreateTexture(rTexWidth, rTexHeight,
    csimg2D, "rgba8", CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS);
  texh->SetAlphaType (csAlphaMode::alphaBinary);

  // Create the material.
  csRef<iTextureWrapper> tex = engine->GetTextureList()->CreateTexture(texh);
  newSpace->material = engine->CreateMaterial("impostermat", tex);

  // Set shaders.
  if(!shaderLoaded)
  {
    csRef<iLoader> ldr = csQueryRegistry<iLoader>(engine->GetObjectRegistry());
    ldr->LoadShader("/shader/lighting/lighting_imposter.xml");
    shaderLoaded = true;
  }

  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet>(
    engine->GetObjectRegistry(), "crystalspace.shared.stringset");
  csStringID shadertype = strings->Request("base");
  csRef<iShaderManager> shman = csQueryRegistry<iShaderManager>(engine->objectRegistry);
  iShader* shader = shman->GetShader("lighting_imposter");
  newSpace->material->GetMaterial()->SetShader(shadertype, shader);

  // Now allocate part of this texture for use.
  newSpace->Allocate(imposter->texWidth, imposter->texHeight, texCoords);
  return newSpace->material;
}

void csImposterManager::InitialiseImposter(ImposterMat* imposter)
{
  csImposterMesh* csIMesh = static_cast<csImposterMesh*>(&*imposter->mesh);
  csMeshWrapper* csMesh = static_cast<csMeshWrapper*>(&*csIMesh->closestInstanceMesh);

  if(!csIMesh->camera.IsValid())
    return;

  // Set up camera.
  csRef<iCustomMatrixCamera> newCamera = engine->CreateCustomMatrixCamera(csIMesh->camera);

  // Move camera to look at mesh.
  csVector3 mesh_pos = csMesh->GetWorldBoundingBox ().GetCenter ();
  const csVector3& cam_pos = newCamera->GetCamera()->GetTransform ().GetOrigin ();
  csVector3 camdir = mesh_pos-cam_pos;
  newCamera->GetCamera()->GetTransform ().LookAt (camdir, csVector3(0,1,0));

  // Get screen bounding box of the mesh.
  csScreenBoxResult rbox = csMesh->GetScreenBoundingBox(newCamera->GetCamera());
  imposter->texWidth = rbox.sbox.MaxX() - rbox.sbox.MinX();
  imposter->texHeight = rbox.sbox.MaxY() - rbox.sbox.MinY();

  // Allocate texture space.
  csIMesh->mat = AllocateTexture(imposter, csIMesh->texCoords);

  // Normalise the texture coordinates.
  csIMesh->texCoords.Set(csIMesh->texCoords.MinX()/g3d->GetWidth(),
                         csIMesh->texCoords.MinY()/g3d->GetHeight(),
                         csIMesh->texCoords.MaxX()/g3d->GetWidth(),
                         csIMesh->texCoords.MaxY()/g3d->GetHeight());

  // Calculate required projection shift.
  CS::Math::Matrix4 projShift (
      1, 0, 0, 2*csIMesh->texCoords.MinX() + (g3d->GetWidth()-2*rbox.sbox.MinX())/g3d->GetWidth() - 1,
      0, 1, 0, 2*csIMesh->texCoords.MinY() + (g3d->GetHeight()-2*rbox.sbox.MinY())/g3d->GetHeight() - 1,
      0, 0, 1, 0,
      0, 0, 0, 1);

  newCamera->SetProjectionMatrix (projShift * newCamera->GetCamera()->GetProjectionMatrix());

  // Set up view.
  csRef<iView> newView = csPtr<iView>(new csView(engine, g3d));
  newView->SetCustomMatrixCamera(newCamera);
  newView->GetMeshFilter().SetFilterMode(MESH_FILTER_INCLUDE);
  newView->GetMeshFilter().AddFilterMesh(csMesh);

  // Mark original mesh for r2t draw.
  csMesh->drawing_imposter = scfQueryInterface<iBase>(newCamera);

  // Add view and texture as a render target.
  csRef<iRenderManagerTargets> rmTargets = scfQueryInterface<iRenderManagerTargets>(engine->renderManager);
  rmTargets->RegisterRenderTarget(csIMesh->mat->GetMaterial()->GetTexture(), newView,
    0, iRenderManagerTargets::updateOnce);

  csIMesh->matDirty = true;

  // Move imposter mesh to correct sector.
  csIMesh->mesh->GetMovable()->SetPosition(csVector3(0.0f));
  csIMesh->mesh->GetMovable()->SetSector(csIMesh->sector);
  csIMesh->mesh->GetMovable()->UpdateMove();
}

void csImposterManager::UpdateImposter(ImposterMat* imposter)
{
  csImposterMesh* csIMesh = static_cast<csImposterMesh*>(&*imposter->mesh);
  csMeshWrapper* csMesh = static_cast<csMeshWrapper*>(&*csIMesh->closestInstanceMesh);

  if(csIMesh->closestInstance < imposter->lastDistance)
  {
    // Calculate new texture sizes.
    csScreenBoxResult rbox = csMesh->GetScreenBoundingBox(csIMesh->camera);
    size_t texWidth = csFindNearestPowerOf2(rbox.sbox.MaxX() - rbox.sbox.MinX());
    size_t texHeight = csFindNearestPowerOf2(rbox.sbox.MaxY() - rbox.sbox.MinY());

    if(imposter->texHeight < texHeight || imposter->texWidth < texWidth)
    {
      InitialiseImposter(imposter);
    }

    imposter->lastDistance = csIMesh->closestInstance;
  }

  // Finished updating.
  csIMesh->isUpdating = false;
}

void csImposterManager::Register(iImposterMesh* mesh)
{
  ImposterMat* imposterMat = new ImposterMat(mesh);
  imposterMats.Push(imposterMat);
  updateQueue.Push(imposterMat);
}

void csImposterManager::Update(iImposterMesh* mesh)
{
  for(size_t i=0; i<imposterMats.GetSize(); ++i)
  {
    if(imposterMats[i]->mesh == mesh)
    {
      if(imposterMats[i]->init &&
        !imposterMats[i]->update &&
        !imposterMats[i]->remove)
      {
        updateQueue.Push(imposterMats[i]);
        imposterMats[i]->update = true;
        break;
      }
    }
  }
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
