/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#define SYSDEF_ACCESS
#include "sysdef.h"
#include "version.h"
#include "cssys/common/system.h"
#include "support/console.h"
#include "support/command.h"
#include "walktest/walktest.h"
#include "walktest/infmaze.h"
#include "walktest/scon.h"
#include "csparser/csloader.h"
#include "csgeom/csrect.h"
#include "csgeom/frustrum.h"
#include "csengine/dumper.h"
#include "csengine/csview.h"
#include "csengine/stats.h"
#include "csengine/light/light.h"
#include "csengine/light/dynlight.h"
#include "csengine/texture.h"
#include "csengine/objects/thing.h"
#include "csengine/wirefrm.h"
#include "csengine/library.h"
#include "csengine/polygon/polytext.h"
#include "csengine/scripts/csscript.h"
#include "csengine/scripts/intscri.h"
#include "csengine/colldet/being.h"
#include "csengine/2d/csspr2d.h"
#include "csutil/sparse3d.h"
#include "csutil/inifile.h"
#include "csutil/impexp/impexp.h"
#include "csobject/nameobj.h"
#include "csobject/dataobj.h"
#include "csgfxldr/csimage.h"
#include "cssndldr/common/sndbuf.h"
#include "csparser/sndbufo.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "isndrdr.h"

#if defined(OS_DOS) || defined(OS_WIN32) || defined (OS_OS2)
#  include <io.h>
#elif defined(OS_UNIX)
#  include <unistd.h>
#endif

#include "debug/fpu80x86.h"	// for debugging numerical instabilities

WalkTest *Sys;
converter *ImportExport;

#define Gfx3D System->piG3D
#define Gfx2D System->piG2D

//-----------------------------------------------------------------------------

void DrawZbuffer ()
{
  for (int y = 0; y < FRAME_HEIGHT; y++)
  {
    int gi_pixelbytes;
    System->piGI->GetPixelBytes (gi_pixelbytes);

    if (gi_pixelbytes == 4)
    {
      //@@@
    }
    if (gi_pixelbytes == 2)
    {
      UShort *dest;
      Gfx2D->GetPixelAt(0, y, (unsigned char**)&dest);

      ULong *zbuf;
      Gfx3D->GetZBufPoint (0, y, &zbuf);

      for (int x = 0; x < FRAME_WIDTH; x++)
        *dest++ = (unsigned short)(*zbuf++ >> 13);
    }
    else
    {
      unsigned char *dest;
      Gfx2D->GetPixelAt(0, y, &dest);

      ULong *zbuf;
      Gfx3D->GetZBufPoint(0, y, &zbuf);

      for (int x = 0; x < FRAME_WIDTH; x++)
        *dest++ = (unsigned char)(*zbuf++ >> 16);
    }
  }
}

int collcount = 0;

