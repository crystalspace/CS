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

#define SYSDEF_SOFTWARE2D
#include "cssysdef.h"
#include "qint.h"
#include "null_g3d.h"
#include "null_txt.h"
#include "icfgfile.h"
#include "ipolygon.h"
#include "isystem.h"
#include "igraph2d.h"
#include "ilghtmap.h"

#define SysPrintf System->Printf

///---------------------------------------------------------------------------

IMPLEMENT_FACTORY (csGraphics3DNull)

EXPORT_CLASS_TABLE (null3d)
  EXPORT_CLASS_DEP (csGraphics3DNull, "crystalspace.graphics3d.null",
    "NULL 3D graphics driver for Crystal Space", "crystalspace.font.server.")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics3DNull)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
IMPLEMENT_IBASE_END

csGraphics3DNull::csGraphics3DNull (iBase *iParent) : G2D (NULL)
{
  CONSTRUCT_IBASE (iParent);

  texman = NULL;

  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.fog = G3DFOGMETHOD_NONE;
  Caps.NeedsPO2Maps = false;
  Caps.MaxAspectRatio = 32768;
}

csGraphics3DNull::~csGraphics3DNull ()
{
  Close ();
  delete texman;
  if (G2D)
    G2D->DecRef ();
}

bool csGraphics3DNull::Initialize (iSystem *iSys)
{
  System = iSys;

  config.AddConfig(System, "/config/null3d.cfg");

  width = height = -1;

  const char *driver = iSys->GetOptionCL ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.Null.Canvas", SOFTWARE_2D_DRIVER);

  G2D = LOAD_PLUGIN (System, driver, NULL, iGraphics2D);
  if (!G2D)
    return false;

  texman = new csTextureManagerNull (System, G2D, config);

  return true;
}

bool csGraphics3DNull::Open (const char *Title)
{
  DrawMode = 0;

  if (!G2D->Open (Title))
  {
    SysPrintf (MSG_FATAL_ERROR, "Error opening Graphics2D context.\n");
    // set "not opened" flag
    width = height = -1;

    return false;
  }

  int nWidth = G2D->GetWidth ();
  int nHeight = G2D->GetHeight ();
  bool bFullScreen = G2D->GetFullScreen ();

  pfmt = *G2D->GetPixelFormat ();
  if (pfmt.PalEntries)
  {
    // If we don't have truecolor we simulate 5:6:5 bits
    // for R:G:B in the masks anyway because we still need the
    // 16-bit format for our light mixing
    pfmt.RedShift   = RGB2PAL_BITS_G + RGB2PAL_BITS_B;
    pfmt.GreenShift = RGB2PAL_BITS_B;
    pfmt.BlueShift  = 0;
    pfmt.RedMask    = ((1 << RGB2PAL_BITS_G) - 1) << pfmt.RedShift;
    pfmt.GreenMask  = ((1 << RGB2PAL_BITS_G) - 1) << pfmt.GreenShift;
    pfmt.BlueMask   = ((1 << RGB2PAL_BITS_B) - 1);
    pfmt.RedBits    = RGB2PAL_BITS_R;
    pfmt.GreenBits  = RGB2PAL_BITS_G;
    pfmt.BlueBits   = RGB2PAL_BITS_B;
  }
  texman->SetPixelFormat (pfmt);

  SetDimensions (nWidth, nHeight);

  SysPrintf (MSG_INITIALIZATION, "Using %s mode %dx%d.\n",
            bFullScreen ? "full screen" : "windowed", width, height);

  z_buf_mode = CS_ZBUF_NONE;

  return true;
}

void csGraphics3DNull::Close()
{
  if ((width == height) && (width == -1))
    return;

  G2D->Close ();
  width = height = -1;
}

void csGraphics3DNull::SetDimensions (int nwidth, int nheight)
{
  width = nwidth;
  height = nheight;
  width2 = width/2;
  height2 = height/2;
}

void csGraphics3DNull::SetPerspectiveCenter (int x, int y)
{
  width2 = x;
  height2 = y;
}

void csGraphics3DNull::SetClipper (csVector2* /*vertices*/, int /*num_vertices*/)
{
}

void csGraphics3DNull::GetClipper (csVector2* /*vertices*/, int& num_vertices)
{
  num_vertices = 0;
}

bool csGraphics3DNull::BeginDraw (int DrawFlags)
{
  if ((G2D->GetWidth() != width) || 
      (G2D->GetHeight() != height)) 
    SetDimensions (G2D->GetWidth(), G2D->GetHeight());
  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw())
      return false;
  }

  if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  DrawMode = DrawFlags;

  return true;
}

void csGraphics3DNull::FinishDraw ()
{
  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
    G2D->FinishDraw ();
  DrawMode = 0;
}

void csGraphics3DNull::Print (csRect *area)
{
  G2D->Print (area);
}

void csGraphics3DNull::DrawPolygon (G3DPolygonDP& /*poly*/)
{
}

void csGraphics3DNull::DrawPolygonFX (G3DPolygonDPFX& /*poly*/)
{
}

bool csGraphics3DNull::SetRenderState (G3D_RENDERSTATEOPTION op,
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

long csGraphics3DNull::GetRenderState (G3D_RENDERSTATEOPTION op)
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      return z_buf_mode;
    default:
      return 0;
  }
}

void csGraphics3DNull::DrawLine (const csVector3& /*v1*/, const csVector3& /*v2*/,
  float /*fov*/, int /*color*/)
{
}
