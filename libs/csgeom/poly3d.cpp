/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "csgeom/poly3d.h"
#include "csgeom/poly2d.h"

csPoly3D::csPoly3D (size_t start_size)
{
  CS_ASSERT (start_size > 0);

  vertices.SetLength (start_size);
  MakeEmpty ();
}

csPoly3D::csPoly3D (const csPoly3D &copy)
{
  vertices = copy.vertices;
}

csPoly3D::~csPoly3D ()
{
}

void csPoly3D::MakeEmpty ()
{
  vertices.SetLength (0);
}

bool csPoly3D::In (const csVector3 &v) const
{
  size_t i, i1;
  i1 = vertices.Length () - 1;
  for (i = 0; i < vertices.Length (); i++)
  {
    if (csMath3::WhichSide3D (v, vertices[i1], vertices[i]) < 0)
      return false;
    i1 = i;
  }

  return true;
}

bool csPoly3D::In (csVector3 *poly, size_t num_poly, const csVector3 &v)
{
  CS_ASSERT (num_poly > 0);

  size_t i, i1;
  i1 = num_poly - 1;
  for (i = 0; i < num_poly; i++)
  {
    if (csMath3::WhichSide3D (v, poly[i1], poly[i]) < 0) return false;
    i1 = i;
  }

  return true;
}

void csPoly3D::MakeRoom (size_t new_max)
{
  vertices.SetCapacity (new_max);
}

size_t csPoly3D::AddVertex (float x, float y, float z)
{
  return vertices.Push (csVector3 (x, y, z));
}

bool csPoly3D::ProjectXPlane (
  const csVector3 &point,
  float plane_x,
  csPoly2D *poly2d) const
{
  poly2d->SetVertexCount (vertices.Length ());
  csVector2* verts = poly2d->GetVertices ();

  csVector3 v;
  float x_dist = plane_x - point.x;
  size_t i;
  for (i = 0; i < vertices.Length (); i++)
  {
    v = vertices[i] - point;
    if (ABS (v.x) < SMALL_EPSILON) return false;
    verts[i].x = point.y + x_dist * v.y / v.x;
    verts[i].y = point.z + x_dist * v.z / v.x;
  }

  return true;
}

bool csPoly3D::ProjectYPlane (
  const csVector3 &point,
  float plane_y,
  csPoly2D *poly2d) const
{
  poly2d->SetVertexCount (vertices.Length ());
  csVector2* verts = poly2d->GetVertices ();

  csVector3 v;
  float y_dist = plane_y - point.y;
  size_t i;
  for (i = 0; i < vertices.Length (); i++)
  {
    v = vertices[i] - point;
    if (ABS (v.y) < SMALL_EPSILON) return false;
    verts[i].x = point.x + y_dist * v.x / v.y;
    verts[i].y = point.z + y_dist * v.z / v.y;
  }

  return true;
}

bool csPoly3D::ProjectZPlane (
  const csVector3 &point,
  float plane_z,
  csPoly2D *poly2d) const
{
  poly2d->SetVertexCount (vertices.Length ());
  csVector2* verts = poly2d->GetVertices ();

  csVector3 v;
  float z_dist = plane_z - point.z;
  size_t i;
  for (i = 0; i < vertices.Length (); i++)
  {
    v = vertices[i] - point;
    if (ABS (v.z) < SMALL_EPSILON) return false;
    verts[i].x = point.x + z_dist * v.x / v.z;
    verts[i].y = point.y + z_dist * v.y / v.z;
  }

  return true;
}

int csPoly3D::Classify (
  const csPlane3 &pl,
  const csVector3 *vertices,
  size_t num_vertices)
{
  size_t i;
  size_t front = 0, back = 0;

  for (i = 0; i < num_vertices; i++)
  {
    float dot = pl.Classify (vertices[i]);
    if (ABS (dot) < EPSILON) dot = 0;
    if (dot > 0)
      back++;
    else if (dot < 0)
      front++;
  }

  if (back == 0 && front == 0) return CS_POL_SAME_PLANE;
  if (back == 0) return CS_POL_FRONT;
  if (front == 0) return CS_POL_BACK;
  return CS_POL_SPLIT_NEEDED;
}

