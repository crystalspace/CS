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
#include "csgeom/sphere.h"
#include "csgeom/trimesh.h"
#include "cstool/primitives.h"


void csPrimitives::GenerateBox (
      const csBox3& box,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles)
{
  mesh_vertices.SetLength (24);
  mesh_texels.SetLength (24);
  mesh_normals.SetLength (24);
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

  csVector2* texels = mesh_texels.GetArray ();
  // the comments indicate which face
  // (numbered 1-6) the texel applies to
  texels[0].Set(0, 0); // 1
  texels[1].Set(0, 1); // 2
  texels[2].Set(1, 0); // 3

  texels[3].Set(0, 0); // 2
  texels[4].Set(0, 0); // 3
  texels[5].Set(1, 0); // 4

  texels[6].Set(1, 0); // 2
  texels[7].Set(0, 0); // 4
  texels[8].Set(1, 0); // 5

  texels[9].Set(1, 0); // 1
  texels[10].Set(1, 1); // 2
  texels[11].Set(0, 0); // 5

  texels[12].Set(0, 1); // 3
  texels[13].Set(1, 1); // 4
  texels[14].Set(1, 1); // 6

  texels[15].Set(0, 1); // 4
  texels[16].Set(1, 1); // 5
  texels[17].Set(1, 0); // 6

  texels[18].Set(1, 1); // 1
  texels[19].Set(0, 1); // 5
  texels[20].Set(0, 0); // 6

  texels[21].Set(0, 1); // 1
  texels[22].Set(1, 1); // 3
  texels[23].Set(0, 1); // 6

  mesh_triangles.SetLength (12);
  csTriangle* triangles = mesh_triangles.GetArray ();
  triangles[0].a = 0; triangles[0].b = 9; triangles[0].c = 18;
  triangles[1].a = 0; triangles[1].b = 18; triangles[1].c = 21;

  triangles[2].a = 3; triangles[2].b = 6; triangles[2].c = 10;
  triangles[3].a = 3; triangles[3].b = 10; triangles[3].c = 1;

  triangles[4].a = 4; triangles[4].b = 2; triangles[4].c = 22;
  triangles[5].a = 4; triangles[5].b = 22; triangles[5].c = 12;

  triangles[6].a = 7; triangles[6].b = 5; triangles[6].c = 13;
  triangles[7].a = 7; triangles[7].b = 13; triangles[7].c = 15;

  triangles[8].a = 11; triangles[8].b = 8; triangles[8].c = 16;
  triangles[9].a = 11; triangles[9].b = 16; triangles[9].c = 19;

  triangles[10].a = 23; triangles[10].b = 20; triangles[10].c = 17;
  triangles[11].a = 23; triangles[11].b = 17; triangles[11].c = 14;

  csVector3* normals = mesh_normals.GetArray ();
  normals[0].Set(box.MinX(),box.MaxY(),box.MinZ()); normals[0].Normalize();
  normals[1].Set(box.MinX(),box.MaxY(),box.MinZ()); normals[1].Normalize();
  normals[2].Set(box.MinX(),box.MaxY(),box.MinZ()); normals[2].Normalize();

  normals[3].Set(box.MinX(),box.MaxY(),box.MaxZ()); normals[3].Normalize();
  normals[4].Set(box.MinX(),box.MaxY(),box.MaxZ()); normals[4].Normalize();
  normals[5].Set(box.MinX(),box.MaxY(),box.MaxZ()); normals[5].Normalize();

  normals[6].Set(box.MaxX(),box.MaxY(),box.MaxZ()); normals[6].Normalize();
  normals[7].Set(box.MaxX(),box.MaxY(),box.MaxZ()); normals[7].Normalize();
  normals[8].Set(box.MaxX(),box.MaxY(),box.MaxZ()); normals[8].Normalize();

  normals[9].Set(box.MaxX(),box.MaxY(),box.MinZ()); normals[9].Normalize();
  normals[10].Set(box.MaxX(),box.MaxY(),box.MinZ()); normals[10].Normalize();
  normals[11].Set(box.MaxX(),box.MaxY(),box.MinZ()); normals[11].Normalize();

  normals[12].Set(box.MinX(),box.MinY(),box.MaxZ()); normals[12].Normalize();
  normals[13].Set(box.MinX(),box.MinY(),box.MaxZ()); normals[13].Normalize();
  normals[14].Set(box.MinX(),box.MinY(),box.MaxZ()); normals[14].Normalize();

  normals[15].Set(box.MaxX(),box.MinY(),box.MaxZ()); normals[15].Normalize();
  normals[16].Set(box.MaxX(),box.MinY(),box.MaxZ()); normals[16].Normalize();
  normals[17].Set(box.MaxX(),box.MinY(),box.MaxZ()); normals[17].Normalize();

  normals[18].Set(box.MaxX(),box.MinY(),box.MinZ()); normals[18].Normalize();
  normals[19].Set(box.MaxX(),box.MinY(),box.MinZ()); normals[19].Normalize();
  normals[20].Set(box.MaxX(),box.MinY(),box.MinZ()); normals[20].Normalize();

  normals[21].Set(box.MinX(),box.MinY(),box.MinZ()); normals[21].Normalize();
  normals[22].Set(box.MinX(),box.MinY(),box.MinZ()); normals[22].Normalize();
  normals[23].Set(box.MinX(),box.MinY(),box.MinZ()); normals[23].Normalize();
}

