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
#include "lghtmap.h"
#include "polygon.h"
#include "thing.h"
#include "curve.h"
#include "csutil/util.h"
#include "csutil/debug.h"
#include "csutil/memfile.h"
#include "iutil/vfs.h"
#include "iutil/cache.h"
#include "iengine/statlght.h"

#define LMMAGIC	    "LM03" // must be 4 chars!


#ifdef DEBUG_OVERFLOWOPT
int countfast = 0;
int countslow = 0;
#endif

csShadowMap::csShadowMap ()
{
  Light = NULL;
  max_shadow = 255;  // use worst case until calc'd
  DG_ADD (this, NULL);
  DG_TYPE (this, "csShadowMap");
}

csShadowMap::~csShadowMap ()
{
  DG_REM (this);
}

void csShadowMap::Alloc (iLight *l, int w, int h)
{
  Light = l;

  int lw = csLightMap::CalcLightMapWidth (w);
  int lh = csLightMap::CalcLightMapHeight (h);
  csShadowMapHelper::Alloc (lw * lh);
  memset (GetArray (), 0, GetSize ());
}

void csShadowMap::Copy (const csShadowMap *source)
{
  csShadowMapHelper::Copy (source);
  Light = source->Light;
  max_shadow = source->max_shadow;
}

void csShadowMap::CalcMaxShadow()
{
  max_shadow=0;
  int len = GetSize();
  for (int i=0; i<len; i++)
      if (GetArray()[i] > max_shadow)
          max_shadow = GetArray()[i];
}

//---------------------------------------------------------------------------
int csLightMap::lightcell_size = 16;
int csLightMap::lightcell_shift = 4;
int csLightMap::default_lightmap_cell_size = 16;

SCF_IMPLEMENT_IBASE(csLightMap)
  SCF_IMPLEMENTS_INTERFACE(iLightMap)
SCF_IMPLEMENT_IBASE_END

csLightMap::csLightMap ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  DG_ADDI (this, NULL);
  DG_TYPE (this, "csLightMap");
  first_smap = NULL;
  cachedata = NULL;
  delayed_light_info = NULL;
  dyn_dirty = true;
  mean_recalc = true;
  max_static_color_values.Set(255,255,255);  // use slowest safest method by default
}

csLightMap::~csLightMap ()
{
  CS_ASSERT (cachedata == NULL);
  while (first_smap)
  {
    csShadowMap *smap = first_smap->next;
    DG_UNLINK (this, first_smap);
    delete first_smap;
    first_smap = smap;
  }

  static_lm.Clear ();
  real_lm.Clear ();
  DG_REM (this);
}

void csLightMap::SetLightCellSize (int size)
{
  lightcell_size = size;
  lightcell_shift = csLog2 (size);
}

void csLightMap::DelShadowMap (csShadowMap *smap)
{
  first_smap = smap->next;
  DG_UNLINK (this, smap);
  delete smap;
}

csShadowMap *csLightMap::NewShadowMap (iLight *light, int w, int h)
{
  csShadowMap *smap = new csShadowMap ();
  smap->Light = light;
  smap->next = first_smap;
  first_smap = smap;
  DG_LINK (this, smap);

  smap->Alloc (light, w, h);

  return smap;
}

csShadowMap *csLightMap::FindShadowMap (iLight *light)
{
  csShadowMap *smap = first_smap;
  while (smap)
  {
    if (smap->Light == light) return smap;
    smap = smap->next;
  }

  return NULL;
}

void csLightMap::SetSize (int w, int h)
{
  rwidth = lwidth = csLightMap::CalcLightMapWidth (w);
  rheight = lheight = csLightMap::CalcLightMapHeight (h);
  lm_size = lwidth * lheight;
}

void csLightMap::Alloc (int w, int h, int r, int g, int b)
{
  SetSize (w, h);
  static_lm.Clear ();
  real_lm.Clear ();

  static_lm.Alloc (lm_size);
  real_lm.Alloc (lm_size);

  csRGBpixel *map = static_lm.GetArray ();
  csRGBpixel def (r, g, b);

  // don't know why, but the previous implementation did this:
  def.alpha = 128;

  int i;
  for (i = 0; i < lm_size; i++) map[i] = def;
}

void csLightMap::CopyLightMap (csLightMap *source)
{
  lm_size = source->lm_size;
  static_lm.Copy (&source->static_lm);
  real_lm.Copy (&source->real_lm);
  lwidth = source->lwidth;
  lheight = source->lheight;
  rwidth = source->rwidth;
  rheight = source->rheight;

  csShadowMap *smap, *smap2;
  while (first_smap)
  {
    smap = first_smap->next;
    DG_UNLINK (this, first_smap);
    delete first_smap;
    first_smap = smap;
  }

  smap = source->first_smap;
  while (smap)
  {
    smap2 = new csShadowMap ();
    smap2->next = first_smap;
    first_smap = smap2;
    DG_LINK (this, smap2);
    smap2->Copy (smap);
    smap = smap->next;
  }
  max_static_color_values = source->max_static_color_values;
}

