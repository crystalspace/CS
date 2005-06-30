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
#include "csutil/event.h"

#include "iaws/aws.h"
//#include "iaws/awscnvs.h"
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
  object_reg = 0;
}

csVideoPreferences::~csVideoPreferences ()
{
}

bool csVideoPreferences::Setup (iObjectRegistry* object_reg)
{
  csVideoPreferences::object_reg = object_reg;

  csRef<iPluginManager> plugmgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
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
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.tools.vidprefs",
      "Couldn't find VFS plugin!");
    return false;
  }

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
  exit_loop = false;

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  g2d = g3d->GetDriver2D ();

  //---------
  // The window
  //---------
  if (!SetupWindow ())
    return false;

  return true;
}

bool csVideoPreferences::SetupWindow ()
{
  // These actions are now done be default //csn
  //aws_canvas = csPtr<iAwsCanvas> (aws->CreateCustomCanvas (g2d, g3d));
  //aws->SetFlag (AWSF_AlwaysRedrawWindows);

  aws->SetupCanvas (0, g2d, g3d);

  // Setup sink.
  iAwsSink* sink = aws->GetSinkMgr ()->CreateSink ((intptr_t)this);
  sink->RegisterTrigger ("Software", &SetSoftware);
  sink->RegisterTrigger ("OpenGL", &SetOpenGL);
  aws->GetSinkMgr ()->RegisterSink ("VidPrefsSink", sink);

  // now load preferences
  if (!aws->GetPrefMgr()->Load ("/varia/vidprefs.def"))
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
  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return true;
    //g2d->Clear (0);
    aws->Redraw ();
    aws->Print (g3d, 64);
    return false;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdFinalProcess)
  {
    g3d->FinishDraw ();
    g3d->Print (0);
    return false;
  }
  aws->HandleEvent (ev);
  return false;
}

void csVideoPreferences::SetSoftwareL (iAwsSource *)
{
  csPrintf ("Software mode!\n"); fflush (stdout);

  mode = "crystalspace.graphics3d.software";
  exit_loop = true;
}

void csVideoPreferences::SetOpenGLL (iAwsSource *)
{
  csPrintf ("OpenGL mode!\n"); fflush (stdout);

  mode = "crystalspace.graphics3d.opengl";
  exit_loop = true;
}

void csVideoPreferences::SetSoftware (intptr_t vp, iAwsSource* source)
{
  csVideoPreferences* vidprefs = (csVideoPreferences*)vp;
  vidprefs->SetSoftwareL (source);
}

void csVideoPreferences::SetOpenGL (intptr_t vp, iAwsSource* source)
{
  csVideoPreferences* vidprefs = (csVideoPreferences*)vp;
  vidprefs->SetOpenGLL (source);
}
