/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A Wijngaards

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
#include "bumptest.h"
#include "prbump.h"

#include "cssys/system.h"
#include "csgeom/transfrm.h"
#include "cstool/proctex.h"
#include "cstool/prdots.h"
#include "cstool/prplasma.h"
#include "cstool/prfire.h"
#include "cstool/prwater.h"
#include "cstool/csview.h"

#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"
#include "imesh/object.h"
#include "imesh/cube.h"
#include "imesh/ball.h"
#include "imesh/sprite3d.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/ptextype.h"
#include "imap/parser.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "iengine/light.h"
#include "iengine/dynlight.h"
#include "iengine/camera.h"
#include "igraphic/imageio.h"
#include "imesh/object.h"
#include "imesh/lighting.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// the global system driver variable
BumpTest *System;

BumpTest::BumpTest ()
{
  view = NULL;
  engine = NULL;
  dynlight = NULL;
  prBump = NULL;
  matBump = NULL;
  LevelLoader = NULL;
  bumplight = NULL;
  animli = 0.0;
  going_right = true;
  myG3D = NULL;
}

BumpTest::~BumpTest ()
{
  if (view) view->DecRef ();
  if (engine) engine->DecRef ();
  delete prBump;
  if (LevelLoader) LevelLoader->DecRef();
  if (myG3D) myG3D->DecRef ();
}

void Cleanup ()
{
  System->ConsoleOut ("Cleaning up...\n");
  delete System;
}

bool BumpTest::InitProcDemo ()
{
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  iMaterialWrapper* itm = engine->FindMaterial ("wood");

  //char *vfsfilename = "/lib/std/mystone2.gif";
  char *vfsfilename = "/lib/std/stone4.gif";
  iTextureWrapper *bptex = engine->CreateTexture("bumptex", vfsfilename, 0,
    CS_TEXTURE_2D| CS_TEXTURE_3D);
  iMaterialWrapper* ibp = engine->CreateMaterial("bumptexture", bptex);
  engine->Prepare ();
  //iImageIO *imgloader = CS_QUERY_PLUGIN(System, iImageIO);
  //iVFS *VFS = CS_QUERY_PLUGIN(System, iVFS);
  //iDataBuffer *buf = VFS->ReadFile (vfsfilename);
  //iImage *map = imgloader->Load(buf->GetUint8 (), buf->GetSize (),
      //txtmgr->GetTextureFormat ());
  //map->IncRef();
  //buf->DecRef();
  //VFS->DecRef();
  //imgloader->DecRef();
  iImage *map = bptex->GetImageFile();
  prBump = new csProcBump (map);
  //prBump->SetBumpMap(map);
  matBump = prBump->Initialize (this, engine, txtmgr, "bumps");
  prBump->PrepareAnim ();
  iMeshObjectType* thing_type = engine->GetThingType ();
  iMeshObjectFactory* thing_fact = thing_type->NewFactory ();
  iMeshObject* thing_obj = SCF_QUERY_INTERFACE (thing_fact, iMeshObject);
  thing_fact->DecRef ();


  iMaterialWrapper* imatBump = SCF_QUERY_INTERFACE (matBump, iMaterialWrapper);
  iThingState* thing_state = SCF_QUERY_INTERFACE (thing_obj, iThingState);
  float dx = 1, dy = 1, dz = 1;
  iPolygon3D* p;

  /// the stone
  p = thing_state->CreatePolygon ();
  p->SetMaterial (itm);
  p->CreateVertex (csVector3 (-dx, +dy, -dz));
  p->CreateVertex (csVector3 (+dx, +dy, -dz));
  p->CreateVertex (csVector3 (+dx, -dy, -dz));
  p->CreateVertex (csVector3 (-dx, -dy, -dz));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1.0);

  // to seee unbumpmapped
  p = thing_state->CreatePolygon ();
  p->SetMaterial (itm);
  p->CreateVertex (csVector3 (-dx, +dy, -dz) + csVector3(-2.5,0,0));
  p->CreateVertex (csVector3 (+dx, +dy, -dz) + csVector3(-2.5,0,0));
  p->CreateVertex (csVector3 (+dx, -dy, -dz) + csVector3(-2.5,0,0));
  p->CreateVertex (csVector3 (-dx, -dy, -dz) + csVector3(-2.5,0,0));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1.0);
  csVector3 overdist(0,0,-0.01); // to move slightly in front

