/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "cstool/vidprefs.h"
#include "cstool/initapp.h"
#include "csutil/objreg.h"
#include "csutil/csevent.h"

#include "iaws/aws.h"
#include "iaws/awscnvs.h"
#include "iutil/plugin.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "iutil/vfs.h"
#include "iutil/cfgmgr.h"
#include "ivaria/reporter.h"
#include "igraphic/imageio.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "ivideo/txtmgr.h"

csVideoPreferences::csVideoPreferences ()
{
  object_reg = NULL;
  plugmgr = NULL;
  aws = NULL;
  aws_canvas = NULL;
  g2d = NULL;
  g3d = NULL;
  fontserv = NULL;
  imageio = NULL;
  vfs = NULL;
}

csVideoPreferences::~csVideoPreferences ()
{
  CleanUp ();
}

void csVideoPreferences::CleanUp ()
{
  iConfigManager* cfgmgr = CS_QUERY_REGISTRY (object_reg, iConfigManager);

  if (aws_canvas) { aws_canvas->DecRef (); aws_canvas = NULL; }
  if (imageio)
  {
    object_reg->Unregister (imageio, "iImageIO");
    iComponent* comp = SCF_QUERY_INTERFACE (imageio, iComponent);
    plugmgr->UnloadPlugin (comp);
    comp->DecRef ();
    imageio->DecRef ();
    imageio = NULL;
  }
  if (aws)
  {
    object_reg->Unregister (aws, "iAws");
    iComponent* comp = SCF_QUERY_INTERFACE (aws, iComponent);
    plugmgr->UnloadPlugin (comp);
    comp->DecRef ();
    aws->DecRef ();
    aws = NULL;
  }
  if (g3d)
  {
    object_reg->Unregister (g3d, "iGraphics3D");
    iComponent* comp = SCF_QUERY_INTERFACE (g3d, iComponent);
    plugmgr->UnloadPlugin (comp);
    comp->DecRef ();
    g3d->DecRef ();
    g3d = NULL;
  }
  if (g2d)
  {
    object_reg->Unregister (g2d, "iGraphics2D");
    iComponent* comp = SCF_QUERY_INTERFACE (g2d, iComponent);
    plugmgr->UnloadPlugin (comp);
    comp->DecRef ();
    g2d->DecRef ();
    g2d = NULL;
  }
  if (fontserv)
  {
    object_reg->Unregister (fontserv, "iFontServer");
    iComponent* comp = SCF_QUERY_INTERFACE (fontserv, iComponent);
    plugmgr->UnloadPlugin (comp);
    comp->DecRef ();
    fontserv->DecRef ();
    fontserv = NULL;
  }
  if (vfs) { vfs->DecRef (); vfs = NULL; }
  if (plugmgr) { plugmgr->DecRef (); plugmgr = NULL; }

  cfgmgr->FlushRemoved ();
  cfgmgr->DecRef ();
}

void csVideoPreferences::SelectMode ()
{
  CleanUp ();
  iPluginManager* plugmgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  iFontServer* fs = CS_LOAD_PLUGIN (plugmgr, "crystalspace.font.server.default",
  	iFontServer);
  object_reg->Register (fs, "iFontServer");
  fs->DecRef ();

  if (mode == 0)
  {
    iGraphics3D* g;
    g = CS_LOAD_PLUGIN (plugmgr, "crystalspace.graphics3d.software",
  	iGraphics3D);
    object_reg->Register (g, "iGraphics3D");
    g->DecRef ();
  }
  else
  {
    iGraphics3D* g;
    g = CS_LOAD_PLUGIN (plugmgr, "crystalspace.graphics3d.opengl",
  	iGraphics3D);
    object_reg->Register (g, "iGraphics3D");
    g->DecRef ();
  }
  plugmgr->DecRef ();
}