void WalkTest::DrawFrame (long elapsed_time, long current_time)
{
  (void)elapsed_time; (void)current_time;

  //not used since we need WHITE background not black
  int drawflags = 0; /* do_clear ? CSDRAW_CLEARSCREEN : 0; */
  if (do_clear || Sys->world->map_mode == MAP_ON)
  {
    if (Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS) != S_OK)
      return;
    Gfx2D->Clear (Sys->world->map_mode == MAP_ON ? 0 : 255);
  }

  if (!System->Console->IsActive ()
   || ((SimpleConsole*)(System->Console))->IsTransparent ())
  {
    // Tell Gfx3D we're going to display 3D things
    if (Gfx3D->BeginDraw (drawflags | CSDRAW_3DGRAPHICS) != S_OK)
      return;
    if (Sys->world->map_mode != MAP_ON) view->Draw ();
    // no need to clear screen anymore
    drawflags = 0;

    Sys->world->AdvanceSpriteFrames (current_time);

    csDynLight* dyn = Sys->world->GetFirstDynLight ();
    while (dyn)
    {
      extern void HandleDynLight (csDynLight*);
      csDynLight* dn = dyn->GetNext ();
      if (dyn->GetObj(csDataObject::Type())) HandleDynLight (dyn);
      dyn = dn;
    }
    int i;
    for (i = 0 ; i < Sys->world->sprites.Length () ; i++)
    {
      extern void HandleSprite (csSprite3D*);
      csSprite3D* spr = (csSprite3D*)Sys->world->sprites[i];
      HandleSprite (spr);
    }
  }

  // Start drawing 2D graphics
  if (Gfx3D->BeginDraw (drawflags | CSDRAW_2DGRAPHICS) != S_OK)
    return;
  if (do_show_z) DrawZbuffer ();
  if (Sys->world->map_mode)
  {
    Sys->world->wf->GetWireframe ()->Draw (Gfx3D, Sys->world->wf->GetCamera ());
  }
  SimpleConsole* scon = (SimpleConsole*)System->Console;
  scon->Print (NULL);

  if (!System->Console->IsActive ())
  {
    if (do_fps)
    {
      GfxWrite(11, FRAME_HEIGHT-11, 0, -1, "FPS=%f", timeFPS);
      GfxWrite(10, FRAME_HEIGHT-10, scon->get_fg (), -1, "FPS=%f", timeFPS);
    } /* endif */
    if (do_stats)
    {
      char buffer[30];
      sprintf (buffer, "pc=%d pd=%d po=%d", Stats::polygons_considered,
        Stats::polygons_drawn, Stats::portals_drawn);
      GfxWrite(FRAME_WIDTH-24*8-1, FRAME_HEIGHT-11, 0, -1, "%s", buffer);
      GfxWrite(FRAME_WIDTH-24*8, FRAME_HEIGHT-10, scon->get_fg (), -1, "%s", buffer);
    }
    else if (do_show_coord)
    {
      char buffer[100];
      sprintf (buffer, "%2.2f,%2.2f,%2.2f: %s",
        view->GetCamera ()->GetW2CTranslation ().x, view->GetCamera ()->GetW2CTranslation ().y,
        view->GetCamera ()->GetW2CTranslation ().z, csNameObject::GetName(*(view->GetCamera ()->GetSector())));
      Gfx2D->Write(FRAME_WIDTH-24*8-1, FRAME_HEIGHT-11, 0, -1, buffer);
      Gfx2D->Write(FRAME_WIDTH-24*8, FRAME_HEIGHT-10, scon->get_fg (), -1, buffer);
    }
    else if (do_cd)
    {
      char buffer[200], names[30];
      sprintf (buffer,"CD %d cam pos %2.2f,%2.2f,%2.2f %c %c",
         collcount,
         view->GetCamera ()->GetW2CTranslation ().x, view->GetCamera ()->GetW2CTranslation ().y,
         view->GetCamera ()->GetW2CTranslation ().z,
         csBeing::player->falling?'F':' ', csBeing::player->climbing?'C':' ');
      for (int i = 0; i < collcount; i++)
      {
        csCollider *id1, *id2;
        if (csCollider::Report(&id1, &id2))
	{
          sprintf(names,"[%s,%s] ",id1->GetName (),id2->GetName ());
          strcat(buffer,names);
	}
      }
      Gfx2D->Write(8-1, FRAME_HEIGHT-21, 0, -1, buffer);
      Gfx2D->Write(8, FRAME_HEIGHT-20, scon->get_fg (), -1, buffer);
    } /* endif */

    if (cslogo)
    {
      int w = cslogo->Width()  * FRAME_WIDTH  / 640;
      int h = cslogo->Height() * FRAME_HEIGHT / 480;
      int x = FRAME_WIDTH - 2 - w*152/256;
      cslogo->Draw(Gfx2D, x,2,w,h);
    }
  } /* endif */

  // Drawing code ends here
  Gfx3D->FinishDraw ();
  // Print the output.
  Gfx3D->Print (NULL);
}

