/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csutil/csendian.h"
#include "polygon.h"
#include "polytext.h"
#include "lghtmap.h"
#include "lppool.h"
#include "thing.h"
#include "csgeom/frustum.h"
#include "csgeom/textrans.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/debug.h"
#include "csutil/memfile.h"
#include "iutil/vfs.h"
#include "iutil/cache.h"
#include "csgeom/matrix2.h"
#include "csgeom/poly3d.h"
#include "csqint.h"
#include "csqsqrt.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/light.h"
#include "iengine/shadows.h"
#include "iengine/movable.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/txtmgr.h"
#include "ivideo/graph3d.h"


// This is a static vector array which is adapted to the
// right size everytime it is used. In the beginning it means
// that this array will grow a lot but finally it will
// stabilize to a maximum size (not big). The advantage of
// this approach is that we don't have a static array which can
// overflow. And we don't have to do allocation every time we
// come here. We do an IncRef on this object each time a new
// csPolygon3D is created and an DecRef each time it is deleted.
// Thus, when the engine is cleaned, the array is automatically
// cleaned too.

typedef csDirtyAccessArray<csVector3> engine3d_VectorArray;
CS_IMPLEMENT_STATIC_VAR (GetStaticVectorArray, engine3d_VectorArray,())

static engine3d_VectorArray *VectorArray = 0;

//---------------------------------------------------------------------------

csPolygon3DStatic::csPolygon3DStatic ()
{
  VectorArray = GetStaticVectorArray();
  thing_static = 0;

  material = 0;
  name = 0;

  polygon_data.tmapping = 0;
  polygon_data.num_vertices = 0;
  polygon_data.vertices = 0;
  polygon_data.useLightmap = false;
  polygon_data.objNormals = 0;

  flags.SetAll (CS_POLY_LIGHTING | CS_POLY_COLLDET | CS_POLY_VISCULL);

  VectorArray->IncRef ();
}

csPolygon3DStatic::~csPolygon3DStatic ()
{
  SetNumVertices (0);	// Clear vertices.
  delete[] name;

  thing_static->thing_type->blk_texturemapping.Free (polygon_data.tmapping);

  VectorArray->DecRef ();
}

void csPolygon3DStatic::SetNumVertices (int count)
{
  int old_count = polygon_data.num_vertices;
  polygon_data.num_vertices = count;
  if (old_count == count) return;
  if (old_count >= 1 && old_count <= 3 && count >= 1 && count <= 3) return;
  if (old_count >= 7 && old_count <= 20 && count >= 7 && count <= 20) return;
  if (old_count >= 21 && count >= 21) return;

  int* old_data = polygon_data.vertices;
  csThingObjectType* t = thing_static->thing_type;
  switch (count)
  {
    case 0:
      polygon_data.vertices = 0;
      break;
    case 1:
    case 2:
    case 3:
      polygon_data.vertices = t->blk_polidx3.Alloc ()->ar;
      break;
    case 4:
      polygon_data.vertices = t->blk_polidx4.Alloc ()->ar;
      break;
    case 5:
      if (!t->blk_polidx5)
        t->blk_polidx5 = new csBlockAllocator<intar5> (500);
      polygon_data.vertices = t->blk_polidx5->Alloc ()->ar;
      break;
    case 6:
      if (!t->blk_polidx6)
        t->blk_polidx6 = new csBlockAllocator<intar6> (500);
      polygon_data.vertices = t->blk_polidx6->Alloc ()->ar;
      break;
    default:
      if (count <= 20)
      {
        if (!t->blk_polidx20)
          t->blk_polidx20 = new csBlockAllocator<intar20> (300);
        polygon_data.vertices = t->blk_polidx20->Alloc ()->ar;
      }
      else
      {
        if (!t->blk_polidx60)
          t->blk_polidx60 = new csBlockAllocator<intar60> (300);
        polygon_data.vertices = t->blk_polidx60->Alloc ()->ar;
      }
      break;
  }

  if (old_data)
  {
    int minsize = MIN (count, old_count);
    memcpy (polygon_data.vertices, old_data, sizeof (int) * minsize);
    switch (old_count)
    {
      case 1:
      case 2:
      case 3:
        t->blk_polidx3.Free ((intar3*)old_data);
	break;
      case 4: t->blk_polidx4.Free ((intar4*)old_data); break;
      case 5: t->blk_polidx5->Free ((intar5*)old_data); break;
      case 6: t->blk_polidx6->Free ((intar6*)old_data); break;
      default:
        if (old_count <= 20)
          t->blk_polidx20->Free ((intar20*)old_data);
	else
          t->blk_polidx60->Free ((intar60*)old_data);
	break;
    }
  }
}

csPolygon3DStatic* csPolygon3DStatic::Clone (csThingStatic* new_parent)
{
  csThingObjectType* t = thing_static->thing_type;
  csPolygon3DStatic* clone = t->blk_polygon3dstatic.Alloc ();
  clone->SetParent (new_parent);
  clone->SetMaterial (material);
  clone->SetName (name);
  clone->SetNumVertices (polygon_data.num_vertices);
  memcpy (clone->polygon_data.vertices, polygon_data.vertices,
  	sizeof (int) * polygon_data.num_vertices);

  clone->polygon_data.plane_obj = polygon_data.plane_obj;
  if (polygon_data.tmapping)
  {
    clone->polygon_data.tmapping = t->blk_texturemapping.Alloc ();
    *clone->polygon_data.tmapping = *polygon_data.tmapping;
  }
  else
  {
    clone->polygon_data.tmapping = 0;
  }
  clone->flags.SetAll (flags.Get ());

  return clone;
}

void csPolygon3DStatic::MappingSetTextureSpace (
  const csPlane3 &plane_wor,
  const csVector3 &v_orig,
  const csVector3 &v1,
  float len)
{
  MappingSetTextureSpace (
    plane_wor,
    v_orig.x, v_orig.y, v_orig.z,
    v1.x, v1.y, v1.z,
    len);
}

