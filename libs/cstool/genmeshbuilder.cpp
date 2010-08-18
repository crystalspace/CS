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
 
//---------------------------------------------------------------------------

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
      tri.a += int (cur_vt_count);
      tri.b += int (cur_vt_count);
      tri.c += int (cur_vt_count);
      factory->AddTriangle (tri);
    }
  }
  else
  {
    factory->SetVertexCount (int (mesh_vertices.GetSize ()));
    factory->SetTriangleCount (int (mesh_triangles.GetSize ()));
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

//---------------------------------------------------------------------------

TesselatedQuad::TesselatedQuad (const csVector3& v0,
    const csVector3& v1, const csVector3& v2)
{
  TesselatedQuad::v0 = v0;
  TesselatedQuad::v1 = v1;
  TesselatedQuad::v2 = v2;
  tesselations = 1;
  mapper = 0;
}

void TesselatedQuad::Append (iGeneralFactoryState* state)
{
  bool append = state->GetVertexCount () > 0 || state->GetTriangleCount () > 0;
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateTesselatedQuad (v0, v1, v2,
      tesselations,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, mapper);
  AppendOrSetData (state, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

//---------------------------------------------------------------------------

void Box::Init (const csBox3& box)
{
  Box::box = box;
  mapper = 0;
  flags = 0;
}

void Box::Append (iGeneralFactoryState* state)
{
  bool append = state->GetVertexCount () > 0 || state->GetTriangleCount () > 0;
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateBox (box,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, flags, mapper);
  AppendOrSetData (state, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

//---------------------------------------------------------------------------

void TesselatedBox::Init (const csBox3& box)
{
  TesselatedBox::box = box;
  tesselations = 1;
  mapper = 0;
  flags = 0;
}

static void SidedTesselatedQuad (
    bool in,
    iGeneralFactoryState* factory,
    const csVector3 &v0,
    const csVector3 &v1, const csVector3 &v2,
    int tesselations,
    TextureMapper* mapper)
{
  TesselatedQuad quad (v0, in ? v1 : v2, in ? v2 : v1);
  quad.SetLevel (tesselations);
  quad.SetMapper (mapper);
  quad.Append (factory);
}

void TesselatedBox::Append (iGeneralFactoryState* state)
{
  bool in = flags & Primitives::CS_PRIMBOX_INSIDE;
  SidedTesselatedQuad (in, state,
	box.GetCorner (CS_BOX_CORNER_xYZ),
	box.GetCorner (CS_BOX_CORNER_XYZ),
	box.GetCorner (CS_BOX_CORNER_xyZ),
	tesselations, mapper);
  SidedTesselatedQuad (in, state,
	box.GetCorner (CS_BOX_CORNER_XYZ),
	box.GetCorner (CS_BOX_CORNER_XYz),
	box.GetCorner (CS_BOX_CORNER_XyZ),
	tesselations, mapper);
  SidedTesselatedQuad (in, state,
	box.GetCorner (CS_BOX_CORNER_xYz),
	box.GetCorner (CS_BOX_CORNER_xYZ),
	box.GetCorner (CS_BOX_CORNER_xyz),
	tesselations, mapper);
  SidedTesselatedQuad (in, state,
	box.GetCorner (CS_BOX_CORNER_xYz),
	box.GetCorner (CS_BOX_CORNER_XYz),
	box.GetCorner (CS_BOX_CORNER_xYZ),
	tesselations, mapper);
  SidedTesselatedQuad (in, state,
	box.GetCorner (CS_BOX_CORNER_xyZ),
	box.GetCorner (CS_BOX_CORNER_XyZ),
	box.GetCorner (CS_BOX_CORNER_xyz),
	tesselations, mapper);
  SidedTesselatedQuad (in, state,
	box.GetCorner (CS_BOX_CORNER_XYz),
	box.GetCorner (CS_BOX_CORNER_xYz),
	box.GetCorner (CS_BOX_CORNER_Xyz),
	tesselations, mapper);
}

//---------------------------------------------------------------------------

Capsule::Capsule (float l, float r, uint sides)
{
  Capsule::l = l;
  Capsule::r = r;
  Capsule::sides = sides;
  mapper = 0;
}

void Capsule::Append (iGeneralFactoryState* state)
{
  bool append = state->GetVertexCount () > 0 || state->GetTriangleCount () > 0;
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateCapsule (l, r, sides,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, mapper);
  AppendOrSetData (state, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

//---------------------------------------------------------------------------

Sphere::Sphere (const csEllipsoid& ellips, int num)
{
  Sphere::ellips = ellips;
  Sphere::num = num;
  mapper = 0;
  cyl_mapping = false;
  toponly = false;
  reversed = false;
}

void Sphere::Append (iGeneralFactoryState* state)
{
  bool append = state->GetVertexCount () > 0 || state->GetTriangleCount () > 0;
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateSphere (ellips, num,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, cyl_mapping, toponly, reversed,
      mapper);
  AppendOrSetData (state, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

//---------------------------------------------------------------------------

Cone::Cone (float l, float r, uint sides)
{
  Cone::l = l;
  Cone::r = r;
  Cone::sides = sides;
  mapper = 0;
}

void Cone::Append (iGeneralFactoryState* state)
{
  bool append = state->GetVertexCount () > 0 || state->GetTriangleCount () > 0;
  csDirtyAccessArray<csVector3> mesh_vertices;
  csDirtyAccessArray<csVector2> mesh_texels;
  csDirtyAccessArray<csVector3> mesh_normals;
  csDirtyAccessArray<csTriangle> mesh_triangles;
  Primitives::GenerateCone (l, r, sides,
      mesh_vertices, mesh_texels, mesh_normals,
      mesh_triangles, mapper);
  AppendOrSetData (state, append, mesh_vertices, mesh_texels,
      mesh_normals, mesh_triangles);
}

//---------------------------------------------------------------------------

csPtr<iMeshFactoryWrapper> GeneralMeshBuilder::CreateFactory (
	iEngine* engine, const char* name, Primitive* primitive)
{
  csRef<iMeshFactoryWrapper> factory = engine->CreateMeshFactory (
      "crystalspace.mesh.object.genmesh", name);
  if (primitive) primitive->Append (factory);
  return (csPtr<iMeshFactoryWrapper>)factory;
}

csPtr<iMeshWrapper> GeneralMeshBuilder::CreateMesh (
	iEngine* engine, iSector* sector, const char* name,
	iMeshFactoryWrapper* factory)
{
  csRef<iMeshWrapper> mesh = engine->CreateMeshWrapper (factory,
      name, sector, csVector3 (0));
  mesh->SetZBufMode (CS_ZBUF_USE);
  mesh->SetRenderPriority (engine->GetObjectRenderPriority ());
  return csPtr<iMeshWrapper> (mesh);
}

csPtr<iMeshWrapper> GeneralMeshBuilder::CreateMesh (
	iEngine* engine, iSector* sector, const char* name,
	const char* factoryname)
{
  iMeshFactoryWrapper* factory = engine->FindMeshFactory (factoryname);
  if (!factory) return 0;
  return CreateMesh (engine, sector, name, factory);
}

csPtr<iMeshWrapper> GeneralMeshBuilder::CreateFactoryAndMesh (
    iEngine* engine, iSector* sector,
    const char* name, const char* factoryname, Primitive* primitive)
{
  csRef<iMeshFactoryWrapper> fact = CreateFactory (engine, factoryname,
      primitive);

  csRef<iMeshWrapper> mesh = engine->CreateMeshWrapper (fact,
      name, sector, csVector3 (0));
  mesh->SetZBufMode (CS_ZBUF_USE);
  mesh->SetRenderPriority (engine->GetObjectRenderPriority ());
  return csPtr<iMeshWrapper> (mesh);
}

} // namespace Geometry
} // namespace CS

//---------------------------------------------------------------------------
