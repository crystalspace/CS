/*
    Copyright (C) 2000 by Jorrit Tyberghein
    With additions by Samuel Humphreys

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
#include "csutil/sysfunc.h"
#include "cstool/initapp.h"
#include "apps/perftest/perftest.h"
#include "apps/perftest/ptests3d.h"
#include "apps/perftest/ptests2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivideo/material.h"
#include "ivaria/conout.h"
#include "igraphic/imageio.h"
#include "igraphic/image.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "csutil/cmdhelp.h"
#include "ivaria/reporter.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "csutil/event.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global system driver
PerfTest *System;

PerfTest::PerfTest ()
{
  draw_3d = true;
  draw_2d = true;
  test_skip = false;
}

PerfTest::~PerfTest ()
{
}

void PerfTest::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.application.perftest", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
    fflush (stdout);
  }
  va_end (arg);
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = 0;
  csInitializer::DestroyApplication (object_reg);
}

csPtr<iMaterialHandle> PerfTest::LoadMaterial (char* file)
{
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  csRef<iImage> image;
  csRef<iDataBuffer> buf (myVFS->ReadFile (file));
  if (!buf || !buf->GetSize ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error loading texture '%s'!", file);
    exit (-1);
  }
  image = ImageLoader->Load (buf->GetUint8 (), buf->GetSize (),
  	txtmgr->GetTextureFormat ());
  if (!image) exit (-1);
  csRef<iTextureHandle> texture (
  	txtmgr->RegisterTexture (image, CS_TEXTURE_3D));
  if (!texture) exit (-1);
  return txtmgr->RegisterMaterial (texture);
}

static bool PerfEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    System->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    System->FinishFrame ();
    return true;
  }
  else
  {
    return System ? System->HandleEvent (ev) : false;
  }
}

bool PerfTest::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_CONSOLEOUT,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, PerfEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    return false;
  }

  iGraphics2D* myG2D = myG3D->GetDriver2D ();
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Graphics Performance Tester");

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  ImageLoader = CS_QUERY_REGISTRY (object_reg, iImageIO);
  if (!ImageLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No image loader plugin!");
    return false;
  }

  myVFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!myVFS)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iVFS plugin!");
    return false;
  }

  // Setup the texture manager
  iTextureManager* txtmgr = myG3D->GetTextureManager ();

  // Initialize textures.
  materials[0] = LoadMaterial ("/lib/std/stone4.gif");
  materials[1] = LoadMaterial ("/lib/std/mystone2.gif");
  materials[2] = LoadMaterial ("/lib/std/andrew_marble4.gif");
  materials[3] = LoadMaterial ("/lib/std/andrew_wood.gif");
  txtmgr->PrepareTextures ();

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "Crystal Space 3D Performance Tester 0.1.");

  const char *val;
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));
  val = cmdline->GetOption ("2d");
  if (val)
  {
    current_tester = new StringTester ();
    draw_3d = false;
  }
  else
    current_tester = new SinglePolygonTester ();

  val = cmdline->GetOption ("3d");
  if (val)
    draw_2d = false;

  needs_setup = true;

  return true;
}

static csTicks last_time;

void PerfTest::SetupFrame ()
{
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (draw_3d ? CSDRAW_3DGRAPHICS : CSDRAW_2DGRAPHICS))
    return;

  // Setup if needed.
  if (needs_setup)
  {
    current_tester->Setup (myG3D, this);
    last_time = current_time;
  }

  // Do the test frame.
  if (current_tester)
  {
    current_tester->Draw (myG3D);
    if (test_skip || (current_time-last_time >= 10000))
    {
      test_skip = false;
      float totalelapsed = float(current_time - last_time)/1000.f;
      Report (CS_REPORTER_SEVERITY_NOTIFY, "%f FPS",
      	current_tester->GetCount ()/totalelapsed);
      Tester* next_tester = current_tester->NextTester ();
      delete current_tester;
      current_tester = next_tester;
      if (current_tester)
      {
        needs_setup = true;
	current_tester->Setup (myG3D, this);
      }
      else
      {
	if (draw_3d && draw_2d)
	{
	  draw_3d = false;
	  current_tester = new LineTester2D ();
	  needs_setup = true;
	  current_tester->Setup (myG3D, this);
	}
	else
	{
	  csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
	  if (q)
	    q->GetEventOutlet()->Broadcast (cscmdQuit);
        }
      }
    }
  }

  // Start drawing 2D graphics.
  if (needs_setup)
  {
    if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    csRef<iConsoleOutput> Console (
    	CS_QUERY_REGISTRY (object_reg, iConsoleOutput));
    if (Console)
      Console->Clear ();
    last_time = current_time;
    char desc[255];
    current_tester->Description (desc);
    Report (CS_REPORTER_SEVERITY_NOTIFY, desc);
    needs_setup = false;
  }
}

void PerfTest::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (0);
}

bool PerfTest::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }
  else if ((Event.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_SPACE))
  {
    test_skip = true;
    return true;
  }

  return false;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (0));

  // Create our main class.
  System = new PerfTest ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, 0))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  csDefaultRunLoop(System->object_reg);

  // Cleanup.
  Cleanup ();

  return 0;
}
