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

#include "sysdef.h"
#include "cssys/common/csendian.h"
#include "csengine/light/lghtmap.h"
#include "csengine/polygon/polygon.h"
#include "csengine/sector.h"
#include "csengine/objects/thing.h"
#include "csengine/light/light.h"
#include "csengine/world.h"
#include "csutil/archive.h"
#include "csobject/nameobj.h"

//---------------------------------------------------------------------------

csShadowMap::csShadowMap ()
{
  map = NULL;
}

csShadowMap::~csShadowMap ()
{
  CHK (delete [] map);
}

void csShadowMap::Alloc (csLight*, int w, int h, int lms)
{
  int i;
  CHK (delete [] map); map = NULL;

  int lw = w/lms+2;
  int lh = h/lms+2;
  long lm_size = lw*lh;
  CHK (map = new unsigned char [lm_size]);
  for (i = 0 ; i < lm_size ; i++) map[i] = 0;
}

void csShadowMap::MipmapLightmap (int w, int h, int lms, csShadowMap* source, int w2, int h2, int lms2)
{
  int size = w*h;

  int lw = w/lms+2;
  int lh = h/lms+2;
  int lw2 = w2/lms2+2;
  int lh2 = h2/lms2+2;
  int u, v, uv, uv2;

  uv2 = 0;
  for (v = 0 ; v < lh ; v++)
    for (u = 0 ; u < lw ; u++)
    {
      uv = v*lw + u;

      if (u+u >= lw2 || v+v >= lh2)
      {
	if (u+u >= lw2) uv2 = (v+v)*lw2 + lw2-1;
	else if (v+v >= lh2) uv2 = (lh2-1)*lw2 + u+u;
      }
      else uv2 = (v+v)*lw2 + u+u;

      map[uv] = source->map[uv2];
    }

  light = source->light;
}

void csShadowMap::CopyLightmap (csShadowMap* source, int size)
{
  if (map) CHKB (delete [] map);
  CHK (map = new unsigned char [size]);
  memcpy (map, source->map, size);
  light = source->light;
}

//---------------------------------------------------------------------------

IMPLEMENT_UNKNOWN_NODELETE( csLightMap )

BEGIN_INTERFACE_TABLE( csLightMap )
	IMPLEMENTS_COMPOSITE_INTERFACE( LightMap )
END_INTERFACE_TABLE()

csLightMap::csLightMap ()
{
  first_smap = NULL;
  hicolorcache = NULL;
  in_memory = false;
}

csLightMap::~csLightMap ()
{
  while (first_smap)
  {
    csShadowMap* smap = first_smap->next;
    CHK (delete first_smap);
    first_smap = smap;
  }
}

void csLightMap::DelShadowMap (csShadowMap* smap)
{
  first_smap = smap->next;
  CHK (delete smap);
}

csShadowMap* csLightMap::NewShadowMap (csLight* light, int w, int h, int lms)
{
  CHK (csShadowMap* smap = new csShadowMap ());
  smap->light = light;
  smap->next = first_smap;
  first_smap = smap;

  smap->Alloc (light, w, h, lms);

  return smap;
}

csShadowMap* csLightMap::FindShadowMap (csLight* light)
{
  csShadowMap* smap = first_smap;
  while (smap)
  {
    if (smap->light == light) return smap;
    smap = smap->next;
  }
  return NULL;
}

// finds nearest number that is a power of two
int find_nearest_po2 (int n)
{
  int w=1;

  while (n > w)  w <<= 1;

  return w;
}


// returns true if n is a power of two
bool is_po2 (int n)
{
  int i;
  int po2 = 1;

  if (!n) return false; 
    
  for (i=0; i<32; i++, po2<<=1)
    if (n==po2) return true;

  return false;
}

void csLightMap::SetSize (int w, int h, int lms)
{
  size = w*h;
  lwidth = w/lms+2;
  lheight = h/lms+2;
  lm_size = lwidth*lheight;
  rwidth = lwidth;
  rheight = lheight;
}