int csPoly3D::ClassifyX (float x) const
{
  size_t i;
  int front = 0, back = 0;

  for (i = 0; i < vertices.Length (); i++)
  {
    float xx = vertices[i].x - x;
    if (xx < -EPSILON)
      front++;
    else if (xx > EPSILON)
      back++;
  }

  if (back == 0) return CS_POL_FRONT;
  if (front == 0) return CS_POL_BACK;
  return CS_POL_SPLIT_NEEDED;
}

int csPoly3D::ClassifyY (float y) const
{
  size_t i;
  int front = 0, back = 0;

  for (i = 0; i < vertices.Length (); i++)
  {
    float yy = vertices[i].y - y;
    if (yy < -EPSILON)
      front++;
    else if (yy > EPSILON)
      back++;
  }

  if (back == 0) return CS_POL_FRONT;
  if (front == 0) return CS_POL_BACK;
  return CS_POL_SPLIT_NEEDED;
}

int csPoly3D::ClassifyZ (float z) const
{
  size_t i;
  int front = 0, back = 0;

  for (i = 0; i < vertices.Length (); i++)
  {
    float zz = vertices[i].z - z;
    if (zz < -EPSILON)
      front++;
    else if (zz > EPSILON)
      back++;
  }

  if (back == 0) return CS_POL_FRONT;
  if (front == 0) return CS_POL_BACK;
  return CS_POL_SPLIT_NEEDED;
}

void csPoly3D::SplitWithPlane (
  csPoly3D &poly1,
  csPoly3D &poly2,
  const csPlane3 &split_plane) const
{
  poly1.MakeEmpty ();
  poly2.MakeEmpty ();

  csVector3 ptB;
  float sideA, sideB;
  csVector3 ptA = vertices[vertices.Length () - 1];
  sideA = split_plane.Classify (ptA);
  if (ABS (sideA) < SMALL_EPSILON) sideA = 0;

  int i;
  for (i = -1; ++i < (int)vertices.Length ();)
  {
    ptB = vertices[i];
    sideB = split_plane.Classify (ptB);
    if (ABS (sideB) < SMALL_EPSILON) sideB = 0;
    if (sideB > 0)
    {
      if (sideA < 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -split_plane.Classify (ptA) /
          (split_plane.Normal () * v);
        v *= sect;
        v += ptA;
        poly1.AddVertex (v);
        poly2.AddVertex (v);
      }

      poly2.AddVertex (ptB);
    }
    else if (sideB < 0)
    {
      if (sideA > 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -split_plane.Classify (ptA) /
          (split_plane.Normal () * v);
        v *= sect;
        v += ptA;
        poly1.AddVertex (v);
        poly2.AddVertex (v);
      }

      poly1.AddVertex (ptB);
    }
    else
    {
      poly1.AddVertex (ptB);
      poly2.AddVertex (ptB);
    }

    ptA = ptB;
    sideA = sideB;
  }
}

void csPoly3D::CutToPlane (const csPlane3 &split_plane)
{
  csPoly3D old (*this);
  MakeEmpty ();

  csVector3 ptB;
  float sideA, sideB;
  csVector3 ptA = old.vertices[old.vertices.Length () - 1];
  sideA = split_plane.Classify (ptA);
  if (ABS (sideA) < SMALL_EPSILON) sideA = 0;

  int i;
  for (i = -1; ++i < (int)old.vertices.Length ();)
  {
    ptB = old.vertices[i];
    sideB = split_plane.Classify (ptB);
    if (ABS (sideB) < SMALL_EPSILON) sideB = 0;
    if (sideB > 0)
    {
      if (sideA < 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -split_plane.Classify (ptA) /
          (split_plane.Normal () * v);
        v *= sect;
        v += ptA;
        AddVertex (v);
      }
    }
    else if (sideB < 0)
    {
      if (sideA > 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -split_plane.Classify (ptA) /
          (split_plane.Normal () * v);
        v *= sect;
        v += ptA;
        AddVertex (v);
      }

      AddVertex (ptB);
    }
    else
    {
      AddVertex (ptB);
    }

    ptA = ptB;
    sideA = sideB;
  }
}

