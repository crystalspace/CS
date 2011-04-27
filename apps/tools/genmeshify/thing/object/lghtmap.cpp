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

#include "csgfx/packrgb.h"
#include "csutil/csendian.h"
#include "csutil/memfile.h"
#include "csutil/util.h"

#include "iengine/engine.h"
#include "iutil/cache.h"
#include "iutil/vfs.h"

#include "lghtmap.h"
#include "polygon.h"
#include "thing.h"

#define LMMAGIC	    "LM04" // must be 4 chars!


csShadowMap::csShadowMap ()
{
  Light = 0;
  max_shadow = 255;  // use worst case until calc'd
}

csShadowMap::~csShadowMap ()
{
}

void csShadowMap::Alloc (iLight *l)
{
  Light = l;
}

void csShadowMap::CalcMaxShadow (long lmsize)
{
  max_shadow=0;
  if (!map.IsValid()) return;
  const uint8* array = map->GetUint8();
  for (int i=0; i<lmsize; i++)
    if (array[i] > max_shadow) max_shadow = array[i];
}

//---------------------------------------------------------------------------

#include "csutil/custom_new_disable.h"

class ParasiticDataBufferBlockAllocated : public csParasiticDataBufferBase
{
  typedef csBlockAllocator<ParasiticDataBufferBlockAllocated> Allocator;
  CS_DECLARE_STATIC_CLASSVAR_REF(bufAlloc, BufAlloc, 
    Allocator);
public:
  void DecRef()
  {
    CS_ASSERT_MSG("Refcount decremented for destroyed object", 
      scfRefCount != 0);
    csRefTrackerAccess::TrackDecRef (GetSCFObject(), scfRefCount);
    scfRefCount--;
    if (scfRefCount == 0)
    {
      //scfRemoveRefOwners ();
      //if (scfParent) scfParent->DecRef();
      BufAlloc().Free (this);
    }
  }
  static ParasiticDataBufferBlockAllocated* Alloc (iDataBuffer* parent, 
    size_t offs, size_t size = (size_t)~0)
  {
    ParasiticDataBufferBlockAllocated* p = BufAlloc().Alloc();
    p->SetContents (parent, offs, size);
    return p;
  }
};

#include "csutil/custom_new_enable.h"

CS_IMPLEMENT_STATIC_CLASSVAR_REF(ParasiticDataBufferBlockAllocated, bufAlloc, 
  BufAlloc, ParasiticDataBufferBlockAllocated::Allocator, (1024));

//---------------------------------------------------------------------------
int csLightMap::lightcell_size = 16;
int csLightMap::lightcell_shift = 4;
int csLightMap::default_lightmap_cell_size = 16;

CS_IMPLEMENT_STATIC_CLASSVAR_REF(csLightMap, shadowMapAlloc, ShadowMapAlloc, 
  csLightMap::ShadowMapAllocator, (1024));

csLightMap::csLightMap ()
{
  first_smap = 0;
  // Use slowest safest method by default.
  max_static_color_values.Set (255,255,255);
}

csLightMap::~csLightMap ()
{
  while (first_smap)
  {
    csShadowMap *smap = first_smap->next;
    ShadowMapAlloc().Free (first_smap);
    first_smap = smap;
  }
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
  ShadowMapAlloc().Free (smap);
}

