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
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/texture.h"
#include "csengine/polyplan.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/light.h"
#include "csengine/dynlight.h"
#include "csengine/lghtmap.h"
#include "csengine/camera.h"
#include "csengine/portal.h"
#include "csengine/dumper.h"
#include "csobject/nameobj.h"
#include "csutil/cleanup.h"
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

  flags = CS_POLY_MIPMAP | CS_POLY_LIGHTING;

  cosinus_factor = -1;
  lightmap_up_to_date = false;

  lightpatches = NULL;
  uv_coords = NULL;
  colors = NULL;
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

  flags = poly.flags;
  cosinus_factor = poly.cosinus_factor;
  lightmap_up_to_date = false;

  lightpatches = NULL;
  uv_coords = NULL;
  colors = NULL;

  flat_color = poly.flat_color;
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
  if (portal && portal->GetSector () == sector) return;
  if (portal && delete_portal) { CHK (delete portal); portal = NULL; }
  if (!sector) return;
  CHK (portal = new csPortal);
  delete_portal = true;
  portal->DisableSpaceWarping ();
  portal->SetSector (sector);
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
  csVector3 ptB;
  float sideA, sideB;
  csVector3 ptA = Vwor (num_vertices - 1);
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
  return ((csPolygon3D*)p)->plane->NearlyEqual (plane);
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
  if (uv_coords || CheckFlags (CS_POLY_FLATSHADING)) return;

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
  if (CheckFlags (CS_POLY_LIGHTING) && TEXW(tex)*TEXH(tex) < 1000000)
  {
    CHK (lightmap = new csLightMap ());
    int r, g, b;
    GetSector ()->GetAmbientColor (r, g, b);
    lightmap->Alloc (TEXW(tex), TEXH(tex), def_mipmap_size, r, g, b);
    tex->SetMipmapSize (def_mipmap_size); tex->lm = lightmap;
  }
}

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
  CHK (lightmap1 = new csLightMap ());
  tex1->lm = lightmap1;
  lightmap1->CopyLightmap (lightmap);

  tex2->SetMipmapSize (tex1->mipmap_size>>1);

  // @@@ Normally the two mipmap levels can share the same lightmap.
  // But with the new sub-texture optimization in combination with the dynamic
  // lighting extension this gives some problems. For the moment we just copy
  // the lightmaps even though they are actually the same.
  CHK (lightmap2 = new csLightMap ());
  tex2->lm = lightmap2;
  lightmap2->CopyLightmap (lightmap);

  if (tex2->mipmap_size < 1)
  {
    tex2->SetMipmapSize (1);
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
    CHK (lightmap3 = new csLightMap ());
    tex3->lm = lightmap3;
    lightmap3->CopyLightmap (lightmap);

    if (tex3->mipmap_size < 1)
    {
      tex3->SetMipmapSize (1);
      CHK (delete lightmap3);
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
  if (v >= poly_set->GetNumVertices ())
  {
    CsPrintf (MSG_FATAL_ERROR, "Index number %d is too high for a polygon (max=%d)!\n",
    	v, poly_set->GetNumVertices ());
    fatal_exit (0, false);
  }
  if (v < 0)
  {
    CsPrintf (MSG_FATAL_ERROR, "Bad negative vertex index %d!\n", v);
    fatal_exit (0, false);
  }
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

int csPolygon3D::AddVertex (const csVector3& v)
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
    if (!mirror)
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
  if (!mirror)
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
    //vis[i] = csMath3::Visible (Vcam (i), *portal_plane);
    vis[i] = portal_plane->Classify (Vcam (i)) <= SMALL_EPSILON;
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
  csVector3* source, int num_verts, csPolygon2D* dest, csVector2* /*orig_triangle*/,
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
    if (uv_coords || CheckFlags (CS_POLY_FLATSHADING)) return false;

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

     // Perspective correct the point.
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
  csVector3 normal;
  csVector3 relend = end;
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
  if (do_force_recalc || !csWorld::current_world->IsLightingCacheEnabled ())
  {
    tex->InitLightmaps ();
    lightmap_up_to_date = false;
  }
  else if (!tex->lm->ReadFromCache (TEXW(tex), TEXH(tex), def_mipmap_size, owner, this, index, csWorld::current_world))
  {
    tex->InitLightmaps ();
    lightmap_up_to_date = true;
  }
  else lightmap_up_to_date = true;
}

void csPolygon3D::FillLightmap (csLightView& lview)
{
  if (orig_poly) return;

  if (lview.callback)
  {
    lview.callback (&lview, CALLBACK_POLYGON, (void*)this);
    return;
  }

  if (uv_coords || CheckFlags (CS_POLY_FLATSHADING))
  {
    // We are working for a vertex lighted polygon.
    csLight* light = (csLight*)lview.l;
    float rd = lview.r / light->GetRadius ();
    float gd = lview.g / light->GetRadius ();
    float bd = lview.b / light->GetRadius ();

    if (lview.dynamic)
    {
      // Currently not yet supported. @@@
      // Support for this requires something similar to the static and the
      // real lightmap so that we can store static gouraud shaded stuff seperatelly.
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
        if (colors && !lview.gouraud_color_reset) col = colors[i];
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

  if (lview.gouraud_only) return;

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

    // Copy shadow frustrums.
    csShadowFrustrum* sf, * copy_sf;
    sf = lview.shadows.GetFirst ();
    while (sf)
    {
      if (sf->relevant)
      {
        CHK (copy_sf = new csShadowFrustrum (*sf));
        lp->shadows.AddLast (copy_sf);
      }
      sf = sf->next;
    }

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
    tex->FillLightmap (lview);
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

bool csPolygon3D::MarkRelevantShadowFrustrums (csLightView& lview, csPlane& plane)
{
  // @@@ Currently this function only checks if a shadow frustrum is inside
  // the light frustrum. There is no checking done if shadow frustrums obscure
  // each other.

  int i, count;
  bool contains;
  csShadowFrustrum* sf = lview.shadows.GetFirst ();
  while (sf)
  {
    // First check if the plane of the shadow frustrum is close to the plane
    // of the polygon (the input parameter 'plane'). If so then we discard the
    // frustrum as not relevant.
    if (csMath3::PlanesClose (*sf->GetBackPlane (), plane))
    {
      sf->relevant = false;
    }
    else
    {
      // Assume that the shadow frustrum is relevant.
      sf->relevant = true;

      // Check if any of the vertices of the shadow polygon is inside the light frustrum.
      // If that is the case then the shadow frustrum is indeed relevant.
      contains = false;
      for (i = 0 ; i < sf->GetNumVertices () ; i++)
        if (lview.light_frustrum->Contains (sf->GetVertex (i))) { contains = true; break; }
      if (!contains)
      {
        // All vertices of the shadow polygon are outside the light frustrum. In this
        // case it is still possible that the light frustrum is completely inside the
        // shadow frustrum.
        count = 0;
        for (i = 0 ; i < lview.light_frustrum->GetNumVertices () ; i++)
          if (sf->Contains (lview.light_frustrum->GetVertex (i))) count++;
        if (count == 0)
        {
          // All vertices of the light frustrum (polygon we are trying to light)
	  // are outside of the shadow frustrum. In this case the shadow frustrum is
	  // not relevant.

	  // @@@ WARNING!!! THIS IS NOT TRUE. There are cases where
	  // it is still possible to have an overlap. We need to
	  // continue the testing here!!!
	  sf->relevant = false;
        }
        else if (count == lview.light_frustrum->GetNumVertices ())
        {
          // All vertices of the light frustrum are inside the shadow frustrum. This
	  // is a special case. Now we know that the light frustrum is totally invisible
	  // and we stop the routine and return false here.
	  return false;
        }
      }
    }
    sf = sf->next;
  }
  return true;
}

// csVectorArray is a subclass of csCleanable which is registered
// to csWorld.cleanup.
class csVectorArray : public csCleanable
{
public:
  csVector3* array;
  int size;
  csVectorArray () : array (NULL), size (0) { }
  virtual ~csVectorArray () { CHK (delete [] array); }
};

void csPolygon3D::CalculateLighting (csLightView* lview)
{
  csPortal* po;
  csFrustrum* light_frustrum = lview->light_frustrum;
  csFrustrum* new_light_frustrum;
  csVector3& center = light_frustrum->GetOrigin ();

  // This is a static vector array which is adapted to the
  // right size everytime it is used. In the beginning it means
  // that this array will grow a lot but finally it will
  // stabilize to a maximum size (not big). The advantage of
  // this approach is that we don't have a static array which can
  // overflow. And we don't have to do allocation every time we
  // come here. We register this memory to the 'cleanup' array
  // in csWorld so that it will be freed later.

  static csVectorArray* cleanable = NULL;
  if (!cleanable)
  {
    CHK (cleanable = new csVectorArray ());
    csWorld::current_world->cleanup.Push (cleanable);
  }

  if (num_vertices > cleanable->size)
  {
    CHK (delete [] cleanable->array);
    CHK (cleanable->array = new csVector3 [num_vertices]);
    cleanable->size = num_vertices;
  }
  csVector3* poly = cleanable->array;

  // If plane is not visible then return.
  if (!plane->VisibleFromPoint (center)) return;

  // Compute the distance from the center of the light to the plane of the polygon.
  float dist_to_plane = GetPolyPlane ()->Distance (center);

  // If distance is too small or greater than the radius of the light then we have a
  // trivial case (no hit).
  if (dist_to_plane < SMALL_EPSILON || dist_to_plane >= lview->l->GetRadius ()) return;

  // Calculate the new frustrum for this polygon.

  int j;
  if (lview->mirror)
    for (j = 0 ; j < num_vertices ; j++)
      poly[j] = Vcam (num_vertices-j-1);
  else
    for (j = 0 ; j < num_vertices ; j++)
      poly[j] = Vcam (j);
  new_light_frustrum = light_frustrum->Intersect (poly, num_vertices);

  if (new_light_frustrum)
  {
    // There is an intersection of the current light frustrum with the polygon.
    // This means that the polygon is hit by the light.

    // The light is close enough to the plane of the polygon. Now we calculate
    // an accurate minimum squared distance from the light to the polygon. Note
    // that we use the new_frustrum which is relative to the center of the light.
    // So this algorithm works with the light position at (0,0,0) (@@@ we should
    // use this to optimize this algorithm further).
    csVector3 o (0, 0, 0);
    float min_sqdist = csSquaredDist::PointPoly (o, new_light_frustrum->GetVertices (),
		new_light_frustrum->GetNumVertices (),
		*(GetPolyPlane ()), dist_to_plane*dist_to_plane);
    if (min_sqdist < lview->l->GetSquaredRadius ())
    {
      csLightView new_lview = *lview;
      new_lview.light_frustrum = new_light_frustrum;

      // Mark all shadow frustrums in 'new_lview' which are relevant. i.e.
      // which are inside the light frustrum and are not obscured (shadowed)
      // by other shadow frustrums.
      // We also give the polygon plane to MarkRelevantShadowFrustrums so that
      // all shadow frustrums which start at the same plane are discarded as well.
      // FillLightmap() will use this information and csPortal::CalculateLighting()
      // will also use it!!
      csPlane poly_plane = *GetPolyPlane ();
      poly_plane.DD += poly_plane.norm * center;	// First translate plane to center of frustrum.
      poly_plane.Invert ();
      if (MarkRelevantShadowFrustrums (new_lview, poly_plane))
      {
        // Update the lightmap given the light and shadow frustrums in new_lview.
        FillLightmap (new_lview);

        po = GetPortal ();
        if (po) po->CalculateLighting (new_lview);
        else if (!new_lview.dynamic && csSector::do_radiosity)
        {
          // If there is no portal we simulate radiosity by creating
	  // a dummy portal for this polygon which reflects light.
	  csPortal mirror;
	  mirror.SetSector (GetSector ());
	  mirror.SetAlpha (10);
	  float r, g, b;
	  GetTextureHandle ()->GetMeanColor (r, g, b);
	  mirror.SetFilter (r/4, g/4, b/4);
	  mirror.SetWarp (csTransform::GetReflect ( *(GetPolyPlane ()) ));
	  mirror.CalculateLighting (new_lview);
        }
      }
    }
    else
    {
      CHK (delete new_light_frustrum);
    }
  }
}


void csPolygon3D::CacheLightmaps (csPolygonSet* owner, int index)
{
  if (orig_poly) return;
  if (!tex->lm) return;
  if (!lightmap_up_to_date)
  {
    lightmap_up_to_date = true;
    if (csWorld::current_world->IsLightingCacheEnabled ())
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

void PreparePolygonFX (G3DPolygonDPFX* g3dpoly, csVector2* clipped_verts, int num_vertices,
	csVector2* orig_triangle, bool gouraud)
{
  // 'was_clipped' will be true if the triangle was clipped.
  // This is the case if rescount != 3 (because we then don't have
  // a triangle) or else if any of the clipped vertices is different.
  bool was_clipped = num_vertices != 3;
  int j;
  for (j = 0; j < num_vertices; j++)
  {
    g3dpoly->vertices [j].sx = clipped_verts [j].x;
    g3dpoly->vertices [j].sy = clipped_verts [j].y;
    if (!was_clipped && clipped_verts[j] != orig_triangle[j]) was_clipped = true;
  }

  // If it was not clipped we don't have to do anything.
  if (!was_clipped) return;
								        
  // first we copy the first three texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  G3DTexturedVertex tritexcoords[3];
  for (int i = 0; i < 3; i++)
    tritexcoords [i] = g3dpoly->vertices [i];

  // Now we have to find the u,v coordinates for every
  // point in the clipped polygon. We know we started
  // from orig_triangle and that texture mapping is not perspective correct.

  // Compute U & V in vertices of the polygon
  // First find the topmost triangle vertex
  int top;
  if (orig_triangle [0].y < orig_triangle [1].y)
    if (orig_triangle [0].y < orig_triangle [2].y)
      top = 0;
    else
      top = 2;
  else
    if (orig_triangle [1].y < orig_triangle [2].y)
      top = 1;
    else
      top = 2;

  int _vbl, _vbr;
  if (top <= 0) _vbl = 2; else _vbl = top - 1;
  if (top >= 2) _vbr = 0; else _vbr = top + 1;
  for (j = 0 ; j < g3dpoly->num ; j++)
  {
    float x = g3dpoly->vertices [j].sx;
    float y = g3dpoly->vertices [j].sy;

    // Find the original triangle top/left/bottom/right vertices
    // between which the currently examined point is located.
    // There are two possible major cases:
    // A*       A*       When DrawPolygonFX works, it will switch
    //  |\       |\      start/final values and deltas ONCE (not more)
    //  | \      | \     per triangle.On the left pictures this happens
    //  |*X\     |  \    at the point B. This means we should "emulate"
    //  |   *B   |   *B  this switch bytaking different start/final values
    //  |  /     |*X/    for interpolation if examined point X is below
    //  | /      | /     the point B.
    //  |/       |/
    // C*       C*  Fig.1 :-)
    int vtl = top, vtr = top, vbl = _vbl, vbr = _vbr;
    int ry = QRound (y);
    if (ry > QRound (orig_triangle [vbl].y))
    {
      vtl = vbl;
      if (--vbl < 0) vbl = 2;
    }
    else if (ry > QRound (orig_triangle [vbr].y))
    {
      vtr = vbr;
      if (++vbr > 2) vbr = 0;
    }

    // Now interpolate Z,U,V,R,G,B by Y
    float tL, tR, xL, xR, tX;
    if (QRound (orig_triangle [vbl].y) != QRound (orig_triangle [vtl].y))
      tL = (y - orig_triangle [vtl].y) / (orig_triangle [vbl].y - orig_triangle [vtl].y);
    else
      tL = (x - orig_triangle [vtl].x) / (orig_triangle [vbl].x - orig_triangle [vtl].x);
    if (QRound (orig_triangle [vbr].y) != QRound (orig_triangle [vtr].y))
      tR = (y - orig_triangle [vtr].y) / (orig_triangle [vbr].y - orig_triangle [vtr].y);
    else
      tR = (x - orig_triangle [vtr].x) / (orig_triangle [vbr].x - orig_triangle [vtr].x);
    xL = orig_triangle [vtl].x + tL * (orig_triangle [vbl].x - orig_triangle [vtl].x);
    xR = orig_triangle [vtr].x + tR * (orig_triangle [vbr].x - orig_triangle [vtr].x);
    tX = xR - xL;
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
    INTERPOLATE(g3dpoly->vertices [j].z,
                tritexcoords [vtl].z, tritexcoords [vbl].z,
                tritexcoords [vtr].z, tritexcoords [vbr].z);
    if (g3dpoly->txt_handle)
    {
      // Calculate U
      INTERPOLATE(g3dpoly->vertices [j].u,
                  tritexcoords [vtl].u, tritexcoords [vbl].u,
                  tritexcoords [vtr].u, tritexcoords [vbr].u);
      // Calculate V
      INTERPOLATE(g3dpoly->vertices [j].v,
                  tritexcoords [vtl].v, tritexcoords [vbl].v,
                  tritexcoords [vtr].v, tritexcoords [vbr].v);
    }
    if (gouraud)
    {
      // Calculate R
      INTERPOLATE(g3dpoly->vertices [j].r,
                  tritexcoords [vtl].r, tritexcoords [vbl].r,
                  tritexcoords [vtr].r, tritexcoords [vbr].r);
      // Calculate G
      INTERPOLATE (g3dpoly->vertices [j].g,
                  tritexcoords [vtl].g, tritexcoords [vbl].g,
                  tritexcoords [vtr].g, tritexcoords [vbr].g);
      // Calculate B
      INTERPOLATE (g3dpoly->vertices [j].b,
                  tritexcoords [vtl].b, tritexcoords [vbl].b,
                  tritexcoords [vtr].b, tritexcoords [vbr].b);
    }
    else
    {
      g3dpoly->vertices[j].r = 0;
      g3dpoly->vertices[j].g = 0;
      g3dpoly->vertices[j].b = 0;
    }
  }
}

// Remove this for old fog
#define USE_EXP_FOG

// After such number of values fog density coefficient can be considered 0.
#define FOG_EXP_TABLE_SIZE 1600
static float *fog_exp_table = NULL;

static void InitializeFogTable ()
{
  fog_exp_table = new float [FOG_EXP_TABLE_SIZE];
  for (int i = 0; i < FOG_EXP_TABLE_SIZE; i++)
    fog_exp_table [i] = 1 - exp (-float (i) / 256.);
}

#define SMALL_D 0.01
void CalculateFogPolygon (csRenderView* rview, G3DPolygonDP& poly)
{
  if (!rview->fog_info || poly.num < 3) { poly.use_fog = false; return; }
  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
#endif

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float inv_aspect = poly.inv_aspect;
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A;
  Bc = poly.normal.B;
  Cc = poly.normal.C;
  Dc = poly.normal.D;

  float M, N, O;
  if (ABS (Dc) < SMALL_D) Dc = -SMALL_D;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means that
    // the plane of the polygon is almost perpendicular to the eye of the
    // viewer. In this case, nothing much can be seen of the plane anyway
    // so we just take one value for the entire polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1/poly.z_value;
  }
  else
  {
    inv_Dc = 1/Dc;
    M = -Ac*inv_Dc*inv_aspect;
    N = -Bc*inv_Dc*inv_aspect;
    O = -Cc*inv_Dc;
  }

  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1. / (M * (poly.vertices[i].sx - csWorld::shift_x) + N * (poly.vertices[i].sy - csWorld::shift_y) + O);
    v.x = (poly.vertices[i].sx - csWorld::shift_x) * v.z * inv_aspect;
    v.y = (poly.vertices[i].sy - csWorld::shift_y) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fog_info = rview->fog_info;
    while (fog_info)
    {
      float dist1, dist2;
      if (fog_info->has_incoming_plane)
      {
	const csPlane& pl = fog_info->incoming_plane;
	float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
	//dist1 = v.Norm () * (-pl.DD / denom);
	dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;
      const csPlane& pl = fog_info->outgoing_plane;
      float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
      //dist2 = v.Norm () * (-pl.DD / denom);
      dist2 = v.z * (-pl.DD / denom);

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      UInt table_index = QRound ((100 * ABS (dist2 - dist1)) * fog_info->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table [table_index];
      else
        I2 = 0.;
#else
      float I2 = ABS (dist2 - dist1) * fog_info->fog->density;
#endif

      if (poly.fog_info[i].intensity)
      {
        // We already have a previous fog level. In this case we do some
	// mathematical tricks to combine both fog levels. Substitute one
	// fog expresion in the other. The basic fog expression is:
	//	C = I*F + (1-I)*P
	//	with I = intensity
	//	     F = fog color
	//	     P = polygon color
	//	     C = result
	float I1 = poly.fog_info[i].intensity;
	poly.fog_info[i].intensity = I1 + I2 - I1 * I2;
	float fact = 1. / (I1 + I2 - I1*I2);
	poly.fog_info[i].r = (I2*fog_info->fog->red + I1*poly.fog_info[i].r + I1*I2*poly.fog_info[i].r) * fact;
	poly.fog_info[i].g = (I2*fog_info->fog->green + I1*poly.fog_info[i].g + I1*I2*poly.fog_info[i].g) * fact;
	poly.fog_info[i].b = (I2*fog_info->fog->blue + I1*poly.fog_info[i].b + I1*I2*poly.fog_info[i].b) * fact;
      }
      else
      {
        // The first fog level.
        poly.fog_info[i].intensity = I2;
        poly.fog_info[i].r = fog_info->fog->red;
        poly.fog_info[i].g = fog_info->fog->green;
        poly.fog_info[i].b = fog_info->fog->blue;
      }
      fog_info = fog_info->next;
    }
  }
}

// @@@ We should be able to avoid having the need for two functions
// which are almost exactly the same.
void CalculateFogPolygon (csRenderView* rview, G3DPolygonDPFX& poly)
{
  if (!rview->fog_info || poly.num < 3) { poly.use_fog = false; return; }
  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
#endif

  float inv_aspect = poly.inv_aspect;

  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1. / poly.vertices[i].z;
    v.x = (poly.vertices[i].sx - csWorld::shift_x) * v.z * inv_aspect;
    v.y = (poly.vertices[i].sy - csWorld::shift_y) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fog_info = rview->fog_info;
    while (fog_info)
    {
      float dist1, dist2;
      if (fog_info->has_incoming_plane)
      {
	const csPlane& pl = fog_info->incoming_plane;
	float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
	dist1 = v.Norm () * (-pl.DD / denom);
      }
      else
        dist1 = 0;
      const csPlane& pl = fog_info->outgoing_plane;
      float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
      dist2 = v.Norm () * (-pl.DD / denom);

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      UInt table_index = QRound ((100 * ABS (dist2 - dist1)) * fog_info->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table [table_index];
      else
        I2 = 0.;
#else
      float I2 = ABS (dist2 - dist1) * fog_info->fog->density;
#endif

      if (poly.fog_info[i].intensity)
      {
        // We already have a previous fog level. In this case we do some
	// mathematical tricks to combine both fog levels. Substitute one
	// fog expresion in the other. The basic fog expression is:
	//	C = I*F + (1-I)*P
	//	with I = intensity
	//	     F = fog color
	//	     P = polygon color
	//	     C = result
	float I1 = poly.fog_info[i].intensity;
	poly.fog_info[i].intensity = I1 + I2 - I1*I2;
	float fact = 1. / (I1 + I2 - I1*I2);
	poly.fog_info[i].r = (I2*fog_info->fog->red + I1*poly.fog_info[i].r + I1*I2*poly.fog_info[i].r) * fact;
	poly.fog_info[i].g = (I2*fog_info->fog->green + I1*poly.fog_info[i].g + I1*I2*poly.fog_info[i].g) * fact;
	poly.fog_info[i].b = (I2*fog_info->fog->blue + I1*poly.fog_info[i].b + I1*I2*poly.fog_info[i].b) * fact;
      }
      else
      {
        // The first fog level.
        poly.fog_info[i].intensity = I2;
        poly.fog_info[i].r = fog_info->fog->red;
        poly.fog_info[i].g = fog_info->fog->green;
        poly.fog_info[i].b = fog_info->fog->blue;
      }
      fog_info = fog_info->next;
    }
  }
}


//---------------------------------------------------------------------------

void csPolygon2D::DrawFilled (csRenderView* rview, csPolygon3D* poly, csPolyPlane* plane,
	bool use_z_buf)
{
  int i;
  bool debug = false;
  bool mirror = rview->IsMirrored ();

  if (use_z_buf)
  {
    rview->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, true);
    rview->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  }
  else
  {
    rview->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, false);
    rview->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  }

  if (poly->GetUVCoords () || poly->CheckFlags (CS_POLY_FLATSHADING))
  {
    static G3DPolygonDPFX g3dpoly;
    csVector2 orig_triangle[3];

    //memset (&g3dpoly, 0, sizeof (g3dpoly));
    g3dpoly.num = num_vertices;
    g3dpoly.txt_handle = poly->GetTextureHandle ();
    g3dpoly.inv_aspect = csCamera::inv_aspect;

    csColor* po_colors = poly->GetColors ();
    g3dpoly.flat_color_r = poly->GetFlatColor ().red;
    g3dpoly.flat_color_g = poly->GetFlatColor ().green;
    g3dpoly.flat_color_b = poly->GetFlatColor ().blue;
    if (poly->CheckFlags (CS_POLY_FLATSHADING)) g3dpoly.txt_handle = NULL;

    // We are going to use DrawPolygonFX.
    // Here we have to do a little messy thing because PreparePolygonFX()
    // still requires the original triangle that was valid before clipping.
    float iz;
    iz = 1. / poly->Vcam (0).z;
    g3dpoly.vertices[0].z = iz;
    iz *= csCamera::aspect;
    orig_triangle[0].x = poly->Vcam (0).x * iz + csWorld::shift_x;
    orig_triangle[0].y = poly->Vcam (0).y * iz + csWorld::shift_y;

    iz = 1. / poly->Vcam (1).z;
    g3dpoly.vertices[1].z = iz;
    iz *= csCamera::aspect;
    orig_triangle[1].x = poly->Vcam (1).x * iz + csWorld::shift_x;
    orig_triangle[1].y = poly->Vcam (1).y * iz + csWorld::shift_y;

    iz = 1. / poly->Vcam (2).z;
    g3dpoly.vertices[2].z = iz;
    iz *= csCamera::aspect;
    orig_triangle[2].x = poly->Vcam (2).x * iz + csWorld::shift_x;
    orig_triangle[2].y = poly->Vcam (2).y * iz + csWorld::shift_y;

    if (g3dpoly.txt_handle)
    {
      g3dpoly.vertices[0].u = poly->GetUVCoords ()[0].x;
      g3dpoly.vertices[0].v = poly->GetUVCoords ()[0].y;
      g3dpoly.vertices[1].u = poly->GetUVCoords ()[1].x;
      g3dpoly.vertices[1].v = poly->GetUVCoords ()[1].y;
      g3dpoly.vertices[2].u = poly->GetUVCoords ()[2].x;
      g3dpoly.vertices[2].v = poly->GetUVCoords ()[2].y;
    }
    if (po_colors)
    {
      g3dpoly.vertices[0].r = po_colors[0].red;
      g3dpoly.vertices[0].g = po_colors[0].green;
      g3dpoly.vertices[0].b = po_colors[0].blue;
      g3dpoly.vertices[1].r = po_colors[1].red;
      g3dpoly.vertices[1].g = po_colors[1].green;
      g3dpoly.vertices[1].b = po_colors[1].blue;
      g3dpoly.vertices[2].r = po_colors[2].red;
      g3dpoly.vertices[2].g = po_colors[2].green;
      g3dpoly.vertices[2].b = po_colors[2].blue;
    }
    PreparePolygonFX (&g3dpoly, vertices, num_vertices, orig_triangle, po_colors != NULL);
    rview->g3d->StartPolygonFX (g3dpoly.txt_handle, CS_FX_COPY | (po_colors ? CS_FX_GOURAUD : 0));
    CalculateFogPolygon (rview, g3dpoly);
    rview->g3d->DrawPolygonFX (g3dpoly);
    rview->g3d->FinishPolygonFX ();
  }
  else
  {
    static G3DPolygonDP g3dpoly;

    //memset (&g3dpoly, 0, sizeof (g3dpoly));
    g3dpoly.num = num_vertices;
    g3dpoly.txt_handle = poly->GetTextureHandle ();
    g3dpoly.inv_aspect = csCamera::inv_aspect;

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

    g3dpoly.alpha           = poly->GetAlpha();
    g3dpoly.uses_mipmaps    = poly->CheckFlags (CS_POLY_MIPMAP);
    g3dpoly.z_value         = poly->Vcam(0).z;

    for (int mipmaplevel = 0; mipmaplevel<4; mipmaplevel++)
    {
      g3dpoly.poly_texture[mipmaplevel] =
        GetIPolygonTextureFromcsPolyTexture(poly->GetPolyTex(mipmaplevel));
    }

    g3dpoly.plane.m_cam2tex = &plane->m_cam2tex;
    g3dpoly.plane.v_cam2tex = &plane->v_cam2tex;

    float Ac, Bc, Cc, Dc;
    plane->GetCameraNormal (&Ac, &Bc, &Cc, &Dc);
    g3dpoly.normal.A = Ac;
    g3dpoly.normal.B = Bc;
    g3dpoly.normal.C = Cc;
    g3dpoly.normal.D = Dc;

    if (debug)
      rview->g3d->DrawPolygonDebug (g3dpoly);
    else
    {
      CalculateFogPolygon (rview, g3dpoly);
      if (FAILED (rview->g3d->DrawPolygon (g3dpoly)))
      {
        CsPrintf (MSG_STDOUT, "Drawing polygon '%s/%s' failed!\n",
                  csNameObject::GetName(*((csSector*)poly->GetParent ())),
                  csNameObject::GetName(*poly));
      }
    }
  }
}

void csPolygon2D::AddFogPolygon (IGraphics3D* g3d, csPolygon3D* /*poly*/, csPolyPlane* plane, bool mirror, CS_ID id, int fogtype)
{
  int i;

  static G3DPolygonAFP g3dpoly;
  memset(&g3dpoly, 0, sizeof(g3dpoly));
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
  //g3dpoly.polygon = GetIPolygon3DFromcsPolygon3D(poly); //DPQFIX

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
