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

#include <math.h>

#define SYSDEF_ALLOCA
#include "sysdef.h"
#include "qint.h"
#include "csgeom/fastsqrt.h"
#include "csutil/bitset.h"
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

#define lightcell_size	csPolygon3D::lightcell_size
#define lightcell_shift	csPolygon3D::lightcell_shift

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
    v1 = polygon->Vwor (i);   // Coordinates of vertex in world space.
    v1 -= txt_pl->v_world2tex;
    v2 = (txt_pl->m_world2tex) * v1;  // Coordinates of vertex in texture space.
    if (v2.x < min_u) min_u = v2.x;
    if (v2.x > max_u) max_u = v2.x;
    if (v2.y < min_v) min_v = v2.y;
    if (v2.y > max_v) max_v = v2.y;
  }

  // DAN: used in hardware accel drivers
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

/*
 * Added by Denis Dmitriev for correct lightmaps shining. This code above draws
 * perfectly (like perfect texture mapping -- I mean most correctly)
 * anti-aliased polygon on lightmap and adjusts it according to the actual
 * polygon shape on the texture
 */
#define EPS   0.0001

float calc_area (int n, csVector2 *p)
{
  float area = 0;

  for (int i = 0; i < n; i++)
  {
    int j = (i != n - 1) ? i + 1 : 0;
    area += (p [i].y + p [j].y) * (p [i].x - p [j].x);
  }

  return fabs (area / 2.0);
}

static int __texture_width;
static float *__texture;
static unsigned char *__mark;

struct __rect
{
  int left, right;
  int top, bottom;
};

static void (*__draw_func)(int, int, float);

FILE *fo;

static void lixel_intensity (int x, int y, float density)
{
  int addr=x+y*__texture_width;

  if (density>=1.0)
    density=1.0;

  __texture[addr] = density;
}

static void correct_results (int x, int y, float density)
{
  if (density < EPS || density >= 1 - EPS)
    return;

  int addr = x + y * __texture_width;
  float res = __texture [addr] / density;

  if (res > 1)
    res = 1;

  __texture [addr] = res;
  __mark [addr] = 1;
}

/* I was interested in these values */
static int max_depth = 0, depth = 0;

static void poly_fill (int n, csVector2 *p2d, __rect &visible)
{
  depth++;

  if (depth > max_depth)
    max_depth = depth;

  // Calculate the complete are of visible rectangle
  int height = visible.bottom - visible.top;
  int width = visible.right - visible.left;
  int visarea = width * height;

  // Sanity check
  if (visarea <= 0)
  {
    depth--;
    return;
  }

  // Calculate the complete area of the polygon
  float a = calc_area (n, p2d);

  // Check if polygon is hollow
  if (a < EPS)
  {
    // this area is hollow
    depth--;
    return;
  }

  // Check if polygon surface equals the visible rectangle surface
  if (fabs (a - visarea) < EPS)
  {
    // this area is completely covered

    int x = visible.left, y = visible.top;
    for (int i = 0 ; i < height; i++)
      for (int j = 0 ; j < width; j++)
        __draw_func (j + x, i + y, 1);

    depth--;
    return;
  }

  if (height == 1 && width == 1)
  {
    __draw_func (visible.left, visible.top, a);

    depth--;
    return;
  }

  int sub_x = visible.left + width / 2;
  int sub_y = visible.top + height / 2;

  // 0 -- horizontal
  // 1 -- vertical
  int how_to_divide = (height > width) ? 0 : 1;

  int n2 [2];
  csVector2 *p2 [2];

  p2 [0] = (csVector2 *)alloca (sizeof (csVector2) * (n + 1));
  p2 [1] = (csVector2 *)alloca (sizeof (csVector2) * (n + 1));

  n2 [0] = n2 [1] = 0;

  if (how_to_divide)
  {
    // Split the polygon vertically by the line "x = sub_x"
    // (p2 [0] -- left poly, p2 [1] -- right poly)

    int where_are_we = p2d [0].x > sub_x;
    p2 [where_are_we] [n2 [where_are_we]++] = p2d [0];
    for (int v = 1, prev = 0; v <= n; v++)
    {
      // Check whenever current vertex is on left or right side of divider
      int cur = (v == n) ? 0 : v;
      int now_we_are = p2d [cur].x > sub_x;

      if (now_we_are == where_are_we)
      {
        // Do not add the first point since it will be added at the end
        if (cur) p2 [where_are_we] [n2 [where_are_we]++] = p2d [cur];
      }
      else
      {
        // The most complex case: find the Y at intersection point
        float y = p2d [prev].y + (p2d [cur].y - p2d [prev].y) *
          (sub_x - p2d [prev].x) / (p2d [cur].x - p2d [prev].x);

        // Add the intersection point to both polygons
  	p2 [0] [n2 [0]++] = p2 [1] [n2 [1]++] = csVector2 (sub_x,y);

  	if (cur) p2 [now_we_are] [n2 [now_we_are]++] = p2d [cur];
      }

      where_are_we = now_we_are;
      prev = cur;
    }

    __rect u;
    u.left = visible.left;
    u.right = sub_x;
    u.top = visible.top;
    u.bottom = visible.bottom;
    poly_fill (n2 [0], p2 [0], u);

    u.left = sub_x;
    u.right = visible.right;
    poly_fill (n2[1], p2[1], u);
  }
  else
  {
    // Split the polygon horizontally by the line "y = sub_y"
    // (p[0] -- top poly, p[1] -- bottom poly)

    int where_are_we = p2d [0].y > sub_y;
    p2 [where_are_we] [n2 [where_are_we]++] = p2d [0];
    for (int v = 1, prev = 0; v <= n; v++)
    {
      // Check whenever current vertex is on top or down side of divider
      int cur = (v == n) ? 0 : v;
      int now_we_are = p2d [cur].y > sub_y;

      if (now_we_are == where_are_we)
      {
        // Do not add the first point since it will be added at the end
  	if (cur) p2 [where_are_we] [n2 [where_are_we]++] = p2d [cur];
      }
      else
      {
        // The most complex case: find the X at intersection point
        float x = p2d [prev].x + (p2d [cur].x - p2d [prev].x) *
          (sub_y - p2d [prev].y) / (p2d [cur].y - p2d [prev].y);

        // Add the intersection point to both polygons
  	p2 [0] [n2 [0]++] = p2 [1] [n2 [1]++] = csVector2 (x, sub_y);

  	if (cur) p2 [now_we_are] [n2 [now_we_are]++] = p2d [cur];
      }

      where_are_we = now_we_are;
      prev = cur;
    }

    __rect u;
    u.left = visible.left;
    u.right = visible.right;
    u.top = visible.top;
    u.bottom = sub_y;
    poly_fill (n2[0], p2[0], u);

    u.top = sub_y;
    u.bottom = visible.bottom;
    poly_fill (n2 [1], p2 [1], u);
  }

  depth--;
}

