/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "walktest.h"
#include "qint.h"
#include "csgeom/frustum.h"
#include "ivaria/view.h"
#include "csgeom/csrect.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "iengine/light.h"
#include "iengine/camera.h"

#define Gfx3D Sys->myG3D
#define Gfx2D Sys->myG2D

extern WalkTest* Sys;

//-----------------------------------------------------------------------------

void DrawZbuffer ()
{
#ifndef CS_USE_NEW_RENDERER
  int y, x;
  for (y = 0; y < FRAME_HEIGHT; y++)
  {
    int gi_pixelbytes = Gfx2D->GetPixelBytes ();

    uint32 *zbuf = Gfx3D->GetZBuffAt (0, y);

    if (zbuf)
      if (gi_pixelbytes == 4)
      {
        uint32 *dest = (uint32 *)Gfx2D->GetPixelAt (0, y);
        for (x = 0; x < FRAME_WIDTH; x++)
          *dest++ = *zbuf++ >> 10;
      }
      else if (gi_pixelbytes == 2)
      {
        uint16 *dest = (uint16 *)Gfx2D->GetPixelAt(0, y);
        for (x = 0; x < FRAME_WIDTH; x++)
          *dest++ = (unsigned short)(*zbuf++ >> 13);
      }
      else
      {
        unsigned char *dest = Gfx2D->GetPixelAt (0, y);
        for (x = 0; x < FRAME_WIDTH; x++)
          *dest++ = (unsigned char)(*zbuf++ >> 16);
      }
  }
#endif
}

void DrawPalette ()
{
  if (Gfx2D->GetPixelBytes () != 1)
    return;
  int pw = Gfx2D->GetWidth () / 16;
  int ph = Gfx2D->GetHeight () / 16;
  int i, j;
  for (i = 0; i < 16; i++)
    for (j = 0; j < 16; j++)
      Gfx2D->DrawBox (i * pw, j * ph, pw, ph, j * 16 + i);
}

int collcount = 0;

//------------------------------------------------------------------------
// The following series of functions are all special callback functions
// which are called by csEngine::DrawFunc() or csLight::LightingFunc().
//------------------------------------------------------------------------

// Callback for DrawFunc() to select an object with the mouse. The coordinate
// to check for is in 'coord_check_vector'.
bool check_poly;
bool check_light;
csVector2 coord_check_vector;

void select_object (iRenderView* rview, int type, void* entity)
{
  (void)rview; (void)type; (void)entity;
#if 0
//@@@@@@@@@@@@@
  if (type == CS_CALLBACK_POLYGON2D)
  {
    int i;
    csPolygon2D* polygon = (csPolygon2D*)entity;
    int num = polygon->GetVertexCount ();
    csPolygon2D* pp = new csPolygon2D ();
    if (rview->IsMirrored ())
      for (i = 0 ; i < num ; i++)
        pp->AddVertex  (polygon->GetVertices ()[num-i-1]);
    else
      for (i = 0 ; i < num ; i++)
        pp->AddVertex  (polygon->GetVertices ()[i]);
    if (csMath2::InPoly2D (coord_check_vector, pp->GetVertices (),
        pp->GetVertexCount (), &pp->GetBoundingBox ()) != CS_POLY_OUT)
      //Dumper::dump (polygon, "csPolygon2D");

    delete pp;
  }
  else if (type == CS_CALLBACK_SECTOR)
  {
    csSector* sector = (csSector*)entity;
    int i;
    csVector3 v;
    float iz;
    int px, py, r;
    for (i = 0 ; i < sector->GetLightCount () ; i++)
    {
      v = rview->Other2This (sector->GetLight (i)->GetCenter ());
      if (v.z > SMALL_Z)
      {
        iz = rview->GetFOV ()/v.z;
        px = QInt (v.x * iz + rview->GetShiftX ());
        py = csEngine::frame_height - 1 - QInt (v.y * iz + rview->GetShiftY ());
        r = QInt (.3 * iz);
        if (ABS (coord_check_vector.x - px) < 5 &&
		ABS (coord_check_vector.y - (csEngine::frame_height-1-py)) < 5)
        {
	  csLight* light = sector->GetLight (i);
	  if (check_light)
	  {
            if (Sys->selected_light == light) Sys->selected_light = 0;
	    else Sys->selected_light = light;
	    //check_light = false;
	  }
          Sys->Printf (CS_MSG_CONSOLE, "Selected light %s/(%f,%f,%f).\n",
                    sector->GetName (), light->GetCenter ().x,
                    light->GetCenter ().y, light->GetCenter ().z);
          Sys->Printf (CS_MSG_DEBUG_0, "Selected light %s/(%f,%f,%f).\n",
                    sector->GetName (), light->GetCenter ().x,
                    light->GetCenter ().y, light->GetCenter ().z);
        }
      }
    }
  }
#endif
}

