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

#include <math.h>
#define CS_SYSDEF_PROVIDE_ALLOCA
#include "cssysdef.h"
#include "qint.h"
#include "qsqrt.h"
#include "csgeom/polyclip.h"
#include "csgeom/polyaa.h"
#include "csengine/polytext.h"
#include "csengine/polyplan.h"
#include "csengine/polytmap.h"
#include "csengine/polygon.h"
#include "csengine/light.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/lghtmap.h"
#include "csutil/bitset.h"
#include "csutil/debug.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"

//------------------------------------------------------------------------------

// Option variable: cosinus factor.
float csPolyTexture::cfg_cosinus_factor = 0;

SCF_IMPLEMENT_IBASE (csPolyTexture)
  SCF_IMPLEMENTS_INTERFACE (iPolygonTexture)
SCF_IMPLEMENT_IBASE_END

csPolyTexture::csPolyTexture ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  DG_ADDI (this, NULL);
  DG_TYPE (this, "csPolyTexture");
  lm = NULL;
  cache_data [0] = cache_data [1] = cache_data [2] = cache_data [3] = NULL;
  polygon = NULL;
  ipolygon = NULL;
  shadow_bitmap = NULL;
}

csPolyTexture::~csPolyTexture ()
{
  if (csEngine::current_engine->G3D)
    csEngine::current_engine->G3D->RemoveFromCache (this);
  CS_ASSERT (cache_data[0] == NULL);
  CS_ASSERT (cache_data[1] == NULL);
  CS_ASSERT (cache_data[2] == NULL);
  CS_ASSERT (cache_data[3] == NULL);
  delete shadow_bitmap;
  if (lm)
  {
    DG_UNLINK (this, lm);
    lm->DecRef ();
  }
  DG_REM (this);
}

void csPolyTexture::SetLightMap (csLightMap* lightmap)
{
  if (lightmap)
  {
    DG_LINK (this, lightmap);
    lightmap->IncRef ();
  }
  if (lm)
  {
    DG_UNLINK (this, lm);
    lm->DecRef ();
  }
  lm = lightmap;
}

void csPolyTexture::CreateBoundingTextureBox ()
{
  // First we compute the bounding box in 2D texture space (uv-space).
  float min_u = 1000000000.;
  float min_v = 1000000000.;
  float max_u = -1000000000.;
  float max_v = -1000000000.;

  csPolyTxtPlane* txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();

  int i;
  csVector3 v1, v2;
  for (i = 0; i < polygon->GetVertices ().GetVertexCount (); i++)
  {
    v1 = polygon->Vobj (i);           // Coordinates of vertex in object space.
    v1 -= txt_pl->v_obj2tex;
    v2 = (txt_pl->m_obj2tex) * v1;  // Coordinates of vertex in texture space.
    if (v2.x < min_u) min_u = v2.x;
    if (v2.x > max_u) max_u = v2.x;
    if (v2.y < min_v) min_v = v2.y;
    if (v2.y > max_v) max_v = v2.y;
  }

  // used in hardware accel drivers
  Fmin_u = min_u;
  Fmax_u = max_u;
  Fmin_v = min_v;
  Fmax_v = max_v;

  int ww, hh;
  if (mat_handle && mat_handle->GetTexture ())
    mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
  else
    ww = hh = 64;
  Imin_u = QRound (min_u * ww);
  Imin_v = QRound (min_v * hh);
  Imax_u = QRound (max_u * ww);
  Imax_v = QRound (max_v * hh);

  h = Imax_v - Imin_v;
  w_orig = Imax_u - Imin_u;
  w = 1;
  shf_u = 0;
  and_u = 0;
  while (true)
  {
    if (w_orig <= w) break;
    w <<= 1;
    shf_u++;
    and_u = (and_u << 1) + 1;
  }

  fdu = min_u * ww;
  fdv = min_v * hh;
}

bool csPolyTexture::RecalculateDynamicLights ()
{
  if (!lm) return false;

  // first combine the static and pseudo-dynamic lights
  if (!lm->UpdateRealLightMap ())
    return false;

  //---
  // Now add all dynamic lights.
  //---
  csLightPatch* lp = polygon->GetBasePolygon ()->GetLightpatches ();
  while (lp)
  {
    ShineDynLightMap (lp);
    lp = lp->GetNextPoly ();
  }

  return true;
}

void csPolyTexture::InitLightMaps ()
{
}

