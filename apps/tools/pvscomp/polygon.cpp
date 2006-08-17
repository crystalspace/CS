/*
    Copyright (C) 2006 by Benjamin Stover

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
#include "igeom/polymesh.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/objmodel.h"
#include "csutil/array.h"
#include "csgeom/vector3.h"
#include "csgeom/box.h"
#include "pvscomp.h"
//#include "iutil/object.h"
//#include "csutil/csstring.h"

Polygon::Polygon (const csVector3& a, const csVector3& b, const csVector3& c,
    const csVector3& d)
{
  numVertices = 4;

  vertices = new csVector3[4];
  vertices[0] = a;
  vertices[1] = b;
  vertices[2] = c;
  vertices[3] = d;

  index = new int[4];
  index[0] = 0;
  index[1] = 1;
  index[2] = 2;
  index[3] = 3;
}

Polygon::Polygon (iPolygonMesh* mesh, int polygonIndex)
{
  csMeshedPolygon* cspolygon = mesh->GetPolygons () + polygonIndex;
  numVertices = cspolygon->num_vertices;
  index = cspolygon->vertices;
  vertices = mesh->GetVertices ();
  freeData = false;
}

void Polygon::Print () const
{
  printf ("Num vertices: %d\n", numVertices);
  for (int i = 0; i < numVertices; i++)
  {
    printf("Index:  %d", index[i]);
    printf ("  (%f, %f, %f)\n", vertices[index[i]][0],
      vertices[index[i]][1], vertices[index[i]][2]);
  }
}

Polygon::~Polygon()
{
  if (freeData)
  {
    delete[] vertices;
    delete[] index;
  }
}

void Polygon::Fill (iMeshWrapper* wrapper, csArray<Polygon*>& fill)
{
  iPolygonMesh* polymesh = 
    wrapper->GetMeshObject ()->GetObjectModel ()->GetPolygonMeshViscull ();
  for (int i = 0; i < polymesh->GetPolygonCount (); i++)
  {
    Polygon* p = new Polygon (polymesh, i);
    fill.Push (p);
//    p.Print ();
  }
}

void Polygon::Fill (const csBox3& region, csArray<Polygon*>& fill)
{
  fill.Push (new Polygon (region.GetCorner (0), region.GetCorner (4),
                          region.GetCorner (5), region.GetCorner (1)));
  fill.Push (new Polygon (region.GetCorner (2), region.GetCorner (3),
                          region.GetCorner (7), region.GetCorner (6)));
  fill.Push (new Polygon (region.GetCorner (0), region.GetCorner (1),
                          region.GetCorner (3), region.GetCorner (2)));
  fill.Push (new Polygon (region.GetCorner (4), region.GetCorner (6),
                          region.GetCorner (7), region.GetCorner (5)));
  fill.Push (new Polygon (region.GetCorner (1), region.GetCorner (5),
                          region.GetCorner (7), region.GetCorner (3)));
  fill.Push (new Polygon (region.GetCorner (0), region.GetCorner (2),
                          region.GetCorner (6), region.GetCorner (4)));
}

void Polygon::Print (const csArray<Polygon*>& array)
{
  for (unsigned int i = 0; i < array.Length (); i++)
  {
    printf ("[Face #%d] ", i + 1);
    array[i]->Print ();
  }
}

void Polygon::Free (csArray<Polygon*>& array)
{
  for (unsigned int i = 0; i < array.Length (); i++)
    delete array[i];
  array.Empty ();
}

/*static csVector3 FindCenterOfRectangle (Polygon* p)
{
  csVector3 diag = (p->vertices[p->index[2]] - p->vertices[p->index[0]]) / 2;
  return p->vertices[p->index[2]] + diag;
} */

static void TestFillWithFaces ()
{
  csArray<Polygon*> regionFaces;
  csBox3 box;
  box.AddBoundingVertex(0, 0, 0);
  box.AddBoundingVertex(1, 1, 1);
  Polygon::Fill (box, regionFaces);
  Polygon::Print (regionFaces);
  Polygon::Free (regionFaces);
}
