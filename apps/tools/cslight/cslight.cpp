/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "csutil/cscolor.h"
#include "csutil/stringarray.h"
#include "csutil/sysfunc.h"
#include "iengine/engine.h"
#include "igraphic/imageio.h"
#include "imap/loader.h"
#include "iutil/cmdline.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/natwin.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "cslight.h"

CS_IMPLEMENT_APPLICATION

// The global system driver
Lighter* System;

//-----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csCsLightProgressMeter)
  SCF_IMPLEMENTS_INTERFACE (iProgressMeter)
SCF_IMPLEMENT_IBASE_END

csCsLightProgressMeter::csCsLightProgressMeter (int n)
	: granularity(10), total(n), current(0), tick_scale(2),
	  anchor(0)
{
  SCF_CONSTRUCT_IBASE (0);
}

csCsLightProgressMeter::~csCsLightProgressMeter()
{
  SCF_DESTRUCT_IBASE ();
}

void csCsLightProgressMeter::SetProgressDescription (const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  SetProgressDescriptionV (id, description, arg);
  va_end (arg);
}

void csCsLightProgressMeter::SetProgressDescriptionV (const char* /*id*/,
	const char* description, va_list list)
{
  cur_description.FormatV (description, list);
}

void csCsLightProgressMeter::Step(unsigned int n)
{
  if (current < total)
  {
    current+=n;
    if (current>total) current = total;

    int const units = (current == total ? 100 :
      (((100 * current) / total) / granularity) * granularity);
    int const extent = units / tick_scale;
    if (anchor < extent)
    {
      csString buff; // Batch the update here before emitting it.
      int i;
      for (i = anchor + 1; i <= extent; i++)
      {
        if (i % (10 / tick_scale) != 0)
	  buff << '.';
	else
	{
	  buff.AppendFmt ("%d%%", i * tick_scale);
	}
      }
      csPrintf ("%s", buff.GetData());
      anchor = extent;
    }
    if (current == total)
      csPrintf ("\n");

    int fw = System->g2d->GetWidth ();
    int fh = System->g2d->GetHeight ();
    int where = current * (fw-20) / total;
    System->g3d->BeginDraw (CSDRAW_2DGRAPHICS);
    System->g2d->Clear (System->color_bg);
    int lw, lh;
    System->logo->GetRendererDimensions (lw, lh);
    int w = lw * fw / 300;
    int h = lh * fh / 200;
    System->g3d->DrawPixmap (System->logo, (fw - w)/2, 20, w, h,
	0, 0, lw, lh);
    if (System->font)
      System->g2d->Write (System->font, 20, fh*3/4-40, System->color_text,
        System->color_bg, cur_description);
    System->g2d->DrawBox (10, fh*3/4-10, where, 20, System->color_done);
    System->g2d->DrawBox (10+where, fh*3/4-10, fw-where-20, 20, System->color_todo);
    System->g3d->FinishDraw ();
    System->g3d->Print (0);
  }
}

void csCsLightProgressMeter::Restart()
{
  Reset();
  csPrintf ("0%%");
}

void csCsLightProgressMeter::Abort ()
{
  current = total;
  csPrintf ("\n");
}

void csCsLightProgressMeter::Finalize ()
{
  current = total;
  csPrintf ("\n");
}

void csCsLightProgressMeter::SetGranularity(int n)
{
  if (n >= 1 && n <= 100)
    granularity = n;
}

//-----------------------------------------------------------------------------

Lighter::Lighter ()
{
}

Lighter::~Lighter ()
{
}

void Lighter::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.application.cslight", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
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

bool Lighter::SetMapDir (const char* map_dir)
{
  csRef<iVFS> myVFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  csStringArray paths;
  paths.Push ("/lev/");
  if (!myVFS->ChDirAuto (map_dir, &paths, 0, "world"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error setting directory '%s'!",
    	map_dir);
    return false;
  }
  return true;
}