void csPolygon3DStatic::MappingSetTextureSpace (
  const csPlane3 &plane_wor,
  float xo,
  float yo,
  float zo,
  float x1,
  float y1,
  float z1,
  float len1)
{
  float A = plane_wor.A ();
  float B = plane_wor.B ();
  float C = plane_wor.C ();
  csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      xo, yo, zo,
      x1, y1, z1,
      len1,
      A, B, C);
  thing_static->scfiObjectModel.ShapeChanged ();
}

void csPolygon3DStatic::MappingSetTextureSpace (
  const csVector3 &v_orig,
  const csVector3 &v1,
  float len1,
  const csVector3 &v2,
  float len2)
{
  csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      v_orig, v1,
      len1, v2,
      len2);
  thing_static->scfiObjectModel.ShapeChanged ();
}

void csPolygon3DStatic::MappingSetTextureSpace (
  const csVector3 &v_orig,
  const csVector3 &v_u,
  const csVector3 &v_v)
{
  csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      v_orig,
      v_u,
      v_v);
  thing_static->scfiObjectModel.ShapeChanged ();
}

void csPolygon3DStatic::MappingSetTextureSpace (
  float xo, float yo, float zo,
  float xu, float yu, float zu,
  float xv, float yv, float zv)
{
  const csVector3 o (xo, yo, zo);
  const csVector3 u (xu, yu, zu);
  const csVector3 v (xv, yv, zv);
  csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      o, u, v);
  thing_static->scfiObjectModel.ShapeChanged ();
}

void csPolygon3DStatic::MappingSetTextureSpace (
  float xo, float yo, float zo,
  float xu, float yu, float zu,
  float xv, float yv, float zv,
  float xw, float yw, float zw)
{
  csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      xo, yo, zo,
      xu, yu, zu,
      xv, yv, zv,
      xw, yw, zw);
  thing_static->scfiObjectModel.ShapeChanged ();
}

void csPolygon3DStatic::MappingSetTextureSpace (
  const csMatrix3 &tx_matrix,
  const csVector3 &tx_vector)
{
  polygon_data.tmapping->SetO2T (tx_matrix),
  polygon_data.tmapping->SetO2TTranslation (tx_vector),
  thing_static->scfiObjectModel.ShapeChanged ();
}

void csPolygon3DStatic::SetParent (csThingStatic *thing_static)
{
  csPolygon3DStatic::thing_static = thing_static;
  if (thing_static)
  {
    polygon_data.p_obj_verts = &thing_static->obj_verts;
    polygon_data.objNormals = &thing_static->obj_normals;
  }
}

void csPolygon3DStatic::EnableTextureMapping (bool enable)
{
  if (enable && polygon_data.tmapping != 0) return;
  if (!enable && polygon_data.tmapping == 0) return;

  if (thing_static) thing_static->scfiObjectModel.ShapeChanged ();
  if (enable)
  {
    polygon_data.tmapping = thing_static->thing_type
    	->blk_texturemapping.Alloc ();
  }
  else
  {
    thing_static->thing_type->blk_texturemapping.Free (polygon_data.tmapping);
    polygon_data.tmapping = 0;
  }
}

void csPolygon3DStatic::Reset ()
{
  SetNumVertices (0);
}

bool csPolygon3DStatic::Overlaps (csPolygon3DStatic *overlapped)
{
  csPolygon3DStatic *totest = overlapped;

  // Algorithm: if any of the vertices of the 'totest' polygon
  // is facing away from the front of this polygon (i.e. the vertex
  // cannot see this polygon) then there is a chance that this polygon
  // overlaps the other. If this is not the case then we can return false
  // already. Otherwise we have to see that the 'totest' polygon is
  // itself not facing away from this polygon. To test that we see if
  // there is a vertex of this polygon that is in front of the 'totest'
  // polygon. If that is the case then we return true.
  csPlane3 &this_plane = polygon_data.plane_obj;
  csPlane3 &test_plane = totest->polygon_data.plane_obj;
  int i;
  for (i = 0; i < totest->polygon_data.num_vertices; i++)
  {
    if (this_plane.Classify (totest->Vobj (i)) >= SMALL_EPSILON)
    {
      int j;
      for (j = 0; j < polygon_data.num_vertices; j++)
      {
        if (test_plane.Classify (Vobj (j)) <= SMALL_EPSILON)
        {
          return true;
        }
      }

      return false;
    }
  }

  return false;
}

const csVector3& csPolygon3DStatic::Vobj (int idx) const
{
  return thing_static->Vobj (polygon_data.vertices[idx]);
}

void csPolygon3DStatic::SetMaterial (iMaterialWrapper *material)
{
  csPolygon3DStatic::material = material;
}

iMaterialHandle *csPolygon3DStatic::GetMaterialHandle ()
{
  return material ? material->GetMaterialHandle () : 0;
}

bool csPolygon3DStatic::IsTransparent ()
{
  iTextureHandle *txt_handle = GetMaterialHandle ()->GetTexture ();
  return txt_handle && ((txt_handle->GetAlphaMap ()
    || txt_handle->GetKeyColor ()));
}

int csPolygon3DStatic::Classify (const csPlane3 &pl)
{
  if (&GetObjectPlane () == &pl) return CS_POL_SAME_PLANE;
  if (csMath3::PlanesEqual (pl, GetObjectPlane ())) return CS_POL_SAME_PLANE;

  int i;
  int front = 0, back = 0;

  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    float dot = pl.Classify (Vobj (i));
    if (ABS (dot) < EPSILON) dot = 0;
    if (dot > 0)
      back++;
    else if (dot < 0)
      front++;
  }

  if (back == 0) return CS_POL_FRONT;
  if (front == 0) return CS_POL_BACK;
  return CS_POL_SPLIT_NEEDED;
}

