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
#include "csengine/lghtmap.h"
#include "csengine/polygon.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/light.h"
#include "csengine/engine.h"
#include "csengine/curve.h"
#include "csutil/util.h"
#include "csutil/debug.h"
#include "csutil/memfile.h"
#include "iutil/vfs.h"
#include "iutil/cache.h"

csShadowMap::csShadowMap ()
{
  Light = NULL;
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
}

//---------------------------------------------------------------------------
int csLightMap:: lightcell_size = 16;
int csLightMap:: lightcell_shift = 4;

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

csShadowMap *csLightMap::NewShadowMap (csLight *light, int w, int h)
{
  csShadowMap *smap = new csShadowMap ();
  smap->Light = &light->scfiLight;
  smap->next = first_smap;
  first_smap = smap;
  DG_LINK (this, smap);

  smap->Alloc (&light->scfiLight, w, h);

  iStatLight *slight = SCF_QUERY_INTERFACE_FAST (light, iStatLight);
  slight->GetPrivateObject ()->RegisterLightMap (this);
  slight->DecRef ();

  return smap;
}

csShadowMap *csLightMap::FindShadowMap (csLight *light)
{
  csShadowMap *smap = first_smap;
  while (smap)
  {
    if (smap->Light == &light->scfiLight) return smap;
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

void CacheName (
  char *buf,
  char *prefix,
  int id,
  unsigned long ident,
  char *suffix)
{
  if (id == 0)
    sprintf (buf, "%s%lu%s", prefix, ident, suffix);
  else
    sprintf (buf, "lm/%s%d_%lu%s", prefix, id, ident, suffix);
}

bool csLightMap::ReadFromCache (
  iCacheManager* cache_mgr,
  int id,
  int w,
  int h,
  csObject *obj,
  bool isPolygon,
  csEngine *engine)
{
  PolySave ps, pswanted;
  LightHeader lh;
  LightSave ls;
  csLight *light;
  int i;

  csPolygon3D *poly = NULL;
  csCurve *curve = NULL;

  if (isPolygon)
    poly = (csPolygon3D *)obj;
  else
    curve = (csCurve *)obj;

  SetSize (w, h);

  char* type;
  uint32 uid;

  strcpy (pswanted.header, "LM02");
  if (poly)
  {
    pswanted.x1 = convert_endian (float2short (poly->Vobj (0).x));
    pswanted.y1 = convert_endian (float2short (poly->Vobj (0).y));
    pswanted.z1 = convert_endian (float2short (poly->Vobj (0).z));
    pswanted.x2 = convert_endian (float2short (poly->Vobj (1).x));
    pswanted.y2 = convert_endian (float2short (poly->Vobj (1).y));
    pswanted.z2 = convert_endian (float2short (poly->Vobj (1).z));
    type = "lmpol";
    uid = poly->GetPolygonID ();
  }
  else
  {
    type = "lmcur";
    uid = curve->GetCurveID ();
  }

  pswanted.lm_size = convert_endian (lm_size);
  pswanted.lm_cnt = convert_endian ((int32) 111);

  iDataBuffer *data = cache_mgr->ReadCache (type, NULL, uid);
  if (!data) return false;

  char *d = **data;
  memcpy (ps.header, d, 4);
  d += 4;
  memcpy (&ps.x1, d, sizeof (ps.x1));
  d += sizeof (ps.x1);
  memcpy (&ps.y1, d, sizeof (ps.y1));
  d += sizeof (ps.y1);
  memcpy (&ps.z1, d, sizeof (ps.z1));
  d += sizeof (ps.z1);
  memcpy (&ps.x2, d, sizeof (ps.x2));
  d += sizeof (ps.x2);
  memcpy (&ps.y2, d, sizeof (ps.y2));
  d += sizeof (ps.y2);
  memcpy (&ps.z2, d, sizeof (ps.z2));
  d += sizeof (ps.z2);
  memcpy (&ps.lm_size, d, sizeof (ps.lm_size));
  d += sizeof (ps.lm_size);
  memcpy (&ps.lm_cnt, d, sizeof (ps.lm_cnt));
  d += sizeof (ps.lm_cnt);

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
    data->DecRef ();
    return false;
  }

  //-------------------------------
  // The cached item is valid.
  //-------------------------------
  static_lm.Clear ();

  static_lm.Alloc (lm_size);
  memcpy (static_lm.GetArray (), d, lm_size * 4);
  d += lm_size * 4;

  data->DecRef ();

  //-------------------------------
  // Now load the dynamic data.
  //-------------------------------
  if (poly)
    type = "lmpol_d";
  else
    type = "lmcur_d";

  data = cache_mgr->ReadCache (type, NULL, uid);
  if (!data) return true;   // No dynamic data. @@@ Recalc dynamic data?
  d = **data;
  memcpy (lh.header, d, 4);
  d += 4;
  memcpy (&lh.dyn_cnt, d, 4);
  d += 4;
  lh.dyn_cnt = convert_endian (lh.dyn_cnt);

  for (i = 0; i < lh.dyn_cnt; i++)
  {
    memcpy (&ls.light_id, d, sizeof (ls.light_id));
    d += sizeof (ls.light_id);
    ls.light_id = convert_endian (ls.light_id);

    iStatLight *il = engine->FindLight (ls.light_id);
    if (il)
    {
      light = il->GetPrivateObject ();

      csShadowMap *smap = NewShadowMap (light, w, h);
      memcpy (smap->GetArray (), d, lm_size);
    }
    else
    {
      csEngine::current_engine->Warn (
          "Warning! Light (%ld) not found!\n",
          ls.light_id);
    }

    d += lm_size;
  }

  data->DecRef ();

  return true;
}

void csLightMap::Cache (
  iCacheManager* cache_mgr,
  int id,
  csPolygon3D *poly,
  csCurve *curve,
  csEngine *engine)
{
  (void)engine;

  char buf[200];
  PolySave ps;
  long l;
  short s;

  char* type;
  uint32 uid;

  strcpy (ps.header, "LM02");
  if (poly)
  {
    ps.x1 = convert_endian (float2short (poly->Vobj (0).x));
    ps.y1 = convert_endian (float2short (poly->Vobj (0).y));
    ps.z1 = convert_endian (float2short (poly->Vobj (0).z));
    ps.x2 = convert_endian (float2short (poly->Vobj (1).x));
    ps.y2 = convert_endian (float2short (poly->Vobj (1).y));
    ps.z2 = convert_endian (float2short (poly->Vobj (1).z));
    type = "lmpol";
    uid = poly->GetPolygonID ();
  }
  else
  {
    type = "lmcur";
    uid = curve->GetCurveID ();
  }

  ps.lm_size = convert_endian (lm_size);
  ps.lm_cnt = 0;
  ps.lm_cnt = 111;          // Dummy!
  ps.lm_cnt = convert_endian (ps.lm_cnt);

  //-------------------------------
  // Write the normal lightmap data.
  //-------------------------------
  csMemFile* cf = new csMemFile ();

  cf->Write (ps.header, 4);
  s = ps.x1;
  cf->Write ((char *) &s, sizeof (s));
  s = ps.y1;
  cf->Write ((char *) &s, sizeof (s));
  s = ps.z1;
  cf->Write ((char *) &s, sizeof (s));
  s = ps.x2;
  cf->Write ((char *) &s, sizeof (s));
  s = ps.y2;
  cf->Write ((char *) &s, sizeof (s));
  s = ps.z2;
  cf->Write ((char *) &s, sizeof (s));
  l = ps.lm_size;
  cf->Write ((char *) &l, sizeof (l));
  l = ps.lm_cnt;
  cf->Write ((char *) &l, sizeof (l));

  if (static_lm.GetArray ())
    cf->Write ((char *)static_lm.GetArray (), lm_size * 4);

  if (!cache_mgr->CacheData ((void*)(cf->GetData ()), cf->GetSize (),
  	type, NULL, uid))
  {
    cf->DecRef ();
    return;
  }

  // Close the file.
  cf->DecRef ();

  //-------------------------------
  // Write the dynamic data.
  //-------------------------------
  LightHeader lh;

  csShadowMap *smap = first_smap;
  if (smap)
  {
    strcpy (lh.header, "DYNL");
    lh.dyn_cnt = 0;
    while (smap)
    {
      lh.dyn_cnt++;
      smap = smap->next;
    }

    smap = first_smap;

    if (poly)
      type = "lmpol_d";
    else
      type = "lmcur_d";
    cf = new csMemFile ();

    cf->Write (lh.header, 4);
    l = convert_endian (lh.dyn_cnt);
    cf->Write ((char *) &l, 4);
    while (smap)
    {
      csLight *light = smap->Light->GetPrivateObject ();
      if (smap->GetArray ())
      {
        LightSave ls;
        ls.light_id = convert_endian (light->GetLightID ());
        cf->Write ((char *) &ls.light_id, sizeof (ls.light_id));
        cf->Write ((char *)(smap->GetArray ()), lm_size);
      }

      smap = smap->next;
    }

    if (!cache_mgr->CacheData ((void*)(cf->GetData ()), cf->GetSize (),
    	type, NULL, uid))
    {
      cf->DecRef ();
      return;
    }
    cf->DecRef ();
  }
}

bool csLightMap::UpdateRealLightMap ()
{
  if (!dyn_dirty) return false;

  dyn_dirty = false;

  //---

  // First copy the static lightmap to the real lightmap.

  // Remember the real lightmap first so that we can see if

  // there were any changes.

  //---
  memcpy (real_lm.GetArray (), static_lm.GetArray (), 4 * lm_size);

  //---

  // Then add all pseudo-dynamic lights.

  //---
  csLight *light;
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
      light = smap->Light->GetPrivateObject ();
      red = light->GetColor ().red;
      green = light->GetColor ().green;
      blue = light->GetColor ().blue;
      p = smap->GetArray ();
      last_p = p + lm_size;
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

      smap = smap->next;
    } while (smap);
  }

  return true;
}

void csLightMap::ConvertToMixingMode ()
{
  int i;
  int mer, meg, meb;
  mer = 0;
  meg = 0;
  meb = 0;

  csRGBpixel *map = static_lm.GetArray ();
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
