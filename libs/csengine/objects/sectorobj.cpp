/*
    Copyright (C) 2003 by Jorrit Tyberghein

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
#include "qsqrt.h"
#include "csgeom/sphere.h"
#include "igeom/objmodel.h"
#include "csengine/sectorobj.h"
#include "csengine/sector.h"
#include "csengine/meshobj.h"
#include "csengine/meshlod.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "iengine/portal.h"
#include "csutil/debug.h"
#include "iengine/rview.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "ivideo/graph3d.h"


// ---------------------------------------------------------------------------
// csMeshWrapper
// ---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csSectorObject)
  SCF_IMPLEMENTS_INTERFACE (iVisibilityObject)
SCF_IMPLEMENT_IBASE_EXT_END

csSectorObject::csSectorObject (iMeshWrapper* theParent) : csObject()
{
  movable.scfParent = (iBase*)(csObject*)this;
  visnr = 0;
  wor_bbox_movablenr = -1;
  movable.SetSectorObject (this);
  Parent = theParent;
  if (Parent) movable.SetParent (Parent->GetMovable ());

  render_priority = csEngine::current_engine->GetObjectRenderPriority ();
}

csSectorObject::~csSectorObject ()
{
}

float csSectorObject::GetSquaredDistance (iRenderView *rview)
{
  iCamera* camera = rview->GetCamera ();
  // calculate distance from camera to mesh
  csBox3 obox;
  GetObjectModel ()->GetObjectBoundingBox (obox, CS_BBOX_MAX);
  csVector3 obj_center = (obox.Min () + obox.Max ()) / 2;
  csVector3 wor_center;
  if (movable.IsFullTransformIdentity ())
    wor_center = obj_center;
  else
    wor_center = movable.GetFullTransform ().This2Other (obj_center);
  csVector3 cam_origin = camera->GetTransform ().GetOrigin ();
  float wor_sq_dist = csSquaredDist::PointPoint (cam_origin, wor_center);
  return wor_sq_dist;
}

void csSectorObject::GetFullBBox (csBox3& box)
{
  GetObjectModel ()->GetObjectBoundingBox (box, CS_BBOX_MAX);
  iMovable* mov = &movable.scfiMovable;
  while (mov)
  {
    if (!mov->IsTransformIdentity ())
    {
      const csReversibleTransform& trans = mov->GetTransform ();
      csBox3 b (trans.This2Other (box.GetCorner (0)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (1)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (2)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (3)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (4)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (5)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (6)));
      b.AddBoundingVertexSmart (trans.This2Other (box.GetCorner (7)));
      box = b;
    }
    mov = mov->GetParent ();
  }
}


void csSectorObject::PlaceMesh ()
{
  iSectorList *movable_sectors = movable.GetSectors ();
  if (movable_sectors->GetCount () == 0) return ; // Do nothing
  csSphere sphere;
  csVector3 radius;
  GetObjectModel ()->GetRadius (radius, sphere.GetCenter ());

  iSector *sector = movable_sectors->Get (0);
  movable.SetSector (sector);       // Make sure all other sectors are removed

  // Transform the sphere from object to world space.
  float max_radius = radius.x;
  if (max_radius < radius.y) max_radius = radius.y;
  if (max_radius < radius.z) max_radius = radius.z;
  sphere.SetRadius (max_radius);
  if (!movable.IsFullTransformIdentity ())
    sphere = movable.GetFullTransform ().This2Other (sphere);
  max_radius = sphere.GetRadius ();
  float max_sq_radius = max_radius * max_radius;

  csRef<iMeshWrapperIterator> it = csEngine::current_engine
  	->GetNearbyMeshes (sector, sphere.GetCenter (), max_radius, true);

  int j;
  while (it->HasNext ())
  {
    iMeshWrapper* mesh = it->Next ();
    if (mesh->GetMeshObject ()->GetPortalCount () == 0)
      continue;		// No portals to consider.

    csRef<iThingState> thing = SCF_QUERY_INTERFACE (
        mesh->GetMeshObject (), iThingState);
    if (thing)
    {
      // @@@ This function will currently only consider portals on things
      // that cannot move.
      if (thing->GetMovingOption () == CS_THING_MOVE_NEVER)
      {
        iThingFactoryState* thing_fact = thing->GetFactory ();
        for (j = 0; j < thing_fact->GetPortalCount (); j++)
        {
          iPortal *portal = thing_fact->GetPortal (j);
          iSector *dest_sector = portal->GetSector ();
          if (movable_sectors->Find (dest_sector) == -1)
          {
            iPolygon3D *portal_poly = thing->GetPortalPolygon (j);
            const csPlane3 &pl = portal_poly->GetWorldPlane ();

            float sqdist = csSquaredDist::PointPlane (
                sphere.GetCenter (), pl);
            if (sqdist <= max_sq_radius)
            {
              // Plane of portal is close enough.
              // If N is the normal of the portal plane then we
              // can use that to calculate the point on the portal plane.
              csVector3 testpoint = sphere.GetCenter () + pl.Normal () * qsqrt (
                  sqdist);
              if (portal_poly->GetStaticData ()->PointOnPolygon (testpoint))
                movable_sectors->Add (dest_sector);
            }
          }
        }
      }
    }
  }
}

int csSectorObject::HitBeamBBox (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  csBox3 b;
  GetObjectModel ()->GetObjectBoundingBox (b, CS_BBOX_MAX);

  csSegment3 seg (start, end);
  return csIntersect3::BoxSegment (b, seg, isect, pr);
}

void csSectorObject::GetWorldBoundingBox (csBox3 &cbox)
{
  if (wor_bbox_movablenr != movable.GetUpdateNumber ())
  {
    wor_bbox_movablenr = movable.GetUpdateNumber ();

    if (movable.IsFullTransformIdentity ())
      GetObjectModel ()->GetObjectBoundingBox (wor_bbox, CS_BBOX_MAX);
    else
    {
      csBox3 obj_bbox;
      GetObjectModel ()->GetObjectBoundingBox (obj_bbox, CS_BBOX_MAX);

      // @@@ Maybe it would be better to really calculate the bounding box
      // here instead of just transforming the object space bounding box?
      csReversibleTransform mt = movable.GetFullTransform ();
      wor_bbox.StartBoundingBox (mt.This2Other (obj_bbox.GetCorner (0)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (1)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (2)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (3)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (4)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (5)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (6)));
      wor_bbox.AddBoundingVertexSmart (mt.This2Other (obj_bbox.GetCorner (7)));
    }
  }

  cbox = wor_bbox;
}

void csSectorObject::GetTransformedBoundingBox (
  const csReversibleTransform &trans,
  csBox3 &cbox)
{
  csBox3 box;
  GetObjectModel ()->GetObjectBoundingBox (box);
  cbox.StartBoundingBox (trans * box.GetCorner (0));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (1));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (2));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (3));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (4));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (5));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (6));
  cbox.AddBoundingVertexSmart (trans * box.GetCorner (7));
}

float csSectorObject::GetScreenBoundingBox (
  const iCamera *camera,
  csBox2 &sbox,
  csBox3 &cbox)
{
  csVector2 oneCorner;
  csReversibleTransform tr_o2c = camera->GetTransform ();
  if (!movable.IsFullTransformIdentity ())
    tr_o2c /= movable.GetFullTransform ();
  GetTransformedBoundingBox (tr_o2c, cbox);

  // if the entire bounding box is behind the camera, we're done
  if ((cbox.MinZ () < 0) && (cbox.MaxZ () < 0))
  {
    return -1;
  }

  // Transform from camera to screen space.
  if (cbox.MinZ () <= 0)
  {
    // Mesh is very close to camera.
    // Just return a maximum bounding box.
    sbox.Set (-10000, -10000, 10000, 10000);
  }
  else
  {
    camera->Perspective (cbox.Max (), oneCorner);
    sbox.StartBoundingBox (oneCorner);

    csVector3 v (cbox.MinX (), cbox.MinY (), cbox.MaxZ ());
    camera->Perspective (v, oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
    camera->Perspective (cbox.Min (), oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
    v.Set (cbox.MaxX (), cbox.MaxY (), cbox.MinZ ());
    camera->Perspective (v, oneCorner);
    sbox.AddBoundingVertexSmart (oneCorner);
  }

  return cbox.MaxZ ();
}