#if 1
  /// the bumpoverlay
  // this does not work - the texture is flat shaded too.
  p = thing_state->CreatePolygon ();
  p->SetTextureType(POLYTXT_LIGHTMAP);
  //p->SetLightingMode(false);
  p->GetFlags().Set(CS_POLY_LIGHTING, 0); // the overlay is not lit
  p->SetMaterial (imatBump);
  p->CreateVertex (csVector3 (-dx, +dy, -dz)+overdist);
  p->CreateVertex (csVector3 (+dx, +dy, -dz)+overdist);
  p->CreateVertex (csVector3 (+dx, -dy, -dz)+overdist);
  p->CreateVertex (csVector3 (-dx, -dy, -dz)+overdist);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1.0);
  //const char *conv[] = {"POLYTXT_NONE", "POLYTXT_FLAT", "POLYTXT_GOURAUD",
    //"POLYTXT_LIGHTMAP"};
  //printf("PolyTexType = %s\n", conv[p->GetTextureType()]);
  //printf("MaterialWrap = %x  Handle %x\n", (int)imatBump, (int)imatBump->GetMaterialHandle());
  iPolyTexType *ipn = p->GetPolyTexType();
  if(!ipn) printf("No PolyTexNone info!\n");
  else ipn->SetMixMode(CS_FX_MULTIPLY2);
  //if(ipn) ipn->SetMixMode(CS_FX_COPY);
#endif

  ////// copy of bumps for debug
  p = thing_state->CreatePolygon ();
  p->SetMaterial (imatBump);
  p->GetFlags().Set(CS_POLY_LIGHTING, 0); // not lit
  p->CreateVertex (csVector3 (-dx, +0, -dz) + csVector3(2.5,0,0));
  p->CreateVertex (csVector3 (+0, +0, -dz) + csVector3(2.5,0,0));
  p->CreateVertex (csVector3 (+0, -dy, -dz) + csVector3(2.5,0,0));
  p->CreateVertex (csVector3 (-dx, -dy, -dz) + csVector3(2.5,0,0));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1.0);

  //ibp->GetMaterialHandle()->Prepare();
  p = thing_state->CreatePolygon ();
  p->SetMaterial (ibp);
  p->CreateVertex (csVector3 (-dx, +0, -dz) + csVector3(2.5,1,0));
  p->CreateVertex (csVector3 (+0, +0, -dz) + csVector3(2.5,1,0));
  p->CreateVertex (csVector3 (+0, -dy, -dz) + csVector3(2.5,1,0));
  p->CreateVertex (csVector3 (-dx, -dy, -dz) + csVector3(2.5,1,0));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1.0);
  //engine->Prepare();

  /*
  p = thing_state->CreatePolygon ();
  p->SetMaterial (imatBump);
  p->CreateVertex (csVector3 (+dx, +dy, -dz));
  p->CreateVertex (csVector3 (-dx, +dy, -dz));
  p->CreateVertex (csVector3 (-dx, +dy, +dz));
  p->CreateVertex (csVector3 (+dx, +dy, +dz));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), .5);
  p = thing_state->CreatePolygon ();
  p->SetMaterial (imatBump);
  p->CreateVertex (csVector3 (+dx, -dy, +dz));
  p->CreateVertex (csVector3 (-dx, -dy, +dz));
  p->CreateVertex (csVector3 (-dx, -dy, -dz));
  p->CreateVertex (csVector3 (+dx, -dy, -dz));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1);
  p = thing_state->CreatePolygon ();
  p->SetMaterial (imatBump);
  p->GetFlags().Set (CS_POLY_LIGHTING, 0);
  p->CreateVertex (csVector3 (-dx, +dy, -dz));
  p->CreateVertex (csVector3 (+dx, +dy, -dz));
  p->CreateVertex (csVector3 (+dx, -dy, -dz));
  p->CreateVertex (csVector3 (-dx, -dy, -dz));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 2);
  p = thing_state->CreatePolygon ();
  p->SetMaterial (imatBump);
  p->GetFlags().Set (CS_POLY_LIGHTING, 0);
  p->CreateVertex (csVector3 (-dx, -dy, -dz));
  p->CreateVertex (csVector3 (-dx, -dy, +dz));
  p->CreateVertex (csVector3 (-dx, +dy, +dz));
  p->CreateVertex (csVector3 (-dx, +dy, -dz));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 1);
  p = thing_state->CreatePolygon ();
  p->SetMaterial (imatBump);
  p->CreateVertex (csVector3 (+dx, -dy, +dz));
  p->CreateVertex (csVector3 (+dx, -dy, -dz));
  p->CreateVertex (csVector3 (+dx, +dy, -dz));
  p->CreateVertex (csVector3 (+dx, +dy, +dz));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), .5);
  p = thing_state->CreatePolygon ();
  p->SetMaterial (imatBump);
  p->CreateVertex (csVector3 (-dx, -dy, +dz));
  p->CreateVertex (csVector3 (+dx, -dy, +dz));
  p->CreateVertex (csVector3 (+dx, +dy, +dz));
  p->CreateVertex (csVector3 (-dx, +dy, +dz));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), .5);
  */

