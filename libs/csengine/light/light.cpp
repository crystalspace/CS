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
#include "csgeom/frustum.h"
#include "csengine/cbufcube.h"
#include "csengine/covcube.h"
#include "csengine/light.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/lghtmap.h"
#include "csengine/engine.h"
#include "csengine/lppool.h"
#include "csengine/polygon.h"
#include "csengine/curve.h"
#include "csengine/halo.h"
#include "csengine/meshobj.h"

/// Max number of polygons that can be lit by one light. (bad practice !!!@@@)
#define MAX_NUM_LIGHTMAP 2000

int csLight::ambient_red = DEFAULT_LIGHT_LEVEL;
int csLight::ambient_green = DEFAULT_LIGHT_LEVEL;
int csLight::ambient_blue = DEFAULT_LIGHT_LEVEL;

IMPLEMENT_CSOBJTYPE (csLight,csObject);

IMPLEMENT_IBASE_EXT (csLight)
  IMPLEMENTS_EMBEDDED_INTERFACE (iLight)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csLight::Light)
  IMPLEMENTS_INTERFACE (iLight)
IMPLEMENT_EMBEDDED_IBASE_END

csLight::csLight (float x, float y, float z, float d,
  float red, float green, float blue) : csObject()
{
  CONSTRUCT_EMBEDDED_IBASE (scfiLight);
  center.x = x;
  center.y = y;
  center.z = z;

  dist = d;
  sqdist = d * d;
  inv_dist = 1 / d;
  halo = NULL;

  color.red = red;
  color.green = green;
  color.blue = blue;

  attenuation = CS_ATTN_LINEAR;
}

csLight::~csLight ()
{
  if (flags.Check (CS_LIGHT_ACTIVEHALO))
    csEngine::current_engine->RemoveHalo (this);
  delete halo;
}

void csLight::SetHalo (csHalo *Halo)
{
  delete halo;
  halo = Halo;
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

void csLight::Light::SetSector (iSector* sector)
{
  scfParent->SetSector (sector->GetPrivateObject ());
}

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csStatLight,csLight);

IMPLEMENT_IBASE_EXT (csStatLight)
  IMPLEMENTS_EMBEDDED_INTERFACE (iStatLight)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csStatLight::eiStaticLight)
  IMPLEMENTS_INTERFACE (iStatLight)
IMPLEMENT_EMBEDDED_IBASE_END

csStatLight::csStatLight (float x, float y, float z, float dist,
  float red, float green, float blue, bool dynamic)
  : csLight (x, y, z, dist, red, green, blue)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiStatLight);
  csStatLight::dynamic = dynamic;
  lightmaps = NULL;
  num_lightmap = 0;
  flags.SetAll (CS_LIGHT_THINGSHADOWS);
}

csStatLight::~csStatLight ()
{
  delete [] lightmaps;
}

void poly_light_func (csObject* obj, csFrustumView* lview)
{
  csPolygon3D* poly = (csPolygon3D*)obj;
  poly->CalculateLighting (lview);
}

void curve_light_func (csObject* obj, csFrustumView* lview)
{
  csCurve* curve = (csCurve*)obj;
  curve->CalculateLighting (*lview);
}

