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
#include "csengine/polytmap.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/light.h"
#include "csengine/dynlight.h"
#include "csengine/lghtmap.h"
#include "csengine/camera.h"
#include "csengine/portal.h"
#include "csengine/dumper.h"
#include "csengine/lppool.h"
#include "csutil/garray.h"
#include "igraph3d.h"
#include "itexture.h"
#include "itxtmgr.h"

bool csPolygon3D::do_force_recalc = false;
bool csPolygon3D::do_not_force_recalc = false;
int csPolygon3D::def_mipmap_size = 16;
bool csPolygon3D::do_lightmap_highqual = true;
bool csPolygon3D::do_cache_lightmaps = true;

// This is a static vector array which is adapted to the
// right size everytime it is used. In the beginning it means
// that this array will grow a lot but finally it will
// stabilize to a maximum size (not big). The advantage of
// this approach is that we don't have a static array which can
// overflow. And we don't have to do allocation every time we
// come here. We do an IncRef on this object each time a new
// csPolygon3D is created and an DecRef each time it is deleted.
// Thus, when the world is cleaned, the array is automatically
// cleaned too.
static DECLARE_GROWING_ARRAY (VectorArray, csVector3)

//---------------------------------------------------------------------------

csLightMapped::csLightMapped () : csPolygonTextureType ()
{
  txt_plane = NULL;
  theDynLight = NULL;
  CHK (tex = new csPolyTexture ());
  CHK (tex1 = new csPolyTexture ());
  CHK (tex2 = new csPolyTexture ());
  CHK (tex3 = new csPolyTexture ());
  lightmap_up_to_date = false;
}

csLightMapped::~csLightMapped ()
{
  if (tex) tex->DecRef ();
  if (tex1) tex1->DecRef ();
  if (tex2) tex2->DecRef ();
  if (tex3) tex3->DecRef ();
  if (txt_plane) txt_plane->DecRef ();
}

void csLightMapped::Setup (csPolygon3D* poly3d, csTextureHandle* txtMM)
{
  tex->SetPolygon (poly3d);
  tex1->SetPolygon (poly3d);
  tex2->SetPolygon (poly3d);
  tex3->SetPolygon (poly3d);

  tex->SetMipmapLevel (0);
  tex1->SetMipmapLevel (1);
  tex2->SetMipmapLevel (2);
  tex3->SetMipmapLevel (3);

  tex->SetTextureHandle (txtMM->GetTextureHandle ());
  tex1->SetTextureHandle (txtMM->GetTextureHandle ());
  tex2->SetTextureHandle (txtMM->GetTextureHandle ());
  tex3->SetTextureHandle (txtMM->GetTextureHandle ());

  tex->CreateBoundingTextureBox ();
  tex1->CreateBoundingTextureBox ();
  tex2->CreateBoundingTextureBox ();
  tex3->CreateBoundingTextureBox ();
}

csPolyTexture* csLightMapped::GetPolyTex (int mipmap)
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

void csLightMapped::SetTxtPlane (csPolyTxtPlane* txt_pl)
{
  if (txt_plane) txt_plane->DecRef ();
  txt_plane = txt_pl;
  txt_plane->IncRef ();
}

void csLightMapped::NewTxtPlane ()
{
  if (txt_plane) txt_plane->DecRef ();
  CHK (txt_plane = new csPolyTxtPlane ());
}

//---------------------------------------------------------------------------

csGouraudShaded::csGouraudShaded () : csPolygonTextureType ()
{
  uv_coords = NULL;
  colors = NULL;
  static_colors = NULL;
  num_vertices = -1;
}

csGouraudShaded::~csGouraudShaded ()
{
  Clear ();
}

void csGouraudShaded::Clear ()
{
  CHK (delete [] uv_coords); uv_coords = NULL;
  CHK (delete [] colors); colors = NULL;
  CHK (delete [] static_colors); static_colors = NULL;
}

void csGouraudShaded::Setup (int num_vertices)
{
  if (num_vertices == csGouraudShaded::num_vertices) return;
  bool use_gouraud = (colors || static_colors);
  Clear ();
  csGouraudShaded::num_vertices = num_vertices;
  CHK (uv_coords = new csVector2 [num_vertices]);
  if (use_gouraud)
  {
    CHK (colors = new csColor [num_vertices]);
    CHK (static_colors = new csColor [num_vertices]);
    int j;
    for (j = 0 ; j < num_vertices ; j++)
    {
      colors[j].Set (0, 0, 0);
      static_colors[j].Set (0, 0, 0);
    }
  }
}

