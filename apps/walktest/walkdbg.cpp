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

#include "sysdef.h"
#include "walktest/walktest.h"
#include "version.h"
#include "qint.h"
#include "cssys/system.h"
#include "csgeom/frustrum.h"
#include "csengine/dumper.h"
#include "csengine/stats.h"
#include "csengine/light.h"
#include "csengine/dynlight.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csengine/wirefrm.h"
#include "csengine/polytext.h"
#include "csengine/polyset.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/sector.h"
#include "csengine/world.h"
#include "csengine/covtree.h"
#include "csengine/solidbsp.h"
#include "csengine/cssprite.h"
#include "csengine/csview.h"
#include "csutil/csrect.h"
#include "csobject/dataobj.h"
#include "igraph3d.h"
#include "itxtmgr.h"

#define Gfx3D System->G3D
#define Gfx2D System->G2D

//-----------------------------------------------------------------------------

void DrawZbuffer ()
{
  for (int y = 0; y < FRAME_HEIGHT; y++)
  {
    int gi_pixelbytes = System->G2D->GetPixelBytes ();

    ULong *zbuf = Gfx3D->GetZBuffAt (0, y);

    if (zbuf)
      if (gi_pixelbytes == 4)
      {
        ULong *dest = (ULong *)Gfx2D->GetPixelAt (0, y);
        for (int x = 0; x < FRAME_WIDTH; x++)
          *dest++ = *zbuf++ >> 10;
      }
      else if (gi_pixelbytes == 2)
      {
        UShort *dest = (UShort *)Gfx2D->GetPixelAt(0, y);
        for (int x = 0; x < FRAME_WIDTH; x++)
          *dest++ = (unsigned short)(*zbuf++ >> 13);
      }
      else
      {
        unsigned char *dest = Gfx2D->GetPixelAt (0, y);
        for (int x = 0; x < FRAME_WIDTH; x++)
          *dest++ = (unsigned char)(*zbuf++ >> 16);
      }
  }
}

void DrawPalette ()
{
  if (System->G2D->GetPixelBytes () != 1)
    return;
  int pw = System->G2D->GetWidth () / 16;
  int ph = System->G2D->GetHeight () / 16;
  for (int i = 0; i < 16; i++)
    for (int j = 0; j < 16; j++)
      System->G2D->DrawBox (i * pw, j * ph, pw, ph, j * 16 + i);
}

int collcount = 0;

//------------------------------------------------------------------------
// The following series of functions are all special callback functions
// which are called by csWorld::DrawFunc() or csLight::LightingFunc().
//------------------------------------------------------------------------

// Callback for LightingFunc() to show the lighting frustum for the
// selected light.
void show_frustum (csFrustumView* lview, int type, void* /*entity*/)
{
  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
  int white = txtmgr->FindRGB (255, 255, 255);
  int red = txtmgr->FindRGB (255, 0, 0);

  if (type == CALLBACK_POLYGON)
  {
    csCamera* cam = Sys->view->GetCamera ();
    csFrustum* fr = lview->light_frustum;
    csVector3 v0, v1, v2;
    csVector3 light_cam = cam->Other2This (fr->GetOrigin ());
    int j;

    for (j = 0 ; j < fr->GetNumVertices () ; j++)
    {
      v0 = fr->GetVertices ()[j] + fr->GetOrigin ();
      v1 = cam->Other2This (v0);
      v0 = fr->GetVertices ()[(j+1)%fr->GetNumVertices ()] + fr->GetOrigin ();
      v2 = cam->Other2This (v0);
      Gfx3D->DrawLine (light_cam, v1, cam->aspect, red);
      Gfx3D->DrawLine (light_cam, v2, cam->aspect, red);
      Gfx3D->DrawLine (v1, v2, cam->aspect, white);
    }
  }
}

// Callback for DrawFunc() to select an object with the mouse. The coordinate
// to check for is in 'coord_check_vector'.
bool check_poly;
bool check_light;
csVector2 coord_check_vector;

