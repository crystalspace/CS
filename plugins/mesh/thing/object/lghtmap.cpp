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
#include "csutil/util.h"
#include "csutil/debug.h"
#include "csutil/memfile.h"
#include "iutil/vfs.h"
#include "iutil/cache.h"
#include "iengine/statlght.h"

#define LMMAGIC	    "LM04" // must be 4 chars!


csShadowMap::csShadowMap ()
{
  Light = 0;
  max_shadow = 255;  // use worst case until calc'd
}

csShadowMap::~csShadowMap ()
{
}

void csShadowMap::Alloc (iLight *l, int w, int h)
{
  Light = l;

  int lw = csLightMap::CalcLightMapWidth (w);
  int lh = csLightMap::CalcLightMapHeight (h);
  csShadowMapHelper::SetLength (lw * lh);
  memset (GetArray (), 0, Length ());
}

void csShadowMap::CalcMaxShadow()
{
  max_shadow=0;
  int len = Length ();
  for (int i=0; i<len; i++)
      if (GetArray()[i] > max_shadow)
          max_shadow = GetArray()[i];
}

//---------------------------------------------------------------------------
int csLightMap::lightcell_size = 16;
int csLightMap::lightcell_shift = 4;
int csLightMap::default_lightmap_cell_size = 16;

csLightMap::csLightMap ()
{
  first_smap = 0;
  mean_recalc = true;
  max_static_color_values.Set(255,255,255);  // use slowest safest method by default
}

csLightMap::~csLightMap ()
{
  while (first_smap)
  {
    csShadowMap *smap = first_smap->next;
    delete first_smap;
    first_smap = smap;
  }

  static_lm.DeleteAll ();
  real_lm.DeleteAll ();
}

void csLightMap::SetLightCellSize (int size)
{
  lightcell_size = size;
  lightcell_shift = csLog2 (size);
}