struct PolySave
{
  char header[4];
  int16 x1, y1, z1;         // Coordinate of vertex 1
  int16 x2, y2, z2;         // Coordinate of vertex 2
  int32 lm_size;            // Size of lightmap
  int32 lm_cnt;             // Number of non-dynamic lightmaps (normally 3)
};

struct LightSave
{
  unsigned long light_id;
};

struct LightHeader
{
  char header[4];
  long dyn_cnt;             // Number of dynamic maps
};

bool csLightMap::ReadFromCache (
  iFile* file,
  int w,
  int h,
  csObject *obj,
  bool isPolygon,
  iEngine *engine)
{
  PolySave ps, pswanted;
  LightHeader lh;
  LightSave ls;
  iLight *light;
  int i;

  csPolygon3D *poly = NULL;
  csCurve *curve = NULL;
  csThing* parent;

  if (isPolygon)
  {
    poly = (csPolygon3D *)obj;
    parent = poly->GetParent ();
  }
  else
  {
    curve = (csCurve *)obj;
    parent = curve->GetParentThing ();
  }

  SetSize (w, h);

  uint32 uid;

  strcpy (pswanted.header, LMMAGIC);
  if (poly)
  {
    pswanted.x1 = convert_endian (float2short (poly->Vobj (0).x));
    pswanted.y1 = convert_endian (float2short (poly->Vobj (0).y));
    pswanted.z1 = convert_endian (float2short (poly->Vobj (0).z));
    pswanted.x2 = convert_endian (float2short (poly->Vobj (1).x));
    pswanted.y2 = convert_endian (float2short (poly->Vobj (1).y));
    pswanted.z2 = convert_endian (float2short (poly->Vobj (1).z));
    uid = poly->GetPolygonID ();
  }
  else
  {
    uid = curve->GetCurveID ();
  }

  pswanted.lm_size = convert_endian (lm_size);
  pswanted.lm_cnt = convert_endian ((int32) 111);

  char type[5];
  if (file->Read (type, 4) != 4)
    return false;
  type[4] = 0;
  if (strcmp (type, "lmpn") != 0)
    return false;

  if (file->Read ((char*)&ps, sizeof (ps)) != sizeof (ps))
    return false;

  //-------------------------------
  // Check if cached item is still valid.
  //-------------------------------
  if (
    strncmp (ps.header, pswanted.header, 4) != 0 ||
    (
      poly &&
      (
        ps.lm_cnt != pswanted.lm_cnt ||
        ps.x1 != pswanted.x1 ||
        ps.y1 != pswanted.y1 ||
        ps.z1 != pswanted.z1 ||
        ps.x2 != pswanted.x2 ||
        ps.y2 != pswanted.y2 ||
        ps.z2 != pswanted.z2 ||
        ps.lm_size != pswanted.lm_size
      )
    ))
  {
    // Invalid.
    return false;
  }

  //-------------------------------
  // The cached item is valid.
  //-------------------------------
  static_lm.Clear ();

  static_lm.Alloc (lm_size);

  int n = lm_size;
  char *lm_ptr = (char*)static_lm.GetArray ();
  while (--n >= 0) 
  {
    if (file->Read ((char*)lm_ptr, 3) != 3)
      return false;
    lm_ptr += 3;
    *(lm_ptr++) = -127;
  }

  //-------------------------------
  // Check if we have dynamic data.
  //-------------------------------
  uint8 have_dyn;
  if (file->Read ((char*)&have_dyn, sizeof (have_dyn)) != sizeof (have_dyn))
    return false;
  if (have_dyn == 0)
    goto stop;   // No dynamic data. @@@ Recalc dynamic data?

  //-------------------------------
  // Now load the dynamic data.
  //-------------------------------
  if (file->Read ((char*)lh.header, 4) != 4)
    return false;
  if (file->Read ((char*)&lh.dyn_cnt, 4) != 4)
    return false;
  lh.dyn_cnt = convert_endian (lh.dyn_cnt);

  uint32 size;
  if (file->Read ((char*)&size, sizeof (size)) != sizeof (size))
    return false;

  // Calculate the expected size and see if it matches with the
  // size we still have in our data buffer. If it doesn't match
  // we don't load anything.
  size = convert_endian (size);

  unsigned int expected_size;
  expected_size = lh.dyn_cnt * (sizeof (ls.light_id) + lm_size);
  if (expected_size != size)
    return false;

  iLightingInfo* li;
  li = &(parent->scfiLightingInfo);

  for (i = 0 ; i < lh.dyn_cnt ; i++)
  {
    if (file->Read ((char*)&ls.light_id, sizeof (ls.light_id))
    	!= sizeof (ls.light_id))
      return false;
    ls.light_id = convert_endian (ls.light_id);

    iStatLight *il = engine->FindLight (ls.light_id);
    if (il)
    {
      light = il->QueryLight ();
      csShadowMap *smap = NewShadowMap (light, w, h);

      csRef<iStatLight> slight (SCF_QUERY_INTERFACE (il, iStatLight));
      slight->AddAffectedLightingInfo (li);

      if ((long) file->Read ((char*)(smap->GetArray ()), lm_size) != lm_size)
        return false;
      smap->CalcMaxShadow ();
    }
    else
    {
      parent->thing_type->Warn (
          "Warning! Light (%ld) not found!\n", ls.light_id);
    }
  }

stop:
  CalcMaxStatic ();
  
  return true;
}

