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

#include "sysdef.h"
#include "qint.h"
#include "csengine/sysitf.h"
#include "csengine/polygon/polygon.h"
#include "csengine/polygon/pol2d.h"
#include "csengine/polygon/polytext.h"
#include "csengine/texture.h"
#include "csengine/polygon/polyplan.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/light/light.h"
#include "csengine/light/dynlight.h"
#include "csengine/light/lghtmap.h"
#include "csengine/camera.h"
#include "csengine/polygon/portal.h"
#include "csengine/dumper.h"
#include "csobject/nameobj.h"
#include "igraph3d.h"
#include "itexture.h"
#include "itxtmgr.h"

// Option variable: force lightmap recalculation?
bool csPolygon3D::do_force_recalc = false;
bool csPolygon3D::do_not_force_recalc = false;
// Option variable: shadow mipmap size
int csPolygon3D::def_mipmap_size = 16;

//---------------------------------------------------------------------------

IMPLEMENT_UNKNOWN_NODELETE (csPolygon3D)

#ifndef BUGGY_EGCS_COMPILER

BEGIN_INTERFACE_TABLE (csPolygon3D)
	IMPLEMENTS_COMPOSITE_INTERFACE (Polygon3D)
END_INTERFACE_TABLE ()

#else

const INTERFACE_ENTRY *csPolygon3D::GetInterfaceTable ()
{
  static INTERFACE_ENTRY InterfaceTable[2];
  InterfaceTable[0].pIID = &IID_IPolygon3D;
  InterfaceTable[0].pfnFinder = ENTRY_IS_OFFSET;
  InterfaceTable[0].dwData = COMPOSITE_OFFSET(csPolygon3D, IPolygon3D, IPolygon3D, m_xPolygon3D);
  InterfaceTable[1].pIID = 0;
  InterfaceTable[1].pfnFinder = 0;
  InterfaceTable[1].dwData = 0;
  return InterfaceTable;
}

#endif

CSOBJTYPE_IMPL(csPolygon3D,csObject);

csPolygon3D::csPolygon3D (csTextureHandle* texture)
	: csObject (), csPolygonInt (), theDynLight (0)
{
  vertices_idx = NULL;
  max_vertices = num_vertices = 0;

  CHK (tex = new csPolyTexture ());
  tex->SetPolygon (this);
  CHK (tex1 = new csPolyTexture ());
  tex1->SetPolygon (this);
  CHK (tex2 = new csPolyTexture ());
  tex2->SetPolygon (this);
  CHK (tex3 = new csPolyTexture ());
  tex3->SetPolygon (this);

  txtMM = texture;
  if (texture) SetTexture (texture);
  lightmap = lightmap1 = lightmap2 = lightmap3 = NULL;

  delete_tex_info = true;

  plane = NULL;
  delete_plane = false;

  portal = NULL;
  delete_portal = false;

  sector = NULL;
  orig_poly = NULL;
  dont_draw = false;

  no_mipmap = false;
  no_lighting = false;
  cosinus_factor = -1;
  lightmap_up_to_date = false;

  lightpatches = NULL;
  uv_coords = NULL;
  colors = NULL;

  use_flat_color = false;
}

csPolygon3D::csPolygon3D (csPolygon3D& poly) : csObject (), csPolygonInt ()
{
  const char* tname = csNameObject::GetName(poly);
  if (tname) csNameObject::AddName(*this, tname);

  theDynLight = poly.theDynLight;
  vertices_idx = NULL;
  max_vertices = num_vertices = 0;

  poly_set = poly.poly_set;
  sector = poly.sector;

  portal = poly.portal;
  delete_portal = false;	// This polygon is no owner

  plane = poly.plane;
  delete_plane = false;		// This polygon is no owner

  txtMM = poly.txtMM;

  tex = poly.tex;
  tex1 = poly.tex1;
  tex2 = poly.tex2;
  tex3 = poly.tex3;

  lightmap = lightmap1 = lightmap2 = lightmap3 = NULL;

  delete_tex_info = false;
  poly.dont_draw = true;
  dont_draw = false;
  orig_poly = poly.orig_poly ? poly.orig_poly : &poly;

  no_mipmap = poly.no_mipmap;
  no_lighting = poly.no_lighting;
  cosinus_factor = poly.cosinus_factor;
  lightmap_up_to_date = false;

  lightpatches = NULL;
  uv_coords = NULL;
  colors = NULL;

  flat_color = poly.flat_color;
  use_flat_color = poly.use_flat_color;
}

csPolygon3D::~csPolygon3D ()
{
  CHK (delete [] uv_coords);
  CHK (delete [] colors);
  CHK (delete [] vertices_idx);
  if (delete_tex_info)
  {
    CHK (delete tex);
    CHK (delete tex1);
    CHK (delete tex2);
    CHK (delete tex3);
    CHK (delete lightmap);
    CHK (delete lightmap1);
    CHK (delete lightmap2);
    CHK (delete lightmap3);
  }
  if (plane && delete_plane) CHKB (delete plane);
  if (portal && delete_portal) CHKB (delete portal);
  while (lightpatches) CHKB (delete lightpatches);
}

void csPolygon3D::Reset ()
{
  num_vertices = max_vertices = 0;
  CHK (delete [] vertices_idx);
  vertices_idx = NULL;
  ResetUV ();
}

void csPolygon3D::SetUV (int i, float u, float v)
{
  if (!uv_coords) CHKB (uv_coords = new csVector2 [num_vertices]);
  uv_coords[i].x = u;
  uv_coords[i].y = v;
}

void csPolygon3D::ResetUV ()
{
  CHK (delete [] uv_coords);
  uv_coords = NULL;
  CHK (delete [] colors);
  colors = NULL;
}

void csPolygon3D::SetColor (int i, float r, float g, float b)
{
  if (!colors)
  {
    CHK (colors = new csColor [num_vertices]);
    int j;
    for (j = 0 ; j < num_vertices ; j++)
      colors[j].Set (0, 0, 0);
  }
  colors[i].Set (r, g, b);
}

void csPolygon3D::SetCSPortal (csSector* sector)
{
  if (portal && portal->PortalType () == PORTAL_CS)
  {
    if (((csPortalCS*)portal)->GetSector () == sector) return;
  }
  if (portal && delete_portal) { CHK (delete portal); portal = NULL; }
  if (!sector) return;
  csPortalCS* csp;
  CHK (csp = new csPortalCS);
  portal = (csPortal*)csp;
  delete_portal = true;
  portal->DisableSpaceWarping ();
  csp->SetSector (sector);
  //portal->SetTexture (txtMM->get_texture_handle ());
}

void csPolygon3D::SetPortal (csPortal* prt)
{
  if (portal && delete_portal) { CHK (delete portal); portal = NULL; }
  portal = prt;
  delete_portal = true;
}