void csLightMap::Alloc (int w, int h, int lms, int r, int g, int b)
{
  SetSize (w, h, lms);
  int i;
  static_lm.Clear ();
  real_lm.Clear ();

//@@@
#if 0
  if (Textures::mixing == MIX_NOCOLOR)
  {
    static_lm.AllocRed (lm_size);
    real_lm.AllocRed (lm_size);
  }
  else
#endif
  {
    static_lm.AllocRed (lm_size);
    static_lm.AllocGreen (lm_size);
    static_lm.AllocBlue (lm_size);
    real_lm.AllocRed (lm_size);
    real_lm.AllocGreen (lm_size);
    real_lm.AllocBlue (lm_size);
  }

  for (i = 0 ; i < lm_size ; i++)
  {
    static_lm.mapR[i] = r;
    if (static_lm.mapG) static_lm.mapG[i] = g;
    if (static_lm.mapB) static_lm.mapB[i] = b;
  }
}

void csLightMap::MipmapLightmap (int w, int h, int lms, csLightMap* source, int w2, int h2, int lms2)
{
  Alloc (w, h, lms, 0, 0, 0);

  int lw2 = source->GetWidth ();
  int lh2 = source->GetHeight ();
  int u, v, uv, uv2;

  uv2 = 0;
  for (v = 0 ; v < lheight ; v++)
    for (u = 0 ; u < lwidth ; u++)
    {
      uv = v*lwidth + u;

      if (u+u >= lw2 || v+v >= lh2)
      {
	if (u+u >= lw2) uv2 = (v+v)*lw2 + lw2-1;
	else if (v+v >= lh2) uv2 = (lh2-1)*lw2 + u+u;
      }
      else uv2 = (v+v)*lw2 + u+u;

      static_lm.mapR[uv] = source->static_lm.mapR[uv2];
      if (static_lm.mapG) static_lm.mapG[uv] = source->static_lm.mapG[uv2];
      if (static_lm.mapB) static_lm.mapB[uv] = source->static_lm.mapB[uv2];
    }

  csShadowMap* smap, * smap2;
  while (first_smap)
  {
    smap = first_smap->next;
    CHK (delete first_smap);
    first_smap = smap;
  }

  smap = source->first_smap;
  while (smap)
  {
    smap2 = NewShadowMap (smap->light, w, h, lms);
    smap2->MipmapLightmap (w, h, lms, smap, w2, h2, lms2);
    smap = smap->next;
  }
}

void csLightMap::CopyLightmap (csLightMap* source)
{
  lm_size = source->lm_size;
  size = source->size;
  static_lm.Copy (source->static_lm, lm_size);
  real_lm.Copy (source->real_lm, lm_size);
  lwidth = source->lwidth;
  lheight = source->lheight;
  rwidth = source->rwidth;
  rheight = source->rheight;

  csShadowMap* smap, * smap2;
  while (first_smap)
  {
    smap = first_smap->next;
    CHK (delete first_smap);
    first_smap = smap;
  }

  smap = source->first_smap;
  while (smap)
  {
    CHK (smap2 = new csShadowMap ());
    smap2->next = first_smap;
    first_smap = smap2;
    smap2->CopyLightmap (smap, lm_size);
    smap = smap->next;
  }
}

struct PolySave
{
  char header[4];
  float x1, y1, z1;	// Coordinate of vertex 1
  float x2, y2, z2;	// Coordinate of vertex 2
  long lm_size;		// Size of lightmap
  long lm_cnt;		// Number of non-dynamic lightmaps (normally 3)
};
#define POLYSAVE_LEN (4*9)

struct LightSave
{
  float x, y, z, dist;
};
#define LIGHTSAVE_LEN (4*4)

struct LightHeader
{
  char header[4];
  long dyn_cnt;		// Number of dynamic maps
};
#define LIGHTHDR_LEN (4*2)

void CacheName (char* buf, csPolygonSet* owner, int index, char* suffix)
{
  if (owner->GetType () == csThing::Type())
    sprintf (buf, "LM_%s_%s_%d%s",
    	csNameObject::GetName(*(owner->GetSector())),
    	csNameObject::GetName(*owner),
	index,
	suffix);
  else
    sprintf (buf, "LM_%s_%d%s",
    	csNameObject::GetName(*owner),
	index,
	suffix);
}