// @@@ This is realy an ugly function which should be precalculated!
static bool TestAdjacency (csVector3* poly1, int num_vertices1,
	csPolygon3D*p,  int num_vertices2, const csVector3& lightpos)
{
  int i, j, j1;
  for (i = 0 ; i < num_vertices1 ; i++)
  {
    csVector3 v = poly1[i]+lightpos;
    j1 = num_vertices2-1;
    for (j = 0 ; j < num_vertices2 ; j++)
    {
      if (ABS (csMath3::Area3 (v, p->Vwor (j), p->Vwor (j1))) < SMALL_EPSILON)
      {
        // Check location of all other vertices relative to edge for polygon 1.
	int sgn = 0;
	int k;
	for (k = 0 ; k < num_vertices1 ; k++)
	  if (k != i)
	  {
	    csVector3 vtest = poly1[k]+lightpos;
	    float area1 = csMath3::Area3 (vtest, p->Vwor (j), p->Vwor (j1));
	    if (ABS (area1) >= SMALL_EPSILON)
	    {
	      int newsgn = SIGN (area1);
	      if (sgn == 0) sgn = newsgn;
	      else if (sgn != newsgn) goto bad;
	    }
	  }
        // Check location of all other vertices relative to edge for polygon 2.
	sgn = -sgn;
	for (k = 0 ; k < num_vertices2 ; k++)
	  if (k != j && k != j1)
	  {
	    float area2 = csMath3::Area3 (p->Vwor (k),
	    	p->Vwor (j), p->Vwor (j1));
	    if (ABS (area2) >= SMALL_EPSILON)
	    {
	      int newsgn = SIGN (area2);
	      if (sgn != newsgn) goto bad;
	    }
	  }
        return true;
      }
// @@@ Yes, I know this is bad: this is a temporary routine :-)
bad:
      j1 = j;
    }
  }
  return false;
}