void select_object (csRenderView* rview, int type, void* entity)
{
  if (type == CALLBACK_POLYGON2D)
  {
    int i;
    csPolygon2D* polygon = (csPolygon2D*)entity;
    int num = polygon->GetNumVertices ();
    csPolygon2D* pp = new csPolygon2D ();
    if (rview->IsMirrored ())
      for (i = 0 ; i < num ; i++)
        pp->AddVertex  (polygon->GetVertices ()[num-i-1]);
    else
      for (i = 0 ; i < num ; i++)
        pp->AddVertex  (polygon->GetVertices ()[i]);
    if (csMath2::InPoly2D (coord_check_vector, pp->GetVertices (),
        pp->GetNumVertices (), &pp->GetBoundingBox ()) != CS_POLY_OUT)
      Dumper::dump (polygon, "csPolygon2D");

    delete pp;
  }
  else if (type == CALLBACK_SECTOR)
  {
    csSector* sector = (csSector*)entity;
    int i;
    csVector3 v;
    float iz;
    int px, py, r;
    for (i = 0 ; i < sector->lights.Length () ; i++)
    {
      v = rview->Other2This (((csStatLight*)sector->lights[i])->GetCenter ());
      if (v.z > SMALL_Z)
      {
        iz = rview->aspect/v.z;
        px = QInt (v.x * iz + rview->shift_x);
        py = csWorld::frame_height - 1 - QInt (v.y * iz + rview->shift_y);
        r = QInt (.3 * iz);
        if (ABS (coord_check_vector.x - px) < 5 && ABS (coord_check_vector.y - (csWorld::frame_height-1-py)) < 5)
        {
	  csLight* light = (csLight*)sector->lights[i];
	  if (check_light)
	  {
            if (Sys->selected_light == light) Sys->selected_light = NULL;
	    else Sys->selected_light = light;
	    //check_light = false;
	  }
          Sys->Printf (MSG_CONSOLE, "Selected light %s/(%f,%f,%f).\n",
                    sector->GetName (), light->GetCenter ().x,
                    light->GetCenter ().y, light->GetCenter ().z);
          Sys->Printf (MSG_DEBUG_0, "Selected light %s/(%f,%f,%f).\n",
                    sector->GetName (), light->GetCenter ().x,
                    light->GetCenter ().y, light->GetCenter ().z);
        }
      }
    }
  }
}

// Callback for DrawFunc() to show an outline for all polygons and lights.
// If callback_data in 'rview' is not NULL then we only show outline for
// selected light and/or polygon.
void draw_edges (csRenderView* rview, int type, void* entity)
{
  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
  int selcol;
  int white = txtmgr->FindRGB (255, 255, 255);
  int red = txtmgr->FindRGB (255, 0, 0);
  int blue = txtmgr->FindRGB (0, 0, 255);
  int yellow = txtmgr->FindRGB (255, 255, 0);

  bool hilighted_only = !!rview->callback_data;
  if (hilighted_only) selcol = yellow;
  else selcol = white;
  static csPolygon3D* last_poly = NULL;

  if (type == CALLBACK_POLYGON)
  {
    // Here we depend on CALLBACK_POLYGON being called right before CALLBACK_POLYGON2D.
    last_poly = (csPolygon3D*)entity;
  }
  else if (type == CALLBACK_POLYGON2D)
  {
    csPolygon2D* polygon = (csPolygon2D*)entity;
    if (!hilighted_only || Sys->selected_polygon == last_poly)
      polygon->Draw (rview->g2d, selcol);
  }
  else if (type == CALLBACK_POLYGONQ)
  {
    if (!hilighted_only)
    {
      G3DPolygonDPFX* dpfx = (G3DPolygonDPFX*)entity;
      int i1 = dpfx->num-1;
      int i;
      for (i = 0 ; i < dpfx->num ; i++)
      {
        rview->g2d->DrawLine (dpfx->vertices[i].sx, csWorld::frame_height - 1 - dpfx->vertices[i].sy,
      	  dpfx->vertices[i1].sx, csWorld::frame_height - 1 - dpfx->vertices[i1].sy, blue);
        i1 = i;
      }
    }
  }
  else if (type == CALLBACK_SECTOR)
  {
    csSector* sector = (csSector*)entity;
    int i;
    csVector3 v;
    float iz;
    int px, py, r;
    for (i = 0 ; i < sector->lights.Length () ; i++)
    {
      csStatLight* light = (csStatLight*)(sector->lights[i]);
      if (!hilighted_only || Sys->selected_light == light)
      {
        v = rview->Other2This (light->GetCenter ());
        if (v.z > SMALL_Z)
        {
          iz = rview->aspect/v.z;
          px = QInt (v.x * iz + rview->shift_x);
          py = csWorld::frame_height - 1 - QInt (v.y * iz + rview->shift_y);
          r = QInt (.3 * iz);
          rview->g2d->DrawLine (px-r, py-r, px+r, py+r, selcol);
          rview->g2d->DrawLine (px+r, py-r, px-r, py+r, selcol);
          rview->g2d->DrawLine (px, py-2, px, py+2, red);
          rview->g2d->DrawLine (px+2, py, px-2, py, red);
        }
      }
    }
  }
}

