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
#include "cssys/system.h"
#include "csutil/cscolor.h"
#include "cslight.h"
#include "iengine/engine.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "imap/parser.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "isys/plugin.h"

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
}

Lighter::~Lighter ()
{
  if (engine) engine->DecRef ();
  if (loader) loader->DecRef();
  if (g3d) g3d->DecRef ();
}

void Cleanup ()
{
  System->ConsoleOut ("Cleaning up...\n");
  delete System;
}

bool Lighter::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  iObjectRegistry* object_reg = GetObjectRegistry ();
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  iCommandLineParser* cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);

  iVFS* VFS = CS_QUERY_PLUGIN (plugin_mgr, iVFS);
  if (!VFS)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iVFS plugin!\n");
    abort ();
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_PLUGIN (plugin_mgr, iEngine);
  if (!engine)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iEngine plugin!\n");
    abort ();
  }

  loader = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_LVLLOADER, iLoader);
  if (!loader)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iLoader plugin!\n");
    abort ();
  }

  g3d = CS_QUERY_PLUGIN_ID (plugin_mgr, CS_FUNCID_VIDEO, iGraphics3D);
  if (!g3d)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iGraphics3D plugin!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Lighter"))
  {
    Printf (CS_MSG_FATAL_ERROR, "Error opening system!\n");
    Cleanup ();
    exit (1);
  }

  // Setup the texture manager.
  iTextureManager* txtmgr = g3d->GetTextureManager ();
  txtmgr->SetVerbose (true);
  g2d = g3d->GetDriver2D ();
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

  char map_dir[255];
  const char* val = cmdline->GetName ();
  if (!val)
  {
    Printf (CS_MSG_FATAL_ERROR,
    	"Please give a level (either a zip file or a VFS dir)!\n");
    Cleanup ();
    exit (1);
  }
  // if an absolute path is given, copy it. Otherwise prepend "/lev/".
  if (val[0] == '/')
    strcpy (map_dir, val);
  else
    sprintf (map_dir, "/lev/%s", val);

  // Check the map and mount it if required
  char tmp [100];
  sprintf (tmp, "%s/", map_dir);
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
  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    Printf (CS_MSG_FATAL_ERROR, "Couldn't load level '%s'!\n", map_dir);
    Cleanup ();
    exit (1);
  }

  csGfxProgressMeter* meter = new csGfxProgressMeter (320);
  engine->Prepare (meter);
  delete meter;

  txtmgr->SetPalette ();
  VFS->DecRef ();
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

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.engine.3d:Engine");
  System->RequestPlugin ("crystalspace.level.loader:LevelLoader");

  // Initialize the main system. This will load all needed plug-ins.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Printf (CS_MSG_FATAL_ERROR, "Error initializing system!\n");
    Cleanup ();
    exit (1);
  }

  // Cleanup.
  Cleanup ();

  return 0;
}
