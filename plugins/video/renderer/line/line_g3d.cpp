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

#include "sysdef.h"
#include "qint.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "cs3d/line/line_g3d.h"
#include "cs3d/line/line_txt.h"
#include "cs3d/common/memheap.h"
#include "csutil/inifile.h"
#include "ipolygon.h"
#include "isystem.h"
#include "igraph2d.h"
#include "ilghtmap.h"

///---------------------------------------------------------------------------

IMPLEMENT_FACTORY (csGraphics3DLine)

EXPORT_CLASS_TABLE (line3d)
  EXPORT_CLASS (csGraphics3DLine, "crystalspace.graphics3d.line",
    "Line 3D graphics driver for Crystal Space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics3DLine)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
  IMPLEMENTS_EMBEDDED_INTERFACE (iConfig)
IMPLEMENT_IBASE_END

#if defined (OS_LINUX)
char* get_software_2d_driver ()
{
  if (getenv ("GGI_DISPLAY"))
    return SOFTWARE_2D_DRIVER_GGI;
  else if (getenv ("DISPLAY"))
    return "crystalspace.graphics2d.linexlib";
  else
    return SOFTWARE_2D_DRIVER_SVGA;
}
#elif defined (OS_UNIX) && !defined (OS_BE)
// by the way, other unices has SVGALib support too... through GGI ;-)
char* get_software_2d_driver ()
{
  if (getenv ("GGI_DISPLAY"))
    return SOFTWARE_2D_DRIVER_GGI;
  else
    return "crystalspace.graphics2d.linexlib";
}
#endif

csGraphics3DLine::csGraphics3DLine (iBase *iParent) : G2D (NULL)
{
  CONSTRUCT_IBASE (iParent);
  CONSTRUCT_EMBEDDED_IBASE (scfiConfig);

  config = new csIniFile ("line3d.cfg");

  txtmgr = NULL;
}

csGraphics3DLine::~csGraphics3DLine ()
{
  Close ();
  CHK (delete txtmgr);
  if (G2D)
    G2D->DecRef ();
  CHK (delete config;)
}

bool csGraphics3DLine::Initialize (iSystem *iSys)
{
  System = iSys;

  width = height = -1;

  if (!System->RegisterDriver ("iGraphics3D", this))
    return false;

  char *driver = config->GetStr ("Hardware", "Driver2D", SOFTWARE_2D_DRIVER);
  G2D = LOAD_PLUGIN (System, driver, iGraphics2D);
  if (!G2D)
    return false;

  CHK (txtmgr = new csTextureManagerLine (System, G2D));
  txtmgr->SetConfig (config);
  txtmgr->Initialize ();

  return true;
}

bool csGraphics3DLine::Open (const char *Title)
{
  DrawMode = 0;

  if (!G2D->Open (Title))
  {
    SysPrintf (MSG_FATAL_ERROR, "Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;

    return false;
  }

  int nWidth = G2D->GetWidth ();
  int nHeight = G2D->GetHeight ();
  bool bFullScreen = G2D->GetFullScreen ();

  SetDimensions (nWidth, nHeight);

  SysPrintf (MSG_INITIALIZATION, "Using %s mode %dx%d.\n",
            bFullScreen ? "full screen" : "windowed", width, height);

  z_buf_mode = CS_ZBUF_NONE;

  return true;
}

void csGraphics3DLine::Close()
{
  if ((width == height) && (width == -1))
    return;

  G2D->Close ();
  width = height = -1;
}

void csGraphics3DLine::SetDimensions (int nwidth, int nheight)
{
  width = nwidth;
  height = nheight;
  width2 = width/2;
  height2 = height/2;
}

void csGraphics3DLine::SetPerspectiveCenter (int x, int y)
{
  width2 = x;
  height2 = y;
}

bool csGraphics3DLine::BeginDraw (int DrawFlags)
{
  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw())
      return false;
  }

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  if (DrawFlags & CSDRAW_3DGRAPHICS)
  {
    G2D->Clear (0);
  }
  else if (DrawMode & CSDRAW_3DGRAPHICS)
  {
  }

  DrawMode = DrawFlags;

  return true;
}

void csGraphics3DLine::FinishDraw ()
{
  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();
  DrawMode = 0;
}

void csGraphics3DLine::Print (csRect *area)
{
  G2D->Print (area);
}

void csGraphics3DLine::SetZBufMode (G3DZBufMode mode)
{
  z_buf_mode = mode;
}

void csGraphics3DLine::DrawPolygon (G3DPolygonDP& poly)
{
  if (poly.num < 3)
    return;
  int i;
  int color = txtmgr->FindRGB (255, 255, 255);
  for (i = 0 ; i < poly.num ; i++)
  {
    G2D->DrawLine (poly.vertices[i].sx, height-poly.vertices[i].sy,
    	poly.vertices[(i+1)%poly.num].sx, height-poly.vertices[(i+1)%poly.num].sy,
	color);
  }
}

void csGraphics3DLine::DrawPolygonFX (G3DPolygonDPFX& poly)
{
  if (poly.num < 3)
    return;
  int i;
  int color = txtmgr->FindRGB (255, 0, 0);
  for (i = 0 ; i < poly.num ; i++)
  {
    G2D->DrawLine (poly.vertices[i].sx, height-poly.vertices[i].sy,
    	poly.vertices[(i+1)%poly.num].sx, height-poly.vertices[(i+1)%poly.num].sy,
	color);
  }
}