void csPolygon3D::SplitWithPlane (csVector3* front, int& front_n,
				  csVector3* back, int& back_n, csPlane& plane)
{
  csVector3 ptA, ptB;
  float sideA, sideB;
  ptA = Vwor (num_vertices - 1);
  sideA = plane.Classify (ptA);

  front_n = back_n = 0;

  for (int i = -1 ; ++i < num_vertices ; )
  {
    ptB = Vwor (i);
    sideB = plane.Classify (ptB);
    if (sideB > 0)
    {
      if (sideA < 0)
      {
	// Compute the intersection point of the line
	// from point A to point B with the partition
	// plane. This is a simple ray-plane intersection.
	csVector3 v = ptB; v -= ptA;
	float sect = - plane.Classify (ptA) / ( plane.Normal () * v ) ;
	v *= sect; v += ptA;
	*back++ = *front++ = v;
	back_n++; front_n++;
      }
      *back++ = ptB;
      back_n++;
    }
    else if (sideB < 0)
    {
      if (sideA > 0)
      {
	// Compute the intersection point of the line
	// from point A to point B with the partition
	// plane. This is a simple ray-plane intersection.
	csVector3 v = ptB; v -= ptA;
	float sect = - plane.Classify (ptA) / ( plane.Normal () * v );
	v *= sect; v += ptA;
	*back++ = *front++ = v;
	back_n++; front_n++;
      }
      *front++ = ptB;
      front_n++;
    }
    else
    {
      *back++ = *front++ = ptB;
      back_n++; front_n++;
    }
    ptA = ptB;
    sideA = sideB;
  }
}

void csPolygon3D::SetTexture (csTextureHandle* texture)
{
  txtMM = texture;
  tex->SetMipmapLevel (0);
  tex1->SetMipmapLevel (1);
  tex2->SetMipmapLevel (2);
  tex3->SetMipmapLevel (3);
}

ITextureHandle* csPolygon3D::GetTextureHandle ()
{
  return txtMM ? txtMM->GetTextureHandle () : (ITextureHandle*)NULL;
}


csPolyTexture* csPolygon3D::GetPolyTex (int mipmap)
{
  switch (mipmap)
  {
    case 0: return tex;
    case 1: return tex1;
    case 2: return tex2;
    case 3: return tex3;
  }
  return tex;
}

bool csPolygon3D::IsTransparent ()
{
  if (GetAlpha ())
    return true;
  bool transp;
  GetTextureHandle ()->GetTransparent (transp);
  return transp;
}

bool csPolygon3D::SamePlane (csPolygonInt* p)
{
  if (((csPolygon3D*)p)->plane == plane) return true;
  return ((csPolygon3D*)p)->plane->Close (plane);
}

int csPolygon3D::Classify (csPolygonInt* spoly)
{
  if (SamePlane (spoly)) return POL_SAME_PLANE;

  int i;
  int front = 0, back = 0;
  csPolygon3D* poly = (csPolygon3D*)spoly;
  csPolyPlane* pl = poly->GetPlane ();

  for (i = 0 ; i < num_vertices ; i++)
  {
    float dot = pl->Classify (Vwor (i));
    if (ABS (dot) < SMALL_EPSILON) dot = 0;
    if (dot > 0) back++;
    else if (dot < 0) front++;
  }
  if (back == 0) return POL_FRONT;
  if (front == 0) return POL_BACK;
  return POL_SPLIT_NEEDED;
}

void csPolygon3D::ComputeNormal ()
{
  float A, B, C, D;
  PlaneNormal (&A, &B, &C);
  D = -A*Vobj (0).x - B*Vobj (0).y - C*Vobj (0).z;

  // By default the world space normal is equal to the object space normal.
  plane->GetObjectPlane ().Set (A, B, C, D);
  plane->GetWorldPlane ().Set (A, B, C, D);
}

void csPolygon3D::ObjectToWorld (const csReversibleTransform& t)
{
  plane->ObjectToWorld (t, Vwor (0));
  if (portal) portal->ObjectToWorld (t);
}

#define TEXW(t) ((t)->w_orig)
#define TEXH(t) ((t)->h)

void csPolygon3D::Finish ()
{
  if (orig_poly) return;
  if (uv_coords || use_flat_color) return;

  tex->SetTextureHandle (txtMM->GetTextureHandle ());
  tex1->SetTextureHandle (txtMM->GetTextureHandle ());
  tex2->SetTextureHandle (txtMM->GetTextureHandle ());
  tex3->SetTextureHandle (txtMM->GetTextureHandle ());
  if (portal) portal->SetTexture (txtMM->GetTextureHandle ());

  tex->CreateBoundingTextureBox (); tex->lm = NULL;
  tex1->CreateBoundingTextureBox (); tex1->lm = NULL;
  tex2->CreateBoundingTextureBox (); tex2->lm = NULL;
  tex3->CreateBoundingTextureBox (); tex3->lm = NULL;

  lightmap = lightmap1 = lightmap2 = lightmap3 = NULL;
  if (!no_lighting && TEXW(tex)*TEXH(tex) < 1000000)
  {
    CHK (lightmap = new csLightMap ());
    int r, g, b;
    GetSector ()->GetAmbientColor (r, g, b);
    lightmap->Alloc (TEXW(tex), TEXH(tex), def_mipmap_size, r, g, b);
    tex->SetMipmapSize (def_mipmap_size); tex->lm = lightmap;
  }
}

#define USE_SUBTEX_OPT 1

void csPolygon3D::CreateLightmaps (IGraphics3D* g3d)
{
  if (!lightmap) return;

  CHK (delete lightmap1); lightmap1 = NULL;
  CHK (delete lightmap2); lightmap2 = NULL;
  CHK (delete lightmap3); lightmap3 = NULL;

  if (orig_poly) return;

  ITextureManager* txtmgr;
  g3d->GetTextureManager (&txtmgr);
  bool vn;
  txtmgr->GetVeryNice (vn);

  if (vn)
    tex1->SetMipmapSize (def_mipmap_size);
  else
    tex1->SetMipmapSize (def_mipmap_size>>1);
# if !USE_SUBTEX_OPT
  tex1->lm = lightmap;
# else
  CHK (lightmap1 = new csLightMap ());
  tex1->lm = lightmap1;
  lightmap1->CopyLightmap (lightmap);
# endif

  tex2->SetMipmapSize (tex1->mipmap_size>>1);

  // @@@ Normally the two mipmap levels can share the same lightmap.
  // But with the new sub-texture optimization in combination with the dynamic
  // lighting extension this gives some problems. For the moment we just copy
  // the lightmaps even though they are actually the same.
# if !USE_SUBTEX_OPT
  tex2->lm = lightmap;
# else
  CHK (lightmap2 = new csLightMap ());
  tex2->lm = lightmap2;
  lightmap2->CopyLightmap (lightmap);
# endif

  if (tex2->mipmap_size < 1)
  {
    tex2->SetMipmapSize (1);
#   if !USE_SUBTEX_OPT
    CHK (delete lightmap2);
#   endif
    CHK (lightmap2 = new csLightMap ());
    lightmap2->MipmapLightmap (TEXW(tex2), TEXH(tex2), 1, lightmap, TEXW(tex1), TEXH(tex1), tex1->mipmap_size);
    tex2->lm = lightmap2;

    tex3->SetMipmapSize (1);
    CHK (lightmap3 = new csLightMap ());
    lightmap3->MipmapLightmap (TEXW(tex3), TEXH(tex3), 1, lightmap2, TEXW(tex2), TEXH(tex2), tex2->mipmap_size);
    tex3->lm = lightmap3;
  }
  else
  {
    tex3->SetMipmapSize (tex2->mipmap_size>>1);
#   if !USE_SUBTEX_OPT
    tex3->lm = lightmap;
#   else
    CHK (lightmap3 = new csLightMap ());
    tex3->lm = lightmap3;
    lightmap3->CopyLightmap (lightmap);
#   endif

    if (tex3->mipmap_size < 1)
    {
      tex3->SetMipmapSize (1);
#     if !USE_SUBTEX_OPT
      CHK (delete lightmap3);
#     endif
      CHK (lightmap3 = new csLightMap ());
      lightmap3->MipmapLightmap (TEXW(tex3), TEXH(tex3), 1, lightmap, TEXW(tex2), TEXH(tex2), tex2->mipmap_size);
      tex3->lm = lightmap3;
    }
  }

  int aspect;
  bool po2 = (g3d->NeedsPO2Maps() == S_OK);

  g3d->GetMaximumAspectRatio(aspect);

  if (lightmap) lightmap->ConvertFor3dDriver (po2, aspect);
  if (lightmap1) lightmap1->ConvertFor3dDriver (po2, aspect);
  if (lightmap2) lightmap2->ConvertFor3dDriver (po2, aspect);
  if (lightmap3) lightmap3->ConvertFor3dDriver (po2, aspect);
}

