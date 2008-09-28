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
#include <math.h>

#include "csqint.h"
#include "csqsqrt.h"

#include "csgeom/frustum.h"
#include "csgeom/math.h"
#include "csgeom/poly3d.h"
#include "csgeom/polyaa.h"
#include "csgeom/polyclip.h"

#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/movable.h"
#include "iengine/texture.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"

#include "thing.h"
#include "polytext.h"
#include "polygon.h"
#include "lppool.h"
#include "lghtmap.h"

// Option variable: cosinus factor.
float csPolyTexture::cfg_cosinus_factor = 0;

csPolyTexture::csPolyTexture ()
{
  lm = 0;
  //polygon = 0;
  shadow_bitmap = 0;
  light_version = 0;
}

void csPolyTexture::ObjectToWorld (const csMatrix3& m_obj2tex,
  const csVector3& v_obj2tex, const csReversibleTransform &obj,
  csMatrix3& m_world2tex,
  csVector3& v_world2tex)
{
  // From: T = Mot * (O - Vot)
  //       W = Mow * O - Vow
  // To:   T = Mwt * (W - Vwt)
  // Mwo * (W + Vow) = O
  // T = Mot * (Mwo * (W + Vow) - (Mwo * Mow) * Vot)
  // T = Mot * Mwo * (W + Vow - Mow * Vot)
  // ===>
  // Mwt = Mot * Mwo
  // Vwt = Mow * Vot - Vow

  m_world2tex = m_obj2tex;
  m_world2tex *= obj.GetO2T ();
  v_world2tex = obj.This2Other (v_obj2tex);
}

csPolyTexture::~csPolyTexture ()
{
  delete shadow_bitmap;
}

void csPolyTexture::SetLightMap (csLightMap *lightmap)
{
  lm = lightmap;
}

bool csPolyTexture::RecalculateDynamicLights (
	const csMatrix3& m_world2tex,
	const csVector3& v_world2tex,
	csPolygon3D* polygon,
	const csPlane3& polygon_world_plane,
	const csColor& amb,
	csLightingScratchBuffer& finalLM)
{
  if (!lm) return false;

  // first combine the static and pseudo-dynamic lights
  csThing* thing = polygon->GetParent ();

  if (!lm->UpdateRealLightMap (amb.red, amb.green, amb.blue,
      thing->GetLightVersion () != light_version, finalLM))
    return false;

  light_version = thing->GetLightVersion ();

  //---
  // Now add all dynamic lights.
  //---
  csLightPatch *lp = polygon->GetLightpatches ();
  while (lp)
  {
    ShineDynLightMap (lp, m_world2tex, v_world2tex, polygon,
    	polygon_world_plane, finalLM);
    lp = lp->GetNext ();
  }

  return true;
}

void csPolyTexture::InitLightMaps ()
{
  // To force updating we decrease light_version.
  light_version--;
}

