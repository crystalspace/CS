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
#include "cstool/initapp.h"
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
#include "imesh/cube.h"
#include "imesh/sprite2d.h"
#include "imesh/particle.h"
#include "imesh/partsys.h"
#include "imesh/fountain.h"
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "genmaze.h"

CS_IMPLEMENT_APPLICATION

// The global system driver
IsoTest *System;

IsoTest::IsoTest ()
{
  engine = NULL;
  view = NULL;
  world = NULL;
  font = NULL;
  light = NULL;
  myG2D = NULL;
  myG3D = NULL;
  lastclick.Set(0,0,0);
  walking = false;
}

IsoTest::~IsoTest ()
{
  if(view) view->DecRef();
  if (myG2D) myG2D->DecRef ();
  if (myG3D) myG3D->DecRef ();
  if(world) world->DecRef();
  if(engine) engine->DecRef();
  if(font) font->DecRef();
  if(light) light->DecRef();
}

void IsoTest::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (System->GetObjectRegistry (), iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.application.isotest", msg, arg);
  else
  {
    csVPrintf (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  delete System;
}

struct PlayerGridChange : public iGridChangeCallback
{
  IsoTest* app;
  SCF_DECLARE_IBASE;
  PlayerGridChange () { SCF_CONSTRUCT_IBASE (NULL); }
  virtual ~PlayerGridChange() { }
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


bool IsoTest::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  csInitializeApplication (this);
  iObjectRegistry* object_reg = GetObjectRegistry ();
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  // Find the pointer to engine plugin
  engine = CS_QUERY_PLUGIN (plugin_mgr, iIsoEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No IsoEngine plugin!");
    abort ();
  }

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    abort ();
  }

  myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics2D plugin!");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("IsoTest Crystal Space Application");
  if (!Open ())
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  iFontServer *fsvr = CS_QUERY_PLUGIN(plugin_mgr, iFontServer);
  font = fsvr->LoadFont(CSFONT_LARGE);
  fsvr->DecRef();

  // Setup the texture manager
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
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
  Report (CS_REPORTER_SEVERITY_NOTIFY,
    "IsoTest Crystal Space Application version 0.1.");

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");
  Report (CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  // create our world to play in, and a view on it.
  world = engine->CreateWorld();
  view = engine->CreateView(world);

  iMaterialWrapper *math1 = engine->CreateMaterialWrapper(
    "/lib/std/stone4.gif", "floor1");
  iMaterialWrapper *math2 = engine->CreateMaterialWrapper(
    "/lib/std/mystone2.gif", "floor2");
  iMaterialWrapper *snow = engine->CreateMaterialWrapper(
    "/lib/std/snow.jpg", "player");
  iMaterialWrapper *halo = engine->CreateMaterialWrapper(
    "/lib/stdtex/flare_purp.jpg", "halo");

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
        sprite->SetMaterialWrapper(math1);
      else sprite->SetMaterialWrapper(math2);
      world->AddSprite(sprite);
      for(my=0; my<multy; my++)
        for(mx=0; mx<multx; mx++)
          grid->SetGroundValue(x, y, mx, my, 0.0);
    }

  // add the player sprite to the world
  csVector3 startpos(10,0,5);
  player = engine->CreateFrontSprite(startpos, 1.3, 2.7);
  player->SetMaterialWrapper(snow);
  player->SetMixMode(CS_FX_ADD);
  world->AddSprite(player);
  PlayerGridChange* cb = new PlayerGridChange ();
  cb->app = this;
  player->SetGridChangeCallback(cb);
  cb->DecRef ();

  // add a light to the scene.
  iIsoLight *scenelight = engine->CreateLight();
  scenelight->SetPosition(csVector3(3,2,6));
  scenelight->SetGrid(grid);
  scenelight->SetAttenuation(CSISO_ATTN_INVERSE);
  scenelight->SetRadius(5.0);
  scenelight->SetColor(csColor(0.0, 0.4, 1.0));

  // add a glowing halo to the light
  sprite = engine->CreateFrontSprite(scenelight->GetPosition() -
    csVector3(0,1.0,0), 2.0, 2.0);
  sprite->SetMaterialWrapper(halo);
  sprite->SetMixMode(CS_FX_ADD);
  world->AddSprite(sprite);

  // add a dynamic light for the player, above players head.
  light = engine->CreateLight();
  light->Flags().Set(CSISO_LIGHT_DYNAMIC);
  light->SetPosition(startpos+csVector3(0,5,0));
  light->SetGrid(grid);
  light->SetRadius(5.0);
  light->SetColor(csColor(1.0, 0.4, 0.2));

  // make a bump to cast shadows
  for(my=0; my<multy; my++)
    for(mx=0; mx<multx; mx++)
      grid->SetGroundValue(5, 15, mx, my, 4.0);
  sprite = engine->CreateFloorSprite(csVector3(15,4,5), 1.0, 1.0);
  sprite->SetMaterialWrapper(math1);
  world->AddSprite(sprite);
  for(my=0; my<4; my++)
  {
    sprite = engine->CreateXWallSprite(csVector3(15,my,5), 1.0, 1.0);
    sprite->SetMaterialWrapper(math1);
    world->AddSprite(sprite);
    sprite = engine->CreateZWallSprite(csVector3(16,my,5), 1.0, 1.0);
    sprite->SetMaterialWrapper(math1);
    world->AddSprite(sprite);
  }

  for(mx=0; mx<8; mx++)
  {
    sprite = engine->CreateFloorSprite(csVector3(5,4,2+mx), 1.0, 1.0);
    sprite->SetMaterialWrapper(math1);
    world->AddSprite(sprite);
  }
  for(my=0; my<4; my++)
  {
    sprite = engine->CreateXWallSprite(csVector3(5,my,2), 1.0, 1.0);
    sprite->SetMaterialWrapper(math1);
    world->AddSprite(sprite);
    sprite = engine->CreateXWallSprite(csVector3(5,my,7), 1.0, 1.0);
    sprite->SetMaterialWrapper(math2);
    world->AddSprite(sprite);
    for(mx=0; mx<3; mx++)
    {
      sprite = engine->CreateZWallSprite(csVector3(6,my,2+mx), 1.0, 1.0);
      sprite->SetMaterialWrapper(math1);
      world->AddSprite(sprite);
      sprite = engine->CreateZWallSprite(csVector3(6,my,7+mx), 1.0, 1.0);
      sprite->SetMaterialWrapper(math1);
      world->AddSprite(sprite);
    }
  }
  sprite = engine->CreateZWallSprite(csVector3(6,3,5), 1.0, 1.0);
  sprite->SetMaterialWrapper(math1);
  world->AddSprite(sprite);
  sprite = engine->CreateZWallSprite(csVector3(6,3,6), 1.0, 1.0);
  sprite->SetMaterialWrapper(math1);
  world->AddSprite(sprite);
  for(x=0; x<3; x++)
    for(my=0; my<multy; my++)
      for(mx=0; mx<multx; mx++)
      {
        grid->SetGroundValue(2+x, 5, mx, my, 4.0);
        grid->SetGroundValue(7+x, 5, mx, my, 4.0);
      }

  //bool res = grid->GroundHitBeam(csVector3(10,1,5), csVector3(15,-2,8));
  //printf("Hitbeam gave %d\n", (int)res);
  //res = grid->GroundHitBeam(csVector3(10,1,5), csVector3(20,0,10));
  //printf("Hitbeam gave %d\n", (int)res);

  /// create a mesh object
  const char* classId = "crystalspace.mesh.object.cube";
  iMeshObjectFactory *mesh_fact = engine->CreateMeshFactory(classId, 
    "cubeFact");

  ///*
  iCubeFactoryState* cubelook = SCF_QUERY_INTERFACE(mesh_fact, iCubeFactoryState);
  cubelook->SetMaterialWrapper(math2);
  //cubelook->SetSize(.5, .5, .5);
  cubelook->SetSize(1,1,1);
  cubelook->SetMixMode (CS_FX_COPY);
  cubelook->DecRef();
  //*/
  /*
  iSprite2DFactoryState *fstate = SCF_QUERY_INTERFACE(mesh_fact, 
    iSprite2DFactoryState);
  fstate->SetMaterialWrapper(math2);
  fstate->SetMixMode(CS_FX_ADD);
  fstate->SetLighting(false);
  fstate->DecRef();
  */

  iMeshObject *mesh_obj = mesh_fact->NewInstance();

  /*
  iSprite2DState *ostate = SCF_QUERY_INTERFACE(mesh_obj, iSprite2DState);
  ostate->CreateRegularVertices(6, true);
  ostate->DecRef();
  //iParticle *pstate = SCF_QUERY_INTERFACE(mesh_obj, iParticle);
  //pstate->ScaleBy(10.0);
  //pstate->DecRef();
  */

  /// add a mesh object sprite
  iIsoMeshSprite *meshspr = engine->CreateMeshSprite();
  meshspr->SetMeshObject(mesh_obj);
  //meshspr->SetZBufMode(CS_ZBUF_NONE);
  meshspr->SetPosition( csVector3(12 + 0.5, +0.5, 2+0.5) );
  world->AddSprite(meshspr);

  //player = meshspr;
  
  const char* fo_classId = "crystalspace.mesh.object.fountain";
  mesh_fact = engine->CreateMeshFactory(fo_classId, "fountainFact");

  mesh_obj = mesh_fact->NewInstance();
  iParticleState *pastate = SCF_QUERY_INTERFACE(mesh_obj, iParticleState);
  iFountainState *fostate = SCF_QUERY_INTERFACE(mesh_obj, iFountainState);
  pastate->SetMaterialWrapper(halo);
  pastate->SetMixMode(CS_FX_ADD);
  pastate->SetColor( csColor(0.125, 0.5, 1.0) );
  fostate->SetParticleCount(100);
  fostate->SetDropSize(0.1, 0.1);
  fostate->SetOrigin( csVector3(0,0,0) );
  fostate->SetLighting(false);
  fostate->SetAcceleration( csVector3(0, -.3, 0) );
  fostate->SetElevation(PI/2.0);
  fostate->SetAzimuth(1.0);
  fostate->SetOpening(0.14);
  fostate->SetSpeed(1.0);
  fostate->SetFallTime(6.0);
  pastate->DecRef();
  fostate->DecRef();

  meshspr = engine->CreateMeshSprite();
  meshspr->SetMeshObject(mesh_obj);
  meshspr->SetZBufMode(CS_ZBUF_TEST);
  meshspr->SetPosition( csVector3(10, 0, 8) );
  world->AddSprite(meshspr);

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
      for(my=0; my<multy; my++)
        for(mx=0; mx<multx; mx++)
          grid2->SetGroundValue(x-10, y-10, mx, my, 0.0);
    }

  // add a light
  scenelight = engine->CreateLight();
  scenelight->SetPosition(csVector3(13,2,26));
  scenelight->SetGrid(grid2);
  scenelight->SetAttenuation(CSISO_ATTN_INVERSE);
  scenelight->SetRadius(5.0);
  scenelight->SetColor(csColor(0.0, 0.4, 1.0));
  // add the light to both grids to make it look right.
  scenelight = engine->CreateLight();
  scenelight->SetPosition(csVector3(13,2,26));
  scenelight->SetGrid(grid);
  scenelight->SetAttenuation(CSISO_ATTN_INVERSE);
  scenelight->SetRadius(5.0);
  scenelight->SetColor(csColor(0.0, 0.4, 1.0));

  /// add maze grid
  printf("Adding maze grid.\n");
  AddMazeGrid(world, 20, 20, math2, math1);
  printf("Adding maze grid done.\n");

  // prepare texture manager
  txtmgr->PrepareTextures ();
  txtmgr->PrepareMaterials ();
  txtmgr->SetPalette ();

  // scroll view to show player position at center of screen
  view->SetScroll(startpos, csVector2(myG3D->GetWidth()/2, myG3D->GetHeight()/2));

  return true;
}


