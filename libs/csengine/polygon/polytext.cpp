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
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"

// This is a private class used to track all postponed lighting references
// for a lightmap that is shared by several polygons. See the large comment
// before csPolyTexture::GetLightmapBounds for details.
// An object of this type is inserted into original polygon, so that no
// extra storage except that provided by csObject is required.
class csDelayedLightingInfo : public csFrustumViewCleanup
{
  struct LightViewInfo
  {
    // The view frustum
    csFrustumView *frustum;
    // The context.
    csFrustumContext* ctxt;
    // The old value of csFrustumViewCleanup "next" field
    csFrustumViewCleanup *old_next;
    // The list of shadow frustums
    csShadowBlock shadows;
    // The list of polygons that are still unlit; if NULL we have no more shares
    csPolygon3D *unlit_poly;

    LightViewInfo (csFrustumView *frust, csFrustumContext* ctxt,
    	csFrustumViewCleanup *next, csPolygon3D *orig_poly)
        : shadows (32, 32)
    {
      frustum = frust;
      LightViewInfo::ctxt = ctxt;
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
	{
	  prev_poly = poly;
          poly = poly->GetNextShare ();
        }
      }
    }

    int GetShadowCount ()
    {
      return shadows.GetShadowCount ();
    }

    iShadowIterator* GetShadowIterator ()
    {
      return shadows.GetShadowIterator ();
    }

    ~LightViewInfo ()
    {
      shadows.DeleteShadows ();
    }
  };

  // The list of lightviewinfo structures
  CS_DECLARE_TYPED_VECTOR (LightViewVector, LightViewInfo) lvlist;

  // The polygon texture we are filling
  csPolyTexture *polytex;

public:
  // Constructor
  csDelayedLightingInfo (csPolyTexture *ptex);

  // Return total number of shadows
  int GetShadowCount ()
  {
    int l = lvlist.Length ();
    if (!l) return 0;
    return lvlist.Get (l-1)->GetShadowCount ();
  }
  // Get iterator to iterate over all shadows.
  iShadowIterator* GetShadowIterator ()
  {
    int l = lvlist.Length ();
    if (!l) return NULL;
    return lvlist.Get (l-1)->GetShadowIterator ();
  }

  // Collect a reference from given frustum to our lightmap
  bool Collect (csFrustumView *lview, csFrustumContext* ctxt,
  	csPolygon3D *poly);

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

csDelayedLightingInfo::csDelayedLightingInfo (csPolyTexture *ptex) :
	lvlist (4, 4)
{
  polytex = ptex;
  action = csPolyTexture::ProcessDelayedLightmaps;
}

bool csDelayedLightingInfo::Collect (csFrustumView *lview,
	csFrustumContext* ctxt, csPolygon3D *poly)
{
  // If we have no lighting info, or this frustumview/context is different
  // from the last collected frustum, we have a new (recursive) light source,
  // that possibly came through a mirror or warped portal or such. Note that we
  // CAN use frustumview+ctxt address to check uniquity because a frustumview
  // cannot be deleted (and another one created on the same address) without
  // the frustum being removed from lvlist (this is done from the cleanup
  // callback, called from frustumview destructor).
  int l = lvlist.Length ();
  if (!l ||
    lvlist.Get (l-1)->frustum != lview ||
    lvlist.Get (l-1)->ctxt != ctxt)
  {
#ifdef CS_DEBUG
    // Check if this frustum has been seen EARLIER: if so, this means that
    // during the destruction of frustum views that are "on top" of it
    // ProcessDelayedLightMaps() has not been called.
    for (int i = l-2; i >= 0; i--)
      if (lvlist.Get (i)->frustum == lview && lvlist.Get (i)->ctxt == ctxt)
        DEBUG_BREAK;
#endif
    // Create a new light frustum info structure
    lvlist.Push (new LightViewInfo (lview, ctxt, next,
    	poly->GetBasePolygon ()));
    l++;
    // Register with that frustumview for cleanup
    ctxt->RegisterCleanup (this);
  }

  LightViewInfo *lvi = lvlist.Get (l-1);

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
    prev_poly = cur_poly;
    cur_poly = cur_poly->GetNextShare ();
  }

  // We should never cleanly quit of loop. If it is so, it means
  // we have a polygon that is not on share list (which means something
  // went wrong).
  CS_ASSERT (cur_poly);

  // Check if any shadow frustums we have now have not been seen in the past
  lvi->shadows.AddUniqueRelevantShadows (ctxt->GetShadows ());
  //@@@ DEBUG?lvi->shadows.AddUniqueRelevantShadows (lview->GetFrustumContext ()->GetShadows ());
  return !lvi->unlit_poly;
}