/* Modified by me to correct some lightmap's border problems -- D.D. */
void csPolyTexture::ShineDynLightMap (csLightPatch *lp,
  const csMatrix3& m_world2tex,
  const csVector3& v_world2tex,
  csPolygon3D* polygon,
  const csPlane3& polygon_world_plane,
  csLightingScratchBuffer& finalLM)
{
  csPolyTextureMapping* tmapping = polygon->GetStaticPoly ()
  	->GetTextureMapping ();
  int lw = 1 +
    ((tmapping->GetLitWidth () + csLightMap::lightcell_size - 1) >>
      csLightMap::lightcell_shift);
  int lh = 1 +
    ((tmapping->GetLitHeight () + csLightMap::lightcell_size - 1) >>
      csLightMap::lightcell_shift);

  int ww, hh;
  iMaterial* mat = polygon->GetStaticPoly()->GetMaterial();
  mat->GetTexture ()->GetRendererDimensions (ww, hh);

  float cosfact = polygon->GetParent ()->GetStaticData ()->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csMatrix3 m_t2w = m_world2tex.GetInverse ();
  csVector3 vv = v_world2tex;

  csVector3 v1, v2;

  float invww, invhh;
  invww = 1.0f / (float)ww;
  invhh = 1.0f / (float)hh;

  csRGBcolor* map = finalLM.GetArray();
  iLight *light = lp->GetLight ();

  int i;
  float miny = 1000000, maxy = -1000000;
  int MaxIndex = -1, MinIndex = -1;
  float inv_lightcell_size = 1.0f / csLightMap::lightcell_size;

  csVector3 lightpos;
  if (lp->GetLightFrustum ())
    lightpos = lp->GetLightFrustum ()->GetOrigin ();
  else
    lightpos = light->GetMovable ()->GetFullPosition ();

  float infradius_sq = csSquare (light->GetCutoffDistance ());

  // Calculate the uv's for all points of the frustum (the
  // frustum is actually a clipped version of the polygon).
  csVector2 f_uv[100];
  if (lp->GetVertices ())
  {
    int mi;
    for (i = 0; i < lp->GetVertexCount (); i++)
    {
      //if (lview.IsMirrored ()) mi = lview.num_frustum-i-1;
      //else mi = i;
      mi = i;

      // T = Mwt * (W - Vwt)
      //v1 = pl->m_world2te *(lp->vertices[mi] + lp->center - pl->v_world2tex);
      v1 = m_world2tex * (lp->GetVertex (mi) + lightpos - v_world2tex);
      f_uv[i].x = (v1.x * ww - tmapping->GetIMinU ()) * inv_lightcell_size;
      f_uv[i].y = (v1.y * hh - tmapping->GetIMinV ()) * inv_lightcell_size;
      if (f_uv[i].y < miny) miny = f_uv[MinIndex = i].y;
      if (f_uv[i].y > maxy) maxy = f_uv[MaxIndex = i].y;
    }
  }
  else return;

  csColor color = light->GetColor () * CS_NORMAL_LIGHT_LEVEL;

  int new_lw = lm->GetWidth ();

  int scanL1, scanL2, scanR1, scanR2;// scan vertex left/right start/final
  float sxL, sxR, dxL, dxR;          // scanline X left/right and deltas
  int sy, fyL, fyR;                  // scanline Y, final Y left, final Y right
  int xL, xR;

  sxL = sxR = dxL = dxR = 0;
  scanL2 = scanR2 = MaxIndex;

  // sy = fyL = fyR =
  //   (csQround (f_uv[scanL2].y)>lh-1)?lh-1:csQround (f_uv[scanL2].y);
  sy = fyL = fyR = (csQround (ceil (f_uv[scanL2].y)) > lh - 1) ?
    lh - 1 : csQround (ceil (f_uv[scanL2].y));

  for (;;)
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
a:
        if (scanR2 == MinIndex) return;
        scanR1 = scanR2;
        scanR2 = (scanR2 + 1) % lp->GetVertexCount ();

        if (ABS (f_uv[scanR2].y - f_uv[MaxIndex].y) < EPSILON)
        {
          // oops! we have a flat bottom!
          goto a;
        }

        /*      if (scanR2 == MinIndex) return;
        scanR1 = scanR2;
        scanR2 = (scanR2 + 1) % lp->num_vertices;
*/
        fyR = csQround (floor (f_uv[scanR2].y));

        float dyR = (f_uv[scanR1].y - f_uv[scanR2].y);
        sxR = f_uv[scanR1].x;
        if (dyR != 0)
        {
          dxR = (f_uv[scanR2].x - sxR) / dyR;

          // horizontal pixel correction
          sxR += dxR * (f_uv[scanR1].y - ((float)sy));
        }
        else
          dxR = 0;
        leave = false;
      }

      if (sy <= fyL)
      {
b:
        if (scanL2 == MinIndex) return;
        scanL1 = scanL2;
        scanL2 = (scanL2 - 1 + lp->GetVertexCount ()) % lp->GetVertexCount ();

        if (ABS (f_uv[scanL2].y - f_uv[MaxIndex].y) < EPSILON)
        {
          // oops! we have a flat bottom!
          goto b;
        }

        //scanL1 = scanL2;

        //scanL2 = (scanL2 - 1 + lp->num_vertices) % lp->num_vertices;
        fyL = csQround (floor (f_uv[scanL2].y));

        float dyL = (f_uv[scanL1].y - f_uv[scanL2].y);
        sxL = f_uv[scanL1].x;
        if (dyL != 0)
        {
          dxL = (f_uv[scanL2].x - sxL) / dyL;

          // horizontal pixel correction
          sxL += dxL * (f_uv[scanL1].y - ((float)sy));
        }
        else
          dxL = 0;
        leave = false;

      }
    } while (!leave);

    // Find the trapezoid top (or bottom in inverted Y coordinates)
    int fin_y;
    if (fyL > fyR)
      fin_y = fyL;
    else
      fin_y = fyR;

    while (sy >= fin_y)
    {
      // @@@ The check below should not be needed but it is.
      if (sy < 0)
      {
        return;
      }

      // Compute the rounded screen coordinates of horizontal strip
      float _l = sxL, _r = sxR;

      if (_r > _l)
      {
        float _ = _r;
        _r = _l;
        _l = _;
      }

      xL = 1 + csQround (ceil (_l));
      xR = csQround (floor (_r));

      //if (xR > xL) { int xswap = xR; xR = xL; xL = xswap; }
      if (xR < 0) xR = 0;
      if (xL > lw) xL = lw;

      v1.x = float ((xR << csLightMap::lightcell_shift)
      	+ tmapping->GetIMinU ()) * invww;
      v1.y = float ((sy << csLightMap::lightcell_shift)
      	+ tmapping->GetIMinV ()) * invhh;
      v1.z = 0;
      float dv1x = float (csLightMap::lightcell_size) * invww;

      int uv = sy * new_lw + xR;
      csRGBcolor* map_uv = map + uv;

      // We have to calculate v2 = vv + m_t2w * v1 every time in the
      // loop below. However, v1.z is 0 and v1.y never changes so we
      // can optimize this a little bit.
      v2.x = vv.x + m_t2w.m11 * v1.x + m_t2w.m12 * v1.y;
      v2.y = vv.y + m_t2w.m21 * v1.x + m_t2w.m22 * v1.y;
      v2.z = vv.z + m_t2w.m31 * v1.x + m_t2w.m32 * v1.y;
      float dv2x_x = m_t2w.m11 * dv1x;
      float dv2x_y = m_t2w.m21 * dv1x;
      float dv2x_z = m_t2w.m31 * dv1x;

      // We work relative to lightpos.
      v2 -= lightpos;

      int du = xL - xR;
      if (cosfact > 0.0001)
        ShineDynLightMapHorizCosfact (du, map_uv, v2,
	  dv2x_x, dv2x_y, dv2x_z,
	  light, color, infradius_sq, cosfact,
	  polygon_world_plane);
      else
        ShineDynLightMapHoriz (du, map_uv, v2,
	  dv2x_x, dv2x_y, dv2x_z,
	  light, color, infradius_sq,
	  polygon_world_plane);

      if (!sy) return;
      sxL += dxL;
      sxR += dxR;
      sy--;
    }
  }
}