// static helper
static void AddWall(iIsoEngine *engine, iIsoWorld *world, iIsoGrid *grid,
  int x, int y, int offx, int offy, int multx, int multy, 
  float bot, float height, 
  iMaterialWrapper *side, iMaterialWrapper *top)
{
  height = 0;
  return;
  for(int my=0; my<multy; my++)
    for(int mx=0; mx<multx; mx++)
      grid->SetGroundValue(x, y, mx, my, height);
  iIsoSprite *sprite = 0;
  if(bot != height)
  {
    height -= bot;
    height = 1.0;
    sprite = engine->CreateZWallSprite(csVector3(y+offy+0.999,bot,x+offy), 
      1.0, height);
    sprite->SetMaterialWrapper(side);
    //sprite->SetMixMode( CS_FX_COPY | CS_FX_TILING );
    world->AddSprite(sprite);
    sprite = engine->CreateXWallSprite(csVector3(y+offy,bot,x+offy), 
      1.0, height);
    sprite->SetMaterialWrapper(side);
    //sprite->SetMixMode( CS_FX_COPY | CS_FX_TILING );
    world->AddSprite(sprite);
  }
  sprite = engine->CreateFloorSprite(csVector3(y+offy,height,x+offx), 1.0, 1.0);
  sprite->SetMaterialWrapper(top);
  world->AddSprite(sprite);
}