void csLightMap::Cache (
  iFile* file,
  csPolygon3D *poly,
  csCurve *curve,
  iEngine *engine)
{
  (void)engine;

  PolySave ps;

  uint32 uid;

  strcpy (ps.header, LMMAGIC);
  if (poly)
  {
    ps.x1 = convert_endian (float2short (poly->Vobj (0).x));
    ps.y1 = convert_endian (float2short (poly->Vobj (0).y));
    ps.z1 = convert_endian (float2short (poly->Vobj (0).z));
    ps.x2 = convert_endian (float2short (poly->Vobj (1).x));
    ps.y2 = convert_endian (float2short (poly->Vobj (1).y));
    ps.z2 = convert_endian (float2short (poly->Vobj (1).z));
    uid = poly->GetPolygonID ();
  }
  else
  {
    uid = curve->GetCurveID ();
  }

  if (file->Write ("lmpn", 4) != 4)
    return;

  ps.lm_size = convert_endian (lm_size);
  ps.lm_cnt = 0;
  ps.lm_cnt = 111;          // Dummy!
  ps.lm_cnt = convert_endian (ps.lm_cnt);

  //-------------------------------
  // Write the normal lightmap data.
  //-------------------------------
  file->Write ((char*)&ps, sizeof (ps));

  int n = lm_size;
  char* lm_ptr = (char*)static_lm.GetArray ();
  while (--n >= 0) 
  {
    file->Write (lm_ptr, 3);
    lm_ptr += 4;
  }

  //-------------------------------
  // Write the dynamic data.
  //-------------------------------
  LightHeader lh;

  csShadowMap *smap = first_smap;
  if (smap)
  {
    uint8 have_dyn = 1;
    file->Write ((char*)&have_dyn, sizeof (have_dyn));

    strcpy (lh.header, "DYNL");
    lh.dyn_cnt = 0;
    while (smap)
    {
      lh.dyn_cnt++;
      smap = smap->next;
    }

    smap = first_smap;

    file->Write (lh.header, 4);
    uint32 l = convert_endian (lh.dyn_cnt);
    file->Write ((char *) &l, 4);

    // unsigned long == ls.light_id.
    uint32 size = lh.dyn_cnt * (sizeof (unsigned long) + lm_size);
    file->Write ((char*)&size, sizeof (size));

    while (smap)
    {
      iLight *light = smap->Light;
      if (smap->GetArray ())
      {
        LightSave ls;
        ls.light_id = convert_endian (light->GetLightID ());
        file->Write ((char *) &ls.light_id, sizeof (ls.light_id));
        file->Write ((char *)(smap->GetArray ()), lm_size);
      }

      smap = smap->next;
    }
  }
  else
  {
    uint8 have_dyn = 0;
    file->Write ((char*)&have_dyn, sizeof (have_dyn));
  }
}