bool Lighter::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Can't init app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_PLUGIN("crystalspace.font.server.freetype2", iFontServer),
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Can't init app!");
    return false;
  }

  csRef<iStandardReporterListener> repl (CS_QUERY_REGISTRY (object_reg, iStandardReporterListener));
  if (repl)
  {
    // tune the reporter to be a bit more chatty
    repl->SetMessageDestination (
  	  CS_REPORTER_SEVERITY_BUG, false, true, true, true, true);
    repl->SetMessageDestination (
  	  CS_REPORTER_SEVERITY_ERROR, false, true, true, true, true);
    repl->SetMessageDestination (
  	  CS_REPORTER_SEVERITY_WARNING, true, false, true, false, true);
    repl->SetMessageDestination (
  	  CS_REPORTER_SEVERITY_NOTIFY, true, false, true, false, true);
    repl->SetMessageDestination (
  	  CS_REPORTER_SEVERITY_DEBUG, true, false, true, false, true);
    repl->ShowMessageID (CS_REPORTER_SEVERITY_WARNING, true);
    repl->ShowMessageID (CS_REPORTER_SEVERITY_NOTIFY, true);
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    exit (-1);
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    exit (-1);
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    exit (-1);
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  g2d = g3d->GetDriver2D ();
  iNativeWindow* nw = g2d->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Lighting Application");
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  // Setup the texture manager.
  iTextureManager* txtmgr = g3d->GetTextureManager ();
  color_bg = g2d->FindRGB (0, 0, 0);
  color_text = g2d->FindRGB (200, 220, 255);
  color_done = g2d->FindRGB (255, 0, 0);
  color_todo = g2d->FindRGB (0, 200, 200);

  // Setup font.
  iFontServer* fntsvr = g2d->GetFontServer ();
  if (fntsvr)
  {
    font = fntsvr->LoadFont (CSFONT_COURIER);
  }
  else
    font = 0;

  // Load logo.
  logo = loader->LoadTexture ("/lib/std/cslogo.gif",
	CS_TEXTURE_2D, txtmgr);
  logo->SetKeyColor (0, 0, 255);

  engine->SetLightingCacheMode (CS_ENGINE_CACHE_WRITE);

  // First look for a cache: entry.
  int cmd_idx = 0;
  for (;;)
  {
    csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	  iCommandLineParser));
    const char* val = cmdline->GetName (cmd_idx);
    cmd_idx++;
    if (!val) break;
    if (strlen (val) > 7 && !strncmp ("cache:", val, 6))
    {
      val += 6;
      if (!SetMapDir (val))
      {
        Cleanup ();
        exit (1);
      }

      // First we force a clear of the cache manager in the engine
      // so that a new one will be made soon.
      engine->SetCacheManager (0);
      // And then we get the cache manager which will force it
      // to be created based on current VFS dir.
      engine->GetCacheManager ();
      break;
    }
  }
  
  cmd_idx = 0;
  int map_idx = 0;
  for (;;)
  {
    csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	  iCommandLineParser));
    const char* val = cmdline->GetName (cmd_idx);
    cmd_idx++;
    if (!val)
    {
      if (map_idx > 0)
      {
	// We already have one map so it is sufficient here.
	break;
      }
      Report (CS_REPORTER_SEVERITY_ERROR,
    	  "Please give a level (either a zip file or a VFS dir)!");
      Cleanup ();
      exit (1);
    }
    if (strlen (val) > 7 && !strncmp ("cache:", val, 6))
    {
      // We already treated the cache entry so we just continue here.
      continue;
    }
    if (!SetMapDir (val))
    {
      Cleanup ();
      exit (1);
    }

    // Load the level file which is called 'world'.
    if (!loader->LoadMapFile ("world", map_idx == 0))	// Only clear engine for first level.
    {
      Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't load level '%s'!", val);
      Cleanup ();
      exit (1);
    }
    map_idx++;
  }

  csCsLightProgressMeter* meter = new csCsLightProgressMeter (320);
  engine->Prepare (meter);
  delete meter;

  return true;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (0));

  // Create our main class.
  System = new Lighter ();

  // Initialize the main system. This will load all needed plug-ins.
  if (!System->Initialize (argc, argv, 0))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Cleanup.
  Cleanup ();

  return 0;
}