void csPolygon3D::SetTextureSpace (csPolyPlane* pl)
{
  if (plane && delete_plane) CHKB (delete plane);
  plane = pl;
  delete_plane = false;

  ComputeNormal ();
}

void csPolygon3D::SetTextureSpace (csPolygon3D* copy_from)
{
  if (plane && delete_plane) CHKB (delete plane);
  plane = copy_from->plane;
  delete_plane = false;

  ComputeNormal ();
}

void csPolygon3D::SetTextureSpace (csMatrix3& tx_matrix, csVector3& tx_vector)
{
  if (plane && delete_plane) CHKB (delete plane);
  CHK (plane = new csPolyPlane ());
  delete_plane = true;

  ComputeNormal ();
  plane->SetTextureSpace (tx_matrix, tx_vector);
}

void csPolygon3D::SetTextureSpace (csVector3& v_orig, csVector3& v1, float len1)
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
	float xo, float yo, float zo,
	float x1, float y1, float z1,
	float len1)
{
  if (plane && delete_plane) CHKB (delete plane);
  CHK (plane = new csPolyPlane ());
  delete_plane = true;

  ComputeNormal ();
  plane->SetTextureSpace (xo, yo, zo, x1, y1, z1, len1);
}

void csPolygon3D::SetTextureSpace (
	float xo, float yo, float zo,
	float x1, float y1, float z1,
	float len1,
	float x2, float y2, float z2,
	float len2)
{
  if (plane && delete_plane) CHKB (delete plane);
  CHK (plane = new csPolyPlane ());
  delete_plane = true;

  ComputeNormal ();
  plane->SetTextureSpace (xo, yo, zo, x1, y1, z1, len1, x2, y2, z2, len2);
}


void csPolygon3D::MakeDirtyDynamicLights ()
{
  if (tex) tex->MakeDirtyDynamicLights ();
  if (tex1) tex1->MakeDirtyDynamicLights ();
  if (tex2) tex2->MakeDirtyDynamicLights ();
  if (tex3) tex3->MakeDirtyDynamicLights ();
}

void csPolygon3D::UnlinkLightpatch (csLightPatch* lp)
{
  if (lp->next_poly) lp->next_poly->prev_poly = lp->prev_poly;
  if (lp->prev_poly) lp->prev_poly->next_poly = lp->next_poly;
  else lightpatches = lp->next_poly;
  lp->prev_poly = lp->next_poly = NULL;
  lp->polygon = NULL;
  MakeDirtyDynamicLights ();
}

void csPolygon3D::AddLightpatch (csLightPatch* lp)
{
  lp->next_poly = lightpatches;
  lp->prev_poly = NULL;
  if (lightpatches) lightpatches->prev_poly = lp;
  lightpatches = lp;
  lp->polygon = this;
  MakeDirtyDynamicLights ();
}

int csPolygon3D::AddVertex (int v)
{
  if (!vertices_idx)
  {
    max_vertices = 4;
    CHK (vertices_idx = new int [max_vertices]);
  }
  while (num_vertices >= max_vertices)
  {
    max_vertices += 2;
    CHK (int* new_vertices_idx = new int [max_vertices]);
    memcpy (new_vertices_idx, vertices_idx, sizeof (int)*num_vertices);
    CHK (delete [] vertices_idx);
    vertices_idx = new_vertices_idx;
  }

  vertices_idx[num_vertices++] = v;
  return num_vertices-1;
}

int csPolygon3D::AddVertex (csVector3& v)
{
  int i = poly_set->AddVertexSmart (v);
  AddVertex (i);
  return i;
}

int csPolygon3D::AddVertex (float x, float y, float z)
{
  int i = poly_set->AddVertexSmart (x, y, z);
  AddVertex (i);
  return i;
}


enum InFlag { IN_P, IN_Q, IN_UNKNOWN };

void csPolygon3D::PlaneNormal (float* yz, float* zx, float* xy)
{
  float ayz = 0;
  float azx = 0;
  float axy = 0;
  int i, i1;
  float x1, y1, z1, x, y, z;

  i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; i++)
  {
    x = Vwor (i).x;
    y = Vwor (i).y;
    z = Vwor (i).z;
    x1 = Vwor (i1).x;
    y1 = Vwor (i1).y;
    z1 = Vwor (i1).z;
    ayz += (z1+z) * (y-y1);
    azx += (x1+x) * (z-z1);
    axy += (y1+y) * (x-x1);
    i1 = i;
  }

  float d = sqrt (ayz*ayz + azx*azx + axy*axy);

  if (d < SMALL_EPSILON) d = SMALL_EPSILON;

  *yz = ayz / d;
  *zx = azx / d;
  *xy = axy / d;
}


bool csPolygon3D::ClipPoly (csVector3* frustrum, int m, bool mirror, csVector3** dest, int* num_dest)
{
  int i, i1;
  if (!frustrum)
  {
    // Infinite frustrum.
    CHK (csVector3* dd = new csVector3 [num_vertices]);
    *dest = dd;
        if(!mirror)
      for (i = 0 ; i < num_vertices ; i++)
        dd[i] = Vcam (i);
        else
      for (i = 0 ; i < num_vertices ; i++)
        dd[i] = Vcam (num_vertices-i-1);
    *num_dest = num_vertices;
    return true;
  }

  CHK (csVector3* dd = new csVector3 [num_vertices+m]);     // For safety
  *dest = dd;
  if(!mirror)
    for (i = 0 ; i <num_vertices ; i++)
      dd[i] = Vcam (i);
  else
    for (i = 0 ; i <num_vertices ; i++)
      dd[i] = Vcam (num_vertices-i-1);
  *num_dest = num_vertices;

  i1 = -1;
  csVector3 *p = frustrum + m;

  for (i = -m-1 ; ++i ; )
  {
    ClipPolyPlane (dd, num_dest, mirror, p[i1], p[i]);
    if ((*num_dest) < 3)
    {
      CHK (delete [] dd);
      *dest = NULL;
      return false;
    }
    i1 = i;
  }

  return true;
}

