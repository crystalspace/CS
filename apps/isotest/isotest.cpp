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


    isotest - the main test application for the isometric 3d engine
*/

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "qsqrt.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "apps/isotest/isotest.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/iso.h"
#include "imap/parser.h"
#include "igraphic/imageio.h"
#include "csgeom/box.h"
#include "imesh/object.h"
#include "imesh/genmesh.h"
#include "imesh/sprite2d.h"
#include "imesh/particle.h"
#include "imesh/partsys.h"
#include "imesh/fountain.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "genmaze.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "csutil/event.h"

CS_IMPLEMENT_APPLICATION

// The global system driver
IsoTest *System;

IsoTest::IsoTest ()
{
  lastclick.Set(0,0,0);
  walking = false;
}

IsoTest::~IsoTest ()
{
}

void IsoTest::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  csRef<iReporter> rep (CS_QUERY_REGISTRY (System->object_reg, iReporter));
  if (rep)
    rep->ReportV (severity, "crystalspace.application.isotest", msg, arg);
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


/// call during 2d phase
static void DebugZBufShow(iGraphics3D *g3d, iGraphics2D *g2d,
  iTextureManager* /*txtmgr*/)
{
  int w = g3d->GetWidth();
  int h = g3d->GetHeight();
  for(int y=0; y<h; y++)
    for(int x=0; x<w; x++)
    {
      float zbuf = g3d->GetZBuffValue(x,y);
      uint32 zint = (uint32)zbuf;
      g2d->DrawPixel(x, y, zint);
    }
}

struct PlayerGridChange : public iGridChangeCallback
{
  IsoTest* app;
  SCF_DECLARE_IBASE;
  PlayerGridChange () { SCF_CONSTRUCT_IBASE (0); }
  virtual ~PlayerGridChange() { SCF_DESTRUCT_IBASE(); }
  virtual void GridChange (iIsoSprite* spr);
};

SCF_IMPLEMENT_IBASE (PlayerGridChange)
  SCF_IMPLEMENTS_INTERFACE (iGridChangeCallback)
SCF_IMPLEMENT_IBASE_END

/// helper grid change callback for the player sprite - to move the light
void PlayerGridChange::GridChange (iIsoSprite *sprite)
{
  if(app->GetLight())
    app->GetLight()->SetGrid(sprite->GetGrid());
}

static bool IsoEventHandler (iEvent& ev)
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

