/*
  Copyright (C) 2005 by Marten Svanfeldt

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
#include "geometryextract.h"
#include "lightmesh.h"
#include "scene.h"

#include "csgeom/poly3d.h"
#include "csgeom/transfrm.h"
#include "csgeom/tri.h"

#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "imesh/thing.h"

bool litGeometryExtractor::ExtractGeometry (litScene *scene,
                                           iMeshObjectFactory *csfact, 
                                           litLightingMeshFactory *lightmeshfact)
{
  return true;
}

bool litGenmeshGeometryExtractor::ExtractGeometry (litScene *scene,
                                                  iMeshObjectFactory *csfact, 
                                                  litLightingMeshFactory *lightmeshfact)
{
  csRef<iGeneralFactoryState> factState = 
    SCF_QUERY_INTERFACE (csfact, iGeneralFactoryState);
  if (!factState) return false;

  csVector3 *overts = factState->GetVertices ();
  size_t numVerts = factState->GetVertexCount ();
  csTriangle *tris = factState->GetTriangles ();
  size_t numTri = factState->GetTriangleCount ();

  // Extract vertices
  uint i;
  for (i = 0; i < numVerts; i++)
  {
    lightmeshfact->vertices.Push (overts[i]);
  }
  
  // Extract and create faces
  for (i = 0; i < numTri; i++)
  {
    litLightingMeshFactory::litMeshFactoryFace factFace;
    factFace.material = scene->materials.Get (factState->GetMaterialWrapper ()->GetMaterial (),0);
    factFace.indices.Push (tris[i].a);
    factFace.indices.Push (tris[i].b);
    factFace.indices.Push (tris[i].c);
  }

  return true;
}

bool litThingGeometryExtractor::ExtractGeometry (litScene *scene,
                                                iMeshObjectFactory *csfact, 
                                                litLightingMeshFactory *lightmeshfact)
{
  csRef<iThingFactoryState> factState = 
    SCF_QUERY_INTERFACE (csfact, iThingFactoryState);
  if (!factState) return false;

  // Get our object2world transform too
  uint i,j;

  const csVector3 *overts = factState->GetVertices ();
  size_t numVerts = factState->GetVertexCount ();

  for (i = 0; i < numVerts; i++)
  {
    lightmeshfact->vertices.Push (overts[i]);
  }

  for (i = 0; i < (uint)factState->GetPolygonCount (); i++)
  {
    litLightingMeshFactory::litMeshFactoryFace factFace;
    factFace.material = scene->materials.Get (factState->GetPolygonMaterial (i)->GetMaterial (),0);
    
    int *idx = factState->GetPolygonVertexIndices (i);
    uint idxCount = factState->GetPolygonVertexCount (i);
    for (j = 0; j < idxCount; j++)
    {
      factFace.indices.Push (idx[j]);
    }
  }

  return true;
}
