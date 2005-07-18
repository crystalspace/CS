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

#include "csgeom/poly3d.h"
#include "csgeom/transfrm.h"

#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "imesh/thing.h"

bool csGeometryExtractor::ExtractGeometry (iMeshWrapper *csmesh, csLightingMesh *lightmesh)
{
  return true;
}

bool csGenmeshGeometryExtractor::ExtractGeometry (iMeshWrapper *csmesh, csLightingMesh *lightmesh)
{
  csRef<iGeneralMeshState> meshState = SCF_QUERY_INTERFACE (csmesh->GetMeshObject (), iGeneralMeshState);
  if (!meshState) return false;

  csRef<iGeneralFactoryState> factState = 
    SCF_QUERY_INTERFACE (csmesh->GetMeshObject ()->GetFactory (), iGeneralFactoryState);
  if (!factState) return false;

  // Get our object2world transform too
  csReversibleTransform o2w = csmesh->GetMovable ()->GetFullTransform ();

  csVector3 *overts = factState->GetVertices ();
  size_t numVerts = factState->GetVertexCount ();
  csTriangle *tris = factState->GetTriangles ();
  size_t numTri = factState->GetTriangleCount ();

  // Extract vertices
  uint i;
  for (i = 0; i < numVerts; i++)
  {
    lightmesh->vertexList.Push (o2w.This2Other (overts[i]));
  }
  lightmesh->colorList.SetLength (numVerts, csColor (0,0,0));
  lightmesh->orignalVertexCount = numVerts;

  // Extract and create faces
  for (i = 0; i < numTri; i++)
  {
    csMeshFace *face = new csMeshFace;
    face->mesh = lightmesh;
    lightmesh->faces.Push (face);
    
    csMeshPatch *patch = new csMeshPatch;
    patch->parentFace = face;
    face->patches.Push (patch);
    patch->SetVertexIdx (tris[i].a, tris[i].b, tris[i].c);
    patch->SetColorIdx (tris[i].a, tris[i].b, tris[i].c);    
    
    csVector3 geoNormal = (patch->GetVertex (1) - patch->GetVertex (0)) % (patch->GetVertex (2) - patch->GetVertex (0));

    patch->area = 0.5f * geoNormal.Norm ();
    face->geoNormal = geoNormal.Unit ();
  }
  // later on we should detect neightbours.. but ignore that for now

  return true;
}

bool csThingGeometryExtractor::ExtractGeometry (iMeshWrapper *csmesh, csLightingMesh *lightmesh)
{
  csRef<iThingState> meshState = SCF_QUERY_INTERFACE (csmesh->GetMeshObject (), iThingState);
  if (!meshState) return false;

  csRef<iThingFactoryState> factState = 
    SCF_QUERY_INTERFACE (csmesh->GetMeshObject ()->GetFactory (), iThingFactoryState);
  if (!factState) return false;

  // Get our object2world transform too
  csReversibleTransform o2w = csmesh->GetMovable ()->GetFullTransform ();
  uint i;

  const csVector3 *overts = factState->GetVertices ();
  size_t numVerts = factState->GetVertexCount ();

  for (i = 0; i < numVerts; i++)
  {
    lightmesh->vertexList.Push (o2w.This2Other (overts[i]));
  }
  lightmesh->colorList.SetLength (numVerts, csColor (0,0,0));
  lightmesh->orignalVertexCount = numVerts;

  for (i = 0; i < (uint)factState->GetPolygonCount (); i++)
  {
    uint polyVertCount = factState->GetPolygonVertexCount (i);
    int *idxList = factState->GetPolygonVertexIndices (i);

    csMeshFace *face = new csMeshFace;
    face->mesh = lightmesh;
    lightmesh->faces.Push (face);

    csMeshPatch *patch = new csMeshPatch;
    patch->parentFace = face;
    face->patches.Push (patch);

    csVector3 geoNormal;

    if (polyVertCount == 3)
    {
      patch->SetVertexIdx (idxList[0], idxList[1], idxList[2]);
      patch->SetColorIdx (idxList[0], idxList[1], idxList[2]);
      geoNormal = (patch->GetVertex (1) - patch->GetVertex (0)) % (patch->GetVertex (2) - patch->GetVertex (0));
      patch->area = 0.5f * geoNormal.Norm ();
    }
    else if (polyVertCount == 4)
    {
      patch->SetVertexIdx (idxList[0], idxList[1], idxList[2], idxList[3]);
      patch->SetColorIdx (idxList[0], idxList[1], idxList[2], idxList[3]);
      geoNormal = (patch->GetVertex (1) - patch->GetVertex (0)) % (patch->GetVertex (2) - patch->GetVertex (0));
      
      csPoly3D p;
      p.AddVertex (patch->GetVertex (0));p.AddVertex (patch->GetVertex (1));
      p.AddVertex (patch->GetVertex (2));p.AddVertex (patch->GetVertex (3));
      patch->area = p.GetArea ();
    }
    else
    {
      // Triangulate.. unfortunatly we need to triangulate to triangles :/
      patch->SetVertexIdx (idxList[0], idxList[1], idxList[2]);
      patch->SetColorIdx (idxList[0], idxList[1], idxList[2]);
      geoNormal = (patch->GetVertex (1) - patch->GetVertex (0)) % (patch->GetVertex (2) - patch->GetVertex (0));
      patch->area = 0.5f * geoNormal.Norm ();

      for (uint j = 1; j < polyVertCount-2; j++)
      {
        csMeshPatch *patch = new csMeshPatch;
        patch->parentFace = face;
        face->patches.Push (patch);
        patch->SetVertexIdx (idxList[0], idxList[j+1], idxList[j+2]);
        patch->SetColorIdx (idxList[0], idxList[j+1], idxList[j+2]);
        geoNormal = (patch->GetVertex (1) - patch->GetVertex (0)) % (patch->GetVertex (2) - patch->GetVertex (0));
        patch->area = 0.5f * geoNormal.Norm ();
      }
    }

    face->geoNormal = geoNormal.Unit ();
  }

  return true;
}