bool csVideoPreferences::Setup (iObjectRegistry* object_reg)
{
  csVideoPreferences::object_reg = object_reg;

  plugmgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  if (!plugmgr)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't find plugin manager!");
    return false;
  }

  //---------
  // VFS
  //---------
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!vfs)
  {
    vfs = CS_LOAD_PLUGIN (plugmgr, "crystalspace.kernel.vfs", iVFS);
    if (vfs)
    {
      object_reg->Register (vfs, "iVFS");
    }
  }
  if (!vfs)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't find VFS plugin!");
    return false;
  }

  //---------
  // iImageIO
  //---------
  imageio = CS_LOAD_PLUGIN (plugmgr, "crystalspace.graphic.image.io.multiplex",
  	iImageIO);
  if (!imageio)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't load image loader!");
    return false;
  }
  object_reg->Register (imageio, "iImageIO");

  //---------
  // AWS
  //---------
  aws = CS_LOAD_PLUGIN (plugmgr, "crystalspace.window.alternatemanager", iAws);
  if (!aws)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't load AWS plugin!");
    return false;
  }
  object_reg->Register (aws, "iAws");

  //---------
  // iFontServer
  //---------
  fontserv = CS_LOAD_PLUGIN (plugmgr, "crystalspace.font.server.default",
  	iFontServer);
  if (!fontserv)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't load font server!");
    return false;
  }
  object_reg->Register (fontserv, "iFontServer");

  //---------
  // iGraphics3D
  //---------
  g3d = CS_LOAD_PLUGIN (plugmgr, "crystalspace.graphics3d.software",
  	iGraphics3D);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't load software renderer!");
    return false;
  }
  object_reg->Register (g3d, "iGraphics3D");

  //---------
  // iGraphics2D
  //---------
  g2d = g3d->GetDriver2D ();
  if (!g2d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't find a 2D canvas!");
    return false;
  }
  object_reg->Register (g2d, "iGraphics2D");

  exit_loop = false;

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.tools.vidprefs",
    	"Error opening system!");
    return false;
  }

  iTextureManager* txtmgr = g3d->GetTextureManager ();
  txtmgr->SetPalette ();

  //---------
  // The window
  //---------
  if (!SetupWindow ())
    return false;

  return true;
}

bool csVideoPreferences::SetupWindow ()
{
  aws_canvas = aws->CreateCustomCanvas (g2d, g3d);

  aws->SetFlag (AWSF_AlwaysRedrawWindows);
  aws->SetCanvas (aws_canvas);

  // Setup sink.
  iAwsSink* sink = aws->GetSinkMgr ()->CreateSink ((void*)this);
  sink->RegisterTrigger ("Software", &SetSoftware);
  sink->RegisterTrigger ("OpenGL", &SetOpenGL);
  aws->GetSinkMgr ()->RegisterSink ("VidPrefsSink", sink);

  // now load preferences
  if (!aws->GetPrefMgr()->Load ("/this/data/temp/vidprefs.def"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't load window definition file!");
    return false;
  }
  aws->GetPrefMgr ()->SelectDefaultSkin ("Normal Windows");

  iAwsWindow *test = aws->CreateWindowFrom ("PrefsWindow");
  if (test) test->Show ();
  return true;
}

bool csVideoPreferences::HandleEvent (iEvent& ev)
{
  if (exit_loop) return true;
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return true;
    g2d->Clear (0);
    aws->Redraw ();
    aws->Print (g3d, 64);
    return false;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    g3d->FinishDraw ();
    g3d->Print (NULL);
    return false;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdSystemClose)
  {
    return false;
  }
  else
  {
    aws->HandleEvent (ev);
    return false;
  }

  return false;
}

void csVideoPreferences::SetSoftwareL (iAwsSource *)
{
  printf ("Software mode!\n"); fflush (stdout);

  csInitializer::CloseApplication (object_reg);
  mode = 0;
  exit_loop = true;
}

void csVideoPreferences::SetOpenGLL (iAwsSource *)
{
  printf ("OpenGL mode!\n"); fflush (stdout);

  csInitializer::CloseApplication (object_reg);
  mode = 1;
  exit_loop = true;
}

void csVideoPreferences::SetSoftware (void* vp, iAwsSource* source)
{
  csVideoPreferences* vidprefs = (csVideoPreferences*)vp;
  vidprefs->SetSoftwareL (source);
}

void csVideoPreferences::SetOpenGL (void* vp, iAwsSource* source)
{
  csVideoPreferences* vidprefs = (csVideoPreferences*)vp;
  vidprefs->SetOpenGLL (source);
}