void csPolyTexture::FillLightMap (csFrustumView* lview, bool vis,
	csPolygon3D* subpoly)
{
  if (!lm) return;
  int j;

  csFrustum* light_frustum = lview->GetFrustumContext ()->GetLightFrustum ();
  csVector3& lightpos = light_frustum->GetOrigin ();
  float inv_lightcell_size = 1.0 / csLightMap::lightcell_size;
  int ww, hh;
  if (mat_handle && mat_handle->GetTexture ())
    mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
  else
    ww = hh = 64;
  csPolyTxtPlane* txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();

  if (!shadow_bitmap)
  {
    int lm_w = lm->rwidth;
    int lm_h = lm->rheight;
    shadow_bitmap = new csShadowBitmap (lm_w, lm_h, csEngine::lightmap_quality,
    	light_frustum->IsInfinite () ? 1 : 0);
    iFrustumViewUserdata* ud = lview->GetUserdata ();
    csLightingPolyTexQueue* lptq = (csLightingPolyTexQueue*)ud;
    lptq->AddPolyTexture (this);
  }

  if (shadow_bitmap->IsFullyShadowed ()) return;

  // Room for our shadow in lightmap space.
  csVector2 s2d[MAX_OUTPUT_VERTICES];

  if (!vis && (subpoly == polygon))
  {
    // If the subpoly is equal to the polygon and it is completely
    // shadowed due to external causes (i.e. c-buffer) then we just
    // fill it as entirely shadowed.
    shadow_bitmap->RenderTotal (1);
  }
  else
  {
    // We will now start rendering an optional light frustum and several
    // shadow frustums on our shadow bitmap. But first we will create a
    // clipping polygon on our polygon which is guaranteed to be larger
    // than the lightmap.
    // @@@ Actually we only want to project the light and shadow frustums
    // on the polygon plane but I'm not sure yet how to do that efficiently.

    // poly will be the polygon to which we clip all frustums.
    csVector3 poly[4];
    csPolygon3D* base_poly = polygon->GetBasePolygon ();
    int num_vertices = 4;
    csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse ();
    csVector3& v_t2w = txt_pl->v_world2tex;
    csVector3 v;
    float inv_ww = 1. / float (ww);
    float inv_hh = 1. / float (hh);
    v.z = 0;
    v.x = (-2 * csLightMap::lightcell_size + Imin_u) * inv_ww;
    v.y = (-2 * csLightMap::lightcell_size + Imin_v) * inv_hh;
    poly[0] = m_t2w * v + v_t2w - lightpos;
    v.x = ((lm->rwidth+2) * csLightMap::lightcell_size + Imin_u) * inv_ww;
    poly[1] = m_t2w * v + v_t2w - lightpos;
    v.y = ((lm->rheight+2) * csLightMap::lightcell_size + Imin_v) * inv_hh;
    poly[2] = m_t2w * v + v_t2w - lightpos;
    v.x = (-2 * csLightMap::lightcell_size + Imin_u) * inv_ww;
    poly[3] = m_t2w * v + v_t2w - lightpos;

    // To transform a frustum coordinate to lightmap space we use
    // the following equation:
    //    F: frustum space vertex from polygon
    //    P: light position
    //    Mw2t: world -> texture transformation matrix
    //    Vw2t: world -> texture transformation vector
    //    T: texture coordinate (only first two coordinates are used)
    //    L: lightmap coordinate
    //
    //    T = Mw2t * (F + P - Vw2t)
    //    L.x = (T.x * ww - Imin_u) * inv_lightcell_size
    //    L.y = (T.y * hh - Imin_v) * inv_lightcell_size
    // This can be optimized to:
    //    T = Mw2t * F + Mw2t * (P - Vw2t)
    //    L.x = (T.x * ww * inv_lightcell_size) - (Imin_u * inv_lightcell_size)
    //    L.y = (T.y * hh * inv_lightcell_size) - (Imin_v * inv_lightcell_size)
    // Let's say that:
    //    wl = ww * inv_lightcell_size
    //    hl = hh * inv_lightcell_size
    //    ul = - Imin_u * inv_lightcell_size
    //    vl = - Imin_v * inv_lightcell_size
    // Then we can say:
    //    L.x = (T.x * wl) + ul
    //    L.y = (T.y * hl) + vl
    // We can also say:
    //    TL = Mw2t * (P - Vw2t)
    // So that:
    //    T = Mw2t * F + TL
    //    T.x = Mw2t.m11 * F.x + Mw2t.m12 * F.y + Mw2t.m13 * F.z + TL.x
    //    T.y = Mw2t.m21 * F.x + Mw2t.m22 * F.y + Mw2t.m23 * F.z + TL.y
    // Let's call m<x><y> = Mw2t.m<x><y> for easy of use:
    //    T.x = m11 * F.x + m12 * F.y + m13 * F.z + TL.x
    //    T.y = m21 * F.x + m22 * F.y + m23 * F.z + TL.y
    //    L.x = m11*wl * F.x + m12*wl * F.y + m13*wl * F.z + TL.x*wl + ul
    //    L.y = m11*hl * F.x + m12*hl * F.y + m13*hl * F.z + TL.y*hl + vl
    csMatrix3& Mw2t = txt_pl->m_world2tex;
    csVector3& Vw2t = txt_pl->v_world2tex;
    csVector3 TL = Mw2t * (lightpos - Vw2t);
    float wl = float (ww) * inv_lightcell_size;
    float hl = float (hh) * inv_lightcell_size;
    float ul = - float (Imin_u) * inv_lightcell_size;
    float vl = - float (Imin_v) * inv_lightcell_size;
    csVector3 trans_u (Mw2t.m11 * wl, Mw2t.m12 * wl, Mw2t.m13 * wl);
    float shift_u = TL.x * wl + ul;
    csVector3 trans_v (Mw2t.m21 * hl, Mw2t.m22 * hl, Mw2t.m23 * hl);
    float shift_v = TL.y * hl + vl;

    // First render our frustum if it is not infinite.
    if (!light_frustum->IsInfinite ())
    {
      csFrustum* new_frustum = light_frustum->Intersect (poly, num_vertices);
      if (new_frustum)
      {
        int nv = new_frustum->GetVertexCount ();
        if (nv > MAX_OUTPUT_VERTICES) nv = MAX_OUTPUT_VERTICES;
        csVector3* s3d = new_frustum->GetVertices ();
        for (j = 0; j < nv; j++)
        {
	  s2d[j].x = trans_u * s3d[j] + shift_u;
	  s2d[j].y = trans_v * s3d[j] + shift_v;
        }
	new_frustum->DecRef ();
        shadow_bitmap->RenderPolygon (s2d, nv, 0);
      }
    }

    // Render all shadow polygons on the shadow-bitmap.
    csFrustumContext* ctxt = lview->GetFrustumContext ();
    iShadowIterator* shadow_it = ctxt->GetShadows ()->GetShadowIterator ();
    while (shadow_it->HasNext ())
    {
      csFrustum* shadow_frust = shadow_it->Next ();

      //@@@ QUESTION!
      //Why do we have to copy the polygon data to the shadow frustum?
      //We have a pointer to the polygon in the shadow frustum. That should
      //be enough. On the other hand, we have to be careful with space
      //warping portals. In that case the list of shadow frustums is
      //currently transformed and we cannot do that to the original polygon.

      // @@@ Optimization: Check for shadow_it->IsRelevant() here.
      csPolygon3D* shad_poly = (csPolygon3D*)(shadow_it->GetUserData ());
      if (shad_poly->GetBasePolygon () == base_poly)
        continue;
      // Check if planes of two polygons are equal
      // (@@@ should be precalculated).
      csPlane3 base_pl = *(base_poly->GetPolyPlane ());
      csPlane3 shad_pl = *(shad_poly->GetPolyPlane ());
      if (csMath3::PlanesClose (base_pl, shad_pl))
        continue;

      // @@@ TODO: Optimize. Custom version of Intersect that is more
      // optimal (doesn't require to copy 'poly') and detects cases
      // that should not shadow.
      // @@@ TODO: Actually we don't want to intersect really. We
      // only want to project the shadow frustum on the polygon plane!!!
      csFrustum* new_shadow = shadow_frust->Intersect (poly, num_vertices);
      if (new_shadow)
      {
        // Test if two polygons are adjacent (@@@ should be precalculated).
        if (TestAdjacency (new_shadow->GetVertices (),
      	    new_shadow->GetVertexCount (), polygon, polygon->GetVertexCount (),
	    lightpos))
	{
	  new_shadow->DecRef ();
	  continue;
	}
        int nv = new_shadow->GetVertexCount ();
        if (nv > MAX_OUTPUT_VERTICES) nv = MAX_OUTPUT_VERTICES;
        csVector3* s3d = new_shadow->GetVertices ();
        for (j = 0; j < nv; j++)
        {
	  s2d[j].x = trans_u * s3d[j] + shift_u;
	  s2d[j].y = trans_v * s3d[j] + shift_v;
        }
	new_shadow->DecRef ();
        shadow_bitmap->RenderPolygon (s2d, nv, 1);
	if (shadow_bitmap->IsFullyShadowed ()) break;
      }
    }
    shadow_it->DecRef ();
  }
}