// Callback for DrawFunc() to show a 3D map of everything that is visible.
void draw_map (csRenderView* /*rview*/, int type, void* entity)
{
  csWireFrame* wf = Sys->wf->GetWireframe ();
  if (type == CALLBACK_POLYGON)
  {
    csPolygon3D* poly = (csPolygon3D*)entity;
    int j;
    csWfPolygon* po = wf->AddPolygon ();
    po->SetVisColor (wf->GetYellow ());
    po->SetNumVertices (poly->GetVertices ().GetNumVertices ());
    for (j = 0 ; j < poly->GetVertices ().GetNumVertices () ; j++)
      po->SetVertex (j, poly->Vwor (j));
    po->Prepare ();
  }
  else if (type == CALLBACK_SECTOR)
  {
    csSector* sector = (csSector*)entity;
    int i;
    for (i = 0 ; i < sector->lights.Length () ; i++)
    {
      csWfVertex* vt = wf->AddVertex (((csStatLight*)sector->lights[i])->GetCenter ());
      vt->SetColor (wf->GetRed ());
    }
  }
}

// Callback for DrawFunc() to dump debug information about everything
// that is currently visible. This is useful to debug clipping errors
// and other visual errors.
int dump_visible_indent = 0;
void dump_visible (csRenderView* /*rview*/, int type, void* entity)
{
  int i;
  char indent_spaces[255];
  int ind = dump_visible_indent;
  if (ind > 254) ind = 254;
  for (i = 0 ; i < ind ; i++) indent_spaces[i] = ' ';
  indent_spaces[ind] = 0;

  if (type == CALLBACK_POLYGON)
  {
    csPolygon3D* poly = (csPolygon3D*)entity;
    const char* name = poly->GetName ();
    if (!name) name = "(NULL)";
    const char* pname = ((csPolygonSet*)poly->GetParent ())->GetName ();
    if (!pname) pname = "(NULL)";
    Sys->Printf (MSG_DEBUG_0, "%03d%sPolygon '%s/%s' ------\n",
    	dump_visible_indent, indent_spaces, pname, name);
    if (poly->GetPortal ())
      Sys->Printf (MSG_DEBUG_0, "%03d%s   | Polygon has a portal.\n",
        dump_visible_indent, indent_spaces);
    for (i = 0 ; i < poly->GetVertices ().GetNumVertices () ; i++)
    {
      csVector3& vw = poly->Vwor (i);
      csVector3& vc = poly->Vcam (i);
      Sys->Printf (MSG_DEBUG_0, "%03d%s   | %d: wor=(%f,%f,%f) cam=(%f,%f,%f)\n",
      	dump_visible_indent, indent_spaces, i, vw.x, vw.y, vw.z, vc.x, vc.y, vc.z);
    }
  }
  else if (type == CALLBACK_POLYGON2D)
  {
    csPolygon2D* poly = (csPolygon2D*)entity;
    Sys->Printf (MSG_DEBUG_0, "%03d%s2D Polygon ------\n", dump_visible_indent, indent_spaces);
    for (i = 0 ; i < poly->GetNumVertices () ; i++)
    {
      csVector2 v = *poly->GetVertex (i);
      Sys->Printf (MSG_DEBUG_0, "%03d%s   | %d: persp=(%f,%f)\n",
      	dump_visible_indent, indent_spaces, i, v.x, v.y);
    }
  }
  else if (type == CALLBACK_POLYGONQ)
  {
    // G3DPolygonDPQ* dpq = (G3DPolygonDPQ*)entity;
  }
  else if (type == CALLBACK_SECTOR)
  {
    csSector* sector = (csSector*)entity;
    const char* name = sector->GetName ();
    if (!name) name = "(NULL)";
    Sys->Printf (MSG_DEBUG_0, "%03d%s BEGIN Sector '%s' ------------\n",
    	dump_visible_indent+1, indent_spaces, name);
    for (i = 0 ; i < sector->GetNumVertices () ; i++)
    {
      csVector3& vw = sector->Vwor (i);
      csVector3& vc = sector->Vcam (i);
      Sys->Printf (MSG_DEBUG_0, "%03d%s   | %d: wor=(%f,%f,%f) cam=(%f,%f,%f)\n",
      	dump_visible_indent+1, indent_spaces, i, vw.x, vw.y, vw.z, vc.x, vc.y, vc.z);
    }
    dump_visible_indent++;
  }
  else if (type == CALLBACK_SECTOREXIT)
  {
    csSector* sector = (csSector*)entity;
    const char* name = sector->GetName ();
    if (!name) name = "(NULL)";
    Sys->Printf (MSG_DEBUG_0, "%03d%sEXIT Sector '%s' ------------\n",
    	dump_visible_indent, indent_spaces, name);
    dump_visible_indent--;
  }
  else if (type == CALLBACK_THING)
  {
    csThing* thing = (csThing*)entity;
    const char* name = thing->GetName ();
    if (!name) name = "(NULL)";
    Sys->Printf (MSG_DEBUG_0, "%03d%s BEGIN Thing '%s' ------------\n",
    	dump_visible_indent+1, indent_spaces, name);
    for (i = 0 ; i < thing->GetNumVertices () ; i++)
    {
      csVector3& vw = thing->Vwor (i);
      csVector3& vc = thing->Vcam (i);
      Sys->Printf (MSG_DEBUG_0, "%03d%s   | %d: wor=(%f,%f,%f) cam=(%f,%f,%f)\n",
      	dump_visible_indent+1, indent_spaces, i, vw.x, vw.y, vw.z, vc.x, vc.y, vc.z);
    }
    dump_visible_indent++;
  }
  else if (type == CALLBACK_THINGEXIT)
  {
    csThing* thing = (csThing*)entity;
    const char* name = thing->GetName ();
    if (!name) name = "(NULL)";
    Sys->Printf (MSG_DEBUG_0, "%03d%sEXIT Thing '%s' ------------\n",
    	dump_visible_indent, indent_spaces, name);
    dump_visible_indent--;
  }
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

  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
  int col = (int)((40.0-z1)*(255./40.));
  if (col < 0) col = 0;
  else if (col > 255) col = 255;
  int color = txtmgr->FindRGB (col, col, col);

  Gfx2D->DrawLine (px1, py1, px2, py2, color);
}