int cnt = 1;
time_t time0 = (time_t)-1;

void WalkTest::PrepareFrame (long elapsed_time, long current_time)
{
  (void)elapsed_time; (void)current_time;

  CLights::LightIdle (); // SJI
  if (do_cd && csBeing::init)
  {
    // TODO ALEX: In future this should depend on whether the whole world
    // or 'active' sectors need to be set up as monsters hunt for player
    // outside of current sector, but this will do for now.

    // Test camera collision.
    // Load camera location into player.
    csBeing::player->sector = view->GetCamera ()->GetSector ();
    csBeing::player->transform = view->GetCamera ();
    collcount = csBeing::player->CollisionDetect ();
    // Load player transformation back into camera.
    view->GetCamera ()->SetW2C (csBeing::player->transform->GetO2T ());
    view->GetCamera ()->SetPosition (csBeing::player->transform->GetO2TTranslation ());
    view->GetCamera ()->SetSector (csBeing::player->sector);

  }
  if (cnt <= 0)
  {
    time_t time1 = SysGetTime ();
    if (time0 != (time_t)-1)
    {
      if (time1 != time0)
        timeFPS=10000.0f/(float)(time1-time0);
    }
    cnt = 10;
    time0 = SysGetTime ();
  }
  cnt--;

  layer->step_run ();
}

void perf_test ()
{
  Sys->busy_perf_test = true;
  time_t t1, t2, t;
  Sys->Printf (MSG_CONSOLE, "Performance test busy...\n");
  t = t1 = SysGetTime ();
  int i;
  for (i = 0 ; i < 100 ; i++)
  {
    Sys->layer->step_run ();
    Sys->DrawFrame (SysGetTime ()-t, SysGetTime ());
    t = SysGetTime ();
  }
  t2 = SysGetTime ();
  Sys->Printf (MSG_CONSOLE, "%f secs to render 100 frames: %f fps\n",
        (float)(t2-t1)/1000., 100000./(float)(t2-t1));
  Sys->Printf (MSG_DEBUG_0, "%f secs to render 100 frames: %f fps\n",
        (float)(t2-t1)/1000., 100000./(float)(t2-t1));
  cnt = 1;
  time0 = (time_t)-1;
  Sys->busy_perf_test = false;
}

void CaptureScreen (void)
{
#if !defined(OS_MACOS)  // SJI - NON mac stuff - the mac has its own way of doing screen captures
  int i = 0;
  char name[255];
  UByte pall[768];
  UByte *pal = &pall[0];

  do
  {
    sprintf (name, "cryst%02d.pcx", i++);
  } while (i < 100 && (access (name, 0) == 0));

  if (i >= 100) return;

  RGBpaletteEntry* pPalette;
  System->piGI->GetPalette (&pPalette);

  if (pPalette)
  {
	  for (i=0; i<256; i++)
	  {
		  *pal++ = pPalette[i].red;
		  *pal++ = pPalette[i].green;
		  *pal++ = pPalette[i].blue;
	  }
  }

  extern void WritePCX (char *name, unsigned char *data, UByte *pal,
			int width,int height);
  Gfx3D->BeginDraw(CSDRAW_2DGRAPHICS);

  unsigned char* pFirstPixel;
  Gfx2D->GetPixelAt(0,0, &pFirstPixel);

  WritePCX (name, pFirstPixel, pall, FRAME_WIDTH, FRAME_HEIGHT);
  Gfx3D->FinishDraw();
  Sys->Printf (MSG_CONSOLE, "Screenshot: %s", name);

#endif // !OS_MACOS
}

/*---------------------------------------------
 * Our main event loop.
 *---------------------------------------------*/

/*
 * Do a large debug dump just before the program
 * exits. This function can be installed as a last signal
 * handler if the program crashes (for systems that support
 * this).
 */
