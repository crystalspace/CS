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
#include "apps/demosky/demosky.h"
#include "cstool/proctex.h"
#include "cstool/prsky.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "ivaria/conout.h"
#include "imesh/sprite2d.h"
#include "imesh/object.h"
#include "imap/parser.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "ivaria/reporter.h"
#include "igraphic/imageio.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// the global system driver variable
DemoSky *System;

void DemoSky::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (System->object_reg, iReporter);
  if (rep)
    rep->ReportV (severity, "crystalspace.application.demosky", msg, arg);
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

DemoSky::DemoSky ()
{
  view = NULL;
  engine = NULL;
  sky = NULL;
  sky_f = NULL;
  sky_b = NULL;
  sky_l = NULL;
  sky_r = NULL;
  sky_u = NULL;
  sky_d = NULL;
  flock = NULL;
  myG2D = NULL;
  myG3D = NULL;
  LevelLoader = NULL;
  kbd = NULL;
}

DemoSky::~DemoSky ()
{
  delete flock;
  if (view) view->DecRef ();;
  if(font) font->DecRef ();
  if (LevelLoader) LevelLoader->DecRef ();
  if (engine) engine->DecRef ();
  if (myG2D) myG2D->DecRef ();
  if (myG3D) myG3D->DecRef ();
  if (kbd) kbd->DecRef ();

  delete sky;
  delete sky_f;
  delete sky_b;
  delete sky_l;
  delete sky_r;
  delete sky_u;
  delete sky_d;
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = NULL;
  csInitializer::DestroyApplication (object_reg);
}

void DemoSky::SetTexSpace(csProcSkyTexture *skytex, iPolygon3D *poly, 
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
  skytex->SetTextureSpace(texorig, texu-texorig, texv-texorig);
}

static bool DemoSkyEventHandler (iEvent& ev)
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

bool DemoSky::Initialize (int argc, const char* const argv[],
  const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment ();
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg, argc, argv,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_CONSOLEOUT,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::Initialize (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, DemoSkyEventHandler))
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
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    abort ();
  }
  engine->IncRef ();

  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    abort ();
  }
  LevelLoader->IncRef ();

  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    abort ();
  }
  myG3D->IncRef ();

  myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics2D plugin!");
    abort ();
  }
  myG2D->IncRef ();

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    abort ();
  }
  kbd->IncRef();

  // Open the main system. This will open all the previously loaded plug-ins.
  iNativeWindow* nw = myG2D->GetNativeWindow ();
  if (nw) nw->SetTitle ("Crystal Space Procedural Sky Demo");
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
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

  font = myG2D->GetFontServer()->LoadFont(CSFONT_LARGE);

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Crystal Space Procedural Sky Demo.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  sky = new csProcSky();
  sky->SetAnimated(false);
  sky_f = new csProcSkyTexture(sky);
  iMaterialWrapper* imatf = sky_f->Initialize(object_reg, engine, txtmgr, "sky_f");
  sky_b = new csProcSkyTexture(sky);
  iMaterialWrapper* imatb = sky_b->Initialize(object_reg, engine, txtmgr, "sky_b");
  sky_l = new csProcSkyTexture(sky);
  iMaterialWrapper* imatl = sky_l->Initialize(object_reg, engine, txtmgr, "sky_l");
  sky_r = new csProcSkyTexture(sky);
  iMaterialWrapper* imatr = sky_r->Initialize(object_reg, engine, txtmgr, "sky_r");
  sky_u = new csProcSkyTexture(sky);
  iMaterialWrapper* imatu = sky_u->Initialize(object_reg, engine, txtmgr, "sky_u");
  sky_d = new csProcSkyTexture(sky);
  iMaterialWrapper* imatd = sky_d->Initialize(object_reg, engine, txtmgr, "sky_d");

  room = engine->CreateSector ("room");
  iMeshWrapper* walls = engine->CreateSectorWallsMesh (room, "walls");
  iThingState* walls_state = SCF_QUERY_INTERFACE (walls->GetMeshObject (),
  	iThingState);
  iPolygon3D* p;
  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatd);
  float size = 500.0; /// size of the skybox -- around 0,0,0 for now.
  float simi = size; //*255./256.; /// sizeminor
  p->CreateVertex (csVector3 (-size, -simi, size));
  p->CreateVertex (csVector3 (size, -simi, size));
  p->CreateVertex (csVector3 (size, -simi, -size));
  p->CreateVertex (csVector3 (-size, -simi, -size));

  SetTexSpace (sky_d, p, 256, p->GetVertex  (0), p->GetVertex  (1), 
	       2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatu);
  p->CreateVertex (csVector3 (-size, simi, -size));
  p->CreateVertex (csVector3 (size, simi, -size));
  p->CreateVertex (csVector3 (size, simi, size));
  p->CreateVertex (csVector3 (-size, simi, size));

  SetTexSpace (sky_u, p, 256, p->GetVertex  (0), p->GetVertex  (1), 
	       2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatf);
  p->CreateVertex (csVector3 (-size, size, simi));
  p->CreateVertex (csVector3 (size, size, simi));
  p->CreateVertex (csVector3 (size, -size, simi));
  p->CreateVertex (csVector3 (-size, -size, simi));

  SetTexSpace (sky_f, p, 256, p->GetVertex  (0), p->GetVertex  (1), 
	       2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatr);
  p->CreateVertex (csVector3 (simi, size, size));
  p->CreateVertex (csVector3 (simi, size, -size));
  p->CreateVertex (csVector3 (simi, -size, -size));
  p->CreateVertex (csVector3 (simi, -size, size));

  SetTexSpace (sky_r, p, 256, p->GetVertex  (0), p->GetVertex  (1), 
	       2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatl);
  p->CreateVertex (csVector3 (-simi, size, -size));
  p->CreateVertex (csVector3 (-simi, size, size));
  p->CreateVertex (csVector3 (-simi, -size, size));
  p->CreateVertex (csVector3 (-simi, -size, -size));

  SetTexSpace (sky_l, p, 256, p->GetVertex  (0), p->GetVertex  (1), 
	       2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);

  p = walls_state->CreatePolygon ();
  p->SetMaterial (imatb);
  p->CreateVertex (csVector3 (size, size, -simi));
  p->CreateVertex (csVector3 (-size, size, -simi));
  p->CreateVertex (csVector3 (-size, -size, -simi));
  p->CreateVertex (csVector3 (size, -size, -simi));

  SetTexSpace (sky_b, p, 256, p->GetVertex  (0), p->GetVertex  (1), 
	       2.*size, p->GetVertex (3), 2.*size);
  p->GetFlags ().Set(CS_POLY_LIGHTING, 0);
  walls_state->DecRef ();
  walls->DecRef ();

  LevelLoader->LoadTexture ("seagull", "/lib/std/seagull.gif");
  iMaterialWrapper *sg = engine->GetMaterialList ()->FindByName("seagull");
  flock = new Flock(engine, 10, sg, room);

  engine->Prepare ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, myG3D);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  view->SetRectangle (0, 0, myG2D->GetWidth (), myG2D->GetHeight ());

  txtmgr->SetPalette ();

  return true;
}