// Clip a polygon against a plane.
void csPolygon3D::ClipPolyPlane (csVector3* verts, int* num, bool mirror,
  csVector3& v1, csVector3& v2)
{
  int cw_offset = -1;
  int ccw_offset;
  bool first_vertex_side;
  csVector3 isect_cw,isect_ccw;
  csVector3 Plane_Normal;
  int i;

  //  do the check only once at the beginning instead of twice during the routine.
  if (mirror)
    Plane_Normal = v2%v1;
  else
    Plane_Normal = v1%v2;

  //  On which side is the first vertex?
  first_vertex_side = (Plane_Normal*(verts[(*num) -1] -v1) > 0);

  for (i = 0; i < (*num) - 1; i++)
  {
    if ( (Plane_Normal*(verts[i] - v1) > 0) != first_vertex_side)
      {cw_offset = i;break;}
  }

  if (cw_offset == -1)
  {
    //  Return , if there is no intersection.
    if (first_vertex_side)
      {*num = 0;}      // The whole polygon is behind the plane because the first is.
    return;
  }

  for (ccw_offset = (*num) -2; ccw_offset >= 0; ccw_offset--)
  {
    if ((Plane_Normal*(verts[ccw_offset] - v1) > 0) != first_vertex_side)
      {break;}
  }

  // calculate the intersection points.
  i = cw_offset - 1;
  if (i < 0) {i = (*num) -1;}
  csIntersect3::Plane (verts[cw_offset], verts[i], Plane_Normal, v1,isect_cw);
  csIntersect3::Plane (verts[ccw_offset], verts[ccw_offset + 1],Plane_Normal, v1, isect_ccw);

  // remove the obsolete point and insert the intersection points.
  if (first_vertex_side)
  {
    for (i = 0; i < ccw_offset - cw_offset + 1; i++)
      verts[i] = verts[i + cw_offset];
    verts[i] = isect_ccw;
    verts[i+1] = isect_cw;
    *num = 3 + ccw_offset - cw_offset;
  }
  else
  {
    if (cw_offset + 1 < ccw_offset)
      for (i = 0; i < (*num) - ccw_offset - 1; i++)
        verts[cw_offset + 2 + i] = verts[ccw_offset + 1 + i];
    else if (cw_offset + 1 > ccw_offset)
      for (i = (*num) - 2 - ccw_offset;i >= 0; i--)
        verts[cw_offset + 2 + i] = verts[ccw_offset + 1 + i];

    verts[cw_offset] = isect_cw;
    verts[cw_offset+1] = isect_ccw;
    *num = 2 + cw_offset + (*num) - ccw_offset - 1;
  }
}

bool csPolygon3D::ClipFrustrum (csVector3& center, csVector3* frustrum, int num_frustrum, bool mirror,
	csVector3** new_frustrum, int* new_num_frustrum)
{
  //if (!plane->VisibleFromPoint (center)) return false;
  if (!plane->VisibleFromPoint (center) || ABS (plane->Classify (center)) < SMALL_EPSILON) return false;
  return ClipPoly (frustrum, num_frustrum, mirror, new_frustrum, new_num_frustrum);
}

bool csPolygon3D::ClipToPlane (csPlane* portal_plane, const csVector3& v_w2c,
	csVector3*& pverts, int& num_verts, bool cw)
{
  int i, i1, cnt_vis;
  float r;
  bool zs, z1s;

  // Assume maximum 100 vertices! (@@@ HARDCODED LIMIT)
  static csVector3 verts[100];
  bool vis[100];

  // Count the number of visible vertices for this polygon (note
  // that the transformation from world to camera space for all the
  // vertices has been done earlier).
  // If there are no visible vertices this polygon need not be drawn.
  cnt_vis = 0;
  for (i = 0 ; i < num_vertices ; i++)
    if (Vcam (i).z >= 0) cnt_vis++;
    //if (Vcam (i).z >= SMALL_Z) cnt_vis++;
  if (cnt_vis == 0) return false;

  // Perform backface culling.
  // Note! The plane normal needs to be correctly calculated for this
  // to work! @@@ can be optimized
  if (plane->VisibleFromPoint (v_w2c) != cw && plane->Classify (v_w2c) != 0) return false;

  // Copy the vertices to verts.
  for (i = 0 ; i < num_vertices ; i++) verts[i] = Vcam (i);
  pverts = verts;

  // If there is no portal polygon then everything is ok.
  if (!portal_plane) { num_verts = num_vertices; return true; }

  // Otherwise we will have to clip this polygon in 3D against the
  // portal polygon. This is to make sure that objects behind the
  // portal polygon are not accidently rendered.

  // First count how many vertices are before the portal polygon
  // (so are visible as seen from the portal).
  cnt_vis = 0;
  for (i = 0 ; i < num_vertices ; i++)
  {
    vis[i] = csMath3::Visible(Vcam(i), *portal_plane);
    if (vis[i]) cnt_vis++;
  }

  if (cnt_vis == 0) return false; // Polygon is not visible.

  // If all vertices are visible then everything is ok.
  if (cnt_vis == num_vertices) { num_verts = num_vertices; return true; }

  // We really need to clip.
  num_verts = 0;

  float A = portal_plane->A ();
  float B = portal_plane->B ();
  float C = portal_plane->C ();
  float D = portal_plane->D ();

  i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; i++)
  {
    zs = !vis[i];
    z1s = !vis[i1];

    if (z1s && !zs)
    {
      csIntersect3::Plane (Vcam (i1), Vcam (i), A, B, C, D, verts[num_verts], r);
      num_verts++;
      verts[num_verts++] = Vcam (i);
    }
    else if (!z1s && zs)
    {
      csIntersect3::Plane (Vcam (i1), Vcam (i), A, B, C, D, verts[num_verts], r);
      num_verts++;
    }
    else if (!z1s && !zs)
    {
      verts[num_verts++] = Vcam (i);
    }
    i1 = i;
  }

  return true;
}

#define EXPERIMENTAL_BUG_FIX 1
bool csPolygon3D::DoPerspective (const csTransform& trans,
	csVector3* source, int num_verts, csPolygon2D* dest, csVector2* orig_triangle,
	bool mirror)
{
  csVector3 *ind, *end = source+num_verts;

  if (num_verts==0) return false;
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
    if (ind->z >= SMALL_Z) dest->AddPerspective (*ind);
    else break;
    ind++;
  }

  // Check if special or mixed processing is required
  if (ind != end)
  {
    // If we are processing a triangle (uv_coords != NULL) then
    // we stop here because the triangle is only visible if all
    // vertices are visible (this is not exactly true but it is
    // easier this way! @@@ CHANGE IN FUTURE).
    if (uv_coords || use_flat_color) return false;

    csVector3 *exit = NULL, *exitn = NULL, *reenter = NULL, *reentern = NULL;
    csVector2 *evert = NULL;

    if (ind == source)
    {
      while (ind < end)
      {
        if (ind->z >= SMALL_Z) { reentern = ind;  reenter = ind-1;  break; }
        ind++;
      }
    }
    else
    {
      exit = ind;
      exitn = ind-1;
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
       if (ind->z >= SMALL_Z) { reentern = ind;  reenter = ind-1;  break; }
       ind++;
      }
      if (ind == end) { reentern = source;  reenter = ind-1; }
       else needfinish = true;
     } /* if (exit) */
     else
     {
      // we know where the polygon becomes NORMAL, now we need to
      // to find out on which edge it ceases to be NORMAL.
      while (ind < end)
      {
       if (ind->z >= SMALL_Z) dest->AddPerspective (*ind);
       else { exit = ind;  exitn = ind-1;  break; }
       ind++;
      }
      if (ind == end) { exit = source;  exitn = ind-1; }
      evert = dest->GetLast ();
     }

     // Add the NEAR points appropriately.
