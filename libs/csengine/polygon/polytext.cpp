/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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
#define SYSDEF_ALLOCA
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
#include "csengine/quadcube.h"
#include "csengine/lghtmap.h"
#include "csutil/bitset.h"
#include "igraph3d.h"
#include "itexture.h"

// This is a private class used to track all postponed lighting references
// for a lightmap that is shared by several polygons. See the large comment
// before csPolyTexture::GetLightmapBounds for details.
// A object of this type is inserted into original polygon, so that no extra
// storage except that provided by csObject is required.
class csDelayedLightingInfo : public csFrustrumViewCleanup
{
  struct LightViewInfo
  {
    // The view frustum
    csFrustumView *frustum;
    // The old value of csFrustrumViewCleanup "next" field
    csFrustrumViewCleanup *old_next;
    // The list of shadow frustums
    csVector shadows;
    // The list of polygons that are still unlit; if NULL we have no more shares
    csPolygon3D *unlit_poly;

    LightViewInfo (csFrustumView *frust, csFrustrumViewCleanup *next,
      csPolygon3D *orig_poly)
      : shadows (32, 32)
    {
      frustum = frust;
      old_next = next;

      // Count how many visible polygons that share this lightmap we have
      // and move all of them to our list
      csPolygon3D *poly = orig_poly;
      csPolygon3D *prev_poly = NULL;
      unlit_poly = NULL;
      while (poly)
      {
        if (!poly->flags.Check (CS_POLY_SPLIT))
	{
          // orig_poly should be always invisible
          if (prev_poly)
          {
            prev_poly->SetNextShare (poly->GetNextShare ());
            poly->SetNextShare (unlit_poly);
            unlit_poly = poly;
            poly = prev_poly->GetNextShare ();
          }
          else
            DEBUG_BREAK;
        }
        else
          poly = (prev_poly = poly)->GetNextShare ();
      }
    }

    int GetShadowCount ()
    { return shadows.Length (); }

    csShadowFrustum *GetShadow (int idx)
    { return (csShadowFrustum *)(idx < shadows.Length () ? shadows.Get (idx) : NULL); }

    ~LightViewInfo ()
    {
      for (int i = 0; i < shadows.Length (); i++)
        GetShadow (i)->DecRef ();
    }

    void CheckShadow (csShadowFrustum *frust, int count)
    {
      if (!frust->relevant)
        return;
      for (int i = 0; i < count; i++)
        if (GetShadow (i) == frust)
          return;
      frust->IncRef ();
      shadows.Push (frust);
    }
  };

  // The list of lightviewinfo structures
  DECLARE_TYPED_VECTOR (LightViewVector, LightViewInfo) lvlist;

  // The polygon texture we are filling
  csPolyTexture *polytex;

public:
  // Constructor
  csDelayedLightingInfo (csPolyTexture *ptex);

  // Return total number of shadows
  int GetShadowCount ()
  { return lvlist.Get (lvlist.Length () - 1)->GetShadowCount (); }
  // Get Nth shadow frustum
  csShadowFrustum *GetShadow (int idx)
  { return lvlist.Get (lvlist.Length () - 1)->GetShadow (idx); }

  // Collect a reference from given frustum to our lightmap
  bool Collect (csFrustumView *lview, csPolygon3D *poly);

  // Delete the top lighting info on the stack.
  bool FinishLightView ();

  // Get any of the polygons that share this lightmap
  csPolygon3D *GetPolygon ()
  { return polytex->GetCSPolygon (); }
  // Query unlit polygon count
  int GetUnlitPolyCount ();
  // Get next unlit polygon
  csPolygon3D *GetNextUnlitPolygon ();
};

csDelayedLightingInfo::csDelayedLightingInfo (csPolyTexture *ptex) : lvlist (4, 4)
{
  polytex = ptex;
  action = csPolyTexture::ProcessDelayedLightmaps;
}