/* Modified by me to add nice lightmaps recalculations -- D.D. */
void csPolyTexture::FillLightMap (csLightView& lview)
{
  if (!lm) return;
  csStatLight* light = (csStatLight*)lview.l;

#define QUADTREE_SHADOW 0
#if QUADTREE_SHADOW
  csQuadcube* qc = csWorld::current_world->GetQuadcube ();
#endif

  int lw = lm->GetWidth ();
  int lh = lm->GetHeight ();

  int u, uv;

  int l1, l2 = 0, l3 = 0;
  float d, dl;

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
  csMatrix3 m_t2w = txt_pl->m_world2tex.GetInverse ();
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

  csVector3 v1, v2(0);

  int ru, rv;
  float invww, invhh;
  invww = 1. / (float)ww;
  invhh = 1. / (float)hh;

  bool hit;     // Set to true if there is a hit
  bool first_time = false;  // Set to true if this is the first pass for the dynamic light
  int dyn;

  unsigned char* mapR;
  unsigned char* mapG;
  unsigned char* mapB;
  csShadowMap* smap;

  int i;
  smap = NULL;
  hit = false;

  dyn = light->IsDynamic ();
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
  long lm_size = lm->lm_size;

  float miny = 1000000, maxy = -1000000;
  int MaxIndex = -1, MinIndex = -1;

  // Calculate the uv's for all points of the frustrum (the
  // frustrum is actually a clipped version of the polygon).
  csVector2* f_uv = NULL;

  // Our polygon on its own texture space. Weird, isn't it? ;)
  csVector2* rp = NULL;
  int rpv=0;

  csFrustrum* light_frustrum = lview.light_frustrum;
  int num_frustrum = light_frustrum->GetNumVertices ();
  csVector3* frustrum = light_frustrum->GetVertices ();
  int mi;
  CHK (f_uv = new csVector2 [num_frustrum]);

  rpv=polygon->GetVertices ().GetNumVertices();
  CHK (rp = new csVector2 [rpv]);

  csVector3 projector;

  for (i = 0; i < rpv; i++)
  {
    projector = txt_pl->m_world2tex * (polygon->Vwor (i) - txt_pl->v_world2tex);
    rp [i].x = (projector.x * ww - Imin_u) / lightcell_size + 0.5;
    rp [i].y = (projector.y * hh - Imin_v) / lightcell_size + 0.5;
  }

  for (i = 0; i < num_frustrum; i++)
  {
    if (lview.mirror)
      mi = num_frustrum - i - 1;
    else
      mi = i;

    // T = Mwt * (W - Vwt)
    v1 = txt_pl->m_world2tex *
      (frustrum [mi] + light_frustrum->GetOrigin () - txt_pl->v_world2tex);
    f_uv [i].x = (v1.x * ww - Imin_u) / lightcell_size + 0.5;
    f_uv [i].y = (v1.y * hh - Imin_v) / lightcell_size + 0.5;
    if (f_uv [i].y < miny) miny = f_uv [MinIndex = i].y;
    if (f_uv [i].y > maxy) maxy = f_uv [MaxIndex = i].y;
  }

  csColor color = csColor (lview.r, lview.g, lview.b) * NORMAL_LIGHT_LEVEL;

  __texture_width = lw;
  __texture = (float *)calloc (lh * lw, sizeof (float));
  __mark = (unsigned char *)calloc (lh, lw);

  __rect vis = { 0, lw, 0, lh };

  __draw_func = lixel_intensity;
  poly_fill (num_frustrum, f_uv, vis);
  __draw_func = correct_results;
  poly_fill (rpv, rp, vis);

  uv = 0;
  for (int sy = 0; sy < lh; sy++)
  {
    for (u = 0; u < lw; u++, uv++)
    {
      //@@@ (Note from Jorrit): The following test should not be needed
      // but it appears to be anyway. 'uv' can get too large.
      if (uv >= lm_size) continue;

      float usual_value = 1.0;

      float lightintensity = __texture[uv];

      if (!lightintensity)
      {
        usual_value = 0.0;

        if (u && __mark [uv - 1])
        {
          lightintensity += __texture [uv - 1];
          usual_value++;
        }

        if (sy && __mark [uv - lw])
        {
          lightintensity += __texture [uv - lw];
          usual_value++;
        }

        if ((u != lw - 1) && __mark [uv + 1])
        {
          lightintensity += __texture [uv + 1];
          usual_value++;
        }

        if ((sy != lh - 1) && __mark [uv + lw])
        {
          lightintensity += __texture [uv + lw];
          usual_value++;
        }

        if (!lightintensity)
          continue;
      }

      float lightness = lightintensity / usual_value;

      ru = u << lightcell_shift;
      rv = sy << lightcell_shift;

      // rc tests wether or not the lumel will be shadowed.
      // If true then shadow.
      bool rc = false;
      int tst;
      static int shift_u [5] = { 0, 2, 0, -2, 0 };
      static int shift_v [5] = { 0, 0, 2, 0, -2 };
      for (tst = 0 ; tst < 5 ; tst++)
      {
        v1.x = (float)(ru + shift_u [tst] + Imin_u) * invww;
        v1.y = (float)(rv + shift_v [tst] + Imin_v) * invhh;
        if (ABS (txt_C) < SMALL_EPSILON)
          v1.z = 0;
        else
          v1.z = - (txt_D + txt_A*v1.x + txt_B*v1.y) / txt_C;
        v2 = vv + m_t2w * v1;

        // Check if the point on the polygon is shadowed. To do this
        // we traverse all shadow frustrums and see if it is contained in any of them.
        csShadowFrustrum *shadow_frust;
        shadow_frust = lview.shadows.GetFirst ();
        bool shadow = false;
#if QUADTREE_SHADOW
        int state = qc->TestPoint (v2-light_frustrum->GetOrigin ());
        if (state == CS_QUAD_FULL)
        {
          // The quadtree indicates that we have shadow. However, it is possible
          // that we have an adjacent polygon which gives false shadows. Therefor
          // we test if the lumel coordinate falls outside the polygon and if so
          // we do the full frustrum test below.
          if (!light_frustrum->Contains (v2)) state = CS_QUAD_PARTIAL;
        }

        if (state == CS_QUAD_EMPTY) shadow = false;
        else if (state == CS_QUAD_FULL) shadow = true;
        else
#endif
        {
          while (shadow_frust)
          {
            if (shadow_frust->relevant && shadow_frust->polygon != polygon)
              if (shadow_frust->Contains (v2-shadow_frust->GetOrigin ()))
                { shadow = true; break; }
            shadow_frust = shadow_frust->next;
          }
        }
        if (!shadow) { rc = false; break; }

        if (!do_accurate_things) break;
        rc = true;
      }

      if (!rc)
      {
        d = csSquaredDist::PointPoint (lview.light_frustrum->GetOrigin (), v2);
        if (d >= light->GetSquaredRadius ()) continue;

        d = FastSqrt (d);
        hit = true;
        l1 = mapR[uv];

        float cosinus = (v2-lview.light_frustrum->GetOrigin ())*polygon->GetPolyPlane ()->Normal ();
        cosinus /= d;
        cosinus += cosfact;
        if (cosinus < 0) cosinus = 0;
        else if (cosinus > 1) cosinus = 1;

        if (dyn)
        {
          dl = NORMAL_LIGHT_LEVEL/light->GetRadius ();
          l1 = l1 + QInt (lightness*(cosinus * (NORMAL_LIGHT_LEVEL - d*dl)));
          if (l1 > 255) l1 = 255;
          mapR[uv] = l1;
        }
        else
        {
          float brightness = cosinus * light->GetBrightnessAtDistance (d);

          if (lview.r > 0)
          {
            l1 = l1 + QInt (lightness*(color.red * brightness));
            if (l1 > 255) l1 = 255;
            mapR[uv] = l1;
          }
          if (lview.g > 0 && mapG)
          {
            l2 = mapG[uv] + QInt (lightness*(color.green * brightness));
            if (l2 > 255) l2 = 255;
            mapG[uv] = l2;
          }
          if (lview.b > 0 && mapB)
          {
            l3 = mapB[uv] + QInt (lightness*(color.blue * brightness));
            if (l3 > 255) l3 = 255;
            mapB[uv] = l3;
          }
        }
      }
      //mapR[uv] = 128;
      //mapG[uv] = 128;
      //mapB[uv] = 128;
      //if (u == 0 && (v & 1)) { mapR[uv] = 255; mapG[uv] = 0; mapB[uv] = 0; }
      //else if (v == 0 && (u & 1)) { mapR[uv] = 0; mapG[uv] = 255; mapB[uv] = 0; }
      //else if (u == lw-1 && (v & 1)) { mapR[uv] = 0; mapG[uv] = 0; mapB[uv] = 255; }
      //else if (v == lh-1 && (u & 1)) { mapR[uv] = 255; mapG[uv] = 0; mapB[uv] = 255; }
      //else if (u == v) { mapR[uv] = 255; mapG[uv] = 255; mapB[uv] = 0; }
      //else if (u == lh-1-v) { mapR[uv] = 0; mapG[uv] = 255; mapB[uv] = 255; }
    }
  }

  CHK (delete [] f_uv);
  CHK (delete [] rp);

  free(__texture);
  free(__mark);

  if (dyn && first_time)
  {
    if (!hit)
    {
      // There was no hit. Just remove the dynamic light map from the polygon
      // unless it was not allocated this turn.
      lm->DelShadowMap (smap);
    }
    else
    {
      // There was a hit. Register this polygon with the light.
      light->RegisterPolygon (polygon);
    }
  }
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
  long lm_size = lm->lm_size;

  int i;
  float miny = 1000000, maxy = -1000000;
  int MaxIndex = -1, MinIndex = -1;

  // Calculate the uv's for all points of the frustrum (the
  // frustrum is actually a clipped version of the polygon).
  csVector2* f_uv = NULL;
  if (lp->vertices)
  {
    int mi;
    CHK (f_uv = new csVector2 [lp->num_vertices]);
    for (i = 0 ; i < lp->num_vertices ; i++)
    {
      //if (lview.mirror) mi = lview.num_frustrum-i-1;
      //else mi = i;
      mi = i;

      // T = Mwt * (W - Vwt)
      //v1 = pl->m_world2tex * (lp->vertices[mi] + lp->center - pl->v_world2tex);
      //@@@ This is only right if we don't allow reflections on dynamic lights
      v1 = txt_pl->m_world2tex * (lp->vertices[mi] + light->GetCenter () - txt_pl->v_world2tex);
      f_uv[i].x = (v1.x * ww - Imin_u) / lightcell_size;
      f_uv[i].y = (v1.y * hh - Imin_v) / lightcell_size;
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

        if(fabs(f_uv[scanR2].y-f_uv[MaxIndex].y)<EPS)
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

        if(fabs(f_uv[scanL2].y-f_uv[MaxIndex].y)<EPS)
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

	//@@@ (Note from Jorrit): The following test should not be needed
	// but it appears to be anyway. 'uv' can get both negative and too large.
	if (uv < 0 || uv >= lm_size) continue;

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
	// we traverse all shadow frustrums and see if it is contained in any of them.
	csShadowFrustrum* shadow_frust;
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
  polygon->IncRef ();
  return polygon;
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