bool IsoTest::Initialize (int argc, const char* const argv[],
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
	CS_REQUEST_PLUGIN("crystalspace.engine.iso", iIsoEngine),
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, IsoEventHandler))
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

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iIsoEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No IsoEngine plugin!");
    return false;
  }

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    return false;
  }

  myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics2D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    return false;
  }

  mouse = CS_QUERY_REGISTRY (object_reg, iMouseDriver);
  if (!mouse)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iMouseDriver!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("IsoTest Crystal Space Application");
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  csRef<iFontServer> fsvr (CS_QUERY_REGISTRY (object_reg, iFontServer));
  font = fsvr->LoadFont(CSFONT_LARGE);

  // Setup the texture manager
  iTextureManager* txtmgr = myG3D->GetTextureManager ();

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "IsoTest Crystal Space Application version 0.1.");

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");
  Report (CS_REPORTER_SEVERITY_NOTIFY, "------------------------------");

  // create our world to play in, and a view on it.
  world = csPtr<iIsoWorld> (engine->CreateWorld());
  view = csPtr<iIsoView> (engine->CreateView(world));

  iMaterialWrapper *math1 = engine->CreateMaterialWrapper(
    "/lib/std/stone4.gif", "floor1");
  iMaterialWrapper *math2 = engine->CreateMaterialWrapper(
    "/lib/std/mystone2.gif", "floor2");
  iMaterialWrapper *snow = engine->CreateMaterialWrapper(
    "/lib/std/snow.jpg", "player");
  iMaterialWrapper *halo = engine->CreateMaterialWrapper(
    "/lib/stdtex/flare_purp.jpg", "halo");


  csVector3 startpos(10,0,5);

  // create a rectangular grid in the world
  // the grid is 10 units wide (from 0..10 in the +z direction)
  // the grid is 20 units tall (from 0..20 in the +x direction)
  iIsoGrid *grid = world->CreateGrid(10, 20);
  grid->SetSpace(0, 0, -1.0, +10.0);
  int multx = 2, multy = 2;
  grid->SetGroundMult(multx, multy);
  iIsoSprite *sprite = 0;
  int x,y, mx,my;
  for(y=0; y<20; y++)
    for(x=0; x<10; x++)
    {
      // put tiles on the floor
      sprite = engine->CreateFloorSprite(csVector3(y,0,x), 1.0, 1.0);
      if((x+y)&1)
        sprite->SetMaterialWrapper(math1);
      else sprite->SetMaterialWrapper(math2);
      world->AddSprite(sprite);
      sprite->DecRef ();
      for(my=0; my<multy; my++)
        for(mx=0; mx<multx; mx++)
          grid->SetGroundValue(x, y, mx, my, 0.0);
    }


  // add the player sprite to the world
  player = csPtr<iIsoSprite> (engine->CreateFrontSprite(startpos, 1.3f, 2.7f));
  player->SetMaterialWrapper(snow);
  player->SetMixMode(CS_FX_ADD);
  world->AddSprite(player);
  PlayerGridChange* cb = new PlayerGridChange ();
  cb->app = this;
  player->SetGridChangeCallback(cb);
  cb->DecRef ();

  // add a light to the scene.
  csRef<iIsoLight> scenelight (csPtr<iIsoLight> (engine->CreateLight()));
  scenelight->SetPosition(csVector3(3,2,6));
  scenelight->SetGrid(grid);
  scenelight->SetAttenuation(CSISO_ATTN_INVERSE);
  scenelight->SetRadius(5.0);
  scenelight->SetColor(csColor(0.0f, 0.4f, 1.0f));

  // add a glowing halo to the light
  sprite = engine->CreateFrontSprite(scenelight->GetPosition() -
    csVector3(0,1.0,0), 2.0, 2.0);
  sprite->SetMaterialWrapper(halo);
  sprite->SetMixMode(CS_FX_ADD);
  world->AddSprite(sprite);
  sprite->DecRef ();

  // add a dynamic light for the player, above players head.
  light = csPtr<iIsoLight> (engine->CreateLight());
  light->Flags().Set(CSISO_LIGHT_DYNAMIC);
  light->SetPosition(startpos+csVector3(0,5,0));
  light->SetGrid(grid);
  light->SetRadius(5.0);
  light->SetColor(csColor(1.0f, 0.4f, 0.2f));

  // make a bump to cast shadows
  for(my=0; my<multy; my++)
    for(mx=0; mx<multx; mx++)
      grid->SetGroundValue(5, 15, mx, my, 4.0);
  sprite = engine->CreateFloorSprite(csVector3(15,4,5), 1.0, 1.0);
  sprite->SetMaterialWrapper(math1);
  world->AddSprite(sprite);
  sprite->DecRef ();
  for(my=0; my<4; my++)
  {
    sprite = engine->CreateXWallSprite(csVector3(15,my,5), 1.0, 1.0);
    sprite->SetMaterialWrapper(math1);
    world->AddSprite(sprite);
    sprite->DecRef ();
    sprite = engine->CreateZWallSprite(csVector3(16,my,5), 1.0, 1.0);
    sprite->SetMaterialWrapper(math1);
    world->AddSprite(sprite);
    sprite->DecRef ();
  }

  for(mx=0; mx<8; mx++)
  {
    sprite = engine->CreateFloorSprite(csVector3(5,4,2+mx), 1.0, 1.0);
    sprite->SetMaterialWrapper(math1);
    world->AddSprite(sprite);
    sprite->DecRef ();
  }
  for(my=0; my<4; my++)
  {
    sprite = engine->CreateXWallSprite(csVector3(5,my,2), 1.0, 1.0);
    sprite->SetMaterialWrapper(math1);
    world->AddSprite(sprite);
    sprite->DecRef ();
    sprite = engine->CreateXWallSprite(csVector3(5,my,7), 1.0, 1.0);
    sprite->SetMaterialWrapper(math2);
    world->AddSprite(sprite);
    sprite->DecRef ();
    for(mx=0; mx<3; mx++)
    {
      sprite = engine->CreateZWallSprite(csVector3(6,my,2+mx), 1.0, 1.0);
      sprite->SetMaterialWrapper(math1);
      world->AddSprite(sprite);
      sprite->DecRef ();
      sprite = engine->CreateZWallSprite(csVector3(6,my,7+mx), 1.0, 1.0);
      sprite->SetMaterialWrapper(math1);
      world->AddSprite(sprite);
      sprite->DecRef ();
    }
  }
  sprite = engine->CreateZWallSprite(csVector3(6,3,5), 1.0, 1.0);
  sprite->SetMaterialWrapper(math1);
  world->AddSprite(sprite);
  sprite->DecRef ();
  sprite = engine->CreateZWallSprite(csVector3(6,3,6), 1.0, 1.0);
  sprite->SetMaterialWrapper(math1);
  world->AddSprite(sprite);
  sprite->DecRef ();
  for(x=0; x<3; x++)
    for(my=0; my<multy; my++)
      for(mx=0; mx<multx; mx++)
      {
        grid->SetGroundValue(2+x, 5, mx, my, 4.0);
        grid->SetGroundValue(7+x, 5, mx, my, 4.0);
      }

  /// create a mesh object
  const char* classId = "crystalspace.mesh.object.genmesh";
  iMeshFactoryWrapper* mesh_wrap = engine->CreateMeshFactory (classId,
  	"meshFact");
  iMeshObjectFactory *mesh_fact = mesh_wrap->GetMeshObjectFactory ();

  csRef<iGeneralFactoryState> cubelook (
    SCF_QUERY_INTERFACE(mesh_fact, iGeneralFactoryState));
  cubelook->SetMaterialWrapper(math2);
  cubelook->GenerateBox (csBox3 (-.5,-.5,-.5,.5,.5,.5));
  cubelook->CalculateNormals ();

  csRef<iMeshObject> mesh_obj (mesh_fact->NewInstance());
  csRef<iGeneralMeshState> cubestate (
    SCF_QUERY_INTERFACE (mesh_obj, iGeneralMeshState));
  cubestate->SetMixMode (CS_FX_COPY);

  /// add a mesh object sprite
  iIsoMeshSprite *meshspr = engine->CreateMeshSprite();
  meshspr->SetMeshObject(mesh_obj);
  meshspr->SetPosition( csVector3(12 + 0.5, +0.5, 2+0.5) );
  world->AddSprite(meshspr);
  meshspr->DecRef ();

  const char* fo_classId = "crystalspace.mesh.object.fountain";
  mesh_wrap = engine->CreateMeshFactory(fo_classId, "fountainFact");
  mesh_fact = mesh_wrap->GetMeshObjectFactory ();

  mesh_obj = mesh_fact->NewInstance();
  csRef<iParticleState> pastate (SCF_QUERY_INTERFACE(mesh_obj, iParticleState));
  csRef<iFountainState> fostate (SCF_QUERY_INTERFACE(mesh_obj, iFountainState));
  pastate->SetMaterialWrapper(halo);
  pastate->SetMixMode(CS_FX_ADD);
  pastate->SetColor( csColor(0.125, 0.5, 1.0) );
  fostate->SetParticleCount(100);
  fostate->SetDropSize(0.1f, 0.1f);
  fostate->SetOrigin( csVector3(0,0,0) );
  fostate->SetLighting(false);
  fostate->SetAcceleration( csVector3(0, -0.3f, 0) );
  fostate->SetElevation(HALF_PI);
  fostate->SetAzimuth(1.0);
  fostate->SetOpening(0.14f);
  fostate->SetSpeed(1.0);
  fostate->SetFallTime(6.0);

  meshspr = engine->CreateMeshSprite();
  meshspr->SetMeshObject(mesh_obj);
  meshspr->SetZBufMode(CS_ZBUF_TEST);
  meshspr->SetPosition( csVector3(10, 0, 8) );
  world->AddSprite(meshspr);
  meshspr->DecRef ();

  // create second grid
  iIsoGrid *grid2 = world->CreateGrid(20, 10);
  grid2->SetSpace(10, 10, -1.0, +10.0);
  grid2->SetGroundMult(multx, multy);
  for(y=10; y<20; y++)
    for(x=10; x<30; x++)
    {
      // put tiles on the floor
      sprite = engine->CreateFloorSprite(csVector3(y,0,x), 1.0, 1.0);
      sprite->SetMaterialWrapper(math2);
      world->AddSprite(sprite);
      sprite->DecRef ();
      for(my=0; my<multy; my++)
        for(mx=0; mx<multx; mx++)
          grid2->SetGroundValue(x-10, y-10, mx, my, 0.0);
    }

  // add a light
  scenelight = csPtr<iIsoLight> (engine->CreateLight());
  scenelight->SetPosition(csVector3(13,2,26));
  scenelight->SetGrid(grid2);
  scenelight->SetAttenuation(CSISO_ATTN_INVERSE);
  scenelight->SetRadius(5.0);
  scenelight->SetColor(csColor(0.0f, 0.4f, 1.0f));
  // add the light to both grids to make it look right.
  scenelight = csPtr<iIsoLight> (engine->CreateLight());
  scenelight->SetPosition(csVector3(13,2,26));
  scenelight->SetGrid(grid);
  scenelight->SetAttenuation(CSISO_ATTN_INVERSE);
  scenelight->SetRadius(5.0);
  scenelight->SetColor(csColor(0.0f, 0.4f, 1.0f));


  /// add maze grid
  printf("Adding maze grid.\n");
  AddMazeGrid(world, 20, 20, math2, math1);
  printf("Adding maze grid done.\n");


  // prepare texture manager
  txtmgr->PrepareTextures ();
  txtmgr->PrepareMaterials ();

  // scroll view to show player position at center of screen
  view->SetScroll(
    startpos, csVector2(myG3D->GetWidth()/2, myG3D->GetHeight()/2));

  math1->DecRef ();
  math2->DecRef ();
  snow->DecRef ();
  halo->DecRef ();

  return true;
}


