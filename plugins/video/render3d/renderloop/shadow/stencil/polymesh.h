/* 
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_STENCIL_POLYMESH_H__
#define __CS_STENCIL_POLYMESH_H__

#include "csgeom/polymesh.h"

class csStencilPolygonMesh : public csPolygonMesh
{
  csDirtyAccessArray<csVector3> verts;
  csDirtyAccessArray<csMeshedPolygon> polys;
public:
  virtual ~csStencilPolygonMesh ()
  {
    int i;
    for (i = 0; i < polys.Length (); i++)
    {
      delete[] polys[i].vertices;
    }
  }

  void AddPolys (const csArray<csMeshedPolygon>& polysToAdd)
  {
    int i;
    for (i = 0; i < polysToAdd.Length (); i++)
    {
      polys.Push (polysToAdd[i]);
    }
    ShapeChanged ();
  }

  void CopyFrom (iPolygonMesh* polyMesh)
  {
    int numVerts = polyMesh->GetVertexCount ();
    csVector3* oldVerts = polyMesh->GetVertices ();
    verts.SetLength (numVerts);
    int i;
    for (i = 0; i < numVerts; i++)
    {
      verts[i].Set (oldVerts[i]);
    }

    int numPolys = polyMesh->GetPolygonCount ();
    csMeshedPolygon* oldPolys = polyMesh->GetPolygons ();
    polys.SetLength (numPolys);
    for (i = 0; i < numPolys; i++)
    {
      csMeshedPolygon poly;
      poly.num_vertices = oldPolys[i].num_vertices;
      poly.vertices = new int[poly.num_vertices];
      memcpy (poly.vertices, oldPolys[i].vertices, 
	poly.num_vertices * sizeof (int));
      polys[i] = poly;
    }

    ShapeChanged ();
  }

  virtual int GetVertexCount () 
  { 
    return verts.Length (); 
  }
  virtual csVector3* GetVertices () 
  { 
    return verts.GetArray (); 
  }
  virtual int GetPolygonCount () 
  { 
    return polys.Length (); 
  }
  virtual csMeshedPolygon* GetPolygons () 
  { 
    return polys.GetArray ();
  }
};

#endif // __CS_STENCIL_POLYMESH_H__
