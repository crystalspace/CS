/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "apps/isotest/isotest.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/iso.h"
#include "isomater.h"
#include "imap/parser.h"
#include "igraphic/loader.h"


IsoTest::IsoTest ()
{
  engine = NULL;
  view = NULL;
  world = NULL;
  font = NULL;
  light = NULL;
}

IsoTest::~IsoTest ()
{
  if(view) view->DecRef();
  if(world) world->DecRef();
  if(engine) engine->DecRef();
  if(font) font->DecRef();
  if(light) light->DecRef();
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}


/// helper texture loader
static iMaterialHandle* LoadTexture(iSystem *sys, iTextureManager *txtmgr, 
  iVFS *VFS, char *name)
{
  iImageLoader *imgloader = QUERY_PLUGIN(sys, iImageLoader);
  iDataBuffer *buf = VFS->ReadFile (name);
  if(!buf) printf("Could not read file %s\n", name);
  iImage *image = imgloader->Load(buf->GetUint8 (), buf->GetSize (),
    txtmgr->GetTextureFormat ());
  if(!image) printf("Could not load image %s\n", name);
  iTextureHandle *handle = txtmgr->RegisterTexture(image, CS_TEXTURE_2D |
    CS_TEXTURE_3D);
  if(!handle) printf("Could not register texture %s\n", name);
  csIsoMaterial *material = new csIsoMaterial(handle);
  iMaterialHandle *math = txtmgr->RegisterMaterial(material);

  buf->DecRef();
  imgloader->DecRef();
  return math;
}


/// helper grid change callback for the player sprite - to move the light
static void PlayerGridChange(iIsoSprite *sprite, void *mydata)
{
  IsoTest* app = (IsoTest*)mydata;
  if(app->GetLight())
    app->GetLight()->SetGrid(sprite->GetGrid());
}


bool IsoTest::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Find the pointer to engine plugin
  engine = QUERY_PLUGIN (this, iIsoEngine);
  if (!engine)
  {
    Printf (MSG_FATAL_ERROR, "No IsoEngine plugin!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("IsoTest Crystal Space Application"))
  {
    Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    exit (1);
  }

  iFontServer *fsvr = QUERY_PLUGIN(this, iFontServer);
  font = fsvr->LoadFont(CSFONT_LARGE);
  fsvr->DecRef();

  // Setup the texture manager
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Initialize the texture manager
  txtmgr->ResetPalette ();
  
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
    "IsoTest Crystal Space Application version 0.1.\n");

  // Create our world.
  Printf (MSG_INITIALIZATION, "Creating world!...\n");

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  // create our world to play in, and a view on it.
  world = engine->CreateWorld();
  view = engine->CreateView(world);

  iMaterialHandle *math1 = LoadTexture(this, txtmgr, VFS, 
    "/lib/std/stone4.gif");
  iMaterialHandle *math2 = LoadTexture(this, txtmgr, VFS, 
    "/lib/std/mystone2.gif");
  iMaterialHandle *snow = LoadTexture(this, txtmgr, VFS, 
    "/lib/std/snow.jpg");

  // create a rectangular grid in the world
  // the grid is 10 units wide (from 0..10 in the +z direction)
  // the grid is 20 units tall (from 0..20 in the +x direction)
  iIsoGrid *grid = world->CreateGrid(10, 20);
  grid->SetSpace(0, 0, -1.0, +10.0);
  int multx = 2, multy = 2;
  grid->SetGroundMult(multx, multy);
  iIsoSprite *sprite = NULL;
  int x,y, mx,my;
  for(y=0; y<20; y++)
    for(x=0; x<10; x++)
    {
      // put tiles on the floor
      sprite = engine->CreateFloorSprite(csVector3(y,0,x), 1.0, 1.0);
      if((x+y)&1)
        sprite->SetMaterialHandle(math1);
      else sprite->SetMaterialHandle(math2);
      world->AddSprite(sprite);
      for(my=0; my<multy; my++)
        for(mx=0; mx<multx; mx++)
          grid->SetGroundValue(x, y, mx, my, 0.0);
    }

  // add the player sprite to the world
  csVector3 startpos(10,0,5);
  player = engine->CreateFrontSprite(startpos, 1.0, 4.0);
  player->SetMaterialHandle(snow);
  world->AddSprite(player);
  player->SetGridChangeCallback(PlayerGridChange, (void*)this);

  // add a light to the scene.
  iIsoLight *scenelight = engine->CreateLight();
  scenelight->SetPosition(csVector3(3,3,3));
  scenelight->SetGrid(grid);
  scenelight->SetRadius(4.0);
  scenelight->SetColor(csColor(0.0, 0.4, 1.0));

  // add a light for the player
  light = engine->CreateLight();
  light->SetPosition(startpos+csVector3(0,1,0));
  light->SetGrid(grid);
  light->SetRadius(5.0);
  light->SetColor(csColor(1.0, 0.4, 0.2));
  for(my=0; my<multy; my++)
    for(mx=0; mx<multx; mx++)
      grid->SetGroundValue(5, 15, mx, my, 0.8);

  bool res = grid->GroundHitBeam(csVector3(10,1,5), csVector3(15,-2,8));
  printf("Hitbeam gave %d\n", (int)res);
  res = grid->GroundHitBeam(csVector3(10,1,5), csVector3(20,0,10));
  printf("Hitbeam gave %d\n", (int)res);

  txtmgr->PrepareTextures ();
  txtmgr->PrepareMaterials ();
  txtmgr->SetPalette ();

  // scroll view to show player position at center of screen
  view->SetScroll(startpos, csVector2(G3D->GetWidth()/2, G3D->GetHeight()/2));

  return true;
}

