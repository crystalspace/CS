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
#include "qint.h"
#include "csengine/sysitf.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/polytext.h"
#include "csengine/texture.h"
#include "csengine/polyplan.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/light.h"
#include "csengine/dynlight.h"
#include "csengine/lghtmap.h"
#include "csengine/camera.h"
#include "csengine/portal.h"
#include "csengine/dumper.h"
#include "igraph3d.h"
#include "itexture.h"
#include "itxtmgr.h"

csPolygon2DFactory* csPolygon2DFactory::SharedFactory()
{
  static csPolygon2DFactory p;
  return &p;
}

void csPolygon2D::AddPerspective (float x, float y, float z)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices+5);

  float iz = csWorld::current_world->current_camera->aspect/z;
  float px, py;

  px = x * iz + csWorld::current_world->current_camera->shift_x;
  py = y * iz + csWorld::current_world->current_camera->shift_y;
  vertices[num_vertices].x = px;
  vertices[num_vertices].y = py;

  num_vertices++;
  bbox.AddBoundingVertex (px, py);
}

void csPolygon2D::AddPerspectiveUnit (float x, float y, float z)
{
  if (num_vertices >= max_vertices)
    MakeRoom (max_vertices+5);

  float iz = 1./z;
  float px, py;

  px = x * iz;
  py = y * iz;

  vertices[num_vertices].x = px;
  vertices[num_vertices].y = py;

  num_vertices++;
  bbox.AddBoundingVertex (px, py);
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
    g2d->DrawLine (x1, csWorld::frame_height - 1 - y1, x2, csWorld::frame_height - 1 - y2, col);

    x1 = x2;
    y1 = y2;
  }
}

//---------------------------------------------------------------------------

void PreparePolygonFX (G3DPolygonDPFX* g3dpoly, csVector2* clipped_verts,
	int num_vertices, csVector2* orig_triangle, bool gouraud)
{
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
  G3DTexturedVertex tritexcoords[3];
  for (int i = 0; i < 3; i++)
    tritexcoords [i] = g3dpoly->vertices [i];

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
    //  |   *B   |   *B  this switch bytaking different start/final values
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
                tritexcoords [vtl].z, tritexcoords [vbl].z,
                tritexcoords [vtr].z, tritexcoords [vbr].z);
    if (g3dpoly->txt_handle)
    {
      // Calculate U
      INTERPOLATE(g3dpoly->vertices [j].u,
                  tritexcoords [vtl].u, tritexcoords [vbl].u,
                  tritexcoords [vtr].u, tritexcoords [vbr].u);
      // Calculate V
      INTERPOLATE(g3dpoly->vertices [j].v,
                  tritexcoords [vtl].v, tritexcoords [vbl].v,
                  tritexcoords [vtr].v, tritexcoords [vbr].v);
    }
    if (gouraud)
    {
      // Calculate R
      INTERPOLATE(g3dpoly->vertices [j].r,
                  tritexcoords [vtl].r, tritexcoords [vbl].r,
                  tritexcoords [vtr].r, tritexcoords [vbr].r);
      // Calculate G
      INTERPOLATE (g3dpoly->vertices [j].g,
                  tritexcoords [vtl].g, tritexcoords [vbl].g,
                  tritexcoords [vtr].g, tritexcoords [vbr].g);
      // Calculate B
      INTERPOLATE (g3dpoly->vertices [j].b,
                  tritexcoords [vtl].b, tritexcoords [vbl].b,
                  tritexcoords [vtr].b, tritexcoords [vbr].b);
    }
    else
    {
      g3dpoly->vertices[j].r = 0;
      g3dpoly->vertices[j].g = 0;
      g3dpoly->vertices[j].b = 0;
    }
  }
}

// Remove this for old fog
//#define USE_EXP_FOG

// After such number of values fog density coefficient can be considered 0.
#ifdef USE_EXP_FOG

