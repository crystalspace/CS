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
#include "qint.h"
#include "polygon.h"
#include "pol2d.h"
#include "polytext.h"
#include "lppool.h"
#include "lghtmap.h"
#include "portal.h"
#include "iengine/camera.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/dynlight.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/txtmgr.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"


CS_IMPLEMENT_STATIC_CLASSVAR(csPolygon2DFactory,sharedFactory,SharedFactory,csPolygon2DFactory,())

static void Perspective (const csVector3& v, csVector2& p, int
	aspect, float shift_x, float shift_y)
{
  float iz = aspect / v.z;
  p.x = v.x * iz + shift_x;
  p.y = v.y * iz + shift_y;
}

void csPolygon2D::AddPerspective (const csVector3 &v, int aspect,
	float shift_x, float shift_y)
{
  if (num_vertices >= max_vertices) MakeRoom (max_vertices + 5);
  Perspective (v, vertices[num_vertices], aspect, shift_x, shift_y);
  bbox.AddBoundingVertex (vertices[num_vertices]);
  num_vertices++;
}

void csPolygon2D::AddPerspectiveUnit (const csVector3 &v)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices + 5);
  float iz = 1.0f / v.z;
  vertices[num_vertices].x = v.x * iz;
  vertices[num_vertices].y = v.y * iz;
  bbox.AddBoundingVertex (vertices[num_vertices]);
  num_vertices++;
}

void csPolygon2D::AddPerspectiveAspect (const csVector3 &v, float ratio,
  float shift)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices + 5);
  float iz = ratio / v.z;
  vertices[num_vertices].x = v.x * iz + shift;
  vertices[num_vertices].y = v.y * iz + shift;
  bbox.AddBoundingVertex (vertices[num_vertices]);
  num_vertices++;
}

void csPolygon2D::Draw (iGraphics2D * g2d, int col)
{
  int i;
  int x1, y1, x2, y2;
  
  if (!num_vertices) return;
  x1 = QRound (vertices[num_vertices - 1].x);
  y1 = QRound (vertices[num_vertices - 1].y);
  for (i = 0; i < num_vertices ; i++)
  {
    x2 = QRound(vertices[i].x);
    y2 = QRound (vertices[i].y);
    g2d->DrawLine (x1, g2d->GetHeight () - 1 - y1, x2,
    	g2d->GetHeight () - 1 - y2, col);
    x1 = x2;
    y1 = y2;
  }
}

//---------------------------------------------------------------------------
void csPolygon2D::DrawFilled (
  iRenderView *rview,
  csPolygon3D *poly,
  const csPlane3& camera_plane,
  csZBufMode zbufMode)
{
  csPolygon3DStatic* spoly = poly->GetStaticData ();
  if (!spoly->IsTextureMappingEnabled ()) return;
  int i;
  bool debug = false;
  iCamera *icam = rview->GetCamera ();
  bool mirror = icam->IsMirrored ();

  rview->GetGraphics3D ()->SetRenderState (
      G3DRENDERSTATE_ZBUFFERMODE,
      zbufMode);

  static G3DPolygonDP g3dpoly;

  g3dpoly.num = num_vertices;
  iMaterialWrapper* rm = poly->GetRealMaterial ();
  if (rm) rm->Visit ();
  g3dpoly.mat_handle = rm ? rm->GetMaterialHandle () : 0;
  g3dpoly.mixmode = spoly->GetMixMode ();
  g3dpoly.mixmode &= ~(CS_FX_ALPHA | CS_FX_MASK_ALPHA);
  if (spoly->GetAlpha ())
    g3dpoly.mixmode |= CS_FX_SETALPHA_INT (spoly->GetAlpha ());

  // We are going to use DrawPolygon.
  if (mirror)
  {
    for (i = 0; i < num_vertices; i++)
    {
      g3dpoly.vertices[num_vertices - i - 1].x = vertices[i].x;
      g3dpoly.vertices[num_vertices - i - 1].y = vertices[i].y;
    }
  }
  else
  {
    memcpy (g3dpoly.vertices, vertices, num_vertices * sizeof (csVector2));
  }

  g3dpoly.z_value = poly->Vcam (0).z;
  csPolyTexture* lmi = poly->GetPolyTexture ();
  //g3dpoly.poly_texture = lmi;
  g3dpoly.lmap = lmi->GetLMapping ();
  g3dpoly.texmap = lmi->GetTMapping ();
  g3dpoly.rlm = lmi->GetRendererLightmap ();
  g3dpoly.do_fullbright = spoly->flags.Check (CS_POLY_LM_REFUSED);

  csMatrix3 m_cam2tex;
  csVector3 v_cam2tex;
  lmi->WorldToCamera (icam->GetTransform (), m_cam2tex, v_cam2tex);
  g3dpoly.cam2tex.m_cam2tex = &m_cam2tex;
  g3dpoly.cam2tex.v_cam2tex = &v_cam2tex;
  g3dpoly.normal = camera_plane;

  if (debug)
    rview->GetGraphics3D ()->DrawPolygonDebug (g3dpoly);
  else
  {
    rview->CalculateFogPolygon (g3dpoly);
    rview->GetGraphics3D ()->DrawPolygon (g3dpoly);
  }
}