void csLightMap::DelShadowMap (csShadowMap *smap)
{
  if (smap == first_smap)
  {
    first_smap = smap->next;
  }
  else
  {
    csShadowMap* map = first_smap;
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

csShadowMap *csLightMap::NewShadowMap (iLight *light, int w, int h)
{
  csShadowMap *smap = new csShadowMap ();
  smap->Light = light;
  smap->next = first_smap;
  first_smap = smap;

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

  return 0;
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
  static_lm.DeleteAll ();
  real_lm.DeleteAll ();

  static_lm.SetLength (lm_size);
  real_lm.SetLength (lm_size);

  csRGBpixel def (r, g, b);

  // don't know why, but the previous implementation did this:
  def.alpha = 128;

  int i;
  for (i = 0; i < lm_size; i++) static_lm[i] = def;
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
  char light_id[16];
};

struct LightHeader
{
  char header[4];
  int32 dyn_cnt;             // Number of dynamic maps
};

const char* csLightMap::ReadFromCache (
  iFile* file,
  int w,
  int h,
  csPolygon3D* poly,
  iEngine *engine)
{
  PolySave ps, pswanted;
  LightHeader lh;
  LightSave ls;
  iLight *light;
  int i;

  csThing* parent = poly->GetParent ();

  SetSize (w, h);

  strcpy (pswanted.header, LMMAGIC);
  if (poly)
  {
    csPolygon3DStatic* spoly = poly->GetStaticData ();
    pswanted.x1 = float2short (spoly->Vobj (0).x);
    pswanted.y1 = float2short (spoly->Vobj (0).y);
    pswanted.z1 = float2short (spoly->Vobj (0).z);
    pswanted.x2 = float2short (spoly->Vobj (1).x);
    pswanted.y2 = float2short (spoly->Vobj (1).y);
    pswanted.z2 = float2short (spoly->Vobj (1).z);
  }
  pswanted.lm_size = lm_size;
  pswanted.lm_cnt = 111;

  char type[5];
  if (file->Read (type, 4) != 4)
    return "File too short while reading magic number!";
  type[4] = 0;
  if (strcmp (type, "lmpn") != 0)
    return "File doesn't appear to be a lightmap (magic number mismatch)!";

  if (file->Read ((char*)&ps, sizeof (ps)) != sizeof (ps))
    return "File too short while reading lightmap info header!";

  // Endian convert the file header.
  ps.x1 = convert_endian (ps.x1);
  ps.y1 = convert_endian (ps.y1);
  ps.z1 = convert_endian (ps.z1);
  ps.x2 = convert_endian (ps.x2);
  ps.y2 = convert_endian (ps.y2);
  ps.z2 = convert_endian (ps.z2);
  ps.lm_size = convert_endian (ps.lm_size);
  ps.lm_cnt = convert_endian (ps.lm_cnt);

  //-------------------------------
  // From this point on we will try to skip the wrong lightmap
  // so that we can continue processing further lightmaps.
  //-------------------------------

  //-------------------------------
  // Check if cached item is still valid.
  //-------------------------------
  static char error_buf[512];
  *error_buf = 0;
  if (strncmp (ps.header, pswanted.header, 4) != 0)
    sprintf (error_buf, "Cached lightmap header doesn't match!");
  else if (poly)
  {
    if (ps.lm_cnt != pswanted.lm_cnt)
      sprintf (error_buf,
      	"Cached lightmap header mismatch (got cnt=%d, expected %d)!",
	ps.lm_cnt, pswanted.lm_cnt);
    else if (ps.lm_size != pswanted.lm_size)
      sprintf (error_buf,
      	"Cached lightmap base texture mismatch (got size=%d, expected %d)!",
	ps.lm_size, pswanted.lm_size);
    else if (ps.x1 != pswanted.x1 || ps.y1 != pswanted.y1
    		|| ps.z1 != pswanted.z1)
      sprintf (error_buf, "Cached lightmap first vertex mismatch!");
    else if (ps.x2 != pswanted.x2 || ps.y2 != pswanted.y2
    		|| ps.z2 != pswanted.z2)
      sprintf (error_buf, "Cached lightmap second vertex mismatch!");
  }
  if (*error_buf)
  {
    // Invalid.
    // First try to skip the cached lightmap.
    char* data = new char [ps.lm_size*3];
    if (file->Read (data, ps.lm_size*3) != (size_t)ps.lm_size*3)
      return error_buf;
    delete[] data;

    uint8 have_dyn;
    if (file->Read ((char*)&have_dyn, sizeof (have_dyn)) != sizeof (have_dyn))
      return error_buf;
    if (have_dyn)
    {
      if (file->Read ((char*)lh.header, 4) != 4)
        return error_buf;
      if (file->Read ((char*)&lh.dyn_cnt, 4) != 4)
        return error_buf;
      lh.dyn_cnt = convert_endian (lh.dyn_cnt);
      uint32 size;
      if (file->Read ((char*)&size, sizeof (size)) != sizeof (size))
        return error_buf;
      size = convert_endian (size);
      data = new char[size];
      file->Read (data, size);
      delete[] data;
    }

    return error_buf;
  }

  //-------------------------------
  // The cached item is valid.
  //-------------------------------
  static_lm.DeleteAll ();
  static_lm.SetLength (lm_size);

  int n = lm_size;
  char *lm_ptr = (char*)static_lm.GetArray ();
  while (--n >= 0) 
  {
    if (file->Read ((char*)lm_ptr, 3) != 3)
      return "File too short while reading static lightmap data!";
    lm_ptr += 3;
    *(lm_ptr++) = -127;
  }

  //-------------------------------
  // Check if we have dynamic data.
  //-------------------------------
  uint8 have_dyn;
  if (file->Read ((char*)&have_dyn, sizeof (have_dyn)) != sizeof (have_dyn))
    return "File too short while reading pseudo-dynamic lighting indicator!";
  if (have_dyn == 0)
    goto stop;   // No dynamic data. @@@ Recalc dynamic data?

  //-------------------------------
  // Now load the dynamic data.
  //-------------------------------
  if (file->Read ((char*)lh.header, 4) != 4)
    return "File too short at start of dynamic lightmaps!";
  if (file->Read ((char*)&lh.dyn_cnt, 4) != 4)
    return "File too short at start of dynamic lightmaps!";
  lh.dyn_cnt = convert_endian (lh.dyn_cnt);

  uint32 size;
  if (file->Read ((char*)&size, sizeof (size)) != sizeof (size))
    return "File too short at start of dynamic lightmaps!";

  // Calculate the expected size and see if it matches with the
  // size we still have in our data buffer. If it doesn't match
  // we don't load anything.
  size = convert_endian (size);

  unsigned int expected_size;
  expected_size = lh.dyn_cnt * (sizeof (LightSave) + lm_size);
  if (expected_size != size)
  {
    // Skip the data in the cache so we can potentially proceed
    // to the next lightmap.
    char* data = new char[size];
    file->Read (data, size);
    delete[] data;
    return "Mismatch with expected number of pseudo-dynamic lightmaps!";
  }

  iLightingInfo* li;
  li = &(parent->scfiLightingInfo);

  for (i = 0 ; i < lh.dyn_cnt ; i++)
  {
    if (file->Read ((char*)&ls.light_id, sizeof (LightSave))
    	!= sizeof (LightSave))
      return "File too short while reading pseudo-dynamic lightmap header!";
    size -= sizeof (LightSave);

    iStatLight *il = engine->FindLightID (ls.light_id);
    if (il)
    {
      light = il->QueryLight ();
      csShadowMap *smap = NewShadowMap (light, w, h);

      il->AddAffectedLightingInfo (li);

      if ((long) file->Read ((char*)(smap->GetArray ()), lm_size) != lm_size)
        return "File too short while reading pseudo-dynamic lightmap data!";
      size -= lm_size;
      smap->CalcMaxShadow ();
    }
    else
    {
      // Skip the data in the cache so we can potentially proceed to the
      // next lightmap.
      char* data = new char[size];
      file->Read (data, size);
      delete[] data;
      return "Couldn't find the pseudo-dynamic light for this lightmap!";
    }
  }

stop:
  CalcMaxStatic ();
  
  return 0;
}

void csLightMap::Cache (
  iFile* file,
  csPolygon3D *poly,
  iEngine *engine)
{
  (void)engine;

  PolySave ps;

  strcpy (ps.header, LMMAGIC);
  if (poly)
  {
    csPolygon3DStatic* spoly = poly->GetStaticData ();
    ps.x1 = convert_endian (float2short (spoly->Vobj (0).x));
    ps.y1 = convert_endian (float2short (spoly->Vobj (0).y));
    ps.z1 = convert_endian (float2short (spoly->Vobj (0).z));
    ps.x2 = convert_endian (float2short (spoly->Vobj (1).x));
    ps.y2 = convert_endian (float2short (spoly->Vobj (1).y));
    ps.z2 = convert_endian (float2short (spoly->Vobj (1).z));
  }

  if (file->Write ("lmpn", 4) != 4)
    return;

  ps.lm_size = convert_endian ((int32)lm_size);
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
    uint32 size = lh.dyn_cnt * (sizeof (LightSave) + lm_size);
    uint32 s = convert_endian (size);
    file->Write ((char*)&s, sizeof (s));

    while (smap)
    {
      iLight *light = smap->Light;
      if (smap->GetArray ())
      {
        LightSave ls;
	memcpy (ls.light_id, light->GetLightID (), sizeof (LightSave));
        file->Write ((char *) &ls.light_id, sizeof (LightSave));
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
    int dr = QInt (dyn_ambient_r * 128);
    int dg = QInt (dyn_ambient_g * 128);
    int db = QInt (dyn_ambient_b * 128);
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

      int tm_r = temp_max_color_values.red   + QInt (smap->max_shadow * red);
      int tm_g = temp_max_color_values.green + QInt (smap->max_shadow * green);
      int tm_b = temp_max_color_values.blue  + QInt (smap->max_shadow * blue);

      if (tm_r < 256  && tm_g < 256  && tm_b < 256)
      {
        // Again, if there is no risk of overflow, use fastest possible merge.
        do
        {
          s = *p++;

          map->red   += QRound (red * s);
          map->green += QRound (green * s);
          map->blue  += QRound (blue * s);
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

          l = map->red + QRound (red * s);
          map->red = l < 255 ? l : 255;
          l = map->green + QRound (green * s);
          map->green = l < 255 ? l : 255;
          l = map->blue + QRound (blue * s);
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
  static_lm.TransferTo (o_stat);
  real_lm.TransferTo (o_real);

  lm_size = lwidth * lheight;

  // Allocate new data and transform old to new.
  static_lm.SetLength (lm_size);
  ResizeMap2 (
    o_stat.GetArray (),
    oldw,
    oldh,
    static_lm.GetArray (),
    lwidth,
    lheight);

  real_lm.SetLength (lm_size);
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
    unsigned char *old_map = new unsigned char[smap->Length ()];
    memcpy (old_map, smap->GetArray (), smap->Length ());
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
    UpdateRealLightMap (0, 0, 0, false);
    CalcMeanLighting ();
    mean_recalc = false;
  }

  r = mean_color.red; 
  g = mean_color.green; 
  b = mean_color.blue; 
}
