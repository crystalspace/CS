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
#include "walktest/walktest.h"
#include "qint.h"
#include "cssys/system.h"
#include "csgeom/frustum.h"
#include "csengine/dumper.h"
#include "csengine/stats.h"
#include "csengine/light.h"
#include "csengine/texture.h"
#include "csengine/thing.h"
#include "csengine/wirefrm.h"
#include "csengine/polytext.h"
#include "csengine/polygon.h"
#include "csengine/pol2d.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/covtree.h"
#include "csengine/solidbsp.h"
#include "csengine/meshobj.h"
#include "csengine/csview.h"
#include "csengine/octree.h"
#include "csgeom/csrect.h"
#include "csobject/dataobj.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"

#define Gfx3D System->G3D
#define Gfx2D System->G2D

extern WalkTest* Sys;

//-----------------------------------------------------------------------------

void DrawZbuffer ()
{
  for (int y = 0; y < FRAME_HEIGHT; y++)
  {
    int gi_pixelbytes = System->G2D->GetPixelBytes ();

    uint32 *zbuf = Gfx3D->GetZBuffAt (0, y);

    if (zbuf)
      if (gi_pixelbytes == 4)
      {
        uint32 *dest = (uint32 *)Gfx2D->GetPixelAt (0, y);
        for (int x = 0; x < FRAME_WIDTH; x++)
          *dest++ = *zbuf++ >> 10;
      }
      else if (gi_pixelbytes == 2)
      {
        uint16 *dest = (uint16 *)Gfx2D->GetPixelAt(0, y);
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
// which are called by csEngine::DrawFunc() or csLight::LightingFunc().
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
    csCamera* cam = Sys->view->GetCamera ()->GetPrivateObject ();
    csFrustumContext* ctxt = lview->GetFrustumContext ();
    csFrustum* fr = ctxt->GetLightFrustum ();
    csVector3 v0, v1, v2;
    csVector3 light_cam = cam->Other2This (fr->GetOrigin ());
    int j;

    for (j = 0 ; j < fr->GetNumVertices () ; j++)
    {
      v0 = fr->GetVertices ()[j] + fr->GetOrigin ();
      v1 = cam->Other2This (v0);
      v0 = fr->GetVertices ()[(j+1)%fr->GetNumVertices ()] + fr->GetOrigin ();
      v2 = cam->Other2This (v0);
      Gfx3D->DrawLine (light_cam, v1, cam->GetFOV (), red);
      Gfx3D->DrawLine (light_cam, v2, cam->GetFOV (), red);
      Gfx3D->DrawLine (v1, v2, cam->GetFOV (), white);
    }
  }
}

// Callback for DrawFunc() to select an object with the mouse. The coordinate
// to check for is in 'coord_check_vector'.
bool check_poly;
bool check_light;
csVector2 coord_check_vector;

void select_object (iRenderView* rview, int type, void* entity)
{
#if 0
//@@@@@@@@@@@@@
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
#endif
}

#if 0
/// Static vertex array.
static DECLARE_GROWING_ARRAY (walkdbg_tr_verts, csVector3);
/// The perspective corrected vertices.
static DECLARE_GROWING_ARRAY (walkdbg_persp, csVector2);
/// Array which indicates which vertices are visible and which are not.
static DECLARE_GROWING_ARRAY (walkdbg_visible, bool);

static void WalkDbgDrawMesh (G3DTriangleMesh& mesh, iGraphics3D* g3d,
	iGraphics2D* g2d)
{
  iTextureManager* txtmgr = g3d->GetTextureManager ();
  int green = txtmgr->FindRGB (0, 255, 0);
  const csReversibleTransform& o2c = g3d->GetObjectToCamera ();
  csVector2 clip_verts[64];
  int num_clipper;
  g3d->GetClipper (&clip_verts[0], num_clipper);
  csClipper* clipper;
  if (num_clipper)
    clipper = new csPolygonClipper (clip_verts, num_clipper, false, true);
  else
    clipper = NULL;
  float aspect = g3d->GetPerspectiveAspect ();
  int width2, height2;
  g3d->GetPerspectiveCenter (width2, height2);

  int i;

  // Update work tables.
  if (mesh.num_vertices > walkdbg_tr_verts.Limit ())
  {
    walkdbg_tr_verts.SetLimit (mesh.num_vertices);
    walkdbg_persp.SetLimit (mesh.num_vertices);
    walkdbg_visible.SetLimit (mesh.num_vertices);
  }

  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  csVector3* f1 = mesh.vertices[0];
  csVector3* work_verts;
  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tween_ratio = mesh.morph_factor;
    float remainder = 1 - tween_ratio;
    csVector3* f2 = mesh.vertices[1];
    if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
      for (i = 0 ; i < mesh.num_vertices ; i++)
        walkdbg_tr_verts[i] = o2c * (tween_ratio * f2[i] + remainder * f1[i]);
    else
      for (i = 0 ; i < mesh.num_vertices ; i++)
        walkdbg_tr_verts[i] = tween_ratio * f2[i] + remainder * f1[i];
    work_verts = walkdbg_tr_verts.GetArray ();
  }
  else
  {
    if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
    {
      for (i = 0 ; i < mesh.num_vertices ; i++)
        walkdbg_tr_verts[i] = o2c * f1[i];
      work_verts = walkdbg_tr_verts.GetArray ();
    }
    else
      work_verts = f1;
  }

  // Perspective project.
  for (i = 0 ; i < mesh.num_vertices ; i++)
  {
    if (work_verts[i].z >= SMALL_Z)
    {
      float z_vert = 1. / work_verts[i].z;
      float iz = aspect * z_vert;
      walkdbg_persp[i].x = work_verts[i].x * iz + width2;
      walkdbg_persp[i].y = work_verts[i].y * iz + height2;
      walkdbg_visible[i] = true;
    }
    else
      walkdbg_visible[i] = false;
  }

  // Clipped polygon (assume it cannot have more than 64 vertices)
  G3DPolygonDPFX poly;
  memset (&poly, 0, sizeof(poly));

  // The triangle in question
  csVector2 triangle[3];
  csVector2 clipped_triangle[MAX_OUTPUT_VERTICES];	//@@@BAD HARCODED!
  csVertexStatus clipped_vtstats[MAX_OUTPUT_VERTICES];
  UByte clip_result;

  // Draw all triangles.
  csTriangle* triangles = mesh.triangles;
  for (i = 0 ; i < mesh.num_triangles ; i++)
  {
    int a = triangles[i].a;
    int b = triangles[i].b;
    int c = triangles[i].c;
    if (walkdbg_visible[a] && walkdbg_visible[b] && walkdbg_visible[c])
    {
      //-----
      // Do backface culling. Note that this depends on the
      // mirroring of the current view.
      //-----
      int j;
      float area = csMath2::Area2 (walkdbg_persp [a], walkdbg_persp [b],
      	walkdbg_persp [c]);
      if (!area) continue;
      if (mesh.do_mirror)
      {
        if (area <= -SMALL_EPSILON) continue;
        triangle [2] = walkdbg_persp[a];
        triangle [1] = walkdbg_persp[b];
        triangle [0] = walkdbg_persp[c];
      }
      else
      {
        if (area >= SMALL_EPSILON) continue;
        triangle [0] = walkdbg_persp[a];
        triangle [1] = walkdbg_persp[b];
        triangle [2] = walkdbg_persp[c];
      }

      // Clip triangle. Note that the clipper doesn't care about the
      // orientation of the triangle vertices. It works just as well in
      // mirrored mode.
      int rescount = 0;
      if (mesh.do_clip && clipper)
      {
        clip_result = clipper->Clip (triangle, 3, clipped_triangle, rescount,
		clipped_vtstats);
        if (clip_result == CS_CLIP_OUTSIDE) continue;
        poly.num = rescount;
      }
      else
      {
        clip_result = CS_CLIP_INSIDE;
        poly.num = 3;
      }

      // If mirroring we store the vertices in the other direction.
      if (clip_result != CS_CLIP_INSIDE)
        for (j = 0 ; j < poly.num ; j++)
        {
          poly.vertices[j].sx = clipped_triangle[j].x;
          poly.vertices[j].sy = clipped_triangle[j].y;
        }
      else
      {
        poly.vertices [0].sx = triangle [0].x;
        poly.vertices [0].sy = triangle [0].y;
        poly.vertices [1].sx = triangle [1].x;
        poly.vertices [1].sy = triangle [1].y;
        poly.vertices [2].sx = triangle [2].x;
        poly.vertices [2].sy = triangle [2].y;
      }

      int j1 = poly.num-1;
      for (j = 0 ; j < poly.num ; j++)
      {
        g2d->DrawLine (poly.vertices[j].sx,
	  csEngine::frame_height - 1 - poly.vertices[j].sy,
      	  poly.vertices[j1].sx,
	  csEngine::frame_height - 1 - poly.vertices[j1].sy, green);
        j1 = j;
      }
    }
  }
}
#endif

