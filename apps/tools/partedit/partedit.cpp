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
#include "cssys/sysfunc.h"
#include "iutil/vfs.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "ivideo/natwin.h"
#include "igraphic/imageio.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "csutil/cmdhelp.h"
#include "qsqrt.h"

#include "imesh/emit.h"
#include "iutil/plugin.h"

#include "partedit.h"
#include "awssink.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to PartEdit
PartEdit *System;

awsSink *s;

PartEdit::PartEdit (iObjectRegistry* object_reg)
{
  PartEdit::object_reg = object_reg;
  width = 0.05;
  part_time = 2000;
  keydown=false;
  value = 0;
}

PartEdit::~PartEdit ()
{
}

void PartEdit::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();
  if(s->GetUpdateState()) {
  
	  s->UpdateLabel("X",1);
	  s->UpdateLabel("Y",2);
	  s->UpdateLabel("Z",3); 
	  s->UpdateLabel("Other",4);

  printf("State: %i\n",s->GetState());
  switch(s->GetState())
  {
    case 0: 
	  s->UpdateInput(0.0,1);
	  s->UpdateInput(0.0,2);
	  s->UpdateInput(0.0,3);
	  s->UpdateInput(0.0,4);
	  break;
    case 1: 
	  s->UpdateInput(s->GetValueX(0.001,-500),1);
	  s->UpdateInput(s->GetValueY(0.001,-500),2);
	  s->UpdateInput(s->GetValueZ(0.001,-500),3);
      emitvector->SetValue(csVector3(s->GetValueX(0.001,-500),s->GetValueY(0.001,-500),s->GetValueZ(0.001,-500)));
	  emitState->SetStartSpeedEmit(emitvector); 
      break;
	case 2: 
	  s->UpdateInput(s->GetValueX(0.002,-500),1);
	  s->UpdateInput(s->GetValueY(0.002,-500),2);
	  s->UpdateInput(s->GetValueZ(0.002,-500),3);
	  emitvector->SetValue(csVector3(s->GetValueX(0.002,-500),s->GetValueY(0.002,-500),s->GetValueZ(0.002,-500)));
	  emitState->SetStartAccelEmit(emitvector);
	  break;
	case 3:
	  s->UpdateInput(s->GetValueX(0.002,-500),1);
	  s->UpdateInput(s->GetValueY(0.002,-500),2);
	  s->UpdateInput(s->GetValueZ(0.002,-500),3);
	  emitvector->SetValue(csVector3(s->GetValueX(0.002,-500),s->GetValueY(0.002,-500),s->GetValueZ(0.002,-500)));
	  emitState->SetStartPosEmit(emitvector);
	  break;
	case 4:
	  s->UpdateInput(s->GetValueX(0.0005,0),1);
	  s->UpdateInput(s->GetValueY(0.0005,0),2);
	  emitState->SetRectParticles(s->GetValueX(0.0005,0),s->GetValueY(0.0005,0));
	  break;
	case 5:
	  s->UpdateInput((int)(s->GetValueOther(0.2)),4);
	  emitState->SetParticleCount((int)(s->GetValueOther(0.2)));
	  break;
	case 6:
	  
	  if(s->GetValueOther(5.0) <= 0) 
	  {
		s->UpdateInput(5.0,4);
		emitState->SetParticleTime(5);
	  }
	  else 
	  {
		s->UpdateInput((s->GetValueOther(5.0)),4);
		emitState->SetParticleTime(int(s->GetValueOther(5.0)));
	  }
	  break;
	case 7:
	  s->UpdateInput(s->GetValueX(0.001,0),1);
	  s->UpdateInput(s->GetValueY(0.001,0),2);
	  s->UpdateInput(s->GetValueZ(0.001,0),3);
	  s->UpdateLabel("Red",1);
	  s->UpdateLabel("Green",2);
	  s->UpdateLabel("Blue",3);
	  emitState->AddAge(0, csColor(s->GetValueX(0.001,0),s->GetValueY(0.001,0),s->GetValueZ(0.001,0)), 0.3,0.3, 1, 1);
	  break;
  }
	s->SetUpdateState(false);
  }

  if (kbd->GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP) && keydown == false) {
    //c->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
	keydown = true;
	width = width + 0.1;
	if(width <= 0.05)
		    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"width did not increment");
	  emitState->AddAge(0, csColor(0,width,1), 0.3,
    0.3, 1, 1);
	//emitState->SetRectParticles(width,0.05);
	//printf("Y_speed value: %f\n",y_speed); 
	//startspeed->SetValue(csVector3(0,y_speed,0));
	//emitState->SetStartSpeedEmit(startspeed);
  }
  else
	 // keydown = false;
  if (kbd->GetKeyState (CSKEY_PGDN))
    c->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    c->Move (CS_VEC_FORWARD * 4 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    c->Move (CS_VEC_BACKWARD * 4 * speed);
  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;
  
  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Start drawing 2D graphics.
  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
	return;
  // Make sure invalidated areas get a chance to
  // redraw themselves.
  aws->Redraw ();
  // Draw the current view of the window system to a
  // graphics context with a certain alpha value.
  aws->Print (g3d, 64);
}