bool csDelayedLightingInfo::Collect (csFrustumView *lview, csPolygon3D *poly)
{
  // If we have no lighting info, or this frustumview is different from the
  // last collected frustum, we have a new (recursive) light source, that
  // possibly came through a mirror or warped portal or such. Note that we
  // CAN use frustumview address to check uniquity because a frustumview
  // cannot be deleted (and another one created on the same address) without
  // the frustum being removed from lvlist (this is done from the cleanup
  // callback, called from frustumview destructor).
  if (!lvlist.Length ()
   || lvlist.Get (lvlist.Length () - 1)->frustum != lview)
  {
#ifdef CS_DEBUG
    // Check if this frustum has been seen EARLIER: if so, this means that
    // during the destruction of frustum views that are "on top" of it
    // ProcessDelayedLightMaps() has not been called.
    for (int i = lvlist.Length () - 2; i >= 0; i--)
      if (lvlist.Get (i)->frustum == lview)
        DEBUG_BREAK;
#endif
    // Create a new light frustum info structure
    lvlist.Push (new LightViewInfo (lview, next, poly->GetBasePolygon ()));
    // Register with that frustumview for cleanup
    lview->RegisterCleanup (this);
  }

  LightViewInfo *lvi = lvlist.Get (lvlist.Length () - 1);

  // Check if this polygon is one of those that shares the lightmap
  csPolygon3D *cur_poly = lvi->unlit_poly, *prev_poly = NULL;
  csPolygon3D *orig_poly = poly->GetBasePolygon ();
  while (cur_poly)
  {
    if (cur_poly == poly)
    {
      if (!prev_poly)
        lvi->unlit_poly = poly->GetNextShare ();
      else
        prev_poly->SetNextShare (poly->GetNextShare ());

      poly->SetNextShare (orig_poly->GetNextShare ());
      orig_poly->SetNextShare (poly);
      break;
    }
    cur_poly = (prev_poly = cur_poly)->GetNextShare ();
  }

  // We should never cleanly quit ot of loop. If it is so, it means
  // we have a polygon that is not on share list (which means something
  // went wrong).
  CS_ASSERT (cur_poly);

  // Check if any shadow frustums we have now have not been seen in the past
  csShadowFrustum *csf = lview->shadows.GetFirst ();
  int ns = lvi->shadows.Length ();
  while (csf)
  {
    lvi->CheckShadow (csf, ns);
    csf = csf->next;
  }
  return !lvi->unlit_poly;
}

bool csDelayedLightingInfo::FinishLightView ()
{
  int idx = lvlist.Length () - 1;

  // We shouldn't get here for empty DelayedLightInfo's
  CS_ASSERT (idx >= 0);

  // Deregister ourselves from the cleanup list of frustumview
  lvlist.Get (idx)->frustum->DeregisterCleanup (this);
  next = lvlist.Get (idx)->old_next;

  lvlist.Delete (idx);
  return (lvlist.Length () == 0);
}

int csDelayedLightingInfo::GetUnlitPolyCount ()
{
  LightViewInfo *lvi = lvlist.Get (lvlist.Length () - 1);
  int count = 0;
  csPolygon3D *poly = lvi->unlit_poly;
  while (poly) { count++; poly = poly->GetNextShare (); }
  return count;
}

csPolygon3D *csDelayedLightingInfo::GetNextUnlitPolygon ()
{
  LightViewInfo *lvi = lvlist.Get (lvlist.Length () - 1);
  csPolygon3D *poly = lvi->unlit_poly;
  if (!poly) return NULL;

  // Remove polygon from our unsplit polygon list
  lvi->unlit_poly = poly->GetNextShare ();
  csPolygon3D *orig_poly = poly->GetBasePolygon ();
  poly->SetNextShare (orig_poly->GetNextShare ());
  orig_poly->SetNextShare (poly);

  return poly;
}

//------------------------------------------------------------------------------

// Option variable: cosinus factor.
float csPolyTexture::cfg_cosinus_factor = 0;

#define lightcell_size	csLightMap::lightcell_size
#define lightcell_shift	csLightMap::lightcell_shift

IMPLEMENT_IBASE (csPolyTexture)
  IMPLEMENTS_INTERFACE (iPolygonTexture)
IMPLEMENT_IBASE_END

csPolyTexture::csPolyTexture ()
{
  CONSTRUCT_IBASE (NULL);
  lm = NULL;
  cache_data [0] = cache_data [1] = cache_data [2] = cache_data [3] = NULL;
}