#    define MAX_VALUE 1000000.

     // First, for the exit point.
     float ex, ey, epointx, epointy;
     ex = exitn->z * exit->x - exitn->x * exit->z;
     ey = exitn->z * exit->y - exitn->y * exit->z;
     if (ABS(ex) < SMALL_EPSILON && ABS(ey) < SMALL_EPSILON)
     {
      // Uncommon special case:  polygon passes through origin.
      plane->WorldToCamera (trans, source[0]);
      ex = plane->GetCameraPlane ().A();
      ey = plane->GetCameraPlane ().B();
      if (ABS(ex) < SMALL_EPSILON && ABS(ey) < SMALL_EPSILON)
      {
       // Downright rare case:  polygon near parallel with viewscreen.
       ex = exit->x - exitn->x;
       ey = exit->y - exitn->y;
      }
     }
     if (ABS(ex) > ABS(ey))
     {
       if (ex>0) epointx = MAX_VALUE;
       else epointx = -MAX_VALUE;
       epointy = (epointx - evert->x)*ey/ex + evert->y;
     }
     else
     {
       if (ey>0) epointy = MAX_VALUE;
       else epointy = -MAX_VALUE;
       epointx = (epointy - evert->y)*ex/ey + evert->x;
     }

     // Next, for the reentry point.
     float rx, ry, rpointx, rpointy;

     // Obligatory ugly hack :)
     // Can be fixed if someone writes a function something like this:
     // csVector2 get_perspective(const csVector3&)
     //@@@
     //csPolygon2D p(1);
     //p.AddPerspective (*reentern);
     //rvert = p.GetFirst ();
     float iz = csCamera::aspect/reentern->z;
     csVector2 rvert;
     rvert.x = reentern->x * iz + csWorld::shift_x;
     rvert.y = reentern->y * iz + csWorld::shift_y;

     if (reenter == exit && reenter->z > -SMALL_EPSILON)
     { rx = ex;  ry = ey; }
     else
     {
       rx = reentern->z * reenter->x - reentern->x * reenter->z;
       ry = reentern->z * reenter->y - reentern->y * reenter->z;
     }
     if (ABS(rx) < SMALL_EPSILON && ABS(ry) < SMALL_EPSILON)
     {
      // Uncommon special case:  polygon passes through origin.
      plane->WorldToCamera (trans, source[0]);
      rx = plane->GetCameraPlane ().A();
      ry = plane->GetCameraPlane ().B();
      if (ABS(rx) < SMALL_EPSILON && ABS(ry) < SMALL_EPSILON)
      {
       // Downright rare case:  polygon near parallel with viewscreen.
       rx = reenter->x - reentern->x;
       ry = reenter->y - reentern->y;
      }
     }
     if (ABS(rx) > ABS(ry))
     {
       if (rx>0) rpointx = MAX_VALUE;
       else rpointx = -MAX_VALUE;
       rpointy = (rpointx - rvert.x)*ry/rx + rvert.y;
     }
     else
     {
       if (ry>0) rpointy = MAX_VALUE;
       else rpointy = -MAX_VALUE;
       rpointx = (rpointy - rvert.y)*rx/ry + rvert.x;
     }

#    define QUADRANT(x,y) ( (y<x?1:0)^(x<-y?3:0) )
#    define MQUADRANT(x,y) ( (y<x?3:0)^(x<-y?1:0) )

    dest->AddVertex (epointx,epointy);
#   if EXPERIMENTAL_BUG_FIX
    if (mirror)
    {
      int quad = MQUADRANT(epointx, epointy);
      int rquad = MQUADRANT(rpointx, rpointy);
      if ((quad==0 && -epointx==epointy)||(quad==1 && epointx==epointy)) 
        quad++;
      if ((rquad==0 && -rpointx==rpointy)||(rquad==1 && rpointx==rpointy)) 
        rquad++;
      while (quad != rquad)
      {
        epointx = (quad&2)           ?  MAX_VALUE : -MAX_VALUE;
        epointy = (quad==0||quad==3) ?  MAX_VALUE : -MAX_VALUE;
        dest->AddVertex (epointx, epointy);
        quad = (quad+1)&3;
      }
    }
    else
    {
      int quad = QUADRANT(epointx, epointy);
      int rquad = QUADRANT(rpointx, rpointy);
      if ((quad==0 && epointx==epointy)||(quad==1 && -epointx==epointy)) 
        quad++;
      if ((rquad==0 && rpointx==rpointy)||(rquad==1 && -rpointx==rpointy)) 
        rquad++;
      while (quad != rquad)
      {
        epointx = (quad&2)           ? -MAX_VALUE :  MAX_VALUE;
        epointy = (quad==0||quad==3) ?  MAX_VALUE : -MAX_VALUE;
        dest->AddVertex (epointx, epointy);
        quad = (quad+1)&3;
      }
    }
#   endif
    dest->AddVertex (rpointx,rpointy);

     // Add the rest of the vertices, which are all NORMAL points.
     if (needfinish) while (ind < end)
      dest->AddPerspective (*ind++);

    } /* if (exit || reenter) */

    // Do special processing (all points are NEAR or BEHIND)
    else
    {
      if (mirror)
      {
        csVector3* ind2 = end - 1;
        for (ind = source;  ind < end;  ind2=ind, ind++)
          if ((ind->x - ind2->x)*(ind2->y) - (ind->y - ind2->y)*(ind2->x) > -SMALL_EPSILON)
            return false;
        dest->AddVertex ( MAX_VALUE,-MAX_VALUE);
        dest->AddVertex ( MAX_VALUE, MAX_VALUE);
        dest->AddVertex (-MAX_VALUE, MAX_VALUE);
        dest->AddVertex (-MAX_VALUE,-MAX_VALUE);
      }
      else
      {
        csVector3* ind2 = end - 1;
        for (ind = source;  ind < end;  ind2=ind, ind++)
          if ((ind->x - ind2->x)*(ind2->y) - (ind->y - ind2->y)*(ind2->x) < SMALL_EPSILON)
            return false;
        dest->AddVertex (-MAX_VALUE,-MAX_VALUE);
        dest->AddVertex (-MAX_VALUE, MAX_VALUE);
        dest->AddVertex ( MAX_VALUE, MAX_VALUE);
        dest->AddVertex ( MAX_VALUE,-MAX_VALUE);
      }
    }

  } /* if (ind != end) */

  plane->WorldToCamera (trans, source[0]);
  return true;
}

bool csPolygon3D::IntersectRay (const csVector3& start, const csVector3& end)
{
  // First we do backface culling on the polygon with respect to
  // the starting point of the beam.
  csPlane& pl = plane->GetWorldPlane ();
  float dot1 = pl.D () + pl.A ()*start.x + pl.B ()*start.y + pl.C ()*start.z;
  if (dot1 > 0) return false;

  // If this vector is perpendicular to the plane of the polygon we
  // need to catch this case here.
  float dot2 = pl.D () + pl.A ()*end.x + pl.B ()*end.y + pl.C ()*end.z;
  if (ABS (dot1-dot2) < SMALL_EPSILON) return false;

  // Now we generate a plane between the starting point of the ray and
  // every edge of the polygon. With the plane normal of that plane we
  // can then check if the end of the ray is on the same side for all
  // these planes.
  csVector3 normal, relend;
  relend = end;
  relend -= start;

  int i, i1;
  i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; i++)
  {
    csMath3::CalcNormal (normal, start, Vwor (i1), Vwor (i));
    if ( (relend * normal) > 0) return false;
    i1 = i;
  }

  return true;
}

bool csPolygon3D::IntersectSegment (const csVector3& start, const csVector3& end,
                                   csVector3& isect, float* pr)
{
  if (!IntersectRay (start, end)) return false;
  return plane->IntersectSegment (start, end, isect, pr);
}

void csPolygon3D::InitLightmaps (csPolygonSet* owner, bool do_cache, int index)
{
  if (orig_poly) return;
  if (!tex->lm) return;
  if (!do_cache) { lightmap_up_to_date = false; return; }
  if (do_force_recalc) { tex->InitLightmaps (); lightmap_up_to_date = false; }
  else if (!tex->lm->ReadFromCache (TEXW(tex), TEXH(tex), def_mipmap_size, owner, this, index, csWorld::current_world))
  {
    tex->InitLightmaps ();
    lightmap_up_to_date = true;
  }
  else lightmap_up_to_date = true;
}