void PartEdit::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool PartEdit::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevKeyDown && ev.Key.Code == CSKEY_ESC)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }
  return aws->HandleEvent(ev);
}

bool PartEdit::EventHandler (iEvent& ev)
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

bool PartEdit::Initialize ()
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
	CS_REQUEST_PLUGIN("crystalspace.window.alternatemanager", iAws),
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, EventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"Can't initialize event handler!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (vc == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (engine == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
	"No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (loader == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (g3d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iGraphics3D plugin!");
    return false;
  }
  g2d = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (g2d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iGraphics3D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (kbd == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  aws = CS_QUERY_REGISTRY (object_reg, iAws);
  if (aws == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"No iAws plugin!");
    return false;
  }

  iNativeWindow* nw = g2d->GetNativeWindow ();
  if (nw) nw->SetTitle ("Particle System Editor");

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"Error opening system!");
    return false;
  }  

  PluginManager = CS_QUERY_REGISTRY(object_reg, iPluginManager);
  if (!PluginManager)
    return "No iPluginManager plugin!";



  // First disable the lighting cache. Our app is PartEdit enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"Error loading 'stone4' texture!");
    return false;
  }
  if (!loader->LoadTexture ("energy", "/lib/std/energy.jpg"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.partedit",
    	"Error loading 'stone4' texture!");
    return false;
  }

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws = SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iStatLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10,
  	csColor (1, 0, 0), false);
  ll->Add (light->QueryLight ());

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10,
  	csColor (0, 0, 1), false);
  ll->Add (light->QueryLight ());

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10,
  	csColor (0, 1, 0), false);
  ll->Add (light->QueryLight ());

  // Get the emit mesh object plug-in.
  csRef<iMeshObjectType> emit_type (CS_LOAD_PLUGIN(PluginManager,
  	"crystalspace.mesh.object.emit", iMeshObjectType));  

   // Emit
  csRef<iMeshObjectFactory> EmitObjectFactory = emit_type->NewFactory();
  csRef<iEmitFactoryState> EmitFactoryState = SCF_QUERY_INTERFACE(EmitObjectFactory, iEmitFactoryState);

  iEmitBox* box = EmitFactoryState->CreateBox ();
  box->SetContent (csVector3 (0, 0, 0), csVector3 (1, 1, 1));

  csRef<iMeshObject> mesh = EmitObjectFactory->NewInstance();  
  mesh->SetMaterialWrapper(engine->GetMaterialList()->FindByName("energy"));

  emitState = SCF_QUERY_INTERFACE (mesh, iEmitState);

  iEmitFixed* startpos = EmitFactoryState->CreateFixed();
  iEmitFixed* startspeed = EmitFactoryState->CreateFixed();
  iEmitFixed* startaccel = EmitFactoryState->CreateFixed();
  emitvector = EmitFactoryState->CreateFixed();
  startpos->SetValue(csVector3 (0,0,0));
  startspeed->SetValue(csVector3 (0,0.5,0));
  startaccel->SetValue(csVector3 (0,0,0));
  emitState->SetParticleCount(25);
  emitState->SetLighting (false);
  emitState->SetRectParticles(width,0.05);
  emitState->SetParticleTime(int(part_time));
  emitState->SetStartPosEmit(startpos);
  emitState->SetStartSpeedEmit(startspeed);
  emitState->SetStartAccelEmit(startaccel);
  //emitState->AddAge(0, csColor(0,1,1), 0.3,0.3, 1, 1);

  csRef<iMeshWrapper> mw = engine->CreateMeshWrapper(mesh, "emit", room,csVector3 (0, 5, 0));
 
  engine->Prepare ();

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // awsCanvas = csPtr<iAwsCanvas> (aws->CreateCustomCanvas (g2d, g3d));
  aws->SetFlag (AWSF_AlwaysRedrawWindows);
  aws->SetupCanvas (NULL,g2d,g3d);

  // Setup sink.  
  s  = new awsSink();
  iAwsSink    *sink =aws->GetSinkMgr()->CreateSink(s);

  s->SetSink(sink);
  s->SetWindowManager(aws);

  aws->GetSinkMgr()->RegisterSink("parteditSink", sink);

  // now load preferences
  if (!aws->GetPrefMgr()->Load("/this/data/temp/partedit.def"))
    csReport(object_reg,CS_REPORTER_SEVERITY_ERROR,
	       "crystalspace.application.partedit",
	       "couldn't load skin definition file!");

  aws->GetPrefMgr()->SelectDefaultSkin("Normal Windows");

  iAwsWindow *test = aws->CreateWindowFrom("Another");
  if (test) test->Show ();

  return true;
}

void PartEdit::Start ()
{
  csDefaultRunLoop (object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);

  System = new PartEdit (object_reg);
  if (System->Initialize ())
    System->Start ();
  delete System;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}