csPolyTexture::~csPolyTexture ()
{
  csEngine::current_engine->G3D->RemoveFromCache (this);
  if (lm) lm->DecRef ();
}

void csPolyTexture::SetLightMap (csLightMap* lightmap)
{
  if (lm) lm->DecRef ();
  lm = lightmap;
  if (lm) lm->IncRef ();
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
  for (i = 0; i < polygon->GetVertices ().GetNumVertices (); i++)
  {
    v1 = polygon->Vwor (i);           // Coordinates of vertex in world space.
    v1 -= txt_pl->v_world2tex;
    v2 = (txt_pl->m_world2tex) * v1;  // Coordinates of vertex in texture space.
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
  mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
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
  // first combine the static and pseudo-dynamic lights
  if (!lm || !lm->UpdateRealLightMap() )
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

void csPolyTexture::ProcessDelayedLightmaps (csFrustumView *lview,
  csFrustrumViewCleanup *lighting_info)
{
  csDelayedLightingInfo *dli = (csDelayedLightingInfo *)lighting_info;

  // Apply lighting to the lightmap
  dli->GetPolygon ()->CalculateDelayedLighting (lview);
}

/**
 * We need a way to avoid applying lighting to the same lightmap several times
 * by the same light. This can happen if lightmap is shared by several polygons.
 *
 * The solution is to maintain a "hit count" for each lightmap. When hit count
 * reaches the number of polygons that share the lightmap, the proper lighting
 * is performed. Also during each "hit" we collect the light and all shadow
 * frustums that are meant to hit our lightmap, and during lighting process
 * we first apply the light frustum (which can be only one) and then all
 * shadow frustums. This resolves the issue with STATBSP levels (where the
 * list of frustums may be incomplete during any of hits, because the lighing
 * is performed in a recursive tree-walk manner, and shadows are added/removed
 * to frustumview dynamically).
 *
 * To avoid the situation when frustum is destroyed before we can use it, we
 * maintain a reference count for every frustum. When we take a frustum to be
 * processed later, we increment his reference counter.
 *
 * Since not all polygons that share a lightmap may be hit by a single light,
 * the ProcessDelayedLightmaps method is called as soon as a frustumview
 * is going to be destroyed, so that all lightmaps which's lighting was
 * postponed can be processeed at least now.
 */
bool csPolyTexture::GetLightmapBounds (csFrustumView *lview, csVector3 *bounds)
{
  csPolyTxtPlane *txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse ();
  csVector3 &v_t2w = txt_pl->v_world2tex;

  int lmw = lm->rwidth;
  int lmh = lm->rheight;
  csVector3 &lightpos = lview->light_frustum->GetOrigin ();

  int ww, hh;
  mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
  float inv_ww = 1.0 / ww;
  float inv_hh = 1.0 / hh;

  // Calculate the responsability grid bounds as a polygon in world space.
  for (int i = 0; i < 4; i++)
  {
    csVector3 v;
    v.x = (i == 0 || i == 3) ? -0.5 : lmw - 0.5;
    v.y = (i < 2) ? -0.5 : lmh - 0.5;
    v.z = 0.0;

    v.x = ((v.x * lightcell_size) + Imin_u) * inv_ww;
    v.y = ((v.y * lightcell_size) + Imin_v) * inv_hh;

    v = (m_t2w * v + v_t2w) - lightpos;

    bounds [lview->mirror ? 3 - i : i] = v;
  } /* endfor */

  // If the lightmap is shared, take care to fill lightmap only
  // after all shadow frustums are collected.
  return !(lm->delayed_light_info || polygon->GetNextShare ());
}

bool csPolyTexture::CollectShadows (csFrustumView *lview, csPolygon3D *poly)
{
  if (!lm->delayed_light_info)
    lm->delayed_light_info = new csDelayedLightingInfo (this);

  return (lm->delayed_light_info->Collect (lview, poly));
}

//
// Here goes a little theory about lightmap calculations (A.Z.).
// Since lightmaps are bi-linearily interpolated (both for hardware and
// software 3D drivers) and every light cell covers a relatively large number
// of texels (typically 16x16), the lightmap is like a grid that covers the
// actual lighted polygon. The picture below illustrates a lightmap grid
// covering a irregular polygon:
//
//   0      1      2      3
// 0 *------*------*-*====* Thick lines denotes the grid, thin lines represents
//   #\     #      # |    # the polygon. Since the light cells are always square
//   # \    #      # |    # (usually they are 16x16 texels) the lightmap is a
//   #  \   #      # *    # bit larger than the polygon. The lightmap contains
// 1 *===\==*======*/=====* the lighting values in the nodes, or crosses of the
//   #    \ #      /      # lightmap grid. We will address lightmap cells as
//   #     *------*#      # lm[x,y], for example the top-left lightmap cell is
//   #      #      #      # lm[0,0], the bottom-right lightmap cell is lm[3,2].
// 2 *======*======*======*
//           fig.1
//
// Every lightmap cell is basically responsible for containing the average
// light level for a certain area around the grid cross. We will consider
// these areas as squares that are located around every grid cross (which
// form another grid themselves, which we will call "responsability grid"
// in the following):
//
//     a  0  b  1  c  2  d    Thick lines denotes the actual lightmap grid.
//   a +..#..+..#..+..#..+    Dot lines shows the "responsability domain"
//     :  #  :  #  :  #  :    for each lightmap cell. Thus, the lightmap
// 0 =====*-----*-----*-*===  cell lm[0,0] is responsable for the following
//     :  #\ :  #  :  # |:    area: [a,a] - [b,a] - [b,b] - [a,b]. This means
//   b +..#.\+..#..+..#.|+    that the lighting value in the cell [0,0] should
//     :  #  \  #  :  # *:    approximate the lightness of the entire mentioned
// 1 =====*===\=*=====*/====  square. Mean square lightness is computed as
//     :  #  : \#  :  /  :    light intensity in the lightmap grid cross
//   c +..#..+..*----*#..+    multiplied by a factor that consist of:
//     :  #  :  #  :  #  :    - Plus area of domain that is covered by light.
// 2 =====*=====*=====*=====    That is, if light frustum is limited by
//     :  #  :  #  :  #  :      something (e.g. a portal), we should see first
//   d +..#..+..#..+..#..+      if the light covers given domain, and how much.
//        #     #     #       - Minus area of domain that is covered by shadows.
//           fig.2              Note that these areas can overlap, e.g. a light
//                              can be blocked by a shadow.
//
// The part of sub-polygon that is covered by the light is computed in the
// following way: first we clip all shadow frustums (in 2D) against the
// light frustum. That is, the parts of "shadow polygons" that are outside
// of the "light polygon" are removed. Then we clip every shadow polygon
// against each other to get the common shadow area polygons, to compensate
// later for doubly-shaded areas. The shadow polygons are left in their
// original form, but we remember the result of each clipping.
//
// Finally, we proceed to split all polygons against the responsability grid:
// one light polygon, several shadow polygons and several "doubly-shadowed"
// polygons. The result of every split is the sub-area of polygon that falls
// inside each grid cell. We initialize every cell of the lighting matrix to
// zero, then we add to every cell the area of the light polygon and the area
// of "doubly-shadowed" polygons; finally we subtract the area of shadow
// polygons.
//

static void __add_PutPixel (int x, int y, float area, void *arg)
{
  csCoverageMatrix *cm = (csCoverageMatrix *)arg;
  if (x >= cm->width || y >= cm->height || x < 0 || y < 0)
  {
#ifdef CS_DEBUG
    CsPrintf(MSG_INTERNAL_ERROR, "Array bound error in file %s, line %d\n",
      __FILE__, __LINE__);
#endif
    return;
  }
  cm->coverage [cm->width * y + x] += area;
}

static void __add_DrawBox (int x, int y, int w, int h, void *arg)
{
  csCoverageMatrix *cm = (csCoverageMatrix *)arg;
  if (x >= cm->width || y >= cm->height || x < 0 || y < 0 ||
      x + w > cm->width || y + h > cm->height || w < 0 || h < 0)
  {
#ifdef CS_DEBUG
    CsPrintf(MSG_INTERNAL_ERROR, "Array bound error in file %s, line %d\n",
      __FILE__, __LINE__);
#endif
    return;
  }
  int ofs = cm->width * y + x;
  int delta = cm->width - w;
  for (int yy = h; yy > 0; yy--)
  {
    for (int xx = w; xx > 0; xx--)
      cm->coverage [ofs++] += 1.0;
    ofs += delta;
  } /* endfor */
}

static void __sub_PutPixel (int x, int y, float area, void *arg)
{
  csCoverageMatrix *cm = (csCoverageMatrix *)arg;
  if (x >= cm->width || y >= cm->height || x < 0 || y < 0)
  {
#ifdef CS_DEBUG
    CsPrintf(MSG_INTERNAL_ERROR, "Array bound error in file %s, line %d\n",
      __FILE__, __LINE__);
#endif
    return;
  }
  cm->coverage [cm->width * y + x] -= area;
}

static void __sub_DrawBox (int x, int y, int w, int h, void *arg)
{
  csCoverageMatrix *cm = (csCoverageMatrix *)arg;
  if (x >= cm->width || y >= cm->height || x < 0 || y < 0 ||
      x + w > cm->width || y + h > cm->height || w < 0 || h < 0)
  {
#ifdef CS_DEBUG
    CsPrintf(MSG_INTERNAL_ERROR, "Array bound error in file %s, line %d\n",
      __FILE__, __LINE__);
#endif
    return;
  }
  int ofs = cm->width * y + x;
  int delta = cm->width - w;
  for (int yy = h; yy > 0; yy--)
  {
    for (int xx = w; xx > 0; xx--)
      cm->coverage [ofs++] -= 1.0;
    ofs += delta;
  } /* endfor */
}

void csPolyTexture::GetCoverageMatrix (csFrustumView& lview, csCoverageMatrix &cm)
{
  if (!lm) return;

  int ww, hh;
  mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);

  csPolyTxtPlane *txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();

  csVector3 &lightpos = lview.light_frustum->GetOrigin ();
  csDelayedLightingInfo *dli = lm->delayed_light_info;

  // Now allocate the space for the projected lighted polygon
  int nvlf = lview.light_frustum->GetNumVertices ();
  csVector3 *lf3d = lview.light_frustum->GetVertices ();
  ALLOC_STACK_ARRAY (lf2d, csVector2, nvlf);
  // Project the light polygon from world space to responsability grid space
  float inv_lightcell_size = 1.0 / lightcell_size;
  int i, j;
  for (i = 0; i < nvlf; i++)
  {
    // T = Mwt * (W - Vwt)
    csVector3 v = txt_pl->m_world2tex *
      (lf3d [i] + lightpos - txt_pl->v_world2tex);
    lf2d [i].x = (v.x * ww - Imin_u) * inv_lightcell_size + 0.5;
    lf2d [i].y = (v.y * hh - Imin_v) * inv_lightcell_size + 0.5;
  }

  // Now fill the lightmap polygon with light coverage values
  csAntialiasedPolyFill (lf2d, nvlf, &cm, __add_PutPixel, __add_DrawBox);

  // Now subtract all shadow polygons from the coverage matrix.
  // At the same time, add the overlapping shadows to the coverage matrix.
  int nsf;
  csShadowFrustum *csf;
  if (dli)
  {
    nsf = dli->GetShadowCount () + dli->GetUnlitPolyCount ();
    csf = dli->GetShadow (0);
  }
  else
  {
    nsf = 0;
    csf = lview.shadows.GetFirst ();
    while (csf) { if (csf->relevant) nsf++; csf = csf->next; }
    csf = lview.shadows.GetFirst ();
  }

  ALLOC_STACK_ARRAY (sfc, csPolygonClipper *, nsf);
  for (i = 0; i < nsf; i++, csf = (dli ? dli->GetShadow (i) : csf->next))
  {
    if (!dli)
      while (!csf->relevant)
        csf = csf->next;

    // MAX_OUTPUT_VERTICES should be far too enough
    csVector2 s2d [MAX_OUTPUT_VERTICES];
    int nv;
    bool mirror;

    if (!dli || i < dli->GetShadowCount ())
    {
      csFrustum *shadow = csf->Intersect (*lview.light_frustum);
      if (!shadow) { sfc [i] = NULL; continue; }

      // Translate the shadow frustum to polygon plane
      nv = shadow->GetNumVertices ();
      csVector3 *s3d = shadow->GetVertices ();
      if (nv > MAX_OUTPUT_VERTICES) nv = MAX_OUTPUT_VERTICES;
      mirror = !shadow->IsMirrored ();
      for (j = 0; j < nv; j++)
      {
        csVector3 v = txt_pl->m_world2tex *
          (s3d [j] + lightpos - txt_pl->v_world2tex);
        s2d [j].x = (v.x * ww - Imin_u) * inv_lightcell_size + 0.5;
        s2d [j].y = (v.y * hh - Imin_v) * inv_lightcell_size + 0.5;
      }
      delete shadow;
    }
    else
    {
      mirror = false;
      csPolygon3D *poly = dli->GetNextUnlitPolygon ();
      nv = poly->GetNumVertices ();
      if (nv > MAX_OUTPUT_VERTICES) nv = MAX_OUTPUT_VERTICES;
      float w2 = cm.width / 2.0;
      float h2 = cm.height / 2.0;
      for (j = 0; j < nv; j++)
      {
        // T = Mwt * (W - Vwt)
        csVector3 v = txt_pl->m_world2tex * (poly->Vwor (j) - txt_pl->v_world2tex);
        float x = (v.x * ww - Imin_u) * inv_lightcell_size;
        float y = (v.y * hh - Imin_v) * inv_lightcell_size;
        if (x > w2) x += 1.0;
        if (y > h2) y += 1.0;
        s2d [j].x = x;
        s2d [j].y = y;
      }
    }

    // Now subtract the shadow from the coverage matrix
    csAntialiasedPolyFill (s2d, nv, &cm, __sub_PutPixel, __sub_DrawBox);

//@@@@@@@
//todo: implement accurate lighting (computing overlapping portions of
//shadow polygons and add/subtract them accordingly).
#if 1
    sfc [i] = NULL;
#else
    // Create a polygon clipper from this shadow polygon
    sfc [i] = new csPolygonClipper (s2d, nv, mirror, true);

    // Now subtract the common part of this and
    // all previous shadows from the coverage matrix.
    for (k = 0; k < i; k++)
    {
      if (!sfc [k]) continue;
      // Intersect current shadow with one of the previous shadows
      csVector2 sfi [MAX_OUTPUT_VERTICES];
      int sfic;
      if (sfc [k]->Clip (s2d, nv, sfi, sfic) == CS_CLIP_OUTSIDE)
        continue;

      // Now add the common shadow to the coverage matrix
      csAntialiasedPolyFill (sfi, sfic, &cm, __add_PutPixel, __add_DrawBox);

    } /* endfor */
#endif

  } /* endfor */

  // Free all shadow frustums
  for (i = 0; i < nsf; i++)
    delete sfc [i];

  // Free delayed lighting info if needed
  if (dli && dli->FinishLightView ())
  {
    lm->delayed_light_info = NULL;
    delete dli;
  }
}

void csPolyTexture::FillLightMap (csFrustumView& lview)
{
  if (!lm) return;

  // We will compute the lighting of the entire lightmap, disregarding
  // polygon bounds. This removes both the "black borders" and "white
  // borders" problems. However, we should take care not to fill same
  // lightmap twice, otherwise we'll get very bright lighting for
  // shared lightmaps.
  int lmw = lm->rwidth;
  int lmh = lm->rheight;
  // Create the light coverage array
  csCoverageMatrix lc (lmw, lmh);
  // Compute the light coverage for every cell of the lightmap
  GetCoverageMatrix (lview, lc);

  unsigned char *mapR;
  unsigned char *mapG;
  unsigned char *mapB;
  csShadowMap *smap = NULL;

  int ww, hh;
  mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
  float inv_ww = 1.0 / ww;
  float inv_hh = 1.0 / hh;

  bool hit = false;         // Set to true if there is a hit
  bool first_time = false;  // Set to true if this is the first pass for the dynamic light
  csStatLight *light = (csStatLight *)lview.userdata;
  bool dyn = light->IsDynamic ();
  csVector3 &lightpos = lview.light_frustum->GetOrigin ();

  if (dyn)
  {
    smap = lm->FindShadowMap (light);
    if (!smap)
    { smap = lm->NewShadowMap (light, w, h); first_time = true; }
    else
      first_time = false;
    mapR = smap->map;
    mapG = NULL;
    mapB = NULL;
  }
  else
  {
    mapR = lm->GetStaticMap ().GetRed ();
    mapG = lm->GetStaticMap ().GetGreen ();
    mapB = lm->GetStaticMap ().GetBlue ();
  }

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csPolyTxtPlane *txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse ();
  csVector3 &v_t2w = txt_pl->v_world2tex;

  // Cosinus factor
  float cosfact = polygon->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  // Finally, use the light coverage values to compute the actual lighting
  int covaddr = 0;
  float light_r = lview.r * NORMAL_LIGHT_LEVEL;
  float light_g = lview.g * NORMAL_LIGHT_LEVEL;
  float light_b = lview.b * NORMAL_LIGHT_LEVEL;
  for (int i = 0; i < lmh; i++)
  {
    int uv = i * lm->GetWidth ();
    for (int j = 0; j < lmw; j++, uv++)
    {
      float lightness = lc.coverage [covaddr++];
      if (lightness < EPSILON)
        continue;

      int ru = j << lightcell_shift;
      int rv = i << lightcell_shift;

      csVector3 v (float (ru + Imin_u) * inv_ww, float (rv + Imin_v) * inv_hh, 0);
      v = v_t2w + m_t2w * v;

      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= light->GetSquaredRadius ()) continue;

      d = qsqrt (d);
      hit = true;

      float cosinus = (v - lightpos) * polygon->GetPolyPlane ()->Normal ();
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0)
        cosinus = 0;
      else if (cosinus > 1)
        cosinus = 1;

      int l;
      float brightness = cosinus * light->GetBrightnessAtDistance (d);

      if (dyn)
      {
        l = mapR [uv] + QRound (NORMAL_LIGHT_LEVEL * lightness * brightness);
        mapR [uv] = l < 255 ? l : 255;
      }
      else
      {
        l = mapR [uv] + QRound (light_r * lightness * brightness);
        mapR [uv] = l < 255 ? l : 255;
        l = mapG [uv] + QRound (light_g * lightness * brightness);
        mapG [uv] = l < 255 ? l : 255;
        l = mapB [uv] + QRound (light_b * lightness * brightness);
        mapB [uv] = l < 255 ? l : 255;
      } /* endif */
    } /* endfor */
  } /* endfor */
}

