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
#include "cssys/csendian.h"
#include "polygon.h"
#include "pol2d.h"
#include "polytext.h"
#include "polytmap.h"
#include "lghtmap.h"
#include "portal.h"
#include "lppool.h"
#include "thing.h"
#include "csgeom/frustum.h"
#include "csutil/garray.h"
#include "csutil/debug.h"
#include "csutil/memfile.h"
#include "iutil/vfs.h"
#include "iutil/cache.h"
#include "csgeom/matrix2.h"
#include "csgeom/poly3d.h"
#include "qint.h"
#include "qsqrt.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/dynlight.h"
#include "iengine/shadows.h"
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

typedef csGrowingArray<csVector3> engine3d_VectorArray;
CS_IMPLEMENT_STATIC_VAR (GetStaticVectorArray, engine3d_VectorArray,())

static engine3d_VectorArray *VectorArray = NULL;

//---------------------------------------------------------------------------

csPolyTexLightMap::csPolyTexLightMap ()
{
  DG_ADDI (this, NULL);
  DG_TYPE (this, "csPolyTexLightMap");
  txt_plane = NULL;
  tex = new csPolyTexture ();
  DG_LINK (this, tex);
  lightmap_up_to_date = false;
  Alpha = 0;
#ifndef CS_USE_NEW_RENDERER
  MixMode = CS_FX_COPY;
#endif // CS_USE_NEW_RENDERER
}

csPolyTexLightMap::~csPolyTexLightMap ()
{
  if (tex)
  {
    DG_UNLINK (this, tex);
    tex->DecRef ();
  }

  if (txt_plane) txt_plane->DecRef ();
  DG_REM (this);
}

void csPolyTexLightMap::Setup (csPolygon3D *poly3d, iMaterialWrapper *mat)
{
  tex->SetPolygon (poly3d);
  tex->CreateBoundingTextureBox ();
}

csPolyTexture *csPolyTexLightMap::GetPolyTex ()
{
  return tex;
}

iPolyTxtPlane *csPolyTexLightMap::GetPolyTxtPlane () const
{
  return &(GetTxtPlane ()->scfiPolyTxtPlane);
}

void csPolyTexLightMap::SetTxtPlane (csPolyTxtPlane *txt_pl)
{
  if (txt_pl) txt_pl->IncRef ();
  if (txt_plane) txt_plane->DecRef ();
  txt_plane = txt_pl;
}

void csPolyTexLightMap::NewTxtPlane (csThingObjectType* thing_type)
{
  if (txt_plane) txt_plane->DecRef ();
  txt_plane = new csPolyTxtPlane (thing_type);
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csPolygon3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iPolygon3D)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPolygon3D::eiPolygon3D)
  SCF_IMPLEMENTS_INTERFACE(iPolygon3D)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csPolygon3D::csPolygon3D (iMaterialWrapper *material) :
    csObject(),
    vertices(4)
{
  VectorArray = GetStaticVectorArray();
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiPolygon3D);
  DG_TYPE ((csObject *)this, "csPolygon3D");
  thing = NULL;

  csPolygon3D::material = material;

  txt_info = NULL;

  portal = NULL;

  flags.SetAll (CS_POLY_LIGHTING | CS_POLY_COLLDET | CS_POLY_VISCULL);

  lightpatches = NULL;

  EnableTextureMapping (true);

  VectorArray->IncRef ();
#ifdef DO_HW_UVZ
  uvz = NULL;
  isClipped = false;
#endif
}

csPolygon3D::~csPolygon3D ()
{
  if (txt_info)
  {
    DG_UNLINK ((csObject *)this, txt_info);
    delete txt_info;
    txt_info = NULL;
  }

  if (portal && flags.Check (CS_POLY_DELETE_PORTAL))
  {
    portal->SetSector (NULL);
    delete portal;
    portal = NULL;
    if (thing) thing->RemovePortalPolygon (this);
  }

  if (thing)
  {
    while (lightpatches)
    {
      iDynLight* dl = lightpatches->GetLight ();
      if (dl)
        dl->RemoveAffectedLightingInfo (&(thing->scfiLightingInfo));
      thing->thing_type->lightpatch_pool->Free (lightpatches);
    }
  }
  VectorArray->DecRef ();
#ifdef DO_HW_UVZ
  delete[] uvz;
#endif
}

void csPolygon3D::SetParent (csThing *thing)
{
  if (thing == csPolygon3D::thing) return ;           // Nothing to do.
  if (csPolygon3D::thing && portal)
    csPolygon3D::thing->RemovePortalPolygon (this);
  csPolygon3D::thing = thing;
  if (portal) thing->AddPortalPolygon (this);
}

void csPolygon3D::EnableTextureMapping (bool enable)
{
  if (enable && txt_info != NULL) return;
  if (!enable && txt_info == NULL) return;

  if (enable)
  {
    txt_info = new csPolyTexLightMap ();
    DG_LINK ((csObject *)this, txt_info);
  }
  else
  {
    DG_UNLINK ((csObject *)this, txt_info);
    delete txt_info;
    txt_info = NULL;
  }
}

void csPolygon3D::CopyTextureType (iPolygon3D *ipt)
{
  csPolygon3D *pt = ipt->GetPrivateObject ();
  EnableTextureMapping (pt->IsTextureMappingEnabled ());

  csMatrix3 m;
  csVector3 v (0);
  csPolyTexLightMap *txtlmi_src = pt->GetLightMapInfo ();
  if (txtlmi_src)
  {
    txtlmi_src->GetTxtPlane ()->GetTextureSpace (m, v);
  }

  SetTextureSpace (m, v);
}

void csPolygon3D::Reset ()
{
  vertices.MakeEmpty ();
#ifdef DO_HW_UVZ
  if (uvz)
  {
    delete[] uvz;
    uvz = NULL;
  }
#endif
}