#if 0
  iSector* iroom = SCF_QUERY_INTERFACE (room, iSector);
#endif
  iMeshWrapper* thing_wrap = engine->CreateMeshObject ("Bumpy");
  thing_obj->DecRef ();

  thing_wrap->SetMeshObject (thing_obj);
  thing_wrap->HardTransform (csTransform (csMatrix3 (), csVector3 (0, 5, 1)));
  thing_wrap->GetMovable ()->SetSector (room);
  thing_wrap->GetMovable ()->UpdateMove ();
  thing_state->DecRef ();

  iLightingInfo* linfo = SCF_QUERY_INTERFACE (thing_obj, iLightingInfo);
  linfo->InitializeDefault ();
  room->ShineLights (thing_wrap);
  linfo->PrepareLighting ();
  linfo->DecRef ();

#if 0
  iMeshFactoryWrapper* sprfact = engine->CreateMeshFactory (
    "crystalspace.mesh.object.sprite.3d", "sprite3d");
  iSprite3DFactoryState* sprfactstate = SCF_QUERY_INTERFACE(
    sprfact->GetMeshObjectFactory(), iSprite3DFactoryState);
  sprfactstate->SetMaterialWrapper(imatBump);
  iSpriteAction *a0 = sprfactstate->AddAction();
  a0->SetName("Action0");
  iSpriteFrame *f0 = sprfactstate->AddFrame();
  a0->AddFrame(f0, 100);
  sprfactstate->AddVertices(4);
  sprfactstate->GetVertex(0,0).Set (csVector3 (-dx, +dy, -dz)+overdist);
  sprfactstate->GetVertex(0,1).Set (csVector3 (+dx, +dy, -dz)+overdist);
  sprfactstate->GetVertex(0,2).Set (csVector3 (+dx, -dy, -dz)+overdist);
  sprfactstate->GetVertex(0,3).Set (csVector3 (-dx, -dy, -dz)+overdist);
  sprfactstate->GetTexel(0,0).Set(0,0);
  sprfactstate->GetTexel(0,1).Set(2,0);
  sprfactstate->GetTexel(0,2).Set(2,2);
  sprfactstate->GetTexel(0,3).Set(0,2);
  sprfactstate->AddTriangle(0,1,2);
  sprfactstate->AddTriangle(0,2,3);
  sprfactstate->DecRef();
  iMeshWrapper* sprite = engine->CreateMeshObject(sprfact, "bumpspr",
    iroom, csVector3(0, 5, 1) );
  sprite->GetMovable ()->UpdateMove ();
  iSprite3DState* spstate = SCF_QUERY_INTERFACE (sprite->GetMeshObject (), 
    iSprite3DState);

  //sprite->GetFlags().Set(CS_ENTITY_NOLIGHTING, CS_ENTITY_NOLIGHTING);
  //sprite->GetFlags().Set(CS_ENTITY_NOSHADOWS, CS_ENTITY_NOSHADOWS);
  spstate->SetLighting(false);
  spstate->SetBaseColor(csColor(1.,1.,1.));
  spstate->SetMaterialWrapper(imatBump);
  //spstate->SetMixMode(CS_FX_COPY);
  spstate->SetMixMode(CS_FX_MULTIPLY2 | CS_FX_TILING);
  spstate->DecRef ();
#endif

// @@@ mem leaks, but the engine does not IncRef these yet
/*
  iroom->DecRef();
  //sprfact->DecRef(); // mem leak

  thing_state->DecRef ();
  imatBump->DecRef ();
  itm->DecRef();
*/

  return true;
}