// Callback for DrawFunc() to show an outline for all polygons and lights.
// If callback_data in 'rview' is not NULL then we only show outline for
// selected light and/or polygon.
void draw_edges (iRenderView* rview, int type, void* entity)
{
#if 0
//@@@@@@@@@@@@
  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
  int selcol;
  int white = txtmgr->FindRGB (255, 255, 255);
  int red = txtmgr->FindRGB (255, 0, 0);
  int blue = txtmgr->FindRGB (0, 0, 255);
  int yellow = txtmgr->FindRGB (255, 255, 0);

  bool hilighted_only = !!rview->GetCallbackData ();
  if (hilighted_only) selcol = yellow;
  else selcol = white;
  static csPolygon3D* last_poly = NULL;

  if (type == CALLBACK_POLYGON)
  {
    // Here we depend on CALLBACK_POLYGON being called right before
    // CALLBACK_POLYGON2D.
    last_poly = (csPolygon3D*)entity;
  }
  else if (type == CALLBACK_POLYGON2D)
  {
    csPolygon2D* polygon = (csPolygon2D*)entity;
    if (!hilighted_only || Sys->selected_polygon == last_poly)
      polygon->Draw (rview->GetGraphics2D (), selcol);
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
        rview->GetGraphics2D ()->DrawLine (dpfx->vertices[i].sx,
	  csEngine::frame_height - 1 - dpfx->vertices[i].sy,
      	  dpfx->vertices[i1].sx,
	  csEngine::frame_height - 1 - dpfx->vertices[i1].sy, blue);
        i1 = i;
      }
    }
  }
  else if (type == CALLBACK_MESH)
  {
    if (!hilighted_only)
    {
      G3DTriangleMesh* mesh = (G3DTriangleMesh*)entity;
      WalkDbgDrawMesh (*mesh, rview->GetG3D (), rview->GetGraphics2D ());
    }
  }
  else if (type == CALLBACK_SECTOR)
  {
    csSector* sector = (csSector*)entity;
    int i;
    csVector3 v;
    float iz;
    int px, py, r;
    for (i = 0 ; i < sector->GetLightCount () ; i++)
    {
      csStatLight* light = sector->GetLight (i);
      if (!hilighted_only || Sys->selected_light == light)
      {
        v = rview->Other2This (light->GetCenter ());
        if (v.z > SMALL_Z)
        {
          iz = rview->GetFOV ()/v.z;
          px = QInt (v.x * iz + rview->GetShiftX ());
          py = csEngine::frame_height - 1 - QInt (v.y * iz + rview->GetShiftY ());
          r = QInt (.3 * iz);
          rview->GetGraphics2D ()->DrawLine (px-r, py-r, px+r, py+r, selcol);
          rview->GetGraphics2D ()->DrawLine (px+r, py-r, px-r, py+r, selcol);
          rview->GetGraphics2D ()->DrawLine (px, py-2, px, py+2, red);
          rview->GetGraphics2D ()->DrawLine (px+2, py, px-2, py, red);
        }
      }
    }
  }