csShadowMap *csLightMap::NewShadowMap (iLight *light)
{
  csShadowMap *smap = ShadowMapAlloc().Alloc();
  smap->Light = light;
  smap->next = first_smap;
  first_smap = smap;

  smap->Alloc (light);

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

csShadowMap *csLightMap::GetShadowMap (size_t n)
{
  csShadowMap *smap = first_smap;
  while (smap && (n-- > 0))
  {
    smap = smap->next;
  }

  return smap;
}

void csLightMap::SetSize (int w, int h)
{
  lwidth = csLightMap::CalcLightMapWidth (w);
  lheight = csLightMap::CalcLightMapHeight (h);
}

void csLightMap::InitColor (int r, int g, int b)
{
  long lm_size = lwidth * lheight;
  csRGBcolor def (r, g, b);
  if (!staticLmBuffer.IsValid())
  {
    staticLmBuffer.AttachNew (
      new csDataBuffer (lm_size * sizeof (csRGBcolor)));
  }
  /* @@@ FIXME: if staticLmBuffer != 0, it may come from a file. We don't want
   * to write the colors to it then ... */
  csRGBcolor* static_lm = (csRGBcolor*)staticLmBuffer->GetData();
  for (int i = 0; i < lm_size; i++) static_lm[i] = def;
}

void csLightMap::Alloc (int w, int h)
{
  SetSize (w, h);
  staticLmBuffer = 0;
  max_static_color_values.Set (0, 0, 0);
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

// Copied from csendian.h to avoid deprecation warnings
static inline short FloatToShort (float f)
{
  int exp;
  long mant = csQroundSure (frexp (f, &exp) * 0x1000);
  long sign = mant & 0x8000;
  if (mant < 0) mant = -mant;
  if (exp > 7) mant = 0x7ff, exp = 7; else if (exp < -8) mant = 0, exp = -8;
  return short(sign | ((exp & 0xf) << 11) | (mant & 0x7ff));
}

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

  //csThing* parent = poly->GetParent ();

  SetSize (w, h);
  size_t lm_size = lwidth * lheight;

  memcpy (pswanted.header, LMMAGIC, 4);
  if (poly)
  {
    pswanted.x1 = FloatToShort (spoly->Vobj (0).x);
    pswanted.y1 = FloatToShort (spoly->Vobj (0).y);
    pswanted.z1 = FloatToShort (spoly->Vobj (0).z);
    pswanted.x2 = FloatToShort (spoly->Vobj (1).x);
    pswanted.y2 = FloatToShort (spoly->Vobj (1).y);
    pswanted.z2 = FloatToShort (spoly->Vobj (1).z);
  }
  pswanted.lm_size = (int32)lm_size;
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
  ps.x1 = csLittleEndian::Convert (ps.x1);
  ps.y1 = csLittleEndian::Convert (ps.y1);
  ps.z1 = csLittleEndian::Convert (ps.z1);
  ps.x2 = csLittleEndian::Convert (ps.x2);
  ps.y2 = csLittleEndian::Convert (ps.y2);
  ps.z2 = csLittleEndian::Convert (ps.z2);
  ps.lm_size = csLittleEndian::Convert (ps.lm_size);
  ps.lm_cnt = csLittleEndian::Convert (ps.lm_cnt);

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
    if (!file->SetPos (file->GetPos() + ps.lm_size*3))
      return error_buf;

    uint8 have_dyn;
    if (file->Read ((char*)&have_dyn, sizeof (have_dyn)) != sizeof (have_dyn))
      return error_buf;
    if (have_dyn)
    {
      if (file->Read ((char*)lh.header, 4) != 4)
        return error_buf;
      if (file->Read ((char*)&lh.dyn_cnt, 4) != 4)
        return error_buf;
      lh.dyn_cnt = csLittleEndian::Convert (lh.dyn_cnt);
      uint32 size;
      if (file->Read ((char*)&size, sizeof (size)) != sizeof (size))
        return error_buf;
      size = csLittleEndian::Convert (size);
      file->SetPos (file->GetPos() + size);
    }

    return error_buf;
  }

  //-------------------------------
  // The cached item is valid.
  //-------------------------------
  {
    csRef<iDataBuffer> lmDataBuffer = file->GetAllData();
    const size_t bufSize = lm_size * 3;
    lmDataBuffer.AttachNew (ParasiticDataBufferBlockAllocated::Alloc (
      lmDataBuffer, file->GetPos(), bufSize));
    if ((lmDataBuffer->GetSize() != bufSize)
      || (!file->SetPos (file->GetPos() + bufSize)))
      return "File too short while reading static lightmap data!";
    if (csPackRGB::IsRGBcolorSane())
      staticLmBuffer = lmDataBuffer;
    else
    {
      staticLmBuffer.AttachNew (
        new csDataBuffer (lm_size * sizeof (csRGBcolor)));
      csPackRGB::UnpackRGBtoRGBcolor ((csRGBcolor*)staticLmBuffer->GetData(),
        lmDataBuffer->GetUint8(), lm_size);
    }
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
  lh.dyn_cnt = csLittleEndian::Convert (lh.dyn_cnt);

  uint32 size;
  if (file->Read ((char*)&size, sizeof (size)) != sizeof (size))
    return "File too short at start of dynamic lightmaps!";

  // Calculate the expected size and see if it matches with the
  // size we still have in our data buffer. If it doesn't match
  // we don't load anything.
  size = csLittleEndian::Convert (size);

  unsigned int expected_size;
  expected_size = (uint)(lh.dyn_cnt * (sizeof (LightSave) + lm_size));
  if (expected_size != size)
  {
    // Skip the data in the cache so we can potentially proceed
    // to the next lightmap.
    file->SetPos (file->GetPos() + size);
    return "Mismatch with expected number of pseudo-dynamic lightmaps!";
  }

  //iLightingInfo* li;
  //li = (iLightingInfo*)parent;

  for (i = 0 ; i < lh.dyn_cnt ; i++)
  {
    if (file->Read ((char*)&ls.light_id, sizeof (LightSave))
    	!= sizeof (LightSave))
      return "File too short while reading pseudo-dynamic lightmap header!";
    size -= sizeof (LightSave);

    iLight *light = engine->FindLightID (ls.light_id);
    if (light)
    {
      csShadowMap *smap = NewShadowMap (light);

      //light->AddAffectedLightingInfo (li);

      csRef<iDataBuffer> smDataBuffer = file->GetAllData();
      smDataBuffer.AttachNew (ParasiticDataBufferBlockAllocated::Alloc (
        smDataBuffer, file->GetPos(), lm_size));
      if ((smDataBuffer->GetSize() != lm_size)
        || (!file->SetPos (file->GetPos() + lm_size)))
        return "File too short while reading pseudo-dynamic lightmap data!";
      size -= (uint32)lm_size;
      smap->map = smDataBuffer;
      smap->CalcMaxShadow ((long)lm_size);
    }
    else
    {
      // Skip the data in the cache so we can potentially proceed to the
      // next lightmap.
      file->SetPos (file->GetPos() + size);
      return "Couldn't find the pseudo-dynamic light for this lightmap!";
    }
  }

stop:
  return 0;
}

bool csLightMap::UpdateRealLightMap (float dyn_ambient_r,
                                     float dyn_ambient_g,
                                     float dyn_ambient_b,
                                     bool  dyn_dirty,
				     csLightingScratchBuffer& finalLM)
{
  if (!dyn_dirty) return false;

  csRGBcolor temp_max_color_values = max_static_color_values;
  csRGBcolor* static_lm = GetStaticMap();

  long lm_size = lwidth * lheight;
  finalLM.SetSize (lm_size);

  //---
  // First copy the static lightmap to the real lightmap.
  // Remember the real lightmap first so that we can see if
  // there were any changes.
  //---
  if (dyn_ambient_r || dyn_ambient_g || dyn_ambient_b)
  {
    csRGBcolor ambient;
    int dr = csQint (dyn_ambient_r * 128);
    int dg = csQint (dyn_ambient_g * 128);
    int db = csQint (dyn_ambient_b * 128);
    if (dr > 255) dr = 255;
    if (dg > 255) dg = 255;
    if (db > 255) db = 255;
    ambient.Set ((unsigned char)dr, (unsigned char)dg, (unsigned char)db);

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
        csRGBcolor t = max_static_color_values;
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
        csRGBcolor t = max_static_color_values;
	t.SafeAdd (ambient);
        for (int i=0; i<lm_size; i++) finalLM[i] = t;
      }
      temp_max_color_values.SafeAdd (ambient);
    }
  }
  else
  {
    if (static_lm)
      memcpy (finalLM.GetArray(), static_lm, lm_size * sizeof (csRGBcolor));
    else
      for (int i=0; i<lm_size; i++)
        finalLM[i] = max_static_color_values;
  }

  //---
  // Then add all pseudo-dynamic lights.
  //---
  iLight *light;
  csRGBcolor* map;
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
      p = smap->map->GetUint8();
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
  max_static_color_values.Set (0,0,0);

  csRGBcolor *map = GetStaticMap();
  if (!map) return;
  long lm_size = lwidth * lheight;
  csRGBcolor min_static_color_values;
  if (lm_size & 1)
  {
    min_static_color_values = *map;
    max_static_color_values = *map;
    lm_size--;
  }
  else
    min_static_color_values.Set (255,255,255);
  for (int i = 0; i < lm_size; i+=2)
  {
    const csRGBcolor& a = map[0];
    const csRGBcolor& b = map[1];

    if (a.red < b.red)
    {
      if (a.red < min_static_color_values.red)
        min_static_color_values.red = a.red;
      if (b.red > max_static_color_values.red)
        max_static_color_values.red = b.red;
    }
    else
    {
      if (a.red > max_static_color_values.red)
        max_static_color_values.red = a.red;
      if (b.red < min_static_color_values.red)
        min_static_color_values.red = b.red;
    }

    if (a.green < b.green)
    {
      if (a.green < min_static_color_values.green)
        min_static_color_values.green = a.green;
      if (b.green > max_static_color_values.green)
        max_static_color_values.green = b.green;
    }
    else
    {
      if (a.green > max_static_color_values.green)
        max_static_color_values.green = a.green;
      if (b.green < min_static_color_values.green)
        min_static_color_values.green = b.green;
    }

    if (a.blue < b.blue)
    {
      if (a.blue < min_static_color_values.blue)
        min_static_color_values.blue = a.blue;
      if (b.blue > max_static_color_values.blue)
        max_static_color_values.blue = b.blue;
    }
    else
    {
      if (a.blue > max_static_color_values.blue)
        max_static_color_values.blue = a.blue;
      if (b.blue < min_static_color_values.blue)
        min_static_color_values.blue = b.blue;
    }

    map+=2;
  }
  if (min_static_color_values.red < r) min_static_color_values.red = r;
  if (min_static_color_values.green < g) min_static_color_values.green = g;
  if (min_static_color_values.blue < b) min_static_color_values.blue = b;

#define THRESHOLD 2
  if (max_static_color_values.red - min_static_color_values.red <= THRESHOLD &&
      max_static_color_values.green - min_static_color_values.green <= THRESHOLD &&
      max_static_color_values.blue - min_static_color_values.blue <= THRESHOLD)
  {
    staticLmBuffer = 0;
  }
}

