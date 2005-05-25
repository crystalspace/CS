/*
    Copyright (C) 2002 by Bhasker Hariharan

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

#include "pathtut.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to PathTut
PathTut *pathtut;

PathTut::PathTut (iObjectRegistry* object_reg)
{
  PathTut::object_reg = object_reg;
  m_Path = 0;
}

PathTut::~PathTut ()
{
  delete m_Path;
}

void PathTut::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  iCamera* c = view->GetCamera();
  if (kbd->GetKeyState (CSKEY_RIGHT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    c->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    c->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    c->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    c->Move (CS_VEC_FORWARD * 4 * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    c->Move (CS_VEC_BACKWARD * 4 * speed);

  Animate (elapsed_time);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

void PathTut::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool PathTut::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    pathtut->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdFinalProcess)
  {
    pathtut->FinishFrame ();
    return true;
  }
  else if ((ev.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeDown) &&
    (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC))
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->GetEventOutlet()->Broadcast (cscmdQuit);
    return true;
  }

  return false;
}

bool PathTut::PathTutEventHandler (iEvent& ev)
{
  return pathtut->HandleEvent (ev);
}

bool PathTut::Initialize (int argc, const char* const argv[])
{
  (void)argc;
  (void)argv;
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
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, PathTutEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
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
  if (!vc)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
	"Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
	"No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
    	"No iGraphics3D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
    	"Error opening system!");
    return false;
  }

  // First disable the lighting cache. Our app is PathTut enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
    	"Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (-3, 5, 0), 10,
  	csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 5,  0), 10,
  	csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 5, -3), 10,
  	csColor (0, 1, 0));
  ll->Add (light);

  engine->Prepare ();

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  iTextureManager* txtmgr = g3d->GetTextureManager ();

  // Load a texture for our sprite.
  iTextureWrapper* txt = loader->LoadTexture ("spark",
  	"/lib/std/spark.png", CS_TEXTURE_3D, txtmgr, true);
  if (txt == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
    	"Error loading texture!");
    return false;
  }

  // Load a sprite template from disk.
  csRef<iMeshFactoryWrapper> imeshfact (loader->LoadMeshObjectFactory (
  	"/lib/std/sprite1"));
  if (imeshfact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.PathTut",
    	"Error loading mesh object factory!");
    return false;
  }

  csMatrix3 m; m.Identity ();
  m *= 0.3f;
  imeshfact->HardTransform (csReversibleTransform (m, csVector3 (0)));

  // Create the sprite and add it to the engine.
  sprite = engine->CreateMeshWrapper (
  	imeshfact, "MySprite", room,
	csVector3 (-3, 5, 3));
  sprite->GetMovable ()->UpdateMove ();
  csRef<iSprite3DState> spstate (SCF_QUERY_INTERFACE (sprite->GetMeshObject (),
  	iSprite3DState));
  spstate->SetAction ("default");
 
  // Initialize the Path to move the sprite around in a ellipse.
  InitializePath ();

  // The Sprite will complete one revolution in 10secs.
  m_Duration = 10000;
  m_CurrentTime = 0;

  // The following two calls are not needed since CS_ZBUF_USE and
  // Object render priority are the default but they show how you
  // can do this.
  sprite->SetZBufMode (CS_ZBUF_USE);
  sprite->SetRenderPriority (engine->GetObjectRenderPriority ());

  return true;
}

void PathTut::Animate (csTicks elapsedTime)
{
  csVector3 pos, forward, up;
  if (m_CurrentTime+elapsedTime < m_Duration)
  {
    // Update the Sprite Position.
    m_CurrentTime += elapsedTime;
    float time_val = (float)m_CurrentTime / (float)m_Duration;
    m_Path->CalculateAtTime (time_val);
    m_Path->GetInterpolatedPosition (pos);
    m_Path->GetInterpolatedForward (forward);
    m_Path->GetInterpolatedUp (up);
    sprite->GetMovable()->SetPosition (pos);
    sprite->GetMovable()->GetTransform ().LookAt (forward.Unit(), up.Unit());
    sprite->GetMovable()->UpdateMove ();
  }
  else
  {
    // One cycle is over reset m_CurrentTime.
    m_CurrentTime = 0;
  }
}

void PathTut::InitializePath ()
{
  m_Path = new csPath (5);

  // Here we use the temp path to get the mid pt of the line.
  csVector3 leftpt, rightpt, midpt;
  leftpt = sprite->GetMovable ()->GetPosition ();
  rightpt = csVector3 (leftpt.x+6, leftpt.y, leftpt.z);
  midpt = (leftpt + rightpt) / 2.0;

  // Now we have the mid pt..now we create an elliptical path
  // by adding creating two pts.. a few units above and below the mid pt.
  csVector3 PositionVectors[5], ForwardVectors[5], UpVectors[5];
  PositionVectors[0] = leftpt;
  PositionVectors[1] = csVector3 (midpt.x, midpt.y+2.5, midpt.z);
  PositionVectors[2] = rightpt;
  PositionVectors[3] = csVector3 (midpt.x, midpt.y-2.5, midpt.z);
  PositionVectors[4] = leftpt; // Close the loop.
  m_Path->SetPositionVectors (PositionVectors);  

  // Set the direction vectors.
  ForwardVectors[0] = PositionVectors[1]-PositionVectors[0];
  ForwardVectors[1] = PositionVectors[2]-PositionVectors[1];
  ForwardVectors[2] = PositionVectors[3]-PositionVectors[2];
  ForwardVectors[3] = PositionVectors[4]-PositionVectors[3];
  ForwardVectors[4] = PositionVectors[0]-PositionVectors[4];
  m_Path->SetForwardVectors (ForwardVectors);

  // Set the up vectors.
  UpVectors[0] = csVector3 (0,0,1);
  UpVectors[1] = csVector3 (0,.5,.5);
  UpVectors[2] = csVector3 (0,1,0);
  UpVectors[3] = csVector3 (0,.5,.5);
  UpVectors[4] = csVector3 (0,0,1);

  m_Path->SetUpVectors (UpVectors);
  // Now we set the time values for the various pts in the paths..in our 
  // case since we want a constant speed between pts we set the time values
  // as diff 0.25;
  float time_vals[5];
  time_vals[0] = 0;
  time_vals[1] = 0.25;
  time_vals[2] = 0.5;
  time_vals[3] = 0.75;
  time_vals[4] = 1.0;
  m_Path->SetTimes (time_vals);

  // Path Setup is complete now.
}

void PathTut::Start ()
{
  csDefaultRunLoop (object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  pathtut = new PathTut (object_reg);

  if (pathtut->Initialize (argc, argv))
    pathtut->Start ();

  delete pathtut;

  csInitializer::DestroyApplication (object_reg);

  return 0;
}