#endif
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
    for (i = 0 ; i < sector->GetLightCount () ; i++)
    {
      csWfVertex* vt = wf->AddVertex (sector->GetLight (i)->GetCenter ());
      vt->SetColor (wf->GetRed ());
    }
  }
}

// Callback for DrawFunc() to dump debug information about everything
// that is currently visible. This is useful to debug clipping errors
// and other visual errors.
int dump_visible_indent = 0;
void dump_visible (iRenderView* /*rview*/, int type, void* entity)
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
    const char* pname = poly->GetParent ()->GetName ();
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
#if 0
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
#endif
}

//------------------------------------------------------------------------
// Draw debug boxes
//------------------------------------------------------------------------

void DrawDebugBoxSide (csCamera* cam, bool do3d,
    	const csVector3& v1, const csColor& c1,
    	const csVector3& v2, const csColor& c2,
    	const csVector3& v3, const csColor& c3,
    	const csVector3& v4, const csColor& c4)
{
  G3DPolygonDPFX poly;
  csVector3 v;
  csVector2 persp;
  poly.num = 4;
  poly.use_fog = false;
  poly.mat_handle = NULL;
  poly.flat_color_r = 255;
  poly.flat_color_g = 255;
  poly.flat_color_b = 255;
  v = cam->World2Camera (v1);
  if (v.z < .01) return;
  cam->Perspective (v, persp);
  if (do3d)
  {
    if (persp.x <= 0) persp.x = 1;
    if (persp.x > FRAME_WIDTH) persp.x = FRAME_WIDTH-1;
    if (persp.y <= 0) persp.y = 1;
    if (persp.y > FRAME_HEIGHT) persp.y = FRAME_HEIGHT-1;
  }
  poly.vertices[0].sx = persp.x;
  poly.vertices[0].sy = persp.y;
  poly.vertices[0].z = 1./v.z;
  poly.vertices[0].u = 0;
  poly.vertices[0].v = 0;
  poly.vertices[0].r = c1.red;
  poly.vertices[0].g = c1.green;
  poly.vertices[0].b = c1.blue;
  v = cam->World2Camera (v2);
  if (v.z < .01) return;
  cam->Perspective (v, persp);
  if (do3d)
  {
    if (persp.x <= 0) persp.x = 1;
    if (persp.x > FRAME_WIDTH) persp.x = FRAME_WIDTH-1;
    if (persp.y <= 0) persp.y = 1;
    if (persp.y > FRAME_HEIGHT) persp.y = FRAME_HEIGHT-1;
  }
  poly.vertices[1].sx = persp.x;
  poly.vertices[1].sy = persp.y;
  poly.vertices[1].z = 1./v.z;
  poly.vertices[1].u = 1;
  poly.vertices[1].v = 0;
  poly.vertices[1].r = c2.red;
  poly.vertices[1].g = c2.green;
  poly.vertices[1].b = c2.blue;
  v = cam->World2Camera (v3);
  if (v.z < .01) return;
  cam->Perspective (v, persp);
  if (do3d)
  {
    if (persp.x <= 0) persp.x = 1;
    if (persp.x > FRAME_WIDTH) persp.x = FRAME_WIDTH-1;
    if (persp.y <= 0) persp.y = 1;
    if (persp.y > FRAME_HEIGHT) persp.y = FRAME_HEIGHT-1;
  }
  poly.vertices[2].sx = persp.x;
  poly.vertices[2].sy = persp.y;
  poly.vertices[2].z = 1./v.z;
  poly.vertices[2].u = 1;
  poly.vertices[2].v = 1;
  poly.vertices[2].r = c3.red;
  poly.vertices[2].g = c3.green;
  poly.vertices[2].b = c3.blue;
  v = cam->World2Camera (v4);
  if (v.z < .01) return;
  cam->Perspective (v, persp);
  if (do3d)
  {
    if (persp.x <= 0) persp.x = 1;
    if (persp.x > FRAME_WIDTH) persp.x = FRAME_WIDTH-1;
    if (persp.y <= 0) persp.y = 1;
    if (persp.y > FRAME_HEIGHT) persp.y = FRAME_HEIGHT-1;
  }
  poly.vertices[3].sx = persp.x;
  poly.vertices[3].sy = persp.y;
  poly.vertices[3].z = 1./v.z;
  poly.vertices[3].u = 0;
  poly.vertices[3].v = 1;
  poly.vertices[3].r = c4.red;
  poly.vertices[3].g = c4.green;
  poly.vertices[3].b = c4.blue;
  poly.mat_handle = NULL;
  poly.mixmode = CS_FX_ADD|CS_FX_GOURAUD;
  if (do3d)
  {
    Gfx3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
    Gfx3D->DrawPolygonFX (poly);
  }
  else
  {
    iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
    int color = txtmgr->FindRGB (QInt (255.*c1.red*2), 0, QInt (255.*c1.blue*2));
    Gfx2D->DrawLine (poly.vertices[0].sx, FRAME_HEIGHT-poly.vertices[0].sy, poly.vertices[1].sx, FRAME_HEIGHT-poly.vertices[1].sy, color);
    Gfx2D->DrawLine (poly.vertices[1].sx, FRAME_HEIGHT-poly.vertices[1].sy, poly.vertices[2].sx, FRAME_HEIGHT-poly.vertices[2].sy, color);
    Gfx2D->DrawLine (poly.vertices[2].sx, FRAME_HEIGHT-poly.vertices[2].sy, poly.vertices[3].sx, FRAME_HEIGHT-poly.vertices[3].sy, color);
    Gfx2D->DrawLine (poly.vertices[3].sx, FRAME_HEIGHT-poly.vertices[3].sy, poly.vertices[0].sx, FRAME_HEIGHT-poly.vertices[0].sy, color);
  }
}

