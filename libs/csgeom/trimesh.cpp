/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csgeom/trimesh.h"

//---------------------------------------------------------------------------

csTriangleMesh::csTriangleMesh (const csTriangleMesh& mesh)
{
  triangles.SetLength (mesh.GetTriangleCount ());
  memcpy (triangles.GetArray (), mesh.GetTriangles (),
  	sizeof (csTriangle)*mesh.GetTriangleCount ());
}

csTriangleMesh::~csTriangleMesh ()
{
}

void csTriangleMesh::Clear ()
{
  triangles.SetLength (0);
}

void csTriangleMesh::SetSize (int count)
{
  triangles.SetLength (count);
}

void csTriangleMesh::SetTriangles (csTriangle const* trigs, int count)
{
  triangles.SetLength (count);
  memcpy (triangles.GetArray (), trigs, sizeof(csTriangle) * count);
}

void csTriangleMesh::AddTriangle (int a, int b, int c)
{
  csTriangle t (a, b, c);
  triangles.Push (t);
}

//---------------------------------------------------------------------------

void csTriangleVertex::AddTriangle (int idx)
{
  con_triangles.PushSmart (idx);
}

void csTriangleVertex::AddVertex (int idx)
{
  con_vertices.PushSmart (idx);
}

csTriangleVertices::csTriangleVertices (csTriangleMesh* mesh,
	csVector3* verts, int num_verts)
{
  vertices = new csTriangleVertex [num_verts];
  num_vertices = num_verts;

  // Build connectivity information for all vertices in this mesh.
  csTriangle* triangles = mesh->GetTriangles ();
  int i, j;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].pos = verts[i];
    vertices[i].idx = i;
    for (j = 0 ; j < mesh->GetTriangleCount () ; j++)
      if (triangles[j].a == i || triangles[j].b == i || triangles[j].c == i)
      {
        vertices[i].AddTriangle (j);
	if (triangles[j].a != i) vertices[i].AddVertex (triangles[j].a);
	if (triangles[j].b != i) vertices[i].AddVertex (triangles[j].b);
	if (triangles[j].c != i) vertices[i].AddVertex (triangles[j].c);
      }
  }
}

csTriangleVertices::~csTriangleVertices ()
{
  delete [] vertices;
}

void csTriangleVertices::UpdateVertices (csVector3* verts)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].pos = verts[i];
}

