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

#include <stdarg.h>

#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "inf_g3d.h"
#include "inf_txt.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "video/renderer/inf/inf_g3d.h"
#include "video/renderer/inf/inf_txt.h"
#include "video/renderer/common/polybuf.h"
#include "iutil/cfgfile.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "qint.h"
#include "iutil/plugin.h"

///---------------------------------------------------------------------------

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics3DInfinite)
SCF_IMPLEMENT_FACTORY (csGraphics2DInfinite)

SCF_EXPORT_CLASS_TABLE (inf3d)
  SCF_EXPORT_CLASS_DEP (csGraphics3DInfinite,
    "crystalspace.graphics3d.infinite",
    "Infinite 3D graphics driver for Crystal Space",
    "crystalspace.font.server.")
  SCF_EXPORT_CLASS_DEP (csGraphics2DInfinite,
    "crystalspace.graphics2d.infinite",
    "Infinite 2D graphics driver for Crystal Space",
    "crystalspace.font.server.")
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csGraphics3DInfinite)
  SCF_IMPLEMENTS_INTERFACE (iGraphics3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iEventHandler)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iConfig)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics3DInfinite::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics3DInfinite::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics3DInfinite::eiInfiniteConfig)
  SCF_IMPLEMENTS_INTERFACE (iConfig)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csGraphics3DInfinite::csGraphics3DInfinite (iBase *iParent) :
  G2D (NULL)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);

  clipper = NULL;
  texman = NULL;
  vbufmgr = NULL;

  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.fog = G3DFOGMETHOD_NONE;
  Caps.NeedsPO2Maps = false;
  Caps.MaxAspectRatio = 32768;

  num_frames = 0;
  num_drawpolygon = 0;
  num_drawpolygonfx = 0;
  num_drawpolygon_mesh = 0;
  num_drawpolygonfx_mesh = 0;
  num_drawtrianglemesh = 0;
  num_drawpolymesh = 0;
  total_2d_time = 0;
  total_3d_time = 0;
  total_none_time = 0;
  total_time = 0;
  in_mesh = false;
  do_overdraw = false;
  do_fastmesh = false;
  pixels_drawn = 0;
  pixels_drawn_fx = 0;
}

csGraphics3DInfinite::~csGraphics3DInfinite ()
{
  Close ();
  texman->Clear();
  texman->DecRef(); texman = NULL;
  vbufmgr->DecRef (); vbufmgr = NULL;
  if (G2D) G2D->DecRef ();
}

bool csGraphics3DInfinite::Initialize (iObjectRegistry *r)
{
  object_reg = r;

  config.AddConfig(object_reg, "/config/inf3d.cfg");

  width = height = -1;

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  G2D = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.graphics2d.infinite",
  	iGraphics2D);
  plugin_mgr->DecRef ();
  if (!G2D)
    return false;
  if (!object_reg->Register (G2D, "iGraphics2D"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
  	"crystalspace.graphics3d.infinite",
	"Could not register the canvas!");
    return false;
  }

  texman = new csTextureManagerInfinite (object_reg, G2D, config);
  vbufmgr = new csPolArrayVertexBufferManager (object_reg);

  iEventQueue* q = CS_QUERY_REGISTRY(object_reg, iEventQueue);
  if (q != 0)
  {
    q->RegisterListener (&scfiEventHandler, CSMASK_Broadcast);
    q->DecRef ();
  }

  return true;
}

bool csGraphics3DInfinite::HandleEvent (iEvent& Event)
{
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
      {
        Open ();
        return true;
      }
      case cscmdSystemClose:
      {
        Close ();
        return true;
      }
    }
  return false;
}

bool csGraphics3DInfinite::Open ()
{
  DrawMode = 0;

  if (!G2D->Open ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.graphics3d.infinite",
	"Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;

    return false;
  }

  int nWidth = G2D->GetWidth ();
  int nHeight = G2D->GetHeight ();

  csPixelFormat pfmt = *G2D->GetPixelFormat ();
  texman->SetPixelFormat (pfmt);

  SetDimensions (nWidth, nHeight);
  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
  	"crystalspace.graphics3d.infinite",
	"Using virtual mode %dx%d.",width,height);
  z_buf_mode = CS_ZBUF_NONE;
  return true;
}

