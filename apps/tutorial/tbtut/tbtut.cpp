/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <cssysdef.h>
#include <csutil/sysfunc.h>
#include <csver.h>

#include <csutil/csobject.h>
#include <csutil/cmdhelp.h>
#include <csutil/cspmeter.h>
#include <csutil/cscolor.h>
#include <cstool/initapp.h>
#include <cstool/csview.h>

#include <iutil/objreg.h>
#include <iutil/vfs.h>
#include <iutil/virtclk.h>
#include <iutil/csinput.h>
#include <iutil/eventq.h>
#include <iutil/event.h>
#include <iutil/plugin.h>
#include <iutil/cfgmgr.h>

#include <iengine/engine.h>
#include <iengine/sector.h>
#include <iengine/camera.h>
#include <iengine/campos.h>
#include <iengine/mesh.h>
#include <iengine/material.h>
#include <iengine/light.h>

#include <imesh/object.h>
#include <imesh/thing.h>
#include <imesh/ball.h>

#include <ivideo/graph3d.h>
#include <ivideo/graph2d.h>
#include <ivideo/txtmgr.h>
#include <ivideo/fontserv.h>

#include <igraphic/imageio.h>

#include <isound/loader.h>
#include <isound/handle.h>
#include <isound/source.h>
#include <isound/listener.h>
#include <isound/wrapper.h>
#include <isound/renderer.h>

#include <ivaria/reporter.h>
#include <ivaria/stdrep.h>
#include <ivaria/conout.h>
#include <ivaria/dynamics.h>

#include <imap/parser.h>

#include <imesh/terrbig.h>
#include "csutil/event.h"

#include "tbtut.h"

CS_IMPLEMENT_APPLICATION

TerrBigTut *Sys = 0;

TerrBigTut::TerrBigTut (iObjectRegistry* o) 
{
  object_reg = o;
}

TerrBigTut::~TerrBigTut ()
{
}

void TerrBigTut::Report (int s, const char *m)
{
  csReport (object_reg, s, "TerrBigTut", m);
}

bool TerrBigTut::Initialize () 
{
  if (!csInitializer::RequestPlugins (object_reg,
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_END)) 
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Failed to initialize plugins"); 
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler)) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to create event handler");  
    return false;
  }

  if (csCommandLineHelper::CheckHelp (object_reg)) {
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  if ((vfs = CS_QUERY_REGISTRY (object_reg, iVFS)) == 0) {
    Report (CS_REPORTER_SEVERITY_ERROR, 
	  "Unable to initialize vfs");
	return false;
  }

  if ((vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock)) == 0) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to initialize virtual clock");
    return false;
  }

  if ((engine = CS_QUERY_REGISTRY (object_reg, iEngine)) == 0) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to initialize engine");
    return false;
  }

  if ((loader = CS_QUERY_REGISTRY (object_reg, iLoader)) == 0) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to initialize map loader");
    return false;
  }

  if ((g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D)) == 0) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to initialize graphics");
    return false;
  }

  if ((kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver)) == 0) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to initialize keyboard");
    return false;
  }

  if (!csInitializer::OpenApplication (object_reg)) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Unable to start the application");
    return false;
  }

  engine->SetLightingCacheMode (0); 

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif")) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Error loading 'stone4' texture!\n");
    return false;
  }
  iMaterialWrapper *tm = engine->GetMaterialList()->FindByName ("stone");

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));

  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-128, 0, -128),
  	csVector3 (128, 100, 128));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iMeshFactoryWrapper> factory;
  factory = engine->CreateMeshFactory ("crystalspace.mesh.object.terrbig",
    "terrain_factory");
  if (!factory) {
    Report (CS_REPORTER_SEVERITY_ERROR, "Can't create terrbig mesh factory!");
    return false;
  }
  csRef<iMeshWrapper> mesh;
  mesh = engine->CreateMeshWrapper (factory, "terrain", room, csVector3 (0,0,0));
  csRef<iTerrBigState> terrbigstate;
  terrbigstate = SCF_QUERY_INTERFACE (mesh->GetMeshObject(), iTerrBigState);

  if (!loader->LoadTexture ("heightmap", "/lev/terrain/heightmap.png")) {
    Report (CS_REPORTER_SEVERITY_ERROR,
      "Error loading 'heightmap' texture!\n");
    return false;
  }
  iMaterialWrapper *mat = engine->GetMaterialList()->FindByName ("heightmap");
  terrbigstate->SetMaterialsList (&mat, 1);
  csRef<iDataBuffer> path = vfs->GetRealPath ("/lev/terrain/test.map");
  terrbigstate->LoadHeightMapFile ((char *)path->GetData());
  terrbigstate->SetScaleFactor (csVector3 (1.0, 10, 1.0));

  mesh->SetRenderPriority (engine->GetRenderPriority ("object"));
  mesh->SetZBufMode (CS_ZBUF_USE);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-50, 20, 0), 50,
  	csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (50, 20,  0), 50,
  	csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 20, -50), 50,
  	csColor (0, 1, 0));
  ll->Add (light);

  engine->Prepare ();

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());
  view->GetCamera()->Move (csVector3 (0, 20, 0));
  return true;
}

void TerrBigTut::MainLoop ()
{
  csDefaultRunLoop (object_reg);
}

bool TerrBigTut::SimpleEventHandler (iEvent &e)
{
  return Sys->HandleEvent (e);
}

bool TerrBigTut::HandleEvent (iEvent &e)
{
  if (e.Type == csevBroadcast && e.Command.Code == cscmdProcess) {
    SetupFrame ();
	return true;
  }
  if (e.Type == csevBroadcast && e.Command.Code == cscmdFinalProcess) {
    FinishFrame ();
	return true;
  }
  if ((e.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&e) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&e) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  };

  return false;
}

void TerrBigTut::SetupFrame () 
{
  float elapsed_time = ((float)vc->GetElapsedTicks ())/1000.0;
  float speed = elapsed_time * 0.6;

  iCamera *c = view->GetCamera ();
  if (kbd->GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    c->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    c->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    c->Move (CS_VEC_FORWARD * 10 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    c->Move (CS_VEC_FORWARD * -10 * speed);

  if (g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS)) {
    view->Draw (); 
  }
}

void TerrBigTut::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

int main (int argc, char *argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);

  Sys = new TerrBigTut(object_reg);
  if (Sys->Initialize ()) {
    Sys->MainLoop ();
  }
  delete Sys;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}