// static helper
static void AddWall(iIsoEngine *engine, iIsoWorld *world, iIsoGrid *grid,
  int x, int y, int offx, int offy, int multx, int multy, 
  float bot, float height,
  iMaterialWrapper *side, iMaterialWrapper *top)
{
  int my, mx;
  for(my=0; my<multy; my++)
    for(mx=0; mx<multx; mx++)
      grid->SetGroundValue(x, y, mx, my, height);

  iIsoSprite *sprite = 0;
  if(bot != height)
  {
    sprite = engine->CreateZWallSprite(csVector3(y+offy+1,bot,x+offy),
      1.0, height);
    sprite->SetMaterialWrapper(side);
    //sprite->SetMixMode( CS_FX_COPY | CS_FX_TILING );
    world->AddSprite(sprite);
    sprite->DecRef ();
    sprite = engine->CreateXWallSprite(csVector3(y+offy,bot,x+offy),
      1.0, height);
    sprite->SetMaterialWrapper(side);
    //sprite->SetMixMode( CS_FX_COPY | CS_FX_TILING );
    world->AddSprite(sprite);
    sprite->DecRef ();
  }
  sprite = engine->CreateFloorSprite(csVector3(y+offy,height,x+offx), 1, 1);
  sprite->SetMaterialWrapper(top);
  world->AddSprite(sprite);
  sprite->DecRef ();
}

