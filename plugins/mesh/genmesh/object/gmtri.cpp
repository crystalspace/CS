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
#include "gmtri.h"

//---------------------------------------------------------------------------

csTriangleMesh::csTriangleMesh (const csTriangleMesh& mesh)
{
  max_triangles = mesh.max_triangles;
  num_triangles = mesh.num_triangles;
  triangles = new csTriangle [max_triangles];
  memcpy (triangles, mesh.triangles, sizeof (csTriangle)*max_triangles);
}

csTriangleMesh::~csTriangleMesh ()
{
  Clear ();
}

void csTriangleMesh::Clear ()
{
  delete [] triangles;
  num_triangles = max_triangles = 0;
}

void csTriangleMesh::SetSize(int count)
{
  delete [] triangles;
  triangles = new csTriangle[count];
  max_triangles = num_triangles = count;
}

void csTriangleMesh::SetTriangles( csTriangle const* trigs, int count)
{
  if (count > max_triangles)
  {
    delete [] triangles;
    triangles = new csTriangle[count];
    max_triangles = count;
  }
  memcpy(triangles, trigs, sizeof(csTriangle) * count);
  num_triangles = count;
}

void csTriangleMesh::Reset ()
{
  num_triangles = 0;
}

void csTriangleMesh::AddTriangle (int a, int b, int c)
{
  if (num_triangles >= max_triangles)
  {
    csTriangle* new_triangles = new csTriangle [max_triangles+8];
    if (triangles)
    {
      memcpy (new_triangles, triangles, sizeof (csTriangle)*max_triangles);
      delete [] triangles;
    }
    triangles = new_triangles;
    max_triangles += 8;
  }
  triangles[num_triangles].a = a;
  triangles[num_triangles].b = b;
  triangles[num_triangles].c = c;
  num_triangles++;
}

//---------------------------------------------------------------------------

void csTriangleVertex::AddTriangle (int idx)
{
  int i;
  for (i = 0 ; i < num_con_triangles ; i++)
    if (con_triangles[i] == idx) return;

  if (num_con_triangles >= max_con_triangles)
  {
    int* new_con_triangles = new int [max_con_triangles+4];
    if (con_triangles)
    {
      memcpy (new_con_triangles, con_triangles, sizeof (int)*max_con_triangles);
      delete [] con_triangles;
    }
    con_triangles = new_con_triangles;
    max_con_triangles += 4;
  }
  con_triangles[num_con_triangles] = idx;
  num_con_triangles++;
}

void csTriangleVertex::AddVertex (int idx)
{
  int i;
  for (i = 0 ; i < num_con_vertices ; i++)
    if (con_vertices[i] == idx) return;

  if (num_con_vertices >= max_con_vertices)
  {
    int* new_con_vertices = new int [max_con_vertices+4];
    if (con_vertices)
    {
      memcpy (new_con_vertices, con_vertices, sizeof (int)*max_con_vertices);
      delete [] con_vertices;
    }
    con_vertices = new_con_vertices;
    max_con_vertices += 4;
  }
  con_vertices[num_con_vertices] = idx;
  num_con_vertices++;
}

csGenTriangleVertices::csGenTriangleVertices (csTriangleMesh* mesh,
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

csGenTriangleVertices::~csGenTriangleVertices ()
{
  delete [] vertices;
}

void csGenTriangleVertices::UpdateVertices (csVector3* verts)
{
  int i;
  for (i = 0 ; i < num_vertices ; i++)
    vertices[i].pos = verts[i];
}

