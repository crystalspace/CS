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

#define CS_SYSDEF_PROVIDE_SOFTWARE2D
#include "cssysdef.h"
#include "qint.h"
#include "null_g3d.h"
#include "null_txt.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/graph2d.h"
#include "ivaria/reporter.h"

///---------------------------------------------------------------------------

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csGraphics3DNull)


SCF_IMPLEMENT_IBASE (csGraphics3DNull)
  SCF_IMPLEMENTS_INTERFACE (iGraphics3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGraphics3DNull::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csGraphics3DNull::EventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_IBASE_END

csGraphics3DNull::csGraphics3DNull (iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);

  scfiEventHandler = 0;

  texmgrnull = 0;
  vbufmgr = 0;

  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.fog = G3DFOGMETHOD_NONE;
  Caps.MaxAspectRatio = 32768;
}

csGraphics3DNull::~csGraphics3DNull ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
    if (q != 0)
      q->RemoveListener (scfiEventHandler);
    scfiEventHandler->DecRef ();
  }

  Close ();
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csGraphics3DNull::Initialize (iObjectRegistry *r)
{
  object_reg = r;
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));

  config.AddConfig(object_reg, "/config/null3d.cfg");

  width = height = -1;

  const char *driver = cmdline->GetOption ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.Null.Canvas", CS_SOFTWARE_2D_DRIVER);

  G2D = CS_LOAD_PLUGIN (plugin_mgr, driver, iGraphics2D);
  if (!G2D)
    return false;
  if (!object_reg->Register (G2D, "iGraphics2D"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
  	"crystalspace.graphics3d.null",
	"Could not register the canvas!");
    return false;
  }

  texmgrnull = new csTextureManagerNull (object_reg, G2D, config);
  vbufmgr = new csPolArrayVertexBufferManager (object_reg);

  if (!scfiEventHandler)
    scfiEventHandler = new EventHandler (this);
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (q != 0)
    q->RegisterListener (scfiEventHandler, CSMASK_Broadcast);

  return true;
}

bool csGraphics3DNull::HandleEvent (iEvent& Event)
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

bool csGraphics3DNull::Open ()
{
  DrawMode = 0;

  if (!G2D->Open ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.graphics3d.null",
    	"Error opening Graphics2D context.");
    // set "not opened" flag
    width = height = -1;

    return false;
  }

  int nWidth = G2D->GetWidth ();
  int nHeight = G2D->GetHeight ();
  bool bFullScreen = G2D->GetFullScreen ();

  pfmt = *G2D->GetPixelFormat ();
  texmgrnull->SetPixelFormat (pfmt);

  SetDimensions (nWidth, nHeight);

  csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
  	"crystalspace.graphics3d.null", "Using %s mode %dx%d.",
            bFullScreen ? "full screen" : "windowed", width, height);

  z_buf_mode = CS_ZBUF_NONE;

  return true;
}

void csGraphics3DNull::Close()
{
  if ((width == height) && (width == -1))
    return;

  texmgrnull->Clear ();
  texmgrnull->DecRef (); texmgrnull = 0;
  vbufmgr->DecRef (); vbufmgr = 0;

  G2D->Close ();
  G2D = 0;
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

void csGraphics3DNull::Print (csRect const* area)
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

long csGraphics3DNull::GetRenderState (G3D_RENDERSTATEOPTION op) const
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      return z_buf_mode;
    default:
      return 0;
  }
}

bool csGraphics3DNull::SetOption (const char*, const char*)
{
  return false;
}

void csGraphics3DNull::DrawLine (const csVector3&, const csVector3&,
  float /*fov*/, int /*color*/)
{
}
