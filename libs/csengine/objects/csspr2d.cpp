/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "csgeom/polyclip.h"
#include "csengine/csspr2d.h"
#include "csengine/world.h"
#include "csengine/sector.h"
#include "csengine/pol2d.h"

//=============================================================================

IMPLEMENT_CSOBJTYPE (csSprite2D, csSprite)

csSprite2D::csSprite2D () : csSprite (), position (0, 0, 0)
{
  cstxt = NULL;
  lighting = true;
}

csSprite2D::~csSprite2D ()
{
}

void csSprite2D::UpdatePolyTreeBBox ()
{
}

void csSprite2D::SetLighting (bool l)
{
  lighting = l;
  if (!lighting)
  {
    int i;
    for (i = 0 ; i < vertices.Length () ; i++)
      vertices[i].color = vertices[i].color_init;
  }
}

void csSprite2D::UpdateLighting (csLight** lights, int num_lights)
{
  defered_num_lights = 0;
  if (!lighting) return;
  csColor color (0, 0, 0);

  csSector* sect = (csSector*)sectors[0];
  if (sect)
  {
    int r, g, b;
    sect->GetAmbientColor (r, g, b);
    color.Set (r / 128.0, g / 128.0, b / 128.0);
  }

  int i;
  for (i = 0 ; i < num_lights ; i++)
  {
    csColor light_color = lights[i]->GetColor () * (256. / NORMAL_LIGHT_LEVEL);
    float sq_light_radius = lights[i]->GetSquaredRadius ();
    // Compute light position.
    csVector3 wor_light_pos = lights[i]->GetCenter ();
    float wor_sq_dist = csSquaredDist::PointPoint (wor_light_pos, position);
    if (wor_sq_dist >= sq_light_radius) continue;
    float wor_dist = sqrt (wor_sq_dist);
    float cosinus = 1.;
    cosinus /= wor_dist;
    light_color *= cosinus * lights[i]->GetBrightnessAtDistance (wor_dist);
    color += light_color;
  }
  for (i = 0 ; i < vertices.Length () ; i++)
  {
    vertices[i].color = vertices[i].color_init + color;
    vertices[i].color.Clamp (2, 2, 2);
  }
}

void csSprite2D::Draw (csRenderView& rview)
{
  if (!cstxt)
  {
    CsPrintf (MSG_FATAL_ERROR, "Error! Trying to draw a 2D sprite with no texture!\n");
    fatal_exit (0, false);
  }

  UpdateDeferedLighting (position);

  // Camera transformation for the single 'position' vector.
  csVector3 cam = rview.Other2This (position);
  if (cam.z < SMALL_Z) return;

  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);

  static G3DPolygonDPFX g3dpolyfx;
  g3dpolyfx.num = vertices.Length ();
  g3dpolyfx.txt_handle = cstxt->GetTextureHandle ();
  g3dpolyfx.inv_aspect = rview.inv_aspect;
  g3dpolyfx.txt_handle->GetMeanColor (g3dpolyfx.flat_color_r,
      g3dpolyfx.flat_color_g, g3dpolyfx.flat_color_b);

  csVector2 poly2d[50]; 	// @@@
  csVector2 clipped_poly2d[MAX_OUTPUT_VERTICES];
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];

  float iz = 1. / cam.z;
  float iza = iz * rview.aspect;

  int i;
  for (i = 0 ; i < vertices.Length () ; i++)
  {
    g3dpolyfx.vertices[i].z = iz;
    poly2d[i].x = (cam.x+vertices[i].pos.x) * iza + rview.shift_x;
    poly2d[i].y = (cam.y+vertices[i].pos.y) * iza + rview.shift_y;
    g3dpolyfx.vertices[i].sx = poly2d[i].x;
    g3dpolyfx.vertices[i].sy = poly2d[i].y;
    g3dpolyfx.vertices[i].u = vertices[i].u;
    g3dpolyfx.vertices[i].v = vertices[i].v;
    g3dpolyfx.vertices[i].r = vertices[i].color.red;
    g3dpolyfx.vertices[i].g = vertices[i].color.green;
    g3dpolyfx.vertices[i].b = vertices[i].color.blue;
  }

  int num_clipped_verts;
  UByte clip_result = rview.view->Clip (poly2d, vertices.Length (),
  	clipped_poly2d, num_clipped_verts, clipped_vtstats);
  if (clip_result == CS_CLIP_OUTSIDE) return;
  g3dpolyfx.num = num_clipped_verts;

  if (clip_result != CS_CLIP_INSIDE)
    PreparePolygonFX2 (&g3dpolyfx, clipped_poly2d, num_clipped_verts,
    	clipped_vtstats, poly2d, vertices.Length (), true);

  extern void CalculateFogPolygon (csRenderView* rview, G3DPolygonDPFX& poly);
  CalculateFogPolygon (&rview, g3dpolyfx);
  rview.g3d->StartPolygonFX (g3dpolyfx.txt_handle,
    	MixMode | CS_FX_GOURAUD);
  rview.g3d->DrawPolygonFX (g3dpolyfx);
  rview.g3d->FinishPolygonFX ();
}