int csPolygon3DStatic::ClassifyX (float x)
{
  int i;
  int front = 0, back = 0;

  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    float xx = Vobj (i).x - x;
    if (xx < -EPSILON)
      front++;
    else if (xx > EPSILON)
      back++;
  }

  if (back == 0 && front == 0) return CS_POL_SAME_PLANE;
  if (back == 0) return CS_POL_FRONT;
  if (front == 0) return CS_POL_BACK;
  return CS_POL_SPLIT_NEEDED;
}

int csPolygon3DStatic::ClassifyY (float y)
{
  int i;
  int front = 0, back = 0;

  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    float yy = Vobj (i).y - y;
    if (yy < -EPSILON)
      front++;
    else if (yy > EPSILON)
      back++;
  }

  if (back == 0 && front == 0) return CS_POL_SAME_PLANE;
  if (back == 0) return CS_POL_FRONT;
  if (front == 0) return CS_POL_BACK;
  return CS_POL_SPLIT_NEEDED;
}

int csPolygon3DStatic::ClassifyZ (float z)
{
  int i;
  int front = 0, back = 0;

  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    float zz = Vobj (i).z - z;
    if (zz < -EPSILON)
      front++;
    else if (zz > EPSILON)
      back++;
  }

  if (back == 0 && front == 0) return CS_POL_SAME_PLANE;
  if (back == 0) return CS_POL_FRONT;
  if (front == 0) return CS_POL_BACK;
  return CS_POL_SPLIT_NEEDED;
}

void csPolygon3DStatic::ComputeNormal ()
{
  float A, B, C, D;
  PlaneNormal (&A, &B, &C);
  D = -A * Vobj (0).x - B * Vobj (0).y - C * Vobj (0).z;

  // By default the world space normal is equal to the object space normal.
  polygon_data.plane_obj.Set (A, B, C, D);
  thing_static->scfiObjectModel.ShapeChanged ();
}

void csPolygon3DStatic::HardTransform (const csReversibleTransform &t)
{
  csPlane3 new_plane;
  t.This2Other (GetObjectPlane (), Vobj (0), new_plane);
  SetObjectPlane (new_plane);
  thing_static->scfiObjectModel.ShapeChanged ();
  if (polygon_data.tmapping)
  {
    polygon_data.tmapping->GetO2T () *= t.GetO2T ();
    polygon_data.tmapping->GetO2TTranslation () = t.This2Other (
    	polygon_data.tmapping->GetO2TTranslation ());
  }
}

bool csPolygon3DStatic::CreateBoundingTextureBox ()
{
  bool rc = true;
  // First we compute the bounding box in 2D texture space (uv-space).
  float min_u = 1000000000.;
  float min_v = 1000000000.;
  float max_u = -1000000000.;
  float max_v = -1000000000.;

  int i;
  csVector3 v1, v2;
  const csVector3& v_obj2tex = polygon_data.tmapping->GetO2TTranslation ();
  const csMatrix3& m_obj2tex = polygon_data.tmapping->GetO2T ();
  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    v1 = Vobj (i);           // Coordinates of vertex in object space.
    v1 -= v_obj2tex;
    v2 = (m_obj2tex) * v1;   // Coordinates of vertex in texture space.
    if (v2.x < min_u) min_u = v2.x;
    if (v2.x > max_u) max_u = v2.x;
    if (v2.y < min_v) min_v = v2.y;
    if (v2.y > max_v) max_v = v2.y;
  }

  // used in hardware accel drivers
  polygon_data.tmapping->SetTextureBox (min_u, min_v, max_u, max_v);

  int ww, hh;
  iMaterialHandle* mat_handle = GetMaterialHandle ();
  if (mat_handle && mat_handle->GetTexture ())
  {
    rc = mat_handle->GetTexture ()->GetRendererDimensions (ww, hh);
    if (!rc) ww = hh = 64;
  }
  else
  {
    ww = hh = 128;
    rc = true;
  }

#define csQroundSure(x) (int ((x) + ((x < 0) ? -0.5 : +0.5)))
#define csQroundVerySure(v,x) \
  { \
    int v1 = csQroundSure((x)-0.001);\
    int v2 = csQroundSure((x)+0.001);\
    if (v1 == v2) v = csQroundSure(x); \
    else  v = csQroundSure((x)-0.2); \
  }

  float min_u_ww = min_u * ww;
  float min_v_hh = min_v * hh;
  float max_u_ww = max_u * ww;
  float max_v_hh = max_v * hh;
  int Imin_u, Imin_v, Imax_u, Imax_v;
  csQroundVerySure (Imin_u, min_u_ww);
  csQroundVerySure (Imin_v, min_v_hh);
  polygon_data.tmapping->SetIMinUV (Imin_u, Imin_v);
  csQroundVerySure (Imax_u, max_u_ww);
  csQroundVerySure (Imax_v, max_v_hh);
//csPrintf ("min_u=%g ww=%d min_u_ww=%g\n", min_u, ww, min_u_ww);
//csPrintf ("max_u=%g ww=%d max_u_ww=%g\n", max_u, ww, max_u_ww);
//csPrintf ("%d,%d,%d,%d\n", Imin_u, Imin_v, Imax_u, Imax_v); fflush (stdout);

  int h = Imax_v - polygon_data.tmapping->GetIMinV ();
  int w_orig = Imax_u - polygon_data.tmapping->GetIMinU ();
  int shf_u = 0;
  int w = 1;
  while (true)
  {
    if (w_orig <= w) break;
    w <<= 1;
    shf_u++;
  }
  polygon_data.tmapping->SetShiftU (shf_u);
  polygon_data.tmapping->SetLitWidth (w);
  polygon_data.tmapping->SetLitHeight (h);
  polygon_data.tmapping->SetLitOriginalWidth (w_orig);

  polygon_data.tmapping->SetFDUV (min_u * ww, min_v * hh);
  return rc;
}

#define TEXW(t) ((t)->w_orig)
#define TEXH(t) ((t)->h)