void IsoTest::AddMazeGrid(iIsoWorld *world, float posx, float posy,
  iMaterialWrapper *floor, iMaterialWrapper *wall)
{
  int width = 10,height = 10;

  // create the maze
  csGenMaze *maze = new csGenMaze(width, height);
  //maze->SetStraightness(0.1);
  //maze->SetCyclicalness(0.1);
  int x,y;

  // Make entry & exit points
  maze->MakeAccess(0, 0);
  maze->MakeAccess(width-1,height-1);
  maze->GenerateMaze(0,0);

  // debug display
  // maze->PrintMaze();

  // create a grid to display the maze in.
  iIsoGrid *mazegrid = world->CreateGrid(maze->ActualWidth(), \
    maze->ActualHeight());

  mazegrid->SetSpace((int)posy, (int)posx, -1.0, +10.0);

  int multx = 1;
  int multy = 1;
  mazegrid->SetGroundMult(multx, multy);

  // add a light
  csRef<iIsoLight> scenelight (csPtr<iIsoLight> (engine->CreateLight()));
  scenelight->SetPosition(csVector3(posy+height+0.5,10,posx+width+0.5));
  scenelight->SetGrid(mazegrid);
  scenelight->SetAttenuation(CSISO_ATTN_INVERSE);
  scenelight->SetRadius(10.0 + width/2 + height/2);
  scenelight->SetColor(csColor(0.0f, 0.4f, 1.0f));

  // add walls

#define ADDWALLFULL(x, y) AddWall(engine, world, mazegrid, x, y, (int)posx, \
  (int)posy, multx, multy, 0.0, 2.0, wall, floor);
#define ADDWALLEMPTY(x, y) AddWall(engine, world, mazegrid, x, y, (int)posx, \
  (int)posy, multx, multy, 0.0, 0.0, wall, floor);

  for(y=0; y<maze->ActualHeight(); y++)
    for(x=0; x<maze->ActualWidth(); x++)
      if(maze->ActualSolid(x,y))
        ADDWALLFULL(x,y)
      else
        ADDWALLEMPTY(x,y)
      
}