/* Modified by me to correct some lightmap's border problems -- D.D. */
void csPolyTexture::ShineDynLightMap (csLightPatch* lp)
{
  int lw = 1 + ((w_orig + csLightMap::lightcell_size - 1) >> csLightMap::lightcell_shift);
  int lh = 1 + ((h + csLightMap::lightcell_size - 1) >> csLightMap::lightcell_shift);

  int u, uv;

  int l1, l2 = 0, l3 = 0;
  float d;

  int ww, hh;
  mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);

  csPolyTxtPlane* txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  float cosfact = polygon->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse();
  csVector3 vv = txt_pl->v_world2tex;

  csVector3 v1, v2;

  int ru, rv;
  float invww, invhh;
  invww = 1. / (float)ww;
  invhh = 1. / (float)hh;

  csRGBMap& remap = lm->GetRealMap ();
  csDynLight* light = (csDynLight*)(lp->GetLight ());
  csRGBpixel* map = remap.GetArray ();

  int i;
  float miny = 1000000, maxy = -1000000;
  int MaxIndex = -1, MinIndex = -1;
  float inv_lightcell_size = 1.0 / csLightMap::lightcell_size;

  csVector3 lightpos;
  if (lp->GetLightFrustum ())
    lightpos = lp->GetLightFrustum ()->GetOrigin ();
  else
    lightpos = light->GetCenter ();

  // Calculate the uv's for all points of the frustum (the
  // frustum is actually a clipped version of the polygon).
  csVector2* f_uv = NULL;
  if (lp->GetVertices ())
  {
    int mi;
    f_uv = new csVector2 [lp->GetVertexCount ()];
    for (i = 0 ; i < lp->GetVertexCount () ; i++)
    {
      //if (lview.IsMirrored ()) mi = lview.num_frustum-i-1;
      //else mi = i;
      mi = i;

      // T = Mwt * (W - Vwt)
      //v1 = pl->m_world2tex * (lp->vertices[mi] + lp->center - pl->v_world2tex);
      v1 = txt_pl->m_world2tex * (lp->GetVertex (mi) + lightpos - txt_pl->v_world2tex);
      f_uv[i].x = (v1.x * ww - Imin_u) * inv_lightcell_size;
      f_uv[i].y = (v1.y * hh - Imin_v) * inv_lightcell_size;
      if (f_uv[i].y < miny) miny = f_uv[MinIndex = i].y;
      if (f_uv[i].y > maxy) maxy = f_uv[MaxIndex = i].y;
    }
  }

  csColor color = light->GetColor () * CS_NORMAL_LIGHT_LEVEL;

  int new_lw = lm->GetWidth ();

  int scanL1, scanL2, scanR1, scanR2;   // scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;             // scanline X left/right and deltas
  int sy, fyL, fyR;                     // scanline Y, final Y left, final Y right
  int xL, xR;

  sxL = sxR = dxL = dxR = 0;            // avoid GCC warnings about "uninitialized variables"
  scanL2 = scanR2 = MaxIndex;
  // sy = fyL = fyR = (QRound (f_uv[scanL2].y)>lh-1)?lh-1:QRound (f_uv[scanL2].y);
  sy = fyL = fyR = (QRound (ceil(f_uv[scanL2].y))>lh-1)?lh-1:QRound (ceil(f_uv[scanL2].y));

  for ( ; ; )
  {
    //-----
    // We have reached the next segment. Recalculate the slopes.
    //-----
    bool leave;
    do
    {
      leave = true;
      if (sy <= fyR)
      {
        // Check first if polygon has been finished
a:      if (scanR2 == MinIndex) goto finish;
        scanR1 = scanR2;
        scanR2 = (scanR2 + 1) % lp->GetVertexCount ();

        if (ABS (f_uv [scanR2].y - f_uv [MaxIndex].y) < EPSILON)
        {
          // oops! we have a flat bottom!
          goto a;
        }
/*      if (scanR2 == MinIndex) goto finish;
        scanR1 = scanR2;
        scanR2 = (scanR2 + 1) % lp->num_vertices;
*/
        fyR = QRound(floor(f_uv[scanR2].y));
        float dyR = (f_uv[scanR1].y - f_uv[scanR2].y);
	sxR = f_uv[scanR1].x;
        if (dyR != 0)
        {
          dxR = (f_uv[scanR2].x - sxR) / dyR;
	  // horizontal pixel correction
          sxR += dxR * (f_uv[scanR1].y - ((float)sy));
        }
	else dxR = 0;
        leave = false;
      }
      if (sy <= fyL)
      {
b:      if (scanL2 == MinIndex) goto finish;
        scanL1 = scanL2;
        scanL2 = (scanL2 - 1 + lp->GetVertexCount ()) % lp->GetVertexCount ();

        if (ABS (f_uv [scanL2].y - f_uv [MaxIndex].y) < EPSILON)
        {
          // oops! we have a flat bottom!
          goto b;
        }

	//scanL1 = scanL2;
	//scanL2 = (scanL2 - 1 + lp->num_vertices) % lp->num_vertices;
        fyL = QRound(floor(f_uv[scanL2].y));
        float dyL = (f_uv[scanL1].y - f_uv[scanL2].y);
	sxL = f_uv[scanL1].x;
        if (dyL != 0)
        {
          dxL = (f_uv[scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (f_uv[scanL1].y - ((float)sy));
        }
	else dxL = 0;
        leave = false;
      }
    }
    while (!leave);

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR) fin_y = fyL;
    else fin_y = fyR;

    while (sy >= fin_y)
    {
      // @@@ The check below should not be needed but it is.
      if (sy < 0)
      {
        goto finish;
      }
    
      // Compute the rounded screen coordinates of horizontal strip
      float _l = sxL, _r = sxR;

      if (_r > _l) { float _=_r; _r=_l; _l=_; }

      xL = 1 + QRound (ceil  (_l));
      xR = QRound (floor (_r));

      //if (xR > xL) { int xswap = xR; xR = xL; xL = xswap; }
      if (xR < 0) xR = 0;
      if (xL > lw) xL = lw;

      for (u = xR; u < xL ; u++)
      {
        uv = sy * new_lw + u;

        ru = u << csLightMap::lightcell_shift;
        rv = sy << csLightMap::lightcell_shift;

        v1.x = (float)(ru + Imin_u) * invww;
        v1.y = (float)(rv + Imin_v) * invhh;
        v1.z = 0;
        v2 = vv + m_t2w * v1;

	// Check if the point on the polygon is shadowed. To do this
	// we traverse all shadow frustums and see if it is contained in any of them.
	iShadowIterator* shadow_it = lp->GetShadowBlock ().GetShadowIterator ();
	bool shadow = false;
	while (shadow_it->HasNext ())
	{
	  csFrustum* shadow_frust = shadow_it->Next ();
	  if (shadow_it->IsRelevant () &&
	  	((csPolygon3D*)(shadow_it->GetUserData ())) != polygon)
	    if (shadow_frust->Contains (v2-shadow_frust->GetOrigin ()))
	    { shadow = true; break; }
	}
	shadow_it->DecRef ();

	if (!shadow)
	{
	  d = csSquaredDist::PointPoint (lightpos, v2);

	  if (d >= light->GetSquaredRadius ()) continue;
	  d = qsqrt (d);

	  float cosinus = (v2-lightpos)*polygon->GetPolyPlane ()->Normal ();
	  cosinus /= d;
	  cosinus += cosfact;
	  if (cosinus < 0) cosinus = 0;
	  else if (cosinus > 1) cosinus = 1;

	  float brightness = cosinus * light->GetBrightnessAtDistance (d);

	  if (color.red > 0)
	  {
	    l1 = QRound (color.red * brightness);
	    if (l1)
	    {
	      CS_ASSERT (uv >= 0 && uv < remap.GetSize ());
	      l1 += map[uv].red;
	      if (l1 > 255) l1 = 255;
	      map[uv].red = l1;
	    }
	  }
	  if (color.green > 0)
	  {
	    l2 = QRound (color.green * brightness);
	    if (l2)
	    {
	      CS_ASSERT (uv >= 0 && uv < remap.GetSize ());
	      l2 += map[uv].green;
	      if (l2 > 255) l2 = 255;
	      map[uv].green = l2;
	    }
	  }
	  if (color.blue > 0)
	  {
	    l3 = QRound (color.blue * brightness);
	    if (l3)
	    {
	      CS_ASSERT (uv >= 0 && uv < remap.GetSize ());
	      l3 += map[uv].blue;
	      if (l3 > 255) l3 = 255;
	      map[uv].blue = l3;
	    }
	  }
        }
      }

      if(!sy) goto finish;
      sxL += dxL;
      sxR += dxR;
      sy--;
    }
  }

finish:

  delete [] f_uv;
}