void DemoSky::SetupFrame ()
{
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  flock->Update(elapsed_time);

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.03 * 20);

  if (kbd->GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (VEC_FORWARD * 4.0f * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (VEC_BACKWARD * 4.0f * speed);

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();

  // Start drawing 2D graphics.
  if (!myG3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  const char *text = "Press 't' to toggle animation. Escape quits."
    " Arrow keys/pgup/pgdown to move.";
  int txtx = 10;
  int txty = myG2D->GetHeight() - 20;
  myG2D->Write(font, txtx+1, txty+1, myG3D->GetTextureManager()->FindRGB(80,80,80), 
    -1, text);
  myG2D->Write(font, txtx, txty, myG3D->GetTextureManager()->FindRGB(255,255,255),
    -1, text);
}

void DemoSky::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (NULL);
}

bool DemoSky::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevKeyDown) && (Event.Key.Code == 't'))
  {
    /// toggle animation
    sky->SetAnimated( !sky->GetAnimated(), csGetTicks ());
    return true;
  }

  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }

  return false;
}

//--- Flock -----------------------
Flock::Flock(iEngine *engine, int num, iMaterialWrapper *mat, iSector *sector)
{
  printf("Creating flock of %d birds\n", num);
  //  mat->IncRef ();
  nr = num;
  spr = new iMeshWrapper* [nr];
  speed = new csVector3[nr];
  accel = new csVector3[nr];
  int i;
  iMeshFactoryWrapper *fact = engine->CreateMeshFactory(
    "crystalspace.mesh.object.sprite.2d", "BirdFactory");
  iSprite2DFactoryState *state = SCF_QUERY_INTERFACE(fact->GetMeshObjectFactory(),
    iSprite2DFactoryState);
  state->SetMaterialWrapper(mat);
  state->SetLighting(false);

  csVector3 startpos(20,10,20);
  csVector3 pos;

  focus = startpos;
  foc_speed.Set(-5, 0, +5);
  foc_accel.Set(0,0,0);

  for(i=0; i<nr; i++)
  {
    pos = startpos;
    speed[i].Set(0,0,0);
    speed[i].x = (float(rand()+1.)/float(RAND_MAX))*3. - 1.5;
    speed[i].y = (float(rand()+1.)/float(RAND_MAX))*1. - 0.5;
    speed[i].z = (float(rand()+1.)/float(RAND_MAX))*3. - 1.5;
    speed[i] += foc_speed*1.0;
    accel[i].Set(0,0,0);
    pos.x += (float(rand()+1.)/float(RAND_MAX))*20. ;
    pos.z -= (float(rand()+1.)/float(RAND_MAX))*20. ;
    spr[i] = engine->CreateMeshWrapper(fact, "Bird", sector, pos);

    iSprite2DState *sprstate = SCF_QUERY_INTERFACE(spr[i]->GetMeshObject(), 
      iSprite2DState);
    sprstate->GetVertices().SetLimit(4);
    sprstate->GetVertices().SetLength(4);
    sprstate->GetVertices()[0].color_init.Set(1.0,1.0,1.0);
    sprstate->GetVertices()[1].color_init.Set(1.0,1.0,1.0);
    sprstate->GetVertices()[2].color_init.Set(1.0,1.0,1.0);
    sprstate->GetVertices()[3].color_init.Set(1.0,1.0,1.0);

    float sz = 1.0;
    sprstate->GetVertices()[0].pos.Set(-sz, sz);
    sprstate->GetVertices()[0].u = 0.2;
    sprstate->GetVertices()[0].v = 0;
    sprstate->GetVertices()[1].pos.Set(+sz, sz);
    sprstate->GetVertices()[1].u = 0.8;
    sprstate->GetVertices()[1].v = 0;
    sprstate->GetVertices()[2].pos.Set(+sz, -sz);
    sprstate->GetVertices()[2].u = 0.8;
    sprstate->GetVertices()[2].v = 1.0;
    sprstate->GetVertices()[3].pos.Set(-sz, -sz);
    sprstate->GetVertices()[3].u = 0.2;
    sprstate->GetVertices()[3].v = 1.0;
    sprstate->DecRef();

  }
  state->DecRef();
  fact->DecRef ();
}