bool csPolygon3DStatic::Finish (iBase* thing_logparent)
{
  bool rc = true;

  if (thing_static->flags.Check (CS_ENTITY_NOLIGHTING))
    flags.Reset (CS_POLY_LIGHTING);

  if (IsTextureMappingEnabled ())
  {
    if (!material || (material->GetMaterialHandle () &&
      !material->GetMaterialHandle ()->GetTexture ()))
    {
      //EnableTextureMapping (false);
      //return true;
    }
    rc = CreateBoundingTextureBox ();
  }
  else
  {
    return true;
  }

  if (csThing::lightmap_enabled && flags.Check (CS_POLY_LIGHTING))
  {
    int lmw = csLightMap::CalcLightMapWidth (
    	polygon_data.tmapping->GetLitOriginalWidth ());
    int lmh = csLightMap::CalcLightMapHeight (
    	polygon_data.tmapping->GetLitHeight ());
    int max_lmw, max_lmh;
    thing_static->thing_type->engine->GetMaxLightmapSize (max_lmw, max_lmh);
    if ((lmw > max_lmw) || (lmh > max_lmh))
    {
      const char* mname = 0;
      if (thing_logparent)
      {
        csRef<iMeshWrapper> m = SCF_QUERY_INTERFACE (thing_logparent,
      	  iMeshWrapper);
        if (m) mname = m->QueryObject ()->GetName ();
        else mname = "<unknown>";
      }
      else mname = "<unknown>";
      thing_static->thing_type->Notify ("Oversize lightmap (%dx%d > %dx%d) "
        "for polygon '%s/%s'", lmw, lmh,
        max_lmw, max_lmh, mname, GetName());
      flags.Reset (CS_POLY_LIGHTING);
    }
  }

  return rc;
}

void csPolygon3DStatic::SetTextureSpace (
  const csMatrix3 &tx_matrix,
  const csVector3 &tx_vector)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    MappingSetTextureSpace (tx_matrix, tx_vector);
  }
}

void csPolygon3DStatic::GetTextureSpace (
  csMatrix3 &tx_matrix,
  csVector3 &tx_vector)
{
  if (IsTextureMappingEnabled ())
  {
    MappingGetTextureSpace (tx_matrix, tx_vector);
  }
}

bool csPolygon3DStatic::SetTextureSpace (
  const csVector3 &p1,
  const csVector2 &uv1,
  const csVector3 &p2,
  const csVector2 &uv2,
  const csVector3 &p3,
  const csVector2 &uv3)
{
  // Some explanation. We have three points for
  // which we know the uv coordinates. This gives:
  //     P1 -> UV1
  //     P2 -> UV2
  //     P3 -> UV3
  // P1, P2, and P3 are on the same plane so we can write:
  //     P = P1 + lambda * (P2-P1) + mu * (P3-P1)
  // For the same lambda and mu we can write:
  //     UV = UV1 + lambda * (UV2-UV1) + mu * (UV3-UV1)
  // What we want is Po, Pu, and Pv (also on the same
  // plane) so that the following uv coordinates apply:
  //     Po -> 0,0
  //     Pu -> 1,0
  //     Pv -> 0,1
  // The UV equation can be written as follows:
  //     U = U1 + lambda * (U2-U1) + mu * (U3-U1)
  //     V = V1 + lambda * (V2-V1) + mu * (V3-V1)
  // This is a matrix equation (2x2 matrix):
  //     UV = UV1 + M * PL
  // We have UV in this case and we need PL so we
  // need to invert this equation:
  //     (1/M) * (UV - UV1) = PL
  csMatrix2 m (uv2.x - uv1.x, uv3.x - uv1.x, uv2.y - uv1.y, uv3.y - uv1.y);
  float det = m.Determinant ();

  if (ABS (det) < CS_POLY_MIN_UV_DET)
  {
    if (csThingObjectType::do_verbose)
    {
      thing_static->thing_type->Warn (
        "Warning: bad UV coordinates for poly '%s'!", GetName ());
    }
    SetTextureSpace (p1, p2, 1);
    return false;
  }
  else
    m.Invert ();

  csVector2 pl;
  csVector3 po, pu, pv;

  // For (0,0) and Po
  pl = m * (csVector2 (0, 0) - uv1);
  po = p1 + pl.x * (p2 - p1) + pl.y * (p3 - p1);

  // For (1,0) and Pu
  pl = m * (csVector2 (1, 0) - uv1);
  pu = p1 + pl.x * (p2 - p1) + pl.y * (p3 - p1);

  // For (0,1) and Pv
  pl = m * (csVector2 (0, 1) - uv1);
  pv = p1 + pl.x * (p2 - p1) + pl.y * (p3 - p1);

  SetTextureSpace (po, pu, (pu - po).Norm (), pv, (pv - po).Norm ());
  return true;
}

void csPolygon3DStatic::SetTextureSpace (
  const csVector3 &v_orig,
  const csVector3 &v1,
  float len1)
{
  float xo = v_orig.x;
  float yo = v_orig.y;
  float zo = v_orig.z;
  float x1 = v1.x;
  float y1 = v1.y;
  float z1 = v1.z;
  SetTextureSpace (xo, yo, zo, x1, y1, z1, len1);
}

void csPolygon3DStatic::SetTextureSpace (
  float xo,
  float yo,
  float zo,
  float x1,
  float y1,
  float z1,
  float len1)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    MappingSetTextureSpace (
          polygon_data.plane_obj,
          xo, yo, zo,
          x1, y1, z1,
          len1);
  }
}

void csPolygon3DStatic::SetTextureSpace (
  const csVector3 &v_orig,
  const csVector3 &v1,
  float len1,
  const csVector3 &v2,
  float len2)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    MappingSetTextureSpace (v_orig, v1, len1, v2, len2);
  }
}

void csPolygon3DStatic::SetTextureSpace (
  float xo,
  float yo,
  float zo,
  float x1,
  float y1,
  float z1,
  float len1,
  float x2,
  float y2,
  float z2,
  float len2)
{
  SetTextureSpace (
    csVector3 (xo, yo, zo),
    csVector3 (x1, y1, z1),
    len1,
    csVector3 (x2, y2, z2),
    len2);
}

