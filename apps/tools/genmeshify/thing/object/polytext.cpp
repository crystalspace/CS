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

int csPolyTexture::GetLightCellSize ()
{
  return csLightMap::lightcell_size;
}

int csPolyTexture::GetLightCellShift ()
{
  return csLightMap::lightcell_shift;
}
