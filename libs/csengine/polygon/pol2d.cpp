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

#define SYSDEF_ALLOCA
#include "cssysdef.h"
#include "qint.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/polytmap.h"
#include "csengine/texture.h"
#include "csengine/polyplan.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"
#include "csengine/camera.h"
#include "csengine/portal.h"
#include "csengine/dumper.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "ivideo/txtmgr.h"

// An ugly hack to avoid "local relocation entries in non-writable section"
// linkage error on OpenStep/HPPA/Sparc when csEngine library is linked into
// a plug-in.  @@@FIXME: Work around this in a cleaner way in the future.
#if !defined(OS_NEXT) || (!defined(PROC_SPARC) && !defined(PROC_HPPA))
#  define CS_LOCAL_STATIC(X,Y) static X Y
#else
#  define CS_LOCAL_STATIC(X,Y) \
     static X* Y##__ = 0; \
     if (Y##__ == 0) Y##__ = new X; \
     X& Y = *Y##__
#endif

csPolygon2DFactory* csPolygon2DFactory::SharedFactory()
{
  CS_LOCAL_STATIC(csPolygon2DFactory,p);
  return &p;
}

void csPolygon2D::AddPerspective (const csVector3& v)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices+5);

  csEngine::current_engine->current_camera->Perspective (v, vertices[num_vertices]);
  bbox.AddBoundingVertex (vertices[num_vertices]);
  num_vertices++;
}

void csPolygon2D::AddPerspectiveUnit (const csVector3& v)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices+5);

  float iz = 1./v.z;
  vertices[num_vertices].x = v.x * iz;
  vertices[num_vertices].y = v.y * iz;
  bbox.AddBoundingVertex (vertices[num_vertices]);
  num_vertices++;
}

void csPolygon2D::AddPerspectiveAspect (const csVector3& v,
	float ratio, float shift)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices+5);

  float iz = ratio/v.z;
  vertices[num_vertices].x = v.x * iz + shift;
  vertices[num_vertices].y = v.y * iz + shift;
  bbox.AddBoundingVertex (vertices[num_vertices]);
  num_vertices++;
}


void csPolygon2D::Draw (iGraphics2D* g2d, int col)
{
  int i;
  int x1, y1, x2, y2;

  if (!num_vertices) return;

  x1 = QRound (vertices[num_vertices-1].x);
  y1 = QRound (vertices[num_vertices-1].y);
  for (i = 0 ; i < num_vertices ; i++)
  {
    x2 = QRound (vertices[i].x);
    y2 = QRound (vertices[i].y);
    g2d->DrawLine (x1, csEngine::frame_height - 1 - y1, x2, csEngine::frame_height - 1 - y2, col);

    x1 = x2;
    y1 = y2;
  }
}

//---------------------------------------------------------------------------

