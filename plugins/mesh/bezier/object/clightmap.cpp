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

#include "csgeom/tri.h"
#include "csutil/csendian.h"
#include "csutil/memfile.h"
#include "csutil/util.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iutil/cache.h"
#include "iutil/vfs.h"

#include "beziermsh.h"
#include "clightmap.h"
#include "curvebase.h"

#define LMMAGIC	    "LM04" // must be 4 chars!

CS_PLUGIN_NAMESPACE_BEGIN(Bezier)
{

csCurveShadowMap::csCurveShadowMap ()
{
  Light = 0;
  max_shadow = 255;  // use worst case until calc'd
}

csCurveShadowMap::~csCurveShadowMap ()
{
}

void csCurveShadowMap::Alloc (iLight *l, int w, int h)
{
  Light = l;

  int lw = csCurveLightMap::CalcLightMapWidth (w);
  int lh = csCurveLightMap::CalcLightMapHeight (h);
  csCurveShadowMapHelper::SetSize (lw * lh);
  memset (GetArray (), 0, GetSize ());
}

void csCurveShadowMap::CalcMaxShadow()
{
  max_shadow=0;
  int len = (int)GetSize ();
  for (int i=0; i<len; i++)
    if (GetArray()[i] > max_shadow)
      max_shadow = GetArray()[i];
}

//---------------------------------------------------------------------------
int csCurveLightMap::lightcell_size = 16;
int csCurveLightMap::lightcell_shift = 4;
int csCurveLightMap::default_lightmap_cell_size = 16;

csCurveLightMap::csCurveLightMap ()
{
  first_smap = 0;
  delayed_light_info = 0;
  mean_recalc = true;
  max_static_color_values.Set(255,255,255);  // use slowest safest method by default
}

csCurveLightMap::~csCurveLightMap ()
{
  while (first_smap)
  {
    csCurveShadowMap *smap = first_smap->next;
    delete first_smap;
    first_smap = smap;
  }

  static_lm.DeleteAll ();
  real_lm.DeleteAll ();
}

void csCurveLightMap::SetLightCellSize (int size)
{
  lightcell_size = size;
  lightcell_shift = csLog2 (size);
}

void csCurveLightMap::DelShadowMap (csCurveShadowMap *smap)
{
  if (smap == first_smap)
  {
    first_smap = smap->next;
  }
  else
  {
    csCurveShadowMap* map = first_smap;
    while (map && map->next != smap)
    {
      map = map->next;
    }
    if (map)
    {
      map->next = smap->next;
    }
  }
  delete smap;
}


csCurveShadowMap *csCurveLightMap::NewShadowMap (iLight *light, int w, int h)
{
  csCurveShadowMap *smap = new csCurveShadowMap ();
  smap->Light = light;
  smap->next = first_smap;
  first_smap = smap;

  smap->Alloc (light, w, h);

  return smap;
}

csCurveShadowMap *csCurveLightMap::FindShadowMap (iLight *light)
{
  csCurveShadowMap *smap = first_smap;
  while (smap)
  {
    if (smap->Light == light) return smap;
    smap = smap->next;
  }

  return 0;
}

void csCurveLightMap::SetSize (int w, int h)
{
  rwidth = lwidth = csCurveLightMap::CalcLightMapWidth (w);
  rheight = lheight = csCurveLightMap::CalcLightMapHeight (h);
  lm_size = lwidth * lheight;
}

void csCurveLightMap::Alloc (int w, int h, int r, int g, int b)
{
  SetSize (w, h);
  static_lm.DeleteAll ();
  real_lm.DeleteAll ();

  static_lm.SetSize (lm_size);
  real_lm.SetSize (lm_size);

  csRGBpixel *map = static_lm.GetArray ();
  csRGBpixel def (r, g, b);

  // don't know why, but the previous implementation did this:
  def.alpha = 128;

  int i;
  for (i = 0; i < lm_size; i++) map[i] = def;
}

struct PolySave
{
  char header[4];
  int32 lm_size;            // Size of lightmap
  int32 lm_cnt;             // Number of non-dynamic lightmaps (normally 3)
};

struct LightSave
{
  char light_id[16];
};

struct LightHeader
{
  char header[4];
  int32 dyn_cnt;             // Number of dynamic maps
};

bool csCurveLightMap::UpdateRealLightMap (float dyn_ambient_r,
                                     float dyn_ambient_g,
                                     float dyn_ambient_b,
                                     bool  dyn_dirty)
{
  if (!dyn_dirty) return false;

  mean_recalc = true;

  csRGBpixel temp_max_color_values = max_static_color_values;

  //---
  // First copy the static lightmap to the real lightmap.
  // Remember the real lightmap first so that we can see if
  // there were any changes.
  //---
  if (dyn_ambient_r || dyn_ambient_g || dyn_ambient_b)
  {
    csRGBpixel ambient;
    int dr = csQint (dyn_ambient_r * 128);
    int dg = csQint (dyn_ambient_g * 128);
    int db = csQint (dyn_ambient_b * 128);
    if (dr > 255) dr = 255;
    if (dg > 255) dg = 255;
    if (db > 255) db = 255;
    ambient.Set ((unsigned char)dr, (unsigned char)dg, (unsigned char)db,
		 0);	// Use alpha 0 so we can use this for UnsafeAdd.

    if (max_static_color_values.red   + ambient.red   < 256  &&
        max_static_color_values.green + ambient.green < 256  &&
        max_static_color_values.blue  + ambient.blue  < 256)
    {
      // No lightmap overflows so we can use fastest loop with no checking.
      for (int i=0; i<lm_size; i++)
      {
        real_lm[i] = static_lm[i];
        real_lm[i].UnsafeAdd (ambient);
      }
      temp_max_color_values.UnsafeAdd (ambient);
    }
    else
    {
      // An overflow is somewhere here, so check each and every addition.
      for (int i=0; i<lm_size; i++)
      {
        real_lm[i] = static_lm[i];
	real_lm[i].SafeAdd (ambient);
      }
      temp_max_color_values.SafeAdd (ambient);
    }
  }
  else
    memcpy (real_lm.GetArray (), static_lm.GetArray (), 4 * lm_size);

  //---
  // Then add all pseudo-dynamic lights.
  //---
  iLight *light;
  csRGBpixel *map;
  float red, green, blue;
  unsigned char *p, *last_p;
  int l, s;

  if (first_smap)
  {
    csCurveShadowMap *smap = first_smap;

    // Color mode.
    do
    {
      map = real_lm.GetArray ();
      light = smap->Light;
      red = light->GetColor ().red;
      green = light->GetColor ().green;
      blue = light->GetColor ().blue;
      p = smap->GetArray ();
      last_p = p + lm_size;

      int tm_r = temp_max_color_values.red   + csQint (smap->max_shadow * red);
      int tm_g = temp_max_color_values.green + csQint (smap->max_shadow * green);
      int tm_b = temp_max_color_values.blue  + csQint (smap->max_shadow * blue);

      if (tm_r < 256  && tm_g < 256  && tm_b < 256)
      {
        // Again, if there is no risk of overflow, use fastest possible merge.
        do
        {
          s = *p++;

          map->red   += csQround (red * s);
          map->green += csQround (green * s);
          map->blue  += csQround (blue * s);
          map++;
        } while (p < last_p);

        // Now update max color to include this merged shadowmap.
        temp_max_color_values.red   = tm_r;
        temp_max_color_values.green = tm_g;
        temp_max_color_values.blue  = tm_b;
      }
      else
      {
        do
        {
          s = *p++;

          l = map->red + csQround (red * s);
          map->red = l < 255 ? l : 255;
          l = map->green + csQround (green * s);
          map->green = l < 255 ? l : 255;
          l = map->blue + csQround (blue * s);
          map->blue = l < 255 ? l : 255;

          map++;

        } while (p < last_p);
        // Now update max color to include this merged shadowmap
        temp_max_color_values.red = (tm_r>255)?255:tm_r;
        temp_max_color_values.green = (tm_g>255)?255:tm_g;
        temp_max_color_values.blue= (tm_b>255)?255:tm_b;
      }
      smap = smap->next;
    } while (smap);
  }

  return true;
}

void csCurveLightMap::ConvertToMixingMode ()
{
}

void csCurveLightMap::CalcMaxStatic()
{
  max_static_color_values.Set(0,0,0);

  csRGBpixel *map = static_lm.GetArray ();
  for (int i = 0; i < lm_size; i++)
  {
    if (max_static_color_values.red < map->red)
        max_static_color_values.red = map->red;
    if (max_static_color_values.green < map->green)
        max_static_color_values.green = map->green;
    if (max_static_color_values.blue < map->blue)
        max_static_color_values.blue = map->blue;
    map++;
  }
}

void csCurveLightMap::CalcMeanLighting ()
{
  int i;
  int mer, meg, meb;
  mer = 0;
  meg = 0;
  meb = 0;

  csRGBpixel *map = real_lm.GetArray ();
  for (i = 0; i < lm_size; i++)
  {
    mer += map->red;
    meg += map->green;
    meb += map->blue;
    map++;
  }

  mean_color.red = mer / lm_size;
  mean_color.green = meg / lm_size;
  mean_color.blue = meb / lm_size;
}

// Only works for expanding a map.
static void ResizeMap2 (
  csRGBpixel *old_map,
  int oldw,
  int oldh,
  csRGBpixel *new_map,
  int neww,
  int

  /*newh*/ )
{
  int row;
  for (row = 0; row < oldh; row++)
    memcpy (new_map + neww * row * 4, old_map + oldw * row * 4, oldw * 4);
}

// Only works for expanding a map.
static void ResizeMap (
  unsigned char *old_map,
  int oldw,
  int oldh,
  unsigned char *new_map,
  int neww,
  int

  /*newh*/ )
{
  int row;
  for (row = 0; row < oldh; row++)
    memcpy (new_map + neww * row, old_map + oldw * row, oldw);
}

void csCurveLightMap::ConvertFor3dDriver (bool requirePO2, int maxAspect)
{
  if (!requirePO2) return ; // Nothing to do.
  int oldw = lwidth, oldh = lheight;

  lwidth = csFindNearestPowerOf2 (lwidth);
  lheight = csFindNearestPowerOf2 (lheight);

  while (lwidth / lheight > maxAspect) lheight += lheight;
  while (lheight / lwidth > maxAspect) lwidth += lwidth;
  if (oldw == lwidth && oldh == lheight) return ; // Already ok, nothing to do.

  // Move the old data to o_stat and o_real.
  csRGBMap o_stat, o_real;
  static_lm.TransferTo (o_stat);
  real_lm.TransferTo (o_real);

  lm_size = lwidth * lheight;

  // Allocate new data and transform old to new.
  static_lm.SetSize (lm_size);
  ResizeMap2 (
    o_stat.GetArray (),
    oldw,
    oldh,
    static_lm.GetArray (),
    lwidth,
    lheight);

  real_lm.SetSize (lm_size);
  ResizeMap2 (
    o_real.GetArray (),
    oldw,
    oldh,
    real_lm.GetArray (),
    lwidth,
    lheight);

  // Convert all shadowmaps.
  csCurveShadowMap *smap = first_smap;
  while (smap)
  {
    unsigned char *old_map = new unsigned char[smap->GetSize ()];
    memcpy (old_map, smap->GetArray (), smap->GetSize ());
    ResizeMap (old_map, oldw, oldh, smap->GetArray (), lwidth, lheight);
    delete[] old_map;
    smap = smap->next;
  }
}

csRGBpixel *csCurveLightMap::GetMapData ()
{
  return GetRealMap ().GetArray ();
}

void csCurveLightMap::GetMeanLighting (int &r, int &g, int &b)
{ 
  if (mean_recalc)
  {
    UpdateRealLightMap (0, 0, 0, false);
    CalcMeanLighting ();
    mean_recalc = false;
  }

  r = mean_color.red; 
  g = mean_color.green; 
  b = mean_color.blue; 
}

}
CS_PLUGIN_NAMESPACE_END(Bezier)