void csPolygon2D::FillZBuf (
  iRenderView *rview,
  csPolygon3D *poly,
  const csPlane3& camera_plane)
{

  rview->GetGraphics3D ()->SetRenderState (
      G3DRENDERSTATE_ZBUFFERMODE,
      CS_ZBUF_FILLONLY);

  iCamera *icam = rview->GetCamera ();

  static G3DPolygonDP g3dpoly;
  g3dpoly.mixmode = CS_FX_COPY;
  g3dpoly.num = num_vertices;

  // We are going to use DrawPolygon.
  int i;
  if (icam->IsMirrored ())
  {
    for (i = 0; i < num_vertices; i++)
    {
      g3dpoly.vertices[num_vertices - i - 1].x = vertices[i].x;
      g3dpoly.vertices[num_vertices - i - 1].y = vertices[i].y;
    }
  }
  else
  {
    memcpy (g3dpoly.vertices, vertices, num_vertices * sizeof (csVector2));
  }

  g3dpoly.z_value = poly->Vcam (0).z;
  g3dpoly.normal = camera_plane;

  rview->GetGraphics3D ()->DrawPolygon (g3dpoly);
}

void csPolygon2D::AddFogPolygon (
  iGraphics3D *g3d,
  csPolygon3D* /*poly*/,
  const csPlane3& camera_plane,
  bool mirror,
  CS_ID id,
  int fogtype)
{
  int i;

  static G3DPolygonDFP g3dpoly;
  memset (&g3dpoly, 0, sizeof (g3dpoly));
  g3dpoly.num = num_vertices;
  if (mirror)
  {
    for (i = 0; i < num_vertices; i++)
    {
      g3dpoly.vertices[num_vertices - i - 1].x = vertices[i].x;
      g3dpoly.vertices[num_vertices - i - 1].y = vertices[i].y;
    }
  }
  else
  {
    memcpy (g3dpoly.vertices, vertices, num_vertices * sizeof (csVector2));
  }

  //g3dpoly.polygon = GetIPolygon3DFromcsPolygon3D(poly); //DPQFIX
  g3dpoly.normal = camera_plane;

  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);
  g3d->DrawFogPolygon (id, g3dpoly, fogtype);
}

//---------------------------------------------------------------------------
csPolygon2DQueue::csPolygon2DQueue (int max_size)
{
  queue = new csQueueElement[max_size];
  max_queue = max_size;
  num_queue = 0;
}

csPolygon2DQueue::~csPolygon2DQueue ()
{
  delete[] queue;
}

void csPolygon2DQueue::Push (csPolygon3D *poly3d, csPolygon2D *poly2d)
{
  queue[num_queue].poly3d = poly3d;
  queue[num_queue].poly2d = poly2d;
  num_queue++;
}

bool csPolygon2DQueue::Pop (csPolygon3D **poly3d, csPolygon2D **poly2d)
{
  if (num_queue <= 0) return false;
  num_queue--;
  *poly3d = queue[num_queue].poly3d;
  *poly2d = queue[num_queue].poly2d;
  return true;
}