void csPolygon3D::CalculateLightmaps (csLightView& lview)
{
  if (orig_poly) return;

  if (uv_coords || use_flat_color)
  {
    // We are working for a vertex lighted polygon.
    csLight* light = (csLight*)lview.l;
    float rd = lview.r / light->GetRadius ();
    float gd = lview.g / light->GetRadius ();
    float bd = lview.b / light->GetRadius ();

    if (lview.dynamic)
    {
      // Currently not yet supported. @@@
      return;
    }
    else
    {
      csColor col;
      int i;
      float cosfact = cosinus_factor;
      if (cosfact == -1) cosfact = csPolyTexture::cfg_cosinus_factor;

      for (i = 0 ; i < num_vertices ; i++)
      {
        if (colors && !lview.gouroud_color_reset) col = colors[i];
	else col.Set (0, 0, 0);
        float d = csSquaredDist::PointPoint (light->GetCenter (), Vwor (i));
	if (d >= light->GetSquaredRadius ()) continue;
	d = sqrt (d);
	float cosinus = (Vwor (i)-light->GetCenter ())*GetPolyPlane ()->Normal ();
	cosinus /= d;
	cosinus += cosfact;
	if (cosinus < 0) cosinus = 0;
	else if (cosinus > 1) cosinus = 1;
	col.red = col.red + cosinus * rd*(light->GetRadius () - d);
	if (col.red > 1) col.red = 1;
	col.green = col.green + cosinus * gd*(light->GetRadius () - d);
	if (col.green > 1) col.green = 1;
	col.blue = col.blue + cosinus * bd*(light->GetRadius () - d);
	if (col.blue > 1) col.blue = 1;
        SetColor (i, col);
      }
    }
    return;
  }

  if (lview.gouroud_only) return;

  if (lview.dynamic)
  {
    // We are working for a dynamic light. In this case we create
    // a light patch for this polygon.
    CHK (csLightPatch* lp = new csLightPatch ());
    AddLightpatch (lp);
    csDynLight* dl = (csDynLight*)lview.l;
    dl->AddLightpatch (lp);

    lp->num_vertices = lview.light_frustrum->GetNumVertices ();
    CHK (lp->vertices = new csVector3 [lp->num_vertices]);

    int i, mi;
    for (i = 0 ; i < lp->num_vertices ; i++)
    {
      if (lview.mirror) mi = lp->num_vertices-i-1;
      else mi = i;
      //lp->vertices[i] = lview.frustrum[mi] + lview.center;
      lp->vertices[i] = lview.light_frustrum->GetVertex (mi);
    }

  }
  else
  {
    if (lightmap_up_to_date) return;
    tex->CalculateLightmaps (lview);
#if 0
//@@@
    // If there is already a lightmap1 this means that we are
    // redoing lighting at run-time. In that case we also
    // need to update the other mipmap levels.
    // @@@ We need specific functionality for this.
    // Doing this with create_lightmaps is NOT efficient.
    if (lightmap1) create_lightmaps ();
#endif
  }
}

void csPolygon3D::ShineLightmaps (csLightView& lview)
{
  if (orig_poly) return;

  if (uv_coords || use_flat_color)
  {
    // We are working for a vertex lighted polygon.
    csLight* light = (csLight*)lview.l;
    float rd = lview.r / light->GetRadius ();
    float gd = lview.g / light->GetRadius ();
    float bd = lview.b / light->GetRadius ();

    if (lview.dynamic)
    {
      // Currently not yet supported. @@@
      return;
    }
    else
    {
      csColor col;
      int i;
      float cosfact = cosinus_factor;
      if (cosfact == -1) cosfact = csPolyTexture::cfg_cosinus_factor;

      for (i = 0 ; i < num_vertices ; i++)
      {
        if (colors && !lview.gouroud_color_reset) col = colors[i];
	else col.Set (0, 0, 0);
        float d = csSquaredDist::PointPoint (light->GetCenter (), Vwor (i));
	if (d >= light->GetSquaredRadius ()) continue;
	d = sqrt (d);
	float cosinus = (Vwor (i)-light->GetCenter ())*GetPolyPlane ()->Normal ();
	cosinus /= d;
	cosinus += cosfact;
	if (cosinus < 0) cosinus = 0;
	else if (cosinus > 1) cosinus = 1;
	col.red = col.red + cosinus * rd*(light->GetRadius () - d);
	if (col.red > 1) col.red = 1;
	col.green = col.green + cosinus * gd*(light->GetRadius () - d);
	if (col.green > 1) col.green = 1;
	col.blue = col.blue + cosinus * bd*(light->GetRadius () - d);
	if (col.blue > 1) col.blue = 1;
        SetColor (i, col);
      }
    }
    return;
  }

  if (lview.gouroud_only) return;

  if (lview.dynamic)
  {
    // We are working for a dynamic light. In this case we create
    // a light patch for this polygon.
    CHK (csLightPatch* lp = new csLightPatch ());
    AddLightpatch (lp);
    csDynLight* dl = (csDynLight*)lview.l;
    dl->AddLightpatch (lp);

    lp->num_vertices = lview.num_frustrum;
    CHK (lp->vertices = new csVector3 [lview.num_frustrum]);

    int i, mi;
    for (i = 0 ; i < lview.num_frustrum ; i++)
    {
      if (lview.mirror) mi = lview.num_frustrum-i-1;
      else mi = i;
      //lp->vertices[i] = lview.frustrum[mi] + lview.center;
      lp->vertices[i] = lview.frustrum[mi];
    }

  }
  else
  {
    if (lightmap_up_to_date) return;
    tex->ShineLightmaps (lview);
#if 0
//@@@
    // If there is already a lightmap1 this means that we are
    // redoing lighting at run-time. In that case we also
    // need to update the other mipmap levels.
    // @@@ We need specific functionality for this.
    // Doing this with create_lightmaps is NOT efficient.
    if (lightmap1) create_lightmaps ();
#endif
  }
}

void csPolygon3D::CacheLightmaps (csPolygonSet* owner, int index)
{
  if (orig_poly) return;
  if (!tex->lm) return;
  if (!lightmap_up_to_date)
  {
    lightmap_up_to_date = true;
    tex->lm->Cache (owner, this, index, csWorld::current_world);
  }
  tex->lm->ConvertToMixingMode ();
}

//---------------------------------------------------------------------------

csPolygon2D csPolygon2D::clipped (MAX_VERTICES, true);

csPolygon2D::csPolygon2D (int max, bool use_uv)
{
  max_vertices = max;
  CHK (vertices = new csVector2 [max]);
  if (use_uv) { CHK (uv_coords = new csVector2 [max]); }
  else uv_coords = NULL;
  MakeEmpty ();
}

csPolygon2D::csPolygon2D (csPolygon2D& copy)
{
  max_vertices = copy.max_vertices;
  CHK (vertices = new csVector2 [max_vertices]);
  num_vertices = copy.num_vertices;
  memcpy (vertices, copy.vertices, sizeof (csVector2)*num_vertices);
  bbox = copy.bbox;
  if (copy.uv_coords)
  {
    CHK (uv_coords = new csVector2 [max_vertices]);
    memcpy (uv_coords, copy.uv_coords, sizeof (csVector2)*num_vertices);
  }
  else uv_coords = NULL;
}

csPolygon2D::~csPolygon2D ()
{
  CHK (delete [] vertices);
  CHK (delete [] uv_coords);
}

void csPolygon2D::MakeEmpty ()
{
  num_vertices = 0;
  bbox.StartBoundingBox ();
}