void csGouraudShaded::EnableGouraud (bool g)
{
  if (!g)
  {
    CHK (delete [] colors); colors = NULL;
    CHK (delete [] static_colors); static_colors = NULL;
  }
  else
  {
    if (colors && static_colors) return;
    if (num_vertices == -1)
    {
      CsPrintf (MSG_INTERNAL_ERROR,
      	"Setup was not called yet for csGouraudShaded!\n");
      fatal_exit (0, false);
    }
    CHK (delete [] colors); colors = NULL;
    CHK (delete [] static_colors); static_colors = NULL;
    CHK (colors = new csColor [num_vertices]);
    CHK (static_colors = new csColor [num_vertices]);
    int j;
    for (j = 0 ; j < num_vertices ; j++)
    {
      colors[j].Set (0, 0, 0);
      static_colors[j].Set (0, 0, 0);
    }
  }
}

void csGouraudShaded::SetUV (int i, float u, float v)
{
  uv_coords[i].x = u;
  uv_coords[i].y = v;
}

void csGouraudShaded::AddColor (int i, float r, float g, float b)
{
  r += static_colors[i].red; if (r > 2) r = 2;
  g += static_colors[i].green; if (g > 2) g = 2;
  b += static_colors[i].blue; if (b > 2) b = 2;
  static_colors[i].Set (r, g, b);
}

void csGouraudShaded::AddDynamicColor (int i, float r, float g, float b)
{
  r += colors[i].red; if (r > 2) r = 2;
  g += colors[i].green; if (g > 2) g = 2;
  b += colors[i].blue; if (b > 2) b = 2;
  colors[i].Set (r, g, b);
}


void csGouraudShaded::SetColor (int i, float r, float g, float b)
{
  static_colors[i].Set (r, g, b);
}

void csGouraudShaded::ResetDynamicColor (int i)
{
  colors[i] = static_colors[i];
}

void csGouraudShaded::SetDynamicColor (int i, float r, float g, float b)
{
  colors[i].Set (r, g, b);
}


//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csPolygon3D,csObject);

IMPLEMENT_IBASE (csPolygon3D)
  IMPLEMENTS_INTERFACE (iPolygon3D)
IMPLEMENT_IBASE_END

csPolygon3D::csPolygon3D (csTextureHandle* texture) : csObject (),
  csPolygonInt (), vertices (4)
{
  CONSTRUCT_IBASE (NULL);
  txtMM = texture;
  if (texture) SetTexture (texture);

  txt_info = NULL;

  plane = NULL;

  portal = NULL;
  delete_portal = false;

  sector = NULL;
  orig_poly = NULL;
  dont_draw = false;

  flags = CS_POLY_MIPMAP | CS_POLY_LIGHTING;

  light_info.cosinus_factor = -1;
  light_info.lightpatches = NULL;
  light_info.dyn_dirty = true;
  light_info.flat_color.red = 0;
  light_info.flat_color.green = 0;
  light_info.flat_color.blue = 0;

  SetTextureType (POLYTXT_LIGHTMAP);

  VectorArray.IncRef ();
}

csPolygon3D::csPolygon3D (csPolygon3D& poly) : csObject (), csPolygonInt (),
  vertices (4)
{
  const char* tname = poly.GetName ();
  if (tname) SetName (tname);

  poly_set = poly.poly_set;
  sector = poly.sector;

  portal = poly.portal;
  delete_portal = false;	// This polygon is no owner

  plane = poly.plane;
  plane->IncRef ();

  txtMM = poly.txtMM;

  // Share txt_info with original polygon.
  txt_info = poly.txt_info;
  txt_info->IncRef ();

  poly.dont_draw = true;
  dont_draw = false;
  orig_poly = poly.orig_poly ? poly.orig_poly : &poly;

  flags = poly.flags;

  light_info.cosinus_factor = poly.light_info.cosinus_factor;
  light_info.lightpatches = NULL;
  light_info.flat_color = poly.light_info.flat_color;
  light_info.dyn_dirty = false;

  VectorArray.IncRef ();
}