void csGraphics3DInfinite::Close()
{
  printf ("=============================\n");
  printf ("Rendered frames: %d\n", num_frames);
  printf ("# DrawPolygon: %d (outside DPM)\n", num_drawpolygon);
  printf ("# DrawPolygonFX: %d (outside DTM)\n", num_drawpolygonfx);
  printf ("# DrawTriangleMesh: %d\n", num_drawtrianglemesh);
  printf ("   # DrawPolygonFX: %d (inside DTM)\n", num_drawpolygonfx_mesh);
  printf ("# DrawPolygonMesh: %d\n", num_drawpolymesh);
  printf ("   # DrawPolygon: %d (inside DPM)\n", num_drawpolygon_mesh);
  printf ("Time spend in 3D rendering: %ld\n", (long)total_3d_time);
  printf ("Time spend in 2D rendering: %ld\n", (long)total_2d_time);
  printf ("Time spend while not rendering: %ld\n", (long)total_none_time);
  printf ("Total time : %ld\n", (long)total_time);
  printf ("Average FPS: %f\n", 1000. * (float)num_frames / (float)total_time);
  if (do_overdraw)
  {
    float tot_pixels = screen_pixels * (float)num_frames;
    printf ("Overdraw ratio (DrawPolygon): %f\n",
    	(pixels_drawn + pixels_drawn_fx) / tot_pixels);
    printf ("   Pixels DrawPolygon: %d\n", (int)pixels_drawn);
    printf ("   Pixels DrawPolygonFX: %d\n", (int)pixels_drawn_fx);
  }

  texman->Clear();
  texman->DecRef(); texman = NULL;
  vbufmgr->DecRef (); vbufmgr = NULL;

  if ((width == height) && (width == -1))
    return;

  G2D->Close ();
  width = height = -1;
  if (clipper) { clipper->DecRef (); clipper = NULL; }
}

void csGraphics3DInfinite::SetDimensions (int nwidth, int nheight)
{
  width = nwidth;
  height = nheight;
  width2 = width/2;
  height2 = height/2;
  screen_pixels = (float)width * (float)height;
}

void csGraphics3DInfinite::SetPerspectiveCenter (int x, int y)
{
  width2 = x;
  height2 = y;
}

void csGraphics3DInfinite::SetClipper (iClipper2D* clip, int cliptype)
{
  if (clip) clip->IncRef ();
  if (clipper) clipper->DecRef ();
  clipper = clip;
  if (!clipper) cliptype = CS_CLIPPER_NONE;
  csGraphics3DInfinite::cliptype = cliptype;
}

long csGraphics3DInfinite::GetAccurateTime ()
{
  return csGetTicks ();
}

static csTicks start2d = 0;
static csTicks start3d = 0;
static csTicks startnone = 0;
static csTicks startfirst = 0;

bool csGraphics3DInfinite::BeginDraw (int DrawFlags)
{
  if (startfirst == 0) startfirst = GetAccurateTime ();

  csTicks endnone = GetAccurateTime ();
  if (startnone != 0)
  {
    total_none_time += endnone-startnone;
    startnone = 0;
  }
  
  if ((DrawFlags & CSDRAW_3DGRAPHICS) && !(DrawMode & CSDRAW_3DGRAPHICS))
  {
    start3d = GetAccurateTime ();
  }
  else if (!(DrawFlags & CSDRAW_3DGRAPHICS) && (DrawMode & CSDRAW_3DGRAPHICS))
  {
    csTicks end3d = GetAccurateTime ();
    total_3d_time += end3d-start3d;
  }

  if ((DrawFlags & CSDRAW_2DGRAPHICS) && !(DrawMode & CSDRAW_2DGRAPHICS))
  {
    start2d = GetAccurateTime ();
  }
  else if (!(DrawFlags & CSDRAW_2DGRAPHICS) && (DrawMode & CSDRAW_2DGRAPHICS))
  {
    csTicks end2d = GetAccurateTime ();
    total_2d_time += end2d-start2d;
  }

  DrawMode = DrawFlags;

  return true;
}

void csGraphics3DInfinite::FinishDraw ()
{
  DrawMode = 0;
  startnone = GetAccurateTime ();
  total_time = startnone-startfirst;
}

void csGraphics3DInfinite::Print (csRect *area)
{
  G2D->Print (area);
  num_frames++;
}

void csGraphics3DInfinite::DrawTriangleMesh (G3DTriangleMesh& mesh)
{
  num_drawtrianglemesh++;
  if (do_fastmesh)
  {
    num_drawpolygonfx_mesh += mesh.num_triangles;
  }
  else
  {
    in_mesh = true;
    DefaultDrawTriangleMesh (mesh, this, o2c, clipper, cliptype, aspect,
      width2, height2);
    in_mesh = false;
  }
}