void debug_dump ()
{
  Sys->view->GetCamera ()->SaveFile ("coord.bug");
  Sys->Printf (MSG_DEBUG_0, "Camera saved in coord.bug\n");
  Dumper::dump (Sys->view->GetCamera ());
  Sys->Printf (MSG_DEBUG_0, "Camera dumped in debug.txt\n");
  Dumper::dump (Sys->world);
  Sys->Printf (MSG_DEBUG_0, "World dumped in debug.txt\n");
}

/*
 * A sample script which just prints a message on screen.
 */
bool do_message_script (IntRunScript* sc, char* data)
{
  sc->get_layer ()->message (data);
  return true;
}

//---------------------------------------------------------------------------


void cleanup ()
{
  pprintf ("Cleaning up...\n");
  free_keymap ();
  csBeing::EndWorld ();
  CHK (delete Sys); Sys = NULL;
  pprintf_close();
}

/*---------------------------------------------------------------------*
 * Demo stuff
 *---------------------------------------------------------------------*/

struct DemoInfo
{
  csWorld* world;
};

DemoInfo* demo_info = NULL;

/*
 * The routine to update the screen in demo mode.
 */
void WalkTest::DemoWrite (const char* buf)
{
  if (Console)
  {
    if (Gfx2D)
    {
      VERIFY_SUCCESS (Gfx2D->BeginDraw ());
      csRect area;
      bool dblbuff;
      Gfx2D->GetDoubleBufferState (dblbuff);
      Gfx2D->Clear (0);
      Console->PutText ("%s", buf);
      Console->Print (&area);
      if (dblbuff)
        area.Union (0, 0, FRAME_WIDTH - 1, FRAME_HEIGHT - 1);
      Gfx2D->FinishDraw ();
      Gfx2D->Print (&area);
    }
  }
}

/*
 * Start the demo in the already open screen.
 */
void start_demo ()
{
  CHK (demo_info = new DemoInfo);
  CHK (Sys->world = demo_info->world = new csWorld ());

  ITextureManager* txtmgr;
  Gfx3D->GetTextureManager (&txtmgr);
//Gfx2D->DoubleBuffer (false);
  demo_info->world->Initialize (GetISystemFromSystem (System), System->piGI, config);
  txtmgr->Prepare ();
  ((SimpleConsole *)System->Console)->SetupColors (txtmgr);
  ((SimpleConsole *)System->Console)->SetMaxLines (1000);       // Some arbitrary high value.
  ((SimpleConsole *)System->Console)->SetTransparent (0);
  txtmgr->AllocPalette ();

  if (Gfx2D->BeginDraw() == S_OK)
  {
    Gfx2D->Print (NULL);
    Gfx2D->FinishDraw ();
  }
  System->DemoReady = true;
}

/*
 * Stop the demo.
 */
void stop_demo ()
{
  if (demo_info)
  {
    CHK (delete demo_info->world);
    CHK (delete demo_info);
  }
}


#if 0

void TestFrustrum ()
{
  csFrustrum* f1 = new csFrustrum (csVector3 (0,10,0));
  csFrustrum* f2 = new csFrustrum (*f1);
  f2->AddVertex (csVector3 (-3,15,3));
  f2->AddVertex (csVector3 (3,15,3));
  f2->AddVertex (csVector3 (0,15,-3));
  csFrustrum* f3 = new csFrustrum (*f1);
  f3->AddVertex (csVector3 (1,15,4));
  f3->AddVertex (csVector3 (4,15,4));
  f3->AddVertex (csVector3 (4,15,-2));
  f3->AddVertex (csVector3 (1,15,-2));
  Dumper::dump (f1, "f1");
  Dumper::dump (f2, "f2");
  Dumper::dump (f3, "f3");
  csFrustrum* f4 = f1->Intersect (*f2);
  csFrustrum* f5 = f2->Intersect (*f1);
  csFrustrum* f6 = f2->Intersect (*f3);
  csFrustrum* f7 = new csFrustrum (*f2);
  f7->ClipToPlane (csVector3 (1,15,4), csVector3 (1,15,-2));
  Dumper::dump (f4, "f4");
  Dumper::dump (f5, "f5");
  Dumper::dump (f6, "f6");
  Dumper::dump (f7, "f7");
  csFrustrum* f8 = new csFrustrum (*f1);
  f8->AddVertex (csVector3 (27,15,3));
  f8->AddVertex (csVector3 (33,15,3));
  f8->AddVertex (csVector3 (30,15,-3));
  csFrustrum* f9 = f8->Intersect (*f3);
  Dumper::dump (f8, "f8");
  Dumper::dump (f9, "f9");
}