bool csLightMap::UpdateRealLightMap (float dyn_ambient_r,
                                     float dyn_ambient_g,
                                     float dyn_ambient_b,
                                     bool  amb_dirty)
{
  if (!dyn_dirty && !amb_dirty) return false;

  dyn_dirty = false;
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
    ambient.Set ((unsigned char)(dyn_ambient_r * (255/1.5)),
                 (unsigned char)(dyn_ambient_g * (255/1.5)),
                 (unsigned char)(dyn_ambient_b * (255/1.5)));

    if (max_static_color_values.red   + ambient.red   < 256  &&
        max_static_color_values.green + ambient.green < 256  &&
        max_static_color_values.blue  + ambient.blue  < 256)
    {
      // no lightmap overflows so we can use fastest loop with no checking
      for (int i=0; i<lm_size; i++)
      {
        real_lm[i] = static_lm[i];
        real_lm[i].UnsafeAdd (ambient);
      }
      temp_max_color_values.UnsafeAdd (ambient);
#ifdef DEBUG_OVERFLOWOPT
      countfast++;
#endif
    }
    else  // an overflow is somewhere here, so check each and every addition
    {
      for (int i=0; i<lm_size; i++)
      {
        real_lm[i] = static_lm[i];
        int color = ambient.red+real_lm[i].red;
        real_lm[i].red = (color > 255) ? 255 : color;
        color = ambient.green+real_lm[i].green;
        real_lm[i].green= (color > 255) ? 255 : color;
        color = ambient.blue+real_lm[i].blue;
        real_lm[i].blue = (color > 255) ? 255 : color;
      }
      int color = ambient.red+max_static_color_values.red;
      temp_max_color_values.red = (color > 255) ? 255 : color;
      color = ambient.green+max_static_color_values.green;
      temp_max_color_values.green = (color > 255) ? 255 : color;
      color = ambient.blue+max_static_color_values.blue;
      temp_max_color_values.blue = (color > 255) ? 255 : color;
#ifdef DEBUG_OVERFLOWOPT
      countslow++;
#endif
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
    csShadowMap *smap = first_smap;

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

      if (temp_max_color_values.red   + smap->max_shadow * red   < 256  &&
          temp_max_color_values.green + smap->max_shadow * green < 256  &&
          temp_max_color_values.blue  + smap->max_shadow * blue  < 256)
      {
        // Again, if there is no risk of overflow, use fastest possible merge
        do
        {
          s = *p++;

          map->red   += QRound (red * s);
          map->green += QRound (green * s);
          map->blue  += QRound (blue * s);
          map++;
        } while (p < last_p);

        // Now update max color to include this merged shadowmap
        temp_max_color_values.red   += (unsigned char)(smap->max_shadow * red);
        temp_max_color_values.green += (unsigned char)(smap->max_shadow * green);
        temp_max_color_values.blue  += (unsigned char)(smap->max_shadow * blue);
#ifdef DEBUG_OVERFLOWOPT
        countfast++;
#endif
      }
      else
      {
        do
        {
          s = *p++;

          l = map->red + QRound (red * s);
          map->red = l < 255 ? l : 255;
          l = map->green + QRound (green * s);
          map->green = l < 255 ? l : 255;
          l = map->blue + QRound (blue * s);
          map->blue = l < 255 ? l : 255;

          map++;

        } while (p < last_p);
        // Now update max color to include this merged shadowmap
        int color = int(temp_max_color_values.red + smap->max_shadow * red);
        temp_max_color_values.red = (color>255)?255:color;
        color = int(temp_max_color_values.green + smap->max_shadow * green);
        temp_max_color_values.green = (color>255)?255:color;
        color = int(temp_max_color_values.blue + smap->max_shadow * blue);
        temp_max_color_values.blue= (color>255)?255:color;

#ifdef DEBUG_OVERFLOWOPT
        countslow++;
#endif
      }
      smap = smap->next;
    } while (smap);
  }

  return true;
}

void csLightMap::ConvertToMixingMode ()
{
}

void csLightMap::CalcMaxStatic()
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

void csLightMap::CalcMeanLighting ()
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

void csLightMap::ConvertFor3dDriver (bool requirePO2, int maxAspect)
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
  o_stat.TakeOver (&static_lm);
  o_real.TakeOver (&real_lm);

  lm_size = lwidth * lheight;

  // Allocate new data and transform old to new.
  static_lm.Alloc (lm_size);
  ResizeMap2 (
    o_stat.GetArray (),
    oldw,
    oldh,
    static_lm.GetArray (),
    lwidth,
    lheight);

  real_lm.Alloc (lm_size);
  ResizeMap2 (
    o_real.GetArray (),
    oldw,
    oldh,
    real_lm.GetArray (),
    lwidth,
    lheight);

  // Convert all shadowmaps.
  csShadowMap *smap = first_smap;
  while (smap)
  {
    unsigned char *old_map = smap->GetArray ();
    smap->TakeOver (new unsigned char[lm_size], smap->GetSize (), false);
    ResizeMap (old_map, oldw, oldh, smap->GetArray (), lwidth, lheight);
    delete[] old_map;
    smap = smap->next;
  }
}

csRGBpixel *csLightMap::GetMapData ()
{
  return GetRealMap ().GetArray ();
}

void csLightMap::GetMeanLighting (int &r, int &g, int &b)
{ 
  if (mean_recalc)
  {
    UpdateRealLightMap ();
    CalcMeanLighting ();
    mean_recalc = false;
  }

  r = mean_color.red; 
  g = mean_color.green; 
  b = mean_color.blue; 
}
