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
#include "imap/parser.h"
#include "igraphic/imageio.h"
#include "csgeom/box.h"
#include "imesh/object.h"
#include "imesh/cube.h"
#include "imesh/sprite2d.h"
#include "imesh/particle.h"
#include "imesh/partsys.h"
#include "imesh/fountain.h"


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

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
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

  myG3D = QUERY_PLUGIN (this, iGraphics3D);
  if (!myG3D)
  {
    Printf (MSG_FATAL_ERROR, "No iGraphics3D plugin!\n");
    abort ();
  }

  myG2D = QUERY_PLUGIN (this, iGraphics2D);
  if (!myG2D)
  {
    Printf (MSG_FATAL_ERROR, "No iGraphics2D plugin!\n");
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
  Printf (MSG_INITIALIZATION,
    "IsoTest Crystal Space Application version 0.1.\n");

  // Create our world.
  Printf (MSG_INITIALIZATION, "Creating world!...\n");

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");

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
  player->SetGridChangeCallback(PlayerGridChange, (void*)this);

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
  //const char* classId = "crystalspace.mesh.object.sprite.2d";
  iMeshObjectType *mesh_type = LOAD_PLUGIN( System, classId, 
    "MeshObj", iMeshObjectType);
  iMeshObjectFactory *mesh_fact = mesh_type->NewFactory();

  ///*
  iCubeFactoryState* cubelook = QUERY_INTERFACE(mesh_fact, iCubeFactoryState);
  cubelook->SetMaterialWrapper(math2);
  cubelook->SetSize(.5, .5, .5);
  cubelook->SetMixMode (CS_FX_COPY);
  cubelook->DecRef();
  //*/
  /*
  iSprite2DFactoryState *fstate = QUERY_INTERFACE(mesh_fact, 
    iSprite2DFactoryState);
  fstate->SetMaterialWrapper(math2);
  fstate->SetMixMode(CS_FX_ADD);
  fstate->SetLighting(false);
  fstate->DecRef();
  */

  iMeshObject *mesh_obj = mesh_fact->NewInstance();

  /*
  iSprite2DState *ostate = QUERY_INTERFACE(mesh_obj, iSprite2DState);
  ostate->CreateRegularVertices(6, true);
  ostate->DecRef();
  //iParticle *pstate = QUERY_INTERFACE(mesh_obj, iParticle);
  //pstate->ScaleBy(10.0);
  //pstate->DecRef();
  */

  /// add a mesh object sprite
  iIsoMeshSprite *meshspr = engine->CreateMeshSprite();
  meshspr->SetMeshObject(mesh_obj);
  //meshspr->SetZBufMode(CS_ZBUF_NONE);
  meshspr->SetPosition( csVector3(12, 1, 4) );
  world->AddSprite(meshspr);

  const char* fo_classId = "crystalspace.mesh.object.fountain";
  mesh_type = LOAD_PLUGIN( System, fo_classId, "MeshObj", iMeshObjectType);
  mesh_fact = mesh_type->NewFactory();

  mesh_obj = mesh_fact->NewInstance();
  iParticleState *pastate = QUERY_INTERFACE(mesh_obj, iParticleState);
  iFountainState *fostate = QUERY_INTERFACE(mesh_obj, iFountainState);
  pastate->SetMaterialWrapper(halo);
  pastate->SetMixMode(CS_FX_ADD);
  pastate->SetColor( csColor(0.125, 0.5, 1.0) );
  fostate->SetNumberParticles(100);
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

  // prepare texture manager
  txtmgr->PrepareTextures ();
  txtmgr->PrepareMaterials ();
  txtmgr->SetPalette ();

  // scroll view to show player position at center of screen
  view->SetScroll(startpos, csVector2(myG3D->GetWidth()/2, myG3D->GetHeight()/2));

  return true;
}

void IsoTest::NextFrame ()
{
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
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