Flock::~Flock()
{
  int i;
  for(i=0; i<nr; i++)
    spr[i]->DecRef();
  delete[] spr;
}


static void Clamp( float &val, float max)
{
  if(val>max) val=max;
  else if(val<-max) val=-max;
}

void Flock::Update(csTicks elapsed)
{
  float dt = float(elapsed)*0.001; /// delta t in seconds
  /// move focus
  /// physics
  int i;
  csVector3 avg(0,0,0);
  for(i=0; i<nr; i++)
    avg += spr[i]->GetMovable()->GetPosition();
  avg /= nr;

  foc_accel = (-avg)*0.1 - focus*0.1;
  foc_accel.y = 0;
  foc_speed += foc_accel * dt;
  focus += foc_speed * dt;

  /// move each bird -- going towards focus
  for(i=0; i<nr; i++)
  {
    /// aim to focus
    csVector3 want = focus - spr[i]->GetMovable()->GetPosition();
    accel[i] += want * 0.1;
    float maxaccel = 2.5;
    if(accel[i].SquaredNorm() > maxaccel)
    {
      Clamp(accel[i].x, maxaccel);
      Clamp(accel[i].y, maxaccel/2.0);
      Clamp(accel[i].z, maxaccel);
    }
    /// physics
    speed[i] += accel[i] * dt;
    float maxspeed = 10.0;
    if(accel[i].SquaredNorm() > maxspeed)
    {
      Clamp(speed[i].x, maxspeed);
      Clamp(speed[i].y, maxspeed/2.0);
      Clamp(speed[i].z, maxspeed);
    }
    float perturb = 0.1;
    speed[i].x += (float(rand()+1.)/float(RAND_MAX))*perturb - perturb *0.5;
    speed[i].y += (float(rand()+1.)/float(RAND_MAX))*perturb - perturb *0.5;
    speed[i].z += (float(rand()+1.)/float(RAND_MAX))*perturb - perturb *0.5;
    speed[i].z *= 1.0 + (float(rand()+1.)/float(RAND_MAX))*0.2 - 0.1;
    csVector3 move = speed[i] * dt;
    spr[i]->GetMovable()->MovePosition(move);
    spr[i]->GetMovable()->UpdateMove();
  }
}


/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class.
  System = new DemoSky ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
	Cleanup ();
    exit (1);
  }

  // Main loop.
  csInitializer::MainLoop (System->object_reg);

  Cleanup ();

  return 0;
}


