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
#include "csutil/csendian.h"
#include "lghtmap.h"
#include "polygon.h"
#include "thing.h"
#include "csutil/util.h"
#include "csutil/debug.h"
#include "csutil/memfile.h"
#include "iutil/vfs.h"
#include "iutil/cache.h"

#define LMMAGIC	    "LM04" // must be 4 chars!


csShadowMap::csShadowMap ()
{
  Light = 0;
  max_shadow = 255;  // use worst case until calc'd
  array = 0;
}

csShadowMap::~csShadowMap ()
{
  delete[] array;
}

void csShadowMap::Alloc (iLight *l, int w, int h)
{
  Light = l;

  int lw = csLightMap::CalcLightMapWidth (w);
  int lh = csLightMap::CalcLightMapHeight (h);
  array = new unsigned char[lw * lh];
  memset (array, 0, lw*lh);
}

void csShadowMap::CalcMaxShadow (long lmsize)
{
  max_shadow=0;
  for (int i=0; i<lmsize; i++)
    if (array[i] > max_shadow) max_shadow = array[i];
}

//---------------------------------------------------------------------------
int csLightMap::lightcell_size = 16;
int csLightMap::lightcell_shift = 4;
int csLightMap::default_lightmap_cell_size = 16;

csLightMap::csLightMap ()
{
  first_smap = 0;
  // Use slowest safest method by default.
  max_static_color_values.Set (255,255,255);
  static_lm = 0;
}

csLightMap::~csLightMap ()
{
  while (first_smap)
  {
    csShadowMap *smap = first_smap->next;
    delete first_smap;
    first_smap = smap;
  }

  delete[] static_lm;
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
  lwidth = csLightMap::CalcLightMapWidth (w);
  lheight = csLightMap::CalcLightMapHeight (h);
}

void csLightMap::InitColor (int r, int g, int b)
{
  long lm_size = lwidth * lheight;
  csRGBpixel def (r, g, b);
  // don't know why, but the previous implementation did this:
  def.alpha = 128;
  int i;
  for (i = 0; i < lm_size; i++) static_lm[i] = def;
}

void csLightMap::Alloc (int w, int h)
{
  SetSize (w, h);
  delete[] static_lm;

  long lm_size = lwidth * lheight;
  static_lm = new csRGBpixel [lm_size];
}