void csPoly3D::SplitWithPlaneX (
  csPoly3D &poly1,
  csPoly3D &poly2,
  float x) const
{
  poly1.MakeEmpty ();
  poly2.MakeEmpty ();

  csVector3 ptB;
  float sideA, sideB;
  csVector3 ptA = vertices[vertices.Length () - 1];
  sideA = ptA.x - x;
  if (ABS (sideA) < SMALL_EPSILON) sideA = 0;

  int i;
  for (i = -1; ++i < (int)vertices.Length ();)
  {
    ptB = vertices[i];
    sideB = ptB.x - x;
    if (ABS (sideB) < SMALL_EPSILON) sideB = 0;
    if (sideB > 0)
    {
      if (sideA < 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -(ptA.x - x) / v.x;
        v *= sect;
        v += ptA;
        poly1.AddVertex (v);
        poly2.AddVertex (v);
      }

      poly2.AddVertex (ptB);
    }
    else if (sideB < 0)
    {
      if (sideA > 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -(ptA.x - x) / v.x;
        v *= sect;
        v += ptA;
        poly1.AddVertex (v);
        poly2.AddVertex (v);
      }

      poly1.AddVertex (ptB);
    }
    else
    {
      poly1.AddVertex (ptB);
      poly2.AddVertex (ptB);
    }

    ptA = ptB;
    sideA = sideB;
  }
}

void csPoly3D::SplitWithPlaneY (
  csPoly3D &poly1,
  csPoly3D &poly2,
  float y) const
{
  poly1.MakeEmpty ();
  poly2.MakeEmpty ();

  csVector3 ptB;
  float sideA, sideB;
  csVector3 ptA = vertices[vertices.Length () - 1];
  sideA = ptA.y - y;
  if (ABS (sideA) < SMALL_EPSILON) sideA = 0;

  int i;
  for (i = -1; ++i < (int)vertices.Length ();)
  {
    ptB = vertices[i];
    sideB = ptB.y - y;
    if (ABS (sideB) < SMALL_EPSILON) sideB = 0;
    if (sideB > 0)
    {
      if (sideA < 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -(ptA.y - y) / v.y;
        v *= sect;
        v += ptA;
        poly1.AddVertex (v);
        poly2.AddVertex (v);
      }

      poly2.AddVertex (ptB);
    }
    else if (sideB < 0)
    {
      if (sideA > 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -(ptA.y - y) / v.y;
        v *= sect;
        v += ptA;
        poly1.AddVertex (v);
        poly2.AddVertex (v);
      }

      poly1.AddVertex (ptB);
    }
    else
    {
      poly1.AddVertex (ptB);
      poly2.AddVertex (ptB);
    }

    ptA = ptB;
    sideA = sideB;
  }
}

void csPoly3D::SplitWithPlaneZ (
  csPoly3D &poly1,
  csPoly3D &poly2,
  float z) const
{
  poly1.MakeEmpty ();
  poly2.MakeEmpty ();

  csVector3 ptB;
  float sideA, sideB;
  csVector3 ptA = vertices[vertices.Length () - 1];
  sideA = ptA.z - z;
  if (ABS (sideA) < SMALL_EPSILON) sideA = 0;

  int i;
  for (i = -1; ++i < (int)vertices.Length ();)
  {
    ptB = vertices[i];
    sideB = ptB.z - z;
    if (ABS (sideB) < SMALL_EPSILON) sideB = 0;
    if (sideB > 0)
    {
      if (sideA < 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -(ptA.z - z) / v.z;
        v *= sect;
        v += ptA;
        poly1.AddVertex (v);
        poly2.AddVertex (v);
      }

      poly2.AddVertex (ptB);
    }
    else if (sideB < 0)
    {
      if (sideA > 0)
      {
        // Compute the intersection point of the line
        // from point A to point B with the partition
        // plane. This is a simple ray-plane intersection.
        csVector3 v = ptB;
        v -= ptA;

        float sect = -(ptA.z - z) / v.z;
        v *= sect;
        v += ptA;
        poly1.AddVertex (v);
        poly2.AddVertex (v);
      }

      poly1.AddVertex (ptB);
    }
    else
    {
      poly1.AddVertex (ptB);
      poly2.AddVertex (ptB);
    }

    ptA = ptB;
    sideA = sideB;
  }
}

