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
#include "demo.h"
#include "demoseq.h"
#include "csutil/cscolor.h"
#include "csgeom/path.h"
#include "csfx/csfxscr.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/polygon.h"
#include "iengine/thing.h"
#include "iengine/light.h"
#include "iengine/view.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/movable.h"
#include "iengine/halo.h"
#include "imesh/sprite3d.h"
#include "imesh/ball.h"
#include "imesh/surf.h"
#include "imesh/object.h"
#include "imap/reader.h"
#include "igraphic/loader.h"
#include "qsqrt.h"

//-----------------------------------------------------------------------------

Demo::Demo ()
{
  engine = NULL;
  seqmgr = NULL;
  message[0] = 0;
}

Demo::~Demo ()
{
  if (engine)
    engine->DecRef ();
  delete seqmgr;
}

void cleanup ()
{
  System->console_out ("Cleaning up...\n");
  delete System;
}

void Demo::LoadFactory (const char* factname, const char* filename,
	const char* classId, iLoaderPlugIn* plug)
{
  iMeshFactoryWrapper* factwrap = engine->CreateMeshFactory (classId,
  	factname);
  iDataBuffer* databuf = System->VFS->ReadFile (filename);
  if (!databuf || !databuf->GetSize ())
  {
    if (databuf) databuf->DecRef ();
    Printf (MSG_FATAL_ERROR, "Could not open file '%s' on VFS!\n", filename);
    exit (0);
  }
  char* buf = **databuf;
  iBase* mof = plug->Parse (buf, engine, factwrap);
  databuf->DecRef ();
  if (!mof)
  {
    Printf (MSG_FATAL_ERROR,
    	"There was an error loading sprite factory from file '%s'!\n",
	filename);
    exit (0);
  }
  factwrap->SetMeshObjectFactory ((iMeshObjectFactory*)mof);
}

void Demo::SetupFactories ()
{
  iMeshFactoryWrapper* fact;
  fact = engine->CreateMeshFactory ("crystalspace.mesh.object.ball",
  	"ball_factory");
  if (!fact)
  {
    Printf (MSG_FATAL_ERROR, "Could not open ball plugin!\n");
    exit (0);
  }

  fact = engine->CreateMeshFactory ("crystalspace.mesh.object.surface",
  	"surf_factory");
  if (!fact)
  {
    Printf (MSG_FATAL_ERROR, "Could not open surface plugin!\n");
    exit (0);
  }

  iLoaderPlugIn* plug;
  plug = LOAD_PLUGIN (this, "crystalspace.mesh.loader.factory.sprite.3d",
  	"MeshLdr", iLoaderPlugIn);
  if (!plug)
  {
    Printf (MSG_FATAL_ERROR, "Could not open sprite 3d factory loader!\n");
    exit (0);
  }
  LoadFactory ("fighter", "/data/demo/fighter", "crystalspace.mesh.object.sprite.3d",
	plug);
  plug->DecRef ();
}

void Demo::LoadMaterial (const char* matname, const char* filename)
{
  iTextureWrapper* txt = engine->CreateTexture (matname, filename,
  	NULL, CS_TEXTURE_3D);
  engine->CreateMaterial (matname, txt);
}

void Demo::SetupMaterials ()
{
  //LoadMaterial ("flare_center", "/lib/std/snow.jpg");
  //LoadMaterial ("flare_spark", "/lib/std/spark.png");
  LoadMaterial ("flare_center", "/lib/stdtex/flare_center.jpg");
  LoadMaterial ("flare_spark1", "/lib/stdtex/flare_rbow.jpg");
  LoadMaterial ("flare_spark2", "/lib/stdtex/flare_purp.jpg");
  LoadMaterial ("flare_spark3", "/lib/stdtex/flare_picir.jpg");
  LoadMaterial ("flare_spark4", "/lib/stdtex/flare_grcir.jpg");
  LoadMaterial ("flare_spark5", "/lib/stdtex/flare_pink.jpg");
  LoadMaterial ("stone", "/lib/std/stone4.gif");
  LoadMaterial ("sun", "/lib/std/white.gif");
  LoadMaterial ("jupiter", "/data/demo/jup0vtt2.jpg");
  LoadMaterial ("saturn", "/data/demo/Saturn.jpg");
  LoadMaterial ("earth", "/data/demo/Earth.jpg");
  LoadMaterial ("earthclouds", "/data/demo/earthclouds.jpg");
  LoadMaterial ("nebula_b", "/data/demo/nebula_b.png");
  LoadMaterial ("nebula_d", "/data/demo/nebula_d.png");
  LoadMaterial ("nebula_f", "/data/demo/nebula_f.png");
  LoadMaterial ("nebula_l", "/data/demo/nebula_l.png");
  LoadMaterial ("nebula_r", "/data/demo/nebula_r.png");
  LoadMaterial ("nebula_u", "/data/demo/nebula_u.png");
  LoadMaterial ("stars", "/data/demo/stars.png");
}

