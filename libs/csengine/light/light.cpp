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
#include "csgeom/frustrum.h"
#include "csengine/quadcube.h"
#include "csengine/sysitf.h"
#include "csengine/light.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/polygon.h"
#include "csengine/cssprite.h"
#include "csengine/world.h"
#include "csengine/lppool.h"

//---------------------------------------------------------------------------

int csLight::ambient_red = DEFAULT_LIGHT_LEVEL;
int csLight::ambient_green = DEFAULT_LIGHT_LEVEL;
int csLight::ambient_blue = DEFAULT_LIGHT_LEVEL;
int csLight::ambient_white = DEFAULT_LIGHT_LEVEL;

IMPLEMENT_CSOBJTYPE (csLight,csObject);

csLight::csLight (float x, float y, float z, float d,
	      float red, float green, float blue) : csObject()
{
  center.x = x;
  center.y = y;
  center.z = z;

  dist = d;
  sqdist = d * d;
  inv_dist = 1 / d;

  color.red = red;
  color.green = green;
  color.blue = blue;
  flags = 0;

  halo_intensity = 0.0f;
  halo_max_intensity = 1.0f;
  in_halo_queue = false;

  attenuation = CS_ATTN_LINEAR;
}

csLight::~csLight ()
{
}

float csLight::GetBrightnessAtDistance (float d)
{
  switch (attenuation)
  {
    case CS_ATTN_NONE      : return 1;
    case CS_ATTN_LINEAR    : return 1 - d * inv_dist;
    case CS_ATTN_INVERSE   : return 1 / d;
    case CS_ATTN_REALISTIC : return 1 / (d*d);
  }
  return 0;
}

void csLight::CorrectForNocolor (unsigned char* rp, unsigned char* gp,
					unsigned char* bp)
{
(void)rp; (void)gp; (void)bp;
//@@@
#if 0
  if (Textures::mixing == MIX_TRUE_RGB)
    return;
  else if (Textures::mixing == MIX_NOCOLOR)
  {
    unsigned char w = (*rp+*gp+*bp)/3;
    *rp = w;
    *gp = w;
    *bp = w;
  }
  else
  {
    int r = *rp;
    int g = *gp;
    int b = *bp;
    int w = r;
    if (g < w) w = g;
    if (b < w) w = b;
    r -= w;
    g -= w;
    b -= w;

    *rp = w;

    if (Textures::color_table2 == TABLE_RED) *gp = r;
    else if (Textures::color_table2 == TABLE_GREEN) *gp = g;
    else if (Textures::color_table2 == TABLE_BLUE) *gp = b;

    if (Textures::color_table3 == TABLE_RED) *bp = r;
    else if (Textures::color_table3 == TABLE_GREEN) *bp = g;
    else if (Textures::color_table3 == TABLE_BLUE) *bp = b;
  }
#endif
}

void csLight::CorrectForNocolor (float* rp, float* gp, float* bp)
{
(void)rp; (void)gp; (void)bp;
//@@@
#if 0
  if (Textures::mixing == MIX_TRUE_RGB)
    return;
  else if (Textures::mixing == MIX_NOCOLOR)
  {
    float w = (*rp+*gp+*bp)/3;
    *rp = w;
    *gp = w;
    *bp = w;
  }
  else
  {
    float r = *rp;
    float g = *gp;
    float b = *bp;
    float w = r;
    if (g < w) w = g;
    if (b < w) w = b;
    r -= w;
    g -= w;
    b -= w;

    *rp = w;

    if (Textures::color_table2 == TABLE_RED) *gp = r;
    else if (Textures::color_table2 == TABLE_GREEN) *gp = g;
    else if (Textures::color_table2 == TABLE_BLUE) *gp = b;

    if (Textures::color_table3 == TABLE_RED) *bp = r;
    else if (Textures::color_table3 == TABLE_GREEN) *bp = g;
    else if (Textures::color_table3 == TABLE_BLUE) *bp = b;
  }
#endif
}

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csStatLight,csLight);

csStatLight::csStatLight (float x, float y, float z, float dist,
	      float red, float green, float blue, bool dynamic)
	      : csLight (x, y, z, dist, red, green, blue)
{
  csStatLight::dynamic = dynamic;
  polygons = NULL;
  flags = CS_LIGHT_THINGSHADOWS;
}

csStatLight::~csStatLight ()
{
  CHK (delete [] polygons);
}

void csStatLight::CalculateLighting ()
{
  csWorld::current_world->GetQuadcube ()->MakeEmpty ();
  //CsPrintf (MSG_INITIALIZATION, "  Shine light (%f,%f,%f).\n", center.x, center.y, center.z);
  csLightView lview;
  lview.l = this;
  lview.mirror = false;
  lview.gouraud_only = false;
  lview.gouraud_color_reset = false;
  lview.r = GetColor ().red;
  lview.g = GetColor ().green;
  lview.b = GetColor ().blue;
  lview.dynamic = false;

  CHK (lview.light_frustrum = new csFrustrum (center));
  lview.light_frustrum->MakeInfinite ();
  sector->CalculateLighting (lview);
}

void csStatLight::CalculateLighting (csThing* th)
{
  csWorld::current_world->GetQuadcube ()->MakeEmpty ();
  //CsPrintf (MSG_INITIALIZATION, "  Shine light (%f,%f,%f).\n", center.x, center.y, center.z);
  csLightView lview;
  lview.l = this;
  lview.mirror = false;
  lview.gouraud_only = false;
  lview.gouraud_color_reset = false;
  lview.r = GetColor ().red;
  lview.g = GetColor ().green;
  lview.b = GetColor ().blue;
  lview.dynamic = false;

  CHK (lview.light_frustrum = new csFrustrum (center));
  lview.light_frustrum->MakeInfinite ();
  th->CalculateLighting (lview);
}

