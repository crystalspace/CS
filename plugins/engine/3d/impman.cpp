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
}

bool csImposterManager::HandleEvent(iEvent &ev)
{
  for(size_t i=0; i<updateQueue.GetSize(); ++i)
  {
    if(updateQueue[i]->remove)
    {
      csImposterMesh* cmesh = static_cast<csImposterMesh*>(&*(updateQueue[i]->mesh));
      if(cmesh->mesh)
      {
        cmesh->mesh->GetMovable()->SetSector(0);
        cmesh->mesh->GetMovable()->UpdateMove();
      }

      RemoveMeshFromImposter(updateQueue[i]->mesh);
      imposterMats.Delete(updateQueue[i]);
      updateQueue.DeleteIndex(i--);
    }
    else if(!updateQueue[i]->init)
    {
       updateQueue[i]->init = InitialiseImposter(updateQueue[i]);
      if(updateQueue[i]->init)
      {
        updateQueue.DeleteIndexFast(i);
        break;
      }
    }
    else if(updateQueue[i]->update)
    {
      updateQueue[i]->update = false;
      if(UpdateImposter(updateQueue[i]))
      {
        updateQueue.DeleteIndexFast(i);
        break;
      }

      updateQueue.DeleteIndex(i--);
    }
  }

  return false;
}

csImposterManager::TextureSpace::TextureSpace(size_t width,
                                              size_t height,
                                              iMaterialWrapper* material,
                                              TextureSpace* parent)
: width(width), height(height), childWidth(0), childHeight(0),
  material(material), parent(parent), full(false)
{
  if(!parent)
  {
    minX = 0;
    minY = 0;
    rTexWidth = width;
    rTexHeight = height;
  }
  else
  {
    rTexWidth = parent->rTexWidth;
    rTexHeight = parent->rTexHeight;

    if(!parent->firstSpace)
    {
      minX = parent->minX;
      minY = parent->minY;
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
        minY = parent->minY + height;
      }
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

    firstSpace.AttachNew(new TextureSpace(childWidth, childHeight, material, this));
    secondSpace.AttachNew(new TextureSpace(childWidth, childHeight, material, this));
  }
}

csImposterManager::TextureSpace* csImposterManager::TextureSpace::Allocate(size_t& rWidth,
                                                                           size_t& rHeight,
                                                                           csBox2& texCoords)
{
  // Check if it'll fit in a child.
  if(childWidth < rWidth || childHeight < rHeight)
  {
    // It can't. Check if we have room in this one.
    if(rWidth <= width && rHeight <= height &&
      (!firstSpace || !firstSpace->IsUsed()) &&
      (!secondSpace || !secondSpace->IsUsed()))
    {
      texCoords.Set(minX, minY, minX+rWidth, minY+rHeight);

      full = true;
      return this;
    }

    return 0;
  }
  
  TextureSpace* space = 0;
  if(!firstSpace->IsFull())
  {
    space = firstSpace->Allocate(rWidth, rHeight, texCoords);
  }

  if(!space && !secondSpace->IsFull())
  {
    space = secondSpace->Allocate(rWidth, rHeight, texCoords);
  }

  if(firstSpace->IsFull() && secondSpace->IsFull())
  {
    full = true;
  }

  return space;
}

bool csImposterManager::TextureSpace::Realloc(size_t& rWidth,
                                              size_t& rHeight,
                                              csBox2& texCoords) const
{
  if(full && (childWidth < rWidth || childHeight < rHeight) &&
     rWidth <= width && rHeight <= height)
  {
    texCoords.Set(minX, minY, minX+rWidth, minY+rHeight);
    return true;
  }

  return false;
}

void csImposterManager::TextureSpace::Free()
{
  full = false;

  if(parent && parent->full)
    parent->Free();
}

void csImposterManager::TextureSpace::GetRenderTextureDimensions(size_t& rTWidth,
                                                                 size_t& rTHeight) const
{
  rTWidth = rTexWidth;
  rTHeight = rTexHeight;
}

bool csImposterManager::TextureSpace::IsUsed() const
{
  return full || (firstSpace && firstSpace->IsUsed()) || (secondSpace && secondSpace->IsUsed());
}

