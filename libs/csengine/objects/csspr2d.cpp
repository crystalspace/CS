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

#define SYSDEF_ALLOCA
#include "cssysdef.h"
#include "csgeom/polyclip.h"
#include "csengine/csspr2d.h"
#include "csengine/engine.h"
#include "csengine/sector.h"
#include "csengine/pol2d.h"
#include "csengine/light.h"
#include <math.h>

IMPLEMENT_CSOBJTYPE (csSprite2D, csSprite)

// This empty SCF extension works around a bizarre NextStep compiler bug which
// manifests as an apparent corruption of the virtual table for csSprite2D.
// Calls to QueryInterface() from a csSprite2D pointer would never actually
// invoke the real, inherited csSprite::QueryInterface(), and would instead
// always return NULL.  It is not even clear which, if any, method was being
// called in lieu of the real csSprite::QueryInterface().  Calls to
// QueryInterface() from a local instance of csSprite2D would succeed as
// expected (since the virtual table is not consulted in such cases).  The work
// around for this problem (for magical reasons) is to declare QueryInterface()
// in csSprite2D which overrides the inherited csSprite::QueryInterface().  It
// is sufficient for this method to simply exist in csSprite2D.  Its actual
// implementation merely invokes its superclass' QueryInterface().  The macros
// below embody this work-around.
IMPLEMENT_IBASE_EXT (csSprite2D)
IMPLEMENT_IBASE_EXT_END

csSprite2D::csSprite2D (csObject* theParent) : csSprite (theParent)
{
  cstxt = NULL;
  lighting = true;
  ptree_obj = NULL;	//@@@
}

csSprite2D::~csSprite2D ()
{
}

void csSprite2D::CreateRegularVertices (int n, bool setuv)
{
  double angle_inc = 2.0 * PI / n;
  double angle = 0.0;
  vertices.SetLimit(n);
  vertices.SetLength(n);
  for (int i = 0; i < vertices.Length (); i++, angle += angle_inc)
  {
    vertices [i].pos.y = cos (angle);
    vertices [i].pos.x = sin (angle);
    if (setuv) 
    {
      // reuse sin/cos values and scale to [0..1]
      vertices [i].u = vertices [i].pos.x / 2.0f + 0.5f;
      vertices [i].v = vertices [i].pos.y / 2.0f + 0.5f;
    }
  }
}

void csSprite2D::UpdateInPolygonTrees ()
{
}

void csSprite2D::ScaleBy (float factor)
{
  for (int i = 0; i < vertices.Length (); i++)
    vertices [i].pos *= factor;
}


void csSprite2D::Rotate (float angle)
{
  for (int i = 0; i < vertices.Length (); i++)
    vertices [i].pos.Rotate (angle);
}


void csSprite2D::Shift (float dx, float dy)
{
  for (int i = 0; i < vertices.Length (); i++)
  {
    vertices [i].pos.x += dx;
    vertices [i].pos.y += dy;
  }
}


void csSprite2D::SetColor (const csColor& col)
{
  for (int i = 0; i < vertices.Length (); i++)
    vertices [i].color_init = col;
  if (!lighting)
    for (int i = 0; i < vertices.Length (); i++)
      vertices [i].color = col;
}


void csSprite2D::AddColor (const csColor& col)
{
  for (int i = 0; i < vertices.Length (); i++)
    vertices [i].color_init += col;
  if (!lighting)
    for (int i = 0; i < vertices.Length (); i++)
      vertices [i].color = vertices [i].color_init;
}

void csSprite2D::SetLighting (bool l)
{
  lighting = l;
  if (!lighting)
  {
    int i;
    for (i = 0; i < vertices.Length (); i++)
      vertices [i].color = vertices [i].color_init;
  }
}


void csSprite2D::UpdateLighting (csLight** lights, int num_lights)
{
  defered_num_lights = 0;
  if (!lighting) return;
  csColor color (0, 0, 0);

  csSector* sect = movable.GetSector (0);
  if (sect)
  {
    int r, g, b;
    sect->GetAmbientColor (r, g, b);
    color.Set (r / 128.0, g / 128.0, b / 128.0);
  }

  int i;
  for (i = 0; i < num_lights; i++)
  {
    csColor light_color = lights[i]->GetColor () * (256. / NORMAL_LIGHT_LEVEL);
    float sq_light_radius = lights [i]->GetSquaredRadius ();
    // Compute light position.
    csVector3 wor_light_pos = lights [i]->GetCenter ();
    float wor_sq_dist =
      csSquaredDist::PointPoint (wor_light_pos, GetPosition ());
    if (wor_sq_dist >= sq_light_radius) continue;
    float wor_dist = sqrt (wor_sq_dist);
    float cosinus = 1.;
    cosinus /= wor_dist;
    light_color *= cosinus * lights [i]->GetBrightnessAtDistance (wor_dist);
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
    CsPrintf (MSG_FATAL_ERROR,
      "Error! Trying to draw a 2D sprite with no texture!\n");
    fatal_exit (0, false);
  }

  // Camera transformation for the single 'position' vector.
  csVector3 cam = rview.Other2This (GetPosition ());
  if (cam.z < SMALL_Z) return;

  UpdateDeferedLighting (GetPosition ());

  rview.g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);

  static G3DPolygonDPFX g3dpolyfx;
  g3dpolyfx.num = vertices.Length ();
  cstxt->Visit ();
  g3dpolyfx.mat_handle = cstxt->GetMaterialHandle ();
  g3dpolyfx.inv_aspect = rview.GetInvFOV ();
  g3dpolyfx.mat_handle->GetTexture ()->GetMeanColor (g3dpolyfx.flat_color_r,
    g3dpolyfx.flat_color_g, g3dpolyfx.flat_color_b);

  ALLOC_STACK_ARRAY (poly2d, csVector2, vertices.Length ());
  csVector2 clipped_poly2d[MAX_OUTPUT_VERTICES];
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];

  float iz = 1. / cam.z;
  float iza = iz * rview.GetFOV ();

  int i;
  for (i = 0; i < vertices.Length (); i++)
  {
    g3dpolyfx.vertices [i].z = iz;
    g3dpolyfx.vertices [i].sx =
    poly2d [i].x = (cam.x + vertices [i].pos.x) * iza + rview.GetShiftX ();
    g3dpolyfx.vertices [i].sy =
    poly2d [i].y = (cam.y + vertices [i].pos.y) * iza + rview.GetShiftY ();
    g3dpolyfx.vertices [i].u = vertices [i].u;
    g3dpolyfx.vertices [i].v = vertices [i].v;
    g3dpolyfx.vertices [i].r = vertices [i].color.red;
    g3dpolyfx.vertices [i].g = vertices [i].color.green;
    g3dpolyfx.vertices [i].b = vertices [i].color.blue;
  }

  int num_clipped_verts;
  UByte clip_result = rview.view->Clip (poly2d, vertices.Length (),
    clipped_poly2d, num_clipped_verts, clipped_vtstats);
  if (clip_result == CS_CLIP_OUTSIDE) return;
  g3dpolyfx.num = num_clipped_verts;

  if (clip_result != CS_CLIP_INSIDE)
    PreparePolygonFX2 (&g3dpolyfx, clipped_poly2d, num_clipped_verts,
    	clipped_vtstats, vertices.Length (), true);

  rview.CalculateFogPolygon (g3dpolyfx);
  rview.g3d->StartPolygonFX (g3dpolyfx.mat_handle,
    	MixMode | CS_FX_GOURAUD);
  rview.g3d->DrawPolygonFX (g3dpolyfx);
  rview.g3d->FinishPolygonFX ();
}
