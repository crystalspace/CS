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
#include "cstool/csview.h"
#include "cstool/mdldata.h"
#include "mdltest.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
#include "imesh/crossbld.h"
#include "imesh/sprite3d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "imap/parser.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

void Cleanup ();

iModelData *Simple::ImportModel (iMaterialWrapper *OtherMaterial)
{
  iModelData *Model = new csModelData ();

  iModelDataMaterial *mat = new csModelDataMaterial ();
  mat->SetMaterialWrapper (OtherMaterial);

  iModelDataObject *Object = new csModelDataObject ();
  Model->QueryObject ()->ObjAdd (Object->QueryObject ());
  iModelDataVertices *Vertices = new csModelDataVertices ();
  Object->SetDefaultVertices (Vertices);

  Vertices->AddVertex (csVector3 (-5, -5, -5));
  Vertices->AddVertex (csVector3 (-5, -5, +5));
  Vertices->AddVertex (csVector3 (+5, -5, +5));
  Vertices->AddVertex (csVector3 (+5, -5, -5));
  Vertices->AddVertex (csVector3 (-5, +5, -5));
  Vertices->AddVertex (csVector3 (-5, +5, +5));
  Vertices->AddVertex (csVector3 (+5, +5, +5));
  Vertices->AddVertex (csVector3 (+5, +5, -5));

  Vertices->AddNormal (csVector3 (1, 0, 0));
  Vertices->AddNormal (csVector3 (-1, 0, 0));
  Vertices->AddNormal (csVector3 (0, 1, 0));
  Vertices->AddNormal (csVector3 (0, -1, 0));
  Vertices->AddNormal (csVector3 (0, 0, 1));
  Vertices->AddNormal (csVector3 (0, 0, -1));

  Vertices->AddColor (csColor (1, 1, 1));

  Vertices->AddTexel (csVector2 (0, 0));
  Vertices->AddTexel (csVector2 (0, 5));
  Vertices->AddTexel (csVector2 (5, 5));
  Vertices->AddTexel (csVector2 (5, 0));

  iModelDataPolygon *Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (0, 3, 0, 0);
  Polygon->AddVertex (1, 3, 0, 1);
  Polygon->AddVertex (2, 3, 0, 2);
  Polygon->AddVertex (3, 3, 0, 3);
  Polygon->SetMaterial (mat);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (1, 1, 0, 0);
  Polygon->AddVertex (0, 1, 0, 1);
  Polygon->AddVertex (4, 1, 0, 2);
  Polygon->AddVertex (5, 1, 0, 3);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (2, 4, 0, 0);
  Polygon->AddVertex (1, 4, 0, 1);
  Polygon->AddVertex (5, 4, 0, 2);
  Polygon->AddVertex (6, 4, 0, 3);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (3, 0, 0, 0);
  Polygon->AddVertex (2, 0, 0, 1);
  Polygon->AddVertex (6, 0, 0, 2);
  Polygon->AddVertex (7, 0, 0, 3);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (0, 5, 0, 0);
  Polygon->AddVertex (3, 5, 0, 1);
  Polygon->AddVertex (7, 5, 0, 2);
  Polygon->AddVertex (4, 5, 0, 3);

  Polygon = new csModelDataPolygon ();
  Object->QueryObject ()->ObjAdd (Polygon->QueryObject ());
  Polygon->AddVertex (7, 2, 0, 0);
  Polygon->AddVertex (6, 2, 0, 1);
  Polygon->AddVertex (5, 2, 0, 2);
  Polygon->AddVertex (4, 2, 0, 3);

  return Model;
}

iMaterialWrapper *Simple::LoadTexture (const char *name, const char *fn)
{
  if (!loader->LoadTexture (name, fn))
  {
    Printf (CS_MSG_FATAL_ERROR, "Error loading texture '%s' !\n", fn);
    Cleanup ();
    exit (1);
  }
  return engine->GetMaterialList ()->FindByName (name);
}

//-----------------------------------------------------------------------------

// The global system driver
Simple *System;

Simple::Simple ()
{
  view = NULL;
  engine = NULL;
  loader = NULL;
  g3d = NULL;
}

Simple::~Simple ()
{
  if (view) view->DecRef ();
  if (engine) engine->DecRef ();
  if (loader) loader->DecRef();
  if (g3d) g3d->DecRef ();
  if (crossbuilder) crossbuilder->DecRef ();
}

void Cleanup ()
{
  System->ConsoleOut ("Cleaning up...\n");
  delete System;
}