void DrawOctreeBoxes (csOctreeNode* node,
	int level, int draw_level)
{
  if (!node) return;
  if (level > draw_level) return;
  if (level == draw_level)
  {
    csCamera* cam = Sys->view->GetCamera ();
    const csBox3& box = node->GetBox ();
    DrawLineDepth (cam->Other2This (box.GetCorner (0)),
  		   cam->Other2This (box.GetCorner (1)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (0)),
  		   cam->Other2This (box.GetCorner (2)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (0)),
  		   cam->Other2This (box.GetCorner (4)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (7)),
  		   cam->Other2This (box.GetCorner (3)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (7)),
  		   cam->Other2This (box.GetCorner (6)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (6)),
  		   cam->Other2This (box.GetCorner (2)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (6)),
  		   cam->Other2This (box.GetCorner (4)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (3)),
  		   cam->Other2This (box.GetCorner (2)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (3)),
  		   cam->Other2This (box.GetCorner (1)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (5)),
  		   cam->Other2This (box.GetCorner (1)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (5)),
  		   cam->Other2This (box.GetCorner (4)), cam->aspect);
    DrawLineDepth (cam->Other2This (box.GetCorner (5)),
  		   cam->Other2This (box.GetCorner (7)), cam->aspect);
  }
  int i;
  for (i = 0 ; i < 8 ; i++)
    DrawOctreeBoxes (node->GetChild (i), level+1, draw_level);
}


void DrawOctreeBoxes (int draw_level)
{
  csCamera* cam = Sys->view->GetCamera ();
  csSector* sector = cam->GetSector ();
  csOctree* tree = (csOctree*)sector->GetStaticTree ();
  if (!tree) return;
  csOctreeNode* node = tree->GetRoot ();
  DrawOctreeBoxes (node, 0, draw_level);
}