#endif

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class which is the driver for WalkTest.
  CHK (Sys = new WalkTest ());

  // Open our configuration file and read the configuration values
  // specific for WalkTest.
  //@@@Config should belong to WalkTest.
  CHK (config = new csIniFile ("cryst.cfg"));

  // create the converter class for testing
  //@@@ I added this conditional because this only works in VC right now
  // I hope to fix it in the UNIX makefile system shortly and remove this
  // -Michael
//#ifdef OS_WIN32
  CHK(ImportExport = new converter());

  // process import/export files from config and print log for testing

  ImportExport->ProcessConfig (config);

  // free memory - delete this if you want to use the data in the buffer

  CHK (delete ImportExport);
//#endif /* OS_WIN32 */
  // end converter test

  Sys->do_fps = config->GetYesNo ("WalkTest", "FPS", true);
  Sys->do_stats = config->GetYesNo ("WalkTest", "STATS", false);
  Sys->do_cd = config->GetYesNo ("WalkTest", "COLLDET", true);
  strcpy (WalkTest::world_file, config->GetStr ("World", "WORLDFILE", "world"));

  // Create our world. The world is the representation of
  // the 3D engine.
  CHK (csWorld* world = new csWorld ());

  // Initialize the main system. This will load all needed
  // COM drivers (3D, 2D, network, sound, ...) and initialize them.
  if (!Sys->Initialize (argc, argv, world->GetEngineConfigCOM ()))
  {
    Sys->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    cleanup ();
    fatal_exit (0, false);
  }

#ifdef DEBUG
  // enable all kinds of useful exceptions on a x86
  // note that we can't do it above since at least on OS/2 each dynamic
  // library on loading/initialization resets the control word to default
  _control87 (0x32, 0x3f);
#else
  // this will disable exceptions on DJGPP (for the "industrial" version)
  _control87 (0x3f, 0x3f);