//------------------------------------------------------------------------
// Draw debug boxes
//------------------------------------------------------------------------

void DrawDebugBoxSide (iCamera* cam, bool do3d,
    	const csVector3& v1, const csColor& c1,
    	const csVector3& v2, const csColor& c2,
    	const csVector3& v3, const csColor& c3,
    	const csVector3& v4, const csColor& c4)
{
#ifndef CS_USE_NEW_RENDERER
  G3DPolygonDPFX poly;
  csVector3 v;
  csVector2 persp;
  poly.num = 4;
  poly.use_fog = false;
  poly.mat_handle = 0;
  poly.flat_color_r = 255;
  poly.flat_color_g = 255;
  poly.flat_color_b = 255;
  v = cam->GetTransform ().Other2This (v1);
  if (v.z < .01) return;
  cam->Perspective (v, persp);
  if (do3d)
  {
    if (persp.x <= 0) persp.x = 1;
    if (persp.x > FRAME_WIDTH) persp.x = FRAME_WIDTH-1;
    if (persp.y <= 0) persp.y = 1;
    if (persp.y > FRAME_HEIGHT) persp.y = FRAME_HEIGHT-1;
  }
  poly.vertices[0].x = persp.x;
  poly.vertices[0].y = persp.y;
  poly.z[0] = 1./v.z;
  poly.texels[0].x = 0;
  poly.texels[0].y = 0;
  poly.colors[0].red = c1.red;
  poly.colors[0].green = c1.green;
  poly.colors[0].blue = c1.blue;
  v = cam->GetTransform ().Other2This (v2);
  if (v.z < .01) return;
  cam->Perspective (v, persp);
  if (do3d)
  {
    if (persp.x <= 0) persp.x = 1;
    if (persp.x > FRAME_WIDTH) persp.x = FRAME_WIDTH-1;
    if (persp.y <= 0) persp.y = 1;
    if (persp.y > FRAME_HEIGHT) persp.y = FRAME_HEIGHT-1;
  }
  poly.vertices[1].x = persp.x;
  poly.vertices[1].y = persp.y;
  poly.z[1] = 1./v.z;
  poly.texels[1].x = 1;
  poly.texels[1].y = 0;
  poly.colors[1].red = c2.red;
  poly.colors[1].green = c2.green;
  poly.colors[1].blue = c2.blue;
  v = cam->GetTransform ().Other2This (v3);
  if (v.z < .01) return;
  cam->Perspective (v, persp);
  if (do3d)
  {
    if (persp.x <= 0) persp.x = 1;
    if (persp.x > FRAME_WIDTH) persp.x = FRAME_WIDTH-1;
    if (persp.y <= 0) persp.y = 1;
    if (persp.y > FRAME_HEIGHT) persp.y = FRAME_HEIGHT-1;
  }
  poly.vertices[2].x = persp.x;
  poly.vertices[2].y = persp.y;
  poly.z[2] = 1./v.z;
  poly.texels[2].x = 1;
  poly.texels[2].y = 1;
  poly.colors[2].red = c3.red;
  poly.colors[2].green = c3.green;
  poly.colors[2].blue = c3.blue;
  v = cam->GetTransform ().Other2This (v4);
  if (v.z < .01) return;
  cam->Perspective (v, persp);
  if (do3d)
  {
    if (persp.x <= 0) persp.x = 1;
    if (persp.x > FRAME_WIDTH) persp.x = FRAME_WIDTH-1;
    if (persp.y <= 0) persp.y = 1;
    if (persp.y > FRAME_HEIGHT) persp.y = FRAME_HEIGHT-1;
  }
  poly.vertices[3].x = persp.x;
  poly.vertices[3].y = persp.y;
  poly.z[3] = 1./v.z;
  poly.texels[3].x = 0;
  poly.texels[3].y = 1;
  poly.colors[3].red = c4.red;
  poly.colors[3].green = c4.green;
  poly.colors[3].blue = c4.blue;
  poly.mat_handle = 0;
  poly.mixmode = CS_FX_ADD;
  if (do3d)
  {
    Gfx3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
    Gfx3D->DrawPolygonFX (poly);
  }
  else
  {
    int color = Gfx2D->FindRGB (QInt (255.*c1.red*2), 0, QInt (255.*c1.blue*2));
    Gfx2D->DrawLine (poly.vertices[0].x, FRAME_HEIGHT-poly.vertices[0].y, poly.vertices[1].x, FRAME_HEIGHT-poly.vertices[1].y, color);
    Gfx2D->DrawLine (poly.vertices[1].x, FRAME_HEIGHT-poly.vertices[1].y, poly.vertices[2].x, FRAME_HEIGHT-poly.vertices[2].y, color);
    Gfx2D->DrawLine (poly.vertices[2].x, FRAME_HEIGHT-poly.vertices[2].y, poly.vertices[3].x, FRAME_HEIGHT-poly.vertices[3].y, color);
    Gfx2D->DrawLine (poly.vertices[3].x, FRAME_HEIGHT-poly.vertices[3].y, poly.vertices[0].x, FRAME_HEIGHT-poly.vertices[0].y, color);
  }
#endif
}