#undef UPDATE_MAP
#define UPDATE_MAP(cname) \
      if (color.cname > 0) \
      { \
        float fl = color.cname * brightness; \
        if (fl > 0.5) \
        { \
          int l = csQint (fl) + map_uv->cname; \
          if (l > 255) l = 255; \
          map_uv->cname = l; \
        } \
      }

void csPolyTexture::ShineDynLightMapHorizCosfact (
	int du, csRGBcolor* map_uv,
	csVector3& v2,
	float dv2x_x, float dv2x_y, float dv2x_z,
	iLight* light, const csColor& color, float infradius_sq,
	float cosfact,
	const csPlane3& polygon_world_plane)
{
  float inv_dist;

  // We select a different loop based on attenuation type for most
  // effiency. This makes a huge difference.
  switch (light->GetAttenuationMode ())
  {
    case CS_ATTN_INVERSE:
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float d = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (d < infradius_sq)
        {
          float inv_d = csQisqrt (d);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float cosinus = v2 * polygon_world_plane.Normal ();
          cosinus *= inv_d;
          cosinus += cosfact;
          if (cosinus < 0.02) continue;
          else if (cosinus > 1) cosinus = 1;

          float brightness = cosinus * inv_d;
          if (brightness < 0.005) continue;
	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
    case CS_ATTN_REALISTIC:
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float sqd = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (sqd < infradius_sq)
        {
          float inv_d = csQisqrt (sqd);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float cosinus = v2 * polygon_world_plane.Normal ();
          cosinus *= inv_d;
          cosinus += cosfact;
          if (cosinus < 0.02) continue;
          else if (cosinus > 1) cosinus = 1;

          float brightness = cosinus / sqd;
          if (brightness < 0.005) continue;
	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
    case CS_ATTN_NONE:
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float d = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (d < infradius_sq)
        {
          float inv_d = csQisqrt (d);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float cosinus = v2 * polygon_world_plane.Normal ();
          cosinus *= inv_d;
          cosinus += cosfact;
	  float brightness;
          if (cosinus < 0.001) continue;
          else if (cosinus > 1) brightness = 1;
	  else brightness = cosinus;

	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
    case CS_ATTN_LINEAR:
      inv_dist = 1.0 / light->GetAttenuationConstants ().x;
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float d = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (d < infradius_sq)
        {
          d = csQsqrt (d);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float cosinus = v2 * polygon_world_plane.Normal ();
          cosinus /= d;
          cosinus += cosfact;
	  float brightness;
          if (cosinus < 0.02) continue;
          else if (cosinus > 1) brightness = 1.0 - d * inv_dist;
	  else brightness = cosinus - cosinus * d * inv_dist;

          if (brightness < 0.005) continue;
	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
    default:
      // Most general loop.
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float d = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (d < infradius_sq)
        {
          d = csQsqrt (d);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float cosinus = v2 * polygon_world_plane.Normal ();
          cosinus /= d;
          cosinus += cosfact;
          if (cosinus < 0.02) continue;
          else if (cosinus > 1) cosinus = 1;

          float brightness = cosinus * light->GetBrightnessAtDistance (d);
          if (brightness < 0.005) continue;
	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
  }
}

void csPolyTexture::ShineDynLightMapHoriz (
	int du, csRGBcolor* map_uv,
	csVector3& v2,
	float dv2x_x, float dv2x_y, float dv2x_z,
	iLight* light, const csColor& color, float infradius_sq,
	const csPlane3& polygon_world_plane)
{
  float inv_dist;

  // We select a different loop based on attenuation type for most
  // effiency. This makes a huge difference.
  switch (light->GetAttenuationMode ())
  {
    case CS_ATTN_INVERSE:
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float d = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (d < infradius_sq)
        {
          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float brightness = v2 * polygon_world_plane.Normal ();
          brightness /= d;
          if (brightness < 0.005) continue;
	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
    case CS_ATTN_REALISTIC:
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float sqd = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (sqd < infradius_sq)
        {
          float d = csQsqrt (sqd);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float cosinus = v2 * polygon_world_plane.Normal ();
          float brightness = cosinus / (d * sqd);
          if (brightness < 0.005) continue;
	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
    case CS_ATTN_NONE:
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float d = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (d < infradius_sq)
        {
          float inv_d = csQisqrt (d);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float brightness = v2 * polygon_world_plane.Normal ();
          brightness *= inv_d;
          if (brightness < 0.005) continue;

	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
    case CS_ATTN_LINEAR:
      inv_dist = 1.0 / light->GetAttenuationConstants ().x;
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float d = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (d < infradius_sq)
        {
          float inv_d = csQisqrt (d);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float cosinus = v2 * polygon_world_plane.Normal ();
	  float brightness = cosinus * (inv_d - inv_dist);

          if (brightness < 0.005) continue;
	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
    default:
      // Most general loop.
      while (du > 0)
      {
        du--;
        // Calculate squared distance between lightpos and transformed point.
        float d = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
        if (d < infradius_sq)
        {
          d = csQsqrt (d);

          // Remember that v2 is point on lightmap in 3D minutes lightpos.
          float cosinus = v2 * polygon_world_plane.Normal ();
          cosinus /= d;
          float brightness = cosinus * light->GetBrightnessAtDistance (d);
          if (brightness < 0.005) continue;
	  UPDATE_MAP (red);
	  UPDATE_MAP (green);
	  UPDATE_MAP (blue);
        }
        v2.x += dv2x_x;
        v2.y += dv2x_y;
        v2.z += dv2x_z;
        map_uv++;
      }
      break;
  }
}
#undef UPDATE_MAP

void csPolyTexture::UpdateFromShadowBitmap (
  iLight *light,
  const csVector3 &lightpos,
  const csColor &lightcolor,
  const csMatrix3& m_world2tex,
  const csVector3& v_world2tex,
  csPolygon3D* polygon,
  const csPlane3& polygon_world_plane)
{
  CS_ASSERT (shadow_bitmap != 0);

  int ww, hh;
  iMaterial* mat = polygon->GetStaticPoly()->GetMaterial();
  if (mat && mat->GetTexture ())
    mat->GetTexture ()->GetRendererDimensions (ww, hh);
  else
    ww = hh = 64;

  float mul_u = 1.0f / ww;
  float mul_v = 1.0f / hh;

  bool dyn = light->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_PSEUDO;

  // From: T = Mwt * (W - Vwt)
  // ===>
  // Mtw * T = W - Vwt
  // Mtw * T + Vwt = W
  csMatrix3 m_t2w = m_world2tex.GetInverse ();
  const csVector3 &v_t2w = v_world2tex;

  // Cosinus factor
  float cosfact = polygon->GetParent ()->GetStaticData ()->GetCosinusFactor ();
  if (cosfact == -1) cosfact = cfg_cosinus_factor;

  csPolyTextureMapping* tmapping = polygon->GetStaticPoly ()->GetTextureMapping ();
  if (dyn)
  {
    if (!(shadow_bitmap->IsFullyShadowed () || shadow_bitmap->IsFullyUnlit ()))
    {
      csShadowMap *smap = lm->FindShadowMap (light);
      bool created = false;
      if (!smap)
      {
        smap = lm->NewShadowMap (light);
	created = true;
      }

      if (!smap->map.IsValid())
      {
        int lw = csLightMap::CalcLightMapWidth (tmapping->GetLitWidth ());
        int lh = csLightMap::CalcLightMapHeight (tmapping->GetLitHeight ());
        smap->map.AttachNew (new csDataBuffer (lw * lh));
        memset (smap->map->GetData(), 0, lw*lh);
      }
      unsigned char *shadowmap = smap->map->GetUint8();
      bool relevant = shadow_bitmap->UpdateShadowMap (
        shadowmap,
        csLightMap::lightcell_shift,
        tmapping->GetIMinU (),
        tmapping->GetIMinV (),
        mul_u,
        mul_v,
        m_t2w,
        v_t2w,
        light,
        lightpos,
        polygon,
	polygon_world_plane,
        cosfact);

      if (!relevant && created)
      {
        // The shadow map is just created but it is not relevant (i.e.
	// the light really doesn't affect it).
	// In that case we simply delete it again.
	lm->DelShadowMap (smap);
      }
      else
      {
        smap->CalcMaxShadow (lm->lwidth * lm->lheight);
      }
    }
  }
  else
  {
    csRGBcolor *lightmap = lm->GetStaticMap ();
    if (!lightmap)
    {
      csColor ambient;
      polygon->GetParent ()->GetStaticData ()
      	->thing_type->engine->GetAmbientLight (ambient);
      lm->InitColor (
          int(ambient.red * 255.0f),
          int(ambient.green * 255.0f),
          int(ambient.blue * 255.0f));
      lightmap = lm->GetStaticMap ();
    }
    shadow_bitmap->UpdateLightMap (
        lightmap,
        csLightMap::lightcell_shift,
        tmapping->GetIMinU (),
        tmapping->GetIMinV (),
        mul_u,
        mul_v,
        m_t2w,
        v_t2w,
        light,
        lightpos,
        lightcolor,
        polygon,
	polygon_world_plane,
        cosfact);
  }

  delete shadow_bitmap;
  shadow_bitmap = 0;
}

int csPolyTexture::GetLightCellSize ()
{
  return csLightMap::lightcell_size;
}

int csPolyTexture::GetLightCellShift ()
{
  return csLightMap::lightcell_shift;
}

//-----------------------------------------------------------------------------
csShadowBitmap::csShadowBitmap (
  int lm_w,
  int lm_h,
  int quality,
  int default_light)
{
  shadow = 0;
  light = 0;
  csShadowBitmap::lm_w = lm_w;
  csShadowBitmap::lm_h = lm_h;
  csShadowBitmap::quality = quality;
  csShadowBitmap::default_light = default_light;
  if (quality >= 0)
  {
    sb_w = lm_w << quality;
    sb_h = lm_h << quality;
  }
  else
  {
    sb_w = (lm_w + 1) >> (-quality);
    sb_h = (lm_h + 1) >> (-quality);
    if (sb_w < 1) sb_w = 1;
    if (sb_h < 1) sb_h = 1;
  }

  cnt_unshadowed = sb_w * sb_h;
  if (default_light)
    cnt_unlit = 0;
  else
    cnt_unlit = sb_w * sb_h;
}

void csShadowBitmap::LightPutPixel (int x, int y, float area, void *arg)
{
  csShadowBitmap *sb = (csShadowBitmap *)arg;
  sb->_LightPutPixel (x, y, area);
}

void csShadowBitmap::LightDrawBox (int x, int y, int w, int h, void *arg)
{
  csShadowBitmap *sb = (csShadowBitmap *)arg;
  sb->_LightDrawBox (x, y, w, h);
}

void csShadowBitmap::ShadowPutPixel (int x, int y, float area, void *arg)
{
  csShadowBitmap *sb = (csShadowBitmap *)arg;
  sb->_ShadowPutPixel (x, y, area);
}

void csShadowBitmap::ShadowDrawBox (int x, int y, int w, int h, void *arg)
{
  csShadowBitmap *sb = (csShadowBitmap *)arg;
  sb->_ShadowDrawBox (x, y, w, h);
}

void csShadowBitmap::_LightPutPixel (int x, int y, float area)
{
  if (x >= sb_w || y >= sb_h || x < 0 || y < 0) return ;
  if (area < .2) return ;
  CS_ASSERT (cnt_unlit >= 0);

  int idx = sb_w * y + x;
  if (!light[idx])
  {
    light[idx] = 1;
    cnt_unlit--;
  }
}

void csShadowBitmap::_LightDrawBox (int x, int y, int w, int h)
{
  if (cnt_unlit == 0) return ;
  if (x + w <= 0 || y + h <= 0 || x >= sb_w || y >= sb_h) return ;
  CS_ASSERT (cnt_unlit >= 0);
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x + w > sb_w) w = sb_w - x;
  if (y + h > sb_h) h = sb_h - y;

  int offs = sb_w * y + x;
  char *l = &(light[offs]);
  int delta = sb_w - w;
  while (h > 0)
  {
    int ww = w;
    while (ww > 0)
    {
      if (!*l)
      {
        *l = 1;
        cnt_unlit--;
      }

      l++;
      ww--;
    }

    l += delta;
    h--;
  }
}

void csShadowBitmap::_ShadowPutPixel (int x, int y, float area)
{
  if (x >= sb_w || y >= sb_h || x < 0 || y < 0) return;
  if (area < .2) return ;
  CS_ASSERT (cnt_unshadowed >= 0);

  int idx = sb_w * y + x;
  if (!shadow[idx])
  {
    shadow[idx] = 1;
    cnt_unshadowed--;
  }
}

void csShadowBitmap::_ShadowDrawBox (int x, int y, int w, int h)
{
  if (cnt_unshadowed == 0) return ;
  if (x + w <= 0 || y + h <= 0 || x >= sb_w || y >= sb_h) return ;
  CS_ASSERT (cnt_unshadowed >= 0);
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x + w > sb_w) w = sb_w - x;
  if (y + h > sb_h) h = sb_h - y;

  int offs = sb_w * y + x;
  char *s = &(shadow[offs]);
  int delta = sb_w - w;
  while (h > 0)
  {
    int ww = w;
    while (ww > 0)
    {
      if (!*s)
      {
        *s = 1;
        cnt_unshadowed--;
      }

      s++;
      ww--;
    }

    s += delta;
    h--;
  }
}

void csShadowBitmap::RenderTotal (int val)
{
  if (!shadow)
  {
    shadow = new char[sb_w * sb_h];
    memset (shadow, val, sb_w * sb_h);
    if (val)
      cnt_unshadowed = 0;
    else
      cnt_unshadowed = sb_w * sb_h;
    light = new char[sb_w * sb_h];

    int lv = val ? default_light : 1;
    memset (light, lv, sb_w * sb_h);
    if (lv)
      cnt_unlit = 0;
    else
      cnt_unlit = sb_w * sb_h;
    return ;
  }

  if (val == 1)
  {
    memset (shadow, 1, sb_w * sb_h);
    cnt_unshadowed = 0;
  }
  else
  {
    memset (light, 1, sb_w * sb_h);
    cnt_unlit = 0;
  }

  return ;
}

void csShadowBitmap::RenderPolygon (
  csVector2 *shadow_poly,
  int num_vertices,
  int val)
{
  int i;

  //-------------------
  // First we convert the polygon from lightmap coordinates to shadow
  // bitmap coordinates.
  //-------------------
  if (quality > 0)
  {
    float mul = float (1 << quality);
    for (i = 0; i < num_vertices; i++)
      shadow_poly[i] = shadow_poly[i] * mul;
  }
  else if (quality < 0)
  {
    float div = 1.0f / float (1 << -quality);
    for (i = 0; i < num_vertices; i++)
      shadow_poly[i] = shadow_poly[i] * div;
  }

  if (!shadow)
  {
    shadow = new char[sb_w * sb_h];
    memset (shadow, 0, sb_w * sb_h);
    light = new char[sb_w * sb_h];
    memset (light, default_light, sb_w * sb_h);
  }

  if (val == 1)
    csAntialiasedPolyFill (
      shadow_poly,
      num_vertices,
      this,
      ShadowPutPixel,
      ShadowDrawBox);
  else
    csAntialiasedPolyFill (
      shadow_poly,
      num_vertices,
      this,
      LightPutPixel,
      LightDrawBox);
  return ;
}

float csShadowBitmap::GetLighting (int lm_u, int lm_v)
{
  if (!shadow) return 1;
  CS_ASSERT (lm_u >= 0 && lm_u < lm_w);
  CS_ASSERT (lm_v >= 0 && lm_v < lm_h);
  if (quality == 0)
  {
    // Shadow-bitmap has equal quality.
    int idx = lm_v * sb_w + lm_u;
    bool l = light[idx] && !shadow[idx];
    return float (l);
  }
  else if (quality > 0)
  {
    // Shadow-bitmap has better quality.

    // Here we will take an average of all shadow-bitmap values.
    int d = 1 << (quality - 1);
    int u = lm_u << quality;
    int v = lm_v << quality;

    // Calculate the bounds in shadow-bitmap space for which we
    // are going to take the average. Make sure we don't exceed
    // the total bounds of the shadow-bitmap.
    int minu = u - d;
    if (minu < 0) minu = 0;

    int maxu = u + d;
    if (maxu > sb_w - 1) maxu = sb_w - 1;

    int minv = v - d;
    if (minv < 0) minv = 0;

    int maxv = v + d;
    if (maxv > sb_h - 1) maxv = sb_h - 1;

    int du = maxu - minu + 1;
    int dv = maxv - minv + 1;
    int s = 0;
    int idx = minv * sb_w + minu;
    char *bml = &light[idx];
    char *bms = &shadow[idx];
    int delta = sb_w - du;
    int ddv = dv;
    while (ddv > 0)
    {
      int ddu = du;
      while (ddu > 0)
      {
        s += (*bml) && !*bms;
        bml++;
        bms++;
        ddu--;
      }

      bml += delta;
      bms += delta;
      ddv--;
    }

    return float (s) / float (du * dv);
  }
  else
  {
    // Shadow-bitmap has lower quality.
    // Here we will interpolate shadow-bitmap values.
    // @@@ TODO!!!
    return 1;
  }
}


void csShadowBitmap::UpdateLightMap (
  csRGBcolor *lightmap,
  int lightcell_shift,
  float shf_u,
  float shf_v,
  float mul_u,
  float mul_v,
  const csMatrix3 &m_t2w,
  const csVector3 &v_t2w,
  iLight *light,
  const csVector3 &lightpos,
  const csColor &lightcolor,
  csPolygon3D* poly,
  const csPlane3& poly_world_plane,
  float cosfact)
{
  if (IsFullyShadowed () || IsFullyUnlit ()) return ;

  float light_r = lightcolor.red * CS_NORMAL_LIGHT_LEVEL;
  float light_g = lightcolor.green * CS_NORMAL_LIGHT_LEVEL;
  float light_b = lightcolor.blue * CS_NORMAL_LIGHT_LEVEL;
  bool ful_lit = IsFullyLit ();
  int i, j, act;
  int base_uv = 0;
  float rv_step = (1 << lightcell_shift) * mul_v;
  float ru_step = (1 << lightcell_shift) * mul_u;
  float rv = shf_v * mul_v;
  float ru_base = shf_u * mul_u - ru_step;

  csVector3 v_ru (m_t2w.m11, m_t2w.m21, m_t2w.m31);
  csVector3 v_rv (m_t2w.m12, m_t2w.m22, m_t2w.m32);
  csVector3 v, v_base;

  v_base = v_t2w + rv * v_rv + ru_base * v_ru;
  v_rv *= rv_step;
  v_ru *= ru_step;

  csThing* poly_parent = poly->GetParent ();
  csThingStatic* poly_parent_static = poly_parent->GetStaticData ();
  bool do_smooth = poly_parent->GetStaticData ()->GetSmoothingFlag();
  float* distances = 0;
  csVector3* nearestNormals = 0;
  csVector3* worldNormals = 0;
  if (do_smooth)
  {
    distances = new float[100];
    nearestNormals = new csVector3[100];
    poly_parent->WorUpdate ();
    csReversibleTransform mesh_transf = poly_parent->GetCachedMovable ()
    	->GetFullTransform ();
    worldNormals = new csVector3[poly_parent_static->num_vertices];
    csVector3* normals = poly_parent_static->GetNormals ();
    for (i = 0 ; i < poly_parent_static->num_vertices ; i++)
      worldNormals[i] = mesh_transf.This2OtherRelative (normals[i]);
  }

  for (i = 0; i < lm_h; i++)
  {
    int uv = base_uv;
    base_uv += lm_w;

    v = v_base;
    v_base += v_rv;

    for (j = 0; j < lm_w; j++, uv++)
    {
      v += v_ru;
      float lightness;
      if (ful_lit)
        lightness = 1;
      else
      {
        lightness = GetLighting (j, i);
        if (lightness < EPSILON) continue;
      }

      // @@@ Optimization: It should be possible to combine these
      // calculations into a more efficient formula.
      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= csSquare (light->GetCutoffDistance ())) continue;

      d = csQsqrt (d);

      // Initialize normal with the flat one
      csVector3 normal = poly_world_plane.Normal ();
      if (do_smooth)
      {
	int* vertexs = poly->GetStaticPoly ()->GetVertexIndices ();
	int vCount = poly->GetStaticPoly ()->GetVertexCount ();

	float shortestDistance = 10000000.0f;  // Big enought?
	int nearestNormal = 0;

	for (act = 0; act < vCount ; act++)
	{
	  // Create the 3D Segments
	  csSegment3 seg;
	  seg.SetStart (poly_parent->Vwor (vertexs[act]));
	  seg.SetEnd (poly_parent->Vwor (vertexs[(act+1)%vCount]));

	  // Find the nearest point from v to the segment
	  // a: Start
	  // b: End
	  // c: point
	  csVector3& v_a = seg.Start ();
	  csVector3& v_b = seg.End ();
	  csVector3& v_c = v;
	  float dt_a = (v_c - v_a) * (v_b - v_a);

	  csVector3 nearest;
	  if (dt_a <= 0)
	  {
	    nearest = v_a;
	  }
	  else
	  {
	    float dt_b = (v_c - v_b) * (v_a - v_b);
	    if (dt_b <= 0)
	      nearest = v_b;
	    else
	      nearest = v_a + ((v_b - v_a) * dt_a)/(dt_a + dt_b);
	  }

	  // Find the normal in the nearest point of the segment
	  // ==> Linear interpolation between vertexs
	  float AllDistance = csQsqrt (
	    csSquaredDist::PointPoint(seg.Start (), seg.End()));
	  float factorA = 1.0f - (csQsqrt (csSquaredDist::PointPoint (
	    seg.Start (),nearest) ) / AllDistance);
	  float factorB = 1.0f - (csQsqrt (csSquaredDist::PointPoint
	    (seg.End (),nearest) ) / AllDistance);
	  nearestNormals[act] = factorA * worldNormals[vertexs[act]]
	    + factorB * worldNormals[vertexs[((act+1)%vCount)]];

	  // Get the distance
	  distances[act] = csQsqrt (csSquaredDist::PointPoint(v,nearest));
	  if (distances[act] < shortestDistance)
	  {
	    nearestNormal = act;
	    shortestDistance = distances[act];
	  }
	}

	if (!poly->GetStaticPoly ()->PointOnPolygon (v))
	{
	  normal = nearestNormals[nearestNormal];
	}
	else
	{
	  normal.x = normal.y = normal.z = 0.0f;
	  for (act = 0; act < vCount ; act++)
	  {
	    if (distances[act] < 0.001f)
	      normal += nearestNormals[act];
	    else
	      normal += shortestDistance * nearestNormals[act]/distances[act];
	  }
	}

	normal.Normalize();
      }

      float cosinus = (v - lightpos) * normal;
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0) continue;
      else if (cosinus > 1) cosinus = 1;

      float scale = cosinus * light->GetBrightnessAtDistance (d) * lightness;

      int l;
      csRGBcolor &lumel = lightmap[uv];
      l = lumel.red + csQround (light_r * scale);
//    if (l<20) l = 20;
      lumel.red = l < 255 ? l : 255;
      l = lumel.green + csQround (light_g * scale);
//    if (l<20) l = 20;
      lumel.green = l < 255 ? l : 255;
      l = lumel.blue + csQround (light_b * scale);
//    if (l<20) l = 20;
      lumel.blue = l < 255 ? l : 255;
    }
  }
  delete [] distances;
  delete [] nearestNormals;
  delete [] worldNormals;
}

bool csShadowBitmap::UpdateShadowMap (
  unsigned char *shadowmap,
  int lightcell_shift,
  float shf_u,
  float shf_v,
  float mul_u,
  float mul_v,
  const csMatrix3 &m_t2w,
  const csVector3 &v_t2w,
  iLight *light,
  const csVector3 &lightpos,
  csPolygon3D* poly,
  const csPlane3& poly_world_plane,
  float cosfact)
{
  if (IsFullyShadowed () || IsFullyUnlit ()) return false;

  bool ful_lit = IsFullyLit ();
  int i, j, act;
  int base_uv = 0;
  float rv_step = (1 << lightcell_shift) * mul_v;
  float ru_step = (1 << lightcell_shift) * mul_u;
  float rv = shf_v * mul_v;
  float ru_base = shf_u * mul_u - ru_step;

  csVector3 v_ru (m_t2w.m11, m_t2w.m21, m_t2w.m31);
  csVector3 v_rv (m_t2w.m12, m_t2w.m22, m_t2w.m32);
  csVector3 v, v_base;

  v_base = v_t2w + rv * v_rv + ru_base * v_ru;
  v_rv *= rv_step;
  v_ru *= ru_step;

  csThing* poly_parent = poly->GetParent ();
  csThingStatic* static_data = poly_parent->GetStaticData ();
  bool do_smooth = static_data->GetSmoothingFlag();
  csSegment3* segments = 0;
  csVector3* nearest = 0;
  csVector3* nearestNormals = 0;
  float *distances = 0;
  float *weights = 0;
  csVector3* worldNormals = 0;
  if (do_smooth)
  {
    segments = new csSegment3[100];
    nearest = new csVector3[100];
    nearestNormals = new csVector3[100];
    distances = new float[100];
    weights = new float[100];
    csReversibleTransform mesh_transf = poly_parent->GetCachedMovable ()
    	->GetFullTransform ();
    worldNormals = new csVector3[static_data->num_vertices];
    csVector3* normals = static_data->GetNormals ();
    for (i = 0 ; i < static_data->num_vertices ; i++)
      worldNormals[i] = mesh_transf.This2OtherRelative (normals[i]);
  }

  bool relevant = false;
  for (i = 0; i < lm_h; i++)
  {
    int uv = base_uv;
    base_uv += lm_w;

    v = v_base;
    v_base += v_rv;

    for (j = 0; j < lm_w; j++, uv++)
    {
      v += v_ru;

      float lightness;
      if (ful_lit)
        lightness = 1;
      else
      {
        lightness = GetLighting (j, i);
        if (lightness < EPSILON) continue;
      }

      // @@@ Optimization: It should be possible to combine these

      // calculations into a more efficient formula.
      float d = csSquaredDist::PointPoint (lightpos, v);
      if (d >= csSquare (light->GetCutoffDistance ())) continue;

      d = csQsqrt (d);

      csVector3 normal = poly_world_plane.Normal ();
      if ( static_data->GetSmoothingFlag() )
      {
        int* vertexs = poly->GetStaticPoly ()->GetVertexIndices ();
        int vCount = poly->GetStaticPoly ()->GetVertexCount ();

        // int nearest;
        float shortestDistance = 10000000.0f;  // Big enought?
        int nearestNormal = 0;

        for (act = 0; act < vCount ; act++)
        {
          // Create the 3D Segments
          segments[act].SetStart (poly_parent->Vwor(vertexs[act]));
          segments[act].SetEnd (poly_parent->Vwor(vertexs[(act+1)%vCount]));

          // Find the nearest point from v to the segment
          // a: Start
          // b: End
          // c: point
          csVector3 v_a = segments[act].Start ();
          csVector3 v_b = segments[act].End ();
          csVector3 v_c = v;
          float dt_a = (v_c.x - v_a.x) * (v_b.x - v_a.x) +
                (v_c.y - v_a.y) * (v_b.y - v_a.y) +
                (v_c.z - v_a.z) * (v_b.z - v_a.z);
          float dt_b = (v_c.x - v_b.x) * (v_a.x - v_b.x) +
                (v_c.y - v_b.y) * (v_a.y - v_b.y) +
                (v_c.z - v_b.z) * (v_a.z - v_b.z);

          if (dt_a <= 0)
          {
            nearest[act] = v_a;
          }
          else
          {
            if (dt_b <=0)
            {
              nearest[act] = v_b;
            }
            else
            {
              nearest[act] = v_a + ((v_b - v_a) * dt_a)/(dt_a + dt_b);
            }
          }

          // Find the normal in the nearest point of the segment
          // ==> Linear interpolation between vertexs
          float AllDistance = csQsqrt (csSquaredDist::PointPoint
            (segments[act].Start (),segments[act].End()));
          float factorA = 1.0f - (csQsqrt (csSquaredDist::PointPoint
            (segments[act].Start (),nearest[act]) ) / AllDistance);
          float factorB = 1.0f - (csQsqrt (csSquaredDist::PointPoint
            (segments[act].End (),nearest[act]) ) / AllDistance);
          nearestNormals[act] = factorA * worldNormals[vertexs[act]]
            + factorB * worldNormals[vertexs[((act+1)%vCount)]];

          // Get the distance
          distances[act] = csQsqrt (csSquaredDist::PointPoint(v,nearest[act]));
          if (distances[act] < shortestDistance)
          {
            nearestNormal = act;
            shortestDistance = distances[act];
          }
        }

        // Get the weights of vertexes
        for (act = 0; act < vCount ; act++)
        {
          if (distances[act] < 0.001f)
            weights[act] = 1.0f;
          else
            weights[act] = shortestDistance / distances[act];
        }

        if (!poly->GetStaticPoly ()->PointOnPolygon (v))
        {
          normal = nearestNormals[nearestNormal];
        }
        else
        {
          normal.x = normal.y = normal.z = 0.0f;
          for (act = 0; act < vCount ; act++)
          {
            normal += weights[act]*nearestNormals[act];
          }
        }

        normal.Normalize();
      }

      float cosinus = (v - lightpos) * normal;
      cosinus /= d;
      cosinus += cosfact;
      if (cosinus < 0)
        continue;
      else if (cosinus > 1)
        cosinus = 1;

      float brightness = cosinus * light->GetBrightnessAtDistance (d);

      int l = shadowmap[uv] + csQround (
          CS_NORMAL_LIGHT_LEVEL * lightness * brightness);
      shadowmap[uv] = l < 255 ? l : 255;
      if ((!relevant) && shadowmap[uv] > 0) relevant = true;
    }
  }

  delete [] segments;
  delete [] nearest;
  delete [] nearestNormals;
  delete [] distances;
  delete [] weights;
  delete [] worldNormals;

  return relevant;
}