void IsoTest::AddMazeGrid(iIsoWorld *world, float posx, float posy,
  iMaterialWrapper *floor, iMaterialWrapper *wall)
{
  int width = 40;
  int height = 40;
  /// create the maze
  csGenMaze *maze = new csGenMaze(width, height);
  //maze->SetStraightness(0.1);
  //maze->SetCyclicalness(0.1);
  // add some access points to the maze, at the top.
  int x,y;
  for(x=1; x<width; x+=3)
    maze->MakeAccess(x, 0);
  //maze->GenerateMaze(0,0);

  /// debug display
#if 0
  for(y=0; y<height; y++)
  {
    printf("X");
    for(x=0; x<width; x++)
    {
      if(maze->GetNode(x,y).opening[0])
        printf(" ");
      else printf("X");
      printf("X");
    }
    printf("\n");
    for(x=0 ; x<width; x++)
    {
      if(maze->GetNode(x,y).opening[3])
        printf(" ");
      else printf("X");
      printf(" ");
    }
    if(maze->GetNode(width-1,y).opening[1])
      printf(" ");
    else printf("X");
    printf("\n");
  }
  printf("X");
  for(x=0 ; x<width; x++)
  {
    if(maze->GetNode(x,height-1).opening[2])
      printf(" ");
    else printf("X");
    printf("X");
  }
  printf("\n");
#endif

  // create a grid to display the maze in.
  int gridw = width*2+1;
  int gridh = height*2+1;
  iIsoGrid *mazegrid = world->CreateGrid(gridw, gridh);
  mazegrid->SetSpace((int)posy, (int)posx, -1.0, +10.0);
  return;
  int multx = 1;
  int multy = 1;
  mazegrid->SetGroundMult(multx, multy);

  /*
  iIsoSprite *sprite; int mx, my;
  for(y=(int)posy; y<posy+gridh; y++)
    for(x=(int)posx; x<posx+gridw; x++)
    {
      // put tiles on the floor
      sprite = engine->CreateFloorSprite(csVector3(y,0,x), 1.0, 1.0);
      sprite->SetMaterialWrapper(floor);
      world->AddSprite(sprite);
      for(my=0; my<multy; my++)
        for(mx=0; mx<multx; mx++)
          mazegrid->SetGroundValue(x-posx, y-posy, mx, my, 0.0);
    }
   */


  // add a light
  iIsoLight* scenelight = engine->CreateLight();
  scenelight->SetPosition(csVector3(posy+height+0.5,10,posx+width+0.5));
  scenelight->SetGrid(mazegrid);
  scenelight->SetAttenuation(CSISO_ATTN_INVERSE);
  scenelight->SetRadius(10.0 + width/2 + height/2);
  scenelight->SetColor(csColor(0.0, 0.4, 1.0));

  // add walls
  
#define ADDWALLFULL(x, y) AddWall(engine, world, mazegrid, x, y, (int)posx, \
  (int)posy, multx, multy, 0.0, 2.0, wall, floor);
