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
#include "cssys/sysfunc.h"
#include "csutil/cscolor.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "cslight.h"
#include "iengine/engine.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "imap/parser.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "igraphic/imageio.h"

CS_IMPLEMENT_APPLICATION

// The global system driver
Lighter* System;

//-----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (csGfxProgressMeter)
  SCF_IMPLEMENTS_INTERFACE (iProgressMeter)
SCF_IMPLEMENT_IBASE_END

csGfxProgressMeter::csGfxProgressMeter (int n)
	: granularity(10), total(n), current(0)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

void csGfxProgressMeter::SetProgressDescription (const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  SetProgressDescriptionV (id, description, arg);
  va_end (arg);
}

void csGfxProgressMeter::SetProgressDescriptionV (const char* id,
	const char* description, va_list list)
{
  vsprintf (cur_description, description, list);
}

void csGfxProgressMeter::Step()
{
  if (current < total)
  {
    current++;
    int fw = System->g2d->GetWidth ();
    int fh = System->g2d->GetHeight ();
    int where = current * (fw-20) / total;
    System->g3d->BeginDraw (CSDRAW_2DGRAPHICS);
    System->g2d->Clear (System->color_bg);
    int lw, lh;
    System->logo->GetMipMapDimensions (0, lw, lh);
    int w = lw * fw / 300;
    int h = lh * fh / 200;
    System->g3d->DrawPixmap (System->logo, (fw - w)/2, 20, w, h,
	0, 0, lw, lh);
    if (System->font)
      System->g2d->Write (System->font, 20, fh*3/4-40, System->color_bg,
      	System->color_text, cur_description);
    System->g2d->DrawBox (10, fh*3/4-10, where, 20, System->color_done);
    System->g2d->DrawBox (10+where, fh*3/4-10, fw-where-20, 20, System->color_todo);
    System->g3d->FinishDraw ();
    System->g3d->Print (NULL);
  }
}

void csGfxProgressMeter::Restart()
{
  Reset();
}

void csGfxProgressMeter::Abort ()
{
  current = total;
}

void csGfxProgressMeter::Finalize ()
{
  current = total;
}

void csGfxProgressMeter::SetGranularity(int n)
{
  if (n >= 1 && n <= 100)
    granularity = n;
}

//-----------------------------------------------------------------------------

Lighter::Lighter ()
{
  engine = NULL;
  loader = NULL;
  g3d = NULL;
  vc = NULL;
}

Lighter::~Lighter ()
{
  if (engine) engine->DecRef ();
  if (loader) loader->DecRef();
  if (g3d) g3d->DecRef ();
  if (vc) vc->DecRef ();
}

void Lighter::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (System->object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.application.cslight", msg, arg);
    rep->DecRef ();
  }
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
  delete System; System = NULL;
  csInitializer::DestroyApplication (object_reg);
}

bool Lighter::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment ();
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Can't init app!");
    return false;
  }

  csInitializer::SetupCommandLineParser (object_reg, argc, argv);
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Can't init app!");
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
  txtmgr->SetVerbose (true);
  txtmgr->ResetPalette ();
  color_bg = txtmgr->FindRGB (0, 0, 0);
  color_text = txtmgr->FindRGB (200, 220, 255);
  color_done = txtmgr->FindRGB (255, 0, 0);
  color_todo = txtmgr->FindRGB (0, 200, 200);

  // Setup font.
  iFontServer* fntsvr = g2d->GetFontServer ();
  if (fntsvr)
  {
    font = fntsvr->GetFont (0);
    if (font == NULL)
      font = fntsvr->LoadFont (CSFONT_COURIER);
  }
  else
    font = NULL;

  // Load logo.
  logo = loader->LoadTexture ("/lib/std/cslogo.gif",
	CS_TEXTURE_2D, txtmgr);
  logo->SetKeyColor (0, 0, 255);

  engine->SetLightingCacheMode (CS_ENGINE_CACHE_WRITE);

  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);

  char map_dir[255];
  const char* val = cmdline->GetName ();
  if (!val)
  {
    Report (CS_REPORTER_SEVERITY_ERROR,
    	"Please give a level (either a zip file or a VFS dir)!");
    Cleanup ();
    exit (1);
  }
  cmdline->DecRef ();
  // if an absolute path is given, copy it. Otherwise prepend "/lev/".
  if (val[0] == '/')
    strcpy (map_dir, val);
  else
    sprintf (map_dir, "/lev/%s", val);

  // Check the map and mount it if required
  char tmp [100];
  sprintf (tmp, "%s/", map_dir);
  iVFS* VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iVFS plugin!");
    exit (-1);
  }

  if (!VFS->Exists (map_dir))
  {
    char *name = strrchr (map_dir, '/');
    if (name)
    {
      name++;
      const char *valfiletype = "zip";
      sprintf (tmp, "$.$/data$/%s.%s, $.$/%s.%s, $(..)$/data$/%s.%s",
	name, valfiletype, name, valfiletype, name, valfiletype );
      VFS->Mount (map_dir, tmp);
    }
  }

  // Set VFS current directory to the level we want to load.
  VFS->ChDir (map_dir);
  VFS->DecRef ();
  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't load level '%s'!", map_dir);
    Cleanup ();
    exit (1);
  }

  csGfxProgressMeter* meter = new csGfxProgressMeter (320);
  engine->Prepare (meter);
  delete meter;

  txtmgr->SetPalette ();
  return true;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new Lighter ();

  // Initialize the main system. This will load all needed plug-ins.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Cleanup.
  Cleanup ();

  return 0;
}
