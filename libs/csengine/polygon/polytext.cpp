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
#include "sysdef.h"
#include "qint.h"
#include "csgeom/fastsqrt.h"
#include "csgeom/polyclip.h"
#include "csgeom/polyaa.h"
#include "csengine/polytext.h"
#include "csengine/polyplan.h"
#include "csengine/polytmap.h"
#include "csengine/polygon.h"
#include "csengine/light.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/quadcube.h"
#include "csengine/lghtmap.h"
#include "igraph3d.h"
#include "itexture.h"

// Option variable: do accurate lighting of things (much slower)?
bool csPolyTexture::do_accurate_things = true;
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
  dyn_dirty = true;
  lm = NULL;
  cache_data [0] = cache_data [1] = cache_data [2] = cache_data [3] = NULL;
}

csPolyTexture::~csPolyTexture ()
{
  csWorld::current_world->G3D->RemoveFromCache (this);
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
  txt_handle->GetMipMapDimensions (0, ww, hh);
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

void csPolyTexture::MakeDirtyDynamicLights ()
{
  dyn_dirty = true;
}

bool csPolyTexture::RecalculateDynamicLights ()
{
  if (!dyn_dirty) return false;
  if (!lm) return false;

  dyn_dirty = false;

  //---
  // First copy the static lightmap to the real lightmap.
  // Remember the real lightmap first so that we can see if
  // there were any changes.
  //---
  long lm_size = lm->GetSize ();
  csRGBLightMap& stmap = lm->GetStaticMap ();
  csRGBLightMap& remap = lm->GetRealMap ();

  memcpy (remap.GetMap (), stmap.GetMap (), 3 * lm_size);

  //---
  // Then add all pseudo-dynamic lights.
  //---
  csLight* light;
  unsigned char* mapR, * mapG, * mapB;
  float red, green, blue;
  unsigned char* p, * last_p;
  int l, s;

  if (lm->first_smap)
  {
    csShadowMap* smap = lm->first_smap;

    // Color mode.
    do
    {
      mapR = remap.GetRed ();
      mapG = remap.GetGreen ();
      mapB = remap.GetBlue ();
      light = smap->light;
      red = light->GetColor ().red;
      green = light->GetColor ().green;
      blue = light->GetColor ().blue;
      csLight::CorrectForNocolor (&red, &green, &blue);
      p = smap->map;
      last_p = p+lm_size;
      do
      {
        s = *p++;
        l = *mapR + QRound (red * s);
        *mapR++ = l < 255 ? l : 255;
        l = *mapG + QRound (green * s);
        *mapG++ = l < 255 ? l : 255;
        l = *mapB + QRound (blue * s);
        *mapB++ = l < 255 ? l : 255;
      }
      while (p < last_p);

      smap = smap->next;
    }
    while (smap);
  }

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

bool csPolyTexture::GetLightmapBounds (csFrustumView *lview, csVector3 *bounds)
{
  // Take care not to fill same lightmap twice
  if (lm->last_frustum_id == lview->frustum_id)
    return false;
  lm->last_frustum_id = lview->frustum_id;

  csPolyTxtPlane *txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse ();
  csVector3 &v_t2w = txt_pl->v_world2tex;

  int lmw = lm->rwidth;
  int lmh = lm->rheight;
  csVector3 &lightpos = lview->light_frustum->GetOrigin ();

  int ww, hh;
  txt_handle->GetMipMapDimensions (0, ww, hh);
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

  return true;
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

// This is a private structure used while we build the light coverage data
struct __light_coverage
{
  // The coverage array. Each float corresponds to a lightmap grid cell
  // and contains the area of light cell that is covered by light.
  float *coverage;
  // The width and height of the coverage array
  int width, height;

  __light_coverage (int w, int h)
  { coverage = (float *)calloc ((width = w) * (height = h), sizeof (float)); }
  ~__light_coverage ()
  { free (coverage); }
};

static void __add_PutPixel (int x, int y, float area, void *arg)
{
  __light_coverage *lc = (__light_coverage *)arg;
  lc->coverage [lc->width * y + x] += area;
}

static void __add_DrawBox (int x, int y, int w, int h, void *arg)
{
  __light_coverage *lc = (__light_coverage *)arg;
  int ofs = lc->width * y + x;
  int delta = lc->width - w;
  for (int yy = h; yy > 0; yy--)
  {
    for (int xx = w; xx > 0; xx--)
      lc->coverage [ofs++] += 1.0;
    ofs += delta;
  } /* endfor */
}

static void __sub_PutPixel (int x, int y, float area, void *arg)
{
  __light_coverage *lc = (__light_coverage *)arg;
  lc->coverage [lc->width * y + x] -= area;
}

static void __sub_DrawBox (int x, int y, int w, int h, void *arg)
{
  __light_coverage *lc = (__light_coverage *)arg;
  int ofs = lc->width * y + x;
  int delta = lc->width - w;
  for (int yy = h; yy > 0; yy--)
  {
    for (int xx = w; xx > 0; xx--)
      lc->coverage [ofs++] -= 1.0;
    ofs += delta;
  } /* endfor */
}

void csPolyTexture::FillLightMap (csFrustumView& lview)
{
  if (!lm) return;

  csStatLight *light = (csStatLight *)lview.userdata;

  int ww, hh;
  txt_handle->GetMipMapDimensions (0, ww, hh);

  csPolyTxtPlane *txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csPolyPlane *pl = polygon->GetPlane ();
  float cosfact = polygon->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse ();
  csVector3 &v_t2w = txt_pl->v_world2tex;

  // From: Ax+By+Cz+D = 0
  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T + Vwt = W
  // Get 'w' from 'u' and 'v' (using u,v,w plane).
  float A = pl->GetWorldPlane ().A ();
  float B = pl->GetWorldPlane ().B ();
  float C = pl->GetWorldPlane ().C ();
  float D = pl->GetWorldPlane ().D ();
  float txt_A = A * m_t2w.m11 + B * m_t2w.m21 + C * m_t2w.m31;
  float txt_B = A * m_t2w.m12 + B * m_t2w.m22 + C * m_t2w.m32;
  float txt_C = A * m_t2w.m13 + B * m_t2w.m23 + C * m_t2w.m33;
  float txt_D = A * txt_pl->v_world2tex.x +
                B * txt_pl->v_world2tex.y +
                C * txt_pl->v_world2tex.z + D;

  float inv_ww = 1.0 / ww;
  float inv_hh = 1.0 / hh;

  unsigned char *mapR;
  unsigned char *mapG;
  unsigned char *mapB;
  csShadowMap *smap = NULL;

  bool hit = false;         // Set to true if there is a hit
  bool first_time = false;  // Set to true if this is the first pass for the dynamic light
  bool dyn = light->IsDynamic ();

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

  // We will compute the lighting of the entire lightmap, disregarding
  // polygon bounds. This removes both the "black borders" and "white
  // borders" problems. However, we should take care not to fill same
  // lightmap twice, otherwise we'll get very bright lighting for
  // shared lightmaps.
  int lmw = lm->rwidth;
  int lmh = lm->rheight;
  csVector3 &lightpos = lview.light_frustum->GetOrigin ();

  // Now allocate the space for the projected lighted polygon
  int nvlf = lview.light_frustum->GetNumVertices ();
  csVector3 *lf3d = lview.light_frustum->GetVertices ();
  csVector2 *lf2d = (csVector2 *)alloca (nvlf * sizeof (csVector2));
  // Project the light polygon from world space to responsability grid space
  float inv_lightcell_size = 1.0 / lightcell_size;
  int i, j, k;
  for (i = 0; i < nvlf; i++)
  {
    // T = Mwt * (W - Vwt)
    csVector3 v = txt_pl->m_world2tex *
      (lf3d [i] + lightpos - txt_pl->v_world2tex);
    lf2d [i].x = (v.x * ww - Imin_u) * inv_lightcell_size + 0.5;
    lf2d [i].y = (v.y * hh - Imin_v) * inv_lightcell_size + 0.5;
  }

  // Create the light coverage array
  __light_coverage lc (lmw, lmh);

  // Now fill the lightmap polygon with light coverage values
  csAntialiasedPolyFill (lf2d, nvlf, &lc, __add_PutPixel, __add_DrawBox);

  // Now subtract all shadow polygons from the coverage matrix.
  // At the same time, add the overlapping shadows to the coverage matrix.
  int nsf = 0;
  csShadowFrustum *csf = lview.shadows.GetFirst ();
  while (csf) { nsf++; csf = csf->next; }
  csPolygonClipper **sfc = (csPolygonClipper **)alloca (nsf * sizeof (csPolygonClipper *));
  for (i = 0, csf = lview.shadows.GetFirst (); i < nsf; i++, csf = csf->next)
  {
    sfc [i] = NULL;

    if (!csf->relevant) continue;

    csFrustum *shadow = csf->Intersect (*lview.light_frustum);
    if (!shadow) continue;

    // Translate the shadow frustum to polygon plane
    int nv = shadow->GetNumVertices ();
    csVector3 *s3d = shadow->GetVertices ();
    // MAX_OUTPUT_VERTICES should be far too enough
    csVector2 s2d [MAX_OUTPUT_VERTICES];
    if (nv > MAX_OUTPUT_VERTICES) nv = MAX_OUTPUT_VERTICES;
    for (j = 0; j < nv; j++)
    {
      csVector3 v = txt_pl->m_world2tex *
        (s3d [j] + lightpos - txt_pl->v_world2tex);
      s2d [j].x = (v.x * ww - Imin_u) * inv_lightcell_size + 0.5;
      s2d [j].y = (v.y * hh - Imin_v) * inv_lightcell_size + 0.5;
    }

    // Now subtract the shadow from the coverage matrix
    csAntialiasedPolyFill (s2d, nv, &lc, __sub_PutPixel, __sub_DrawBox);

    // Create a polygon clipper from this shadow polygon
    sfc [i] = new csPolygonClipper (s2d, nv, shadow->IsMirrored (), true);

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
      csAntialiasedPolyFill (sfi, sfic, &lc, __add_PutPixel, __add_DrawBox);
    } /* endfor */
  } /* endfor */

  // Free all shadow frustums
  for (i = 0; i < nsf; i++)
    delete sfc [i];

  // Finally, use the light coverage values to compute the actual lighting
  int covaddr = 0;
  float light_r = lview.r * NORMAL_LIGHT_LEVEL;
  float light_g = lview.g * NORMAL_LIGHT_LEVEL;
  float light_b = lview.b * NORMAL_LIGHT_LEVEL;
  for (i = 0; i < lmh; i++)
  {
    int uv = i * lm->GetWidth ();
    for (j = 0; j < lmw; j++, uv++)
    {
      float lightness = lc.coverage [covaddr++];
      if (lightness < EPSILON)
        continue;

      int ru = j << lightcell_shift;
      int rv = i << lightcell_shift;

      csVector3 v (float (ru + Imin_u) * inv_ww, float (rv + Imin_v) * inv_hh, 0);
      if (ABS (txt_C) > SMALL_EPSILON)
        v.z = - (txt_D + txt_A * v.x + txt_B * v.y) / txt_C;
      v = v_t2w + m_t2w * v;

      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= light->GetSquaredRadius ()) continue;

      d = FastSqrt (d);
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
  txt_handle->GetMipMapDimensions (0, ww, hh);

  csPolyTxtPlane* txt_pl = polygon->GetLightMapInfo ()->GetTxtPlane ();
  csPolyPlane* pl = polygon->GetPlane ();
  float cosfact = polygon->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse();
  csVector3 vv = txt_pl->v_world2tex;

  // From: Ax+By+Cz+D = 0
  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T + Vwt = W
  // Get 'w' from 'u' and 'v' (using u,v,w plane).
  float A = pl->GetWorldPlane ().A ();
  float B = pl->GetWorldPlane ().B ();
  float C = pl->GetWorldPlane ().C ();
  float D = pl->GetWorldPlane ().D ();
  float txt_A = A*m_t2w.m11 + B*m_t2w.m21 + C*m_t2w.m31;
  float txt_B = A*m_t2w.m12 + B*m_t2w.m22 + C*m_t2w.m32;
  float txt_C = A*m_t2w.m13 + B*m_t2w.m23 + C*m_t2w.m33;
  float txt_D = A*txt_pl->v_world2tex.x + B*txt_pl->v_world2tex.y + C*txt_pl->v_world2tex.z + D;

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
    CHK (f_uv = new csVector2 [lp->num_vertices]);
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

        if (fabs (f_uv [scanR2].y - f_uv [MaxIndex].y) < EPSILON)
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

        if (fabs (f_uv [scanL2].y - f_uv [MaxIndex].y) < EPSILON)
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
        if (ABS (txt_C) < SMALL_EPSILON)
          v1.z = 0;
        else
          v1.z = - (txt_D + txt_A*v1.x + txt_B*v1.y) / txt_C;
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
	  d = FastSqrt (d);

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

  CHK (delete [] f_uv);
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

iLightMap *csPolyTexture::GetLightMap () { return lm; }
iTextureHandle *csPolyTexture::GetTextureHandle () { return txt_handle; }
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
bool csPolyTexture::DynamicLightsDirty () { return dyn_dirty && lm; }
int csPolyTexture::GetLightCellSize () { return lightcell_size; }
int csPolyTexture::GetLightCellShift () { return lightcell_shift; }