csPolygon3D::~csPolygon3D ()
{
  if (txt_info) txt_info->DecRef (); 
  if (plane) plane->DecRef ();
  if (delete_portal) CHKB (delete portal);
  while (light_info.lightpatches)
    csWorld::current_world->lightpatch_pool->Free (light_info.lightpatches);
  VectorArray.DecRef ();
}

void csPolygon3D::SetTextureType (int type)
{
  if (txt_info)
    if (txt_info->GetTextureType () == type) 
      return;	// Already that type
    else
      txt_info->DecRef ();
  switch (type)
  {
    case POLYTXT_LIGHTMAP:
      CHK (txt_info = (csPolygonTextureType *)new csLightMapped ());
      break;
    case POLYTXT_GOURAUD:
      CHK (txt_info = (csPolygonTextureType *)new csGouraudShaded ());
      break;
  }
}

void csPolygon3D::Reset ()
{
  vertices.MakeEmpty ();
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

void csPolygon3D::SplitWithPlane (csPolygonInt** poly1, csPolygonInt** poly2,
				  const csPlane& plane)
{
  CHK (csPolygon3D* np1 = new csPolygon3D (*this));
  CHK (csPolygon3D* np2 = new csPolygon3D (*this));
  *poly1 = (csPolygonInt*)np1; // Front
  *poly2 = (csPolygonInt*)np2; // Back
  np1->Reset ();
  np2->Reset ();
  GetParent ()->AddPolygon (np1);
  GetParent ()->AddPolygon (np2);

  csVector3 ptB;
  float sideA, sideB;
  csVector3 ptA = Vwor (GetVertices ().GetNumVertices () - 1);
  sideA = plane.Classify (ptA);
  if (ABS (sideA) < SMALL_EPSILON) sideA = 0;

  for (int i = -1 ; ++i < GetVertices ().GetNumVertices () ; )
  {
    ptB = Vwor (i);
    sideB = plane.Classify (ptB);
    if (ABS (sideB) < SMALL_EPSILON) sideB = 0;
    if (sideB > 0)
    {
      if (sideA < 0)
      {
	// Compute the intersection point of the line
	// from point A to point B with the partition
	// plane. This is a simple ray-plane intersection.
	csVector3 v = ptB; v -= ptA;
	float sect = - plane.Classify (ptA) / ( plane.GetNormal () * v ) ;
	v *= sect; v += ptA;
	np1->AddVertex (v);
	np2->AddVertex (v);
      }
      np2->AddVertex (ptB);
    }
    else if (sideB < 0)
    {
      if (sideA > 0)
      {
	// Compute the intersection point of the line
	// from point A to point B with the partition
	// plane. This is a simple ray-plane intersection.
	csVector3 v = ptB; v -= ptA;
	float sect = - plane.Classify (ptA) / ( plane.GetNormal () * v );
	v *= sect; v += ptA;
	np1->AddVertex (v);
	np2->AddVertex (v);
      }
      np1->AddVertex (ptB);
    }
    else
    {
      np1->AddVertex (ptB);
      np2->AddVertex (ptB);
    }
    ptA = ptB;
    sideA = sideB;
  }

  np1->Finish ();
  np2->Finish ();
}


void csPolygon3D::SetTexture (csTextureHandle* texture)
{
  txtMM = texture;
}

iTextureHandle* csPolygon3D::GetTextureHandle ()
{
  return txtMM ? txtMM->GetTextureHandle () : (iTextureHandle*)NULL;
}


bool csPolygon3D::IsTransparent ()
{
  if (GetAlpha ())
    return true;
  return GetTextureHandle ()->GetTransparent ();
}

int csPolygon3D::Classify (const csPlane& pl)
{
  if (GetPolyPlane () == &pl) return POL_SAME_PLANE;
  if (csMath3::PlanesEqual (pl, *GetPolyPlane ())) return POL_SAME_PLANE;

  int i;
  int front = 0, back = 0;

  for (i = 0 ; i < GetVertices ().GetNumVertices () ; i++)
  {
    float dot = pl.Classify (Vwor (i));
    if (ABS (dot) < EPSILON) dot = 0;
    if (dot > 0) back++;
    else if (dot < 0) front++;
  }
  if (back == 0) return POL_FRONT;
  if (front == 0) return POL_BACK;
  return POL_SPLIT_NEEDED;
}

int csPolygon3D::ClassifyX (float x)
{
  int i;
  int front = 0, back = 0;

  for (i = 0 ; i < GetVertices ().GetNumVertices () ; i++)
  {
    float xx = Vwor (i).x-x;
    if (xx < -EPSILON) front++;
    else if (xx > EPSILON) back++;
  }
  if (back == 0) return POL_FRONT;
  if (front == 0) return POL_BACK;
  return POL_SPLIT_NEEDED;
}

int csPolygon3D::ClassifyY (float y)
{
  int i;
  int front = 0, back = 0;

  for (i = 0 ; i < GetVertices ().GetNumVertices () ; i++)
  {
    float yy = Vwor (i).y-y;
    if (yy < -EPSILON) front++;
    else if (yy > EPSILON) back++;
  }
  if (back == 0) return POL_FRONT;
  if (front == 0) return POL_BACK;
  return POL_SPLIT_NEEDED;
}

int csPolygon3D::ClassifyZ (float z)
{
  int i;
  int front = 0, back = 0;

  for (i = 0 ; i < GetVertices ().GetNumVertices () ; i++)
  {
    float zz = Vwor (i).z-z;
    if (zz < -EPSILON) front++;
    else if (zz > EPSILON) back++;
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

  if (!plane) CHKB (plane = new csPolyPlane ());

  // By default the world space normal is equal to the object space normal.
  plane->GetObjectPlane ().Set (A, B, C, D);
  plane->GetWorldPlane ().Set (A, B, C, D);
}

void csPolygon3D::ObjectToWorld (const csReversibleTransform& t)
{
  plane->ObjectToWorld (t, Vwor (0));
  csLightMapped* lmi = GetLightMapInfo ();
  if (lmi) lmi->GetTxtPlane ()->ObjectToWorld (t);
  if (portal) portal->ObjectToWorld (t);
}

#define TEXW(t) ((t)->w_orig)
#define TEXH(t) ((t)->h)

void csPolygon3D::Finish ()
{
  if (orig_poly) return;
  if (GetTextureType () == POLYTXT_GOURAUD || CheckFlags (CS_POLY_FLATSHADING))
  	return;
  csLightMapped *lmi = GetLightMapInfo ();
  if (!lmi)
  {
    CsPrintf (MSG_INTERNAL_ERROR, "No txt_info in polygon!\n");
    fatal_exit (0, false);
  }
  lmi->Setup (this, txtMM);
  lmi->tex->SetLightMap (NULL);
  lmi->tex1->SetLightMap (NULL);
  lmi->tex2->SetLightMap (NULL);
  lmi->tex3->SetLightMap (NULL);
  if (portal)
    portal->SetTexture (txtMM->GetTextureHandle ());

  if (CheckFlags (CS_POLY_LIGHTING) && TEXW(lmi->tex)*TEXH(lmi->tex) < 1000000)
  {
    CHK (csLightMap* lm = new csLightMap ());
    lmi->tex->SetLightMap (lm);
    int r, g, b;
    GetSector ()->GetAmbientColor (r, g, b);
    lmi->tex->lm->Alloc (TEXW(lmi->tex), TEXH(lmi->tex),
      def_mipmap_size, r, g, b);
    lmi->tex->SetMipmapSize (def_mipmap_size);
    lmi->tex1->SetLightMap (lm);
    lmi->tex2->SetLightMap (lm);
    lmi->tex3->SetLightMap (lm);
    lm->DecRef ();
  }
}

void csPolygon3D::CreateLightMaps (iGraphics3D* g3d)
{
  if (orig_poly) return;
  csLightMapped* lmi = GetLightMapInfo ();
  if (!lmi || lmi->tex->lm == NULL) return;

  iTextureManager* txtmgr = g3d->GetTextureManager ();
  if (txtmgr->GetVeryNice ())
    lmi->tex1->SetMipmapSize (def_mipmap_size);
  else
    lmi->tex1->SetMipmapSize (def_mipmap_size>>1);

  lmi->tex2->SetMipmapSize (lmi->tex1->mipmap_size>>1);

  if (lmi->tex2->mipmap_size < 1)
  {
    lmi->tex2->SetMipmapSize (1);
    lmi->tex3->SetMipmapSize (1);
  }
  else
  {
    lmi->tex3->SetMipmapSize (lmi->tex2->mipmap_size>>1);

    if (lmi->tex3->mipmap_size < 1)
    {
      lmi->tex3->SetMipmapSize (1);
    }
  }

  bool po2 = (g3d->NeedsPO2Maps ());
  int aspect = g3d->GetMaximumAspectRatio ();

  if (lmi->tex->lm) lmi->tex->lm->ConvertFor3dDriver (po2, aspect);
  lmi->tex1->SetLightMap (lmi->tex->lm);
  lmi->tex2->SetLightMap (lmi->tex->lm);
  lmi->tex3->SetLightMap (lmi->tex->lm);
}

void csPolygon3D::SetTextureSpace (csPolyTxtPlane* txt_pl)
{
  ComputeNormal ();
  if (GetTextureType () == POLYTXT_LIGHTMAP)
  {
    csLightMapped* lmi = GetLightMapInfo ();
    if (lmi) lmi->SetTxtPlane (txt_pl);
  }
}

void csPolygon3D::SetTextureSpace (csPolygon3D* copy_from)
{
  ComputeNormal ();
  if (GetTextureType () == POLYTXT_LIGHTMAP)
  {
    csLightMapped* lmi = GetLightMapInfo ();
    if (lmi) lmi->SetTxtPlane (copy_from->GetLightMapInfo ()->GetTxtPlane ());
  }
}

void csPolygon3D::SetTextureSpace (
	const csMatrix3& tx_matrix,
	const csVector3& tx_vector)
{
  ComputeNormal ();
  if (GetTextureType () == POLYTXT_LIGHTMAP)
  {
    csLightMapped* lmi = GetLightMapInfo ();
    if (lmi)
    {
      lmi->NewTxtPlane ();
      lmi->GetTxtPlane() ->SetTextureSpace (tx_matrix, tx_vector);
    }
  }
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
  ComputeNormal ();
  if (GetTextureType () == POLYTXT_LIGHTMAP)
  {
    csLightMapped* lmi = GetLightMapInfo ();
    if (lmi)
    {
      lmi->NewTxtPlane ();
      lmi->GetTxtPlane() ->SetTextureSpace (
	plane->GetWorldPlane (),
  	xo, yo, zo, x1, y1, z1, len1);
    }
  }
}

void csPolygon3D::SetTextureSpace (
	float xo, float yo, float zo,
	float x1, float y1, float z1,
	float len1,
	float x2, float y2, float z2,
	float len2)
{
  ComputeNormal ();
  if (GetTextureType () == POLYTXT_LIGHTMAP)
  {
    csLightMapped* lmi = GetLightMapInfo ();
    if (lmi)
    {
      lmi->NewTxtPlane ();
      csVector3 v1(xo, yo, zo), v2(x1, y1, z1), v3(x2, y2, z2);
      lmi->GetTxtPlane() ->SetTextureSpace (
  	v1, v2, len1, v3, len2);
    }
  }
}


void csPolygon3D::MakeDirtyDynamicLights ()
{
  csPolygon3D* p;
  if (orig_poly) p = orig_poly;
  else p = this;
  p->light_info.dyn_dirty = true;
  csLightMapped* lmi = GetLightMapInfo ();
  if (!lmi) return;
  if (lmi->tex) lmi->tex->MakeDirtyDynamicLights ();
  if (lmi->tex1) lmi->tex1->MakeDirtyDynamicLights ();
  if (lmi->tex2) lmi->tex2->MakeDirtyDynamicLights ();
  if (lmi->tex3) lmi->tex3->MakeDirtyDynamicLights ();
}

void csPolygon3D::UnlinkLightpatch (csLightPatch* lp)
{
  if (lp->next_poly) lp->next_poly->prev_poly = lp->prev_poly;
  if (lp->prev_poly) lp->prev_poly->next_poly = lp->next_poly;
  else light_info.lightpatches = lp->next_poly;
  lp->prev_poly = lp->next_poly = NULL;
  lp->polygon = NULL;
  MakeDirtyDynamicLights ();
}

void csPolygon3D::AddLightpatch (csLightPatch* lp)
{
  lp->next_poly = light_info.lightpatches;
  lp->prev_poly = NULL;
  if (light_info.lightpatches) light_info.lightpatches->prev_poly = lp;
  light_info.lightpatches = lp;
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
  vertices.AddVertex (v);
  return vertices.GetNumVertices ()-1;
}

int csPolygon3D::AddVertex (const csVector3& v)
{
  int i = poly_set->AddVertex (v);
  AddVertex (i);
  return i;
}

int csPolygon3D::AddVertex (float x, float y, float z)
{
  int i = poly_set->AddVertex (x, y, z);
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

  i1 = GetVertices ().GetNumVertices ()-1;
  for (i = 0 ; i < GetVertices ().GetNumVertices () ; i++)
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


bool csPolygon3D::ClipPoly (csVector3* frustrum, int m, bool mirror,
	csVector3** dest, int* num_dest)
{
  int i, i1;
  int num_vertices = GetVertices ().GetNumVertices ();
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

bool csPolygon3D::ClipFrustrum (csVector3& center, csVector3* frustrum,
	int num_frustrum, bool mirror,
	csVector3** new_frustrum, int* new_num_frustrum)
{
  //if (!plane->VisibleFromPoint (center)) return false;
  if (!plane->VisibleFromPoint (center) ||
  	ABS (plane->Classify (center)) < SMALL_EPSILON) return false;
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
  for (i = 0 ; i < GetVertices ().GetNumVertices () ; i++)
    if (Vcam (i).z >= 0) cnt_vis++;
    //if (Vcam (i).z >= SMALL_Z) cnt_vis++;
  if (cnt_vis == 0) return false;

  // Perform backface culling.
  // Note! The plane normal needs to be correctly calculated for this
  // to work! @@@ can be optimized
  if (plane->VisibleFromPoint (v_w2c) != cw && plane->Classify (v_w2c) != 0)
    return false;

  // Copy the vertices to verts.
  int num_vertices = GetVertices ().GetNumVertices ();
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
    if (ind->z >= SMALL_Z)
      dest->AddPerspective (*ind);
    else
      break;
    ind++;
  }

  // Check if special or mixed processing is required
  if (ind != end)
  {
    // If we are processing a triangle (uv_coords != NULL) then
    // we stop here because the triangle is only visible if all
    // vertices are visible (this is not exactly true but it is
    // easier this way! @@@ CHANGE IN FUTURE).
//    if (GetTextureType () == POLYTXT_GOURAUD || CheckFlags (CS_POLY_FLATSHADING))
//      return false;

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
      plane->WorldToCamera (trans, source[0]); //@@@ Why is this needed???
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
     float iz = csWorld::current_world->current_camera->aspect/reentern->z;
     csVector2 rvert;
     rvert.x = reentern->x * iz + csWorld::current_world->current_camera->shift_x;
     rvert.y = reentern->y * iz + csWorld::current_world->current_camera->shift_y;

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
      plane->WorldToCamera (trans, source[0]); //@@@ Why is this needed?
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
  i1 = GetVertices ().GetNumVertices ()-1;
  for (i = 0 ; i < GetVertices ().GetNumVertices () ; i++)
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

void csPolygon3D::InitLightMaps (csPolygonSet* owner, bool do_cache, int index)
{
  if (orig_poly) return;
  csLightMapped* lmi = GetLightMapInfo ();
  if (!lmi || lmi->tex->lm == NULL) return;
  if (!do_cache) { lmi->lightmap_up_to_date = false; return; }
  if (do_force_recalc || !csWorld::current_world->IsLightingCacheEnabled ())
  {
    lmi->tex->InitLightMaps ();
    lmi->lightmap_up_to_date = false;
  }
  else if (!lmi->tex->lm->ReadFromCache (TEXW(lmi->tex), TEXH(lmi->tex),
  	def_mipmap_size, owner, this, index, csWorld::current_world))
  {
    lmi->tex->InitLightMaps ();
    lmi->lightmap_up_to_date = true;
  }
  else lmi->lightmap_up_to_date = true;
}

void csPolygon3D::UpdateVertexLighting (csLight* light, const csColor& lcol,
	bool dynamic, bool reset)
{
  if ( CheckFlags (CS_POLY_FLATSHADING)) return;
  
  csColor poly_color (0,0,0), vert_color;
  if (light) poly_color = lcol;

  int i;
  float cosfact = light_info.cosinus_factor;
  if (cosfact == -1) cosfact = csPolyTexture::cfg_cosinus_factor;
  csGouraudShaded* gs = GetGouraudInfo ();

  for (i = 0 ; i < GetVertices ().GetNumVertices () ; i++)
  {
    if (reset)
    {
      if (dynamic)
        gs->ResetDynamicColor (i);
      else
        gs->SetColor (i, 0, 0, 0);
    }
    if (light)
    {
      float d = csSquaredDist::PointPoint (light->GetCenter (), Vwor (i));
      if (d >= light->GetSquaredRadius ()) continue;
      d = sqrt (d);
      float cosinus = (Vwor (i)-light->GetCenter ())*GetPolyPlane ()->Normal ();
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0) cosinus = 0;
      else if (cosinus > 1) cosinus = 1;
      vert_color = poly_color * cosinus * light->GetBrightnessAtDistance (d);
      if (!dynamic)
        gs->AddColor (i, vert_color.red, vert_color.green, vert_color.blue);
      gs->AddDynamicColor (i, vert_color.red, vert_color.green, vert_color.blue);
    }
  }
}

void csPolygon3D::FillLightMap (csLightView& lview)
{
  //@@@if (orig_poly) return; BE CAREFUL
  if (lview.callback)
  {
    lview.callback (&lview, CALLBACK_POLYGON, (void*)this);
    return;
  }

  if (lview.dynamic)
  {
    // We are working for a dynamic light. In this case we create
    // a light patch for this polygon.
    csLightPatch* lp = csWorld::current_world->lightpatch_pool->Alloc ();
    GetBasePolygon ()->AddLightpatch (lp);
    csDynLight* dl = (csDynLight*)lview.l;
    dl->AddLightpatch (lp);

    lp->Initialize (lview.light_frustrum->GetNumVertices ());

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
    if (GetTextureType () == POLYTXT_GOURAUD ||
    	CheckFlags (CS_POLY_FLATSHADING))
    {
      // We are working for a vertex lighted polygon.
      csColor col (lview.r, lview.g, lview.b);
      UpdateVertexLighting ((csLight*)lview.l, col, false,
      	lview.gouraud_color_reset);
      return;
    }

    if (lview.gouraud_only) return;

    csLightMapped* lmi = GetLightMapInfo ();
    if (lmi->lightmap_up_to_date) return;
    lmi->tex->FillLightMap (lview);
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

bool csPolygon3D::MarkRelevantShadowFrustrums (csLightView& lview,
	csPlane& plane)
{
  // @@@ Currently this function only checks if a shadow frustrum is inside
  // the light frustrum. There is no checking done if shadow frustrums obscure
  // each other.

  int i;
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

      // First we test a special case. If all vertices of the light
      // frustrum are contained in the shadow frustrum then we don't
      // need to process the light frustrum at all. In that case we
      // return NULL.
      bool fully_shadowed = true;
      for (i = 0 ; i < lview.light_frustrum->GetNumVertices () ; i++)
        if (!sf->Contains (lview.light_frustrum->GetVertex (i)))
	{
	  fully_shadowed = false;
	  break;
	}
      if (fully_shadowed) return false;

      // @@@ Replace with a less accurate but faster test.
      if (!csFrustrum::IsVisible (lview.light_frustrum->GetVertices (),
      	  lview.light_frustrum->GetNumVertices (), sf->GetVertices (), sf->GetNumVertices ()))
	sf->relevant = false;
    }
    sf = sf->next;
  }
  return true;
}

void csPolygon3D::CalculateLighting (csLightView* lview)
{
  //@@@if (orig_poly) return; Be careful!!!

  csPortal* po;
  csFrustrum* light_frustrum = lview->light_frustrum;
  csFrustrum* new_light_frustrum;
  csVector3& center = light_frustrum->GetOrigin ();

  int num_vertices = GetVertices ().GetNumVertices ();
  if (num_vertices > VectorArray.GetLimit ())
    VectorArray.SetLimit (num_vertices);
  csVector3 *poly = VectorArray.GetArray ();

  // If plane is not visible then return.
  if (!plane->VisibleFromPoint (center)) return;

  // Compute the distance from the center of the light to the plane of the
  // polygon.
  float dist_to_plane = GetPolyPlane ()->Distance (center);

  // If distance is too small or greater than the radius of the light then we
  // have a trivial case (no hit).
  if (dist_to_plane < SMALL_EPSILON || dist_to_plane >= lview->l->GetRadius ())
    return;

  // Calculate the new frustrum for this polygon.
  int j;
  if (lview->mirror)
    for (j = 0 ; j < num_vertices ; j++)
      poly[j] = Vwor (num_vertices-j-1)-center;
  else
    for (j = 0 ; j < num_vertices ; j++)
      poly[j] = Vwor (j)-center;
  new_light_frustrum = light_frustrum->Intersect (poly, num_vertices);

  if (new_light_frustrum)
  {
    // There is an intersection of the current light frustrum with the polygon.
    // This means that the polygon is hit by the light.

    // The light is close enough to the plane of the polygon. Now we calculate
    // an accurate minimum squared distance from the light to the polygon. Note
    // that we use the new_frustrum which is relative to the center of the
    // light.
    // So this algorithm works with the light position at (0,0,0) (@@@ we should
    // use this to optimize this algorithm further).
    csVector3 o (0, 0, 0);
    float min_sqdist = csSquaredDist::PointPoly (o,
    		new_light_frustrum->GetVertices (),
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
      // all shadow frustrums which start at the same plane are discarded as
      // well.
      // FillLightMap() will use this information and
      // csPortal::CalculateLighting() will also use it!!
      csPlane poly_plane = *GetPolyPlane ();
      // First translate plane to center of frustrum.
      poly_plane.DD += poly_plane.norm * center;
      poly_plane.Invert ();
      if (MarkRelevantShadowFrustrums (new_lview, poly_plane))
      {
        // Update the lightmap given light and shadow frustrums in new_lview.
        FillLightMap (new_lview);

        po = GetPortal ();
        if (po)
          po->CalculateLighting (new_lview);
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


void csPolygon3D::CacheLightMaps (csPolygonSet* owner, int index)
{
  if (orig_poly) return;
  csLightMapped* lmi = GetLightMapInfo ();
  if (!lmi || lmi->tex->lm == NULL) return;
  if (!lmi->lightmap_up_to_date)
  {
    lmi->lightmap_up_to_date = true;
    if (csWorld::current_world->IsLightingCacheEnabled () && do_cache_lightmaps)
      lmi->tex->lm->Cache (owner, this, index, csWorld::current_world);
  }
  lmi->tex->lm->ConvertToMixingMode ();
}

void csPolygon3D::UpdateLightMapSize ()
{
  if (orig_poly) return;
  csLightMapped* lmi = GetLightMapInfo ();
  if (!lmi || lmi->tex->lm == NULL) return;

  if (CheckFlags (CS_POLY_LIGHTING) && TEXW(lmi->tex)*TEXH(lmi->tex) < 1000000)
  {
    CHK (csLightMap* lm = new csLightMap ());
    lmi->tex->SetLightMap (lm);
    lm->DecRef ();
    int r, g, b;
    GetSector ()->GetAmbientColor (r, g, b);
    lmi->tex->lm->Alloc (TEXW(lmi->tex), TEXH(lmi->tex), def_mipmap_size,
    	r, g, b);
    lmi->tex->SetMipmapSize (def_mipmap_size);
  }
}

void csPolygon3D::ScaleLightMaps ()
{
  if (orig_poly) return;
  csLightMapped* lmi = GetLightMapInfo ();
  if (!lmi || lmi->tex->lm == NULL) return;
  lmi->tex->SetMipmapSize (def_mipmap_size);
  lmi->tex->lm->Scale (TEXW (lmi->tex), TEXH (lmi->tex), lmi->tex->mipmap_size);
}

int csPolygon3D::GetAlpha ()
{
  return portal ? portal->GetAlpha () : 0;
}

csVector3 *csPolygon3D::GetCameraVector (int idx)
{
  return &Vcam (idx);
}

iPolygonTexture *csPolygon3D::GetObjectTexture (int nLevel)
{
  csLightMapped* lmi = GetLightMapInfo ();
  if (lmi)
  {
     iPolygonTexture *tex = lmi->GetPolyTex (nLevel);
     tex->IncRef ();
     return (tex);
  }
  else
    return NULL;
}

bool csPolygon3D::UsesMipMaps ()
{
  return CheckFlags (CS_POLY_MIPMAP);
}

iLightMap *csPolygon3D::GetLightMap ()
{
  csLightMapped *lmi = GetLightMapInfo ();
  return lmi ? lmi->GetLightMap () : NULL;
}
