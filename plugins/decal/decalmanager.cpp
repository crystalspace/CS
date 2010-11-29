/*
    Copyright (C) 2006 by Andrew Robberts

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
#include "csqsqrt.h"
#include "decal.h"
#include "decaltemplate.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "csgfx/renderbuffer.h"
#include "iengine/movable.h"
#include "imesh/genmesh.h"
#include "iengine/material.h"
#include "imesh/object.h"
#include "cstool/collider.h"
#include "ivideo/material.h"
#include "csgfx/shadervarcontext.h"
#include "iutil/eventq.h"
#include "csutil/event.h"
#include "csutil/eventhandlers.h"
#include "decalmanager.h"


SCF_IMPLEMENT_FACTORY (csDecalManager)

csDecalManager::csDecalManager (iBase * parent)
  : scfImplementationType (this, parent), objectReg (0)
{
}

csDecalManager::~csDecalManager ()
{
  size_t a = decals.GetSize ();
  while (a)
  {
    --a;
    delete decals[a];
  }

  if (objectReg)
  {
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (objectReg));
    if (q)
      CS::RemoveWeakListener (q, weakEventHandler);
  }
}

bool csDecalManager::Initialize (iObjectRegistry * objectReg)
{
  this->objectReg = objectReg;
  vc = csQueryRegistry<iVirtualClock> (objectReg);

  CS_INITIALIZE_EVENT_SHORTCUTS (objectReg);
  csRef<iEventQueue> q(csQueryRegistry<iEventQueue> (objectReg));
  if (q)
    CS::RegisterWeakListener (q, this, Frame, weakEventHandler);
  return true;
}

iDecal * csDecalManager::CreateDecal (iDecalTemplate * decalTemplate, 
  iSector * sector, const csVector3 & pos, const csVector3 & up, 
  const csVector3 & normal, float width, float height, iDecal * oldDecal)
{
  CS_ASSERT(decalTemplate && sector);

  // compute the maximum distance the decal can reach
  float radius = csQsqrt (width * width + height * height) * 0.5f;

  if (!EnsureEngineReference ())
    return 0;

  // get all meshes that could be affected by this decal
  csRef<iMeshWrapperIterator> meshIter = engine->GetNearbyMeshes (sector, pos, 
    radius, true);
  if (!meshIter->HasNext ())
      return 0;

  // calculate a valid orientation for the decal
  csVector3 n = normal.Unit ();
  csVector3 u = up.Unit ();
  csVector3 right = n % u;
  csVector3 correctUp = right % n;

  // find a reference to the previous decal or create a new one
  csDecal * decal = 0;
  if (oldDecal)
  {
    const size_t len = decals.GetSize ();
    for (size_t a = 0; a < len; ++a)
    {
      if (decals[a] != oldDecal)
	    continue;

      decal = decals[a];
      decals.DeleteIndexFast (a);
      break;
    }
  }
  if (!decal)
    decal = new csDecal (objectReg, this);

  // initialize the decal
  decal->Initialize (decalTemplate, n, pos, correctUp, right, width, height);
  decals.Push (decal);

  // fill the decal with the geometry of the meshes
  while (meshIter->HasNext ())
  {
    iMeshWrapper* mesh = meshIter->Next ();
    if (mesh->GetFlags ().Check (CS_ENTITY_NODECAL))
      continue;

    csVector3 relPos = 
      mesh->GetMovable ()->GetFullTransform ().Other2This (pos);

    decal->BeginMesh (mesh);
    mesh->GetMeshObject ()->BuildDecal (&relPos, radius, (iDecalBuilder*)decal);
    decal->EndMesh ();
  }

  return decal;
}

iDecal * csDecalManager::CreateDecal (iDecalTemplate * decalTemplate, 
  iMeshWrapper * mesh, const csVector3 & pos, const csVector3 & up, 
  const csVector3 & normal, float width, float height, iDecal * oldDecal)
{
  CS_ASSERT(decalTemplate && mesh);

  if (mesh->GetFlags ().Check (CS_ENTITY_NODECAL))
    return 0;

  // compute the maximum distance the decal can reach
  float radius = csQsqrt (width * width + height * height) * 0.5f;

  // calculate a valid orientation for the decal
  csVector3 n = normal.Unit ();
  csVector3 u = up.Unit ();
  csVector3 right = n % u;
  csVector3 correctUp = right % n;

  // find a reference to the previous decal or create a new one
  csDecal * decal = 0;
  if (oldDecal)
  {
    const size_t len = decals.GetSize ();
    for (size_t a = 0; a < len; ++a)
    {
      if (decals[a] != oldDecal)
	    continue;

      decal = decals[a];
      decals.DeleteIndexFast (a);
      break;
    }
  }
  if (!decal)
    decal = new csDecal (objectReg, this);

  // initialize the decal
  decal->Initialize (decalTemplate, n, pos, correctUp, right, width, height);
  decals.Push (decal);

  // fill the decal with the geometry of the mesh
  csVector3 relPos = 
    mesh->GetMovable ()->GetFullTransform ().Other2This (pos);

  decal->BeginMesh (mesh);
  mesh->GetMeshObject ()->BuildDecal (&relPos, radius, (iDecalBuilder*)decal);
  decal->EndMesh ();

  return decal;
}

csRef<iDecalTemplate> csDecalManager::CreateDecalTemplate (
  iMaterialWrapper* pMaterial)
{
  csRef<iDecalTemplate> ret;

  if (!EnsureEngineReference ())
    return (iDecalTemplate*)nullptr;

  ret.AttachNew (new csDecalTemplate);
  ret->SetMaterialWrapper (pMaterial);
  ret->SetRenderPriority (engine->GetAlphaRenderPriority ());
  return ret;
}

void csDecalManager::DeleteDecal (const iDecal * decal)
{
  // we must ensure that this decal is actually active
  const size_t len = decals.GetSize ();
  for (size_t a = 0; a < len; ++a)
  {
    if (decals[a] != decal)
      continue;

    delete decals[a];
    decals.DeleteIndexFast (a);
    return;
  }
}

size_t csDecalManager::GetDecalCount () const
{
  return decals.GetSize ();
}

iDecal * csDecalManager::GetDecal (size_t idx) const
{
  return decals[idx];
}

bool csDecalManager::HandleEvent (iEvent & ev)
{
  if (ev.Name != Frame)
    return false;

  csTicks elapsed = vc->GetElapsedTicks ();
  size_t a=0;

  while (a < decals.GetSize ())
  {
    if (!decals[a]->Age (elapsed))
    {
      delete decals[a];
      decals.DeleteIndexFast (a);
    }
    else
      ++a;
  }
  return true;
}

bool csDecalManager::EnsureEngineReference ()
{
  if (!engine)
  {
    engine = csQueryRegistry<iEngine> (objectReg);
    if (!engine)
    {
      csReport (objectReg, CS_REPORTER_SEVERITY_ERROR, 
        "crystalspace.decal", "Couldn't query engine");
      return false;
    }
  }
  return true;
}

void csDecalManager::RemoveDecalFromList (csDecal * decal)
{
  size_t idx = decals.Find (decal);
  if (idx == csArrayItemNotFound)
    return;

  decals.DeleteIndexFast (idx);
}