bool Simple::Initialize (int argc, const char* const argv[],
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

  loader = CS_QUERY_PLUGIN_ID (this, CS_FUNCID_LVLLOADER, iLoader);
  if (!loader)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iLoader plugin!\n");
    abort ();
  }

  g3d = CS_QUERY_PLUGIN_ID (this, CS_FUNCID_VIDEO, iGraphics3D);
  if (!g3d)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iGraphics3D plugin!\n");
    abort ();
  }

  crossbuilder = CS_QUERY_PLUGIN_ID (this, "CrossBuilder", iCrossBuilder);
  if (!crossbuilder)
  {
    Printf (CS_MSG_FATAL_ERROR, "No iCrossBuilder plugin!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("Simple Crystal Space Application"))
  {
    Printf (CS_MSG_FATAL_ERROR, "Error opening system!\n");
    Cleanup ();
    exit (1);
  }

  // Setup the texture manager
  iTextureManager* txtmgr = G3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // Initialize the texture manager
  txtmgr->ResetPalette ();

  Printf (CS_MSG_INITIALIZATION,
    "Simple Crystal Space Application version 0.1.\n");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Printf (CS_MSG_INITIALIZATION, "Creating world!...\n");

  iMaterialWrapper* tm = LoadTexture ("stone", "/lib/std/stone4.gif");
  iMaterialWrapper* tm2 = LoadTexture ("wood", "/lib/std/andrew_wood.gif");

  room = engine->CreateSector ("room");

  iMeshObjectType *ThingType = engine->GetThingType ();
  iMeshObjectFactory *ThingFactory = ThingType->NewFactory ();
  iMeshFactoryWrapper *SpriteFactory = engine->CreateMeshFactory (
    "crystalspace.mesh.object.sprite.3d", "SpriteFactory");

  iModelData *Model = ImportModel (tm2);
  iThingState *fState =
	SCF_QUERY_INTERFACE (ThingFactory, iThingState);
  iSprite3DFactoryState *sState = SCF_QUERY_INTERFACE (
	SpriteFactory->GetMeshObjectFactory (), iSprite3DFactoryState);
  sState->SetMaterialWrapper (tm);
  crossbuilder->BuildThing (Model, fState, tm);
  crossbuilder->BuildSpriteFactory (Model, sState);
  fState->DecRef ();
  sState->DecRef ();
  Model->DecRef ();

  iMeshObject *ThingObject = ThingFactory->NewInstance ();
  iMeshWrapper *ThingWrapper = engine->CreateMeshObject (ThingObject, "thing");
  iMeshWrapper *SpriteWrapper = engine->CreateMeshObject (SpriteFactory, "sprite");

  ThingWrapper->GetMovable ()->SetSector (room);
  ThingWrapper->GetMovable ()->UpdateMove ();
  ThingWrapper->GetFlags().Set (CS_ENTITY_CONVEX);
  ThingWrapper->SetZBufMode (CS_ZBUF_USE);
  ThingWrapper->SetRenderPriority (engine->GetWallRenderPriority ());

/*
  SpriteWrapper->GetMovable ()->SetSector (room);
  SpriteWrapper->GetMovable ()->UpdateMove ();
  SpriteWrapper->GetFlags().Set (CS_ENTITY_CONVEX);
  SpriteWrapper->SetZBufMode (CS_ZBUF_USE);
  SpriteWrapper->SetRenderPriority (engine->GetWallRenderPriority ());

  iSprite3DState *sprState = SCF_QUERY_INTERFACE (SpriteWrapper->GetMeshObject (),
    iSprite3DState);
  sprState->SetBaseColor (csColor (1, 1, 1));
  sprState->SetLighting (false);
  sprState->DecRef ();
*/

  engine->SetAmbientLight (csColor (0.5, 0.5, 0.5));

  engine->Prepare ();
  Printf (CS_MSG_INITIALIZATION, "Created.\n");

  view = new csView (engine, g3d);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -3));
  view->SetRectangle (0, 0, FrameWidth, FrameHeight);

  txtmgr->SetPalette ();
  return true;
}

bool Simple::HandleEvent (iEvent& Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if (Event.Type == csevKeyDown && Event.Key.Code == CSKEY_ESC)
  {
    Shutdown = true;
    return true;
  }

  return false;
}

void Simple::NextFrame ()
{
  superclass::NextFrame ();
  // First get elapsed time from the system driver.
  csTime elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);
  
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * 2;

  iCamera* c = view->GetCamera();
  if (GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
  if (GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
  if (GetKeyState (CSKEY_PGUP))
    c->GetTransform ().RotateThis (VEC_TILT_UP, speed);
  if (GetKeyState (CSKEY_PGDN))
    c->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);
  if (GetKeyState (CSKEY_UP))
    c->Move (VEC_FORWARD * 4 * speed, false);
  if (GetKeyState (CSKEY_DOWN))
    c->Move (VEC_BACKWARD * 4 * speed, false);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (
      engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS | CSDRAW_CLEARSCREEN |
        CSDRAW_CLEARZBUFFER))
      return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Drawing code ends here.
  g3d->FinishDraw ();
  // Display the final output.
  g3d->Print (NULL);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new Simple ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  //@@@ WHY IS THE FONTSERVER NEEDED FOR OPENGL AND NOT FOR SOFTWARE???
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.engine.3d:Engine");
  System->RequestPlugin ("crystalspace.level.loader:LevelLoader");
  System->RequestPlugin ("crystalspace.mesh.crossbuilder:CrossBuilder");

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
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