#define FOG_EXP_TABLE_SIZE 1600
static float *fog_exp_table = NULL;

static void InitializeFogTable ()
{
  CHK (fog_exp_table = new float [FOG_EXP_TABLE_SIZE]);
  for (int i = 0; i < FOG_EXP_TABLE_SIZE; i++)
    fog_exp_table [i] = 1 - exp (-float (i) / 256.);
}
#endif

#define SMALL_D 0.01
void CalculateFogPolygon (csRenderView* rview, G3DPolygonDP& poly)
{
  if (!rview->fog_info || poly.num < 3) { poly.use_fog = false; return; }
  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
#endif

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float inv_aspect = poly.inv_aspect;
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A;
  Bc = poly.normal.B;
  Cc = poly.normal.C;
  Dc = poly.normal.D;

  float M, N, O;
  if (ABS (Dc) < SMALL_D) Dc = -SMALL_D;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means that
    // the plane of the polygon is almost perpendicular to the eye of the
    // viewer. In this case, nothing much can be seen of the plane anyway
    // so we just take one value for the entire polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1/poly.z_value;
  }
  else
  {
    inv_Dc = 1/Dc;
    M = -Ac*inv_Dc*inv_aspect;
    N = -Bc*inv_Dc*inv_aspect;
    O = -Cc*inv_Dc;
  }

  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1. / (M * (poly.vertices[i].sx - rview->shift_x) + N * (poly.vertices[i].sy - rview->shift_y) + O);
    v.x = (poly.vertices[i].sx - rview->shift_x) * v.z * inv_aspect;
    v.y = (poly.vertices[i].sy - rview->shift_y) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fog_info = rview->fog_info;
    while (fog_info)
    {
      float dist1, dist2;
      if (fog_info->has_incoming_plane)
      {
	const csPlane& pl = fog_info->incoming_plane;
	float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
	//dist1 = v.Norm () * (-pl.DD / denom);
	dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;
      const csPlane& pl = fog_info->outgoing_plane;
      float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
      //dist2 = v.Norm () * (-pl.DD / denom);
      dist2 = v.z * (-pl.DD / denom);

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      UInt table_index = QRound ((100 * ABS (dist2 - dist1)) * fog_info->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table [table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE-1];
#else
      float I2 = ABS (dist2 - dist1) * fog_info->fog->density;
#endif

      if (poly.fog_info[i].intensity)
      {
        // We already have a previous fog level. In this case we do some
	// mathematical tricks to combine both fog levels. Substitute one
	// fog expresion in the other. The basic fog expression is:
	//	C = I*F + (1-I)*P
	//	with I = intensity
	//	     F = fog color
	//	     P = polygon color
	//	     C = result
	float I1 = poly.fog_info[i].intensity;
	poly.fog_info[i].intensity = I1 + I2 - I1 * I2;
	float fact = 1. / (I1 + I2 - I1*I2);
	poly.fog_info[i].r = (I2*fog_info->fog->red + I1*poly.fog_info[i].r + I1*I2*poly.fog_info[i].r) * fact;
	poly.fog_info[i].g = (I2*fog_info->fog->green + I1*poly.fog_info[i].g + I1*I2*poly.fog_info[i].g) * fact;
	poly.fog_info[i].b = (I2*fog_info->fog->blue + I1*poly.fog_info[i].b + I1*I2*poly.fog_info[i].b) * fact;
      }
      else
      {
        // The first fog level.
        poly.fog_info[i].intensity = I2;
        poly.fog_info[i].r = fog_info->fog->red;
        poly.fog_info[i].g = fog_info->fog->green;
        poly.fog_info[i].b = fog_info->fog->blue;
      }
      fog_info = fog_info->next;
    }
  }
}