bool csDelayedLightingInfo::FinishLightView ()
{
  int idx = lvlist.Length () - 1;

  // We shouldn't get here for empty DelayedLightInfo's
  CS_ASSERT (idx >= 0);

  // Deregister ourselves from the cleanup list of frustumview
  lvlist.Get (idx)->ctxt->DeregisterCleanup (this);
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

SCF_IMPLEMENT_IBASE (csPolyTexture)
  SCF_IMPLEMENTS_INTERFACE (iPolygonTexture)
SCF_IMPLEMENT_IBASE_END

csPolyTexture::csPolyTexture ()
{
  SCF_CONSTRUCT_IBASE (NULL);
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
  if (lm) lm->DecRef ();
}

void csPolyTexture::SetLightMap (csLightMap* lightmap)
{
  if (lightmap) lightmap->IncRef ();
  if (lm) lm->DecRef ();
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

void csPolyTexture::ProcessDelayedLightmaps (iFrustumView *lview,
  csFrustumContext* ctxt,
  csFrustumViewCleanup *lighting_info)
{
  csDelayedLightingInfo *dli = (csDelayedLightingInfo *)lighting_info;

  // Apply lighting to the lightmap
  dli->GetPolygon ()->CalculateDelayedLighting ((csFrustumView*)lview,
  	ctxt);
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
 * postponed can be processed at least now.
 */
bool csPolyTexture::GetLightmapBounds (const csVector3& lightpos, bool mirror,
	csVector3 *bounds)
{
  csPolyTxtPlane *txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse ();
  csVector3 &v_t2w = txt_pl->v_world2tex;

  int lmw = lm->rwidth;
  int lmh = lm->rheight;

  int ww, hh;
  if (mat_handle && mat_handle->GetTexture ())
    mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
  else
    ww = hh = 64;
  float inv_ww = 1.0 / ww;
  float inv_hh = 1.0 / hh;

  // Calculate the responsability grid bounds as a polygon in world space.
  for (int i = 0; i < 4; i++)
  {
    csVector3 v;
    v.x = (i == 0 || i == 3) ? -0.5 : lmw - 0.5;
    v.y = (i < 2) ? -0.5 : lmh - 0.5;
    v.z = 0.0;

    v.x = ((v.x * csLightMap::lightcell_size) + Imin_u) * inv_ww;
    v.y = ((v.y * csLightMap::lightcell_size) + Imin_v) * inv_hh;

    v = (m_t2w * v + v_t2w) - lightpos;

    bounds [mirror ? 3 - i : i] = v;
  } /* endfor */

  // If the lightmap is shared, take care to fill lightmap only
  // after all shadow frustums are collected.
  return !(lm->delayed_light_info || polygon->GetNextShare ());
}

bool csPolyTexture::CollectShadows (csFrustumView *lview,
	csFrustumContext* ctxt, csPolygon3D *poly)
{
  if (!lm->delayed_light_info)
    lm->delayed_light_info = new csDelayedLightingInfo (this);

  return (lm->delayed_light_info->Collect (lview, ctxt, poly));
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
    printf ("Array bound error in file %s, line %d\n",
      __FILE__, __LINE__); fflush (stdout);
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
    printf ("Array bound error in file %s, line %d\n",
      __FILE__, __LINE__); fflush (stdout);
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
    printf ("Array bound error in file %s, line %d\n",
      __FILE__, __LINE__); fflush (stdout);
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
    printf ("Array bound error in file %s, line %d\n",
      __FILE__, __LINE__); fflush (stdout);
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
  if (mat_handle && mat_handle->GetTexture ())
    mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
  else
    ww = hh = 64;

  csPolyTxtPlane *txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();

  csFrustum* light_frustum = lview.GetFrustumContext ()->GetLightFrustum ();
  csVector3 &lightpos = light_frustum->GetOrigin ();
  csDelayedLightingInfo *dli = lm->delayed_light_info;

  // Now allocate the space for the projected lighted polygon
  int nvlf = light_frustum->GetVertexCount ();
  csVector3 *lf3d = light_frustum->GetVertices ();
  ALLOC_STACK_ARRAY (lf2d, csVector2, nvlf);
  // Project the light polygon from world space to responsability grid space
  float inv_lightcell_size = 1.0 / csLightMap::lightcell_size;
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
  iShadowIterator* shadow_it;
  csFrustum *csf;
  if (dli)
  {
    nsf = dli->GetShadowCount () + dli->GetUnlitPolyCount ();
    shadow_it = dli->GetShadowIterator ();
  }
  else
  {
    nsf = 0;
    shadow_it = lview.GetFrustumContext ()->GetShadows ()->GetShadowIterator ();
    while (shadow_it->HasNext ())
    {
      shadow_it->Next ();
      if (shadow_it->IsRelevant ()) nsf++;
    }
    shadow_it->Reset ();
  }

  ALLOC_STACK_ARRAY (sfc, csPolygonClipper *, nsf);
  for (i = 0; i < nsf; i++)
  {
    csf = shadow_it->Next ();
    if (!dli)
      while (!shadow_it->IsRelevant ())
        csf = shadow_it->Next ();

    // MAX_OUTPUT_VERTICES should be far too enough
    csVector2 s2d [MAX_OUTPUT_VERTICES];
    int nv;
    bool mirror;

    if (!dli || i < dli->GetShadowCount ())
    {
      csFrustum *shadow = csf->Intersect (*lview.GetFrustumContext ()->
      		GetLightFrustum ());
      if (!shadow) { sfc [i] = NULL; continue; }

      // Translate the shadow frustum to polygon plane
      nv = shadow->GetVertexCount ();
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
      nv = poly->GetVertexCount ();
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
  shadow_it->DecRef ();

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

  csLightingInfo& linfo = lview.GetFrustumContext ()->GetLightingInfo ();

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

  unsigned char *ShadowMap = 0;
  csRGBpixel *LightMap = 0;
  csShadowMap *smap = NULL;

  int ww, hh;
  if (mat_handle && mat_handle->GetTexture ())
    mat_handle->GetTexture ()->GetMipMapDimensions (0, ww, hh);
  else
    ww = hh = 64;
  float inv_ww = 1.0 / ww;
  float inv_hh = 1.0 / hh;

  csStatLight *light = (csStatLight *)lview.GetUserData ();
  bool dyn = light->IsDynamic ();
  csVector3& lightpos = lview.GetFrustumContext ()->GetLightFrustum ()->
  	GetOrigin ();

  if (dyn)
  {
    smap = lm->FindShadowMap (light);
    if (!smap)
      smap = lm->NewShadowMap (light, w, h);
    ShadowMap = smap->GetArray ();
  }
  else
  {
    LightMap = lm->GetStaticMap ().GetArray ();
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
  csColor& col = linfo.GetColor ();
  float light_r = col.red * NORMAL_LIGHT_LEVEL;
  float light_g = col.green * NORMAL_LIGHT_LEVEL;
  float light_b = col.blue * NORMAL_LIGHT_LEVEL;
  for (int i = 0; i < lmh; i++)
  {
    int uv = i * lm->GetWidth ();
    for (int j = 0; j < lmw; j++, uv++)
    {
      float lightness = lc.coverage [covaddr++];
      if (lightness < EPSILON)
        continue;

      int ru = j << csLightMap::lightcell_shift;
      int rv = i << csLightMap::lightcell_shift;

      csVector3 v (float (ru + Imin_u) * inv_ww, float (rv + Imin_v) * inv_hh,
      	0);
      v = v_t2w + m_t2w * v;

      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= light->GetSquaredRadius ()) continue;

      d = qsqrt (d);

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
        l = ShadowMap [uv] + QRound (NORMAL_LIGHT_LEVEL * lightness * brightness);
        ShadowMap [uv] = l < 255 ? l : 255;
      }
      else
      {
        l = LightMap [uv].red + QRound (light_r * lightness * brightness);
        LightMap [uv].red = l < 255 ? l : 255;
        l = LightMap [uv].green + QRound (light_g * lightness * brightness);
        LightMap [uv].green = l < 255 ? l : 255;
        l = LightMap [uv].blue + QRound (light_b * lightness * brightness);
        LightMap [uv].blue = l < 255 ? l : 255;
      }
    }
  }
}

static void Enlarge (csVector2* v, int nv)
{
  csBox2 box;
  int i;
  box.StartBoundingBox ();
  for (i = 0 ; i < nv ; i++)
    box.AddBoundingVertex (v[i]);
  csVector2 center = box.GetCenter ();
  for (i = 0 ; i < nv ; i++)
    v[i] = (v[i]-center) * 1.1 + center;
}

void csPolyTexture::FillLightMapNew (csFrustumView* lview, bool vis,
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

if (!vis) return;

  if (!shadow_bitmap)
  {
    int lm_w = lm->rwidth;
    int lm_h = lm->rheight;
    shadow_bitmap = new csShadowBitmap (lm_w, lm_h, 4);
    iFrustumViewUserdata* ud = lview->GetUserdata ();
    csLightingPolyTexQueue* lptq = (csLightingPolyTexQueue*)ud;
    lptq->AddPolyTexture (this);
  }

  // Room for our shadow in lightmap space.
  csVector2 s2d[MAX_OUTPUT_VERTICES];

#if 1
  //if (!vis)
  {
    // Here we know that the entire polygon is not visible. So we don't
    // actually traverse all shadow frustums here but just render the
    // entire polygon as a shadow.
    for (j = 0 ; j < subpoly->GetVertexCount () ; j++)
    {
      // @@@ TODO: Optimize. Combine the calculations below.
      csVector3 v = txt_pl->m_world2tex *
	  	(subpoly->Vwor (j) - txt_pl->v_world2tex);
      s2d[j].x = (v.x * ww - Imin_u) * inv_lightcell_size;
      s2d[j].y = (v.y * hh - Imin_v) * inv_lightcell_size;
    }
    Enlarge (s2d, subpoly->GetVertexCount ());
    shadow_bitmap->RenderPolygon (s2d, subpoly->GetVertexCount (), 0);
  }
  //else
#endif
  {
    // Our polygon in light space (i.e. frustum).
    // Here we will actually go back to the original polygon so that
    // our shadows will cover the entire polygon and not the sub-part (split
    // polygon).
    csVector3 poly[MAX_OUTPUT_VERTICES];
    csPolygon3D* base_poly = polygon->GetBasePolygon ();
    int num_vertices = base_poly->GetVertexCount ();
    if (lview->GetFrustumContext ()->IsMirrored ())
      for (j = 0 ; j < num_vertices ; j++)
        poly[j] = base_poly->Vwor (num_vertices - j - 1) - lightpos;
    else
      for (j = 0 ; j < num_vertices ; j++)
        poly[j] = base_poly->Vwor (j) - lightpos;

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
      if (((csPolygon3D*)(shadow_it->GetUserData ()))->GetBasePolygon ()
    	!= base_poly)
      {
        // @@@ TODO: Optimize. Custom version of Intersect that is more
        // optimial (doesn't require to copy 'poly') and detects cases
        // that should not shadow.
        csFrustum* new_shadow = shadow_frust->Intersect (poly, num_vertices);
        if (new_shadow)
        {
          int nv = new_shadow->GetVertexCount ();
          if (nv > MAX_OUTPUT_VERTICES) nv = MAX_OUTPUT_VERTICES;
          csVector3* s3d = new_shadow->GetVertices ();
          for (j = 0; j < nv; j++)
          {
	    // @@@ TODO: Optimize. Combine the calculations below.
            csVector3 v = txt_pl->m_world2tex *
	  	(s3d[j] + lightpos - txt_pl->v_world2tex);
            s2d[j].x = (v.x * ww - Imin_u) * inv_lightcell_size;
            s2d[j].y = (v.y * hh - Imin_v) * inv_lightcell_size;
          }
	  new_shadow->DecRef ();
	  //Enlarge (s2d, nv);
          shadow_bitmap->RenderPolygon (s2d, nv, 1);
        }
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

csShadowBitmap::csShadowBitmap (int lm_w, int lm_h, int quality)
{
  shadow = NULL;
  light = NULL;
  full_shadow = false;
  csShadowBitmap::lm_w = lm_w;
  csShadowBitmap::lm_h = lm_h;
  csShadowBitmap::quality = quality;
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
}

void csShadowBitmap::LightPutPixel (int x, int y, float area, void *arg)
{
  csShadowBitmap* sb = (csShadowBitmap*)arg;
  if (x >= sb->sb_w || y >= sb->sb_h || x < 0 || y < 0)
  {
#ifdef CS_DEBUG
    //printf ("Array bound error.\n"); fflush (stdout);
#endif
    return;
  }
  if (area < EPSILON) return;
  sb->light[sb->sb_w * y + x] = 1;
}

void csShadowBitmap::LightDrawBox (int x, int y, int w, int h, void *arg)
{
  csShadowBitmap* sb = (csShadowBitmap*)arg;
  if (x >= sb->sb_w || y >= sb->sb_h || x < 0 || y < 0 ||
      x + w > sb->sb_w || y + h > sb->sb_h || w < 0 || h < 0)
  {
#ifdef CS_DEBUG
    //printf ("Array bound error.\n"); fflush (stdout);
#endif
    return;
  }
  int ofs = sb->sb_w * y + x;
  int delta = sb->sb_w - w;
  for (int yy = h; yy > 0; yy--)
  {
    for (int xx = w; xx > 0; xx--)
      sb->light[ofs++] = 1;
    ofs += delta;
  }
}

void csShadowBitmap::ShadowPutPixel (int x, int y, float area, void *arg)
{
  csShadowBitmap* sb = (csShadowBitmap*)arg;
  if (x >= sb->sb_w || y >= sb->sb_h || x < 0 || y < 0)
  {
#ifdef CS_DEBUG
    //printf ("Array bound error.\n"); fflush (stdout);
#endif
    return;
  }
  if (area < EPSILON) return;
  sb->shadow[sb->sb_w * y + x] = 1;
}

void csShadowBitmap::ShadowDrawBox (int x, int y, int w, int h, void *arg)
{
  csShadowBitmap* sb = (csShadowBitmap*)arg;
  if (x >= sb->sb_w || y >= sb->sb_h || x < 0 || y < 0 ||
      x + w > sb->sb_w || y + h > sb->sb_h || w < 0 || h < 0)
  {
#ifdef CS_DEBUG
    //printf ("Array bound error.\n"); fflush (stdout);
#endif
    return;
  }
  int ofs = sb->sb_w * y + x;
  int delta = sb->sb_w - w;
  for (int yy = h; yy > 0; yy--)
  {
    for (int xx = w; xx > 0; xx--)
      sb->shadow[ofs++] = 1;
    ofs += delta;
  }
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
    memset (light, 0, sb_w * sb_h);
  }

  if (val == 1)
    csAntialiasedPolyFill (shadow_poly, num_vertices, this,
  	ShadowPutPixel, ShadowDrawBox);
  else
    csAntialiasedPolyFill (shadow_poly, num_vertices, this,
  	LightPutPixel, LightDrawBox);
  return;

#if 0
  //-------------------
  // First calculate the minimum and maximum indices (for 'y').
  //-------------------
  csVector2* sp = shadow_poly;		// For conveniance.
  int MaxIndex = -1, MinIndex = -1;
  float miny = 100000000, maxy = -100000000;
  float minx = 100000000, maxx = -100000000;
  for (i = 0 ; i < num_vertices ; i++)
  {
    if (sp[i].y < miny) miny = sp[MinIndex = i].y;
    if (sp[i].y > maxy) maxy = sp[MaxIndex = i].y;
    if (sp[i].x < minx) minx = sp[i].x;
    if (sp[i].x > maxx) maxx = sp[i].x;
  }

  // The shadow does not touch the shadow-bitmap.
  if (maxy < 0 || miny > sb_h-1) return;
  if (maxx < 0 || minx > sb_w-1) return;

  //-------------------
  // If we don't already have a bitmap then we allocate it here.
  // @@@ NOTE: We currently allocate one byte for every shadow-point.
  // That's not optimal. We should use 1/8 byte for every shadow-point.
  //-------------------
  if (!bitmap)
  {
    bitmap = new char [sb_w * sb_h];
    memset (bitmap, 1, sb_w * sb_h);
  }

  //-------------------
  // Now we will render the shadow on the shadow-bitmap.
  //-------------------
  int scanL1, scanL2, scanR1, scanR2;   // Scan vertex left/right start/final.
  float sxL, sxR, dxL, dxR;             // Scanline X left/right and deltas.
  int sy, fyL, fyR;                     // Scanline Y, final Y left and right.
  int xL, xR;

  sxL = sxR = dxL = dxR = 0;
  scanL2 = scanR2 = MaxIndex;
  sy = fyL = fyR = (QRound (ceil (sp[scanL2].y)) > (sb_h-1))
  	? sb_h-1 : QRound (ceil (sp[scanL2].y));

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
a:      if (scanR2 == MinIndex) return;
        scanR1 = scanR2;
        scanR2 = (scanR2 + 1) % num_vertices;

        if (ABS (sp[scanR2].y - sp[MaxIndex].y) < EPSILON)
        {
          // oops! we have a flat bottom!
          goto a;
        }
        fyR = QRound (floor (sp[scanR2].y));
        float dyR = (sp[scanR1].y - sp[scanR2].y);
	sxR = sp[scanR1].x;
        if (dyR != 0)
        {
          dxR = (sp[scanR2].x - sxR) / dyR;
	  // horizontal pixel correction
          sxR += dxR * (sp[scanR1].y - ((float)sy));
        }
	else dxR = 0;
        leave = false;
      }
      if (sy <= fyL)
      {
b:      if (scanL2 == MinIndex) return;
        scanL1 = scanL2;
        scanL2 = (scanL2 - 1 + num_vertices) % num_vertices;

        if (ABS (sp[scanL2].y - sp[MaxIndex].y) < EPSILON)
        {
          // oops! we have a flat bottom!
          goto b;
        }

        fyL = QRound (floor (sp[scanL2].y));
        float dyL = (sp[scanL1].y - sp[scanL2].y);
	sxL = sp[scanL1].x;
        if (dyL != 0)
        {
          dxL = (sp[scanL2].x - sxL) / dyL;
          // horizontal pixel correction
          sxL += dxL * (sp[scanL1].y - ((float)sy));
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
      if (sy < 0) return;
    
      // Compute the rounded screen coordinates of horizontal strip
      float l = sxL, r = sxR;

      if (r > l) { float s = r; r = l; l = s; }

      xL = 1 + QRound (ceil (l));
      xR = QRound (floor (r));

      if (xR < 0) xR = 0;
      if (xL > sb_w) xL = sb_w;

      if (xL > xR)
      {
        char* uv = &bitmap[sy * sb_w + xR];
        char* uv_end = uv + (xL-xR);
        while (uv < uv_end) *uv++ = val;
      }

      if (!sy) return;
      sxL += dxL;
      sxR += dxR;
      sy--;
    }
  }
#endif
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
    int total = (maxu-minu+1) * (maxv-minv+1);
    int s = 0;
    for (v = minv ; v <= maxv ; v++)
    {
      int idx = v * sb_w + minu;
      char* bml = &light[idx];
      char* bms = &shadow[idx];
      for (u = minu ; u <= maxu ; u++)
      {
        int l = *bml && !*bms;
	bml++; bms++;
        s += l;
      }
    }
    return float (s) / float (total);
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
  float light_r = lightcolor.red * NORMAL_LIGHT_LEVEL;
  float light_g = lightcolor.green * NORMAL_LIGHT_LEVEL;
  float light_b = lightcolor.blue * NORMAL_LIGHT_LEVEL;
  for (int i = 0 ; i < lm_h ; i++)
  {
    int uv = i * lm_w;
    for (int j = 0 ; j < lm_w ; j++, uv++)
    {
//@@@
//if (i == j) lightmap[uv].red = 255;
//if (i == lm_w-1-j) lightmap[uv].blue = 255;
//@@@
      float lightness = GetLighting (j, i);
      if (lightness < EPSILON)
        continue;

      // @@@ Optimization: It should be possible to combine these
      // calculations into a more efficient formula.
      int ru = j << lightcell_shift;
      int rv = i << lightcell_shift;
      csVector3 v (float (ru + shf_u) * mul_u, float (rv + shf_v) * mul_v, 0);
      v = v_t2w + m_t2w * v;

      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= light->GetSquaredRadius ()) continue;

      d = qsqrt (d);

      float cosinus = (v - lightpos) * poly_normal;
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0)
        cosinus = 0;
      else if (cosinus > 1)
        cosinus = 1;

      float brightness = cosinus * light->GetBrightnessAtDistance (d);

      int l;
      l = lightmap[uv].red + QRound (light_r * lightness * brightness);
      lightmap[uv].red = l < 255 ? l : 255;
      l = lightmap[uv].green + QRound (light_g * lightness * brightness);
      lightmap[uv].green = l < 255 ? l : 255;
      l = lightmap[uv].blue + QRound (light_b * lightness * brightness);
      lightmap[uv].blue = l < 255 ? l : 255;
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
  for (int i = 0 ; i < lm_h ; i++)
  {
    int uv = i * lm_w;
    for (int j = 0 ; j < lm_w ; j++, uv++)
    {
      float lightness = GetLighting (j, i);
      if (lightness < EPSILON)
        continue;

      // @@@ Optimization: It should be possible to combine these
      // calculations into a more efficient formula.
      int ru = j << lightcell_shift;
      int rv = i << lightcell_shift;
      csVector3 v (float (ru + shf_u) * mul_u, float (rv + shf_v) * mul_v, 0);
      v = v_t2w + m_t2w * v;

      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= light->GetSquaredRadius ()) continue;

      d = qsqrt (d);

      float cosinus = (v - lightpos) * poly_normal;
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0)
        cosinus = 0;
      else if (cosinus > 1)
        cosinus = 1;

      float brightness = cosinus * light->GetBrightnessAtDistance (d);

      int l = shadowmap[uv] +
      	QRound (NORMAL_LIGHT_LEVEL * lightness * brightness);
      shadowmap[uv] = l < 255 ? l : 255;
    }
  }
}

//------------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csLightingPolyTexQueue)
  SCF_IMPLEMENTS_INTERFACE (iFrustumViewUserdata)
SCF_IMPLEMENT_IBASE_END

csLightingPolyTexQueue::csLightingPolyTexQueue ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

csLightingPolyTexQueue::~csLightingPolyTexQueue ()
{
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