#define INTERPOLATE1(component) \
  g3dpoly->vertices [i].##component## = inpoly [vt].##component## + \
    t * (inpoly [vt2].##component## - inpoly [vt].##component##);

#define INTERPOLATE(component) \
{ \
  float v1 = inpoly [edge_from [0]].##component## + \
    t1 * (inpoly [edge_to [0]].##component## - inpoly [edge_from [0]].##component##); \
  float v2 = inpoly [edge_from [1]].##component## + \
    t2 * (inpoly [edge_to [1]].##component## - inpoly [edge_from [1]].##component##); \
  g3dpoly->vertices [i].##component## = v1 + t * (v2 - v1); \
}

void PreparePolygonFX2 (G3DPolygonDPFX* g3dpoly,
  csVector2* clipped_verts, int num_vertices, csVertexStatus* clipped_vtstats,
  int orig_num_vertices, bool gouraud)
{
  // first we copy the first texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  ALLOC_STACK_ARRAY (inpoly, G3DTexturedVertex, orig_num_vertices);
  int i;
  for (i = 0; i < orig_num_vertices; i++)
    inpoly[i] = g3dpoly->vertices[i];

  int vt, vt2;
  float t;
  for (i = 0; i < num_vertices; i++)
  {
    g3dpoly->vertices [i].sx = clipped_verts [i].x;
    g3dpoly->vertices [i].sy = clipped_verts [i].y;
    switch (clipped_vtstats[i].Type)
    {
      case CS_VERTEX_ORIGINAL:
        vt = clipped_vtstats[i].Vertex;
        g3dpoly->vertices [i].z = inpoly [vt].z;
        g3dpoly->vertices [i].u = inpoly [vt].u;
        g3dpoly->vertices [i].v = inpoly [vt].v;
	if (gouraud)
	{
          g3dpoly->vertices [i].r = inpoly [vt].r;
          g3dpoly->vertices [i].g = inpoly [vt].g;
          g3dpoly->vertices [i].b = inpoly [vt].b;
	}
	break;
      case CS_VERTEX_ONEDGE:
        vt = clipped_vtstats[i].Vertex;
	vt2 = vt + 1; if (vt2 >= orig_num_vertices) vt2 = 0;
	t = clipped_vtstats [i].Pos;
	INTERPOLATE1 (z);
	INTERPOLATE1 (u);
	INTERPOLATE1 (v);
	if (gouraud)
	{
	  INTERPOLATE1 (r);
	  INTERPOLATE1 (g);
	  INTERPOLATE1 (b);
	}
	break;
      case CS_VERTEX_INSIDE:
        float x = clipped_verts [i].x;
        float y = clipped_verts [i].y;
        int edge_from [2], edge_to [2];
	int edge = 0;
	int j, j1;
	j1 = orig_num_vertices - 1;
	for (j = 0; j < orig_num_vertices; j++)
	{
          if ((y >= inpoly [j].sy && y <= inpoly [j1].sy) ||
	      (y <= inpoly [j].sy && y >= inpoly [j1].sy))
	  {
	    edge_from [edge] = j;
	    edge_to [edge] = j1;
	    edge++;
	    if (edge >= 2) break;
	  }
	  j1 = j;
	}
	if (edge == 1)
	{
	  // Safety if we only found one edge.
	  edge_from [1] = edge_from [0];
	  edge_to [1] = edge_to [0];
	}
	G3DTexturedVertex& A = inpoly [edge_from [0]];
	G3DTexturedVertex& B = inpoly [edge_to [0]];
	G3DTexturedVertex& C = inpoly [edge_from [1]];
	G3DTexturedVertex& D = inpoly [edge_to [1]];
	float t1 = (y - A.sy) / (B.sy - A.sy);
	float t2 = (y - C.sy) / (D.sy - C.sy);
	float x1 = A.sx + t1 * (B.sx - A.sx);
	float x2 = C.sx + t2 * (D.sx - C.sx);
	t = (x - x1) / (x2 - x1);
	INTERPOLATE (z);
	INTERPOLATE (u);
	INTERPOLATE (v);
	if (gouraud)
	{
	  INTERPOLATE (r);
	  INTERPOLATE (g);
	  INTERPOLATE (b);
	}
	break;
    }
  }
}

#undef INTERPOLATE
#undef INTERPOLATE1

void PreparePolygonFX (G3DPolygonDPFX* g3dpoly, csVector2* clipped_verts,
  int num_vertices, csVector2* orig_triangle, bool gouraud)
{
  // Note: Assumes clockwise vertices, otherwise wouldn't be visible :).
  // 'was_clipped' will be true if the triangle was clipped.
  // This is the case if rescount != 3 (because we then don't have
  // a triangle) or else if any of the clipped vertices is different.
  bool was_clipped = (num_vertices != 3);
  int j;
  for (j = 0; j < num_vertices; j++)
  {
    g3dpoly->vertices [j].sx = clipped_verts [j].x;
    g3dpoly->vertices [j].sy = clipped_verts [j].y;
    if (!was_clipped && clipped_verts[j] != orig_triangle[j])
    	was_clipped = true;
  }

  // If it was not clipped we don't have to do anything.
  if (!was_clipped) return;

  // first we copy the first three texture coordinates to a local buffer
  // to avoid that they are overwritten when interpolating.
  G3DTexturedVertex inpoly[3];
  for (int i = 0; i < 3; i++)
    inpoly [i] = g3dpoly->vertices [i];

  // Now we have to find the u,v coordinates for every
  // point in the clipped polygon. We know we started
  // from orig_triangle and that texture mapping is not perspective correct.

  // Compute U & V in vertices of the polygon
  // First find the topmost triangle vertex
  int top;
  if (orig_triangle [0].y < orig_triangle [1].y)
    if (orig_triangle [0].y < orig_triangle [2].y)
      top = 0;
    else
      top = 2;
  else
    if (orig_triangle [1].y < orig_triangle [2].y)
      top = 1;
    else
      top = 2;

  int _vbl, _vbr;
  if (top <= 0) _vbl = 2; else _vbl = top - 1;
  if (top >= 2) _vbr = 0; else _vbr = top + 1;

  // Rare special case is when triangle edge on, vertices satisfy
  //  *--------->x     a == b && (a.y < c.y) && (a.x > c.x)
  //  |  *a,b          and is clipped at c, where orig_triangle[0]
  //  | /              can be either a, b or c. In other words when
  //  |/               the single vertex is not 'top' and clipped.
  // /|*c              
  //  |                The '-= EPSILON' for both left and right 
  //  y     fig. 2     is fairly arbitrary, this probably needs to be refined.

  if (orig_triangle[top] == orig_triangle[_vbl]) 
      orig_triangle[_vbl].x -= EPSILON;
  if (orig_triangle[top] == orig_triangle[_vbr]) 
      orig_triangle[_vbr].x -= EPSILON;

  for (j = 0 ; j < g3dpoly->num ; j++)
  {
    float x = g3dpoly->vertices [j].sx;
    float y = g3dpoly->vertices [j].sy;

    // Find the original triangle top/left/bottom/right vertices
    // between which the currently examined point is located.
    // There are two possible major cases:
    // A*       A*       When DrawPolygonFX works, it will switch
    //  |\       |\      start/final values and deltas ONCE (not more)
    //  | \      | \     per triangle.On the left pictures this happens
    //  |*X\     |  \    at the point B. This means we should "emulate"
    //  |   *B   |   *B  this switch by taking different start/final values
    //  |  /     |*X/    for interpolation if examined point X is below
    //  | /      | /     the point B.
    //  |/       |/
    // C*       C*  Fig.1 :-)
    int vtl = top, vtr = top, vbl = _vbl, vbr = _vbr;
    int ry = QRound (y); 
    if (ry > QRound (orig_triangle [vbl].y))
    {
      vtl = vbl;
      if (--vbl < 0) vbl = 2;
    }
    else if (ry > QRound (orig_triangle [vbr].y))
    {
      vtr = vbr;
      if (++vbr > 2) vbr = 0;
    }

    // Now interpolate Z,U,V,R,G,B by Y
    float tL, tR, xL, xR, tX;
    if (QRound (orig_triangle [vbl].y) != QRound (orig_triangle [vtl].y))
      tL = (y - orig_triangle [vtl].y) / (orig_triangle [vbl].y - orig_triangle [vtl].y);
    else
	tL = (x - orig_triangle [vtl].x) / (orig_triangle [vbl].x - orig_triangle [vtl].x);
    if (QRound (orig_triangle [vbr].y) != QRound (orig_triangle [vtr].y))
      tR = (y - orig_triangle [vtr].y) / (orig_triangle [vbr].y - orig_triangle [vtr].y);
    else
	tR = (x - orig_triangle [vtr].x) / (orig_triangle [vbr].x - orig_triangle [vtr].x);

    xL = orig_triangle [vtl].x + tL * (orig_triangle [vbl].x - orig_triangle [vtl].x);
    xR = orig_triangle [vtr].x + tR * (orig_triangle [vbr].x - orig_triangle [vtr].x);
    tX = xR - xL;
    if (tX) tX = (x - xL) / tX;

#   define INTERPOLATE(val,tl,bl,tr,br)	\
    {					\
      float vl,vr;				\
      if (tl != bl)				\
        vl = tl + (bl - tl) * tL;		\
      else					\
        vl = tl;				\
      if (tr != br)				\
        vr = tr + (br - tr) * tR;		\
      else					\
        vr = tr;				\
      val = vl + (vr - vl) * tX;		\
    }

    // Calculate Z
    INTERPOLATE(g3dpoly->vertices [j].z,
                inpoly [vtl].z, inpoly [vbl].z,
                inpoly [vtr].z, inpoly [vbr].z);
    if (g3dpoly->mat_handle)
    {
      // Calculate U
      INTERPOLATE(g3dpoly->vertices [j].u,
                  inpoly [vtl].u, inpoly [vbl].u,
                  inpoly [vtr].u, inpoly [vbr].u);
      // Calculate V
      INTERPOLATE(g3dpoly->vertices [j].v,
                  inpoly [vtl].v, inpoly [vbl].v,
                  inpoly [vtr].v, inpoly [vbr].v);
    }
    if (gouraud)
    {
      // Calculate R
      INTERPOLATE(g3dpoly->vertices [j].r,
                  inpoly [vtl].r, inpoly [vbl].r,
                  inpoly [vtr].r, inpoly [vbr].r);
      // Calculate G
      INTERPOLATE (g3dpoly->vertices [j].g,
                  inpoly [vtl].g, inpoly [vbl].g,
                  inpoly [vtr].g, inpoly [vbr].g);
      // Calculate B
      INTERPOLATE (g3dpoly->vertices [j].b,
                  inpoly [vtl].b, inpoly [vbl].b,
                  inpoly [vtr].b, inpoly [vbr].b);
    }
    else
    {
    
      g3dpoly->vertices[j].r = 1;
      g3dpoly->vertices[j].g = 1;
      g3dpoly->vertices[j].b = 1;
      
    }
  }
}

//---------------------------------------------------------------------------

void csPolygon2D::DrawFilled (iRenderView* rview, csPolygon3D* poly,
  csPolyPlane* plane, csZBufMode zbufMode)
{
  int i;
  bool debug = false;
  iCamera* icam = rview->GetCamera ();
  bool mirror = icam->IsMirrored ();

  rview->GetGraphics3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE,
  	zbufMode);

  if (poly->GetTextureType () != POLYTXT_LIGHTMAP)
  {
    // We are going to use DrawPolygonFX
    // Add all dynamic lights if polygon is dirty.
    csPolygon3D *unsplit;
    unsplit = poly->GetBasePolygon ();
    const bool do_light = poly->flags.Check (CS_POLY_LIGHTING);
    if (do_light)
    {
      if (unsplit->IsDirty ())
      {
        csLightPatch* lp = unsplit->GetLightpatches ();
        if (lp)
        {
          // If there is at least one light patch we do the lighting reset
	  // in the first call to UpdateVertexLighting().
          bool reset = true;
          while (lp)
          {
	    iLight* il = QUERY_INTERFACE (lp->GetLight (), iLight);
            unsplit->UpdateVertexLighting (il,
	    	lp->GetLight ()->GetColor (), true, reset);
	    il->DecRef ();
	    reset = false;
            lp = lp->GetNextPoly ();
          }
        }
        else
        {
          // There are no more lightpatches. In this case we need to restore
	  // lighting to the static lighting. This can be done by calling
	  // UpdateVertexLighting() with no light and reset set to true.
	  unsplit->UpdateVertexLighting (NULL, csColor (0, 0, 0), true, true);
        }
        unsplit->MakeCleanDynamicLights ();
      }
    }

    CS_LOCAL_STATIC(G3DPolygonDPFX,g3dpolyfx);

    csPolyTexType *ns = poly->GetNoTexInfo ();
    csPolyTexFlat *fs = poly->GetFlatInfo ();
    csPolyTexGouraud *gs = poly->GetGouraudInfo ();

    g3dpolyfx.num = num_vertices;
    if (poly->GetMaterialWrapper ())
      poly->GetMaterialWrapper ()->Visit ();
    g3dpolyfx.mat_handle = poly->GetMaterialHandle ();
    g3dpolyfx.inv_aspect = icam->GetInvFOV ();

    csColor *po_colors = do_light && gs ? gs->GetColors () : NULL;

    float aspect = icam->GetFOV ();
    float shift_x = icam->GetShiftX ();
    float shift_y = icam->GetShiftY ();

    // Here we have to do a little messy thing because PreparePolygonFX()
    // still requires the original triangle that was valid before clipping.
    csVector2 orig_triangle[3];
    float iz = 1. / unsplit->Vcam (0).z;
    g3dpolyfx.vertices[0].z = iz;
    iz *= aspect;
    orig_triangle[0].x = unsplit->Vcam (0).x * iz + shift_x;
    orig_triangle[0].y = unsplit->Vcam (0).y * iz + shift_y;

    iz = 1. / unsplit->Vcam (1).z;
    g3dpolyfx.vertices[1].z = iz;
    iz *= aspect;
    orig_triangle[1].x = unsplit->Vcam (1).x * iz + shift_x;
    orig_triangle[1].y = unsplit->Vcam (1).y * iz + shift_y;

    iz = 1. / unsplit->Vcam (2).z;
    g3dpolyfx.vertices[2].z = iz;
    iz *= aspect;
    orig_triangle[2].x = unsplit->Vcam (2).x * iz + shift_x;
    orig_triangle[2].y = unsplit->Vcam (2).y * iz + shift_y;

    if (g3dpolyfx.mat_handle->GetTexture () && fs)
    {
      g3dpolyfx.vertices[0].u = fs->GetUVCoords () [0].x;
      g3dpolyfx.vertices[0].v = fs->GetUVCoords () [0].y;
      g3dpolyfx.vertices[1].u = fs->GetUVCoords () [1].x;
      g3dpolyfx.vertices[1].v = fs->GetUVCoords () [1].y;
      g3dpolyfx.vertices[2].u = fs->GetUVCoords () [2].x;
      g3dpolyfx.vertices[2].v = fs->GetUVCoords () [2].y;
    }

    if (po_colors)
    {
      g3dpolyfx.vertices[0].r = po_colors [0].red;
      g3dpolyfx.vertices[0].g = po_colors [0].green;
      g3dpolyfx.vertices[0].b = po_colors [0].blue;
      g3dpolyfx.vertices[1].r = po_colors [1].red;
      g3dpolyfx.vertices[1].g = po_colors [1].green;
      g3dpolyfx.vertices[1].b = po_colors [1].blue;
      g3dpolyfx.vertices[2].r = po_colors [2].red;
      g3dpolyfx.vertices[2].g = po_colors [2].green;
      g3dpolyfx.vertices[2].b = po_colors [2].blue;
    }
    PreparePolygonFX (&g3dpolyfx, vertices, num_vertices,
      orig_triangle, po_colors != NULL);
    rview->GetGraphics3D ()->StartPolygonFX (g3dpolyfx.mat_handle,
      ns->GetMixMode () | (po_colors ? CS_FX_GOURAUD : 0));
    rview->CalculateFogPolygon (g3dpolyfx);
    rview->GetGraphics3D ()->DrawPolygonFX (g3dpolyfx);
    rview->GetGraphics3D ()->FinishPolygonFX ();
  }
  else
  {
    CS_LOCAL_STATIC(G3DPolygonDP,g3dpoly);

    g3dpoly.num = num_vertices;
    if (poly->GetMaterialWrapper ())
      poly->GetMaterialWrapper ()->Visit ();
    g3dpoly.mat_handle = poly->GetMaterialHandle ();
    g3dpoly.inv_aspect = icam->GetInvFOV ();
    g3dpoly.mixmode = poly->GetTextureTypeInfo ()->GetMixMode ();

    // We are going to use DrawPolygon.
    if (mirror)
      for (i = 0 ; i < num_vertices ; i++)
      {
        g3dpoly.vertices[num_vertices-i-1].sx = vertices[i].x;
        g3dpoly.vertices[num_vertices-i-1].sy = vertices[i].y;
      }
    else
      for (i = 0 ; i < num_vertices ; i++)
      {
        g3dpoly.vertices[i].sx = vertices[i].x;
        g3dpoly.vertices[i].sy = vertices[i].y;
      }

    g3dpoly.alpha           = poly->GetAlpha();
    g3dpoly.z_value         = poly->Vcam(0).z;
#ifdef DO_HW_UVZ
#if 0
    g3dpoly.mirror          = mirror;
    if (poly->isClipped || rview->GetView ()->LastClipResult () == CS_CLIP_INSIDE)
       g3dpoly.uvz = NULL;
    else
    {
      g3dpoly.uvz = poly->uvz;
      for (i = 0 ; i < num_vertices ; i++)
      {
        g3dpoly.uvz[i].z = poly->Vcam(i).z;
      }
    }
#else
       g3dpoly.uvz = NULL;
#endif
#endif

    g3dpoly.poly_texture = poly->GetLightMapInfo ()->GetPolyTex ();

    csPolyTxtPlane* txt_plane = poly->GetLightMapInfo ()->GetTxtPlane ();
    csMatrix3 m_cam2tex;
    csVector3 v_cam2tex;
    txt_plane->WorldToCamera (icam->GetTransform (), m_cam2tex, v_cam2tex);
    g3dpoly.plane.m_cam2tex = &m_cam2tex;
    g3dpoly.plane.v_cam2tex = &v_cam2tex;

    float Ac, Bc, Cc, Dc;
    plane->GetCameraNormal (&Ac, &Bc, &Cc, &Dc);
    g3dpoly.normal.A () = Ac;
    g3dpoly.normal.B () = Bc;
    g3dpoly.normal.C () = Cc;
    g3dpoly.normal.D () = Dc;

    if (debug)
      rview->GetGraphics3D ()->DrawPolygonDebug (g3dpoly);
    else
    {
      rview->CalculateFogPolygon (g3dpoly);
      rview->GetGraphics3D ()->DrawPolygon (g3dpoly);
    }
    
#ifdef DO_HW_UVZ
    poly->isClipped = false;
    g3dpoly.uvz = NULL;
#endif
  }
}

void csPolygon2D::FillZBuf (iRenderView* rview, csPolygon3D* poly,
  csPolyPlane* plane)
{
  rview->GetGraphics3D ()->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_FILLONLY);
  iCamera* icam = rview->GetCamera ();

  CS_LOCAL_STATIC(G3DPolygonDP,g3dpoly);
  g3dpoly.mixmode = CS_FX_COPY;
  g3dpoly.num = num_vertices;
  g3dpoly.inv_aspect = icam->GetInvFOV ();

  // We are going to use DrawPolygon.
  int i;
  if (icam->IsMirrored ())
    for (i = 0 ; i < num_vertices ; i++)
    {
      g3dpoly.vertices[num_vertices-i-1].sx = vertices[i].x;
      g3dpoly.vertices[num_vertices-i-1].sy = vertices[i].y;
    }
  else
    for (i = 0 ; i < num_vertices ; i++)
    {
      g3dpoly.vertices[i].sx = vertices[i].x;
      g3dpoly.vertices[i].sy = vertices[i].y;
    }

  g3dpoly.z_value = poly->Vcam(0).z;

  float Ac, Bc, Cc, Dc;
  plane->GetCameraNormal (&Ac, &Bc, &Cc, &Dc);
  g3dpoly.normal.A () = Ac;
  g3dpoly.normal.B () = Bc;
  g3dpoly.normal.C () = Cc;
  g3dpoly.normal.D () = Dc;

  rview->GetGraphics3D ()->DrawPolygon (g3dpoly);
}

void csPolygon2D::AddFogPolygon (iGraphics3D* g3d, csPolygon3D* /*poly*/,
  csPolyPlane* plane, bool mirror, CS_ID id, int fogtype)
{
  int i;

  CS_LOCAL_STATIC(G3DPolygonDFP,g3dpoly);
  memset(&g3dpoly, 0, sizeof(g3dpoly));
  g3dpoly.num = num_vertices;
  g3dpoly.inv_aspect = csEngine::current_engine->current_camera->GetInvFOV ();
#if 0
  memcpy (g3dpoly.vertices, vertices, num_vertices * sizeof (csVector2));
#else
  if (mirror)
    for (i = 0 ; i < num_vertices ; i++)
    {
      g3dpoly.vertices[num_vertices-i-1].sx = vertices[i].x;
      g3dpoly.vertices[num_vertices-i-1].sy = vertices[i].y;
    }
  else
    memcpy (g3dpoly.vertices, vertices, num_vertices * sizeof (csVector2));
#endif
  //g3dpoly.polygon = GetIPolygon3DFromcsPolygon3D(poly); //DPQFIX

  float Ac, Bc, Cc, Dc;
  plane->GetCameraNormal (&Ac, &Bc, &Cc, &Dc);
  g3dpoly.normal.A () = Ac;
  g3dpoly.normal.B () = Bc;
  g3dpoly.normal.C () = Cc;
  g3dpoly.normal.D () = Dc;

  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_NONE);
  g3d->DrawFogPolygon (id, g3dpoly, fogtype);
}

//---------------------------------------------------------------------------

csPolygon2DQueue::csPolygon2DQueue (int max_size)
{
  queue = new csQueueElement [max_size];
  max_queue = max_size;
  num_queue = 0;
}

csPolygon2DQueue::~csPolygon2DQueue ()
{
  delete [] queue;
}

void csPolygon2DQueue::Push (csPolygon3D* poly3d, csPolygon2D* poly2d)
{
  queue[num_queue].poly3d = poly3d;
  queue[num_queue].poly2d = poly2d;
  num_queue++;
}

bool csPolygon2DQueue::Pop (csPolygon3D** poly3d, csPolygon2D** poly2d)
{
  if (num_queue <= 0) return false;
  num_queue--;
  *poly3d = queue[num_queue].poly3d;
  *poly2d = queue[num_queue].poly2d;
  return true;
}