void csStatLight::CalculateLighting ()
{
  //CsPrintf (MSG_INITIALIZATION, "  Shine light (%f,%f,%f).\n", center.x, center.y, center.z);
  csFrustumView lview;
  csFrustumContext* ctxt = lview.GetFrustumContext ();
  csLightingInfo& linfo = ctxt->GetLightingInfo ();
  linfo.SetGouraudOnly (false);
  linfo.SetColor (GetColor ());
  lview.SetUserData ((void*)this);
  lview.SetPolygonFunction (poly_light_func);
  lview.SetCurveFunction (curve_light_func);
  lview.SetRadius (GetRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetDynamic (false);
  lview.SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview.SetProcessMask (CS_ENTITY_NOLIGHTING, 0);

  ctxt->SetLightFrustum (new csFrustum (center));
  ctxt->GetLightFrustum ()->MakeInfinite ();
  sector->CheckFrustum ((iFrustumView*)&lview);
}

void csStatLight::CalculateLighting (iMeshWrapper* th)
{
  //CsPrintf (MSG_INITIALIZATION, "  Shine light (%f,%f,%f).\n", center.x, center.y, center.z);
  csFrustumView lview;
  csFrustumContext* ctxt = lview.GetFrustumContext ();
  csLightingInfo& linfo = ctxt->GetLightingInfo ();
  linfo.SetGouraudOnly (false);
  linfo.SetColor (GetColor ());
  lview.SetUserData ((void*)this);
  lview.SetPolygonFunction (poly_light_func);
  lview.SetCurveFunction (curve_light_func);
  lview.SetRadius (GetRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetDynamic (false);
  lview.SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview.SetProcessMask (CS_ENTITY_NOLIGHTING, 0);

  ctxt->SetLightFrustum (new csFrustum (center));
  ctxt->GetLightFrustum ()->MakeInfinite ();
  // @@@ Engine should not know about iThingState!!!
  iThingState* thing_state = QUERY_INTERFACE (th->GetMeshObject (),
  	iThingState);
  if (thing_state)
  {
    thing_state->CheckFrustum ((iFrustumView*)&lview, th->GetMovable ());
    thing_state->DecRef ();
  }
}

void csStatLight::LightingFunc (csLightingFunc* callback, void* callback_data)
{
#if 0
  csFrustumView lview;
  csFrustumContext* ctxt = lview.GetFrustumContext ();
  csLightingInfo& linfo = ctxt->GetLightingInfo ();
  linfo.SetGouraudOnly (false);
  linfo.SetColor (GetColor ());
  lview.SetUserData ((void*)this);
  lview.SetPolygonFunction (poly_light_func);
  lview.SetCurveFunction (curve_light_func);
  lview.SetRadius (GetRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetDynamic (false);
  lview.callback = callback;
  lview.callback_data = callback_data;
  lview.shadow_thing_mask = CS_ENTITY_NOSHADOWS;
  lview.shadow_thing_value = 0;
  lview.process_thing_mask = CS_ENTITY_NOLIGHTING;
  lview.process_thing_value = 0;

  ctxt->light_frustum = new csFrustum (center);
  ctxt->light_frustum->MakeInfinite ();
  sector->CheckFrustum ((iFrustumView*)&lview);
#endif
}


void csStatLight::RegisterLightMap (csLightMap* lmap)
{
  if (dynamic && !lightmaps)
  {
    num_lightmap = 0;
    lightmaps = new csLightMap* [MAX_NUM_LIGHTMAP];
  }

  int i;
  for (i = 0 ; i < num_lightmap ; i++)
    if (lightmaps[i] == lmap)
      return;
  lightmaps[num_lightmap++] = lmap;
  if (num_lightmap >= MAX_NUM_LIGHTMAP)
  {
    CsPrintf (MSG_WARNING, "Overflow number of lightmaps for dynamic light!\n");
  }
}

void csStatLight::SetColor (const csColor& col)
{
  csLight::SetColor (col);
  int i;
  for (i = 0 ; i < num_lightmap ; i++)
    lightmaps[i]->MakeDirtyDynamicLights ();
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
  light_frustum = NULL;
}

csLightPatch::~csLightPatch ()
{
  delete [] vertices;
  if (light_frustum) light_frustum->DecRef ();
  RemovePatch ();
}

void csLightPatch::RemovePatch ()
{
  if (polygon) polygon->UnlinkLightpatch (this);
  if (curve) curve->UnlinkLightPatch (this);
  if (light) light->UnlinkLightpatch (this);
  shadows.DeleteShadows ();
  if (light_frustum)
  {
    light_frustum->DecRef ();
    light_frustum = NULL;
  }
}

void csLightPatch::Initialize (int n)
{
  if (n > max_vertices)
  {
    delete [] vertices;
    max_vertices = n;
    vertices = new csVector3 [max_vertices];
  }
  num_vertices = n;
}

//---------------------------------------------------------------------------

IMPLEMENT_CSOBJTYPE (csDynLight,csLight);

IMPLEMENT_IBASE_EXT (csDynLight)
  IMPLEMENTS_EMBEDDED_INTERFACE (iDynLight)
IMPLEMENT_IBASE_EXT_END

IMPLEMENT_EMBEDDED_IBASE (csDynLight::eiDynLight)
  IMPLEMENTS_INTERFACE (iDynLight)
IMPLEMENT_EMBEDDED_IBASE_END

csDynLight::csDynLight (float x, float y, float z, float dist,
  float red, float green, float blue)
  : csLight (x, y, z, dist, red, green, blue)
{
  CONSTRUCT_EMBEDDED_IBASE (scfiDynLight);
  lightpatches = NULL;
}

csDynLight::~csDynLight ()
{
  while (lightpatches)
    csEngine::current_engine->lightpatch_pool->Free (lightpatches);
  csEngine::current_engine->RemoveDynLight (this);
}

void csDynLight::Setup ()
{
  while (lightpatches)
    csEngine::current_engine->lightpatch_pool->Free (lightpatches);
  csFrustumView lview;
  csFrustumContext* ctxt = lview.GetFrustumContext ();
  csLightingInfo& linfo = ctxt->GetLightingInfo ();
  linfo.SetGouraudOnly (false);
  linfo.SetColor (GetColor ());
  lview.SetUserData ((void*)this);
  lview.SetPolygonFunction (poly_light_func);
  lview.SetCurveFunction (curve_light_func);
  lview.SetRadius (GetRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetDynamic (true);
  lview.SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview.SetProcessMask (CS_ENTITY_NOLIGHTING, 0);

  ctxt->SetLightFrustum (new csFrustum (center));
  ctxt->GetLightFrustum ()->MakeInfinite ();
  sector->CheckFrustum ((iFrustumView*)&lview);
}

void csDynLight::SetColor (const csColor& col)
{
  csLight::SetColor (col);
  csLightPatch* lp = lightpatches;
  while (lp)
  {
    if (lp->GetPolygon ())
      lp->GetPolygon()->MakeDirtyDynamicLights ();
    else
      lp->GetCurve()->MakeDirtyDynamicLights ();

    lp = lp->GetNextLight ();
  }
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