#define ADDWALLEMPTY(x, y) AddWall(engine, world, mazegrid, x, y, (int)posx, \
  (int)posy, multx, multy, 0.0, 0.0, wall, floor);

  for(y=0; y<height; y++)
  {
    //printf("y=%d\n", y);
    for(x=0; x<width; x++)
    {
      ADDWALLFULL(x*2, y*2)
      if(maze->GetNode(x,y).opening[0])
        ADDWALLEMPTY(x*2+1, y*2)
      else ADDWALLFULL(x*2+1, y*2)
    }
    ADDWALLFULL(gridw-1, y*2)
    for(x=0 ; x<width; x++)
    {
      if(maze->GetNode(x,y).opening[3])
        ADDWALLEMPTY(x*2, y*2+1)
      else ADDWALLFULL(x*2, y*2+1)
      ADDWALLEMPTY(x*2+1, y*2+1)
    }
    if(maze->GetNode(width-1,y).opening[1])
      ADDWALLEMPTY(gridw-1, y*2+1)
    else ADDWALLFULL(gridw-1, y*2+1)
  }

  //printf("y=%d\n", y);
  for(x=0; x<width; x++)
  {
    ADDWALLFULL(x*2, gridh-1)
    if(maze->GetNode(x,height-1).opening[2])
      ADDWALLEMPTY(x*2+1, gridh-1)
    else ADDWALLFULL(x*2+1, gridh-1)
  }
  ADDWALLFULL(gridw-1, gridh-1)
  
}