void csPolygon3D::SetCSPortal (iSector *sector, bool null)
{
  if (portal && portal->GetSector () == sector)
    return ;
  if (portal && flags.Check (CS_POLY_DELETE_PORTAL))
  {
    delete portal;
    portal = NULL;
    if (thing) thing->RemovePortalPolygon (this);
  }

  if (!null && !sector) return ;
  portal = new csPortal ();
  flags.Set (CS_POLY_DELETE_PORTAL);
  portal->flags.Reset (CS_PORTAL_WARP);
  if (sector)
    portal->SetSector (sector);
  else
    portal->SetSector (NULL);
  flags.Reset (CS_POLY_COLLDET);         // Disable CD by default for portals.
  if (thing) thing->AddPortalPolygon (this);

  //portal->SetTexture (texh->get_texture_handle ());
}

void csPolygon3D::SetPortal (csPortal *prt)
{
  if (portal && flags.Check (CS_POLY_DELETE_PORTAL))
  {
    portal->SetSector (NULL);
    delete portal;
    portal = NULL;
    if (thing) thing->RemovePortalPolygon (this);
  }

  portal = prt;
  flags.Set (CS_POLY_DELETE_PORTAL);
  flags.Reset (CS_POLY_COLLDET);         // Disable CD by default for portals.
  if (thing) thing->AddPortalPolygon (this);
}