void csPolyTexture::UpdateFromShadowBitmap (csLight* light,
	const csVector3& lightpos, const csColor& lightcolor)
{
  CS_ASSERT (shadow_bitmap != NULL);

  int ww, hh;
  if (mat_handle && mat_handle->GetTexture ())
    mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
  else
    ww = hh = 64;
  float mul_u = 1.0 / ww;
  float mul_v = 1.0 / hh;

  csStatLight* slight = (csStatLight *)light;
  bool dyn = slight->IsDynamic ();

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csPolyTxtPlane* txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse ();
  csVector3& v_t2w = txt_pl->v_world2tex;

  // Cosinus factor
  float cosfact = polygon->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  if (dyn)
  {
    csShadowMap* smap = lm->FindShadowMap (light);
    if (!smap)
      smap = lm->NewShadowMap (light, w, h);
    unsigned char* shadowmap = smap->GetArray ();
    shadow_bitmap->UpdateShadowMap (shadowmap,
	csLightMap::lightcell_shift,
	Imin_u, Imin_v, mul_u, mul_v,
	m_t2w, v_t2w,
	light, lightpos,
	polygon->GetPolyPlane ()->Normal (),
	cosfact);
  }
  else
  {
    csRGBpixel* lightmap = lm->GetStaticMap ().GetArray ();
    shadow_bitmap->UpdateLightMap (lightmap,
	csLightMap::lightcell_shift,
	Imin_u, Imin_v, mul_u, mul_v,
	m_t2w, v_t2w,
	light, lightpos, lightcolor,
	polygon->GetPolyPlane ()->Normal (),
	cosfact);
  }

  delete shadow_bitmap;
  shadow_bitmap = NULL;
}