/* Modified by me to correct some lightmap's border problems -- D.D. */
void csPolyTexture::ShineDynLightMap (csLightPatch* lp)
{
  int lw = 1 + ((w_orig + lightcell_size - 1) >> lightcell_shift);
  int lh = 1 + ((h + lightcell_size - 1) >> lightcell_shift);

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

  csRGBLightMap& remap = lm->GetRealMap ();
  csDynLight* light = (csDynLight*)(lp->light);
  unsigned char* mapR = remap.GetRed ();
  unsigned char* mapG = remap.GetGreen ();
  unsigned char* mapB = remap.GetBlue ();

  int i;
  float miny = 1000000, maxy = -1000000;
  int MaxIndex = -1, MinIndex = -1;
  float inv_lightcell_size = 1.0 / lightcell_size;

  // Calculate the uv's for all points of the frustum (the
  // frustum is actually a clipped version of the polygon).
  csVector2* f_uv = NULL;
  if (lp->vertices)
  {
    int mi;
    f_uv = new csVector2 [lp->num_vertices];
    for (i = 0 ; i < lp->num_vertices ; i++)
    {
      //if (lview.mirror) mi = lview.num_frustum-i-1;
      //else mi = i;
      mi = i;

      // T = Mwt * (W - Vwt)
      //v1 = pl->m_world2tex * (lp->vertices[mi] + lp->center - pl->v_world2tex);
      //@@@ This is only right if we don't allow reflections on dynamic lights
      v1 = txt_pl->m_world2tex * (lp->vertices[mi] + light->GetCenter () - txt_pl->v_world2tex);
      f_uv[i].x = (v1.x * ww - Imin_u) * inv_lightcell_size;
      f_uv[i].y = (v1.y * hh - Imin_v) * inv_lightcell_size;
      if (f_uv[i].y < miny) miny = f_uv[MinIndex = i].y;
      if (f_uv[i].y > maxy) maxy = f_uv[MaxIndex = i].y;
    }
  }

  csColor color = light->GetColor () * NORMAL_LIGHT_LEVEL;

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
        scanR2 = (scanR2 + 1) % lp->num_vertices;

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
        scanL2 = (scanL2 - 1 + lp->num_vertices) % lp->num_vertices;

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
      // @@@ This should not be needed. But if this line isn't here
      // then I can get crashes with dynamic lights. This happens
      // easily if I use room.zip. We need to investigate exactly
      // why this can occur. In principle it shouldn't.
      if (sy < 0) goto finish;
    
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

        ru = u << lightcell_shift;
        rv = sy << lightcell_shift;

        v1.x = (float)(ru + Imin_u) * invww;
        v1.y = (float)(rv + Imin_v) * invhh;
        v1.z = 0;
        v2 = vv + m_t2w * v1;

	// Check if the point on the polygon is shadowed. To do this
	// we traverse all shadow frustums and see if it is contained in any of them.
	csShadowFrustum* shadow_frust;
	shadow_frust = lp->shadows.GetFirst ();
	bool shadow = false;
	while (shadow_frust)
	{
	  if (shadow_frust->relevant && shadow_frust->polygon != polygon)
	    if (shadow_frust->Contains (v2-shadow_frust->GetOrigin ()))
	    { shadow = true; break; }
	  shadow_frust = shadow_frust->next;
	}

	if (!shadow)
	{
	  //@@@ This is only right if we don't allow reflections for dynamic lights
	  d = csSquaredDist::PointPoint (light->GetCenter (), v2);

	  if (d >= light->GetSquaredRadius ()) continue;
	  d = qsqrt (d);

	  //@@@ This is only right if we don't allow reflections for dynamic lights
	  float cosinus = (v2-light->GetCenter ())*polygon->GetPolyPlane ()->Normal ();
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
	      l1 += mapR[uv];
	      if (l1 > 255) l1 = 255;
	      mapR[uv] = l1;
	    }
	  }
	  if (color.green > 0 && mapG)
	  {
	    l2 = QRound (color.green * brightness);
	    if (l2)
	    {
	      l2 += mapG[uv];
	      if (l2 > 255) l2 = 255;
	      mapG[uv] = l2;
	    }
	  }
	  if (color.blue > 0 && mapB)
	  {
	    l3 = QRound (color.blue * brightness);
	    if (l3)
	    {
	      l3 += mapB[uv];
	      if (l3 > 255) l3 = 255;
	      mapB[uv] = l3;
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

void csPolyTexture::GetTextureBox (float& fMinU, float& fMinV, float& fMaxU, float& fMaxV)
{
  fMinU = Fmin_u; fMaxU = Fmax_u;
  fMinV = Fmin_v; fMaxV = Fmax_v;
}

iPolygon3D *csPolyTexture::GetPolygon ()
{
  return QUERY_INTERFACE(polygon, iPolygon3D);
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

iLightMap *csPolyTexture::GetLightMap () { return lm; }
iMaterialHandle *csPolyTexture::GetMaterialHandle () { return mat_handle; }
int csPolyTexture::GetWidth () { return w; }
int csPolyTexture::GetHeight () { return h; }
float csPolyTexture::GetFDU () { return fdu; }
float csPolyTexture::GetFDV () { return fdv; }
int csPolyTexture::GetShiftU () { return shf_u; }
int csPolyTexture::GetOriginalWidth () { return w_orig; }
int csPolyTexture::GetIMinU () { return Imin_u; }
int csPolyTexture::GetIMinV () { return Imin_v; }
void *csPolyTexture::GetCacheData (int idx) { return cache_data [idx]; }
void csPolyTexture::SetCacheData (int idx, void *d) { cache_data [idx] = d; }
int csPolyTexture::GetLightCellSize () { return lightcell_size; }
int csPolyTexture::GetLightCellShift () { return lightcell_shift; }