static void SetTexSpace (iPolygon3D* poly, 
  int size, const csVector3& orig, const csVector3& upt, float ulen, 
  const csVector3& vpt, float vlen)
{
  csVector3 texorig = orig;
  csVector3 texu = upt;
  float texulen = ulen;
  csVector3 texv = vpt;
  float texvlen = vlen;
  /// copied, now adjust
  csVector3 uvector = upt - orig;
  csVector3 vvector = vpt - orig;
  /// to have 1 pixel going over the edges.
  texorig -= uvector / float(size);
  texorig -= vvector / float(size);
  texu += uvector / float(size);
  texv += vvector / float(size);
  texulen += ulen * 2. / float(size);
  texvlen += vlen * 2. / float(size);
  poly->SetTextureSpace (texorig, texu, texulen, texv, texvlen);
}

void Demo::SetupSector ()
{
  room = engine->CreateSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  walls->GetFlags ().Set (CS_ENTITY_CAMERA);
  walls->SetRenderPriority (engine->GetRenderPriority ("starLevel1"));
  walls->SetZBufMode (CS_ZBUF_NONE);
  iThingState* walls_state = QUERY_INTERFACE (walls->GetMeshObject (),
      iThingState);

  float size = 500.0; /// Size of the skybox -- around 0,0,0 for now.
  iPolygon3D* p;
  p = walls_state->CreatePolygon ("d");
  p->SetMaterial (engine->FindMaterial ("nebula_d"));
  p->CreateVertex (csVector3 (-size, -size, size));
  p->CreateVertex (csVector3 (size, -size, size));
  p->CreateVertex (csVector3 (size, -size, -size));
  p->CreateVertex (csVector3 (-size, -size, -size));
  SetTexSpace (p, 256, p->GetVertex (2), p->GetVertex (3),
  	2.*size, p->GetVertex (1), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("u");
  p->SetMaterial (engine->FindMaterial ("nebula_u"));
  p->CreateVertex (csVector3 (-size, size, -size));
  p->CreateVertex (csVector3 (size, size, -size));
  p->CreateVertex (csVector3 (size, size, size));
  p->CreateVertex (csVector3 (-size, size, size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
  	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("f");
  p->SetMaterial (engine->FindMaterial ("nebula_f"));
  p->CreateVertex (csVector3 (-size, size, size));
  p->CreateVertex (csVector3 (size, size, size));
  p->CreateVertex (csVector3 (size, -size, size));
  p->CreateVertex (csVector3 (-size, -size, size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("r");
  p->SetMaterial (engine->FindMaterial ("nebula_r"));
  p->CreateVertex (csVector3 (size, size, size));
  p->CreateVertex (csVector3 (size, size, -size));
  p->CreateVertex (csVector3 (size, -size, -size));
  p->CreateVertex (csVector3 (size, -size, size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
  	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("l");
  p->SetMaterial (engine->FindMaterial ("nebula_l"));
  p->CreateVertex (csVector3 (-size, size, -size));
  p->CreateVertex (csVector3 (-size, size, size));
  p->CreateVertex (csVector3 (-size, -size, size));
  p->CreateVertex (csVector3 (-size, -size, -size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
  	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ("b");
  p->SetMaterial (engine->FindMaterial ("nebula_b"));
  p->CreateVertex (csVector3 (size, size, -size));
  p->CreateVertex (csVector3 (-size, size, -size));
  p->CreateVertex (csVector3 (-size, -size, -size));
  p->CreateVertex (csVector3 (size, -size, -size));
  SetTexSpace (p, 256, p->GetVertex (0), p->GetVertex (1),
	2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set (CS_POLY_LIGHTING, 0);

  walls_state->DecRef ();

  // Create the stars.
  iMeshWrapper* stars;
  iSurfaceState* ss;

  // Create the stars.
  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsF",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsB",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csYRotMatrix3 (M_PI), csVector3 (0)));

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsR",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csYRotMatrix3 (M_PI/2), csVector3 (0)));

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsL",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csYRotMatrix3 (-M_PI/2), csVector3 (0)));

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsU",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csXRotMatrix3 (M_PI/2), csVector3 (0)));

  stars = engine->CreateMeshObject (
  	engine->FindMeshFactory ("surf_factory"), "StarsD",
  	room, csVector3 (0, 0, 0));
  stars->GetFlags ().Set (CS_ENTITY_CAMERA);
  stars->SetRenderPriority (engine->GetRenderPriority ("starLevel2"));
  stars->SetZBufMode (CS_ZBUF_NONE);
  ss = QUERY_INTERFACE (stars->GetMeshObject (), iSurfaceState);
  ss->SetTopLeftCorner (csVector3 (-100, -100, 100));
  ss->SetScale (200, 200);
  ss->SetResolution (8, 8);
  ss->SetMaterialWrapper (engine->FindMaterial ("stars"));
  ss->SetLighting (false);
  ss->SetMixMode (CS_FX_ADD);
  ss->DecRef ();
  stars->HardTransform (csTransform (csXRotMatrix3 (-M_PI/2), csVector3 (0)));

  iStatLight* light;
  light = engine->CreateLight (csVector3 (-500, 300, 900), 1000000,
  	csColor (1, 1, 1), false);
  iLight* il = QUERY_INTERFACE (light, iLight);
  iFlareHalo* flare = il->CreateFlareHalo ();
  iMaterialWrapper* ifmc = engine->FindMaterial ("flare_center");
  iMaterialWrapper* ifm1 = engine->FindMaterial ("flare_spark1");
  iMaterialWrapper* ifm2 = engine->FindMaterial ("flare_spark2");
  iMaterialWrapper* ifm3 = engine->FindMaterial ("flare_spark3");
  iMaterialWrapper* ifm4 = engine->FindMaterial ("flare_spark4");
  iMaterialWrapper* ifm5 = engine->FindMaterial ("flare_spark5");
  flare->AddComponent (0.0, 0.5, 0.5, CS_FX_ADD, ifmc); // pos, w, h, mixmode
  flare->AddComponent (.01, 1.0, 1.0, CS_FX_ADD, ifm2);
  flare->AddComponent (0.3, 0.1, 0.1, CS_FX_ADD, ifm3);
  flare->AddComponent (0.6, 0.4, 0.4, CS_FX_ADD, ifm4);
  flare->AddComponent (0.8, .05, .05, CS_FX_ADD, ifm5);
  flare->AddComponent (1.0, 0.7, 0.7, CS_FX_ADD, ifm1);
  flare->AddComponent (1.3, 0.1, 0.1, CS_FX_ADD, ifm3);
  flare->AddComponent (1.5, 0.3, 0.3, CS_FX_ADD, ifm4);
  flare->AddComponent (1.8, 0.1, 0.1, CS_FX_ADD, ifm5);
  flare->AddComponent (2.0, 0.5, 0.5, CS_FX_ADD, ifm2);
  flare->AddComponent (2.1, .15, .15, CS_FX_ADD, ifm3);
  il->DecRef ();
  room->AddLight (light);
}

void Demo::SetupObjects ()
{
  iBallState* bs;

  // Create the sun.
  iMeshWrapper* sun = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Sun",
  	NULL, csVector3 (0));
  sun->SetRenderPriority (engine->GetRenderPriority ("object"));
  sun->SetZBufMode (CS_ZBUF_USE);
  bs = QUERY_INTERFACE (sun->GetMeshObject (), iBallState);
  bs->SetRadius (500, 500, 500);
  bs->SetMaterialWrapper (engine->FindMaterial ("sun"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (6);
  bs->SetLighting (false);
  bs->SetColor (csColor (1., 1., 1.));
  bs->DecRef ();

  // Create saturn.
  iMeshWrapper* sat = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Saturn",
  	NULL, csVector3 (0));
  sat->SetRenderPriority (engine->GetRenderPriority ("object"));
  sat->SetZBufMode (CS_ZBUF_USE);
  bs = QUERY_INTERFACE (sat->GetMeshObject (), iBallState);
  bs->SetRadius (100, 100, 100);
  bs->SetMaterialWrapper (engine->FindMaterial ("saturn"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (16);
  sat->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  bs->DecRef ();

  // Create jupiter.
  iMeshWrapper* jup = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Jupiter",
  	NULL, csVector3 (0));
  jup->SetRenderPriority (engine->GetRenderPriority ("object"));
  jup->SetZBufMode (CS_ZBUF_USE);
  bs = QUERY_INTERFACE (jup->GetMeshObject (), iBallState);
  bs->SetRadius (100, 100, 100);
  bs->SetMaterialWrapper (engine->FindMaterial ("jupiter"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (16);
  jup->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  bs->DecRef ();

  // Create the earth.
  iMeshWrapper* earth = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Earth",
  	NULL, csVector3 (0));
  earth->SetRenderPriority (engine->GetRenderPriority ("object"));
  earth->SetZBufMode (CS_ZBUF_USE);
  bs = QUERY_INTERFACE (earth->GetMeshObject (), iBallState);
  bs->SetRadius (50, 50, 50);
  bs->SetMaterialWrapper (engine->FindMaterial ("earth"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (16);
  earth->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  bs->DecRef ();

  // Create the clouds for earth.
  iMeshWrapper* clouds = engine->CreateMeshObject (
  	engine->FindMeshFactory ("ball_factory"), "Clouds",
  	NULL, csVector3 (0));
  clouds->SetRenderPriority (engine->GetRenderPriority ("alpha"));
  clouds->SetZBufMode (CS_ZBUF_TEST);
  bs = QUERY_INTERFACE (clouds->GetMeshObject (), iBallState);
  bs->SetRadius (55, 55, 55);
  bs->SetMaterialWrapper (engine->FindMaterial ("earthclouds"));
  bs->SetCylindricalMapping (true);
  bs->SetRimVertices (16);
  bs->SetMixMode (CS_FX_ADD);
  clouds->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  bs->DecRef ();
 
  // Create fighters.
  iMeshWrapper* fighter;
  fighter = engine->CreateMeshObject (
  	engine->FindMeshFactory ("fighter"), "Fighter1",
  	NULL, csVector3 (0));
  fighter->SetRenderPriority (engine->GetRenderPriority ("object"));
  fighter->SetZBufMode (CS_ZBUF_USE);
  fighter->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);

  fighter = engine->CreateMeshObject (
  	engine->FindMeshFactory ("fighter"), "Fighter2",
  	NULL, csVector3 (0));
  fighter->SetRenderPriority (engine->GetRenderPriority ("object"));
  fighter->SetZBufMode (CS_ZBUF_USE);
  fighter->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
}

bool Demo::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  if (!superclass::Initialize (argc, argv, iConfigName))
    return false;

  // Load the engine plugin.
  engine =
    LOAD_PLUGIN (this, "crystalspace.engine.core", CS_FUNCID_ENGINE, iEngine);
  if (!engine)
  {
    Printf (MSG_FATAL_ERROR, "No engine!\n");
    abort ();
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Open ("The Crystal Space Demo."))
  {
    Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    exit (1);
  }

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

  font = G2D->GetFontServer ()->LoadFont (CSFONT_LARGE);

  // Initialize the console
  if (System->Console != NULL)
    // Don't let messages before this one appear
    System->Console->Clear ();

  // Some commercials...
  Printf (MSG_INITIALIZATION,
    "The Crystal Space Demo.\n");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->EnableLightingCache (false);

  // Create our world.
  Printf (MSG_INITIALIZATION, "Creating world!...\n");

  engine->RegisterRenderPriority ("starLevel1", 1);
  engine->RegisterRenderPriority ("starLevel2", 2);
  engine->RegisterRenderPriority ("object", 3);
  engine->RegisterRenderPriority ("alpha", 4);
  SetupMaterials ();
  SetupFactories ();
  SetupSector ();
  seqmgr = new DemoSequenceManager (this);
  SetupObjects ();
  seqmgr->Setup ("/data/demo/sequences");

  engine->Prepare ();

  Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  view = engine->CreateView (G3D);
  view->SetSector (room);
  view->GetCamera ()->SetPosition (csVector3 (0, 0, -900));
  view->GetCamera ()->GetTransform ().RotateThis (csVector3 (0, 1, 0), .8);
  view->SetRectangle (0, 0, FrameWidth, FrameHeight);

  txtmgr->SetPalette ();
  col_red = txtmgr->FindRGB (255, 0, 0);
  col_blue = txtmgr->FindRGB (0, 0, 255);
  col_white = txtmgr->FindRGB (255, 255, 255);
  col_gray = txtmgr->FindRGB (128, 128, 128);
  col_black = txtmgr->FindRGB (0, 0, 0);
  col_yellow = txtmgr->FindRGB (255, 255, 0);
  col_cyan = txtmgr->FindRGB (0, 255, 255);
  col_green = txtmgr->FindRGB (0, 255, 0);

  return true;
}

#define MAP_OFF 0
#define MAP_OVERLAY 1
#define MAP_EDIT 2
#define MAP_EDIT_FORWARD 3
static int map_enabled = MAP_OFF;
static csVector2 map_tl (-1000, 1000);
static csVector2 map_br (1000, -1000);
static int map_selpoint = 0;
static char map_selpath[255] = { 0 };

void Demo::GfxWrite (int x, int y, int fg, int bg, char *str, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  G2D->Write (font, x, y, fg, bg, buf);
}

void Demo::FileWrite (iFile* file, char *str, ...)
{
  va_list arg;
  char buf[256];

  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  file->Write (buf, strlen (buf));
}

void Demo::ShowMessage (const char* msg, ...)
{
  message_error = false;
  va_list arg;
  va_start (arg, msg);
  vsprintf (message, msg, arg);
  va_end (arg);
  message_timer = GetTime () + 1500;
}

void Demo::ShowError (const char* msg, ...)
{
  message_error = true;
  va_list arg;
  va_start (arg, msg);
  vsprintf (message, msg, arg);
  va_end (arg);
  message_timer = GetTime () + 1500;
}

void Demo::NextFrame ()
{
  superclass::NextFrame ();
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);
 
  // Now rotate the camera according to keyboard state
  csReversibleTransform& camtrans = view->GetCamera ()->GetTransform ();
  if (map_enabled < MAP_EDIT)
  {
    float speed = (elapsed_time / 1000.) * (0.03 * 20);
    if (GetKeyState (CSKEY_RIGHT))
      camtrans.RotateThis (VEC_ROT_RIGHT, speed);
    if (GetKeyState (CSKEY_LEFT))
      camtrans.RotateThis (VEC_ROT_LEFT, speed);
    if (GetKeyState (CSKEY_PGUP))
      camtrans.RotateThis (VEC_TILT_UP, speed);
    if (GetKeyState (CSKEY_PGDN))
      camtrans.RotateThis (VEC_TILT_DOWN, speed);
    if (GetKeyState (CSKEY_UP))
      view->GetCamera ()->Move (VEC_FORWARD * 400.0f * speed);
    if (GetKeyState (CSKEY_DOWN))
      view->GetCamera ()->Move (VEC_BACKWARD * 400.0f * speed);
  }

  if (map_enabled < MAP_EDIT_FORWARD)
    seqmgr->ControlPaths (view->GetCamera (), current_time);

  if (map_enabled == MAP_EDIT_FORWARD)
  {
    csNamedPath* np = seqmgr->GetSelectedPath (map_selpath);
    if (np)
    {
      float r = np->GetTimeValue (map_selpoint);
      np->Calculate (r);
      csVector3 pos, up, forward;
      np->GetInterpolatedPosition (pos);
      np->GetInterpolatedUp (up);
      np->GetInterpolatedForward (forward);
      view->GetCamera ()->SetPosition (pos);
      view->GetCamera ()->GetTransform ().LookAt (forward.Unit (), up.Unit ());
    }
  }

  // Tell 3D driver we're going to display 3D things.
  if (!G3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS
  	| CSDRAW_CLEARZBUFFER))
    return;

  if (map_enabled != MAP_EDIT)
  {
    view->Draw ();
    seqmgr->Draw3DEffects (G3D, current_time);
    if (map_enabled == MAP_EDIT_FORWARD)
      csfxFadeToColor (G3D, .3, csColor (0, 0, 1));
  }

  // Start drawing 2D graphics.
  if (!System->G3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  if (map_enabled == MAP_EDIT)
    G2D->Clear (0);
  else if (map_enabled < MAP_EDIT)
    seqmgr->Draw2DEffects (G2D, current_time);
  if (map_enabled >= MAP_OVERLAY)
    seqmgr->DebugDrawPaths (current_time, map_selpath,
    	map_tl, map_br, map_selpoint);

  int fw, fh;
  font->GetMaxSize (fw, fh);
  int tx = 10;
  int ty = G2D->GetHeight ()-fh-3;
  char messageLine[100];
  messageLine[0] = 0;
  switch (map_enabled)
  {
    case MAP_OFF:
      if (seqmgr->IsSuspended ())
        GfxWrite (tx, ty, col_black, col_white, "[PAUSED]");
      break;
    case MAP_OVERLAY:
      GfxWrite (tx, ty, col_black, col_white, "%sOverlay (%s)",
        seqmgr->IsSuspended () ? "[PAUSED] " : "",
      	map_selpath);
      break;
    case MAP_EDIT:
      GfxWrite (tx, ty, col_black, col_white, "Edit (%s)",
      	map_selpath);
      break;
    case MAP_EDIT_FORWARD:
      GfxWrite (tx, ty, col_black, col_white, "Forward/Up (%s)",
      	map_selpath);
      break;
  }

  if (message[0])
  {
    GfxWrite (10, 10, col_black, message_error ? col_red : col_white, message);
    if (current_time > message_timer) message[0] = 0;
  }

  // Drawing code ends here.
  G3D->FinishDraw ();
  // Print the final output.
  G3D->Print (NULL);
}

bool Demo::HandleEvent (iEvent &Event)
{
  if (superclass::HandleEvent (Event))
    return true;

  if (Event.Type == csevKeyDown)
  {
    cs_time elapsed_time, current_time;
    GetElapsedTime (elapsed_time, current_time);
    bool shift = (Event.Key.Modifiers & CSMASK_SHIFT) != 0;
    bool alt = (Event.Key.Modifiers & CSMASK_ALT) != 0;
    bool ctrl = (Event.Key.Modifiers & CSMASK_CTRL) != 0;

    if (map_enabled == MAP_EDIT_FORWARD)
    {
      //==============================
      // Handle keys in path_edit_forward mode.
      //==============================
      csNamedPath* np = seqmgr->GetSelectedPath (map_selpath);
      if (np)
      {
        float dx = map_br.x - map_tl.x;
        float speed;
        if (shift) speed = dx/20.;
        else if (ctrl) speed = dx/600.;
        else speed = dx/100.;
        if (Event.Key.Code == CSKEY_UP)
        {
          csVector3 v;
	  np->GetPositionVector (map_selpoint, v);
          v.y += speed;
	  np->SetPositionVector (map_selpoint, v);
          return true;
        }
        if (Event.Key.Code == CSKEY_DOWN)
        {
          csVector3 v;
	  np->GetPositionVector (map_selpoint, v);
          v.y -= speed;
	  np->SetPositionVector (map_selpoint, v);
          return true;
        }
        if (Event.Key.Code == CSKEY_LEFT)
        {
          csVector3 up, forward;
	  np->GetUpVector (map_selpoint, up);
	  np->GetForwardVector (map_selpoint, forward);
	  csReversibleTransform trans = view->GetCamera ()->GetTransform ();
          trans.LookAt (forward.Unit (), up.Unit ());
	  trans.RotateThis (csVector3 (0, 0, 1), -.1);
	  np->SetUpVector (map_selpoint,
		  trans.This2Other (csVector3 (0, 1, 0)) - trans.GetOrigin ());
          return true;
        }
        if (Event.Key.Code == CSKEY_RIGHT)
        {
          csVector3 up, forward;
	  np->GetUpVector (map_selpoint, up);
	  np->GetForwardVector (map_selpoint, forward);
	  csReversibleTransform trans = view->GetCamera ()->GetTransform ();
          trans.LookAt (forward.Unit (), up.Unit ());
	  trans.RotateThis (csVector3 (0, 0, 1), .1);
	  np->SetUpVector (map_selpoint,
		  trans.This2Other (csVector3 (0, 1, 0)) - trans.GetOrigin ());
          return true;
        }
      }
      switch (Event.Key.Char)
      {
        case 'c':
          map_enabled = MAP_EDIT;
	  return true;
      }
    }
    else if (map_enabled == MAP_EDIT)
    {
      //==============================
      // Handle keys in path editing mode.
      //==============================
      csNamedPath* np = seqmgr->GetSelectedPath (map_selpath);
      float dx = map_br.x - map_tl.x;
      float dy = map_br.y - map_tl.y;
      float speed;
      if (shift) speed = dx/20.;
      else if (ctrl) speed = dx/600.;
      else speed = dx/100.;
      if (np)
      {
        if (Event.Key.Code == CSKEY_UP)
        {
	  if (alt)
	  {
	    map_tl.y -= dy/10.;
	    map_br.y -= dy/10.;
	  }
	  else
	  {
            csVector3 v;
	    np->GetPositionVector (map_selpoint, v);
            v.z += speed;
	    np->SetPositionVector (map_selpoint, v);
	  }
          return true;
        }
        if (Event.Key.Code == CSKEY_DOWN)
        {
	  if (alt)
	  {
	    map_tl.y += dy/10.;
	    map_br.y += dy/10.;
	  }
	  else
	  {
            csVector3 v;
	    np->GetPositionVector (map_selpoint, v);
            v.z -= speed;
	    np->SetPositionVector (map_selpoint, v);
	  }
          return true;
        }
        if (Event.Key.Code == CSKEY_LEFT)
        {
	  if (alt)
	  {
	    map_tl.x -= dx/10.;
	    map_br.x -= dx/10.;
	  }
	  else
	  {
            csVector3 v;
	    np->GetPositionVector (map_selpoint, v);
            v.x -= speed;
	    np->SetPositionVector (map_selpoint, v);
	  }
          return true;
        }
        if (Event.Key.Code == CSKEY_RIGHT)
        {
	  if (alt)
	  {
	    map_tl.x += dx/10.;
	    map_br.x += dx/10.;
	  }
	  else
	  {
            csVector3 v;
	    np->GetPositionVector (map_selpoint, v);
            v.x += speed;
	    np->SetPositionVector (map_selpoint, v);
	  }
          return true;
        }
      }
      switch (Event.Key.Char)
      {
	case 'm':
          map_enabled = MAP_OFF;
          return true;
	case 's':
	  if (np)
	  {
	    char buf[200], backup[200];
	    strcpy (buf, "/data/demo/path_"); strcat (buf, np->GetName ());
	    // Make a backup of the original file.
	    strcpy (backup, buf);
	    strcat (backup, ".bak");
	    VFS->DeleteFile (backup);
	    iDataBuffer* dbuf = VFS->ReadFile (buf);
	    if (dbuf)
	    {
	      if (dbuf->GetSize ())
	        VFS->WriteFile (backup, **dbuf, dbuf->GetSize ());
	      dbuf->DecRef ();
	    }

	    iFile* fp = VFS->Open (buf, VFS_FILE_WRITE);
	    if (fp)
	    {
	      int i, num = np->GetNumPoints ();
	      FileWrite (fp, "    NUM (%d)\n", num);
	      float* t = np->GetTimeValues ();
	      FileWrite (fp, "    TIMES (%g", t[0]);
	      for (i = 1 ; i < num ; i++)
	        FileWrite (fp, ",%g", t[i]);
	      FileWrite (fp, ")\n");
	      FileWrite (fp, "    POS (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		np->GetPositionVector (i, v);
	        FileWrite (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      FileWrite (fp, "    )\n");
	      FileWrite (fp, "    FORWARD (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		np->GetForwardVector (i, v);
	        FileWrite (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      FileWrite (fp, "    )\n");
	      FileWrite (fp, "    UP (\n");
	      for (i = 0 ; i < num ; i++)
	      {
	        csVector3 v;
		np->GetUpVector (i, v);
	        FileWrite (fp, "      V (%g,%g,%g)\n", v.x, v.y, v.z);
	      }
	      FileWrite (fp, "    )\n");
	      fp->DecRef ();
	      ShowMessage ("Wrote path to file '%s'", buf);
	    }
	    else
	      ShowError ("Error writing to file '%s'!", buf);
	  }
	  break;
        case 'i':
	  if (np)
	  {
	    np->InsertPoint (map_selpoint);
	    map_selpoint++;
	    if (map_selpoint == np->GetNumPoints ()-1)
	    {
	      csVector3 v;
	      np->GetPositionVector (map_selpoint-1, v);
	      np->SetPositionVector (map_selpoint, v);
	      np->GetUpVector (map_selpoint-1, v);
	      np->SetUpVector (map_selpoint, v);
	      np->GetForwardVector (map_selpoint-1, v);
	      np->SetForwardVector (map_selpoint, v);
	      np->SetTimeValue (map_selpoint,
	    	  np->GetTimeValue (map_selpoint-1));
	    }
	    else
	    {
	      csVector3 v1, v2, v;
	      np->GetPositionVector (map_selpoint-1, v1);
	      np->GetPositionVector (map_selpoint+1, v2);
	      np->SetPositionVector (map_selpoint, (v1+v2)/2.);
	      np->GetUpVector (map_selpoint-1, v1);
	      np->GetUpVector (map_selpoint+1, v2);
	      np->SetUpVector (map_selpoint, (v1+v2)/2.);
	      np->GetForwardVector (map_selpoint-1, v1);
	      np->GetForwardVector (map_selpoint+1, v2);
	      np->SetForwardVector (map_selpoint, (v1+v2)/2.);
	      np->SetTimeValue (map_selpoint,
	    	  (np->GetTimeValue (map_selpoint-1)+
		   np->GetTimeValue (map_selpoint+1))/2.);
	    }
	  }
          break;
        case 'd':
	  if (np)
	  {
	    np->RemovePoint (map_selpoint);
	    if (map_selpoint >= np->GetNumPoints ())
	      map_selpoint--;
	  }
	  break;
	case ',':
	  if (np)
	  {
	    if (map_selpoint > 0 && map_selpoint < np->GetNumPoints ()-1)
	    {
	      float t = np->GetTimeValue (map_selpoint);
	      float t1 = np->GetTimeValue (map_selpoint-1);
	      float t2 = np->GetTimeValue (map_selpoint+1);
	      float dt = (t2-t1);
	      if (shift) dt /= 5.;
	      else if (ctrl) dt /= 500.;
	      else dt /= 50.;
	      t -= dt;
	      if (t < t1) t = t1;
	      np->SetTimeValue (map_selpoint, t);
	    }
	  }
	  break;
	case '.':
	  if (np)
	  {
	    if (map_selpoint > 0 && map_selpoint < np->GetNumPoints ()-1)
	    {
	      float t = np->GetTimeValue (map_selpoint);
	      float t1 = np->GetTimeValue (map_selpoint-1);
	      float t2 = np->GetTimeValue (map_selpoint+1);
	      float dt = (t2-t1);
	      if (shift) dt /= 5.;
	      else if (ctrl) dt /= 500.;
	      else dt /= 50.;
	      t += dt;
	      if (t > t2) t = t2;
	      np->SetTimeValue (map_selpoint, t);
	    }
	  }
	  break;
	case '/':
	  if (np && map_selpoint > 0 && map_selpoint < np->GetNumPoints ()-1)
	  {
	    float t1 = np->GetTimeValue (map_selpoint-1);
	    float t2 = np->GetTimeValue (map_selpoint+1);
	    np->SetTimeValue (map_selpoint, (t1+t2)/2.);
	  }
	  break;
	case '?':
	  if (np)
	  {
	    int num = np->GetNumPoints ();
	    float* xv, * yv, * zv;
	    xv = np->GetDimensionValues (0);
	    yv = np->GetDimensionValues (1);
	    zv = np->GetDimensionValues (2);
	    csVector3 v0, v1;
	    // Calculate the total length of the path.
	    float totlen = 0;
	    int i;
	    v0.Set (xv[0], yv[0], zv[0]);
	    for (i = 1 ; i < num ; i++)
	    {
	      v1.Set (xv[i], yv[i], zv[i]);
	      float d = qsqrt (csSquaredDist::PointPoint (v0, v1));
	      totlen += d;
	      v0 = v1;
	    }
	    float list[10000];
	    // Calculate the time value for every path segment,
	    // given the total length of the path.
	    v0.Set (xv[0], yv[0], zv[0]);
	    list[0] = 0;
	    float tot = 0;
	    for (i = 1 ; i < num ; i++)
	    {
	      v1.Set (xv[i], yv[i], zv[i]);
	      float d = qsqrt (csSquaredDist::PointPoint (v0, v1));
	      tot += d;
	      list[i] = tot / totlen;
	      v0 = v1;
	    }
	    np->SetTimeValues (list);
	  }
	  break;
        case '>':
	  if (np)
	  {
            map_selpoint++;
	    if (map_selpoint >= np->GetNumPoints ())
	      map_selpoint = 0;
	  }
	  break;
        case '<':
	  if (np)
	  {
            map_selpoint--;
	    if (map_selpoint < 0)
	      map_selpoint = np->GetNumPoints ()-1;
	  }
	  break;
        case 'c':
	  ShowMessage ("Edit forward and up vectors, press 'c' to exit");
	  map_enabled = MAP_EDIT_FORWARD;
	  break;
        case '+':
	  {
	    float dx = (map_br.x-map_tl.x)/2.;
	    float dy = (map_br.y-map_tl.y)/2.;
	    float cx = (map_br.x+map_tl.x)/2.;
	    float cy = (map_br.y+map_tl.y)/2.;
	    map_tl.x = cx-dx*.9;
	    map_tl.y = cy-dy*.9;
	    map_br.x = cx+dx*.9;
	    map_br.y = cy+dy*.9;
	  }
	  break;
        case '-':
	  {
	    float dx = (map_br.x-map_tl.x)/2.;
	    float dy = (map_br.y-map_tl.y)/2.;
	    float cx = (map_br.x+map_tl.x)/2.;
	    float cy = (map_br.y+map_tl.y)/2.;
	    map_tl.x = cx-dx*1.1;
	    map_tl.y = cy-dy*1.1;
	    map_br.x = cx+dx*1.1;
	    map_br.y = cy+dy*1.1;
	  }
	  break;
        case '=':
	  map_tl.Set (-1000, 1000);
	  map_br.Set (1000, -1000);
	  break;
        case '[':
	  seqmgr->SelectPreviousPath (map_selpath);
	  np = seqmgr->GetSelectedPath (map_selpath);
	  if (np)
	  {
	    if (map_selpoint >= np->GetNumPoints ())
	      map_selpoint = np->GetNumPoints ()-1;
	  }
	  break;
        case ']':
	  seqmgr->SelectNextPath (map_selpath);
	  np = seqmgr->GetSelectedPath (map_selpath);
	  if (np)
	  {
	    if (map_selpoint >= np->GetNumPoints ())
	      map_selpoint = np->GetNumPoints ()-1;
	  }
	  break;
      }
    }
    else
    {
      //==============================
      // Handle keys in demo or overlay mode.
      //==============================
      if (Event.Key.Code == CSKEY_ESC)
      {
        Shutdown = true;
        return true;
      }
      switch (Event.Key.Char)
      {
      	case 'R':
	  ShowMessage ("Restarted sequence manager");
  	  seqmgr->Restart ("/data/demo/sequences");
	  break;
        case 'p':
          if (seqmgr->IsSuspended ()) seqmgr->Resume ();
          else seqmgr->Suspend ();
	  break;
        case '.':
          seqmgr->TimeWarp (100);
	  break;
        case ',':
          seqmgr->TimeWarp (-100);
	  break;
        case '>':
          seqmgr->TimeWarp (3000);
	  break;
        case '<':
          seqmgr->TimeWarp (-3000);
	  break;
        case 'm':
	  map_enabled++;
	  if (map_enabled == MAP_EDIT)
	  {
	    ShowMessage ("Map editing mode, press 'm' to exit");
            seqmgr->Suspend ();
	  }
	  break;
      }
    }
  }
  else if (Event.Type == csevMouseDown)
  {
    if (Event.Mouse.Button == 1)
    {
      csVector2 p (Event.Mouse.x, G2D->GetHeight ()-Event.Mouse.y);
      csVector3 v;
      view->GetCamera ()->InvPerspective (p, 1, v);
      csVector3 vw = view->GetCamera ()->GetTransform ().This2Other (v);
      if (map_enabled == MAP_EDIT_FORWARD)
      {
        csNamedPath* np = seqmgr->GetSelectedPath (map_selpath);
	if (np)
	{
          vw -= view->GetCamera ()->GetTransform ().GetOrigin ();
          np->SetForwardVector (map_selpoint, vw);
	  csVector3 up;
	  np->GetUpVector (map_selpoint, up);
	  up = up % vw;
	  up = - (up % vw);
	  np->SetUpVector (map_selpoint, up);
        }
      }
      else if (map_enabled == MAP_EDIT)
      {
        p.y = Event.Mouse.y;
	int dim = G2D->GetHeight ()-10;
	float dx = (map_br.x-map_tl.x)/2.;
	float dy = (map_br.y-map_tl.y)/2.;
	float cx = map_tl.x + (map_br.x-map_tl.x)*(1-(dim-p.x)/dim);
	float cy = map_tl.y + (map_br.y-map_tl.y)*(1-(dim-p.y)/dim);
	map_tl.x = cx-dx*.9;
	map_tl.y = cy-dy*.9;
	map_br.x = cx+dx*.9;
	map_br.y = cy+dy*.9;
      }
    }
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
  System = new Demo ();

  // We want at least the minimal set of plugins
  System->RequestPlugin ("crystalspace.kernel.vfs:VFS");
  System->RequestPlugin ("crystalspace.font.server.default:FontServer");
  System->RequestPlugin ("crystalspace.graphic.image.io.multiplex:ImageLoader");
  System->RequestPlugin ("crystalspace.graphics3d.software:VideoDriver");
  System->RequestPlugin ("crystalspace.console.output.simple:Console.Output");

  // Initialize the main system. This will load all needed plug-ins
  // and initialize them.
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