// @@@ We should be able to avoid having the need for two functions
// which are almost exactly the same.
void CalculateFogPolygon (csRenderView* rview, G3DPolygonDPFX& poly)
{
  if (!rview->fog_info || poly.num < 3) { poly.use_fog = false; return; }
  poly.use_fog = true;

#ifdef USE_EXP_FOG
  if (!fog_exp_table)
    InitializeFogTable ();
#endif

  float inv_aspect = poly.inv_aspect;

  int i;
  for (i = 0 ; i < poly.num ; i++)
  {
    // Calculate the original 3D coordinate again (camera space).
    csVector3 v;
    v.z = 1. / poly.vertices[i].z;
    v.x = (poly.vertices[i].sx - rview->shift_x) * v.z * inv_aspect;
    v.y = (poly.vertices[i].sy - rview->shift_y) * v.z * inv_aspect;

    // Initialize fog vertex.
    poly.fog_info[i].r = 0;
    poly.fog_info[i].g = 0;
    poly.fog_info[i].b = 0;
    poly.fog_info[i].intensity = 0;

    // Consider a ray between (0,0,0) and v and calculate the thickness of every
    // fogged sector in between.
    csFogInfo* fog_info = rview->fog_info;
    while (fog_info)
    {
      float dist1, dist2;
      if (fog_info->has_incoming_plane)
      {
	const csPlane& pl = fog_info->incoming_plane;
	float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
	//dist1 = v.Norm () * (-pl.DD / denom);
        dist1 = v.z * (-pl.DD / denom);
      }
      else
        dist1 = 0;
      //@@@ assume all FX polygons have no outgoing plane
      if (!rview->added_fog_info)
      {
        const csPlane& pl = fog_info->outgoing_plane;
        float denom = pl.norm.x*v.x + pl.norm.y*v.y + pl.norm.z*v.z;
        //dist2 = v.Norm () * (-pl.DD / denom);
        dist2 = v.z * (-pl.DD / denom);
      }
      else
        dist2 = v.Norm();

#ifdef USE_EXP_FOG
      // Implement semi-exponential fog (linearily approximated)
      UInt table_index = QRound ((100 * ABS (dist2 - dist1)) * fog_info->fog->density);
      float I2;
      if (table_index < FOG_EXP_TABLE_SIZE)
        I2 = fog_exp_table [table_index];
      else
        I2 = fog_exp_table[FOG_EXP_TABLE_SIZE-1];
#else
      float I2 = ABS (dist2 - dist1) * fog_info->fog->density;
#endif

      if (poly.fog_info[i].intensity)
      {
        // We already have a previous fog level. In this case we do some
	// mathematical tricks to combine both fog levels. Substitute one
	// fog expresion in the other. The basic fog expression is:
	//	C = I*F + (1-I)*P
	//	with I = intensity
	//	     F = fog color
	//	     P = polygon color
	//	     C = result
	float I1 = poly.fog_info[i].intensity;
	poly.fog_info[i].intensity = I1 + I2 - I1*I2;
	float fact = 1. / (I1 + I2 - I1*I2);
	poly.fog_info[i].r = (I2*fog_info->fog->red + I1*poly.fog_info[i].r + I1*I2*poly.fog_info[i].r) * fact;
	poly.fog_info[i].g = (I2*fog_info->fog->green + I1*poly.fog_info[i].g + I1*I2*poly.fog_info[i].g) * fact;
	poly.fog_info[i].b = (I2*fog_info->fog->blue + I1*poly.fog_info[i].b + I1*I2*poly.fog_info[i].b) * fact;
      }
      else
      {
        // The first fog level.
        poly.fog_info[i].intensity = I2;
        poly.fog_info[i].r = fog_info->fog->red;
        poly.fog_info[i].g = fog_info->fog->green;
        poly.fog_info[i].b = fog_info->fog->blue;
      }
      fog_info = fog_info->next;
    }
  }
}


//---------------------------------------------------------------------------