iMaterialWrapper* csImposterManager::AllocateTexture(ImposterMat* imposter,
                                                     csBox2& texCoords,
                                                     size_t& rTexWidth,
                                                     size_t& rTexHeight)
{
  // Check whether if we can reuse existing space.
  if(imposter->allocatedSpace)
  {
    if(imposter->allocatedSpace->Realloc(imposter->texWidth,
      imposter->texHeight, texCoords))
    {
      imposter->allocatedSpace->GetRenderTextureDimensions(rTexWidth, rTexHeight);
      return imposter->allocatedSpace->GetMaterial();
    }

    // We can't. So free it and we'll need to allocate new.
    imposter->allocatedSpace->Free();
    imposter->allocatedSpace = 0;
  }

  // Check for space in existing textures.
  for(size_t i=0; i<textureSpace.GetSize(); ++i)
  {
    if(!textureSpace[i]->IsFull())
    {
      imposter->allocatedSpace = textureSpace[i]->Allocate(imposter->texWidth,
        imposter->texHeight, texCoords);
      if(imposter->allocatedSpace)
      {
        imposter->allocatedSpace->GetRenderTextureDimensions(rTexWidth, rTexHeight);
        return textureSpace[i]->GetMaterial();
      }
    }
  }

  // Else create a new texture.
  rTexWidth = csFindNearestPowerOf2((int)imposter->texWidth > g3d->GetWidth() ?
    (int)imposter->texWidth : g3d->GetWidth());
  rTexHeight = csFindNearestPowerOf2((int)imposter->texHeight > g3d->GetHeight() ?
    (int)imposter->texHeight : g3d->GetHeight());

  // Create texture handle. Size is the current screen size (to nearest pow2)
  // as that's the maximum texture size we should have to handle.
  csRef<iTextureManager> texman = g3d->GetTextureManager();
  csRef<iTextureHandle> texh = texman->CreateTexture(rTexWidth, rTexHeight,
    csimg2D, "rgba8", CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS);
  texh->SetAlphaType (csAlphaMode::alphaBinary);

  // Create the material.
  csRef<iTextureWrapper> tex = engine->GetTextureList()->CreateTexture(texh);
  csRef<iMaterialWrapper> material = engine->CreateMaterial("impostermat", tex);

  // Set shaders.
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet>(
    engine->GetObjectRegistry(), "crystalspace.shared.stringset");
  csStringID shadertype = strings->Request("base");
  csRef<iShaderManager> shman = csQueryRegistry<iShaderManager>(engine->objectRegistry);
  iShader* shader = shman->GetShader(imposter->shader);
  material->GetMaterial()->SetShader(shadertype, shader);

  // Create new texture space.
  csRef<TextureSpace> newSpace;
  newSpace.AttachNew(new TextureSpace(rTexWidth, rTexHeight, material));
  textureSpace.Push(newSpace);

  // Now allocate part of this texture for use.
  imposter->allocatedSpace = newSpace->Allocate(imposter->texWidth, imposter->texHeight, texCoords);
  return material;
}

bool csImposterManager::InitialiseImposter(ImposterMat* imposter)
{
  csImposterMesh* csIMesh = static_cast<csImposterMesh*>(&*imposter->mesh);
  csMeshWrapper* csMesh = static_cast<csMeshWrapper*>(&*csIMesh->closestInstanceMesh);

  if(!csIMesh->camera.IsValid())
    return false;

  // Set material shader.
  imposter->shader = csIMesh->shader;

  // Set up camera.
  csRef<iCustomMatrixCamera> newCamera = engine->CreateCustomMatrixCamera(csIMesh->camera);

  // Move camera to look at mesh.
  csVector3 mesh_pos = csMesh->GetWorldBoundingBox ().GetCenter ();
  const csVector3& cam_pos = newCamera->GetCamera()->GetTransform ().GetOrigin ();
  csVector3 camdir = mesh_pos-cam_pos;
  newCamera->GetCamera()->GetTransform ().LookAt (camdir,
    newCamera->GetCamera()->GetTransform().GetT2O().Col2());

  // Get screen bounding box of the mesh.
  csScreenBoxResult rbox = csMesh->GetScreenBoundingBox(newCamera->GetCamera());
  imposter->texWidth = rbox.sbox.MaxX() - rbox.sbox.MinX();
  imposter->texHeight = rbox.sbox.MaxY() - rbox.sbox.MinY();

  // Allocate texture space.
  size_t rTexWidth, rTexHeight;
  csIMesh->mat = AllocateTexture(imposter, csIMesh->texCoords, rTexWidth, rTexHeight);

  // Set up view.
  csRef<iView> newView = csPtr<iView>(new csView(engine, g3d));
  newView->SetCustomMatrixCamera(newCamera);
  newView->GetMeshFilter().SetFilterMode(MESH_FILTER_INCLUDE);
  newView->GetMeshFilter().AddFilterMesh(csMesh);
  newView->SetRectangle(csIMesh->texCoords.MinX(), csIMesh->texCoords.MinY(),
    csIMesh->texCoords.MaxX()-csIMesh->texCoords.MinX(),
    csIMesh->texCoords.MaxY()-csIMesh->texCoords.MinY());
  newView->UpdateClipper ();

  // Normalise the texture coordinates.
  csIMesh->texCoords.Set(csIMesh->texCoords.MinX()/rTexWidth,
                         csIMesh->texCoords.MinY()/rTexHeight,
                         csIMesh->texCoords.MaxX()/rTexWidth,
                         csIMesh->texCoords.MaxY()/rTexHeight);

  // Calculate required projection shift.
  CS::Math::Matrix4 projShift (
      1, 0, 0, 2*csIMesh->texCoords.MinX() + (rTexWidth-2*rbox.sbox.MinX())/rTexWidth - 1,
      0, 1, 0, 2*csIMesh->texCoords.MinY() + (rTexHeight-2*rbox.sbox.MinY())/rTexHeight - 1,
      0, 0, 1, 0,
      0, 0, 0, 1);

  newCamera->SetProjectionMatrix (projShift * newCamera->GetCamera()->GetProjectionMatrix());

  // Mark original mesh for r2t draw.
  csMesh->drawing_imposter = scfQueryInterface<iBase>(newCamera);

  // Add view and texture as a render target.
  csRef<iRenderManagerTargets> rmTargets = scfQueryInterface<iRenderManagerTargets>(engine->renderManager);
  rmTargets->RegisterRenderTarget(csIMesh->mat->GetMaterial()->GetTexture(), newView,
    0, iRenderManagerTargets::updateOnce | iRenderManagerTargets::clearScreen);

  csIMesh->matDirty = true;

  // Check for instancing.
  if(csIMesh->instance)
  {
    // Move imposter mesh to correct sector.
    csIMesh->mesh->GetMovable()->SetPosition(csVector3(0.0f));
    csIMesh->mesh->GetMovable()->SetSector(csIMesh->sector);
    csIMesh->mesh->GetMovable()->UpdateMove();
  }
  else if(!imposter->init)
  {
    // Add imposter mesh to our sector imposter.
    AddMeshToImposter(imposter->mesh);
  }

  return true;
}