void DrawDebugBox (iCamera* cam, bool do3d, const csBox3& box, float r, float b)
{
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (CS_BOX_CORNER_xYz), csColor (r, .5, b),
      	box.GetCorner (CS_BOX_CORNER_XYz), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_Xyz), csColor (r, .5, b),
      	box.GetCorner (CS_BOX_CORNER_xyz), csColor (r, 0, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (CS_BOX_CORNER_XYz), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_XYZ), csColor (r, .5, b),
      	box.GetCorner (CS_BOX_CORNER_XyZ), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_Xyz), csColor (r, .5, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (CS_BOX_CORNER_XYZ), csColor (r, .5, b),
      	box.GetCorner (CS_BOX_CORNER_xYZ), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_xyZ), csColor (r, .5, b),
      	box.GetCorner (CS_BOX_CORNER_XyZ), csColor (r, 0, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (CS_BOX_CORNER_xYZ), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_xYz), csColor (r, .5, b),
      	box.GetCorner (CS_BOX_CORNER_xyz), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_xyZ), csColor (r, .5, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (CS_BOX_CORNER_xYZ), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_XYZ), csColor (r, .5, b),
      	box.GetCorner (CS_BOX_CORNER_XYz), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_xYz), csColor (r, .5, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (CS_BOX_CORNER_xyz), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_Xyz), csColor (r, .5, b),
      	box.GetCorner (CS_BOX_CORNER_XyZ), csColor (r, 0, b),
      	box.GetCorner (CS_BOX_CORNER_xyZ), csColor (r, .5, b));
}

void DrawDebugBoxes (iCamera* cam, bool do3d)
{
  DrawDebugBox (cam, do3d, Sys->debug_box1, .5, 0);
  DrawDebugBox (cam, do3d, Sys->debug_box2, 0, .5);
}

//------------------------------------------------------------------------
// Draw cross signs where the octree centers are.
//------------------------------------------------------------------------

void DrawLineDepth (const csVector3& v1, const csVector3& v2,
        float fov)
{
  if (v1.z < SMALL_Z && v2.z < SMALL_Z)
    return;

  float x1 = v1.x, y1 = v1.y, z1 = v1.z;
  float x2 = v2.x, y2 = v2.y, z2 = v2.z;

  if (z1 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z-z1) / (z2-z1);
    x1 = t*(x2-x1)+x1;
    y1 = t*(y2-y1)+y1;
    z1 = SMALL_Z;
  }
  else if (z2 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z-z1) / (z2-z1);
    x2 = t*(x2-x1)+x1;
    y2 = t*(y2-y1)+y1;
    z2 = SMALL_Z;
  }

  float iz1 = fov/z1;
  int px1 = QInt (x1 * iz1 + (FRAME_WIDTH/2));
  int py1 = FRAME_HEIGHT - 1 - QInt (y1 * iz1 + (FRAME_HEIGHT/2));
  float iz2 = fov/z2;
  int px2 = QInt (x2 * iz2 + (FRAME_WIDTH/2));
  int py2 = FRAME_HEIGHT - 1 - QInt (y2 * iz2 + (FRAME_HEIGHT/2));

  int col = (int)((40.0-z1)*(255./40.));
  if (col < 0) col = 0;
  else if (col > 255) col = 255;
  int color = Gfx2D->FindRGB (col, col, col);

  Gfx2D->DrawLine (px1, py1, px2, py2, color);
}