void csPolygon3DStatic::SetVertex (int idx, int v)
{
  polygon_data.vertices[idx] = v;
}

int csPolygon3DStatic::AddVertex (int v)
{
  if (v >= thing_static->GetVertexCount ())
  {
    thing_static->thing_type->Bug (
        "Index number %d is too high for a polygon (max=%d) (polygon '%s')!",
        v,
        thing_static->GetVertexCount (),
        GetName () ? GetName () : "<noname>");
    return 0;
  }

  if (v < 0)
  {
    thing_static->thing_type->Bug ("Bad negative vertex index %d!", v);
    return 0;
  }

  SetNumVertices (polygon_data.num_vertices+1);
  polygon_data.vertices[polygon_data.num_vertices-1] = v;
  return polygon_data.num_vertices - 1;
}

int csPolygon3DStatic::AddVertex (const csVector3 &v)
{
  int i = thing_static->AddVertex (v);
  AddVertex (i);
  return i;
}

void csPolygon3DStatic::SetVertex (int idx, const csVector3 &v)
{
  int i = thing_static->AddVertex (v);
  SetVertex (idx, i);
}

int csPolygon3DStatic::AddVertex (float x, float y, float z)
{
  int i = thing_static->AddVertex (x, y, z);
  AddVertex (i);
  return i;
}

void csPolygon3DStatic::PlaneNormal (float *yz, float *zx, float *xy)
{
  float ayz = 0;
  float azx = 0;
  float axy = 0;
  int i, i1;
  float x1, y1, z1, x, y, z;

  i1 = polygon_data.num_vertices - 1;
  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    x = Vobj (i).x;
    y = Vobj (i).y;
    z = Vobj (i).z;
    x1 = Vobj (i1).x;
    y1 = Vobj (i1).y;
    z1 = Vobj (i1).z;
    ayz += (z1 + z) * (y - y1);
    azx += (x1 + x) * (z - z1);
    axy += (y1 + y) * (x - x1);
    i1 = i;
  }

  float sqd = ayz * ayz + azx * azx + axy * axy;
  float invd;
  if (sqd < SMALL_EPSILON)
    invd = 1.0f / SMALL_EPSILON;
  else
    invd = csQisqrt (sqd);

  *yz = ayz * invd;
  *zx = azx * invd;
  *xy = axy * invd;
}

bool csPolygon3DStatic::PointOnPolygon (const csVector3 &v)
{
  // First check if point is on the plane.
  csPlane3 &pl = polygon_data.plane_obj;
  float dot = pl.D () + pl.A () * v.x + pl.B () * v.y + pl.C () * v.z;
  if (ABS (dot) >= EPSILON) return false;

  // Check if 'v' is on the same side of all edges.
  int i, i1;
  bool neg = false, pos = false;
  i1 = polygon_data.num_vertices - 1;
  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    float ar = csMath3::Direction3 (v, Vobj (i1), Vobj (i));
    if (ar < 0)
      neg = true;
    else if (ar > 0)
      pos = true;
    if (neg && pos) return false;
    i1 = i;
  }

  return true;
}

bool csPolygon3DStatic::IntersectRay (const csVector3 &start,
    const csVector3 &end)
{
  // First we do backface culling on the polygon with respect to
  // the starting point of the beam.
  csPlane3 &pl = polygon_data.plane_obj;
  float dot1 = pl.D () +
    pl.A () * start.x + pl.B () * start.y + pl.C () * start.z;
  if (dot1 > 0) return false;

  // If this vector is perpendicular to the plane of the polygon we
  // need to catch this case here.
  float dot2 = pl.D () + pl.A () * end.x + pl.B () * end.y + pl.C () * end.z;
  if (ABS (dot1 - dot2) < SMALL_EPSILON) return false;

  // Now we generate a plane between the starting point of the ray and
  // every edge of the polygon. With the plane normal of that plane we
  // can then check if the end of the ray is on the same side for all
  // these planes.
  csVector3 normal;
  csVector3 relend = end;
  relend -= start;

  int i, i1;
  i1 = polygon_data.num_vertices - 1;
  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    csMath3::CalcNormal (normal, start, Vobj (i1), Vobj (i));
    if ((relend * normal) > 0) return false;
    i1 = i;
  }

  return true;
}

bool csPolygon3DStatic::IntersectRayNoBackFace (
  const csVector3 &start,
  const csVector3 &end)
{
  // If this vector is perpendicular to the plane of the polygon we
  // need to catch this case here.
  csPlane3 &pl = polygon_data.plane_obj;
  float dot1 = pl.D () +
    pl.A () * start.x +
    pl.B () * start.y +
    pl.C () * start.z;
  float dot2 = pl.D () + pl.A () * end.x + pl.B () * end.y + pl.C () * end.z;
  if (ABS (dot1 - dot2) < SMALL_EPSILON) return false;

  // If dot1 > 0 the polygon would have been backface culled.
  // In this case we just use the result of this test to reverse
  // the test below.
  // Now we generate a plane between the starting point of the ray and
  // every edge of the polygon. With the plane normal of that plane we
  // can then check if the end of the ray is on the same side for all
  // these planes.
  csVector3 normal;
  csVector3 relend = end;
  relend -= start;

  int i, i1;
  i1 = polygon_data.num_vertices - 1;
  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    csMath3::CalcNormal (normal, start, Vobj (i1), Vobj (i));
    if (dot1 > 0)
    {
      if ((relend * normal) < 0) return false;
    }
    else
    {
      if ((relend * normal) > 0) return false;
    }

    i1 = i;
  }

  return true;
}

bool csPolygon3DStatic::IntersectSegment (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  if (!IntersectRay (start, end)) return false;
  return IntersectSegmentPlane (start, end, isect, pr);
}