//---------------------------------------------------------------------------
size_t csVector3Array::AddVertexSmart (float x, float y, float z)
{
  size_t i;
  for (i = 0; i < vertices.Length (); i++)
  {
    if (
        ABS (x - vertices[i].x) < SMALL_EPSILON &&
        ABS (y - vertices[i].y) < SMALL_EPSILON &&
        ABS (z - vertices[i].z) < SMALL_EPSILON)
      return i;
  }

  return AddVertex (x, y, z);
}

csVector3 csPoly3D::ComputeNormal (const csVector3 *vertices, size_t num)
{
  CS_ASSERT (num > 0);

  float ayz = 0;
  float azx = 0;
  float axy = 0;
  size_t i, i1;
  float x1, y1, z1, x, y, z;

  i1 = num - 1;
  x1 = vertices[i1].x;
  y1 = vertices[i1].y;
  z1 = vertices[i1].z;
  for (i = 0; i < num; i++)
  {
    x = vertices[i].x;
    y = vertices[i].y;
    z = vertices[i].z;
    ayz += (z1 + z) * (y - y1);
    azx += (x1 + x) * (z - z1);
    axy += (y1 + y) * (x - x1);
    x1 = x;
    y1 = y;
    z1 = z;
  }

  float sqd = ayz * ayz + azx * azx + axy * axy;
  float invd;
  if (sqd < SMALL_EPSILON)
    invd = 1.0f / SMALL_EPSILON;
  else
    invd = csQisqrt (sqd);
  return csVector3 (ayz * invd, azx * invd, axy * invd);
}

csVector3 csPoly3D::ComputeNormal (int* poly, size_t num, csVector3* vertices)
{
  CS_ASSERT (num > 0);

  float ayz = 0;
  float azx = 0;
  float axy = 0;
  size_t i, i1;
  float x1, y1, z1, x, y, z;

  i1 = num - 1;
  x1 = vertices[poly[i1]].x;
  y1 = vertices[poly[i1]].y;
  z1 = vertices[poly[i1]].z;
  for (i = 0; i < num; i++)
  {
    x = vertices[poly[i]].x;
    y = vertices[poly[i]].y;
    z = vertices[poly[i]].z;
    ayz += (z1 + z) * (y - y1);
    azx += (x1 + x) * (z - z1);
    axy += (y1 + y) * (x - x1);
    x1 = x;
    y1 = y;
    z1 = z;
  }

  float sqd = ayz * ayz + azx * azx + axy * axy;
  float invd;
  if (sqd < SMALL_EPSILON)
    invd = 1.0f / SMALL_EPSILON;
  else
    invd = csQisqrt (sqd);
  return csVector3 (ayz * invd, azx * invd, axy * invd);
}

csVector3 csPoly3D::ComputeNormal (const csArray<csVector3>& poly)
{
  return ComputeNormal (&poly[0], poly.Length ());
}

csPlane3 csPoly3D::ComputePlane (const csVector3 *vertices, size_t num_vertices)
{
  float D;
  csVector3 pl = ComputeNormal (vertices, num_vertices);
  D = -pl.x * vertices[0].x - pl.y * vertices[0].y - pl.z * vertices[0].z;
  return csPlane3 (pl, D);
}

csPlane3 csPoly3D::ComputePlane (int* poly, size_t num, csVector3* vertices)
{
  float D;
  csVector3 pl = ComputeNormal (poly, num, vertices);
  D = -pl.x * vertices[poly[0]].x - pl.y * vertices[poly[0]].y
  	- pl.z * vertices[poly[0]].z;
  return csPlane3 (pl, D);
}

csPlane3 csPoly3D::ComputePlane (const csArray<csVector3>& poly)
{
  return ComputePlane (&poly[0], poly.Length ());
}

float csPoly3D::GetArea () const
{
  float area = 0.0f;

  // triangulize the polygon, triangles are (0,1,2), (0,2,3), (0,3,4), etc..
  size_t i;
  for (i = 0; i < vertices.Length () - 2; i++)
    area += csMath3::DoubleArea3 (vertices[0], vertices[i + 1],
    	vertices[i + 2]);
  return area / 2.0f;
}

csVector3 csPoly3D::GetCenter () const
{
  size_t i;
  csBox3 bbox;
  bbox.StartBoundingBox (vertices[0]);
  for (i = 1; i < vertices.Length (); i++)
    bbox.AddBoundingVertexSmart (vertices[i]);
  return bbox.GetCenter ();
}