void IsoTest::SetupFrame ()
{
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

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
  if (kbd->GetKeyState (CSKEY_RIGHT)) playermotion += csVector3(0,0,speed);
  if (kbd->GetKeyState (CSKEY_LEFT)) playermotion += csVector3(0,0,-speed);
  if (kbd->GetKeyState (CSKEY_PGUP)) playermotion += csVector3(0,speed,0);
  if (kbd->GetKeyState (CSKEY_PGDN)) playermotion += csVector3(0,-speed,0);
  if (kbd->GetKeyState (CSKEY_UP)) playermotion += csVector3(-speed,0,0);
  if (kbd->GetKeyState (CSKEY_DOWN)) playermotion += csVector3(speed,0,0);
  if(!playermotion.IsZero()) // keyboard stops player movement by mouse
    walking = false;
  if (mouse->GetLastButton(1))
  {
    walking = true;
    int mousex = mouse->GetLastX();
    int mousey = mouse->GetLastY();
    // y is up in camera view
    csVector2 screenpos(mousex, myG3D->GetHeight() - mousey);
    view->S2W(screenpos, lastclick);
  }
  if(walking)
  {
    // if no keyboard cursor use, and there was a click,
    // move towards last click position.
    speed *= 1.412f; // mouse move is as fast as keyboard can be
    playermotion = lastclick - player->GetPosition();
    float playermove = playermotion.Norm(); // distance moved
    if(playermove > speed) playermotion *= speed / playermove;

    // make sure we do not scroll too far
    csBox3 maxbox = player->GetGrid()->GetBox();
    maxbox.SetSize( (maxbox.Max() - maxbox.Min())*1.5 );
    if(!player->GetGrid()->Contains(lastclick)
      && !maxbox.In(view->GetViewScroll()))
      walking = false;
  }
  if(!playermotion.IsZero())
  {
    csVector3 oldpos = player->GetPosition();
    player->MovePosition(playermotion);
    view->MoveScroll(player->GetPosition() - oldpos);
    light->SetPosition(player->GetPosition()+csVector3(0,5,0));
  }

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS
    | CSDRAW_CLEARSCREEN ))
    return;

  view->Draw ();

  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  if(0) /// debug zbuffer
    DebugZBufShow(myG3D, myG2D, txtmgr);

  char buf[255];
  sprintf(buf, "FPS: %g    loc(%g,%g,%g)", fps, player->GetPosition().x,
    player->GetPosition().y, player->GetPosition().z);
  myG2D->Write(font, 10, myG2D->GetHeight() - 20, myG2D->FindRGB(255,255,255),
    -1, buf);
}

void IsoTest::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (0);
}


bool IsoTest::HandleEvent (iEvent &Event)
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
  if ((Event.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_TAB))
  {
    static float settings[][5] = {
    // scale should be *displayheight.
    //xscale, yscale, zscale, zskew, xskew
    {1.0f/16.0f, 1.0f/16.0f, 1.0f/16.0f, 1.0f, 1.0f},
    {1.0f/16.0f, 1.0f/16.0f, 1.0f/16.0f, 0.6f, 0.6f},
    {1.0f/16.0f, 1.0f/16.0f, 1.0f/16.0f, 0.5f, 0.5f},
    {1.0f/40.0f, 1.0f/40.0f, 1.0f/40.0f, 0.5f, 0.5f},
    {1.0f/8.0f,  1.0f/8.0f,  1.0f/8.0f,  0.5f, 0.5f},
    };
    static int nrsettings = 5;
    static int setting = 0;
    setting = (setting+1)%nrsettings;
    float h = engine->GetG3D()->GetHeight();
    float ycorrect = 1.0;
    if(setting!=0)
    {
      ycorrect = qsqrt(settings[setting][4]*settings[setting][4]+1.)
        + qsqrt(settings[setting][3]*settings[setting][3]+1.);
      ycorrect *= .5;
    }
    view->SetAxes( h*settings[setting][0], h*settings[setting][1]*ycorrect,
      h*settings[setting][2], settings[setting][3], settings[setting][4]);
    view->SetScroll(player->GetPosition(),
      csVector2(myG3D->GetWidth()/2, myG3D->GetHeight()/2));
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
  System = new IsoTest ();

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