bool csPolygon3DStatic::IntersectSegmentPlane (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr) const
{
  float x1 = start.x;
  float y1 = start.y;
  float z1 = start.z;
  float x2 = end.x;
  float y2 = end.y;
  float z2 = end.z;
  float r, num, denom;

  // So now we have the plane equation of the polygon:
  // A*x + B*y + C*z + D = 0
  //
  // We also have the parameter line equations of the ray
  // going through 'start' and 'end':
  // x = r*(x2-x1)+x1
  // y = r*(y2-y1)+y1
  // z = r*(z2-z1)+z1
  //
  // =>   A*(r*(x2-x1)+x1) + B*(r*(y2-y1)+y1) + C*(r*(z2-z1)+z1) + D = 0
  // Set *pr to -1 to indicate error if we return false now.
  if (pr) *pr = -1;

  denom = polygon_data.plane_obj.A () * (x2 - x1) +
    polygon_data.plane_obj.B () * (y2 - y1) +
    polygon_data.plane_obj.C () * (z2 - z1);
  if (ABS (denom) < SMALL_EPSILON) return false;  // Lines are parallel
  num = -(polygon_data.plane_obj.A () * x1 +
    polygon_data.plane_obj.B () * y1 +
    polygon_data.plane_obj.C () * z1 +
    polygon_data.plane_obj.D ());
  r = num / denom;

  // Calculate 'r' and 'isect' even if the intersection point is
  // not on the segment. That way we can use this function for testing
  // with rays as well.
  if (pr) *pr = r;

  isect.x = r * (x2 - x1) + x1;
  isect.y = r * (y2 - y1) + y1;
  isect.z = r * (z2 - z1) + z1;

  // If r is not in [0,1] the intersection point is not on the segment.
  if (r < 0 /*-SMALL_EPSILON*/ || r > 1) return false;

  return true;
}

bool csPolygon3DStatic::IntersectRayPlane (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect)
{
  float r;
  IntersectSegmentPlane (start, end, isect, &r);
  return r >= 0;
}

//---------------------------------------------------------------------------

csPolygon3D::csPolygon3D ()
{
  VectorArray = GetStaticVectorArray();
  thing = 0;
  lightpatches = 0;
}

csPolygon3D::~csPolygon3D ()
{
  RemovePolyTexture ();
  if (thing)
  {
    while (lightpatches)
    {
      iLight* dl = lightpatches->GetLight ();
      if (dl)
        dl->RemoveAffectedLightingInfo (
		&(thing->scfiLightingInfo));
      thing->GetStaticData ()->thing_type->lightpatch_pool->Free (lightpatches);
    }
  }
}

void csPolygon3D::RemovePolyTexture ()
{
  if (GetParent ())
  {
    iGraphics3D* G3D = GetParent()->GetStaticData()->thing_type->G3D;
    if (G3D && txt_info.rlm) G3D->RemoveFromCache (txt_info.rlm);
  }
  if (txt_info.lm)
  {
    GetParent ()->GetStaticData ()->thing_type->blk_lightmap.Free (txt_info.lm);
  }
}

int csPolygon3D::GetPolyIdx () const
{
  return (int)thing->polygons.GetIndex (this);
}

csPolygon3DStatic* csPolygon3D::GetStaticPoly () const
{
  return thing->GetStaticData ()->GetPolygon3DStatic (
  	GetPolyIdx ());
}

void csPolygon3D::SetParent (csThing *thing)
{
  csPolygon3D::thing = thing;
}

void csPolygon3D::RefreshFromStaticData ()
{
  RemovePolyTexture ();
}

#define TEXW(t) ((t)->w_orig)
#define TEXH(t) ((t)->h)

void csPolygon3D::Finish (csPolygon3DStatic* spoly)
{
  RefreshFromStaticData ();

  if (spoly->IsTextureMappingEnabled ())
  {
    txt_info.SetLightMap (0);
    if (csThing::lightmap_enabled && spoly->flags.Check (CS_POLY_LIGHTING))
    {
      csLightMap *lm = spoly->thing_static->thing_type
        ->blk_lightmap.Alloc ();
      txt_info.SetLightMap (lm);

      lm->Alloc (spoly->polygon_data.tmapping->GetLitOriginalWidth (),
      	spoly->polygon_data.tmapping->GetLitHeight ());
      /*lm->InitColor (128, 128, 128);*/
    }
  }
}

void csPolygon3D::DynamicLightDisconnect (iLight* dynlight)
{
  csLightPatch* lp = lightpatches;
  while (lp)
  {
    csLightPatch* lpnext = lp->GetNext ();
    if (lp->GetLight () == dynlight)
      thing->GetStaticData ()->thing_type->lightpatch_pool->Free (lp);
    lp = lpnext;
  }
}

void csPolygon3D::StaticLightDisconnect (iLight* statlight)
{
  csLightMap* lm = txt_info.GetLightMap ();
  if (!lm) return;
  csShadowMap* sm = lm->FindShadowMap (statlight);
  if (!sm) return;
  lm->DelShadowMap (sm);
  txt_info.light_version--;
}

void csPolygon3D::UnlinkLightpatch (csLightPatch *lp)
{
  lp->RemoveList (lightpatches);
}

void csPolygon3D::AddLightpatch (csLightPatch *lp)
{
  lp->AddList (lightpatches);
  lp->SetPolyCurve (this);
}

void csPolygon3D::InitializeDefault (bool clear)
{
  if (txt_info.lm == 0) return ;
  txt_info.InitLightMaps ();
  if (clear)
  {
    csColor ambient;
    thing->GetStaticData ()->thing_type->engine->GetAmbientLight (ambient);
    txt_info.lm->InitColor (
          int(ambient.red * 255.0f),
          int(ambient.green * 255.0f),
          int(ambient.blue * 255.0f));
  }
}

const char* csPolygon3D::ReadFromCache (iFile* file, csPolygon3DStatic* spoly)
{
  if (txt_info.lm == 0) return 0;
  const char* error = txt_info.lm->ReadFromCache (
          file,
          spoly->polygon_data.tmapping->GetLitOriginalWidth (),
          spoly->polygon_data.tmapping->GetLitHeight (),
          this, spoly,
	  thing->GetStaticData ()->thing_type->engine);
  if (error != 0)
  {
    txt_info.InitLightMaps ();
  }
  return error;
}