void csPolygon2D::DrawFilled (csRenderView* rview, csPolygon3D* poly, csPolyPlane* plane,
	bool use_z_buf)
{
  //@@@
  static G3DPolygonDPFX g3dpolyfx;
  csVector2 orig_triangle[3];
  csPolygon3D* unsplit;
  //@@@

  int i;
  bool debug = false;
  bool mirror = rview->IsMirrored ();

  if (use_z_buf)
  {
    rview->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, true);
    rview->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  }
  else
  {
    rview->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, false);
    rview->g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  }

  if (poly->GetTextureType () == POLYTXT_GOURAUD || poly->CheckFlags (CS_POLY_FLATSHADING))
  {
    // We have a gouraud shaded polygon.
    // Add all dynamic lights if polygon is dirty.
    //@@@csPolygon3D* unsplit;
    unsplit = poly->GetBasePolygon ();
    const bool do_light = poly->CheckFlags (CS_POLY_LIGHTING);
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
            unsplit->UpdateVertexLighting (lp->GetLight (), lp->GetLight ()->GetColor (),
      	      true, reset);
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

    //@@@static G3DPolygonDPFX g3dpolyfx;
    //@@@csVector2 orig_triangle[3];
    csGouraudShaded* gs = poly->GetGouraudInfo ();

    g3dpolyfx.num = num_vertices;
    g3dpolyfx.txt_handle = poly->GetTextureHandle ();
    g3dpolyfx.inv_aspect = rview->inv_aspect;

    csColor* po_colors = do_light ? gs->GetColors () : NULL;
    if (poly->CheckFlags (CS_POLY_FLATSHADING)) g3dpolyfx.txt_handle = NULL;

    // We are going to use DrawPolygonFX.
    // Here we have to do a little messy thing because PreparePolygonFX()
    // still requires the original triangle that was valid before clipping.
    float iz;
    iz = 1. / unsplit->Vcam (0).z;
    g3dpolyfx.vertices[0].z = iz;
    iz *= rview->aspect;
    orig_triangle[0].x = unsplit->Vcam (0).x * iz + rview->shift_x;
    orig_triangle[0].y = unsplit->Vcam (0).y * iz + rview->shift_y;

    iz = 1. / unsplit->Vcam (1).z;
    g3dpolyfx.vertices[1].z = iz;
    iz *= rview->aspect;
    orig_triangle[1].x = unsplit->Vcam (1).x * iz + rview->shift_x;
    orig_triangle[1].y = unsplit->Vcam (1).y * iz + rview->shift_y;

    iz = 1. / unsplit->Vcam (2).z;
    g3dpolyfx.vertices[2].z = iz;
    iz *= rview->aspect;
    orig_triangle[2].x = unsplit->Vcam (2).x * iz + rview->shift_x;
    orig_triangle[2].y = unsplit->Vcam (2).y * iz + rview->shift_y;

    if (g3dpolyfx.txt_handle)
    {
      g3dpolyfx.vertices[0].u = gs->GetUVCoords ()[0].x;
      g3dpolyfx.vertices[0].v = gs->GetUVCoords ()[0].y;
      g3dpolyfx.vertices[1].u = gs->GetUVCoords ()[1].x;
      g3dpolyfx.vertices[1].v = gs->GetUVCoords ()[1].y;
      g3dpolyfx.vertices[2].u = gs->GetUVCoords ()[2].x;
      g3dpolyfx.vertices[2].v = gs->GetUVCoords ()[2].y;
      float r, g, b;
      g3dpolyfx.txt_handle->GetMeanColor (r, g, b);
      g3dpolyfx.flat_color_r = r;
      g3dpolyfx.flat_color_g = g;
      g3dpolyfx.flat_color_b = b;
    }
    else
    {
      g3dpolyfx.flat_color_r = poly->GetFlatColor ().red;
      g3dpolyfx.flat_color_g = poly->GetFlatColor ().green;
      g3dpolyfx.flat_color_b = poly->GetFlatColor ().blue;
    }
    if (po_colors)
    {
      g3dpolyfx.vertices[0].r = po_colors[0].red;
      g3dpolyfx.vertices[0].g = po_colors[0].green;
      g3dpolyfx.vertices[0].b = po_colors[0].blue;
      g3dpolyfx.vertices[1].r = po_colors[1].red;
      g3dpolyfx.vertices[1].g = po_colors[1].green;
      g3dpolyfx.vertices[1].b = po_colors[1].blue;
      g3dpolyfx.vertices[2].r = po_colors[2].red;
      g3dpolyfx.vertices[2].g = po_colors[2].green;
      g3dpolyfx.vertices[2].b = po_colors[2].blue;
    }
    PreparePolygonFX (&g3dpolyfx, vertices, num_vertices, orig_triangle,
    	po_colors != NULL);
    rview->g3d->StartPolygonFX (g3dpolyfx.txt_handle,
    	CS_FX_COPY | (po_colors ? CS_FX_GOURAUD : 0));
    CalculateFogPolygon (rview, g3dpolyfx);
    rview->g3d->DrawPolygonFX (g3dpolyfx);
    rview->g3d->FinishPolygonFX ();
  }
  else
  {
    static G3DPolygonDP g3dpoly;

    g3dpoly.num = num_vertices;
    g3dpoly.txt_handle = poly->GetTextureHandle ();
    g3dpoly.inv_aspect = rview->inv_aspect;

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
    g3dpoly.uses_mipmaps    = poly->CheckFlags (CS_POLY_MIPMAP);
    g3dpoly.z_value         = poly->Vcam(0).z;

    for (int mipmaplevel = 0; mipmaplevel<4; mipmaplevel++)
      g3dpoly.poly_texture[mipmaplevel] = poly->GetLightMapInfo ()->GetPolyTex (mipmaplevel);

    g3dpoly.plane.m_cam2tex = &plane->m_cam2tex;
    g3dpoly.plane.v_cam2tex = &plane->v_cam2tex;

    float Ac, Bc, Cc, Dc;
    plane->GetCameraNormal (&Ac, &Bc, &Cc, &Dc);
    g3dpoly.normal.A = Ac;
    g3dpoly.normal.B = Bc;
    g3dpoly.normal.C = Cc;
    g3dpoly.normal.D = Dc;

    if (debug)
      rview->g3d->DrawPolygonDebug (g3dpoly);
    else
    {
      CalculateFogPolygon (rview, g3dpoly);
      rview->g3d->DrawPolygon (g3dpoly);
    }
  }
}