bool csLightMap::ReadFromCache (int w, int h, int lms, csPolygonSet* owner, csPolygon3D* poly, int index, csWorld* world)
{
  char buf[200];
  PolySave ps;
  LightHeader lh;
  LightSave ls;
  csLight* light;
  int i;
  Archive* ar = world->GetWorldFile ();

  SetSize (w, h, lms);

  CacheName (buf, owner, index, "");

  char* data = ar->read (buf);
  if (!data) return false;

  char* d = data;
  memcpy (ps.header, d, 4); d += 4;
  memcpy (&ps.x1, d, 4); d += 4;
  memcpy (&ps.y1, d, 4); d += 4;
  memcpy (&ps.z1, d, 4); d += 4;
  memcpy (&ps.x2, d, 4); d += 4;
  memcpy (&ps.y2, d, 4); d += 4;
  memcpy (&ps.z2, d, 4); d += 4;
  memcpy (&ps.lm_size, d, 4); d += 4;
  memcpy (&ps.lm_cnt, d, 4); d += 4;
  ps.x1 = convert_endian (ps.x1);
  ps.y1 = convert_endian (ps.y1);
  ps.z1 = convert_endian (ps.z1);
  ps.x2 = convert_endian (ps.x2);
  ps.y2 = convert_endian (ps.y2);
  ps.z2 = convert_endian (ps.z2);
  ps.lm_size = convert_endian (ps.lm_size);
  ps.lm_cnt = convert_endian (ps.lm_cnt);

  //-------------------------------
  // Check if cached item is still valid.
  //-------------------------------
  if (  poly &&
       (ABS (ps.x1 - poly->Vobj (0).x) > EPSILON ||
  	ABS (ps.y1 - poly->Vobj (0).y) > EPSILON ||
  	ABS (ps.z1 - poly->Vobj (0).z) > EPSILON ||
  	ABS (ps.x2 - poly->Vobj (1).x) > EPSILON ||
  	ABS (ps.y2 - poly->Vobj (1).y) > EPSILON ||
  	ABS (ps.z2 - poly->Vobj (1).z) > EPSILON ||
	ps.lm_size != lm_size))
  {
    // Invalid.
    CHK (delete [] data);
    return false;
  }

  //-------------------------------
  // The cached item is valid.
  //-------------------------------
  static_lm.Clear ();

  static_lm.AllocRed (lm_size);
  memcpy (static_lm.mapR, d, lm_size);
  d += lm_size;

//@@@
  //if (Textures::mixing != MIX_NOCOLOR)
  {
    static_lm.AllocGreen (lm_size);
    memcpy (static_lm.mapG, d, lm_size);
    d += lm_size;
    static_lm.AllocBlue (lm_size);
    memcpy (static_lm.mapB, d, lm_size);
  }

  CHK (delete [] data);

  //-------------------------------
  // Now load the dynamic data.
  //-------------------------------
  CacheName (buf, owner, index, "_d");
  data = ar->read (buf);
  if (!data) return true;	// No dynamic data. @@@ Recalculate dynamic data?

  d = data;
  memcpy (lh.header, d, 4); d += 4;
  memcpy (&lh.dyn_cnt, d, 4); d += 4;
  lh.dyn_cnt = convert_endian (lh.dyn_cnt);

  for (i = 0 ; i < lh.dyn_cnt ; i++)
  {
    memcpy (&ls.x, d, 4); d += 4;
    memcpy (&ls.y, d, 4); d += 4;
    memcpy (&ls.z, d, 4); d += 4;
    memcpy (&ls.dist, d, 4); d += 4;
    ls.x = convert_endian (ls.x);
    ls.y = convert_endian (ls.y);
    ls.z = convert_endian (ls.z);
    ls.dist = convert_endian (ls.dist);

    light = world->FindLight (ls.x, ls.y, ls.z, ls.dist);
    if (light->GetType () == csStatLight::Type() && poly)
    {
      csStatLight* slight = (csStatLight*)light;
      slight->RegisterPolygon (poly);
    }
    csShadowMap* smap = NewShadowMap (light, w, h, lms);
    memcpy (smap->map, d, lm_size);
    d += lm_size;
  }

  CHK (delete [] data);

  return true;
}

