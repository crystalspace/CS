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
#include "cssys/system.h"
#include "apps/perftest/perftest.h"
#include "apps/perftest/ptests3d.h"
#include "apps/perftest/ptests2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"
#include "igraphic/imageio.h"
#include "igraphic/image.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

PerfTest::PerfTest ()
{
  draw_3d = true;
  draw_2d = true;
  myG3D = NULL;
  myVFS = NULL;
  ImageLoader = NULL;
}

PerfTest::~PerfTest ()
{
  DEC_REF (myG3D);
  DEC_REF (myVFS);
  if (ImageLoader) ImageLoader->DecRef();
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}

iMaterialHandle* PerfTest::LoadMaterial (char* file)
{
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  iImage* image;
  iDataBuffer *buf = myVFS->ReadFile (file);
  if (!buf || !buf->GetSize ())
  {
    Printf (MSG_FATAL_ERROR, "Error loading texture '%s'!\n", file);
    exit (-1);
  }
  image = ImageLoader->Load (buf->GetUint8 (), buf->GetSize (),
  	txtmgr->GetTextureFormat ());
  buf->DecRef ();
  if (!image) exit (-1);
  iTextureHandle* texture = txtmgr->RegisterTexture (image, CS_TEXTURE_3D);
  if (!texture) exit (-1);
  image->DecRef ();
  iMaterialHandle* mat = txtmgr->RegisterMaterial (texture);
  return mat;
}

bool PerfTest::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Crystal Space Graphics Performance Tester"))
  {
    Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    exit (1);
  }

  ImageLoader = QUERY_PLUGIN_ID(this, CS_FUNCID_IMGLOADER, iImageIO);
  if (!ImageLoader) {
    Printf (MSG_FATAL_ERROR, "No image loader plugin!\n");
    return false;
  }

  myG3D = QUERY_PLUGIN_ID(this, CS_FUNCID_VIDEO, iGraphics3D);
  if (!myG3D) {
    Printf (MSG_FATAL_ERROR, "No iGraphics3D loader plugin!\n");
    return false;
  }

  myVFS = QUERY_PLUGIN_ID(this, CS_FUNCID_VFS, iVFS);
  if (!myVFS) {
    Printf (MSG_FATAL_ERROR, "No iVFS loader plugin!\n");
    return false;
  }

  // Setup the texture manager
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Initialize the texture manager
  txtmgr->ResetPalette ();

  // Initialize textures.
  materials[0] = LoadMaterial ("/lib/std/stone4.gif");
  materials[1] = LoadMaterial ("/lib/std/mystone2.gif");
  materials[2] = LoadMaterial ("/lib/std/andrew_marble4.gif");
  materials[3] = LoadMaterial ("/lib/std/andrew_wood.gif");
  txtmgr->PrepareTextures ();

  // Allocate a uniformly distributed in R,G,B space palette for console
  // The console will crash on some platforms if this isn't initialize properly
  int r,g,b;
  for (r = 0; r < 8; r++)
    for (g = 0; g < 8; g++)
      for (b = 0; b < 4; b++)
	txtmgr->ReserveColor (r * 32, g * 32, b * 64);
  txtmgr->SetPalette ();

  // Some commercials...
  Printf (MSG_INITIALIZATION,
    "Crystal Space 3D Performance Tester 0.1.\n");

  txtmgr->SetPalette ();

  const char *val;
  if ((val = GetOptionCL ("2d")))
  {
    current_tester = new StringTester ();
    draw_3d = false;
  }
  else
    current_tester = new SinglePolygonTester ();

  if ((val = GetOptionCL ("3d")))
    draw_2d = false;

  needs_setup = true;

  return true;
}

static cs_time last_time;

void PerfTest::NextFrame ()
{
  SysSystemDriver::NextFrame ();
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

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
    if (current_time-last_time >= 10000)
    {
      Printf (MSG_INITIALIZATION, "%f FPS\n",
      	current_tester->GetCount ()/10.);
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
	  Shutdown = true;
      }
    }
  }

  // Start drawing 2D graphics.
  if (needs_setup)
  {
    if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    iConsoleOutput *Console = QUERY_PLUGIN_ID (this, CS_FUNCID_CONSOLE, iConsoleOutput);
    if (Console)
    {
      Console->Clear ();
      Console->DecRef ();
    }
    last_time = current_time;
    char desc[255];
    current_tester->Description (desc);
    Printf (MSG_INITIALIZATION, desc);
    needs_setup = false;
  }

  // Drawing code ends here.
  myG3D->FinishDraw ();
  // Print the final output.
  myG3D->Print (NULL);
}

bool PerfTest::HandleEvent (iEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    Shutdown = true;
    return true;
  }
  else if ((Event.Type == csevKeyDown) && (Event.Key.Code == ' '))
  {
    last_time -= 10000;
    return true;
  }

  return false;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new PerfTest ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.console.output.standard:Console.Output");

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    cleanup ();
    exit (1);
  }

  // Main loop.
  System->Loop ();

  // Cleanup.
  cleanup ();

  return 0;
}