void csPolygon2D::AddVertex (float x, float y)
{
  vertices[num_vertices].x = x;
  vertices[num_vertices].y = y;
  num_vertices++;
  bbox.AddBoundingVertex (x, y);
  if (num_vertices > max_vertices)
  {
    CsPrintf (MSG_FATAL_ERROR, "OVERFLOW add_vertex!\n");
    fatal_exit (0, false);
  }
}

void csPolygon2D::AddPerspective (float x, float y, float z)
{
  float iz = csCamera::aspect/z;
  float px, py;

  if (num_vertices >= max_vertices)
  {
    CsPrintf (MSG_FATAL_ERROR, "OVERFLOW add_perspective!\n");
    fatal_exit (0, false);
  }

  px = x * iz + csWorld::shift_x;
  py = y * iz + csWorld::shift_y;
  vertices[num_vertices].x = px;
  vertices[num_vertices].y = py;

  num_vertices++;
  bbox.AddBoundingVertex (px, py);
}

bool csPolygon2D::ClipAgainst (csClipper* view)
{
  return view->Clip (vertices, num_vertices, max_vertices, &bbox);
}

void csPolygon2D::Draw (IGraphics2D* g2d, int col)
{
  int i;
  int x1, y1, x2, y2;

  if (!num_vertices) return;

  x1 = QRound (vertices[num_vertices-1].x);
  y1 = QRound (vertices[num_vertices-1].y);
  for (i = 0 ; i < num_vertices ; i++)
  {
    x2 = QRound (vertices[i].x);
    y2 = QRound (vertices[i].y);
    g2d->DrawLine (x1, csWorld::frame_height - 1 - y1, x2, csWorld::frame_height - 1 - y2, col);

    x1 = x2;
    y1 = y2;
  }
}

//---------------------------------------------------------------------------

void PreparePolygonQuick (G3DPolygon* g3dpoly, bool gouraud)
{
  csVector2* orig_triangle = g3dpoly->pi_triangle;

  // First we have to find the u,v coordinates for every
  // point in the clipped polygon. We know we started
  // from orig_triangle and that texture mapping is not perspective correct.

  // Compute U & V in vertices of the polygon
  // First find the topmost triangle vertex
  int top;
  if (orig_triangle [0].y < orig_triangle [1].y)
    if (orig_triangle [0].y < orig_triangle [2].y) top = 0;
    else top = 2;
  else
    if (orig_triangle [1].y < orig_triangle [2].y) top = 1;
    else top = 2;

  int j;
  for (j = 0 ; j < g3dpoly->num ; j++)
  {
    float x = g3dpoly->vertices[j].sx;
    float y = g3dpoly->vertices[j].sy;

    // Find the triangle left/right edges between which
    // the given x,y point is located.
    int vtl, vtr, vbl, vbr;
    vtl = vtr = top;
    vbl = (vtl + 1) % 3;
    vbr = (vtl + 3 - 1) % 3;
    if (y > orig_triangle [vbl].y)
    {
      vtl = vbl;
      vbl = (vbl + 1) % 3;
    }
    else if (y > orig_triangle [vbr].y)
    {
      vtr = vbr;
      vbr = (vbr + 3 - 1) % 3;
    }
    else
    {
      // The last two vertices of the triangle have the same height.
      // @@@ I think we should interpolate by 'x' here but this fix at
      // least eliminates most errors.
      vtl = vbl;
      vbl = (vbl + 1) % 3;
    }

    // Now interpolate Z,U,V by Y
    float tL = orig_triangle [vbl].y - orig_triangle [vtl].y;
    if (tL) tL = (y - orig_triangle [vtl].y) / tL;
    float tR = orig_triangle [vbr].y - orig_triangle [vtr].y;
    if (tR) tR = (y - orig_triangle [vtr].y) / tR;
    float xL = orig_triangle [vtl].x + tL * (orig_triangle [vbl].x - orig_triangle [vtl].x);
    float xR = orig_triangle [vtr].x + tR * (orig_triangle [vbr].x - orig_triangle [vtr].x);
    float tX = xR - xL;
    if (tX) tX = (x - xL) / tX;

#   define INTERPOLATE(val,tl,bl,tr,br)	\
        {					\
          float vl,vr;				\
          if (tl != bl)				\
            vl = tl + (bl - tl) * tL;		\
          else					\
            vl = tl;				\
          if (tr != br)				\
            vr = tr + (br - tr) * tR;		\
          else					\
            vr = tr;				\
          val = vl + (vr - vl) * tX;		\
        }

    // Calculate Z
    INTERPOLATE (g3dpoly->pi_texcoords [j].z,
          g3dpoly->pi_tritexcoords [vtl].z, g3dpoly->pi_tritexcoords [vbl].z,
          g3dpoly->pi_tritexcoords [vtr].z, g3dpoly->pi_tritexcoords [vbr].z);
    if (g3dpoly->txt_handle)
    {
      // Calculate U
      INTERPOLATE (g3dpoly->pi_texcoords [j].u,
            g3dpoly->pi_tritexcoords [vtl].u, g3dpoly->pi_tritexcoords [vbl].u,
            g3dpoly->pi_tritexcoords [vtr].u, g3dpoly->pi_tritexcoords [vbr].u);
      // Calculate V
      INTERPOLATE (g3dpoly->pi_texcoords [j].v,
            g3dpoly->pi_tritexcoords [vtl].v, g3dpoly->pi_tritexcoords [vbl].v,
            g3dpoly->pi_tritexcoords [vtr].v, g3dpoly->pi_tritexcoords [vbr].v);
    }
    if (gouraud)
    {
      // Calculate R
      INTERPOLATE (g3dpoly->pi_texcoords [j].r,
            g3dpoly->pi_tritexcoords [vtl].r, g3dpoly->pi_tritexcoords [vbl].r,
            g3dpoly->pi_tritexcoords [vtr].r, g3dpoly->pi_tritexcoords [vbr].r);
      // Calculate G
      INTERPOLATE (g3dpoly->pi_texcoords [j].g,
            g3dpoly->pi_tritexcoords [vtl].g, g3dpoly->pi_tritexcoords [vbl].g,
            g3dpoly->pi_tritexcoords [vtr].g, g3dpoly->pi_tritexcoords [vbr].g);
      // Calculate B
      INTERPOLATE (g3dpoly->pi_texcoords [j].b,
            g3dpoly->pi_tritexcoords [vtl].b, g3dpoly->pi_tritexcoords [vbl].b,
            g3dpoly->pi_tritexcoords [vtr].b, g3dpoly->pi_tritexcoords [vbr].b);
    }
    else
    {
      g3dpoly->pi_texcoords[j].r = 0;
      g3dpoly->pi_texcoords[j].g = 0;
      g3dpoly->pi_texcoords[j].b = 0;
    }
  }
}

//---------------------------------------------------------------------------

G3DPolygon g3dpoly;

// For debugging
csPolygon3D* csPolygon3D::hilight = NULL;
bool do_coord_check;
csVector2 coord_check_vector;