void csLightMap::Cache (csPolygonSet* owner, csPolygon3D* poly, int index, csWorld* world)
{
  char buf[200];
  PolySave ps;
  void* entry;
  long l;
  float f;
  Archive* ar = world->GetWorldFile ();

  strcpy (ps.header, "MAPL");
  if (poly)
  {
    ps.x1 = poly->Vobj (0).x;
    ps.y1 = poly->Vobj (0).y;
    ps.z1 = poly->Vobj (0).z;
    ps.x2 = poly->Vobj (1).x;
    ps.y2 = poly->Vobj (1).y;
    ps.z2 = poly->Vobj (1).z;
  }
  ps.lm_size = lm_size;
  ps.lm_cnt = 0;
  if (static_lm.mapR) ps.lm_cnt++;
  if (static_lm.mapG) ps.lm_cnt++;
  if (static_lm.mapB) ps.lm_cnt++;

  //-------------------------------
  // Write the normal lightmap data.
  //-------------------------------
  CacheName (buf, owner, index, "");
  entry = ar->new_file (buf, POLYSAVE_LEN+lm_size*3);
  ar->write (entry, ps.header, 4);
  f = convert_endian (ps.x1); ar->append (entry, (char*)&f, 4);
  f = convert_endian (ps.y1); ar->append (entry, (char*)&f, 4);
  f = convert_endian (ps.z1); ar->append (entry, (char*)&f, 4);
  f = convert_endian (ps.x2); ar->append (entry, (char*)&f, 4);
  f = convert_endian (ps.y2); ar->append (entry, (char*)&f, 4);
  f = convert_endian (ps.z2); ar->append (entry, (char*)&f, 4);
  l = convert_endian (ps.lm_size); ar->append (entry, (char*)&l, 4);
  l = convert_endian (ps.lm_cnt); ar->append (entry, (char*)&l, 4);

  if (static_lm.mapR) ar->append (entry, (char*)static_lm.mapR, lm_size);
  if (static_lm.mapG) ar->append (entry, (char*)static_lm.mapG, lm_size);
  if (static_lm.mapB) ar->append (entry, (char*)static_lm.mapB, lm_size);

  //-------------------------------
  // Write the dynamic data.
  //-------------------------------
  LightHeader lh;

  csShadowMap* smap = first_smap;
  if (smap)
  {
    strcpy (lh.header, "DYNL");
    lh.dyn_cnt = 0;
    while (smap) { lh.dyn_cnt++; smap = smap->next; }
    smap = first_smap;

    CacheName (buf, owner, index, "_d");
    entry = ar->new_file (buf, LIGHTHDR_LEN+lh.dyn_cnt*(LIGHTSAVE_LEN+lm_size));
    ar->write (entry, lh.header, 4);
    l = convert_endian (lh.dyn_cnt); ar->append (entry, (char*)&l, 4);
  }
  while (smap)
  {
    csLight* light = smap->light;
    if (smap->map)
    {
      LightSave ls;
      ls.x = light->GetCenter ().x;
      ls.y = light->GetCenter ().y;
      ls.z = light->GetCenter ().z;
      ls.dist = light->GetRadius ();
      f = convert_endian (ls.x); ar->append (entry, (char*)&f, 4);
      f = convert_endian (ls.y); ar->append (entry, (char*)&f, 4);
      f = convert_endian (ls.z); ar->append (entry, (char*)&f, 4);
      f = convert_endian (ls.dist); ar->append (entry, (char*)&f, 4);
      ar->append (entry, (char*)(smap->map), lm_size);
    }
    smap = smap->next;
  }
}

