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
#include "csqint.h"
#include "csqsqrt.h"

#include "csgeom/frustum.h"
#include "csgeom/matrix2.h"
#include "csgeom/poly3d.h"
#include "textrans.h"
#include "csutil/csendian.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/memfile.h"
#include "csutil/stringquote.h"

#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/texture.h"
#include "iutil/cache.h"
#include "iutil/vfs.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "polygon.h"
#include "polytext.h"
#include "lghtmap.h"
#include "lppool.h"
#include "thing.h"

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

typedef csDirtyAccessArrayRefCounted<csVector3> engine3d_VectorArray;
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
  SetNumVertices (0);   // Clear vertices.
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

bool csPolygon3DStatic::MappingSetTextureSpace (
  const csPlane3 &plane_wor,
  const csVector3 &v_orig,
  const csVector3 &v1,
  float len)
{
  return MappingSetTextureSpace (
    plane_wor,
    v_orig.x, v_orig.y, v_orig.z,
    v1.x, v1.y, v1.z,
    len);
}

bool csPolygon3DStatic::MappingSetTextureSpace (
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
  bool result = csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      xo, yo, zo,
      x1, y1, z1,
      len1,
      A, B, C);
  thing_static->ShapeChanged ();
  return result;
}

bool csPolygon3DStatic::MappingSetTextureSpace (
  const csVector3 &v_orig,
  const csVector3 &v1,
  float len1,
  const csVector3 &v2,
  float len2)
{
  bool result = csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      v_orig, v1,
      len1, v2,
      len2);
  thing_static->ShapeChanged ();
  return result;
}

bool csPolygon3DStatic::MappingSetTextureSpace (
  const csVector3 &v_orig,
  const csVector3 &v_u,
  const csVector3 &v_v)
{
  bool result = csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      v_orig,
      v_u,
      v_v);
  thing_static->ShapeChanged ();
  return result;
}

bool csPolygon3DStatic::MappingSetTextureSpace (
  float xo, float yo, float zo,
  float xu, float yu, float zu,
  float xv, float yv, float zv)
{
  const csVector3 o (xo, yo, zo);
  const csVector3 u (xu, yu, zu);
  const csVector3 v (xv, yv, zv);
  bool result = csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      o, u, v);
  thing_static->ShapeChanged ();
  return result;
}

bool csPolygon3DStatic::MappingSetTextureSpace (
  float xo, float yo, float zo,
  float xu, float yu, float zu,
  float xv, float yv, float zv,
  float xw, float yw, float zw)
{
  bool result = csTextureTrans::compute_texture_space (
      polygon_data.tmapping->GetO2T (),
      polygon_data.tmapping->GetO2TTranslation (),
      xo, yo, zo,
      xu, yu, zu,
      xv, yv, zv,
      xw, yw, zw);
  thing_static->ShapeChanged ();
  return result;
}

bool csPolygon3DStatic::MappingSetTextureSpace (
  const csMatrix3 &tx_matrix,
  const csVector3 &tx_vector)
{
  polygon_data.tmapping->SetO2T (tx_matrix),
  polygon_data.tmapping->SetO2TTranslation (tx_vector),
  thing_static->ShapeChanged ();
  return true;
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

  if (thing_static) thing_static->ShapeChanged ();
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

iMaterial *csPolygon3DStatic::GetMaterial ()
{
  return material ? material->GetMaterial () : 0;
}

bool csPolygon3DStatic::IsTransparent ()
{
  iTextureHandle *txt_handle = GetMaterial ()->GetTexture ();
  return txt_handle && ((txt_handle->GetAlphaType ()
    != csAlphaMode::alphaNone));
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
  thing_static->ShapeChanged ();
}

void csPolygon3DStatic::HardTransform (const csReversibleTransform &t)
{
  csPlane3 new_plane;
  t.This2Other (GetObjectPlane (), Vobj (0), new_plane);
  SetObjectPlane (new_plane);
  thing_static->ShapeChanged ();
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
  // We do a small correction of max_u/max_v in case they happen to be equal
  // to min_u/min_v so that the GL renderer no longer crashes.
  if (max_u == min_u) max_u = min_u + .1;
  if (max_v == min_v) max_v = min_v + .1;
  polygon_data.tmapping->SetTextureBox (min_u, min_v, max_u, max_v);

  int ww, hh;
  iMaterial* mat = GetMaterial ();
  if (mat && mat->GetTexture ())
  {
    rc = mat->GetTexture ()->GetRendererDimensions (ww, hh);
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
  int w = Imax_u - polygon_data.tmapping->GetIMinU ();
  polygon_data.tmapping->SetLitWidth (w);
  polygon_data.tmapping->SetLitHeight (h);

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
    if (!material || (material->GetMaterial () &&
      !material->GetMaterial ()->GetTexture ()))
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
        polygon_data.tmapping->GetLitWidth ());
    int lmh = csLightMap::CalcLightMapHeight (
        polygon_data.tmapping->GetLitHeight ());
    int max_lmw = 256, max_lmh = 256;
    //@@@thing_static->thing_type->engine->GetMaxLightmapSize (max_lmw, max_lmh);
    if ((lmw > max_lmw) || (lmh > max_lmh))
    {
      const char* mname = 0;
      if (thing_logparent)
      {
        csRef<iMeshWrapper> m = 
          scfQueryInterface<iMeshWrapper> (thing_logparent);
        if (m) mname = m->QueryObject ()->GetName ();
        else mname = "<unknown>";
      }
      else mname = "<unknown>";
      thing_static->thing_type->Notify ("Oversize lightmap (%dx%d > %dx%d) "
        "for polygon %s", lmw, lmh,
        max_lmw, max_lmh,
	CS::Quote::Single (csString().Format ("%s/%s", mname, GetName())));
      flags.Reset (CS_POLY_LIGHTING);
    }
  }

  return rc;
}

