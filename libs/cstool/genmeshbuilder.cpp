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
#include "cstool/genmeshbuilder.h"
#include "imesh/genmesh.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"

namespace CS
{
namespace Geometry
{

csPtr<iMeshFactoryWrapper> GeneralMeshBuilder::CreateFactory (
	iEngine* engine, const char* name)
{
  return engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh", name);
}

csPtr<iMeshWrapper> GeneralMeshBuilder::CreateMesh (
	iEngine* engine, iSector* sector, const char* name,
	const char* factoryname)
{
  iMeshFactoryWrapper* factory = engine->FindMeshFactory (factoryname);
  if (!factory) return 0;

  csRef<iMeshWrapper> mesh = engine->CreateMeshWrapper (factory,
      name, sector, csVector3 (0));
  mesh->SetZBufMode (CS_ZBUF_USE);
  mesh->SetRenderPriority (engine->GetObjectRenderPriority ());
  return csPtr<iMeshWrapper> (mesh);
}

static void AppendOrSetData (iGeneralFactoryState* factory, bool append,
    const csDirtyAccessArray<csVector3>& mesh_vertices,
    const csDirtyAccessArray<csVector2>& mesh_texels,
    const csDirtyAccessArray<csVector3>& mesh_normals,
    const csDirtyAccessArray<csTriangle>& mesh_triangles)
{
  if (append)
  {
    csColor4 black (0, 0, 0);
    size_t cur_vt_count = factory->GetVertexCount ();
    size_t i;
    for (i = 0 ; i < mesh_vertices.GetSize () ; i++)
      factory->AddVertex (mesh_vertices[i], mesh_texels[i],
	  mesh_normals[i], black);
    for (i = 0 ; i < mesh_triangles.GetSize () ; i++)
    {
      csTriangle tri = mesh_triangles[i];
      tri.a += cur_vt_count;
      tri.b += cur_vt_count;
      tri.c += cur_vt_count;
      factory->AddTriangle (tri);
    }
  }
  else
  {
    factory->SetVertexCount (mesh_vertices.GetSize ());
    factory->SetTriangleCount (mesh_triangles.GetSize ());
    memcpy (factory->GetVertices (), mesh_vertices.GetArray (),
      sizeof (csVector3) * mesh_vertices.GetSize ());
    memcpy (factory->GetTexels (), mesh_texels.GetArray (),
      sizeof (csVector2) * mesh_texels.GetSize ());
    memcpy (factory->GetNormals (), mesh_normals.GetArray (),
      sizeof (csVector3) * mesh_normals.GetSize ());
    memcpy (factory->GetTriangles (), mesh_triangles.GetArray (),
      sizeof (csTriangle) * mesh_triangles.GetSize ());
  }
  factory->Invalidate ();
}

void GeneralMeshBuilder::Box (
      iGeneralFactoryState* factory, bool append,
      const csBox3& box, uint32 flags,
      TextureMapper* mapper)
{
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateBox (box,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, flags, mapper);
  AppendOrSetData (factory, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

static void SidedTesselatedQuad (
    bool in,
    iGeneralFactoryState* factory, bool append,
    const csVector3 &v0,
    const csVector3 &v1, const csVector3 &v2,
    int tesselations,
    TextureMapper* mapper)
{
  GeneralMeshBuilder::TesselatedQuad (factory, append,
      v0, in ? v1 : v2, in ? v2 : v1, tesselations, mapper);
}

void GeneralMeshBuilder::TesselatedBox (
      iGeneralFactoryState* factory, bool append,
      const csBox3& box,
      int tesselations,
      uint32 flags,
      TextureMapper* mapper)
{
  bool in = flags & Primitives::CS_PRIMBOX_INSIDE;
  SidedTesselatedQuad (in, factory, append,
	box.GetCorner (CS_BOX_CORNER_xYZ),
	box.GetCorner (CS_BOX_CORNER_XYZ),
	box.GetCorner (CS_BOX_CORNER_xyZ),
	tesselations, mapper);
  SidedTesselatedQuad (in, factory, true,
	box.GetCorner (CS_BOX_CORNER_XYZ),
	box.GetCorner (CS_BOX_CORNER_XYz),
	box.GetCorner (CS_BOX_CORNER_XyZ),
	tesselations, mapper);
  SidedTesselatedQuad (in, factory, true,
	box.GetCorner (CS_BOX_CORNER_xYz),
	box.GetCorner (CS_BOX_CORNER_xYZ),
	box.GetCorner (CS_BOX_CORNER_xyz),
	tesselations, mapper);
  SidedTesselatedQuad (in, factory, true,
	box.GetCorner (CS_BOX_CORNER_xYz),
	box.GetCorner (CS_BOX_CORNER_XYz),
	box.GetCorner (CS_BOX_CORNER_xYZ),
	tesselations, mapper);
  SidedTesselatedQuad (in, factory, true,
	box.GetCorner (CS_BOX_CORNER_xyZ),
	box.GetCorner (CS_BOX_CORNER_XyZ),
	box.GetCorner (CS_BOX_CORNER_xyz),
	tesselations, mapper);
  SidedTesselatedQuad (in, factory, true,
	box.GetCorner (CS_BOX_CORNER_XYz),
	box.GetCorner (CS_BOX_CORNER_xYz),
	box.GetCorner (CS_BOX_CORNER_Xyz),
	tesselations, mapper);
}

void GeneralMeshBuilder::Capsule (
      iGeneralFactoryState* factory, bool append,
      float l, float r, uint sides,
      TextureMapper* mapper)
{
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateCapsule (l, r, sides,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, mapper);
  AppendOrSetData (factory, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

void GeneralMeshBuilder::Quad (
    iGeneralFactoryState* factory, bool append,
    const csVector3 &v1, const csVector3 &v2,
    const csVector3 &v3, const csVector3 &v4,
    TextureMapper* mapper)
{
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateQuad (v1, v2, v3, v4,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, mapper);
  AppendOrSetData (factory, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

void GeneralMeshBuilder::TesselatedQuad (
    iGeneralFactoryState* factory, bool append,
    const csVector3 &v0,
    const csVector3 &v1, const csVector3 &v2,
    int tesselations,
    TextureMapper* mapper)
{
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateTesselatedQuad (v0, v1, v2,
      tesselations,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, mapper);
  AppendOrSetData (factory, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

void GeneralMeshBuilder::Sphere (
      iGeneralFactoryState* factory, bool append,
      const csEllipsoid& ellips, int num,
      bool cyl_mapping, bool toponly, bool reversed,
      TextureMapper* mapper)
{
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateSphere (ellips, num,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, cyl_mapping, toponly, reversed,
      mapper);
  AppendOrSetData (factory, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

} // namespace Geometry
} // namespace CS

//---------------------------------------------------------------------------