bool csGraphics3DLine::SetRenderState (G3D_RENDERSTATEOPTION op,
  long value)
{
  switch (op)
  {
    case G3DRENDERSTATE_NOTHING:
      return true;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      if (value)
      {
         if (z_buf_mode == CS_ZBUF_TEST)
           return true;
         if (z_buf_mode == CS_ZBUF_NONE)
           z_buf_mode = CS_ZBUF_TEST;
         else if (z_buf_mode == CS_ZBUF_FILL)
           z_buf_mode = CS_ZBUF_USE;
      }
      else
      {
         if (z_buf_mode == CS_ZBUF_FILL)
           return true;
         if (z_buf_mode == CS_ZBUF_USE)
           z_buf_mode = CS_ZBUF_FILL;
         else if (z_buf_mode == CS_ZBUF_TEST)
           z_buf_mode = CS_ZBUF_NONE;
      }
      break;
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      if (value)
      {
        if (z_buf_mode == CS_ZBUF_FILL)
          return true;
        if (z_buf_mode == CS_ZBUF_NONE)
          z_buf_mode = CS_ZBUF_FILL;
        else if (z_buf_mode == CS_ZBUF_TEST)
          z_buf_mode = CS_ZBUF_USE;
      }
      else
      {
        if (z_buf_mode == CS_ZBUF_TEST)
          return true;
        if (z_buf_mode == CS_ZBUF_USE)
          z_buf_mode = CS_ZBUF_TEST;
        else if (z_buf_mode == CS_ZBUF_FILL)
          z_buf_mode = CS_ZBUF_NONE;
      }
      break;
    default:
      return false;
  }

  return true;
}

long csGraphics3DLine::GetRenderState (G3D_RENDERSTATEOPTION op)
{
  switch (op)
  {
    case G3DRENDERSTATE_NOTHING:
      return 0;
    case G3DRENDERSTATE_ZBUFFERTESTENABLE:
      return (bool)(z_buf_mode & CS_ZBUF_TEST);
    case G3DRENDERSTATE_ZBUFFERFILLENABLE:
      return (bool)(z_buf_mode & CS_ZBUF_FILL);
    default:
      return 0;
  }
}

void csGraphics3DLine::GetCaps (G3D_CAPS *caps)
{
  if (!caps)
    return;

  caps->ColorModel = G3DCOLORMODEL_RGB;
  caps->CanClip = false;
  caps->SupportsArbitraryMipMapping = true;
  caps->BitDepth = 8;
  caps->ZBufBitDepth = 32;
  caps->minTexHeight = 2;
  caps->minTexWidth = 2;
  caps->maxTexHeight = 1024;
  caps->maxTexWidth = 1024;
  caps->PrimaryCaps.RasterCaps = G3DRASTERCAPS_SUBPIXEL;
  caps->PrimaryCaps.canBlend = true;
  caps->PrimaryCaps.ShadeCaps = G3DRASTERCAPS_LIGHTMAP;
  caps->PrimaryCaps.PerspectiveCorrects = true;
  caps->PrimaryCaps.FilterCaps = G3D_FILTERCAPS((int)G3DFILTERCAPS_NEAREST | (int)G3DFILTERCAPS_MIPNEAREST);
  caps->fog = G3D_FOGMETHOD((int)G3DFOGMETHOD_NONE);
}

void csGraphics3DLine::DrawLine (csVector3& v1, csVector3& v2, float fov, int color)
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
  int px1 = QInt (x1 * iz1 + (width/2));
  int py1 = height - 1 - QInt (y1 * iz1 + (height/2));
  float iz2 = fov/z2;
  int px2 = QInt (x2 * iz2 + (width/2));
  int py2 = height - 1 - QInt (y2 * iz2 + (height/2));

  G2D->DrawLine (px1, py1, px2, py2, color);
}

void csGraphics3DLine::SysPrintf (int mode, char* szMsg, ...)
{
  char buf[1024];
  va_list arg;

  va_start (arg, szMsg);
  vsprintf (buf, szMsg, arg);
  va_end (arg);

  System->Print (mode, buf);
}

//---------------------------------------------------------------------------

IMPLEMENT_EMBEDDED_IBASE (csGraphics3DLine::csLineConfig)
  IMPLEMENTS_INTERFACE (iConfig)
IMPLEMENT_EMBEDDED_IBASE_END

#define NUM_OPTIONS 1

static const csOptionDescription config_options [NUM_OPTIONS] =
{
  { 0, "dummy", "Dummy", CSVAR_BOOL },
};

bool csGraphics3DLine::csLineConfig::SetOption (int id, csVariant* value)
{
  if (value->type != config_options[id].type)
    return false;
  switch (id)
  {
    case 0: break;
    default: return false;
  }
  return true;
}

bool csGraphics3DLine::csLineConfig::GetOption (int id, csVariant* value)
{
  value->type = config_options[id].type;
  switch (id)
  {
    case 0: value->v.b = false; break;
    default: return false;
  }
  return true;
}

bool csGraphics3DLine::csLineConfig::GetOptionDescription
  (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS)
    return false;
  *option = config_options[idx];
  return true;
}