void csPolyTexture::GetTextureBox (float& fMinU, float& fMinV,
	float& fMaxU, float& fMaxV)
{
  fMinU = Fmin_u; fMaxU = Fmax_u;
  fMinV = Fmin_v; fMaxV = Fmax_v;
}

void csPolyTexture::SetPolygon (csPolygon3D* p)
{
  ipolygon = SCF_QUERY_INTERFACE_FAST (p, iPolygon3D);
  ipolygon->DecRef ();
  polygon = p;
}

bool csPolyTexture::DynamicLightsDirty () 
{ 
  return (lm && lm->dyn_dirty); 
}

void csPolyTexture::MakeDirtyDynamicLights () 
{ 
  if (lm) 
    lm->dyn_dirty = true; 
}

iLightMap* csPolyTexture::GetLightMap ()
{
  return lm;
}

int csPolyTexture::GetLightCellSize () { return csLightMap::lightcell_size; }
int csPolyTexture::GetLightCellShift () { return csLightMap::lightcell_shift; }

//------------------------------------------------------------------------------

csShadowBitmap::csShadowBitmap (int lm_w, int lm_h, int quality,
	int default_light)
{
  shadow = NULL;
  light = NULL;
  csShadowBitmap::lm_w = lm_w;
  csShadowBitmap::lm_h = lm_h;
  csShadowBitmap::quality = quality;
  csShadowBitmap::default_light = default_light;
  if (quality >= 0)
  {
    sb_w = lm_w << quality;
    sb_h = lm_h << quality;
  }
  else
  {
    sb_w = (lm_w+1) >> (-quality);
    sb_h = (lm_h+1) >> (-quality);
    if (sb_w < 1) sb_w = 1;
    if (sb_h < 1) sb_h = 1;
  }
  cnt_unshadowed = sb_w * sb_h;
  if (default_light)
    cnt_unlit = 0;
  else
    cnt_unlit = sb_w * sb_h;
}

void csShadowBitmap::LightPutPixel (int x, int y, float area, void *arg)
{
  csShadowBitmap* sb = (csShadowBitmap*)arg;
  sb->_LightPutPixel (x, y, area);
}

void csShadowBitmap::LightDrawBox (int x, int y, int w, int h, void *arg)
{
  csShadowBitmap* sb = (csShadowBitmap*)arg;
  sb->_LightDrawBox (x, y, w, h);
}

void csShadowBitmap::ShadowPutPixel (int x, int y, float area, void *arg)
{
  csShadowBitmap* sb = (csShadowBitmap*)arg;
  sb->_ShadowPutPixel (x, y, area);
}

void csShadowBitmap::ShadowDrawBox (int x, int y, int w, int h, void *arg)
{
  csShadowBitmap* sb = (csShadowBitmap*)arg;
  sb->_ShadowDrawBox (x, y, w, h);
}

void csShadowBitmap::_LightPutPixel (int x, int y, float area)
{
  if (x >= sb_w || y >= sb_h || x < 0 || y < 0) return;
  if (area < .2) return;
  CS_ASSERT (cnt_unlit >= 0);
  int idx = sb_w * y + x;
  if (!light[idx])
  {
    light[idx] = 1;
    cnt_unlit--;
  }
}

void csShadowBitmap::_LightDrawBox (int x, int y, int w, int h)
{
  if (cnt_unlit == 0) return;
  if (x+w <= 0 || y+h <= 0 || x >= sb_w || y >= sb_h) return;
  CS_ASSERT (cnt_unlit >= 0);
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x+w > sb_w) w = sb_w-x;
  if (y+h > sb_h) h = sb_h-y;

  int offs = sb_w * y + x;
  char* l = &(light[offs]);
  int delta = sb_w - w;
  while (h > 0)
  {
    int ww = w;
    while (ww > 0)
    {
      if (!*l)
      {
        *l = 1;
	cnt_unlit--;
      }
      l++;
      ww--;
    }
    l += delta;
    h--;
  }
}

void csShadowBitmap::_ShadowPutPixel (int x, int y, float area)
{
  if (x >= sb_w || y >= sb_h || x < 0 || y < 0) return;
  if (area < .2) return;
  CS_ASSERT (cnt_unshadowed >= 0);
  int idx = sb_w * y + x;
  if (!shadow[idx])
  {
    shadow[idx] = 1;
    cnt_unshadowed--;
  }
}

void csShadowBitmap::_ShadowDrawBox (int x, int y, int w, int h)
{
  if (cnt_unshadowed == 0) return;
  if (x+w <= 0 || y+h <= 0 || x >= sb_w || y >= sb_h) return;
  CS_ASSERT (cnt_unshadowed >= 0);
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x+w > sb_w) w = sb_w-x;
  if (y+h > sb_h) h = sb_h-y;

  int offs = sb_w * y + x;
  char* s = &(shadow[offs]);
  int delta = sb_w - w;
  while (h > 0)
  {
    int ww = w;
    while (ww > 0)
    {
      if (!*s)
      {
        *s = 1;
	cnt_unshadowed--;
      }
      s++;
      ww--;
    }
    s += delta;
    h--;
  }
}