void csLightMap::ReAlloc ()
{
  if (static_lm) return;
  long lm_size = lwidth * lheight;
  static_lm = new csRGBpixel [lm_size];
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

// This really should be inside csLightMap::ReadFromCache, but Cygwin
// crashes on exit if functions have local static variables with complex types
static csString error_buf;

const char* csLightMap::ReadFromCache (
  iFile* file,
  int w,
  int h,
  csPolygon3D* poly,
  csPolygon3DStatic* spoly,
  iEngine *engine)
{
  PolySave ps, pswanted;
  LightHeader lh;
  LightSave ls;
  int i;

  csThing* parent = poly->GetParent ();

  SetSize (w, h);
  long lm_size = lwidth * lheight;

  strcpy (pswanted.header, LMMAGIC);
  if (poly)
  {
    pswanted.x1 = csFloatToShort (spoly->Vobj (0).x);
    pswanted.y1 = csFloatToShort (spoly->Vobj (0).y);
    pswanted.z1 = csFloatToShort (spoly->Vobj (0).z);
    pswanted.x2 = csFloatToShort (spoly->Vobj (1).x);
    pswanted.y2 = csFloatToShort (spoly->Vobj (1).y);
    pswanted.z2 = csFloatToShort (spoly->Vobj (1).z);
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
  ps.x1 = csConvertEndian (ps.x1);
  ps.y1 = csConvertEndian (ps.y1);
  ps.z1 = csConvertEndian (ps.z1);
  ps.x2 = csConvertEndian (ps.x2);
  ps.y2 = csConvertEndian (ps.y2);
  ps.z2 = csConvertEndian (ps.z2);
  ps.lm_size = csConvertEndian (ps.lm_size);
  ps.lm_cnt = csConvertEndian (ps.lm_cnt);

  //-------------------------------
  // From this point on we will try to skip the wrong lightmap
  // so that we can continue processing further lightmaps.
  //-------------------------------

  //-------------------------------
  // Check if cached item is still valid.
  //-------------------------------
  error_buf.Empty();
  if (strncmp (ps.header, pswanted.header, 4) != 0)
    error_buf = "Cached lightmap header doesn't match!";
  else if (poly)
  {
    if (ps.lm_cnt != pswanted.lm_cnt)
      error_buf.Format (
      	"Cached lightmap header mismatch (got cnt=%" PRId32 ", expected %" PRId32 ")!",
	ps.lm_cnt, pswanted.lm_cnt);
    else if (ps.lm_size != pswanted.lm_size)
      error_buf.Format (
      	"Cached lightmap base texture mismatch (got size=%" PRId32 ", expected %" PRId32 ")!",
	ps.lm_size, pswanted.lm_size);
    else if (ps.x1 != pswanted.x1 || ps.y1 != pswanted.y1
    		|| ps.z1 != pswanted.z1)
      error_buf = "Cached lightmap first vertex mismatch!";
    else if (ps.x2 != pswanted.x2 || ps.y2 != pswanted.y2
    		|| ps.z2 != pswanted.z2)
      error_buf = "Cached lightmap second vertex mismatch!";
  }
  if (!error_buf.IsEmpty())
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
      lh.dyn_cnt = csConvertEndian (lh.dyn_cnt);
      uint32 size;
      if (file->Read ((char*)&size, sizeof (size)) != sizeof (size))
        return error_buf;
      size = csConvertEndian (size);
      data = new char[size];
      file->Read (data, size);
      delete[] data;
    }

    return error_buf;
  }

  //-------------------------------
  // The cached item is valid.
  //-------------------------------
  delete[] static_lm;
  static_lm = new csRGBpixel[lm_size];

  int n = lm_size;
  char *lm_ptr = (char*)static_lm;
  while (--n >= 0) 
  {
    if (file->Read ((char*)lm_ptr, 3) != 3)
      return "File too short while reading static lightmap data!";
    lm_ptr += 3;
    *(lm_ptr++) = (char)-127;
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
  lh.dyn_cnt = csConvertEndian (lh.dyn_cnt);

  uint32 size;
  if (file->Read ((char*)&size, sizeof (size)) != sizeof (size))
    return "File too short at start of dynamic lightmaps!";

  // Calculate the expected size and see if it matches with the
  // size we still have in our data buffer. If it doesn't match
  // we don't load anything.
  size = csConvertEndian (size);

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
  li = (iLightingInfo*)parent;

  for (i = 0 ; i < lh.dyn_cnt ; i++)
  {
    if (file->Read ((char*)&ls.light_id, sizeof (LightSave))
    	!= sizeof (LightSave))
      return "File too short while reading pseudo-dynamic lightmap header!";
    size -= sizeof (LightSave);

    iLight *light = engine->FindLightID (ls.light_id);
    if (light)
    {
      csShadowMap *smap = NewShadowMap (light, w, h);

      light->AddAffectedLightingInfo (li);

      if ((long) file->Read ((char*)(smap->array), lm_size) != lm_size)
        return "File too short while reading pseudo-dynamic lightmap data!";
      size -= lm_size;
      smap->CalcMaxShadow (lm_size);
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
  return 0;
}

void csLightMap::Cache (
  iFile* file,
  csPolygon3D *poly,
  csPolygon3DStatic *spoly,
  iEngine *engine)
{
  (void)engine;

  PolySave ps;

  strcpy (ps.header, LMMAGIC);
  if (poly)
  {
    ps.x1 = csConvertEndian (csFloatToShort (spoly->Vobj (0).x));
    ps.y1 = csConvertEndian (csFloatToShort (spoly->Vobj (0).y));
    ps.z1 = csConvertEndian (csFloatToShort (spoly->Vobj (0).z));
    ps.x2 = csConvertEndian (csFloatToShort (spoly->Vobj (1).x));
    ps.y2 = csConvertEndian (csFloatToShort (spoly->Vobj (1).y));
    ps.z2 = csConvertEndian (csFloatToShort (spoly->Vobj (1).z));
  }

  if (file->Write ("lmpn", 4) != 4)
    return;

  long lm_size = lwidth * lheight;
  ps.lm_size = csConvertEndian ((int32)lm_size);
  ps.lm_cnt = 111;          // Dummy!
  ps.lm_cnt = csConvertEndian (ps.lm_cnt);

  //-------------------------------
  // Write the normal lightmap data.
  //-------------------------------
  file->Write ((char*)&ps, sizeof (ps));

  int n = lm_size;
  char* lm_ptr = (char*)static_lm;
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
    uint32 l = csConvertEndian (lh.dyn_cnt);
    file->Write ((char *) &l, 4);

    // unsigned long == ls.light_id.
    uint32 size = lh.dyn_cnt * (sizeof (LightSave) + lm_size);
    uint32 s = csConvertEndian (size);
    file->Write ((char*)&s, sizeof (s));

    while (smap)
    {
      iLight *light = smap->Light;
      if (smap->array)
      {
        LightSave ls;
	memcpy (ls.light_id, light->GetLightID (), sizeof (LightSave));
        file->Write ((char *) &ls.light_id, sizeof (LightSave));
        file->Write ((char *)(smap->array), lm_size);
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
                                     bool  dyn_dirty,
				     csLightingScratchBuffer& finalLM)
{
  if (!dyn_dirty) return false;

  csRGBpixel temp_max_color_values = max_static_color_values;

  long lm_size = lwidth * lheight;
  finalLM.SetLength (lm_size);

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

    if (max_static_color_values.red   + ambient.red   <= 255  &&
        max_static_color_values.green + ambient.green <= 255  &&
        max_static_color_values.blue  + ambient.blue  <= 255)
    {
      // No lightmap overflows so we can use fastest loop with no checking.
      if (static_lm)
      {
        for (int i=0; i<lm_size; i++)
        {
          finalLM[i] = static_lm[i];
          finalLM[i].UnsafeAdd (ambient);
        }
      }
      else
      {
        csRGBpixel t = max_static_color_values;
	t.UnsafeAdd (ambient);
        for (int i=0; i<lm_size; i++) finalLM[i] = t;
      }
      temp_max_color_values.UnsafeAdd (ambient);
    }
    else
    {
      // An overflow is somewhere here, so check each and every addition.
      if (static_lm)
      {
        for (int i=0; i<lm_size; i++)
        {
          finalLM[i] = static_lm[i];
	  finalLM[i].SafeAdd (ambient);
        }
      }
      else
      {
        csRGBpixel t = max_static_color_values;
	t.SafeAdd (ambient);
        for (int i=0; i<lm_size; i++) finalLM[i] = t;
      }
      temp_max_color_values.SafeAdd (ambient);
    }
  }
  else
  {
    if (static_lm)
      memcpy (finalLM.GetArray(), static_lm, 4 * lm_size);
    else
      for (int i=0; i<lm_size; i++)
        finalLM[i] = max_static_color_values;
  }

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
      map = finalLM.GetArray();
      light = smap->Light;
      red = light->GetColor ().red;
      green = light->GetColor ().green;
      blue = light->GetColor ().blue;
      p = smap->array;
      last_p = p + lm_size;

      int tm_r = temp_max_color_values.red   + csQint (smap->max_shadow * red);
      int tm_g = temp_max_color_values.green + csQint (smap->max_shadow * green);
      int tm_b = temp_max_color_values.blue  + csQint (smap->max_shadow * blue);

      if (tm_r <= 255  && tm_g <= 255  && tm_b <= 255)
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

void csLightMap::CalcMaxStatic (int r, int g, int b)
{
  csRGBpixel min_static_color_values (255,255,255);
  max_static_color_values.Set (0,0,0);

  csRGBpixel *map = static_lm;
  long lm_size = lwidth * lheight;
  for (int i = 0; i < lm_size; i++)
  {
    if (max_static_color_values.red < map->red)
        max_static_color_values.red = map->red;
    if (max_static_color_values.green < map->green)
        max_static_color_values.green = map->green;
    if (max_static_color_values.blue < map->blue)
        max_static_color_values.blue = map->blue;

    if (min_static_color_values.red > map->red)
        min_static_color_values.red = map->red;
    if (min_static_color_values.green > map->green)
        min_static_color_values.green = map->green;
    if (min_static_color_values.blue > map->blue)
        min_static_color_values.blue = map->blue;

    map++;
  }
  if (min_static_color_values.red < r) min_static_color_values.red = r;
  if (min_static_color_values.green < g) min_static_color_values.green = g;
  if (min_static_color_values.blue < b) min_static_color_values.blue = b;

#define THRESHOLD 2
  if (max_static_color_values.red - min_static_color_values.red <= THRESHOLD &&
      max_static_color_values.green - min_static_color_values.green <= THRESHOLD &&
      max_static_color_values.blue - min_static_color_values.blue <= THRESHOLD)
  {
    // Optimize!
    delete[] static_lm;
    static_lm = 0;
  }
}