void csPrimitives::GenerateQuad (const csVector3 &v1, const csVector3 &v2,
                          const csVector3 &v3, const csVector3 &v4,
                          csDirtyAccessArray<csVector3>& mesh_vertices,
                          csDirtyAccessArray<csVector2>& mesh_texels,
                          csDirtyAccessArray<csVector3>& mesh_normals,
                          csDirtyAccessArray<csTriangle>& mesh_triangles)
{
  mesh_vertices.SetLength (4);
  mesh_texels.SetLength (4);
  mesh_normals.SetLength (4);
  mesh_triangles.SetLength (4);

  mesh_normals[0] = mesh_vertices[0] = v1;
  mesh_normals[1] = mesh_vertices[1] = v2;
  mesh_normals[2] = mesh_vertices[2] = v3;
  mesh_normals[3] = mesh_vertices[3] = v4;

  mesh_normals[0].Normalize ();
  mesh_normals[1].Normalize ();
  mesh_normals[2].Normalize ();
  mesh_normals[3].Normalize ();

  mesh_texels[0] = csVector2 (0,0);
  mesh_texels[1] = csVector2 (0,1);
  mesh_texels[2] = csVector2 (1,1);
  mesh_texels[3] = csVector2 (1,0);

  mesh_triangles[0].a = 3; mesh_triangles[0].b = 0; mesh_triangles[0].c = 1;
  mesh_triangles[1].a = 0; mesh_triangles[1].b = 1; mesh_triangles[1].c = 2;
  mesh_triangles[2].a = 1; mesh_triangles[2].b = 2; mesh_triangles[2].c = 3;
  mesh_triangles[3].a = 2; mesh_triangles[3].b = 3; mesh_triangles[3].c = 0;
}

void csPrimitives::GenerateSphere (const csEllipsoid& ellips, int num,
      csDirtyAccessArray<csVector3>& mesh_vertices,
      csDirtyAccessArray<csVector2>& mesh_texels,
      csDirtyAccessArray<csVector3>& mesh_normals,
      csDirtyAccessArray<csTriangle>& mesh_triangles,
      bool cyl_mapping, bool toponly, bool reversed)
{
  int num_vertices = 0;
  int num_triangles = 0;
  csDirtyAccessArray<csVector3> vertices;
  csDirtyAccessArray<csVector2> uvverts;
  csDirtyAccessArray<csTriangle> triangles;
  float radius = 1.0f;

  csArray<int> prev_verticesT;
  csArray<int> prev_verticesB;
  float u, v;
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

      new_verticesT.GetExtend (j) = num_vertices;
      vertices.GetExtend (num_vertices).Set (new_radius * (float) cos (angle),
        new_height, new_radius * (float) sin (angle));
      uvverts.GetExtend (num_vertices).Set (u, v);
      num_vertices++;

      if (!toponly)
      {
        new_verticesB.GetExtend (j) = num_vertices;
        vertices.GetExtend (num_vertices).Set (new_radius * (float) cos (angle),
          -new_height, new_radius * (float) sin (angle));

        if (cyl_mapping) v = 1.0f - v;
        uvverts.GetExtend (num_vertices).Set (u, v);
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
  if (cyl_mapping)
    uvverts.GetExtend (num_vertices).Set (0.5f, 0.0f);
  else
    uvverts.GetExtend (num_vertices).Set (0.5f, 0.5f);
  num_vertices++;
  int bottom_vertex = 0;

  if (!toponly)
  {
    bottom_vertex = num_vertices;
    vertices.GetExtend (num_vertices).Set (0.0f, -vert_radius, 0.0f);
    if (cyl_mapping)
      uvverts.GetExtend (num_vertices).Set (0.5f, 1.0f);
    else
      uvverts.GetExtend (num_vertices).Set (0.5f, 0.5f);
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
  mesh_normals.SetLength (num_vertices);
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

  mesh_vertices.SetLength (num_vertices);
  mesh_texels.SetLength (num_vertices);
  csVector3* genmesh_vertices = mesh_vertices.GetArray ();
  memcpy (genmesh_vertices, vertices.GetArray (),
      sizeof(csVector3)*num_vertices);

  csVector2* genmesh_texels = mesh_texels.GetArray ();
  memcpy (genmesh_texels, uvverts.GetArray (),
      sizeof(csVector2)*num_vertices);

  mesh_triangles.SetLength (num_triangles);
  csTriangle* ball_triangles = mesh_triangles.GetArray ();
  memcpy (ball_triangles, triangles.GetArray (),
      sizeof(csTriangle)*num_triangles);
}

//---------------------------------------------------------------------------