void DrawDebugBox (csCamera* cam, bool do3d, const csBox3& box, float r, float b)
{
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (BOX_CORNER_xYz), csColor (r, .5, b),
      	box.GetCorner (BOX_CORNER_XYz), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_Xyz), csColor (r, .5, b),
      	box.GetCorner (BOX_CORNER_xyz), csColor (r, 0, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (BOX_CORNER_XYz), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_XYZ), csColor (r, .5, b),
      	box.GetCorner (BOX_CORNER_XyZ), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_Xyz), csColor (r, .5, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (BOX_CORNER_XYZ), csColor (r, .5, b),
      	box.GetCorner (BOX_CORNER_xYZ), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_xyZ), csColor (r, .5, b),
      	box.GetCorner (BOX_CORNER_XyZ), csColor (r, 0, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (BOX_CORNER_xYZ), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_xYz), csColor (r, .5, b),
      	box.GetCorner (BOX_CORNER_xyz), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_xyZ), csColor (r, .5, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (BOX_CORNER_xYZ), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_XYZ), csColor (r, .5, b),
      	box.GetCorner (BOX_CORNER_XYz), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_xYz), csColor (r, .5, b));
  DrawDebugBoxSide (cam, do3d,
      	box.GetCorner (BOX_CORNER_xyz), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_Xyz), csColor (r, .5, b),
      	box.GetCorner (BOX_CORNER_XyZ), csColor (r, 0, b),
      	box.GetCorner (BOX_CORNER_xyZ), csColor (r, .5, b));
}

