/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#include "csgeom/math3d.h"
#include "csgeom/transfrm.h"
#include "cstool/enginetools.h"
#include "cstool/collider.h"
#include "csutil/flags.h"
#include "csutil/set.h"

#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "iengine/sector.h"
#include "iengine/camera.h"

static bool TestPortalSphere (iPortal* portal, float radius,
	const csVector3& pos, csSet<csPtrKey<iSector> >& visited_sectors)
{
  const csPlane3& wor_plane = portal->GetWorldPlane ();
  // Can we see the portal?
  if (wor_plane.Classify (pos) < -0.001)
  {
    const csVector3* world_vertices = portal->GetWorldVertices ();
    csVector3 poly[100];	//@@@ HARDCODE
    int k;
    int* idx = portal->GetVertexIndices ();
    for (k = 0 ; k < portal->GetVertexIndicesCount () ; k++)
    {
      poly[k] = world_vertices[idx[k]];
    }
    float sqdist_portal = csSquaredDist::PointPoly (
                  pos, poly, portal->GetVertexIndicesCount (),
                  wor_plane);
    if (sqdist_portal <= radius * radius)
    {
      portal->CompleteSector (0);
      iSector* portal_sector = portal->GetSector ();
      if (portal_sector && !visited_sectors.In (portal_sector))
      {
        visited_sectors.Add (portal_sector);
	return true;
      }
    }
  }
  return false;
}

static bool TestCenterPortalSphere (iPortal* portal, float radius,
	const csVector3& pos, csSet<csPtrKey<iSector> >& visited_sectors)
{
  const csPlane3& wor_plane = portal->GetWorldPlane ();
  // Can we see the portal?
  if (wor_plane.Classify (pos) < -0.001)
  {
    const csVector3* world_vertices = portal->GetWorldVertices ();
    int* indices = portal->GetVertexIndices ();
    // Take the average of vertex 0 and 2 under the assumption that
    // this will usually be a point near enough to the center of the portal.
    const csVector3& v1 = world_vertices[indices[0]];
    const csVector3& v2 = world_vertices[indices[2]];
    csVector3 portal_vert = (v1 + v2) / 2.0f;
    float sqdist_portal = csSquaredDist::PointPoint (pos, portal_vert);
    if (sqdist_portal <= radius * radius)
    {
      portal->CompleteSector (0);
      iSector* portal_sector = portal->GetSector ();
      if (portal_sector && !visited_sectors.In (portal_sector))
      {
        visited_sectors.Add (portal_sector);
	return true;
      }
    }
  }
  return false;
}

float csEngineTools::FindShortestDistance (
  	const csVector3& source, iSector* sourceSector,
  	const csVector3& dest, iSector* destSector,
  	float maxradius,
	csSet<csPtrKey<iSector> >& visited_sectors,
	csVector3& direction,
	bool accurate)
{
  if (sourceSector == destSector)
  {
    float sqdist = csSquaredDist::PointPoint (source, dest);
    if (sqdist <= maxradius * maxradius)
    {
      direction = dest-source;
      return sqdist;
    }
    else
      return -1.0f;
  }

  const csSet<csPtrKey<iMeshWrapper> >& portal_meshes = 
    sourceSector->GetPortalMeshes ();
  csSet<csPtrKey<iMeshWrapper> >::GlobalIterator it = 
    portal_meshes.GetIterator ();
  float best_sqdist = 100000000000.0f;
  bool best_found = false;
  while (it.HasNext ())
  {
    iMeshWrapper* portal_mesh = it.Next ();
    iPortalContainer* portal_container = portal_mesh->GetPortalContainer ();
    int i;
    for (i = 0 ; i < portal_container->GetPortalCount () ; i++)
    {
      iPortal* portal = portal_container->GetPortal (i);
      bool rc;
      if (accurate)
        rc = TestPortalSphere (portal, maxradius, source, visited_sectors);
      else
        rc = TestCenterPortalSphere (portal, maxradius, source,
		visited_sectors);
      if (rc)
      {
        csVector3 new_source = source;
        // Now we have to consider space warping for the portal.
	bool do_warp = portal->GetFlags ().Check (CS_PORTAL_WARP);
	if (do_warp)
        {
          iMovable* movable = portal_mesh->GetMovable ();
          csReversibleTransform trans = movable->GetFullTransform ();
          csReversibleTransform warp_wor;
          portal->ObjectToWorld (trans, warp_wor);
          new_source = portal->Warp (warp_wor, source);
        }
	csVector3 local_direction;
        iSector* portal_sector = portal->GetSector ();
	float sqdist = FindShortestDistance (new_source, portal_sector,
	  	    dest, destSector, maxradius, visited_sectors,
		    local_direction, accurate);
	if (sqdist >= 0 && sqdist < best_sqdist)
	{
	  best_found = true;
	  best_sqdist = sqdist;
	  if (do_warp)
	    direction = portal->GetWarp ().Other2ThisRelative (local_direction);
	  else
	    direction = local_direction;
        }
	visited_sectors.Delete (portal_sector);
      }
    }
  }
  return best_found ? best_sqdist : -1.0f;
}

csShortestDistanceResult csEngineTools::FindShortestDistance (
  	const csVector3& source, iSector* sourceSector,
  	const csVector3& dest, iSector* destSector,
  	float maxradius, bool accurate)
{
  csSet<csPtrKey<iSector> > visited_sectors;
  csShortestDistanceResult rc;
  rc.direction.Set (0, 0, 0);
  rc.sqdistance = FindShortestDistance (source, sourceSector, dest,
  	destSector, maxradius, visited_sectors, rc.direction, accurate);
  return rc;
}

#include "csutil/deprecated_warn_off.h"
csScreenTargetResult csEngineTools::FindScreenTarget (const csVector2& pos,
      float maxdist, iCamera* camera, iCollideSystem* cdsys)
{
  csVector2 p (pos.x, camera->GetShiftY () * 2 - pos.y);
  csVector3 v = camera->InvPerspective (p, 1.0f);
  csVector3 end = camera->GetTransform ().This2Other (v);
  iSector* sector = camera->GetSector ();
  CS_ASSERT (sector != 0);
  csVector3 origin = camera->GetTransform ().GetO2TTranslation ();

  // Now move the end until it is at the right distance.
  csVector3 rel = (end-origin).Unit ();
  end = origin + rel * maxdist;
  // Slightly move the origin for safety.
  origin = origin + rel * 0.03f;

  csScreenTargetResult result;
  if (cdsys == 0)
  {
    csSectorHitBeamResult hr = sector->HitBeamPortals (origin, end);
    result.mesh = hr.mesh;
    if (hr.mesh == 0)
    {
      result.isect = end;
      result.polygon_idx = -1;
    }
    else
    {
      result.isect = hr.isect;
      result.polygon_idx = hr.polygon_idx;
    }
  }
  else
  {
    csTraceBeamResult tr = csColliderHelper::TraceBeam (cdsys,
	sector, origin, end, true);
    result.mesh = tr.closest_mesh;
    if (tr.closest_mesh == 0) result.isect = end;
    else result.isect = tr.closest_isect;
    result.polygon_idx = -1;
  }
  return result;
}
#include "csutil/deprecated_warn_on.h"

//----------------------------------------------------------------------