void csShadowBitmap::RenderTotal (int val)
{
  if (!shadow)
  {
    shadow = new char [sb_w * sb_h];
    memset (shadow, val, sb_w * sb_h);
    if (val) cnt_unshadowed = 0;
    else cnt_unshadowed = sb_w * sb_h;
    light = new char [sb_w * sb_h];
    int lv = val ? default_light : 1;
    memset (light, lv, sb_w * sb_h);
    if (lv) cnt_unlit = 0;
    else cnt_unlit = sb_w * sb_h;
    return;
  }

  if (val == 1)
  {
    memset (shadow, 1, sb_w * sb_h);
    cnt_unshadowed = 0;
  }
  else
  {
    memset (light, 1, sb_w * sb_h);
    cnt_unlit = 0;
  }
  return;
}

void csShadowBitmap::RenderPolygon (csVector2* shadow_poly, int num_vertices,
	int val)
{
  int i;
  //-------------------
  // First we convert the polygon from lightmap coordinates to shadow
  // bitmap coordinates.
  //-------------------
  if (quality > 0)
  {
    float mul = float (1 << quality);
    for (i = 0 ; i < num_vertices ; i++)
      shadow_poly[i] = shadow_poly[i] * mul;
  }
  else if (quality < 0)
  {
    float div = 1. / float (1 << -quality);
    for (i = 0 ; i < num_vertices ; i++)
      shadow_poly[i] = shadow_poly[i] * div;
  }

  if (!shadow)
  {
    shadow = new char [sb_w * sb_h];
    memset (shadow, 0, sb_w * sb_h);
    light = new char [sb_w * sb_h];
    memset (light, default_light, sb_w * sb_h);
  }

  if (val == 1)
    csAntialiasedPolyFill (shadow_poly, num_vertices, this,
  	ShadowPutPixel, ShadowDrawBox);
  else
    csAntialiasedPolyFill (shadow_poly, num_vertices, this,
  	LightPutPixel, LightDrawBox);
  return;
}

float csShadowBitmap::GetLighting (int lm_u, int lm_v)
{
  if (!shadow) return 1;
  CS_ASSERT (lm_u >= 0 && lm_u < lm_w);
  CS_ASSERT (lm_v >= 0 && lm_v < lm_h);
  if (quality == 0)
  {
    // Shadow-bitmap has equal quality.
    int idx = lm_v * sb_w + lm_u;
    bool l = light[idx] && !shadow[idx];
    return float (l);
  }
  else if (quality > 0)
  {
    // Shadow-bitmap has better quality.
    // Here we will take an average of all shadow-bitmap values.
    int d = 1 << (quality-1);
    int u = lm_u << quality;
    int v = lm_v << quality;
    // Calculate the bounds in shadow-bitmap space for which we
    // are going to take the average. Make sure we don't exceed
    // the total bounds of the shadow-bitmap.
    int minu = u-d; if (minu < 0) minu = 0;
    int maxu = u+d; if (maxu > sb_w-1) maxu = sb_w-1;
    int minv = v-d; if (minv < 0) minv = 0;
    int maxv = v+d; if (maxv > sb_h-1) maxv = sb_h-1;
    int du = maxu-minu+1;
    int dv = maxv-minv+1;
    int s = 0;
    int idx = minv * sb_w + minu;
    char* bml = &light[idx];
    char* bms = &shadow[idx];
    int delta = sb_w - du;
    int ddv = dv;
    while (ddv > 0)
    {
      int ddu = du;
      while (ddu > 0)
      {
        s += (*bml) && !*bms;
	bml++; bms++;
	ddu--;
      }
      bml += delta;
      bms += delta;
      ddv--;
    }
    return float (s) / float (du * dv);
  }
  else
  {
    // Shadow-bitmap has lower quality.
    // Here we will interpolate shadow-bitmap values.
    // @@@ TODO!!!
    return 1;
  }
}