void csPolygon2D::AddFogPolygon (iGraphics3D* g3d, csPolygon3D* /*poly*/, csPolyPlane* plane, bool mirror, CS_ID id, int fogtype)
{
  int i;

  static G3DPolygonAFP g3dpoly;
  memset(&g3dpoly, 0, sizeof(g3dpoly));
  g3dpoly.num = num_vertices;
  g3dpoly.inv_aspect = csWorld::current_world->current_camera->inv_aspect;
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
  //g3dpoly.polygon = GetIPolygon3DFromcsPolygon3D(poly); //DPQFIX

  float Ac, Bc, Cc, Dc;
  plane->GetCameraNormal (&Ac, &Bc, &Cc, &Dc);
  g3dpoly.normal.A = Ac;
  g3dpoly.normal.B = Bc;
  g3dpoly.normal.C = Cc;
  g3dpoly.normal.D = Dc;

  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERTESTENABLE, true);
  g3d->SetRenderState (G3DRENDERSTATE_ZBUFFERFILLENABLE, true);
  g3d->AddFogPolygon (id, g3dpoly, fogtype);
}

//---------------------------------------------------------------------------

csPolygon2DQueue::csPolygon2DQueue (int max_size)
{
  CHK (queue = new csQueueElement [max_size]);
  max_queue = max_size;
  num_queue = 0;
}

csPolygon2DQueue::~csPolygon2DQueue ()
{
  CHK (delete [] queue);
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

//---------------------------------------------------------------------------