void IsoTest::NextFrame ()
{
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  SysSystemDriver::NextFrame ();
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

  // keep track of fps.
  static bool fpsstarted = false;
  static int numframes = 0;
  static int numtime = 0;
  static float fps = 0.0;
  if(!fpsstarted && (elapsed_time > 0))
  {fpsstarted = true; fps = 1. / (float(elapsed_time)*.001);}
  numtime += elapsed_time;
  numframes ++; 
  if(numtime >= 1000)
  {
     fps = float(numframes) / (float(numtime)*.001);
     numtime = 0;
     numframes = 0;
  }

  // move the player and scroll the view to keep the player centered
  // according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.10 * 20);
  csVector3 playermotion(0,0,0);
  if (GetKeyState (CSKEY_RIGHT)) playermotion += csVector3(0,0,speed);
  if (GetKeyState (CSKEY_LEFT)) playermotion += csVector3(0,0,-speed);
  if (GetKeyState (CSKEY_PGUP)) playermotion += csVector3(0,speed,0);
  if (GetKeyState (CSKEY_PGDN)) playermotion += csVector3(0,-speed,0);
  if (GetKeyState (CSKEY_UP)) playermotion += csVector3(speed,0,0);
  if (GetKeyState (CSKEY_DOWN)) playermotion += csVector3(-speed,0,0);
  if(!playermotion.IsZero())
  {
    player->MovePosition(playermotion);
    view->MoveScroll(playermotion); 
    light->SetPosition(player->GetPosition()+csVector3(0,1,0));
  }

  // Tell 3D driver we're going to display 3D things.
  if (!G3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS
    | CSDRAW_CLEARSCREEN ))
    return;

  view->Draw ();
  
  // Start drawing 2D graphics.
  if (!G3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  char buf[255];
  sprintf(buf, "FPS: %g    loc(%g,%g,%g)", fps, player->GetPosition().x,
    player->GetPosition().y, player->GetPosition().z);
  G2D->Write(font, 10, G2D->GetHeight() - 20, txtmgr->FindRGB(255,255,255),
    -1, buf);
  
  // Drawing code ends here.
  G3D->FinishDraw ();

  // Print the final output.
  G3D->Print (NULL);
}

bool IsoTest::HandleEvent (iEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    Shutdown = true;
    return true;
  }
  if ((Event.Type == csevKeyDown) && (Event.Key.Code == '\t'))
  {
    static float settings[][5] = {
    // scale should be *displayheight.
    //xscale, yscale, zscale, zskew, xskew
    {1./16., 1./16., 1./16., 1.0, 1.0},
    {1./16., 1./16., 1./16., 0.6, 0.6},
    {1./16., 1./16., 1./16., 0.5, 0.5},
    {1./40., 1./40., 1./40., 0.5, 0.5},
    {1./8., 1./8., 1./8., 0.5, 0.5},
    };
    static int nrsettings = 5;
    static int setting = 0;
    setting = (setting+1)%nrsettings;
    float h = engine->GetG3D()->GetHeight();
    view->SetAxes( h*settings[setting][0], h*settings[setting][1], 
      h*settings[setting][2], settings[setting][3], settings[setting][4]);
    view->SetScroll(player->GetPosition(), 
      csVector2(G3D->GetWidth()/2, G3D->GetHeight()/2));
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
  System = new IsoTest ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.image.loader:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.console.output.standard:Console.Output");
  System->RequestPlugin ("crystalspace.engine.iso:Engine.Iso");

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
