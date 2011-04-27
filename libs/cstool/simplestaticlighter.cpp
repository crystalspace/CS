/*
    Copyright (C) 2008 by Jorrit Tyberghein

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
#include "csutil/cscolor.h"
#include "csutil/cscolor.h"
#include "csgeom/math3d.h"
#include "cstool/simplestaticlighter.h"
#include "csgfx/renderbuffer.h"
#include "imesh/genmesh.h"
#include "iengine/mesh.h"
#include "iengine/light.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "imesh/object.h"

namespace CS
{
namespace Lighting
{
 
void SimpleStaticLighter::ConstantColor (iMeshWrapper* mesh, const csColor4& color)
{
  iMeshFactoryWrapper* meshfact = mesh->GetFactory ();
  if (!meshfact) return;
  csRef<iGeneralFactoryState> fact_state = scfQueryInterface<
    iGeneralFactoryState> (meshfact->GetMeshObjectFactory ());
  if (!fact_state) return;	// Not a mesh we recognize.
  size_t count = fact_state->GetVertexCount ();
  csRef<iRenderBuffer> rbuf = csRenderBuffer::CreateRenderBuffer (
      count, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
  CS_ALLOC_STACK_ARRAY (csColor4, colors, count);
  size_t i;
  for (i = 0 ; i < count ; i++)
    colors[i] = color;
  rbuf->CopyInto (colors, count);
  csRef<iGeneralMeshState> state = scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ());
  state->AddRenderBuffer ("static color", rbuf);
}

void SimpleStaticLighter::CalculateLighting (iMeshWrapper* mesh,
    iGeneralFactoryState* fact_state, iLight* light,
    ShadowType shadow_type, csColor4* colors, bool init)
{
  size_t count = fact_state->GetVertexCount ();
  size_t i;

  csVector3 center = light->GetMovable ()->GetFullTransform ().GetOrigin ();
  iSector* light_sector = light->GetMovable ()->GetSectors ()->Get (0);
  csReversibleTransform mesh_trans = mesh->GetMovable ()->GetFullTransform ();

  if (shadow_type == CS_SHADOW_CENTER)
  {
    csSectorHitBeamResult rc = light_sector->HitBeamPortals (center,
	mesh_trans.GetOrigin ());
    if (rc.mesh != 0 && rc.mesh != mesh)
    {
      // Shadow.
      if (init)
        for (i = 0 ; i < count ; i++)
          colors[i] = csColor4 (0, 0, 0, 0);
      return;
    }
  }
  else if (shadow_type == CS_SHADOW_BOUNDINGBOX)
  {
    const csBox3& world_box = mesh->GetWorldBoundingBox ();
    bool shadowed = true;
    for (int j = 0 ; shadowed && j < 8 ; j++)
    {
      csSectorHitBeamResult rc = light_sector->HitBeamPortals (center,
        world_box.GetCorner (j));
      if (rc.mesh == 0 || rc.mesh == mesh) shadowed = false;
    }
    if (shadowed)
    {
      // Shadow.
      if (init)
        for (i = 0 ; i < count ; i++)
          colors[i] = csColor4 (0, 0, 0, 0);
      return;
    }
  }


  // Shaders multiply by 2. So we need to divide by 2 here.
  csColor color = 0.5f * light->GetColor ();
  float sqcutoff = light->GetCutoffDistance ();
  sqcutoff *= sqcutoff;

  csVector3* verts = fact_state->GetVertices ();
  csVector3* normals = fact_state->GetNormals ();

  if (shadow_type != CS_SHADOW_FULL)
  {
    // Transform light to object space here since we don't have to do
    // full accurate shadows anyway.
    center = mesh_trans.Other2This (center);
    for (i = 0 ; i < count ; i++)
    {
      csVector3 relpos = center-verts[i];
      float dist = relpos * relpos;
      bool dark = init;
      if (dist < sqcutoff)
      {
        dist = sqrt (dist);
        float bright = light->GetBrightnessAtDistance (dist);
        bright *= (normals[i] * relpos) / relpos.Norm ();
        if (bright > SMALL_EPSILON)
        {
	  if (init)
            colors[i] = color * bright;
	  else
	    colors[i] += color * bright;
          colors[i].Clamp (1.0, 1.0, 1.0);
	  dark = false;
        }
      }
      if (dark) colors[i].Set (0, 0, 0, 0);
    }
  }
  else
  {
    // With full shadows we need world space coordinates to be
    // able to check the shadow beams.
    bool mesh_trans_identity = mesh_trans.IsIdentity ();
    for (i = 0 ; i < count ; i++)
    {
      csVector3 vworld;
      if (mesh_trans_identity)
	vworld = verts[i];
      else
	vworld = mesh_trans.This2Other (verts[i]);
      csVector3 relpos = center-vworld;
      float dist = relpos * relpos;
      bool dark = init;
      if (dist < sqcutoff)
      {
        dist = sqrt (dist);
        float bright = light->GetBrightnessAtDistance (dist);
        bright *= (normals[i] * relpos) / relpos.Norm ();
        if (bright > SMALL_EPSILON)
        {
	  csSectorHitBeamResult rc = light_sector->HitBeamPortals (center, vworld);
	  if (rc.mesh == 0 || rc.mesh == mesh)
	  {
	    if (init)
              colors[i] = color * bright;
	    else
              colors[i] += color * bright;
            colors[i].Clamp (1.0, 1.0, 1.0);
	    dark = false;
	  }
	}
      }
      if (dark) colors[i].Set (0, 0, 0, 0);
    }
  }
}

void SimpleStaticLighter::ShineLight (iMeshWrapper* mesh, iLight* light,
    ShadowType shadow_type)
{
  iMeshFactoryWrapper* meshfact = mesh->GetFactory ();
  if (!meshfact) return;
  csRef<iGeneralFactoryState> fact_state = scfQueryInterface<
    iGeneralFactoryState> (meshfact->GetMeshObjectFactory ());
  if (!fact_state) return;	// Not a mesh we recognize.
  size_t count = fact_state->GetVertexCount ();
  csRef<iRenderBuffer> rbuf = csRenderBuffer::CreateRenderBuffer (
      count, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
  CS_ALLOC_STACK_ARRAY (csColor4, colors, count);

  CalculateLighting (mesh, fact_state, light, shadow_type, colors, true);

  rbuf->CopyInto (colors, count);
  csRef<iGeneralMeshState> state = scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ());
  state->AddRenderBuffer ("static color", rbuf);
  
  mesh->GetFlags().Set (CS_ENTITY_STATICLIT);
}

void SimpleStaticLighter::ShineLights (iMeshWrapper* mesh, iEngine* engine, int maxlights,
    ShadowType shadow_type)
{
  iMovable* movable = mesh->GetMovable ();
  if (!movable->InSector ()) return;	// No movable, do nothing.

  const csBox3& world_box = mesh->GetWorldBoundingBox ();
  CS_ALLOC_STACK_ARRAY (iLight*, lights, maxlights);
  size_t num = engine->GetNearbyLights (movable->GetSectors ()->Get (0),
	world_box, lights, maxlights);

  if (num == 0)
  {
    ConstantColor (mesh, csColor4 (0, 0, 0, 0));
    return;
  }
  if (num == 1)
  {
    ShineLight (mesh, lights[0], shadow_type);
    return;
  }

  iMeshFactoryWrapper* meshfact = mesh->GetFactory ();
  if (!meshfact) return;
  csRef<iGeneralFactoryState> fact_state = scfQueryInterface<
    iGeneralFactoryState> (meshfact->GetMeshObjectFactory ());
  if (!fact_state) return;	// Not a mesh we recognize.
  size_t count = fact_state->GetVertexCount ();
  csRef<iRenderBuffer> rbuf = csRenderBuffer::CreateRenderBuffer (
      count, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
  CS_ALLOC_STACK_ARRAY (csColor4, colors, count);

  size_t l;
  for (l = 0 ; l < num ; l++)
  {
    iLight* light = lights[l];
    CalculateLighting (mesh, fact_state, light, shadow_type, colors, l == 0);
  }

  rbuf->CopyInto (colors, count);
  csRef<iGeneralMeshState> state = scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ());
  state->AddRenderBuffer ("static color", rbuf);
  
  mesh->GetFlags().Set (CS_ENTITY_STATICLIT);
}

void SimpleStaticLighter::ShineLights (iSector* sector, iEngine* engine, int maxlights,
      ShadowType shadow_type)
{
  iMeshList* meshes = sector->GetMeshes ();
  int i;
  for (i = 0 ; i < meshes->GetCount () ; i++)
  {
    iMeshWrapper* mesh = meshes->Get (i);
    ShineLights (mesh, engine, maxlights, shadow_type);
  }
}

} // namespace Lighting
} // namespace CS

//---------------------------------------------------------------------------