void csLightMap::ConvertToMixingMode ()
{
  int i;
  mean_r = 0;
  mean_g = 0;
  mean_b = 0;
  for (i = 0 ; i < lm_size ; i++)
  {
    mean_r += static_lm.mapR[i];
    mean_g += static_lm.mapG[i];
    mean_b += static_lm.mapB[i];
  }
  mean_r /= lm_size;
  mean_g /= lm_size;
  mean_b /= lm_size;

  //@@@
  //if (Textures::mixing == MIX_TRUE_RGB) return;
  //else convert_to_mixing_mode (static_lm.mapR, static_lm.mapG, static_lm.mapB, lm_size);
}

void csLightMap::ConvertToMixingMode (unsigned char* mr, unsigned char* mg,
				       unsigned char* mb, int sz)
{
  if (!mr || !mg || !mb) return; // If we have a dynamic light there is no conversion needed

  while (sz > 0)
  {
    csLight::CorrectForNocolor (mr, mg, mb);
    mr++;
    mg++;
    mb++;
    sz--;
  }
}

// Only works for expanding a map.
void ResizeMap (unsigned char* old_map, int oldw, int oldh,
		 unsigned char* new_map, int neww, int /*newh*/)
{
  int row;
  for (row = 0 ; row < oldh ; row++)
    memcpy (new_map+neww*row, old_map+oldw*row, oldw);
}

void csLightMap::ConvertFor3dDriver (bool requirePO2, int maxAspect)
{
  if (!requirePO2) return; // Nothing to do.
  int oldw = lwidth, oldh = lheight;
  lwidth = find_nearest_po2 (lwidth);
  lheight = find_nearest_po2 (lheight);
  while (lwidth/lheight > maxAspect) lheight += lheight;
  while (lheight/lwidth > maxAspect) lwidth += lwidth;
  if (oldw == lwidth && oldh == lheight) return;	// Already ok, nothing to do.

  // Move the old data to o_stat and o_real.
  csRGBLightMap o_stat, o_real;
  o_stat.mapR = static_lm.mapR; static_lm.mapR = NULL;
  o_stat.mapG = static_lm.mapG; static_lm.mapG = NULL;
  o_stat.mapB = static_lm.mapB; static_lm.mapB = NULL;
  o_real.mapR = real_lm.mapR; real_lm.mapR = NULL;
  o_real.mapG = real_lm.mapG; real_lm.mapG = NULL;
  o_real.mapB = real_lm.mapB; real_lm.mapB = NULL;

  lm_size = lwidth*lheight;
 
  // Allocate new data and transform old to new.
//@@@
#if 0
  if (Textures::mixing == MIX_NOCOLOR)
  {
    static_lm.AllocRed (lm_size);
    ResizeMap (o_stat.mapR, oldw, oldh, static_lm.mapR, lwidth, lheight);
    real_lm.AllocRed (lm_size);
    ResizeMap (o_real.mapR, oldw, oldh, real_lm.mapR, lwidth, lheight);
  }
  else
#endif
  {
    static_lm.AllocRed (lm_size);
    ResizeMap (o_stat.mapR, oldw, oldh, static_lm.mapR, lwidth, lheight);
    static_lm.AllocGreen (lm_size);
    ResizeMap (o_stat.mapG, oldw, oldh, static_lm.mapG, lwidth, lheight);
    static_lm.AllocBlue (lm_size);
    ResizeMap (o_stat.mapB, oldw, oldh, static_lm.mapB, lwidth, lheight);

    real_lm.AllocRed (lm_size);
    ResizeMap (o_real.mapR, oldw, oldh, real_lm.mapR, lwidth, lheight);
    real_lm.AllocGreen (lm_size);
    ResizeMap (o_real.mapG, oldw, oldh, real_lm.mapG, lwidth, lheight);
    real_lm.AllocBlue (lm_size);
    ResizeMap (o_real.mapB, oldw, oldh, real_lm.mapB, lwidth, lheight);
  }

  // Convert all shadowmaps.
  csShadowMap* smap = first_smap;
  while (smap)
  {
    unsigned char* old_map = smap->map;
    CHK (smap->map = new unsigned char [lm_size]);
    ResizeMap (old_map, oldw, oldh, smap->map, lwidth, lheight);
    CHK (delete [] old_map);
    smap = smap->next;
  }
}

//---------------------------------------------------------------------------