void csPolygon2D::DrawFilled (IGraphics3D* g3d, csPolygon3D* poly, csPolyPlane* plane, bool mirror,
	bool use_z_buf, csVector2* orig_triangle)
{
  int i;
  bool debug = false;

  // If do_coord_check is true we are in a debugging mode. Here we don't
  // draw the polygon but check if the given coordinate is inside the
  // polygon. If so we dump all information about the polygon.
  if (do_coord_check)
  {
    CHK (csPolygon2D* pp = new csPolygon2D (max_vertices));
    if (mirror)
      for (i = 0 ; i < num_vertices ; i++)
        pp->AddVertex  (vertices[num_vertices-i-1]);
    else
      for (i = 0 ; i < num_vertices ; i++)
        pp->AddVertex  (vertices[i]);
    if (csMath2::InPoly2D(coord_check_vector, pp->GetVertices (),
    	pp->GetNumVertices (), &pp->GetBoundingBox ()) != CS_POLY_OUT)
    {
      csPolygonSet* ps = (csPolygonSet*)(poly->GetParent ());
      CsPrintf (MSG_DEBUG_0, "Hit polygon '%s/%s'\n", 
        csNameObject::GetName(*ps), csNameObject::GetName(*poly));
      Dumper::dump (this, "csPolygon2D");
      Dumper::dump (poly);
      debug = true;
      csPolygon3D::hilight = poly;
    }

    CHK (delete pp);
    if (!debug) return;
    // If debug we go to the rest of the processing and
    // call a debugging function in Graph3D.
  }

  if (use_z_buf)
  {
    g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, true);
    g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  }
  else
  {
    g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, false);
    g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  }

  memset (&g3dpoly, 0, sizeof (G3DPolygon));
  g3dpoly.num = num_vertices;
  g3dpoly.txt_handle = poly->GetTextureHandle ();
  g3dpoly.inv_aspect = csCamera::inv_aspect;

  if (poly->GetUVCoords () || poly->UseFlatColor ())
  {
    csColor* po_colors = poly->GetColors ();
    g3dpoly.flat_color_r = poly->GetFlatColor ().red;
    g3dpoly.flat_color_g = poly->GetFlatColor ().green;
    g3dpoly.flat_color_b = poly->GetFlatColor ().blue;
    if (poly->UseFlatColor ()) g3dpoly.txt_handle = NULL;

    // We are going to use DrawPolygonQuick.
    g3dpoly.pi_triangle = orig_triangle;
    g3dpoly.pi_tritexcoords[0].z = 1. / poly->Vcam (0).z;
    g3dpoly.pi_tritexcoords[1].z = 1. / poly->Vcam (1).z;
    g3dpoly.pi_tritexcoords[2].z = 1. / poly->Vcam (2).z;
    if (g3dpoly.txt_handle)
    {
      g3dpoly.pi_tritexcoords[0].u = poly->GetUVCoords ()[0].x;
      g3dpoly.pi_tritexcoords[0].v = poly->GetUVCoords ()[0].y;
      g3dpoly.pi_tritexcoords[1].u = poly->GetUVCoords ()[1].x;
      g3dpoly.pi_tritexcoords[1].v = poly->GetUVCoords ()[1].y;
      g3dpoly.pi_tritexcoords[2].u = poly->GetUVCoords ()[2].x;
      g3dpoly.pi_tritexcoords[2].v = poly->GetUVCoords ()[2].y;
    }
    if (po_colors)
    {
      g3dpoly.pi_tritexcoords[0].r = po_colors[0].red;
      g3dpoly.pi_tritexcoords[0].g = po_colors[0].green;
      g3dpoly.pi_tritexcoords[0].b = po_colors[0].blue;
      g3dpoly.pi_tritexcoords[1].r = po_colors[1].red;
      g3dpoly.pi_tritexcoords[1].g = po_colors[1].green;
      g3dpoly.pi_tritexcoords[1].b = po_colors[1].blue;
      g3dpoly.pi_tritexcoords[2].r = po_colors[2].red;
      g3dpoly.pi_tritexcoords[2].g = po_colors[2].green;
      g3dpoly.pi_tritexcoords[2].b = po_colors[2].blue;
    }
    for (i = 0 ; i < num_vertices ; i++)
    {
      g3dpoly.vertices[i].sx = vertices[i].x;
      g3dpoly.vertices[i].sy = vertices[i].y;
    }
    CHK (g3dpoly.pi_texcoords = new G3DPolygon::poly_texture_def [64]);
    PreparePolygonQuick (&g3dpoly, po_colors != NULL);
    g3d->StartPolygonQuick (g3dpoly.txt_handle, po_colors != NULL);
    g3d->DrawPolygonQuick (g3dpoly, po_colors != NULL);
    g3d->FinishPolygonQuick ();
    CHK (delete [] g3dpoly.pi_texcoords);
  }
  else
  {
    // We are going to use DrawPolygon.
    if (mirror)
      for (i = 0 ; i < num_vertices ; i++)
      {
        g3dpoly.vertices[num_vertices-i-1].sx = vertices[i].x;
        g3dpoly.vertices[num_vertices-i-1].sy = vertices[i].y;
      }
    else
      for (i = 0 ; i < num_vertices ; i++)
      {
        g3dpoly.vertices[i].sx = vertices[i].x;
        g3dpoly.vertices[i].sy = vertices[i].y;
      }
    g3dpoly.polygon = GetIPolygon3DFromcsPolygon3D(poly);
    g3dpoly.id = poly->GetID ();

    g3dpoly.plane.m_cam2tex = &plane->m_cam2tex;
    g3dpoly.plane.v_cam2tex = &plane->v_cam2tex;

    float Ac, Bc, Cc, Dc;
    plane->GetCameraNormal (&Ac, &Bc, &Cc, &Dc);
    g3dpoly.normal.A = Ac;
    g3dpoly.normal.B = Bc;
    g3dpoly.normal.C = Cc;
    g3dpoly.normal.D = Dc;

    if (debug)
      g3d->DrawPolygonDebug (g3dpoly);
    else
      if (FAILED (g3d->DrawPolygon (g3dpoly)))
      {
        CsPrintf (MSG_STDOUT, "Drawing polygon '%s/%s' failed!\n", 
                  csNameObject::GetName(*((csSector*)poly->GetParent ())),
                  csNameObject::GetName(*poly));
      }
  }

  if (csPolygon3D::hilight == poly)
  {
    ITextureManager* txtmgr;
    IGraphics2D* g2d;
    g3d->GetTextureManager (&txtmgr);
    g3d->Get2dDriver (&g2d);
    int white;
    txtmgr->FindRGB (255, 255, 255, white);
    Draw (g2d, white);
  }
}

void csPolygon2D::AddFogPolygon (IGraphics3D* g3d, csPolygon3D* poly, csPolyPlane* plane, bool mirror, CS_ID id, int fogtype)
{
  int i;

  memset(&g3dpoly, 0, sizeof(G3DPolygon));
  g3dpoly.num = num_vertices;
  g3dpoly.inv_aspect = csCamera::inv_aspect;
  if (mirror)
    for (i = 0 ; i < num_vertices ; i++)
    {
      g3dpoly.vertices[num_vertices-i-1].sx = vertices[i].x;
      g3dpoly.vertices[num_vertices-i-1].sy = vertices[i].y;
    }
  else
    for (i = 0 ; i < num_vertices ; i++)
    {
      g3dpoly.vertices[i].sx = vertices[i].x;
      g3dpoly.vertices[i].sy = vertices[i].y;
    }
  g3dpoly.polygon = GetIPolygon3DFromcsPolygon3D(poly);
  g3dpoly.id = poly->GetID ();

  float Ac, Bc, Cc, Dc;
  plane->GetCameraNormal (&Ac, &Bc, &Cc, &Dc);
  g3dpoly.normal.A = Ac;
  g3dpoly.normal.B = Bc;
  g3dpoly.normal.C = Cc;
  g3dpoly.normal.D = Dc;

  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, true);
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  g3d->AddFogPolygon (id, g3dpoly, fogtype);
}

//---------------------------------------------------------------------------