void DrawDebugBoxes (csCamera* cam, bool do3d)
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
  if (draw_level != -1 && level > draw_level) return;
  if (level == draw_level || (draw_level == -1 && node->IsLeaf ()))
  {
    iCamera* cam = Sys->view->GetCamera ();
    csOrthoTransform& ct = cam->GetTransform ();
    const csBox3& box = node->GetBox ();
    DrawLineDepth (ct.Other2This (box.GetCorner (0)),
  		   ct.Other2This (box.GetCorner (1)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (0)),
  		   ct.Other2This (box.GetCorner (2)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (0)),
  		   ct.Other2This (box.GetCorner (4)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (7)),
  		   ct.Other2This (box.GetCorner (3)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (7)),
  		   ct.Other2This (box.GetCorner (6)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (6)),
  		   ct.Other2This (box.GetCorner (2)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (6)),
  		   ct.Other2This (box.GetCorner (4)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (3)),
  		   ct.Other2This (box.GetCorner (2)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (3)),
  		   ct.Other2This (box.GetCorner (1)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (5)),
  		   ct.Other2This (box.GetCorner (1)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (5)),
  		   ct.Other2This (box.GetCorner (4)), cam->GetFOV ());
    DrawLineDepth (ct.Other2This (box.GetCorner (5)),
  		   ct.Other2This (box.GetCorner (7)), cam->GetFOV ());
  }
  int i;
  for (i = 0 ; i < 8 ; i++)
    DrawOctreeBoxes (node->GetChild (i), level+1, draw_level);
}


