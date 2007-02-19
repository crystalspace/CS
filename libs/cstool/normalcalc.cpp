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
#include "csqsqrt.h"
#include "csgeom/math3d.h"
#include "csgeom/trimesh.h"
#include "cstool/normalcalc.h"


struct CompressVertex
{
  size_t orig_idx;
  float x, y, z;
  size_t new_idx;
};

static int compare_vt (const void *p1, const void *p2)
{
  CompressVertex *sp1 = (CompressVertex *)p1;
  CompressVertex *sp2 = (CompressVertex *)p2;
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
  CompressVertex *sp1 = (CompressVertex *)p1;
  CompressVertex *sp2 = (CompressVertex *)p2;
  if (sp1->orig_idx < sp2->orig_idx)
    return -1;
  else if (sp1->orig_idx > sp2->orig_idx)
    return 1;
  return 0;
}

bool csNormalCalculator::CompressVertices (
    csVector3* orig_verts, size_t orig_num_vts,
    csVector3*& new_verts, size_t& new_num_vts,
    csTriangle* orig_tris, size_t num_tris,
    csTriangle*& new_tris,
    size_t*& mapping)
{
  new_num_vts = orig_num_vts;
  new_tris = orig_tris;
  new_verts = orig_verts;
  mapping = 0;
  if (orig_num_vts <= 0) return false;

  // Copy all the vertices.
  CompressVertex *vt = new CompressVertex[orig_num_vts];
  size_t i, j;
  for (i = 0; i < orig_num_vts; i++)
  {
    vt[i].orig_idx = i;
    vt[i].x = (float)ceil (orig_verts[i].x * 1000000);
    vt[i].y = (float)ceil (orig_verts[i].y * 1000000);
    vt[i].z = (float)ceil (orig_verts[i].z * 1000000);
  }

  // First sort so that all (nearly) equal vertices are together.
  qsort (vt, orig_num_vts, sizeof (CompressVertex), compare_vt);

  // Count unique values and tag all doubles with the index of the unique one.
  // new_idx in the vt table will be the index inside vt to the unique vector.
  new_num_vts = 1;
  size_t last_unique = 0;
  vt[0].new_idx = last_unique;
  for (i = 1 ; i < orig_num_vts ; i++)
  {
    if (
      vt[i].x != vt[last_unique].x ||
      vt[i].y != vt[last_unique].y ||
      vt[i].z != vt[last_unique].z)
    {
      last_unique = i;
      new_num_vts++;
    }

    vt[i].new_idx = last_unique;
  }

  // If count_unique == num_vertices then there is nothing to do.
  if (new_num_vts == orig_num_vts)
  {
    delete[] vt;
    return false;
  }

  // Now allocate and fill new vertex tables.
  // After this new_idx in the vt table will be the new index
  // of the vector.
  new_verts = new csVector3[new_num_vts];
  new_verts[0] = orig_verts[vt[0].orig_idx];

  vt[0].new_idx = 0;
  j = 1;
  for (i = 1 ; i < orig_num_vts ; i++)
  {
    if (vt[i].new_idx == i)
    {
      new_verts[j] = orig_verts[vt[i].orig_idx];
      vt[i].new_idx = j;
      j++;
    }
    else
      vt[i].new_idx = j - 1;
  }

  // Now we sort the table back on orig_idx so that we have
  // a mapping from the original indices to the new one (new_idx).
  qsort (vt, orig_num_vts, sizeof (CompressVertex), compare_vt_orig);

  // Now we can remap the vertices in all triangles.
  new_tris = new csTriangle[num_tris];
  for (i = 0 ; i < num_tris ; i++)
  {
    new_tris[i].a = (int)vt[orig_tris[i].a].new_idx;
    new_tris[i].b = (int)vt[orig_tris[i].b].new_idx;
    new_tris[i].c = (int)vt[orig_tris[i].c].new_idx;
  }
  mapping = new size_t[orig_num_vts];
  for (i = 0 ; i < orig_num_vts ; i++)
    mapping[i] = vt[i].new_idx;

  delete[] vt;
  return true;
}

void csNormalCalculator::CalculateNormals (
    csDirtyAccessArray<csVector3>& mesh_vertices,
    csDirtyAccessArray<csTriangle>& mesh_triangles,
    csDirtyAccessArray<csVector3>& mesh_normals,
    bool do_compress)
{
  size_t i;
  size_t j;

  mesh_normals.SetSize (mesh_vertices.GetSize ());
  size_t num_triangles = mesh_triangles.GetSize ();
  csTriangle* tris;
  csVector3* new_verts;
  size_t new_num_verts;
  size_t* mapping;

  bool compressed;
  if (do_compress)
  {
    compressed = CompressVertices (mesh_vertices.GetArray (),
      mesh_vertices.GetSize (),
      new_verts, new_num_verts,
      mesh_triangles.GetArray (), num_triangles, tris,
      mapping);
  }
  else
  {
    compressed = false;
    new_verts = mesh_vertices.GetArray ();
    new_num_verts = mesh_vertices.GetSize ();
    tris = mesh_triangles.GetArray ();
    mapping = 0;
  }

  csVector3* new_normals = mesh_normals.GetArray ();
  if (compressed)
    new_normals = new csVector3[new_num_verts];

  for (i = 0; i < new_num_verts; ++i)
  {
    new_normals[i].Set (0.0f);
  }

  // Calculate normals for each triangle, accumulate at the vertices weighted
  // by angle between edges
  for (i = 0; i < num_triangles; i++)
  {
    const csTriangle& tri = tris[i];

    // Accumulate it at the three vertices
    for (j = 0; j < 3; ++j)
    {
      csVector3 e1 = new_verts[tri[(j+1)%3]] - new_verts[tri[j]];
      csVector3 e2 = new_verts[tri[(j+2)%3]] - new_verts[tri[j]];

      csVector3 normalAtVertex = e1 % e2;

      float sqVertexNorm = normalAtVertex.SquaredNorm ();
      if (sqVertexNorm)
      {
        // This way we only have to compute two square roots instead of three..
        float sinAngleSq = sqVertexNorm / 
          (e1.SquaredNorm () * e2.SquaredNorm ());

        float triAngle = asinf (csClamp (sqrtf (sinAngleSq), 1.0f, 0.0f));

        new_normals[tri[j]] += normalAtVertex * (triAngle / sqrtf (sqVertexNorm));
      }
    }
  }

  for (i = 0; i < new_num_verts; ++i)
  {
    new_normals[i].Normalize ();
  }

  if (compressed)
  {
    // Translate the mapped normal table back to the original table.
    for (j = 0 ; j < mesh_vertices.GetSize () ; j++)
    {
      mesh_normals[j] = new_normals[mapping[j]];
    }

    delete[] new_normals;
    delete[] new_verts;
    delete[] tris;
    delete[] mapping;
  }

  //delete[] mesh_tri_normals;
}

//---------------------------------------------------------------------------