bool csPolygon3DStatic::SetTextureSpace (
  const csMatrix3 &tx_matrix,
  const csVector3 &tx_vector)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    return MappingSetTextureSpace (tx_matrix, tx_vector);
  }
  return true;
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
        "Warning: bad UV coordinates for poly %s!", CS::Quote::Single (GetName ()));
    }
    if (!((p1-p2) < SMALL_EPSILON))
      SetTextureSpace (p1, p2, 1);
    else if (!((p1-p3) < SMALL_EPSILON))
      SetTextureSpace (p1, p3, 1);
    else
    {
      // @@@ Nothing sensible to do?
    }
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

  return SetTextureSpace (po, pu, (pu - po).Norm (), pv, (pv - po).Norm ());  
}

bool csPolygon3DStatic::SetTextureSpace (
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
  return SetTextureSpace (xo, yo, zo, x1, y1, z1, len1);
}

bool csPolygon3DStatic::SetTextureSpace (
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
    return MappingSetTextureSpace (
          polygon_data.plane_obj,
          xo, yo, zo,
          x1, y1, z1,
          len1);
  }
  return true;
}

bool csPolygon3DStatic::SetTextureSpace (
  const csVector3 &v_orig,
  const csVector3 &v1,
  float len1,
  const csVector3 &v2,
  float len2)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    return MappingSetTextureSpace (v_orig, v1, len1, v2, len2);
  }
  return true;
}

bool csPolygon3DStatic::SetTextureSpace (
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
  return SetTextureSpace (
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
        "Index number %d is too high for a polygon (max=%d) (polygon %s)!",
        v,
        thing_static->GetVertexCount (),
        CS::Quote::Single (GetName () ? GetName () : "<noname>"));
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

bool csPolygon3DStatic::InSphere (const csVector3& center, float radius)
{
  int i, i1;
  
  // first check distance from polygon
  float dist = polygon_data.plane_obj.Classify(center);
  if (fabs(dist) > radius)
    return false;

  float rsquared = radius*radius;
  
  // second, check to see if any vertex is inside the sphere
  for (i=0; i<polygon_data.num_vertices; ++i)
  {
    if ((Vobj(i) - center).SquaredNorm() <= rsquared)
      return true;
  }

  csVector3 p, d, pc;
  float pcDotd, dDotd, pcDotpc;
  
  // didn't work, so test sphere with each edge
  i1 = polygon_data.num_vertices - 1;
  for (i = 0; i < polygon_data.num_vertices; i++)
  {
    p = Vobj(i);
    d = Vobj(i1) - p;
    pc = p - center;

    // check if line points in opposite direction
    pcDotd = pc * d;
    if (pcDotd > 0)
      continue;
   
    dDotd = d * d;
    pcDotpc = pc * pc;

    float det = pcDotd * pcDotd - dDotd * (pcDotpc - rsquared);

    // check if there's actually an intersection with the circle
    if (det < 0)
    {
      // no intersection
    }
    else if (det <= 0.01)
    {
      // one intersection
      float t = -pcDotd / dDotd;
      if (t >= 0 && t <= 1)
        return true;
    }
    else
    {
      // two intersections
      float sqrtDet = sqrt(det);
      float t = (-pcDotd - sqrtDet) / dDotd;
      if (t >= 0 && t <= 1)
        return true;
      t = (-pcDotd + sqrtDet) / dDotd;
      if (t >= 0 && t <= 1)
        return true;
    }
    i1 = i;
  }

  // last resort, project sphere center onto polygon and test point in poly
  p = center - polygon_data.plane_obj.Normal() * dist;
  return PointOnPolygon(p);
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
      //iLight* dl = lightpatches->GetLight ();
      //if (dl)
        //dl->RemoveAffectedLightingInfo (
                //(iLightingInfo*)thing);
      thing->GetStaticData ()->thing_type->lightpatch_pool->Free (lightpatches);
    }
  }
}

void csPolygon3D::RemovePolyTexture ()
{
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

      lm->Alloc (spoly->polygon_data.tmapping->GetLitWidth (),
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
          spoly->polygon_data.tmapping->GetLitWidth (),
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
  return true;
}