bool csPolygon3D::WriteToCache (iFile* file, csPolygon3DStatic* spoly)
{
  if (txt_info.lm == 0) return true;
  if (thing->GetStaticData ()->thing_type->engine->GetLightingCacheMode ()
      & CS_ENGINE_CACHE_WRITE)
    txt_info.lm->Cache (file, this,
	spoly,
    	thing->GetStaticData ()->thing_type->engine);
  return true;
}

bool csPolygon3D::MarkRelevantShadowFrustums (
  iFrustumView* lview,
  csPlane3 &plane,
  csPolygon3DStatic* spoly)
{
  // @@@ Currently this function only checks if a shadow frustum is inside
  // the light frustum. There is no checking done if shadow frustums obscure
  // each other.
  int i, i1, j, j1;

  csFrustumContext *ctxt = lview->GetFrustumContext ();
  iShadowIterator *shadow_it = ctxt->GetShadows ()->GetShadowIterator ();
  csFrustum *lf = ctxt->GetLightFrustum ();
  const csVector3 &center = ctxt->GetLightFrustum ()->GetOrigin ();

  // Precalculate the normals for csFrustum::BatchClassify.
  csVector3 *lf_verts = lf->GetVertices ();
  csVector3 lf_normals[100];  // @@@ HARDCODED!
  i1 = lf->GetVertexCount () - 1;
  for (i = 0; i < lf->GetVertexCount (); i++)
  {
    lf_normals[i1] = lf_verts[i1] % lf_verts[i];
    i1 = i;
  }

  int* vt_idx = spoly->GetVertexIndices ();

  // For every shadow frustum...
  while (shadow_it->HasNext ())
  {
    csFrustum *sf = shadow_it->Next ();

    // First check if the plane of the shadow frustum is close to the plane
    // of the polygon (the input parameter 'plane'). If so then we discard the
    // frustum as not relevant.
    if (csMath3::PlanesClose (*sf->GetBackPlane (), plane))
      shadow_it->MarkRelevant (false);
    else
    {
      csPolygon3D *sfp = (csPolygon3D *) (shadow_it->GetUserData ());
      if (sfp == 0)
      {
        shadow_it->MarkRelevant (true);
	continue;
      }
      csPolygon3DStatic* sfp_static = sfp->GetStaticPoly ();
      int* sfp_vt_idx = sfp_static->GetVertexIndices ();
      csThing* sfp_thing = sfp->GetParent ();
      int sfp_vt_cnt = sfp_static->GetVertexCount ();

      switch (csFrustum::BatchClassify (
            lf_verts,
            lf_normals,
            lf->GetVertexCount (),
            sf->GetVertices (),
            sf->GetVertexCount ()))
      {
        case CS_FRUST_PARTIAL:
        case CS_FRUST_INSIDE:
          shadow_it->MarkRelevant (true);

          // If partial then we first test if the light and shadow
          // frustums are adjacent. If so then we ignore the shadow
          // frustum as well (not relevant).

          i1 = spoly->GetVertexCount () - 1;
          for (i = 0; i < spoly->GetVertexCount (); i++)
          {
            j1 = sfp_vt_cnt - 1;

	    const csVector3& v_i1 = thing->Vwor (vt_idx[i1]);
	    const csVector3& v_i = thing->Vwor (vt_idx[i]);
	    const csVector3& sfp_v_j1 = sfp_thing->Vwor (sfp_vt_idx[j1]);
            float a1 = csMath3::Direction3 (v_i1, v_i, sfp_v_j1);
            for (j = 0 ; j < sfp_vt_cnt ; j++)
            {
	      const csVector3& sfp_v_j = sfp_thing->Vwor (sfp_vt_idx[j]);
              float a = csMath3::Direction3 (v_i1, v_i, sfp_v_j);
              if (ABS (a) < EPSILON && ABS (a1) < EPSILON)
              {
                // The two points of the shadow frustum are on the same
                // edge as the current light frustum edge we are examining.
                // In this case we test if the orientation of the two edges
                // is different. If so then the shadow frustum is not
                // relevant.
                csVector3 d1 = v_i - v_i1;
                csVector3 d2 = sfp_v_j - sfp_v_j1;
                if (
                  (d1.x < -EPSILON && d2.x > EPSILON) ||
                  (d1.x > EPSILON && d2.x < -EPSILON) ||
                  (d1.y < -EPSILON && d2.y > EPSILON) ||
                  (d1.y > EPSILON && d2.y < -EPSILON) ||
                  (d1.z < -EPSILON && d2.z > EPSILON) ||
                  (d1.z > EPSILON && d2.z < -EPSILON))
                {
                  shadow_it->MarkRelevant (false);
                  break;
                }
              }

              if (!shadow_it->IsRelevant ()) break;
              j1 = j;
              a1 = a;
            }

            if (!shadow_it->IsRelevant ()) break;
            i1 = i;
          }
          break;
        case CS_FRUST_OUTSIDE:
          shadow_it->MarkRelevant (false);
          break;
        case CS_FRUST_COVERED:
	  {
	    // To see if we really have a 'covered' case we first
	    // test if the covering polygon isn't behind the first
	    // polygon. To do that we take a ray from the center of
	    // the light to the plane of the other polygon and see
	    // if it intersects.
	    csVector3 isect;
	    float dist;
	    const csPlane3& wor_plane = sfp->GetParent ()
	    	->GetPolygonWorldPlaneNoCheck (sfp->GetPolyIdx ());
	    if (!csIntersect3::SegmentPlane (center, thing->Vwor (vt_idx[0]), wor_plane,
	      isect, dist))
	    {
	      shadow_it->MarkRelevant (false);
	      break;
	    }
	  }
          shadow_it->DecRef ();
          return false;
      }
    }
  }

  shadow_it->DecRef ();
  return true;
}