void DrawOctreeBoxes (int draw_level)
{
#if 0
//@@@
  csCamera* cam = Sys->view->GetCamera ();
  csSector* sector = cam->GetSector ();
  csThing* stat = sector->GetStaticThing ();
  if (!stat) return;
  csOctree* tree = (csOctree*)stat->GetStaticTree ();
  csOctreeNode* node = tree->GetRoot ();
  DrawOctreeBoxes (node, 0, draw_level);
#endif
}

#define PLANE_X 0
#define PLANE_Y 1
#define PLANE_Z 2

#if 0
static csVector3 GetVector3 (int plane_nr, float plane_pos,
	const csVector2& p)
{
  csVector3 v;
  switch (plane_nr)
  {
    case PLANE_X: v.Set (plane_pos, p.x, p.y); break;
    case PLANE_Y: v.Set (p.x, plane_pos, p.y); break;
    case PLANE_Z: v.Set (p.x, p.y, plane_pos); break;
    default: v.Set (0, 0, 0); break;
  }
  return v;
}
#endif

void CreateSolidThings (csEngine* engine, csSector* room,
	csOctreeNode* node, int depth)
{
#if 0
//@@@
  if (!node) return;
  int side;
  csVector2 v;
  const csBox3& bbox = node->GetBox ();
  for (side = 0 ; side < 6 ; side++)
  {
    UShort mask = node->GetSolidMask (side);
    if (mask)
    {
      csBox2 box = bbox.GetSide (side);
      csVector2 cor_xy = box.GetCorner (BOX_CORNER_xy);
      csVector2 cor_XY = box.GetCorner (BOX_CORNER_XY);
      int plane_nr = side/2;
      float plane_pos = (side&1) ? bbox.Max (plane_nr) : bbox.Min (plane_nr);

      int bitnr, x, y;
      for (y = 0 ; y < 4 ; y++)
        for (x = 0 ; x < 4 ; x++)
	{
	  bitnr = y*4+x;
          if (mask & (1<<bitnr))
	  {
	    v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
      	    v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
	    csVector3 v1 = GetVector3 (plane_nr, plane_pos, v);
	    v.x = cor_xy.x + (x+1)*(cor_XY.x-cor_xy.x)/4;
	    v.y = cor_xy.y + y*(cor_XY.y-cor_xy.y)/4;
	    csVector3 v2 = GetVector3 (plane_nr, plane_pos, v);
	    v.x = cor_xy.x + (x+1)*(cor_XY.x-cor_xy.x)/4;
	    v.y = cor_xy.y + (y+1)*(cor_XY.y-cor_xy.y)/4;
	    csVector3 v3 = GetVector3 (plane_nr, plane_pos, v);
	    v.x = cor_xy.x + x*(cor_XY.x-cor_xy.x)/4;
	    v.y = cor_xy.y + (y+1)*(cor_XY.y-cor_xy.y)/4;
	    csVector3 v4 = GetVector3 (plane_nr, plane_pos, v);
	    csMaterialWrapper* white = engine->GetMaterials ()->FindByName ("white");
	    csThing* thing = new csThing ();
	    thing->AddVertex (v1);
	    thing->AddVertex (v2);
	    thing->AddVertex (v3);
	    thing->AddVertex (v4);
	    csPolygon3D* p;
	    csPolyTexGouraud* gs;
	    csMatrix3 tx_matrix;
	    csVector3 tx_vector;

	    p = thing->NewPolygon (white);
	    p->AddVertex (0); p->AddVertex (1); p->AddVertex (2);
	    p->SetTextureSpace (tx_matrix, tx_vector);
	    p->SetTextureType (POLYTXT_GOURAUD);
	    gs = p->GetGouraudInfo ();
  	    gs->Setup (p);
  	    gs->SetUV (0, 0, 0); gs->SetUV (1, 1, 0); gs->SetUV (2, 1, 1);
	    gs->AddColor (0, 1, 0, 0); gs->AddColor (1, 0, 1, 0); gs->AddColor (2, 0, 0, 1);
	    p = thing->NewPolygon (white);
	    p->AddVertex (0); p->AddVertex (2); p->AddVertex (3);
	    p->SetTextureSpace (tx_matrix, tx_vector);
	    p->SetTextureType (POLYTXT_GOURAUD);
	    gs = p->GetGouraudInfo ();
  	    gs->Setup (p);
  	    gs->SetUV (0, 0, 0); gs->SetUV (1, 1, 0); gs->SetUV (2, 1, 1);
	    gs->AddColor (0, 1, 0, 0); gs->AddColor (1, 0, 1, 0); gs->AddColor (2, 0, 0, 1);

	    p = thing->NewPolygon (white);
	    p->AddVertex (0); p->AddVertex (3); p->AddVertex (2);
	    p->SetTextureSpace (tx_matrix, tx_vector);
	    p->SetTextureType (POLYTXT_GOURAUD);
	    gs = p->GetGouraudInfo ();
  	    gs->Setup (p);
  	    gs->SetUV (0, 0, 0); gs->SetUV (1, 1, 0); gs->SetUV (2, 1, 1);
	    gs->AddColor (0, 1, 0, 0); gs->AddColor (1, 0, 1, 0); gs->AddColor (2, 0, 0, 1);
	    p = thing->NewPolygon (white);
	    p->AddVertex (0); p->AddVertex (2); p->AddVertex (1);
	    p->SetTextureSpace (tx_matrix, tx_vector);
	    p->SetTextureType (POLYTXT_GOURAUD);
	    gs = p->GetGouraudInfo ();
  	    gs->Setup (p);
  	    gs->SetUV (0, 0, 0); gs->SetUV (1, 1, 0); gs->SetUV (2, 1, 1);
	    gs->AddColor (0, 1, 0, 0); gs->AddColor (1, 0, 1, 0); gs->AddColor (2, 0, 0, 1);

	    room->AddThing (thing);
	  }
	}
    }
  }
  int i;
  for (i = 0 ; i < 8 ; i++)
    CreateSolidThings (engine, room, node->GetChild (i), depth+1);
#endif
}