bool csPolygon3D::Overlaps (csPolygon3D *overlapped)
{
  csPolygon3D *totest = overlapped;

  // Algorithm: if any of the vertices of the 'totest' polygon
  // is facing away from the front of this polygon (i.e. the vertex
  // cannot see this polygon) then there is a chance that this polygon
  // overlaps the other. If this is not the case then we can return false
  // already. Otherwise we have to see that the 'totest' polygon is
  // itself not facing away from this polygon. To test that we see if
  // there is a vertex of this polygon that is in front of the 'totest'
  // polygon. If that is the case then we return true.
  csPlane3 &this_plane = plane_obj;
  csPlane3 &test_plane = totest->plane_obj;
  int i;
  for (i = 0; i < totest->vertices.GetVertexCount (); i++)
  {
    if (this_plane.Classify (totest->Vobj (i)) >= SMALL_EPSILON)
    {
      int j;
      for (j = 0; j < vertices.GetVertexCount (); j++)
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

void csPolygon3D::SetMaterial (iMaterialWrapper *material)
{
  csPolygon3D::material = material;
}

iMaterialHandle *csPolygon3D::GetMaterialHandle ()
{
  return material ? material->GetMaterialHandle () : NULL;
}

void csPolygon3D::eiPolygon3D::SetTextureSpace (iPolyTxtPlane *plane)
{
  scfParent->SetTextureSpace (plane->GetPrivateObject ());
}

iThingState *csPolygon3D::eiPolygon3D::GetParent ()
{
  return &(scfParent->GetParent ()->scfiThingState);
}

void csPolygon3D::eiPolygon3D::CreatePlane (
  const csVector3 &iOrigin,
  const csMatrix3 &iMatrix)
{
  scfParent->SetTextureSpace (iMatrix, iOrigin);
}

bool csPolygon3D::eiPolygon3D::SetPlane (const char *iName)
{
  iPolyTxtPlane *ppl = scfParent->thing->thing_type->FindPolyTxtPlane (iName);
  if (!ppl) return false;
  scfParent->SetTextureSpace (ppl->GetPrivateObject ());
  return true;
}

bool csPolygon3D::IsTransparent ()
{
  if (!txt_info) return true;
  if (GetAlpha ()) return true;
#ifndef CS_USE_NEW_RENDERER
  if (txt_info->GetMixMode () != CS_FX_COPY) return true;
#endif // CS_USE_NEW_RENDERER

  iTextureHandle *txt_handle = GetMaterialHandle ()->GetTexture ();
  return txt_handle && ((txt_handle->GetAlphaMap () || txt_handle->GetKeyColor ()));
}

int csPolygon3D::Classify (const csPlane3 &pl)
{
  if (&GetPolyPlane () == &pl) return CS_POL_SAME_PLANE;
  if (csMath3::PlanesEqual (pl, GetPolyPlane ())) return CS_POL_SAME_PLANE;

  int i;
  int front = 0, back = 0;

  for (i = 0; i < GetVertices ().GetVertexCount (); i++)
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

int csPolygon3D::ClassifyX (float x)
{
  int i;
  int front = 0, back = 0;

  for (i = 0; i < GetVertices ().GetVertexCount (); i++)
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

int csPolygon3D::ClassifyY (float y)
{
  int i;
  int front = 0, back = 0;

  for (i = 0; i < GetVertices ().GetVertexCount (); i++)
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

int csPolygon3D::ClassifyZ (float z)
{
  int i;
  int front = 0, back = 0;

  for (i = 0; i < GetVertices ().GetVertexCount (); i++)
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

void csPolygon3D::ComputeNormal ()
{
  float A, B, C, D;
  PlaneNormal (&A, &B, &C);
  D = -A * Vobj (0).x - B * Vobj (0).y - C * Vobj (0).z;

  // By default the world space normal is equal to the object space normal.
  plane_obj.Set (A, B, C, D);
  plane_wor.Set (A, B, C, D);
}

void csPolygon3D::WorldToCameraPlane (
  const csReversibleTransform &t,
  const csVector3 &vertex1,
  csPlane3& plane_cam)
{
  t.Other2This (plane_wor, vertex1, plane_cam);
}

void csPolygon3D::ComputeCameraPlane (const csReversibleTransform& t,
  	csPlane3& pl)
{
  csVector3 cam_vert = t.Other2This (Vwor (0));
  WorldToCameraPlane (t, cam_vert, pl);
}

void csPolygon3D::ObjectToWorld (
  const csReversibleTransform &t,
  const csVector3 &vwor)
{
  t.This2Other (plane_obj, vwor, plane_wor);
  // This is not efficient and only needed in those cases where the
  // thing is really scaled. We have to see if this is a problem. Normally
  // it is a good thing to avoid calling csThing::Transform() to often.
  // So normally it should not be a problem.
  plane_wor.Normalize ();

  if (txt_info) txt_info->GetTxtPlane ()->ObjectToWorld (t);
  if (portal) portal->ObjectToWorld (t);
}

void csPolygon3D::HardTransform (const csReversibleTransform &t)
{
  if (portal) portal->HardTransform (t);
}

#define TEXW(t) ((t)->w_orig)
#define TEXH(t) ((t)->h)

void csPolygon3D::Finish ()
{
#ifndef CS_USE_NEW_RENDERER

#ifdef DO_HW_UVZ
  if (uvz)
  {
    delete[] uvz;
    uvz = NULL;
  }

  isClipped = false;
#endif
  if (thing->flags.Check (CS_ENTITY_NOLIGHTING))
    flags.Reset (CS_POLY_LIGHTING);

  if (IsTextureMappingEnabled ())
  {
    if (!material->GetMaterialHandle ()->GetTexture ())
    {
      EnableTextureMapping (false);
      return ;
    }
  }
  else
  {
    return;
  }

  txt_info->Setup (this, material);
  txt_info->tex->SetLightMap (NULL);
  if (portal)
    portal->SetFilter (material->GetMaterialHandle ()->GetTexture ());

  if (flags.Check (CS_POLY_LIGHTING))
  {
    int lmw = csLightMap::CalcLightMapWidth (txt_info->tex->w_orig);
    int lmh = csLightMap::CalcLightMapHeight (txt_info->tex->h);
    int max_lmw, max_lmh;
    thing->thing_type->engine->GetMaxLightmapSize (max_lmw, max_lmh);
    if ((lmw > max_lmw) || (lmh > max_lmh))
    {
      thing->thing_type->Notify ("Oversize lightmap (%dx%d > %dx%d) "
        "for polygon '%s'", lmw, lmh, 
	max_lmw, max_lmh, GetName());
    }
    else
    {
      csLightMap *lm = new csLightMap ();
      txt_info->tex->SetLightMap (lm);

      csColor ambient;
      thing->thing_type->engine->GetAmbientLight (ambient);
      lm->Alloc (txt_info->tex->w_orig, txt_info->tex->h,
      	int(ambient.red * 255.0f),
      	int(ambient.green * 255.0f),
      	int(ambient.blue * 255.0f));

      if (!thing->thing_type->G3D->IsLightmapOK (txt_info->GetPolyTex()))
      {
	thing->thing_type->Notify ("Renderer can't handle lightmap "
	  "for polygon '%s'", GetName());
	flags.Set (CS_POLY_LM_REFUSED, CS_POLY_LM_REFUSED);
      }

      lm->DecRef ();
    }
  }

#ifdef DO_HW_UVZ
  SetupHWUV ();
#endif
#endif // CS_USE_NEW_RENDERER
}

#ifdef DO_HW_UVZ
void csPolygon3D::SetupHWUV ()
{
  csVector3 v_o2t;
  csMatrix3 m_o2t;
  int i;

  txt_info->txt_plane->GetTextureSpace (m_o2t, v_o2t);
  uvz = new csVector3[GetVertexCount ()];

  for (i = 0; i < GetVertexCount (); i++)
  {
    uvz[i] = m_o2t * (Vobj (i) - v_o2t);
  }
}
#endif
float csPolygon3D::GetArea ()
{
  float area = 0.0f;

  // triangulize the polygon, triangles are (0,1,2), (0,2,3), (0,3,4), etc..
  int i;
  for (i = 0; i < vertices.GetVertexCount () - 2; i++)
    area += ABS (csMath3::Area3 (Vobj (0), Vobj (i + 1), Vobj (i + 2)));
  return area / 2.0f;
}

void csPolygon3D::SetTextureSpace (csPolyTxtPlane *txt_pl)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    txt_info->SetTxtPlane (txt_pl);
  }
}

void csPolygon3D::SetTextureSpace (csPolygon3D *copy_from)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    txt_info->SetTxtPlane (copy_from->GetLightMapInfo ()->GetTxtPlane ());
  }
}

void csPolygon3D::SetTextureSpace (
  const csMatrix3 &tx_matrix,
  const csVector3 &tx_vector)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    txt_info->NewTxtPlane (thing->thing_type);
    txt_info->GetTxtPlane ()->SetTextureSpace (tx_matrix, tx_vector);
  }
}

void csPolygon3D::SetTextureSpace (
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

  if (ABS (det) < 0.0001f)
  {
    thing->thing_type->Warn (
      "Warning: bad UV coordinates for poly '%s'!",
      GetName ());
    SetTextureSpace (p1, p2, 1);
    return ;
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
}

void csPolygon3D::SetTextureSpace (
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

void csPolygon3D::SetTextureSpace (
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
    txt_info->NewTxtPlane (thing->thing_type);
    txt_info->GetTxtPlane ()->SetTextureSpace (
          plane_obj,
          xo, yo, zo,
          x1, y1, z1,
          len1);
  }
}

void csPolygon3D::SetTextureSpace (
  const csVector3 &v_orig,
  const csVector3 &v1,
  float len1,
  const csVector3 &v2,
  float len2)
{
  ComputeNormal ();
  if (IsTextureMappingEnabled ())
  {
    txt_info->NewTxtPlane (thing->thing_type);
    txt_info->GetTxtPlane ()->SetTextureSpace (v_orig, v1, len1, v2, len2);
  }
}

void csPolygon3D::SetTextureSpace (
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

void csPolygon3D::DynamicLightDisconnect (iDynLight* dynlight)
{
  csLightPatch* lp = lightpatches;
  while (lp)
  {
    csLightPatch* lpnext = lp->GetNext ();
    if (lp->GetLight () == dynlight)
      thing->thing_type->lightpatch_pool->Free (lp);
    lp = lpnext;
  }
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

int csPolygon3D::AddVertex (int v)
{
  if (v >= thing->GetVertexCount ())
  {
    thing->thing_type->Bug (
        "Index number %d is too high for a polygon (max=%d) (polygon '%s')!",
        v,
        thing->GetVertexCount (),
        GetName () ? GetName () : "<noname>");
    return 0;
  }

  if (v < 0)
  {
    thing->thing_type->Bug ("Bad negative vertex index %d!", v);
    return 0;
  }

  vertices.AddVertex (v);
  return vertices.GetVertexCount () - 1;
}

int csPolygon3D::AddVertex (const csVector3 &v)
{
  int i = thing->AddVertex (v);
  AddVertex (i);
  return i;
}

int csPolygon3D::AddVertex (float x, float y, float z)
{
  int i = thing->AddVertex (x, y, z);
  AddVertex (i);
  return i;
}

void csPolygon3D::PlaneNormal (float *yz, float *zx, float *xy)
{
  float ayz = 0;
  float azx = 0;
  float axy = 0;
  int i, i1;
  float x1, y1, z1, x, y, z;

  i1 = GetVertices ().GetVertexCount () - 1;
  for (i = 0; i < GetVertices ().GetVertexCount (); i++)
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
    invd = qisqrt (sqd);

  *yz = ayz * invd;
  *zx = azx * invd;
  *xy = axy * invd;
}

// Clip a polygon against a plane.
void csPolygon3D::ClipPolyPlane (
  csVector3 *verts,
  int *num,
  bool mirror,
  csVector3 &v1,
  csVector3 &v2)
{
  int cw_offset = -1;
  int ccw_offset;
  bool first_vertex_side;
  csVector3 isect_cw, isect_ccw;
  csVector3 Plane_Normal;
  int i;

  // Do the check only once at beginning instead of twice during the routine.
  if (mirror)
    Plane_Normal = v2 % v1;
  else
    Plane_Normal = v1 % v2;

  // On which side is the first vertex?
  first_vertex_side = (Plane_Normal * (verts[(*num) - 1] - v1) > 0);

  for (i = 0; i < (*num) - 1; i++)
  {
    if ((Plane_Normal * (verts[i] - v1) > 0) != first_vertex_side)
    {
      cw_offset = i;
      break;
    }
  }

  if (cw_offset == -1)
  {
    // Return , if there is no intersection.
    if (first_vertex_side)
    {
      *num = 0;
    } // The whole polygon is behind the plane because the first is.
    return ;
  }

  for (ccw_offset = (*num) - 2; ccw_offset >= 0; ccw_offset--)
  {
    if ((Plane_Normal * (verts[ccw_offset] - v1) > 0) != first_vertex_side)
    {
      break;
    }
  }

  // Calculate the intersection points.
  i = cw_offset - 1;
  if (i < 0)
  {
    i = (*num) - 1;
  }

  float dummy;
  csIntersect3::Plane (
      verts[cw_offset],
      verts[i],
      Plane_Normal,
      v1,
      isect_cw,
      dummy);
  csIntersect3::Plane (
      verts[ccw_offset],
      verts[ccw_offset + 1],
      Plane_Normal,
      v1,
      isect_ccw,
      dummy);

  // Remove the obsolete point and insert the intersection points.
  if (first_vertex_side)
  {
    for (i = 0; i < ccw_offset - cw_offset + 1; i++)
      verts[i] = verts[i + cw_offset];
    verts[i] = isect_ccw;
    verts[i + 1] = isect_cw;
    *num = 3 + ccw_offset - cw_offset;
  }
  else
  {
    if (cw_offset + 1 < ccw_offset)
      for (i = 0; i < (*num) - ccw_offset - 1; i++)
        verts[cw_offset + 2 + i] = verts[ccw_offset + 1 + i];
    else if (cw_offset + 1 > ccw_offset)
      for (i = (*num) - 2 - ccw_offset; i >= 0; i--)
        verts[cw_offset + 2 + i] = verts[ccw_offset + 1 + i];

    verts[cw_offset] = isect_cw;
    verts[cw_offset + 1] = isect_ccw;
    *num = 2 + cw_offset + (*num) - ccw_offset - 1;
  }
}

bool csPolygon3D::ClipToPlane (
  csPlane3 *portal_plane,
  const csVector3 &v_w2c,
  csVector3 * &pverts,
  int &num_verts,
  bool cw)
{
  int i, i1;
  int cnt_vis, num_vertices;
  float r;
  bool zs, z1s;

  // Assume maximum 100 vertices! (@@@ HARDCODED LIMIT)
  static csVector3 verts[100];
  static bool vis[100];

  // Count the number of visible vertices for this polygon (note
  // that the transformation from world to camera space for all the
  // vertices has been done earlier).
  // If there are no visible vertices this polygon need not be drawn.
  cnt_vis = 0;
  num_vertices = GetVertices ().GetVertexCount ();
  for (i = 0; i < num_vertices; i++)
    if (Vcam (i).z >= 0)
    {
      cnt_vis++;
      break;
    }

  //if (Vcam (i).z >= SMALL_Z) cnt_vis++;
  if (cnt_vis == 0) return false;

  // Perform backface culling.
  // Note! The plane normal needs to be correctly calculated for this
  // to work!
  const csPlane3 &wplane = plane_wor;
  float cl = wplane.Classify (v_w2c);
  if (cw)
  {
    if (cl > 0) return false;
  }
  else
  {
    if (cl < 0) return false;
  }

  // If there is no portal polygon then everything is ok.
  if (!portal_plane)
  {
    // Copy the vertices to verts.
    for (i = 0; i < num_vertices; i++) verts[i] = Vcam (i);
    pverts = verts;
    num_verts = num_vertices;
    return true;
  }

  // Otherwise we will have to clip this polygon in 3D against the
  // portal polygon. This is to make sure that objects behind the
  // portal polygon are not accidently rendered.
  // First count how many vertices are before the portal polygon
  // (so are visible as seen from the portal).
  cnt_vis = 0;
  for (i = 0; i < num_vertices; i++)
  {
    vis[i] = portal_plane->Classify (Vcam (i)) <= -SMALL_EPSILON;
    if (vis[i]) cnt_vis++;
  }

  if (cnt_vis == 0) return false; // Polygon is not visible.

  // Copy the vertices to verts.
  for (i = 0; i < num_vertices; i++) verts[i] = Vcam (i);
  pverts = verts;

  // If all vertices are visible then everything is ok.
  if (cnt_vis == num_vertices)
  {
    num_verts = num_vertices;
    return true;
  }

  // We really need to clip.
  num_verts = 0;

  i1 = num_vertices - 1;
  z1s = vis[i1];
  for (i = 0; i < num_vertices; i++)
  {
    zs = vis[i];

    if (!z1s && zs)
    {
      csIntersect3::Plane (
          Vcam (i1),
          Vcam (i),
          *portal_plane,
          verts[num_verts],
          r);
      num_verts++;
      verts[num_verts++] = Vcam (i);
#ifdef DO_HW_UVZ
      isClipped = true;
#endif
    }
    else if (z1s && !zs)
    {
      csIntersect3::Plane (
          Vcam (i1),
          Vcam (i),
          *portal_plane,
          verts[num_verts],
          r);
      num_verts++;
#ifdef DO_HW_UVZ
      isClipped = true;
#endif
    }
    else if (z1s && zs)
    {
      verts[num_verts++] = Vcam (i);
    }

    z1s = zs;
    i1 = i;
  }

  return true;
}

#define EXPERIMENTAL_BUG_FIX  1

bool csPolygon3D::DoPerspective (
  csVector3 *source,
  int num_verts,
  csPolygon2D *dest,
  bool mirror,
  int fov, float shift_x, float shift_y,
  const csPlane3& plane_cam)
{
  csVector3 *ind, *end = source + num_verts;

  if (num_verts == 0) return false;
  dest->MakeEmpty ();

  // Classify all points as NORMAL (z>=SMALL_Z), NEAR (0<=z<SMALL_Z), or
  // BEHIND (z<0).  Use several processing algorithms: trivially accept if all
  // points are NORMAL, mixed process if some points are NORMAL and some
  // are not, special process if there are no NORMAL points, but some are
  // NEAR.  Assume that the polygon has already been culled if all points
  // are BEHIND.
  // Handle the trivial acceptance case:
  ind = source;
  while (ind < end)
  {
    if (ind->z >= SMALL_Z)
      dest->AddPerspective (*ind, fov, shift_x, shift_y);
    else
      break;
    ind++;
  }

  // Check if special or mixed processing is required
  if (ind == end) return true;

  // If we are processing a triangle (uv_coords != NULL) then
  // we stop here because the triangle is only visible if all
  // vertices are visible (this is not exactly true but it is
  // easier this way! @@@ CHANGE IN FUTURE).
#ifdef DO_HW_UVZ
  isClipped = true;
#endif

  csVector3 *exit = NULL, *exitn = NULL, *reenter = NULL, *reentern = NULL;
  csVector2 *evert = NULL;

  if (ind == source)
  {
    while (ind < end)
    {
      if (ind->z >= SMALL_Z)
      {
        reentern = ind;
        reenter = ind - 1;
        break;
      }

      ind++;
    }
  }
  else
  {
    exit = ind;
    exitn = ind - 1;
    evert = dest->GetLast ();
  }

  // Check if mixed processing is required
  if (exit || reenter)
  {
    bool needfinish = false;

    if (exit)
    {
      // we know where the polygon is no longer NORMAL, now we need to

      // to find out on which edge it becomes NORMAL again.
      while (ind < end)
      {
        if (ind->z >= SMALL_Z)
        {
          reentern = ind;
          reenter = ind - 1;
          break;
        }

        ind++;
      }

      if (ind == end)
      {
        reentern = source;
        reenter = ind - 1;
      }
      else
        needfinish = true;
    } /* if (exit) */
    else
    {
      // we know where the polygon becomes NORMAL, now we need to
      // to find out on which edge it ceases to be NORMAL.
      while (ind < end)
      {
        if (ind->z >= SMALL_Z)
          dest->AddPerspective (*ind, fov, shift_x, shift_y);
        else
        {
          exit = ind;
          exitn = ind - 1;
          break;
        }

        ind++;
      }

      if (ind == end)
      {
        exit = source;
        exitn = ind - 1;
      }

      evert = dest->GetLast ();
    }

    // Add the NEAR points appropriately.
#define MAX_VALUE 1000000.
    // First, for the exit point.
    float ex, ey, epointx, epointy;
    ex = exitn->z * exit->x - exitn->x * exit->z;
    ey = exitn->z * exit->y - exitn->y * exit->z;
    if (ABS (ex) < SMALL_EPSILON && ABS (ey) < SMALL_EPSILON)
    {
      // Uncommon special case:  polygon passes through origin.
      //plane->WorldToCamera (trans, source[0]);  //@@@ Why is this needed???
      ex = plane_cam.A ();
      ey = plane_cam.B ();
      if (ABS (ex) < SMALL_EPSILON && ABS (ey) < SMALL_EPSILON)
      {
        // Downright rare case:  polygon near parallel with viewscreen.
        ex = exit->x - exitn->x;
        ey = exit->y - exitn->y;
      }
    }

    if (ABS (ex) > ABS (ey))
    {
      if (ex > 0)
        epointx = MAX_VALUE;
      else
        epointx = -MAX_VALUE;
      epointy = (epointx - evert->x) * ey / ex + evert->y;
    }
    else
    {
      if (ey > 0)
        epointy = MAX_VALUE;
      else
        epointy = -MAX_VALUE;
      epointx = (epointy - evert->y) * ex / ey + evert->x;
    }

    // Next, for the reentry point.
    float rx, ry, rpointx, rpointy;

    // Perspective correct the point.
    float iz = fov / reentern->z;
    csVector2 rvert;
    rvert.x = reentern->x * iz + shift_x;
    rvert.y = reentern->y * iz + shift_y;

    if (reenter == exit && reenter->z > -SMALL_EPSILON)
    {
      rx = ex;
      ry = ey;
    }
    else
    {
      rx = reentern->z * reenter->x - reentern->x * reenter->z;
      ry = reentern->z * reenter->y - reentern->y * reenter->z;
    }

    if (ABS (rx) < SMALL_EPSILON && ABS (ry) < SMALL_EPSILON)
    {
      // Uncommon special case:  polygon passes through origin.
      //plane->WorldToCamera (trans, source[0]);  //@@@ Why is this needed?
      rx = plane_cam.A ();
      ry = plane_cam.B ();
      if (ABS (rx) < SMALL_EPSILON && ABS (ry) < SMALL_EPSILON)
      {
        // Downright rare case:  polygon near parallel with viewscreen.
        rx = reenter->x - reentern->x;
        ry = reenter->y - reentern->y;
      }
    }

    if (ABS (rx) > ABS (ry))
    {
      if (rx > 0)
        rpointx = MAX_VALUE;
      else
        rpointx = -MAX_VALUE;
      rpointy = (rpointx - rvert.x) * ry / rx + rvert.y;
    }
    else
    {
      if (ry > 0)
        rpointy = MAX_VALUE;
      else
        rpointy = -MAX_VALUE;
      rpointx = (rpointy - rvert.y) * rx / ry + rvert.x;
    }

#define QUADRANT(x, y)  ((y < x ? 1 : 0) ^ (x < -y ? 3 : 0))
#define MQUADRANT(x, y) ((y < x ? 3 : 0) ^ (x < -y ? 1 : 0))
    dest->AddVertex (epointx, epointy);
#if EXPERIMENTAL_BUG_FIX
    if (mirror)
    {
      int quad = MQUADRANT (epointx, epointy);
      int rquad = MQUADRANT (rpointx, rpointy);
      if (
        (quad == 0 && -epointx == epointy) ||
        (quad == 1 && epointx == epointy))
        quad++;
      if (
        (rquad == 0 && -rpointx == rpointy) ||
        (rquad == 1 && rpointx == rpointy))
        rquad++;
      while (quad != rquad)
      {
        epointx = (float)(quad & 2) ? MAX_VALUE : -MAX_VALUE;
        epointy = (float)(quad == 0 || quad == 3) ? MAX_VALUE : -MAX_VALUE;
        dest->AddVertex (epointx, epointy);
        quad = (quad + 1) & 3;
      }
    }
    else
    {
      int quad = QUADRANT (epointx, epointy);
      int rquad = QUADRANT (rpointx, rpointy);
      if (
        (quad == 0 && epointx == epointy) ||
        (quad == 1 && -epointx == epointy))
        quad++;
      if (
        (rquad == 0 && rpointx == rpointy) ||
        (rquad == 1 && -rpointx == rpointy))
        rquad++;
      while (quad != rquad)
      {
        epointx = (float)(quad & 2) ? -MAX_VALUE : MAX_VALUE;
        epointy = (float)(quad == 0 || quad == 3) ? MAX_VALUE : -MAX_VALUE;
        dest->AddVertex (epointx, epointy);
        quad = (quad + 1) & 3;
      }
    }
#endif
    dest->AddVertex (rpointx, rpointy);

    // Add the rest of the vertices, which are all NORMAL points.
    if (needfinish)
      while (ind < end) dest->AddPerspective (*ind++,
      	fov, shift_x, shift_y);
  } /* if (exit || reenter) */

  // Do special processing (all points are NEAR or BEHIND)
  else
  {
    if (mirror)
    {
      csVector3 *ind2 = end - 1;
      for (ind = source; ind < end; ind2 = ind, ind++)
        if (
          (
            ind->x -
            ind2->x
          ) *
              (ind2->y) -
              (ind->y - ind2->y) *
              (ind2->x) > -SMALL_EPSILON)
          return false;
      dest->AddVertex (MAX_VALUE, -MAX_VALUE);
      dest->AddVertex (MAX_VALUE, MAX_VALUE);
      dest->AddVertex (-MAX_VALUE, MAX_VALUE);
      dest->AddVertex (-MAX_VALUE, -MAX_VALUE);
    }
    else
    {
      csVector3 *ind2 = end - 1;
      for (ind = source; ind < end; ind2 = ind, ind++)
        if (
          (
            ind->x -
            ind2->x
          ) *
              (ind2->y) -
              (ind->y - ind2->y) *
              (ind2->x) < SMALL_EPSILON)
          return false;
      dest->AddVertex (-MAX_VALUE, -MAX_VALUE);
      dest->AddVertex (-MAX_VALUE, MAX_VALUE);
      dest->AddVertex (MAX_VALUE, MAX_VALUE);
      dest->AddVertex (MAX_VALUE, -MAX_VALUE);
    }
  }

  return true;
}

bool csPolygon3D::PointOnPolygon (const csVector3 &v)
{
  // First check if point is on the plane.
  csPlane3 &pl = plane_obj;
  float dot = pl.D () + pl.A () * v.x + pl.B () * v.y + pl.C () * v.z;
  if (ABS (dot) >= EPSILON) return false;

  // Check if 'v' is on the same side of all edges.
  int i, i1;
  bool neg = false, pos = false;
  i1 = GetVertices ().GetVertexCount () - 1;
  for (i = 0; i < GetVertices ().GetVertexCount (); i++)
  {
    float ar = csMath3::Area3 (v, Vobj (i1), Vobj (i));
    if (ar < 0)
      neg = true;
    else if (ar > 0)
      pos = true;
    if (neg && pos) return false;
    i1 = i;
  }

  return true;
}

bool csPolygon3D::IntersectRay (const csVector3 &start, const csVector3 &end)
{
  // First we do backface culling on the polygon with respect to

  // the starting point of the beam.
  csPlane3 &pl = plane_obj;
  float dot1 = pl.D () +
    pl.A () *
    start.x +
    pl.B () *
    start.y +
    pl.C () *
    start.z;
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
  i1 = GetVertices ().GetVertexCount () - 1;
  for (i = 0; i < GetVertices ().GetVertexCount (); i++)
  {
    csMath3::CalcNormal (normal, start, Vobj (i1), Vobj (i));
    if ((relend * normal) > 0) return false;
    i1 = i;
  }

  return true;
}

bool csPolygon3D::IntersectRayNoBackFace (
  const csVector3 &start,
  const csVector3 &end)
{
  // If this vector is perpendicular to the plane of the polygon we
  // need to catch this case here.
  csPlane3 &pl = plane_obj;
  float dot1 = pl.D () +
    pl.A () *
    start.x +
    pl.B () *
    start.y +
    pl.C () *
    start.z;
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
  i1 = GetVertices ().GetVertexCount () - 1;
  for (i = 0; i < GetVertices ().GetVertexCount (); i++)
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

bool csPolygon3D::IntersectSegment (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect,
  float *pr)
{
  if (!IntersectRay (start, end)) return false;
  return IntersectSegmentPlane (start, end, isect, pr);
}

bool csPolygon3D::IntersectSegmentPlane (
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

  denom = plane_obj.A () * (x2 - x1) +
    plane_obj.B () * (y2 - y1) +
    plane_obj.C () * (z2 - z1);
  if (ABS (denom) < SMALL_EPSILON) return false;  // Lines are parallel
  num = -(plane_obj.A () * x1 +
	  plane_obj.B () * y1 +
	  plane_obj.C () * z1 +
	  plane_obj.D ());
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

bool csPolygon3D::IntersectRayPlane (
  const csVector3 &start,
  const csVector3 &end,
  csVector3 &isect)
{
  float r;
  IntersectSegmentPlane (start, end, isect, &r);
  return r >= 0;
}

void csPolygon3D::InitializeDefault ()
{
  if (txt_info)
  {
    if (txt_info->tex->lm == NULL) return ;
    txt_info->tex->InitLightMaps ();
    txt_info->lightmap_up_to_date = false;
    return ;
  }
}

const char* csPolygon3D::ReadFromCache (iFile* file)
{
  if (txt_info)
  {
    if (txt_info->tex->lm == NULL) return NULL;
    const char* error = txt_info->tex->lm->ReadFromCache (
          file,
          txt_info->tex->w_orig,
          txt_info->tex->h,
          this,
          true,
	  thing->thing_type->engine);
    if (error != NULL)
    {
      txt_info->tex->InitLightMaps ();
    }
    txt_info->lightmap_up_to_date = true;
    return error;
  }

  return NULL;
}

bool csPolygon3D::WriteToCache (iFile* file)
{
  if (txt_info)
  {
    if (txt_info->tex->lm == NULL) return true;
    txt_info->lightmap_up_to_date = true;
    if (thing->thing_type->engine->GetLightingCacheMode ()
    	& CS_ENGINE_CACHE_WRITE)
      txt_info->tex->lm->Cache (file, this, NULL,
	  thing->thing_type->engine);
    return true;
  }

  return true;
}

void csPolygon3D::PrepareLighting ()
{
  if (!txt_info || !txt_info->tex->lm) return ;
  txt_info->tex->lm->ConvertToMixingMode ();
  txt_info->tex->lm->ConvertFor3dDriver (
      thing->thing_type->engine->GetLightmapsRequirePO2 (),
      thing->thing_type->engine->GetMaxLightmapAspectRatio ());
}

void csPolygon3D::FillLightMapDynamic (iFrustumView* lview)
{
  csFrustumContext *ctxt = lview->GetFrustumContext ();

  // We are working for a dynamic light. In this case we create
  // a light patch for this polygon.
  csLightPatch *lp = thing->thing_type->lightpatch_pool->Alloc ();
  csRef<iShadowBlock> sb = lview->CreateShadowBlock ();
  lp->SetShadowBlock (sb);
  AddLightpatch (lp);

  iFrustumViewUserdata* fvud = lview->GetUserdata ();
  iLightingProcessInfo* lpi = (iLightingProcessInfo*)fvud;
  iLight* l = lpi->GetLight ();
  csRef<iDynLight> dl = SCF_QUERY_INTERFACE (l, iDynLight);
  lp->SetLight (dl);

  csFrustum *light_frustum = ctxt->GetLightFrustum ();
  lp->Initialize (light_frustum->GetVertexCount ());

  // Copy shadow frustums.
  lp->GetShadowBlock ()->AddRelevantShadows (ctxt->GetShadows ());

  int i, mi;
  for (i = 0; i < lp->GetVertexCount (); i++)
  {
    mi = ctxt->IsMirrored () ? lp->GetVertexCount () - i - 1 : i;
    lp->GetVertex (i) = light_frustum->GetVertex (mi);
  }
}

bool csPolygon3D::MarkRelevantShadowFrustums (
  iFrustumView* lview,
  csPlane3 &plane)
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
      if (sfp == NULL)
      {
        shadow_it->MarkRelevant (true);
	continue;
      }

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
          i1 = GetVertexCount () - 1;
          for (i = 0; i < GetVertexCount (); i++)
          {
            j1 = sfp->GetVertexCount () - 1;

            float a1 = csMath3::Area3 (Vwor (i1), Vwor (i), sfp->Vwor (j1));
            for (j = 0; j < sfp->GetVertexCount (); j++)
            {
              float a = csMath3::Area3 (Vwor (i1), Vwor (i), sfp->Vwor (j));
              if (ABS (a) < EPSILON && ABS (a1) < EPSILON)
              {
                // The two points of the shadow frustum are on the same
                // edge as the current light frustum edge we are examining.
                // In this case we test if the orientation of the two edges
                // is different. If so then the shadow frustum is not
                // relevant.
                csVector3 d1 = Vwor (i) - Vwor (i1);
                csVector3 d2 = sfp->Vwor (j) - sfp->Vwor (j1);
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
	    if (!csIntersect3::Plane (center, Vwor (0), sfp->GetPolyPlane (),
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

bool csPolygon3D::MarkRelevantShadowFrustums (iFrustumView* lview)
{
  csPlane3 poly_plane = GetPolyPlane ();

  // First translate plane to center of frustum.
  poly_plane.DD += poly_plane.norm * lview->GetFrustumContext ()
  	->GetLightFrustum ()->GetOrigin ();
  poly_plane.Invert ();
  return MarkRelevantShadowFrustums (lview, poly_plane);
}

void csPolygon3D::CalculateLightingDynamic (iFrustumView *lview)
{
  csFrustum *light_frustum = lview->GetFrustumContext ()->GetLightFrustum ();
  const csVector3 &center = light_frustum->GetOrigin ();

  // If plane is not visible then return (backface culling).
  if (!csMath3::Visible (center, plane_wor)) return ;

  // Compute the distance from the center of the light
  // to the plane of the polygon.
  float dist_to_plane = GetPolyPlane ().Distance (center);

  // If distance is too small or greater than the radius of the light
  // then we have a trivial case (no hit).
  if (dist_to_plane < SMALL_EPSILON || dist_to_plane >= lview->GetRadius ())
    return ;

  csPortal *po;
  csFrustum *new_light_frustum;

  csVector3 *poly;
  int num_vertices;

  bool fill_lightmap = true;

  num_vertices = GetVertices ().GetVertexCount ();
  if (num_vertices > VectorArray->Limit ())
    VectorArray->SetLimit (num_vertices);
  poly = VectorArray->GetArray ();

  int j;
  if (lview->GetFrustumContext ()->IsMirrored ())
    for (j = 0; j < num_vertices; j++)
      poly[j] = Vwor (num_vertices - j - 1) - center;
  else
    for (j = 0; j < num_vertices; j++) poly[j] = Vwor (j) - center;

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

  if (min_sqdist >= lview->GetSquaredRadius ())
  {
    delete new_light_frustum;
    return ;
  }

  csFrustumContext *old_ctxt = lview->GetFrustumContext ();
  lview->CreateFrustumContext ();

  csFrustumContext *new_ctxt = lview->GetFrustumContext ();

  // @@@ CHECK IF SetLightFrustum doesn't cause memory leaks!!!
  new_ctxt->SetLightFrustum (new_light_frustum);

  // Mark all shadow frustums in 'new_lview' which are relevant. i.e.
  // which are inside the light frustum and are not obscured (shadowed)
  // by other shadow frustums.
  // We also give the polygon plane to MarkRelevantShadowFrustums so that
  // all shadow frustums which start at the same plane are discarded as
  // well.
  // FillLightMap() will use this information and
  // csPortal::CalculateLighting() will also use it!!
  po = GetPortal ();
  if (!MarkRelevantShadowFrustums (lview)) goto stop;

  // Update the lightmap given light and shadow frustums in new_lview.
  if (fill_lightmap) FillLightMapDynamic (lview);

  if (po)
  {
    if (!po->flags.Check (CS_PORTAL_MIRROR))
      po->CheckFrustum (lview, GetAlpha ());
  }

stop:
  lview->RestoreFrustumContext (old_ctxt);
}

void csPolygon3D::CalculateLightingStatic (iFrustumView *lview,
	csLightingPolyTexQueue* lptq, bool vis)
{
  bool do_smooth = GetParent ()->GetSmoothingFlag ();

  bool maybeItsVisible = false;
  csFrustum *light_frustum = lview->GetFrustumContext ()->GetLightFrustum ();
  const csVector3 &center = light_frustum->GetOrigin ();

  // If plane is not visible then return (backface culling).
  if (!csMath3::Visible (center, plane_wor)) 
    if (do_smooth)
      maybeItsVisible = true;
    else
      return;

  // Compute the distance from the center of the light
  // to the plane of the polygon.
  float dist_to_plane = GetPolyPlane ().Distance (center);

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
  bool calc_lmap;
  if (txt_info)
    calc_lmap = txt_info->tex && txt_info->tex->lm
    	&& !txt_info->lightmap_up_to_date;
  else
    calc_lmap = true;

  // Update the lightmap given light and shadow frustums in lview.
  if (calc_lmap)
    FillLightMapStatic (lview, lptq, vis);

  if (maybeItsVisible) 
    return;

  csPortal *po = GetPortal ();

  // @@@@@@@@ We temporarily don't do lighting through space-warping portals.
  // Needs to be fixed soon!!!
  if (po && !po->flags.Check (CS_PORTAL_WARP))
  {
    csFrustumContext *old_ctxt = lview->GetFrustumContext ();
    lview->CreateFrustumContext ();

    csFrustumContext *new_ctxt = lview->GetFrustumContext ();

    int num_vertices = GetVertices ().GetVertexCount ();
    if (num_vertices > VectorArray->Limit ())
      VectorArray->SetLimit (num_vertices);

    csVector3 *poly = VectorArray->GetArray ();

    int j;
    if (old_ctxt->IsMirrored ())
      for (j = 0; j < num_vertices; j++)
        poly[j] = Vwor (num_vertices - j - 1) - center;
    else
      for (j = 0; j < num_vertices; j++) poly[j] = Vwor (j) - center;

    csFrustum *new_light_frustum = light_frustum->Intersect (
        poly,
        num_vertices);

    // @@@ Check if this isn't a memory leak.
    new_ctxt->SetLightFrustum (new_light_frustum);
    if (new_light_frustum)
    {
      po->CheckFrustum ((iFrustumView *)lview, GetAlpha ());
    }

    lview->RestoreFrustumContext (old_ctxt);
  }
}

void csPolygon3D::FillLightMapStatic (iFrustumView *lview,
	csLightingPolyTexQueue* lptq, bool vis)
{
  if (txt_info)
  {
    if (txt_info->lightmap_up_to_date) return ;
    txt_info->tex->FillLightMap (lview, lptq, vis, this);
  }
}