void csStatLight::LightingFunc (csLightingFunc* callback, void* callback_data)
{
  csWorld::current_world->GetQuadcube ()->MakeEmpty ();
  csLightView lview;
  lview.l = this;
  lview.mirror = false;
  lview.gouraud_only = false;
  lview.gouraud_color_reset = false;
  lview.r = GetColor ().red;
  lview.g = GetColor ().green;
  lview.b = GetColor ().blue;
  lview.dynamic = false;
  lview.callback = callback;
  lview.callback_data = callback_data;

  CHK (lview.light_frustrum = new csFrustrum (center));
  lview.light_frustrum->MakeInfinite ();
  sector->CalculateLighting (lview);
}


void csStatLight::RegisterPolygon (csPolygon3D* poly)
{
  if (dynamic && !polygons)
  {
    num_polygon = 0;
    CHK (polygons = new csPolygon3D* [MAX_NUM_POLYGON]);
  }

  int i;
  for (i = 0 ; i < num_polygon ; i++)
    if (polygons[i] == poly)
      return;
  polygons[num_polygon++] = poly;
  if (num_polygon >= MAX_NUM_POLYGON)
  {
    CsPrintf (MSG_WARNING, "Overflow number of polygons for dynamic light!\n");
  }
}

void csStatLight::SetColor (const csColor& col)
{
  csLight::SetColor (col);
  int i;
  for (i = 0 ; i < num_polygon ; i++)
    polygons[i]->MakeDirtyDynamicLights ();
}

//---------------------------------------------------------------------------

csLightHitsSprite::csLightHitsSprite ()
{
  next_sprite = prev_sprite = NULL;
  next_light = prev_light = NULL;
  sprite = NULL;
  light = NULL;
}

csLightHitsSprite::~csLightHitsSprite ()
{
  if (sprite) sprite->UnlinkDynamicLight (this);
  if (light) light->UnlinkLightedSprite (this);
}

//---------------------------------------------------------------------------

csLightPatch::csLightPatch ()
{
  next_poly = prev_poly = NULL;
  next_light = prev_light = NULL;
  num_vertices = 0;
  max_vertices = 0;
  vertices = NULL;
  polygon = NULL;
  light = NULL;
}

csLightPatch::~csLightPatch ()
{
  CHK (delete [] vertices);
  RemovePatch ();
}

void csLightPatch::RemovePatch ()
{
  if (polygon) polygon->UnlinkLightpatch (this);
  if (light) light->UnlinkLightpatch (this);
  shadows.DeleteFrustrums ();
}

void csLightPatch::Initialize (int n)
{
  if (n > max_vertices)
  {
    CHK (delete [] vertices);
    max_vertices = n;
    CHK (vertices = new csVector3 [max_vertices]);
  }
  num_vertices = n;
}

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csDynLight,csLight);

csDynLight::csDynLight (float x, float y, float z, float dist,
	      float red, float green, float blue)
	      : csLight (x, y, z, dist, red, green, blue)
{
  lightpatches = NULL;
}

csDynLight::~csDynLight ()
{
  while (lightpatches)
    csWorld::current_world->lightpatch_pool->Free (lightpatches);
}

void csDynLight::Setup ()
{
  csWorld::current_world->GetQuadcube ()->MakeEmpty ();
  while (lightpatches)
    csWorld::current_world->lightpatch_pool->Free (lightpatches);
  csLightView lview;
  lview.l = this;
  lview.mirror = false;
  lview.gouraud_only = false;
  lview.gouraud_color_reset = false;
  lview.r = GetColor ().red;
  lview.g = GetColor ().green;
  lview.b = GetColor ().blue;
  lview.dynamic = true;

  CHK (lview.light_frustrum = new csFrustrum (center));
  lview.light_frustrum->MakeInfinite ();
  sector->CalculateLighting (lview);
}

void csDynLight::Move (csSector* sector, float x, float y, float z)
{
  if (sector == csDynLight::sector && center.x == x && center.y == y && center.z == z)
  {
    // No move. Just return.
    return;
  }
  SetSector (sector);
  center.x = x;
  center.y = y;
  center.z = z;
}

void csDynLight::UnlinkLightpatch (csLightPatch* lp)
{
  if (lp->next_light) lp->next_light->prev_light = lp->prev_light;
  if (lp->prev_light) lp->prev_light->next_light = lp->next_light;
  else lightpatches = lp->next_light;
  lp->prev_light = lp->next_light = NULL;
  lp->light = NULL;
}

void csDynLight::AddLightpatch (csLightPatch* lp)
{
  lp->next_light = lightpatches;
  lp->prev_light = NULL;
  if (lightpatches) lightpatches->prev_light = lp;
  lightpatches = lp;
  lp->light = this;
}

void csDynLight::UnlinkLightedSprite (csLightHitsSprite* lp)
{
  if (lp->next_light) lp->next_light->prev_light = lp->prev_light;
  if (lp->prev_light) lp->prev_light->next_light = lp->next_light;
  else lightedsprites = lp->next_light;
  lp->prev_light = lp->next_light = NULL;
  lp->light = NULL;
}

void csDynLight::AddLightedSprite (csLightHitsSprite* lp)
{
  lp->next_light = lightedsprites;
  lp->prev_light = NULL;
  if (lightedsprites) lightedsprites->prev_light = lp;
  lightedsprites = lp;
  lp->light = this;
}


//---------------------------------------------------------------------------