void csPolygon3D::CalculateLightingDynamic (iFrustumView *lview,
    iMovable* movable, const csPlane3& world_plane,
    csPolygon3DStatic* spoly)
{
  csFrustum *light_frustum = lview->GetFrustumContext ()->GetLightFrustum ();
  const csVector3 &center = light_frustum->GetOrigin ();

  // If plane is not visible then return (backface culling).
  if (!csMath3::Visible (center, world_plane)) return ;

  // Compute the distance from the center of the light
  // to the plane of the polygon.
  float dist_to_plane = world_plane.Distance (center);

  // If distance is too small or greater than the radius of the light
  // then we have a trivial case (no hit).
  if (dist_to_plane < SMALL_EPSILON || dist_to_plane >= lview->GetRadius ())
    return ;

  csRef<csFrustum> new_light_frustum;

  csVector3 *poly;
  int num_vertices;

  num_vertices = spoly->polygon_data.num_vertices;
  if ((size_t)num_vertices > VectorArray->Length ())
    VectorArray->SetLength (num_vertices);
  poly = VectorArray->GetArray ();

  int j;
  int* vt_idx = spoly->GetVertexIndices ();
  if (lview->GetFrustumContext ()->IsMirrored ())
    for (j = 0; j < num_vertices; j++)
      poly[j] = thing->Vwor (vt_idx[num_vertices - j - 1]) - center;
  else
    for (j = 0; j < num_vertices; j++)
      poly[j] = thing->Vwor (vt_idx[j]) - center;

  new_light_frustum = light_frustum->Intersect (poly, num_vertices);

  // Check if light frustum intersects with the polygon
  if (!new_light_frustum) return ;

  // There is an intersection of the current light frustum with the polygon.
  // This means that the polygon is hit by the light.
  // The light is close enough to the plane of the polygon. Now we calculate
  // an accurate minimum squared distance from the light to the polygon. Note
  // that we use the new_frustum which is relative to the center of the
  // light.
  // So this algorithm works with the light position at (0,0,0).
  csPlane3 poly_plane = csPoly3D::ComputePlane (poly, num_vertices);

  csVector3 o (0, 0, 0);
  float min_sqdist = csSquaredDist::PointPoly (
      o,
      new_light_frustum->GetVertices (),
      new_light_frustum->GetVertexCount (),
      poly_plane,
      dist_to_plane * dist_to_plane);

  if (min_sqdist >= lview->GetSquaredRadius ()) return;

  // Update the lightmap.
  FillLightMapDynamic (lview, new_light_frustum);
}

void csPolygon3D::FillLightMapDynamic (iFrustumView* lview,
	csFrustum* light_frustum)
{
  csFrustumContext *ctxt = lview->GetFrustumContext ();

  // We are working for a dynamic light. In this case we create
  // a light patch for this polygon.
  // @@@ Lots of pointers to get the lightpatch pool!!!
  csLightPatch *lp = thing->GetStaticData ()->thing_type->
    lightpatch_pool->Alloc ();
  AddLightpatch (lp);

  iFrustumViewUserdata* fvud = lview->GetUserdata ();
  iLightingProcessInfo* lpi = (iLightingProcessInfo*)fvud;
  iLight* l = lpi->GetLight ();
  lp->SetLight (l);

  //csFrustum *light_frustum = ctxt->GetLightFrustum ();
  lp->Initialize (light_frustum->GetVertexCount ());

  int i, mi;
  for (i = 0; i < lp->GetVertexCount (); i++)
  {
    mi = ctxt->IsMirrored () ? lp->GetVertexCount () - i - 1 : i;
    lp->GetVertex (i) = light_frustum->GetVertex (mi);
  }
}

void csPolygon3D::CalculateLightingStatic (iFrustumView *lview,
  iMovable* movable,
  csLightingPolyTexQueue* lptq, bool vis,
  const csMatrix3& m_world2tex,
  const csVector3& v_world2tex,
  const csPlane3& world_plane,
  csPolygon3DStatic* spoly)
{
  bool do_smooth = GetParent ()->GetStaticData ()->GetSmoothingFlag ();

  bool maybeItsVisible = false;
  csFrustum *light_frustum = lview->GetFrustumContext ()->GetLightFrustum ();
  const csVector3 &center = light_frustum->GetOrigin ();

  // If plane is not visible then return (backface culling).
  if (!csMath3::Visible (center, world_plane))
    if (do_smooth)
      maybeItsVisible = true;
    else
      return;

  // Compute the distance from the center of the light
  // to the plane of the polygon.
  float dist_to_plane = world_plane.Distance (center);

  // If distance is too small or greater than the radius of the light
  // then we have a trivial case (no hit).
  if ((!do_smooth && dist_to_plane < SMALL_EPSILON)
    || dist_to_plane >= lview->GetRadius ())
    return ;

  // In the following algorithm we ignore the light frustum and only
  // apply shadows on the lightmap.
  // @@@ TODO: Optimization. Use the light frustum to test if the
  // polygon falls inside the light frustum (to avoid unneeded work).
  // Beware of mirroring here.
  // @@@ TODO: Optimization. Calculate minimum squared distance between
  // the light center and the polygon to see if we should bother lighting
  // at all.
  // @@@ TODO: Optimization. Mark all shadow frustums which are relevant. i.e.
  // which are inside the light frustum and are not obscured (shadowed)
  // by other shadow frustums. Maybe only do this for portals.
  // We should also give the polygon plane to MarkRelevantShadowFrustums so
  // that all shadow frustums which start at the same plane are discarded as
  // well.
  // @@@ TODO: Optimization. Precalculated edge-table to detect polygons
  // that are adjacent.

  // Update the lightmap given light and shadow frustums in lview.
  if (txt_info.lm)
    txt_info.FillLightMap (lview, lptq, vis, this,
    	m_world2tex, v_world2tex, world_plane, spoly);

  if (maybeItsVisible)
    return;
}