#endif

  // Open the main system. This will open all the previously loaded
  // COM drivers.
  if (!Sys->Open ("Crystal Space"))
  {
    Sys->Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    fatal_exit (0, false);
  }

  // For debugging purposes.
  Gfx3D->SetRenderState (G3DRENDERSTATE_DEBUGENABLE,
    config->GetYesNo ("WalkTest", "DEBUG", false));

  // Create console object for text and commands.
  CHK (System->Console = new SimpleConsole ());

  // Start the 'demo'. This is currently nothing more than
  // the display of all startup messages on the console.
  start_demo ();

  // Some commercials...
  Sys->Printf (MSG_INITIALIZATION, "Crystal Space version %s (%s).\n", VERSION, RELEASE_DATE);
  Sys->Printf (MSG_INITIALIZATION, "Created by Jorrit Tyberghein and others...\n\n");
  ITextureManager* txtmgr;
  Gfx3D->GetTextureManager (&txtmgr);
  txtmgr->SetVerbose (true);

  // Initialize our world now that the system is ready.
  Sys->world = world;
  world->Initialize (GetISystemFromSystem (System), System->piGI, config);

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  CHK (Sys->view = new csView (world, Gfx3D));

  // Initialize the command processor with the world and camera.
  Command::Initialize (world, Sys->view->GetCamera (), Gfx3D, System->Console, GetISystemFromSystem (System));

  // Create the language layer needed for scripting.
  // Also register a small C script so that it can be used
  // by levels. large.zip uses this script.
  CHK (Sys->layer = new LanguageLayer (world, Sys->view->GetCamera ()));
  int_script_reg.reg ("message", &do_message_script);

  // Now we have two choices. Either we create an infinite
  // maze (random). This happens when the '-infinite' commandline
  // option is given. Otherwise we load the given world.
  csSector* room;

  if (Sys->do_infinite)
  {
    // The infinite maze.

    Sys->Printf (MSG_INITIALIZATION, "Creating initial room!...\n");
    //@@@ Think
    csPolygon3D::do_force_recalc = true;

    // Disable collision detection for the infinite maze because
    // it does not work here yet.
    Sys->do_cd = false;

    // Load the standard library.
    CSLoader::LoadLibrary (world, "standard", "standard.zip");

    // Load two textures that are used in the maze.
    CSLoader::LoadTexture (world, "txt", "stone4.gif");
    CSLoader::LoadTexture (world, "txt2", "mystone2.gif");

    // After loading the textures but BEFORE using them the
    // texture manager needs to be prepared.
    txtmgr->Prepare ();

    // Create the initial (non-random) part of the maze.
    CHK (Sys->infinite_maze = new InfiniteMaze ());
    room = Sys->infinite_maze->create_six_room (world, 0, 0, 0)->sector;
    Sys->infinite_maze->create_six_room (world, 0, 0, 1);
    Sys->infinite_maze->create_six_room (world, 0, 0, 2);
    Sys->infinite_maze->create_six_room (world, 1, 0, 2);
    Sys->infinite_maze->create_six_room (world, 0, 1, 2);
    Sys->infinite_maze->create_six_room (world, 1, 1, 2);
    Sys->infinite_maze->create_six_room (world, 0, 0, 3);
    Sys->infinite_maze->create_six_room (world, 0, 0, 4);
    Sys->infinite_maze->create_six_room (world, -1, 0, 4);
    Sys->infinite_maze->create_six_room (world, -2, 0, 4);
    Sys->infinite_maze->create_six_room (world, 0, -1, 3);
    Sys->infinite_maze->create_six_room (world, 0, -2, 3);
    Sys->infinite_maze->create_six_room (world, 0, 1, 3);
    Sys->infinite_maze->create_six_room (world, 0, 2, 3);
    Sys->infinite_maze->connect_infinite (0, 0, 0, 0, 0, 1);
    Sys->infinite_maze->connect_infinite (0, 0, 1, 0, 0, 2);
    Sys->infinite_maze->connect_infinite (0, 0, 2, 0, 0, 3);
    Sys->infinite_maze->connect_infinite (0, 0, 2, 1, 0, 2);
    Sys->infinite_maze->connect_infinite (0, 0, 2, 0, 1, 2);
    Sys->infinite_maze->connect_infinite (1, 1, 2, 0, 1, 2);
    Sys->infinite_maze->connect_infinite (1, 1, 2, 1, 0, 2);
    Sys->infinite_maze->connect_infinite (0, 0, 3, 0, 0, 4);
    Sys->infinite_maze->connect_infinite (-1, 0, 4, 0, 0, 4);
    Sys->infinite_maze->connect_infinite (-2, 0, 4, -1, 0, 4);
    Sys->infinite_maze->connect_infinite (0, 0, 3, 0, -1, 3);
    Sys->infinite_maze->connect_infinite (0, -1, 3, 0, -2, 3);
    Sys->infinite_maze->connect_infinite (0, 0, 3, 0, 1, 3);
    Sys->infinite_maze->connect_infinite (0, 1, 3, 0, 2, 3);
    Sys->infinite_maze->create_loose_portal (-2, 0, 4, -2, 1, 4);

    // Prepare the world. This will calculate all lighting and
    // prepare the lightmaps for the 3D rasterizer.
    world->Prepare (Gfx3D);
  }
  else
  {
    // Load from a world file.

    Sys->Printf (MSG_INITIALIZATION, "Loading world '%s'...\n", WalkTest::world_file);

    // Load the world from the file.
    if (!CSLoader::LoadWorldFile (world, Sys->layer, WalkTest::world_file))
    {
      Sys->Printf (MSG_FATAL_ERROR, "Loading of world failed!\n");
      cleanup ();
      fatal_exit (0, false);
    }

    // Load the standard library.
    if (!CSLoader::LoadLibrary (world, "standard", "standard.zip"))
    {
      //Error message was already printed be CSLoader...
      cleanup ();
      fatal_exit (0, false);
    }

    //Find the Crystal Space logo and set the renderer Flag to for_2d, to allow 
    //the use in the 2D part.
    csTextureList *texlist = world->GetTextures ();
    ASSERT(texlist);
    csTextureHandle *texh = texlist->GetTextureMM ("cslogo.gif");
    if (texh)
    {
      texh->for_2d = true;
    }

    // Prepare the world. This will calculate all lighting and
    // prepare the lightmaps for the 3D rasterizer.
    world->Prepare (Gfx3D);

    //Create a 2D sprite for the Logo
    if (texh)
    {
      int w, h;
      ITextureHandle* phTex = texh->GetTextureHandle();
      phTex->GetBitmapDimensions(w,h);
      CHK (Sys->cslogo = new csSprite2D (texh, 0, 0, w, h));
    }

    // Look for the start sector in this world.
    char* strt = (char*)(world->start_sector ? world->start_sector : "room");
    room = (csSector*)world->sectors.FindByName (strt);
    if (!room)
    {
      Sys->Printf (MSG_FATAL_ERROR,
          "World file does not contain a room called '%s' which is used\nas a starting point!\n",
	  strt);
      cleanup ();
      fatal_exit (0, false);
    }
  }

  // Initialize collision detection system (even if disabled so that we can enable it later).
  csBeing::InitWorld (Sys->world, Sys->view->GetCamera ());

  Sys->Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  // Wait one second before starting.
  long t = Sys->Time ()+1000;
  while (Sys->Time () < t) ;

  // Reinit console object for 3D engine use.
  ((SimpleConsole *)System->Console)->SetupColors (txtmgr);
  ((SimpleConsole *)System->Console)->SetMaxLines ();
  ((SimpleConsole *)System->Console)->SetTransparent ();
  System->Console->Clear ();

  // Initialize our 3D view.
  Sys->view->SetSector (room);
  Sys->view->GetCamera ()->SetPosition (world->start_vec);
  Sys->view->SetRectangle (2, 2, FRAME_WIDTH - 4, FRAME_HEIGHT - 4);

  // Stop the demo.
  stop_demo ();

  // Allocate the palette as calculated by the texture manager.
  txtmgr->AllocPalette ();

  // Create a wireframe object which will be used for debugging.
  CHK (world->wf = new csWireFrameCam (txtmgr));

  // Load a few sounds.
#ifdef DO_SOUND
  csSoundBuffer* w = csSoundBufferObject::GetSound(*world, "tada.wav");
  if (w) Sys->piSound->PlayEphemeral (w);

  Sys->wMissile_boom = csSoundBufferObject::GetSound(*world, "boom.wav");
  Sys->wMissile_whoosh = csSoundBufferObject::GetSound(*world, "whoosh.wav");
#endif

  // Start the 'autoexec.cfg' script and fully execute it.
  Command::start_script ("autoexec.cfg");
  char cmd_buf[512];
  while (Command::get_script_line (cmd_buf, 511))
    Command::perform_line (cmd_buf);

  // If there is another script given on the commandline
  // start it but do not execute it yet. This will be done
  // frame by frame.
  if (Sys->auto_script)
    Command::start_script (Sys->auto_script);

  //TestFrustrum ();
  // The main loop.
  Sys->Loop ();

  cleanup ();

  return 1;
}