bool BumpTest::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Find the pointer to engine plugin
  engine = CS_QUERY_PLUGIN (this, iEngine);
  if (!engine)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iEngine plugin!\n");
    abort ();
  }

  LevelLoader = CS_QUERY_PLUGIN_ID (this, CS_FUNCID_LVLLOADER, iLoader);
  if (!LevelLoader)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iLoader plugin!\n");
    abort ();
  }

  myG3D = CS_QUERY_PLUGIN_ID (this, CS_FUNCID_VIDEO, iGraphics3D);
  if (!myG3D)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iGraphics3D plugin!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("BumpTest Crystal Space Application"))
  {
    Printf (CS_MSG_FATAL_ERROR, "Error opening system!\n");
    Cleanup ();
    exit (1);
  }

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
  Printf (CS_MSG_INITIALIZATION,
    "BumpTest Crystal Space Application version 0.1.\n");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->EnableLightingCache (false);

  // Create our world.
  Printf (CS_MSG_INITIALIZATION, "Creating world!...\n");

  LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
  LevelLoader->LoadTexture ("wood", "/lib/std/andrew_wood.gif");
  iMaterialWrapper* tm = engine->FindMaterial ("stone");

  room = engine->CreateSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  iPolygon3D* p;
  iThingState* walls_state = SCF_QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->CreateVertex (csVector3 (5, 0, 5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->CreateVertex (csVector3 (-5, 0, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (5, 0, 5));
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 20, 5));
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->CreateVertex (csVector3 (5, 0, 5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (-5, 20, 5));
  p->CreateVertex (csVector3 (-5, 0, 5));
  p->CreateVertex (csVector3 (-5, 0, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (tm);
  p->CreateVertex (csVector3 (5, 20, -5));
  p->CreateVertex (csVector3 (-5, 20, -5));
  p->CreateVertex (csVector3 (-5, 0, -5));
  p->CreateVertex (csVector3 (5, 0, -5));
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  walls_state->DecRef ();

#if 0
  LevelLoader->LoadTexture ("flare_center", "/lib/std/snow.jpg");
  iMaterialWrapper* fmc = engine->FindMaterial ("flare_center");
  LevelLoader->LoadTexture ("flare_spark", "/lib/std/spark.png");
  iMaterialWrapper* fms = engine->FindMaterial ("flare_spark");
#endif

  /*
  csStatLight* light;
  //light = new csStatLight (-3, 5, -2, 10, 1, 0, 0, false); // red
  light = new csStatLight (-3, 5, -2, 10, 1, 1, 1, false); // white
  light->SetHalo (new csCrossHalo (1.0, 0.7));  // intensity, crossfactor
  room->AddLight (light);
  bumplight = SCF_QUERY_INTERFACE(light, iLight);
  //light = new csStatLight (3, 5, 0, 10, 0, 0, 1, false);
  //light = new csStatLight (0, 5, -3, 10, 0, 1, 0, false);
  */

  InitProcDemo ();

  //engine->Prepare ();

  // Create a -dynamic light.
  //float angle = 0;
  //dynlight = new csDynLight (cos (angle)*3, 5, sin (angle)*3, 7, 1, 1, 1);
  dynlight = engine->CreateDynLight (csVector3 (-3, 5, -2), 7, csColor (1, 1, 1));
  dynlight->QueryLight ()->CreateCrossHalo (1.0, 0.7);  // intensity, crossfactor
  dynlight->QueryLight ()->SetSector (room);
  dynlight->Setup ();
  bumplight = dynlight->QueryLight ();

  Printf (CS_MSG_INITIALIZATION, "--------------------------------------\n");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, myG3D);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  view->SetRectangle (0, 0, FrameWidth, FrameHeight);

  txtmgr->SetPalette ();

  return true;
}

void BumpTest::NextFrame ()
{
  SysSystemDriver::NextFrame ();
  csTime elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);
  //printf("elapsed_time %d\n", (int)elapsed_time);

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.03 * 20);

  if (GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
  if (GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
  if (GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_UP, speed);
  if (GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);
  if (GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (VEC_FORWARD * 4.0f * speed);
  if (GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (VEC_BACKWARD * 4.0f * speed);

  // Move the -dynamic light around.
  //angle += elapsed_time * 0.4 / 1000.;
  if(going_right)
  {
    animli += speed * 2.5;
    if(animli > 7.0) going_right = false;
  }
  else 
  {
    animli -= speed * 2.5;
    if(animli < 0.0) going_right = true;
  }
  dynlight->QueryLight ()->SetSector (room);
  dynlight->QueryLight ()->SetCenter (csVector3(-3 + animli, 5, -2));
  //printf("Moved to %g\n", -3 + animli);
  dynlight->Setup ();

  //while (angle >= 2.*3.1415926) angle -= 2.*3.1415926;
  //csVector3 pos (cos (angle)*3, 17, sin (angle)*3);
  //dynlight->Move (room, pos.x, pos.y, pos.z);
  //dynlight->Setup ();
  csVector3 center(0,5,-1);
  csVector3 normal(0,0,-1);
  //(-1,5+1,-1); (+1,5-1,-1);
  csVector3 xdir(1,0,0);
  csVector3 ydir(0,-1,0);
  iLight *l = bumplight;

  //prBump->Recalc(center, normal, xdir, ydir, 1, &l);
  prBump->RecalcFast(center, normal, xdir, ydir, 1, &l);

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();

  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  // Drawing code ends here.
  myG3D->FinishDraw ();

  // Print the final output.
  myG3D->Print (NULL);
}

bool BumpTest::HandleEvent (iEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    Shutdown = true;
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
  System = new BumpTest ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, "/config/csbumptest.cfg"))
  {
    System->Printf (CS_MSG_FATAL_ERROR, "Error initializing system!\n");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  System->Loop ();

  // Cleanup.
  Cleanup ();

  return 0;
}