void csShadowBitmap::UpdateLightMap (csRGBpixel* lightmap,
	int lightcell_shift,
	float shf_u, float shf_v,
	float mul_u, float mul_v,
	const csMatrix3& m_t2w, const csVector3& v_t2w,
	csLight* light, const csVector3& lightpos,
	const csColor& lightcolor,
	const csVector3& poly_normal,
	float cosfact)
{
  if (IsFullyShadowed () || IsFullyUnlit ()) return;
  float light_r = lightcolor.red * CS_NORMAL_LIGHT_LEVEL;
  float light_g = lightcolor.green * CS_NORMAL_LIGHT_LEVEL;
  float light_b = lightcolor.blue * CS_NORMAL_LIGHT_LEVEL;
  bool ful_lit = IsFullyLit ();
  int i, j;
  int base_uv = 0;
  float rv_step = (1 << lightcell_shift) * mul_v;
  float ru_step = (1 << lightcell_shift) * mul_u;
  float rv = shf_v * mul_v;
  float ru_base = shf_u * mul_u - ru_step;

  csVector3 v_ru (m_t2w.m11, m_t2w.m21, m_t2w.m31);
  csVector3 v_rv (m_t2w.m12, m_t2w.m22, m_t2w.m32);
  csVector3 v, v_base;

  v_base = v_t2w + rv*v_rv + ru_base*v_ru;
  v_rv *= rv_step;
  v_ru *= ru_step;

  for (i = 0 ; i < lm_h ; i++)
  {
    int uv = base_uv;
    base_uv += lm_w;

    v = v_base;
    v_base += v_rv;

    for (j = 0 ; j < lm_w ; j++, uv++)
    {
//@@@
//if (i == j) lightmap[uv].red = 255;
//if (i == lm_w-1-j) lightmap[uv].blue = 255;
//@@@

      // our v vector calculation is equivalent to
      // int ru = j << lightcell_shift;
      // int rv = i << lightcell_shift;
      // csVector3 v (float (ru + shf_u) * mul_u, float (rv + shf_v) * mul_v, 0);
      // v = v_t2w + m_t2w * v;

      v += v_ru;

      float lightness;
      if (ful_lit)
        lightness = 1;
      else
      {
        lightness = GetLighting (j, i);
        if (lightness < EPSILON)
          continue;
      }

      // @@@ Optimization: It should be possible to combine these
      // calculations into a more efficient formula.

      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= light->GetSquaredRadius ()) continue;

      d = qsqrt (d);

      float cosinus = (v - lightpos) * poly_normal;
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0)
        continue;
      else if (cosinus > 1)
        cosinus = 1;

      float scale = cosinus * light->GetBrightnessAtDistance (d) * lightness;

      int l;
      csRGBpixel &lumel = lightmap[uv];
      l = lumel.red + QRound (light_r * scale);
      lumel.red = l < 255 ? l : 255;
      l = lumel.green + QRound (light_g * scale);
      lumel.green = l < 255 ? l : 255;
      l = lumel.blue + QRound (light_b * scale);
      lumel.blue = l < 255 ? l : 255;
    }
  }
}

void csShadowBitmap::UpdateShadowMap (unsigned char* shadowmap,
	int lightcell_shift,
	float shf_u, float shf_v,
	float mul_u, float mul_v,
	const csMatrix3& m_t2w, const csVector3& v_t2w,
	csLight* light, const csVector3& lightpos,
	const csVector3& poly_normal,
	float cosfact)
{
  if (IsFullyShadowed () || IsFullyLit ()) return;
  bool ful_lit = IsFullyLit ();
  int i, j;
  int base_uv = 0;
  float rv_step = (1 << lightcell_shift) * mul_v;
  float ru_step = (1 << lightcell_shift) * mul_u;
  float rv = shf_v * mul_v;
  float ru_base = shf_u * mul_u - ru_step;

  csVector3 v_ru (m_t2w.m11, m_t2w.m21, m_t2w.m31);
  csVector3 v_rv (m_t2w.m12, m_t2w.m22, m_t2w.m32);
  csVector3 v, v_base;

  v_base = v_t2w + rv*v_rv + ru_base*v_ru;
  v_rv *= rv_step;
  v_ru *= ru_step;

  for (i = 0 ; i < lm_h ; i++)
  {
    int uv = base_uv;
    base_uv += lm_w;

    v = v_base;
    v_base += v_rv;
    for (j = 0 ; j < lm_w ; j++, uv++)
    {
      v += v_ru;

      float lightness;
      if (ful_lit)
        lightness = 1;
      else
      {
        lightness = GetLighting (j, i);
        if (lightness < EPSILON)
          continue;
      }

      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= light->GetSquaredRadius ()) continue;

      d = qsqrt (d);

      float cosinus = (v - lightpos) * poly_normal;
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0)
        continue;
      else if (cosinus > 1)
        cosinus = 1;

      float brightness = cosinus * light->GetBrightnessAtDistance (d);

      int l = shadowmap[uv] +
      	QRound (CS_NORMAL_LIGHT_LEVEL * lightness * brightness);
      shadowmap[uv] = l < 255 ? l : 255;
    }
  }
}

//------------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csLightingPolyTexQueue)
  SCF_IMPLEMENTS_INTERFACE (iFrustumViewUserdata)
  SCF_IMPLEMENTS_INTERFACE (iLightingProcessInfo)
SCF_IMPLEMENT_IBASE_END

csLightingPolyTexQueue::csLightingPolyTexQueue (csLight* light,
	bool dynamic, bool gouraud_only)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csLightingPolyTexQueue::light = light;
  csLightingPolyTexQueue::dynamic = dynamic;
  csLightingPolyTexQueue::gouraud_only = gouraud_only;
}

csLightingPolyTexQueue::~csLightingPolyTexQueue ()
{
}

iLight* csLightingPolyTexQueue::GetLight () const
{
  return light ? &(light->scfiLight) : 0;
}

void csLightingPolyTexQueue::AddPolyTexture (csPolyTexture* pt)
{
  polytxts.Push (pt);
}

void csLightingPolyTexQueue::UpdateMaps (csLight* light,
	const csVector3& lightpos, const csColor& lightcolor)
{
  int i;
  for (i = 0 ; i < polytxts.Length () ; i++)
  {
    csPolyTexture* pt = (csPolyTexture*)polytxts[i];
    pt->UpdateFromShadowBitmap (light, lightpos, lightcolor);
  }
  polytxts.DeleteAll ();
}

//------------------------------------------------------------------------------

