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
#include "csutil/flags.h"
#include "csutil/set.h"

#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/portal.h"
#include "iengine/portalcontainer.h"
#include "iengine/sector.h"

float csEngineTools::FindShortestDistance (
  	const csVector3& source, iSector* sourceSector,
  	const csVector3& dest, iSector* destSector,
  	float maxradius,
	csArray<iSector*>& visited_sectors)
{
  if (sourceSector == destSector)
  {
    float sqdist = csSquaredDist::PointPoint (source, dest);
    if (sqdist <= maxradius * maxradius)
      return sqdist;
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
      const csPlane3& wor_plane = portal->GetWorldPlane ();
      // Can we see the portal?
      if (wor_plane.Classify (source) < -0.001)
      {
        const csVector3* world_vertices = portal->GetWorldVertices ();
        int* indices = portal->GetVertexIndices ();
        // Take the average of vertex 0 and 2 under the assumption that
        // this will usually be a point near enough to the center of the portal.
        const csVector3& v1 = world_vertices[indices[0]];
        const csVector3& v2 = world_vertices[indices[2]];
        csVector3 portal_vert = (v1 + v2) / 2.0f;
        float sqdist_portal = csSquaredDist::PointPoint (source, portal_vert);
        if (sqdist_portal <= maxradius * maxradius)
        {
          // Ok, we go on. The portal is in our sphere of interest.
	  portal->CompleteSector (0);
	  iSector* portal_sector = portal->GetSector ();
	  if (portal_sector)
	  {
	    size_t l;
	    bool already_visited = false;
            for (l = 0 ; l < visited_sectors.Length () ; l++)
            {
              if (visited_sectors[l] == portal_sector)
              {
                already_visited = true;
                break;
              }
            }
            if (!already_visited)
            {
              visited_sectors.Push (portal_sector);

	      float sqdist;
              // Now we have to consider space warping for the portal.
              if (portal->GetFlags ().Check (CS_PORTAL_WARP))
              {
                iMovable* movable = portal_mesh->GetMovable ();
                csReversibleTransform trans = movable->GetFullTransform ();

                csReversibleTransform warp_wor;
                portal->ObjectToWorld (trans, warp_wor);
                csVector3 new_source = portal->Warp (warp_wor, source);
	        sqdist = FindShortestDistance (new_source, portal_sector,
	  	    dest, destSector, maxradius, visited_sectors);
              }
	      else
	      {
	        sqdist = FindShortestDistance (source, portal_sector,
	  	    dest, destSector, maxradius, visited_sectors);
	      }
	      if (sqdist >= 0 && sqdist < best_sqdist)
	      {
	        best_found = true;
	        best_sqdist = sqdist;
              }
	      visited_sectors.Pop ();
	    }
	  }
        }
      }
    }
  }
  return best_found ? best_sqdist : -1.0f;
}

float csEngineTools::FindShortestDistance (
  	const csVector3& source, iSector* sourceSector,
  	const csVector3& dest, iSector* destSector,
  	float maxradius)
{
  csArray<iSector*> visited_sectors;
  return FindShortestDistance (source, sourceSector, dest,
  	destSector, maxradius, visited_sectors);
}

//----------------------------------------------------------------------