struct db_frust
{
  int max_vis;
  int num_vis;
  csVector queue;
};

static db_frust dbf;

// Callback for DrawFunc() to show an outline for all polygons that
// are in the dbf queue.
void draw_frust_edges (iRenderView* rview, int type, void* entity)
{
  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
  int col = txtmgr->FindRGB (0, 0, 255);
  static csPolygon3D* last_poly;

  if (type == CALLBACK_POLYGON)
  {
    // Here we depend on CALLBACK_POLYGON being called right before CALLBACK_POLYGON2D.
    last_poly = (csPolygon3D*)entity;
  }
  else if (type == CALLBACK_POLYGON2D)
  {
    csPolygon2D* polygon = (csPolygon2D*)entity;
    int idx = dbf.queue.Find (last_poly);
    if (idx != -1)
      polygon->Draw (rview->GetGraphics2D (), col);
  }
}

void poly_db_func (csObject* obj, csFrustumView* /*lview*/)
{
  csPolygon3D* poly = (csPolygon3D*)obj;
  if (dbf.num_vis < dbf.max_vis)
  {
    dbf.num_vis++;
    dbf.queue.Push ((void*)poly);
  }
}

void curve_db_func (csObject*, csFrustumView*)
{
  //csCurve* curve = (csCurve*)obj;
}

void ShowCheckFrustum (csView* view,
	csSector* room, const csVector3& pos, int num_vis)
{
  csFrustumView lview;
  csFrustumContext* ctxt = lview.GetFrustumContext ();
  lview.SetPolygonFunction (poly_db_func);
  lview.SetCurveFunction (curve_db_func);
  dbf.max_vis = num_vis;
  dbf.num_vis = 0;
  dbf.queue.SetLength (0);
  lview.SetUserData ((void*)&dbf);
  lview.SetRadius (1000000000.);
  lview.EnableThingShadows (false);
  lview.SetDynamic (false);
  ctxt->SetLightFrustum (new csFrustum (pos));
  ctxt->GetLightFrustum ()->MakeInfinite ();
  room->CheckFrustum ((iFrustumView*)&lview);
  view->GetEngine ()->DrawFunc (view->GetCamera (),
    view->GetClipper (), draw_frust_edges);
}