void csGraphics3DInfinite::DrawPolygonMesh (G3DPolygonMesh& mesh)
{
  num_drawpolygon_mesh++;
  if (do_fastmesh)
  {
    csPolArrayPolygonBuffer* polbuf = (csPolArrayPolygonBuffer*)mesh.polybuf;
    num_drawpolygon_mesh += polbuf->GetPolygonCount ();
  }
  else
  {
    in_mesh = true;
    DefaultDrawPolygonMesh (mesh, this, o2c, clipper, false, aspect,
      width2, height2);
    in_mesh = false;
  }
}

void csGraphics3DInfinite::DrawPolygon (G3DPolygonDP& poly)
{
  if (poly.num < 3) return;
  if (in_mesh)
    num_drawpolygon_mesh++;
  else
    num_drawpolygon++;
  if (do_overdraw)
  {
    int i;
    csVector2 p1 (poly.vertices[0].x, poly.vertices[0].y);
    csVector2 p2 (poly.vertices[1].x, poly.vertices[1].y);
    csVector2 p3;
    for (i = 2 ; i < poly.num ; i++)
    {
      p3.x = poly.vertices[i].x;
      p3.y = poly.vertices[i].y;
      pixels_drawn += ABS (csMath2::Area2 (p1, p2, p3) / 2);
    }
  }
}

void csGraphics3DInfinite::DrawPolygonFX (G3DPolygonDPFX& poly)
{
  if (poly.num < 3) return;
  if (in_mesh)
    num_drawpolygonfx_mesh++;
  else
    num_drawpolygonfx++;
  if (do_overdraw)
  {
    int i;
    csVector2 p1 (poly.vertices[0].x, poly.vertices[0].y);
    csVector2 p2 (poly.vertices[1].x, poly.vertices[1].y);
    csVector2 p3;
    for (i = 2 ; i < poly.num ; i++)
    {
      p3.x = poly.vertices[i].x;
      p3.y = poly.vertices[i].y;
      pixels_drawn_fx += ABS (csMath2::Area2 (p1, p2, p3) / 2);
    }
  }
}

bool csGraphics3DInfinite::SetRenderState (G3D_RENDERSTATEOPTION op,
  long value)
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      z_buf_mode = csZBufMode (value);
      break;
    default:
      return false;
  }

  return true;
}

long csGraphics3DInfinite::GetRenderState (G3D_RENDERSTATEOPTION op)
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      return z_buf_mode;
    default:
      return 0;
  }
}

void
csGraphics3DInfinite::DrawLine (const csVector3&, const csVector3&, float, int)
{
}

//---------------------------------------------------------------------------

#define NUM_OPTIONS 2

static const csOptionDescription config_options [NUM_OPTIONS] =
{
  { 0, "overdraw", "Enable overdraw calculation", CSVAR_BOOL },
  { 1, "fastmesh", "Enable fast mesh emulation", CSVAR_BOOL },
};

bool
csGraphics3DInfinite::eiInfiniteConfig::SetOption (int id, csVariant* value)
{
  if (value->GetType () != config_options[id].type)
    return false;
  switch (id)
  {
    case 0: scfParent->do_overdraw = value->GetBool (); break;
    case 1: scfParent->do_fastmesh = value->GetBool (); break;
    default: return false;
  }
  return true;
}

bool
csGraphics3DInfinite::eiInfiniteConfig::GetOption (int id, csVariant* value)
{
  switch (id)
  {
    case 0: value->SetBool (scfParent->do_overdraw); break;
    case 1: value->SetBool (scfParent->do_fastmesh); break;
    default: return false;
  }
  return true;
}

bool csGraphics3DInfinite::eiInfiniteConfig::GetOptionDescription
  (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS)
    return false;
  *option = config_options[idx];
  return true;
}

//====================================================================

// csGraphics2DInfinite functions
bool csGraphics2DInfinite::Initialize (iObjectRegistry *object_reg)
{
  if (!csGraphics2D::Initialize (object_reg))
    return false;

  pfmt.RedMask = 0xf800;
  pfmt.GreenMask = 0x07e0;
  pfmt.BlueMask = 0x001f;

  pfmt.complete ();
  pfmt.PalEntries = 0;
  pfmt.PixelBytes = 2;

  return true;
}

csGraphics2DInfinite::~csGraphics2DInfinite ()
{
  Close();
}