bool csImposterManager::UpdateImposter(ImposterMat* imposter)
{
  csImposterMesh* csIMesh = static_cast<csImposterMesh*>(&*imposter->mesh);
  csMeshWrapper* csMesh = static_cast<csMeshWrapper*>(&*csIMesh->closestInstanceMesh);

  if(!csIMesh->camera.IsValid())
  {
    // Finished updating.
    csIMesh->isUpdating = false;
    return false;
  }

  bool updated = false;
  if(csIMesh->materialUpdateNeeded || csIMesh->closestInstance < imposter->lastDistance)
  {
    // Calculate new texture sizes.
    csScreenBoxResult rbox = csMesh->GetScreenBoundingBox(csIMesh->camera);
    size_t texWidth = csFindNearestPowerOf2(rbox.sbox.MaxX() - rbox.sbox.MinX());
    size_t texHeight = csFindNearestPowerOf2(rbox.sbox.MaxY() - rbox.sbox.MinY());

    if(imposter->texHeight < texHeight || imposter->texWidth < texWidth)
    {
      RemoveMeshFromImposter(imposter->mesh);
      InitialiseImposter(imposter);
      AddMeshToImposter(imposter->mesh);
      updated = true;
    }

    csIMesh->materialUpdateNeeded = false;
    imposter->lastDistance = csIMesh->closestInstance;
  }

  // Finished updating.
  csIMesh->isUpdating = false;

  return updated;
}

void csImposterManager::AddMeshToImposter(csImposterMesh* imposter)
{
  for(size_t i=0; i<sectorImposters.GetSize(); ++i)
  {
    if(imposter->sector == sectorImposters[i]->sector &&
       imposter->mat == sectorImposters[i]->sectorImposter->mat)
    {
      sectorImposters[i]->sectorImposter->meshDirty = true;
      sectorImposters[i]->sectorImposter->imposterMeshes.Push(imposter);
      return;
    }
  }

  csRef<SectorImposter> newSectorI;
  newSectorI.AttachNew(new SectorImposter());

  newSectorI->sector = imposter->sector;
  newSectorI->sectorImposter.AttachNew(new csImposterMesh(engine, newSectorI->sector));
  newSectorI->sectorImposter->imposterMeshes.Push(imposter);
  newSectorI->sectorImposter->mat = imposter->mat;

  sectorImposters.Push(newSectorI);
}

void csImposterManager::RemoveMeshFromImposter(csImposterMesh* imposter)
{
  for(size_t i=0; i<sectorImposters.GetSize(); ++i)
  {
    if(imposter->sector == sectorImposters[i]->sector &&
       imposter->mat == sectorImposters[i]->sectorImposter->mat)
    {
      sectorImposters[i]->sectorImposter->meshDirty = true;
      sectorImposters[i]->sectorImposter->imposterMeshes.Delete(imposter);
      return;
    }
  }
}

void csImposterManager::Register(iImposterMesh* mesh)
{
  csRef<ImposterMat> imposterMat;
  imposterMat.AttachNew(new ImposterMat(mesh));
  imposterMats.Push(imposterMat);
  updateQueue.Push(imposterMat);
}

bool csImposterManager::Update(iImposterMesh* mesh)
{
  for(size_t i=0; i<imposterMats.GetSize(); ++i)
  {
    if(&*(imposterMats[i]->mesh) == mesh)
    {
      if(imposterMats[i]->init &&
        !imposterMats[i]->update &&
        !imposterMats[i]->remove)
      {
        updateQueue.Push(imposterMats[i]);
        imposterMats[i]->update = true;
        return true;
      }

      break;
    }
  }

  return false;
}

void csImposterManager::Unregister(iImposterMesh* mesh)
{
  for(size_t i=0; i<imposterMats.GetSize(); ++i)
  {
    if(&*(imposterMats[i]->mesh) == mesh)
    {
      updateQueue.Push(imposterMats[i]);
      imposterMats[i]->remove = true;
      return;
    }
  }
}
