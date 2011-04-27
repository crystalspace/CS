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
#include "csgeom/plane3.h"
#include "csgeom/sphere.h"
#include "csgeom/trimesh.h"
#include "cstool/primitives.h"

namespace CS
{
namespace Geometry
{

//------------------------------------------------------------------------

csVector2 DensityTextureMapper::Map (const csVector3& p,
    const csVector3& n, size_t)
{
  csVector3 a1, a2;
  csPlane3::FindOrthogonalPoints (n, a1, a2);
  // Calculate the closest point from point 'p' on the plane given
  // by 'n' and the origin.
  csVector3 closest = p - n * (n * p);

  csVector2 uv;
  uv.x = a1 * closest * density;
  uv.y = a2 * closest * density;
  return uv;
}

//------------------------------------------------------------------------

static void TriSwap (csTriangle*& tr, const csTriangle tri, bool in)
{
  if (in)
  {
    tr->a = tri.c;
    tr->b = tri.b;
    tr->c = tri.a;
  }
  else
  {
    tr->a = tri.a;
    tr->b = tri.b;
    tr->c = tri.c;
  }
  tr++;
}

static void NormSwap (csVector3& n, bool in)
{
  n.Normalize ();
  if (in) n = -n;
}

csVector2 Primitives::boxTable[] =
{
  csVector2 ( 0, 0 ), csVector2 ( 0, 1 ), csVector2 ( 1, 0 ),
  csVector2 ( 0, 0 ), csVector2 ( 0, 0 ), csVector2 ( 1, 0 ),
  csVector2 ( 1, 0 ), csVector2 ( 0, 0 ), csVector2 ( 1, 0 ),
  csVector2 ( 1, 0 ), csVector2 ( 1, 1 ), csVector2 ( 0, 0 ),
  csVector2 ( 0, 1 ), csVector2 ( 1, 1 ), csVector2 ( 1, 1 ),
  csVector2 ( 0, 1 ), csVector2 ( 1, 1 ), csVector2 ( 1, 0 ),
  csVector2 ( 1, 1 ), csVector2 ( 0, 1 ), csVector2 ( 0, 0 ),
  csVector2 ( 0, 1 ), csVector2 ( 1, 1 ), csVector2 ( 0, 1 )
};

csVector2 Primitives::quadTable[] =
{
  csVector2 (0,0), csVector2 (0,1), csVector2 (1,1), csVector2 (1,0)
};

void Primitives::GenerateBox (
      const csBox3& box,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      uint32 flags, TextureMapper* mapper)
{
  bool alloced = false;
  if (!mapper)
  {
    alloced = true;
    mapper = new TableTextureMapper (boxTable);
  }

  mesh_vertices.SetSize (24);
  mesh_texels.SetSize (24);
  mesh_normals.SetSize (24);
  csVector3* vertices = mesh_vertices.GetArray ();
  vertices[0].Set(box.MinX(), box.MaxY(), box.MinZ());
  vertices[1].Set(box.MinX(), box.MaxY(), box.MinZ());
  vertices[2].Set(box.MinX(), box.MaxY(), box.MinZ());

  vertices[3].Set(box.MinX(), box.MaxY(), box.MaxZ());
  vertices[4].Set(box.MinX(), box.MaxY(), box.MaxZ());
  vertices[5].Set(box.MinX(), box.MaxY(), box.MaxZ());

  vertices[6].Set(box.MaxX(), box.MaxY(), box.MaxZ());
  vertices[7].Set(box.MaxX(), box.MaxY(), box.MaxZ());
  vertices[8].Set(box.MaxX(), box.MaxY(), box.MaxZ());

  vertices[9].Set(box.MaxX(), box.MaxY(), box.MinZ());
  vertices[10].Set(box.MaxX(), box.MaxY(), box.MinZ());
  vertices[11].Set(box.MaxX(), box.MaxY(), box.MinZ());

  vertices[12].Set(box.MinX(), box.MinY(), box.MaxZ());
  vertices[13].Set(box.MinX(), box.MinY(), box.MaxZ());
  vertices[14].Set(box.MinX(), box.MinY(), box.MaxZ());

  vertices[15].Set(box.MaxX(), box.MinY(), box.MaxZ());
  vertices[16].Set(box.MaxX(), box.MinY(), box.MaxZ());
  vertices[17].Set(box.MaxX(), box.MinY(), box.MaxZ());

  vertices[18].Set(box.MaxX(), box.MinY(), box.MinZ());
  vertices[19].Set(box.MaxX(), box.MinY(), box.MinZ());
  vertices[20].Set(box.MaxX(), box.MinY(), box.MinZ());

  vertices[21].Set(box.MinX(), box.MinY(), box.MinZ());
  vertices[22].Set(box.MinX(), box.MinY(), box.MinZ());
  vertices[23].Set(box.MinX(), box.MinY(), box.MinZ());

  mesh_triangles.SetSize (12);
  csTriangle* triangles = mesh_triangles.GetArray ();

  bool in = flags & CS_PRIMBOX_INSIDE;
  TriSwap (triangles, csTriangle (0, 9, 18), in);
  TriSwap (triangles, csTriangle (0, 18, 21), in);
  TriSwap (triangles, csTriangle (3, 6, 10), in);
  TriSwap (triangles, csTriangle (3, 10, 1), in);
  TriSwap (triangles, csTriangle (4, 2, 22), in);
  TriSwap (triangles, csTriangle (4, 22, 12), in);
  TriSwap (triangles, csTriangle (7, 5, 13), in);
  TriSwap (triangles, csTriangle (7, 13, 15), in);
  TriSwap (triangles, csTriangle (11, 8, 16), in);
  TriSwap (triangles, csTriangle (11, 16, 19), in);
  TriSwap (triangles, csTriangle (23, 20, 17), in);
  TriSwap (triangles, csTriangle (23, 17, 14), in);

  csVector3* n = mesh_normals.GetArray ();
  if (flags & CS_PRIMBOX_SMOOTH)
  {
    // Corner -X +Y -Z
    n[0].Set(box.MinX(),box.MaxY(),box.MinZ()); NormSwap (n[0], in);
    n[1].Set(box.MinX(),box.MaxY(),box.MinZ()); NormSwap (n[1], in);
    n[2].Set(box.MinX(),box.MaxY(),box.MinZ()); NormSwap (n[2], in);

    // Corner -X +Y +Z
    n[3].Set(box.MinX(),box.MaxY(),box.MaxZ()); NormSwap (n[3], in);
    n[4].Set(box.MinX(),box.MaxY(),box.MaxZ()); NormSwap (n[4], in);
    n[5].Set(box.MinX(),box.MaxY(),box.MaxZ()); NormSwap (n[5], in);

    // Corner +X +Y +Z
    n[6].Set(box.MaxX(),box.MaxY(),box.MaxZ()); NormSwap (n[6], in);
    n[7].Set(box.MaxX(),box.MaxY(),box.MaxZ()); NormSwap (n[7], in);
    n[8].Set(box.MaxX(),box.MaxY(),box.MaxZ()); NormSwap (n[8], in);

    // Corner +X +Y -Z
    n[9].Set(box.MaxX(),box.MaxY(),box.MinZ()); NormSwap (n[9], in);
    n[10].Set(box.MaxX(),box.MaxY(),box.MinZ()); NormSwap (n[10], in);
    n[11].Set(box.MaxX(),box.MaxY(),box.MinZ()); NormSwap (n[11], in);

    // Corner -X -Y +Z
    n[12].Set(box.MinX(),box.MinY(),box.MaxZ()); NormSwap (n[12], in);
    n[13].Set(box.MinX(),box.MinY(),box.MaxZ()); NormSwap (n[13], in);
    n[14].Set(box.MinX(),box.MinY(),box.MaxZ()); NormSwap (n[14], in);

    // Corner +X -Y +Z
    n[15].Set(box.MaxX(),box.MinY(),box.MaxZ()); NormSwap (n[15], in);
    n[16].Set(box.MaxX(),box.MinY(),box.MaxZ()); NormSwap (n[16], in);
    n[17].Set(box.MaxX(),box.MinY(),box.MaxZ()); NormSwap (n[17], in);

    // Corner +X -Y -Z
    n[18].Set(box.MaxX(),box.MinY(),box.MinZ()); NormSwap (n[18], in);
    n[19].Set(box.MaxX(),box.MinY(),box.MinZ()); NormSwap (n[19], in);
    n[20].Set(box.MaxX(),box.MinY(),box.MinZ()); NormSwap (n[20], in);

    // Corner -X -Y -Z
    n[21].Set(box.MinX(),box.MinY(),box.MinZ()); NormSwap (n[21], in);
    n[22].Set(box.MinX(),box.MinY(),box.MinZ()); NormSwap (n[22], in);
    n[23].Set(box.MinX(),box.MinY(),box.MinZ()); NormSwap (n[23], in);
  }
  else
  {
    // Face 1 (-Z).
    n[0].Set(0, 0, -1); NormSwap (n[0], in);
    n[9].Set(0, 0, -1); NormSwap (n[9], in);
    n[18].Set(0, 0, -1); NormSwap (n[18], in);
    n[21].Set(0, 0, -1); NormSwap (n[21], in);

    // Face 2 (+Y).
    n[1].Set(0, 1, 0); NormSwap (n[1], in);
    n[3].Set(0, 1, 0); NormSwap (n[3], in);
    n[6].Set(0, 1, 0); NormSwap (n[6], in);
    n[10].Set(0, 1, 0); NormSwap (n[10], in);

    // Face 3 (-X).
    n[2].Set(-1, 0, 0); NormSwap (n[2], in);
    n[4].Set(-1, 0, 0); NormSwap (n[4], in);
    n[12].Set(-1, 0, 0); NormSwap (n[12], in);
    n[22].Set(-1, 0, 0); NormSwap (n[22], in);

    // Face 4 (+Z).
    n[5].Set(0, 0, 1); NormSwap (n[5], in);
    n[7].Set(0, 0, 1); NormSwap (n[7], in);
    n[13].Set(0, 0, 1); NormSwap (n[13], in);
    n[15].Set(0, 0, 1); NormSwap (n[15], in);

    // Face 5 (+X).
    n[8].Set(1, 0, 0); NormSwap (n[8], in);
    n[11].Set(1, 0, 0); NormSwap (n[11], in);
    n[16].Set(1, 0, 0); NormSwap (n[16], in);
    n[19].Set(1, 0, 0); NormSwap (n[19], in);

    // Face 6 (-Y).
    n[14].Set(0, -1, 0); NormSwap (n[14], in);
    n[17].Set(0, -1, 0); NormSwap (n[17], in);
    n[20].Set(0, -1, 0); NormSwap (n[20], in);
    n[23].Set(0, -1, 0); NormSwap (n[23], in);
  }

  csVector2* texels = mesh_texels.GetArray ();
  // the comments indicate which face
  // (numbered 1-6) the texel applies to
  size_t i;
  for (i = 0 ; i < 24 ; i++)
  {
    texels[i] = mapper->Map (vertices[i], n[i], i);
  }
  if (alloced) delete mapper;
}

void Primitives::GenerateCylinder (float l, float r, uint sides,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      TextureMapper* mapper)
{
  const uint n = sides * 4;
  l *= 0.5;
  float a = float(PI*2.0)/float(n);
  float sa = (float) sin(a);
  float ca = (float) cos(a);

  mesh_normals.DeleteAll ();
  mesh_texels.DeleteAll ();
  mesh_triangles.DeleteAll ();
  mesh_vertices.DeleteAll ();

  mesh_normals.Push (csVector3 (1, 0, 0));
  mesh_vertices.Push (csVector3 (l, 0, 0));
  mesh_normals.Push (csVector3 (-1, 0, 0));
  mesh_vertices.Push (csVector3 (-l, 0, 0));

  // cylinder body
  float ny = 1, nz = 0;
  for (uint i = 0; i < n; i++)
  {
    mesh_normals.Push (csVector3 (0, ny, nz));
    int v1 = int (mesh_vertices.Push (csVector3 (l, ny * r, nz * r)));
    mesh_normals.Push (csVector3 (0, ny, nz));
    int v2 = int (mesh_vertices.Push (csVector3 (-l, ny * r, nz * r)));

    float tmp =  ca * ny - sa * nz;
    nz = sa*ny + ca*nz;
    ny = tmp;
    
    mesh_triangles.Push (csTriangle (v1, 0, v1 - 2));
    mesh_triangles.Push (csTriangle (v2 - 2, 1, v2));

    if (i > 0)
    {
      mesh_triangles.Push (csTriangle (v2, v1, v1 - 2));
      mesh_triangles.Push (csTriangle (v1 - 2, v2 - 2, v2));

      if (i == n - 1)
      {
        mesh_triangles.Push (csTriangle (2, 0, v1));
	mesh_triangles.Push (csTriangle (v2, 1, 3));
        mesh_triangles.Push (csTriangle (3, 2, v1));
        mesh_triangles.Push (csTriangle (v1, v2, 3));
      }
    }
  }

  if (mapper)
  {
    size_t i;
    for (i = 0 ; i < mesh_vertices.GetSize () ; i++)
      mesh_texels.Push (mapper->Map (mesh_vertices[i], mesh_normals[i], i));
  }
}

void Primitives::GenerateCapsule (float l, float r, uint sides,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      TextureMapper* mapper)
{
  const uint n = sides * 4;
  l *= 0.5;
  float a = float(PI*2.0)/float(n);
  float sa = (float) sin(a);
  float ca = (float) cos(a);

  mesh_normals.DeleteAll ();
  mesh_texels.DeleteAll ();
  mesh_triangles.DeleteAll ();
  mesh_vertices.DeleteAll ();

  // capsule body (ie cylinder part)
  float ny = 1, nz = 0;
  for (uint i = 0; i < n; i++)
  {
    mesh_normals.Push (csVector3 (0, ny, nz));
    int v1 = int (mesh_vertices.Push (csVector3 (l, ny * r, nz * r)));
    mesh_normals.Push (csVector3 (0, ny, nz));
    int v2 = int (mesh_vertices.Push (csVector3 (-l, ny * r, nz * r)));

    float tmp =  ca * ny - sa * nz;
    nz = sa*ny + ca*nz;
    ny = tmp;
    
    if (i > 0)
    {
      mesh_triangles.Push (csTriangle (v2, v1, v1 - 2));
      mesh_triangles.Push (csTriangle (v1 - 2, v2 - 2, v2));

      if (i == n - 1)
      {
	mesh_triangles.Push (csTriangle (1, 0, v1));
        mesh_triangles.Push (csTriangle (v1, v2, 1));
      }
    }
  }

  // top cap
  float start_nx = 0;
  float start_ny = 1;
  for (uint j = 0; j <= sides; j++) 
  {
    float start_nx2 =  ca*start_nx + sa*start_ny;
    float start_ny2 = -sa*start_nx + ca*start_ny;
    float nx = start_nx; ny = start_ny; nz = 0;
    float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
    for (uint i = 0; i <= n; i++) 
    {
      mesh_normals.Push (csVector3 (nx2, ny2, nz2));
      int v1 = int (mesh_vertices.Push (csVector3 (l+nx2*r, ny2*r, nz2*r)));
      mesh_normals.Push (csVector3 (nx, ny, nz));
      int v2 = int (mesh_vertices.Push (csVector3 (l+nx*r, ny*r, nz*r)));

      float tmp = ca*ny - sa*nz;
      nz = sa*ny + ca*nz;
      ny = tmp;
      tmp = ca*ny2- sa*nz2;
      nz2 = sa*ny2 + ca*nz2;
      ny2 = tmp;

      if (i > 0)
      {
        mesh_triangles.Push (csTriangle (v2, v1, v1 - 2));
        mesh_triangles.Push (csTriangle (v1 - 2, v2 - 2, v2));
      }
    }
    start_nx = start_nx2;
    start_ny = start_ny2;
  }

  // second cap
  start_nx = 0;
  start_ny = 1;
  for (uint j = 0; j <= sides; j++) 
  {
    float start_nx2 =  ca*start_nx - sa*start_ny;
    float start_ny2 = sa*start_nx + ca*start_ny;
    float nx = start_nx; ny = start_ny; nz = 0;
    float nx2 = start_nx2, ny2 = start_ny2, nz2 = 0;
    for (uint i = 0; i <= n; i++) 
    {
      mesh_normals.Push (csVector3 (nx, ny, nz));
      int v1 = int (mesh_vertices.Push (csVector3 (-l+nx*r, ny*r, nz*r)));
      mesh_normals.Push (csVector3 (nx2, ny2, nz2));
      int v2 = int (mesh_vertices.Push (csVector3 (-l+nx2*r, ny2*r, nz2*r)));

      float tmp = ca*ny - sa*nz;
      nz = sa*ny + ca*nz;
      ny = tmp;
      tmp = ca*ny2 - sa*nz2;
      nz2 = sa*ny2 + ca*nz2;
      ny2 = tmp;

      if (i > 0)
      {
        mesh_triangles.Push (csTriangle (v2, v1, v1 - 2));
        mesh_triangles.Push (csTriangle (v1 - 2, v2 - 2, v2));
      }
    }
    start_nx = start_nx2;
    start_ny = start_ny2;
  }
  if (mapper)
  {
    size_t i;
    for (i = 0 ; i < mesh_vertices.GetSize () ; i++)
      mesh_texels.Push (mapper->Map (mesh_vertices[i], mesh_normals[i], i));
  }
}

void Primitives::GenerateCone (float l, float r, uint sides,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      TextureMapper* mapper)
{
  /* Generates a cone aligned along the positive y-axis with the base 
   * centered on the origin. */

  // Make sure we have enough sides to actually make a cone.
  sides = MAX(3, sides);

  // The top-point.
  mesh_vertices.Push (csVector3 (0, l, 0));
  mesh_normals.Push (csVector3 (0, 1, 0));

  // Generates the ring of verticies that defines the base.
  float angle = 0.0f;
  const float angleInc = TWO_PI / (float)sides;

  for (uint i = 0; i < sides; i++)
  {
    float x = cos (angle);
    float z = sin (angle);

    mesh_vertices.Push (r * csVector3 (x, 0, z));
    mesh_normals.Push (csVector3 (x, 0, z));

    angle += angleInc;
  }

  // Creates the side and base triangles of the cone.
  for (uint i = 0; i < sides; i++)
  {
    int a = i + 1;
    int b = ((i + 1) % sides) + 1;

    // Side triangle
    mesh_triangles.Push (csTriangle (0, a, b));

    // Base triangle
    mesh_triangles.Push (csTriangle (1, b, a));
  }

  if (mapper)
  {
    for (size_t i = 0; i < mesh_vertices.GetSize (); i++)
      mesh_texels.Push (mapper->Map (mesh_vertices[i], mesh_normals[i], i));
  }
}

void Primitives::GenerateQuad (const csVector3 &v1, const csVector3 &v2,
                          const csVector3 &v3, const csVector3 &v4,
                          csDirtyAccessArray<csVector3>& mesh_vertices,
                          csDirtyAccessArray<csVector2>& mesh_texels,
                          csDirtyAccessArray<csVector3>& mesh_normals,
                          csDirtyAccessArray<csTriangle>& mesh_triangles,
			  TextureMapper* mapper)
{
  bool alloced = false;
  if (!mapper)
  {
    alloced = true;
    mapper = new TableTextureMapper (quadTable);
  }

  mesh_vertices.SetSize (4);
  mesh_texels.SetSize (4);
  mesh_normals.SetSize (4);
  mesh_triangles.SetSize (4);

  mesh_normals[0] = mesh_vertices[0] = v1;
  mesh_normals[1] = mesh_vertices[1] = v2;
  mesh_normals[2] = mesh_vertices[2] = v3;
  mesh_normals[3] = mesh_vertices[3] = v4;

  mesh_normals[0].Normalize ();
  mesh_normals[1].Normalize ();
  mesh_normals[2].Normalize ();
  mesh_normals[3].Normalize ();

  mesh_texels[0] = mapper->Map (mesh_vertices[0], mesh_normals[0], 0);
  mesh_texels[1] = mapper->Map (mesh_vertices[1], mesh_normals[1], 1);
  mesh_texels[2] = mapper->Map (mesh_vertices[2], mesh_normals[2], 2);
  mesh_texels[3] = mapper->Map (mesh_vertices[3], mesh_normals[3], 3);

  mesh_triangles[0].a = 3; mesh_triangles[0].b = 0; mesh_triangles[0].c = 1;
  mesh_triangles[1].a = 0; mesh_triangles[1].b = 1; mesh_triangles[1].c = 2;
  mesh_triangles[2].a = 1; mesh_triangles[2].b = 2; mesh_triangles[2].c = 3;
  mesh_triangles[3].a = 2; mesh_triangles[3].b = 3; mesh_triangles[3].c = 0;

  if (alloced) delete mapper;
}

void Primitives::GenerateTesselatedQuad (const csVector3 &v0,
                          const csVector3 &v1, const csVector3 &v2,
			  int tesselations,
                          csDirtyAccessArray<csVector3>& mesh_vertices,
                          csDirtyAccessArray<csVector2>& mesh_texels,
                          csDirtyAccessArray<csVector3>& mesh_normals,
                          csDirtyAccessArray<csTriangle>& mesh_triangles,
			  TextureMapper* mapper)
{
  bool alloced = false;
  if (!mapper)
  {
    alloced = true;
    mapper = new DensityTextureMapper (1.0f);
  }

  size_t num_verts = (tesselations+1) * (tesselations+1);
  size_t num_tri = tesselations * tesselations * 2;
  mesh_vertices.SetSize (num_verts);
  mesh_texels.SetSize (num_verts);
  mesh_normals.SetSize (num_verts);
  mesh_triangles.SetSize (num_tri);

  csPlane3 plane (v0, v1, v2);
  csVector3 normal = plane.Normal ();
  normal.Normalize ();
  float d = 1.0f / float (tesselations);
  csVector3 v1_d = d * (v1-v0);
  int x, y;
  size_t i = 0;
  for (y = 0 ; y <= tesselations ; y++)
  {
    csVector3 v0_v2_y = v0 + float (y) * d * (v2-v0);
    for (x = 0 ; x <= tesselations ; x++)
    {
      mesh_vertices[i] = v0_v2_y;
      v0_v2_y += v1_d;
      mesh_normals[i] = normal;
      i++;
    }
  }

  i = 0;
  for (y = 0 ; y < tesselations ; y++)
  {
    int yt = y * (tesselations+1);
    for (x = 0 ; x < tesselations ; x++)
    {
      int p1 = yt + x;
      int p2 = yt + x + 1;
      int p3 = tesselations + 1 + p1;
      int p4 = tesselations + 1 + p2;
      mesh_triangles[i++].Set (p1, p2, p4);
      mesh_triangles[i++].Set (p1, p4, p3);
    }
  }

  for (i = 0 ; i < num_verts ; i++)
    mesh_texels[i] = mapper->Map (mesh_vertices[i], mesh_normals[i], i);

  if (alloced) delete mapper;
}

void Primitives::GenerateSphere (const csEllipsoid& ellips, int num,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      bool cyl_mapping, bool toponly, bool reversed,
      TextureMapper* mapper)
{
  int num_vertices = 0;
  int num_triangles = 0;
  csDirtyAccessArray<csVector3> vertices;
  csDirtyAccessArray<csVector2> uvverts;
  csDirtyAccessArray<csTriangle> triangles;
  float radius = 1.0f;

  csArray<int> prev_verticesT;
  csArray<int> prev_verticesB;
  float u = 0.0f, v = 0.0f;
  int i, j;

  // Number of degrees between layers.
  float radius_step = 180.0f / num;
  float vert_radius = radius;

  // If cylindrical mapping is used we duplicate the last column of
  // vertices. That is because we need to connect the two sides of the
  // texture and a vertex can only have one texture coordinate.
  int num2 = num;
  if (cyl_mapping) num2++;

  // Generate the first series of vertices (the outer circle).
  // Calculate u,v for them.
  for (j = 0; j < num2; j++)
  {
    float new_radius = radius;
    float new_height = 0.0f;
    float angle = j * 2.0f * radius_step * TWO_PI / 360.0f;
    prev_verticesT.GetExtend (j) = num_vertices;
    prev_verticesB.GetExtend (j) = num_vertices;
    vertices.GetExtend (num_vertices).Set (new_radius * (float) cos (angle),
      new_height, new_radius * (float) sin (angle));

    if (!mapper)
    {
      if (cyl_mapping)
      {
        u = float (j) / float (num);
        v = 0.5f;
      }
      else
      {
        u = (float) cos (angle) * 0.5f + 0.5f;
        v = (float) sin (angle) * 0.5f + 0.5f;
      }

      uvverts.GetExtend (num_vertices).Set (u, v);
    }
    num_vertices++;
  }

  // Array with new vertex indices.
  csArray<int> new_verticesT;
  csArray<int> new_verticesB;

  // First create the layered triangle strips.
  for (i = 1; i < (num / 2); i++)
  {
    //-----
    // First create a new series of vertices.
    //-----
    // Angle from the center to the new circle of vertices.
    float new_angle = i * radius_step * TWO_PI / 360.0f;
    // Radius of the new circle of vertices.
    float new_radius = radius * (float) cos (new_angle);
    // Height of the new circle of vertices.
    float new_height = vert_radius * (float) sin (new_angle);
    // UV radius.
    float uv_radius = (1.0f - 2.0f * (float) i / (float)num) * 0.5f;
    for (j = 0; j < num2; j++)
    {
      float angle = j * 2.0f * radius_step * TWO_PI / 360.0f;

      if (!mapper)
      {
        if (cyl_mapping)
        {
          u = float (j) / float (num);
          v = 1.0f - float (i + num / 2) / float (num);
        }
        else
        {
          u = uv_radius * (float) cos (angle) + 0.5f;
          v = uv_radius * (float) sin (angle) + 0.5f;
        }
        uvverts.GetExtend (num_vertices).Set (u, v);
      }

      new_verticesT.GetExtend (j) = num_vertices;
      vertices.GetExtend (num_vertices).Set (new_radius * (float) cos (angle),
        new_height, new_radius * (float) sin (angle));
      num_vertices++;

      if (!toponly)
      {
        new_verticesB.GetExtend (j) = num_vertices;
        vertices.GetExtend (num_vertices).Set (new_radius * (float) cos (angle),
          -new_height, new_radius * (float) sin (angle));

	if (!mapper)
	{
          if (cyl_mapping) v = 1.0f - v;
          uvverts.GetExtend (num_vertices).Set (u, v);
	}
        num_vertices++;
      }
    }

    //-----
    // Now make the triangle strips.
    //-----
    for (j = 0; j < num; j++)
    {
      int j1num;
      if (cyl_mapping) j1num = j+1;
      else j1num = (j+1)%num;
      csTriangle& tri1 = triangles.GetExtend (num_triangles);
      tri1.c = prev_verticesT[j];
      tri1.b = new_verticesT[j1num];
      tri1.a = new_verticesT[j];
      num_triangles++;
      csTriangle& tri2 = triangles.GetExtend (num_triangles);
      tri2.c = prev_verticesT[j];
      tri2.b = prev_verticesT[j1num];
      tri2.a = new_verticesT[j1num];
      num_triangles++;

      if (!toponly)
      {
        csTriangle& tri3 = triangles.GetExtend (num_triangles);
        tri3.a = prev_verticesB[j];
        tri3.b = new_verticesB[j1num];
        tri3.c = new_verticesB[j];
        num_triangles++;
        csTriangle& tri4 = triangles.GetExtend (num_triangles);
        tri4.a = prev_verticesB[j];
        tri4.b = prev_verticesB[j1num];
        tri4.c = new_verticesB[j1num];
        num_triangles++;
      }
    }

    //-----
    // Copy the new vertex array to prev_vertices.
    //-----
    for (j = 0 ; j < num2 ; j++)
    {
      prev_verticesT.GetExtend (j) = new_verticesT[j];
      if (!toponly) prev_verticesB.GetExtend (j) = new_verticesB[j];
    }
  }

  // Create the top and bottom vertices.
  int top_vertex = num_vertices;
  vertices.GetExtend (num_vertices).Set (0.0f, vert_radius, 0.0f);
  if (!mapper)
  {
    if (cyl_mapping)
      uvverts.GetExtend (num_vertices).Set (0.5f, 0.0f);
    else
      uvverts.GetExtend (num_vertices).Set (0.5f, 0.5f);
  }
  num_vertices++;
  int bottom_vertex = 0;

  if (!toponly)
  {
    bottom_vertex = num_vertices;
    vertices.GetExtend (num_vertices).Set (0.0f, -vert_radius, 0.0f);
    if (!mapper)
    {
      if (cyl_mapping)
        uvverts.GetExtend (num_vertices).Set (0.5f, 1.0f);
      else
        uvverts.GetExtend (num_vertices).Set (0.5f, 0.5f);
    }
    num_vertices++;
  }


  //-----
  // Make the top triangle fan.
  //-----
  for (j = 0 ; j < num ; j++)
  {
    int j1num;
    if (cyl_mapping) j1num = j+1;
    else j1num = (j+1)%num;
    csTriangle& tri = triangles.GetExtend (num_triangles);
    tri.c = top_vertex;
    tri.b = prev_verticesT[j];
    tri.a = prev_verticesT[j1num];
    num_triangles++;
  }

  //-----
  // Make the bottom triangle fan.
  //-----

  if (!toponly)
    for (j = 0 ; j < num ; j++)
    {
      int j1num;
      if (cyl_mapping) j1num = j+1;
      else j1num = (j+1)%num;
      csTriangle& tri = triangles.GetExtend (num_triangles);
      tri.a = bottom_vertex;
      tri.b = prev_verticesB[j];
      tri.c = prev_verticesB[j1num];
      num_triangles++;
    }

  // Scale and shift all the vertices.
  mesh_normals.SetSize (num_vertices);
  const csVector3& sphere_radius = ellips.GetRadius ();
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i].x *= sphere_radius.x;
    vertices[i].y *= sphere_radius.y;
    vertices[i].z *= sphere_radius.z;
    mesh_normals[i] = vertices[i].Unit ();
    vertices[i] += ellips.GetCenter ();
  }

  // Swap all triangles if needed.
  if (reversed)
  {
    for (i = 0 ; i < num_triangles ; i++)
    {
      int s = triangles[i].a;
      triangles[i].a = triangles[i].c;
      triangles[i].c = s;
    }
  }

  mesh_vertices.SetSize (num_vertices);
  mesh_texels.SetSize (num_vertices);
  csVector3* genmesh_vertices = mesh_vertices.GetArray ();
  memcpy (genmesh_vertices, vertices.GetArray (),
      sizeof(csVector3)*num_vertices);

  csVector2* genmesh_texels = mesh_texels.GetArray ();
  if (mapper)
  {
    for (i = 0 ; i < num_vertices ; i++)
      genmesh_texels[i] = mapper->Map (mesh_vertices[i], mesh_normals[i], i);
  }
  else
  {
    memcpy (genmesh_texels, uvverts.GetArray (),
      sizeof(csVector2)*num_vertices);
  }

  mesh_triangles.SetSize (num_triangles);
  csTriangle* ball_triangles = mesh_triangles.GetArray ();
  memcpy (ball_triangles, triangles.GetArray (),
      sizeof(csTriangle)*num_triangles);
}

} // namespace Geometry
} // namespace CS

//---------------------------------------------------------------------------