static int compare_vt (const void *p1, const void *p2)
{
  csCompressVertex *sp1 = (csCompressVertex *)p1;
  csCompressVertex *sp2 = (csCompressVertex *)p2;
  if (sp1->x < sp2->x)
    return -1;
  else if (sp1->x > sp2->x)
    return 1;
  if (sp1->y < sp2->y)
    return -1;
  else if (sp1->y > sp2->y)
    return 1;
  if (sp1->z < sp2->z)
    return -1;
  else if (sp1->z > sp2->z)
    return 1;
  return 0;
}

static int compare_vt_orig (const void *p1, const void *p2)
{
  csCompressVertex *sp1 = (csCompressVertex *)p1;
  csCompressVertex *sp2 = (csCompressVertex *)p2;
  if (sp1->orig_idx < sp2->orig_idx)
    return -1;
  else if (sp1->orig_idx > sp2->orig_idx)
    return 1;
  return 0;
}

template <class T>
static csCompressVertex* TemplatedCompressVertices (T& vertices,
	size_t num_vertices, csVector3*& new_vertices, size_t& new_count)
{
  new_vertices = 0;
  new_count = 0;
  if (num_vertices <= 0) return 0;

  // Copy all the vertices.
  csCompressVertex *vt = new csCompressVertex[num_vertices];
  size_t i, j;
  for (i = 0; i < num_vertices; i++)
  {
    vt[i].orig_idx = i;
    vt[i].x = (float)ceil (vertices[i].x * 1000000);
    vt[i].y = (float)ceil (vertices[i].y * 1000000);
    vt[i].z = (float)ceil (vertices[i].z * 1000000);
  }

  // First sort so that all (nearly) equal vertices are together.
  qsort (vt, num_vertices, sizeof (csCompressVertex), compare_vt);

  // Count unique values and tag all doubles with the index of the unique one.
  // new_idx in the vt table will be the index inside vt to the unique vector.
  new_count = 1;
  size_t last_unique = 0;
  vt[0].new_idx = last_unique;
  for (i = 1; i < num_vertices; i++)
  {
    if (
      vt[i].x != vt[last_unique].x ||
      vt[i].y != vt[last_unique].y ||
      vt[i].z != vt[last_unique].z)
    {
      last_unique = i;
      new_count++;
    }

    vt[i].new_idx = last_unique;
  }

  // If new_count == num_vertices then there is nothing to do.
  if (new_count == num_vertices)
  {
    delete[] vt;
    return 0;
  }

  // Now allocate and fill new vertex tables.
  // After this new_idx in the vt table will be the new index
  // of the vector.
  new_vertices = new csVector3[new_count];
  new_vertices[0] = vertices[vt[0].orig_idx];

  vt[0].new_idx = 0;
  j = 1;
  for (i = 1; i < num_vertices; i++)
  {
    if (vt[i].new_idx == i)
    {
      new_vertices[j] = vertices[vt[i].orig_idx];
      vt[i].new_idx = j;
      j++;
    }
    else
    {
      vt[i].new_idx = j - 1;
    }
  }

  // Now we sort the table back on orig_idx so that we have
  // a mapping from the original indices to the new one (new_idx).
  qsort (vt, num_vertices, sizeof (csCompressVertex),
  	compare_vt_orig);

  return vt;
}

csCompressVertex* csVector3Array::CompressVertices (csVector3* vertices,
	size_t num_vertices, csVector3*& new_vertices, size_t& new_count)
{
  return TemplatedCompressVertices (vertices, num_vertices,
  	new_vertices, new_count);
}

csCompressVertex* csVector3Array::CompressVertices (
	csArray<csVector3>& vertices)
{
  csVector3* new_vertices;
  size_t new_count;
  csCompressVertex* vt = TemplatedCompressVertices (vertices,
  	vertices.Length (), new_vertices, new_count);
  if (vt == 0) return 0;

  size_t i;
  vertices.DeleteAll ();
  for (i = 0 ; i < new_count ; i++)
    vertices.Push (new_vertices[i]);
  delete[] new_vertices;
  return vt;
}

//---------------------------------------------------------------------------