void IsoTest::NextFrame ()
{
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  SysSystemDriver::NextFrame ();
  csTime elapsed_time, current_time;
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
  if (GetKeyState (CSKEY_UP)) playermotion += csVector3(-speed,0,0);
  if (GetKeyState (CSKEY_DOWN)) playermotion += csVector3(speed,0,0);
  if(!playermotion.IsZero()) // keyboard stops player movement by mouse
    walking = false;
  if (GetMouseButton(1))
  {
    walking = true;
    int mousex, mousey; 
    GetMousePosition(mousex, mousey);
    // y is up in camera view
    csVector2 screenpos(mousex, myG3D->GetHeight() - mousey);
    view->S2W(screenpos, lastclick);
  }
  if(walking) 
  {
    // if no keyboard cursor use, and there was a click,
    // move towards last click position.
    speed *= 1.412; // mouse move is as fast as keyboard can be
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

  char buf[255];
  sprintf(buf, "FPS: %g    loc(%g,%g,%g)", fps, player->GetPosition().x,
    player->GetPosition().y, player->GetPosition().z);
  myG2D->Write(font, 10, myG2D->GetHeight() - 20, txtmgr->FindRGB(255,255,255),
    -1, buf);
  
  // Drawing code ends here.
  myG3D->FinishDraw ();

  // Print the final output.
  myG3D->Print (NULL);
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
    float ycorrect = 1.0;
    if(setting!=0)
    {
      ycorrect = sqrt(settings[setting][4]*settings[setting][4]+1.) 
        + sqrt(settings[setting][3]*settings[setting][3]+1.);
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
  srand (time (NULL));

  // Create our main class.
  System = new IsoTest ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.console.output.standard:Console.Output");
  System->RequestPlugin ("crystalspace.engine.iso:Engine.Iso");

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  System->Loop ();

  // Cleanup.
  Cleanup ();

  return 0;
}
