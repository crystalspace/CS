/*  -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
    Copyright (C) 2004 by Peter Amstutz, Jorrit Tyberghein

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

#include <wx/wx.h>

#define CS_IMPLEMENT_PLATFORM_APPLICATION

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/cfgfile.h"
#include "csutil/cfgmgr.h"
#include "iutil/vfs.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "wxtest.h"
#include "iutil/eventq.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing.h"
#include "imesh/object.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "ivideo/natwin.h"
#include "ivideo/wxwin.h"
#include "igraphic/imageio.h"
#include "imap/loader.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "csutil/cmdhelp.h"
#include "csutil/event.h"

CS_IMPLEMENT_APPLICATION

#if defined(CS_PLATFORM_WIN32)

#ifndef SW_SHOWNORMAL
#define SW_SHOWNORMAL 1
#endif

/*
  WX provides WinMain(), but not main(), which is required for console apps.
 */
int main (int argc, const char* const argv[])
{
  return WinMain (GetModuleHandle (0), 0, GetCommandLineA (), SW_SHOWNORMAL);
}

#endif

//-----------------------------------------------------------------------------


BEGIN_EVENT_TABLE(Simple, wxFrame)
  EVT_SHOW( Simple::OnShow )
  EVT_ICONIZE( Simple::OnIconize )
END_EVENT_TABLE()


// The global pointer to simple
Simple* simple = 0;

Simple::Simple (iObjectRegistry* object_reg)
  : wxFrame(NULL, -1, wxT("Crystal Space WxWidget Canvas test"), wxDefaultPosition, wxSize(500, 500))
{
  Simple::object_reg = object_reg;
}

Simple::~Simple ()
{
}

void Simple::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  iCamera* c = view->GetCamera();

  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * 4 * speed);
  }
  else
  {
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      rotY += speed;
    if (kbd->GetKeyState (CSKEY_LEFT))
      rotY -= speed;
    if (kbd->GetKeyState (CSKEY_PGUP))
      rotX += speed;
    if (kbd->GetKeyState (CSKEY_PGDN))
      rotX -= speed;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 4 * speed);
  }

  // We now assign a new rotation transformation to the camera.  You
  // can think of the rotation this way: starting from the zero
  // position, you first rotate "rotY" radians on your Y axis to get
  // the first rotation.  From there you rotate "rotX" radians on the
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csXRotMatrix3 (rotX) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  c->SetTransform (ot);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

void Simple::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool Simple::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    FinishFrame ();
    return true;
  }
  else if ((ev.Type == csevKeyboard))
  {
    csPrintf("Got key %" PRIu32 " / %" PRIu32 "\n",
           csKeyEventHelper::GetCookedCode(&ev),
           csKeyEventHelper::GetCookedCode(&ev));
    if((csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeDown) &&
       (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC))
    {
      csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
      if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
      return true;
    }
  }
  else if ((ev.Type == csevMouseMove))
  {
    csPrintf("Mouse move to %d %d\n", ev.Mouse.x, ev.Mouse.y);
  }
  else if ((ev.Type == csevMouseDown))
  {
    csPrintf("Mouse button %d down at %d %d\n", ev.Mouse.Button, ev.Mouse.x, ev.Mouse.y);
  }
  else if ((ev.Type == csevMouseUp))
  {
    csPrintf("Mouse button %d up at %d %d\n", ev.Mouse.Button, ev.Mouse.x, ev.Mouse.y);
  }

  return false;
}

bool Simple::SimpleEventHandler (iEvent& ev)
{
  return simple ? simple->HandleEvent (ev) : false;
}

bool Simple::Initialize ()
{
  csRef<iConfigManager> confmgr = CS_QUERY_REGISTRY(object_reg, iConfigManager);
  confmgr->SetDynamicDomainPriority(iConfigManager::PriorityMax);
  confmgr->GetDynamicDomain()->SetStr("Video.OpenGL.Canvas", "crystalspace.graphics2d.wxgl");

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
              "crystalspace.application.simple1",
              "Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.simple1",
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
              "crystalspace.application.simple1",
              "Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (engine == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.simple1",
              "No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (loader == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.simple1",
              "No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (g3d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.simple1",
              "No iGraphics3D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (kbd == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.simple1",
              "No iKeyboardDriver plugin!");
    return false;
  }

  new wxPanel(this, -1, wxPoint(0, 0), wxSize(1, 1));
  wxPanel* panel = new wxPanel(this, -1, wxPoint(50, 50), wxSize(400, 400));
  Show(true);
  panel->Show(true);

  iGraphics2D* g2d = g3d->GetDriver2D();
  csRef<iWxWindow> wxwin = SCF_QUERY_INTERFACE(g2d, iWxWindow);
  wxwin->SetParent(panel);

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.simple1",
              "Error opening system!");
    return false;
  }

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.simple1",
              "Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  // these are used store the current orientation of the camera
  rotY = rotX = 0;

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

  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  return true;
}

void Simple::PushFrame ()
{
  csRef<iEventQueue> q (CS_QUERY_REGISTRY(object_reg, iEventQueue));
  if (!q)
    return ;
  csRef<iVirtualClock> vc (CS_QUERY_REGISTRY(object_reg, iVirtualClock));

  if (vc)
    vc->Advance();
  q->Process();
}

void Simple::OnIconize(wxIconizeEvent& event)
{
  csPrintf("got iconize %d\n", (int)event.Iconized());
}

void Simple::OnShow(wxShowEvent& event)
{
  csPrintf("got show %d\n", (int)event.GetShow());
}

/* There are two ways to drive the CS event loop, from
  a Wx timer or from idle events.  This test app demonstrates
  either method depending on which #define is set below.  Using
  a timer seems to produce better results (a smoother framerate
  and better CPU utilization).
*/

//#define USE_IDLE
#define USE_TIMER

#ifdef USE_TIMER
class Pump : public wxTimer
{
public:
  Simple* s;
  Pump() { };
  virtual void Notify()
    {
    s->PushFrame();
    }
};
#endif


// Define a new application type
class MyApp: public wxApp
{
public:
  iObjectRegistry* object_reg;

  virtual bool OnInit(void);
  virtual int OnExit(void);

#ifdef USE_IDLE
  virtual void OnIdle();
  DECLARE_EVENT_TABLE();
#endif
};

#ifdef USE_IDLE
BEGIN_EVENT_TABLE(MyApp, wxApp)
  EVT_IDLE(MyApp::OnIdle)
END_EVENT_TABLE()
#endif


IMPLEMENT_APP(MyApp)

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
  bool MyApp::OnInit(void)
{
#if defined(wxUSE_UNICODE) && wxUSE_UNICODE
  char** csargv;
    csargv = (char**)malloc(sizeof(char*) * argc);
  for(int i = 0; i < argc; i++) {
    csargv[i] = strdup(wxString(argv[i]).mb_str().data());
  }
  object_reg = csInitializer::CreateEnvironment (argc, csargv);
#else
  object_reg = csInitializer::CreateEnvironment (argc, argv);
#endif

  simple = new Simple (object_reg);
  simple->Initialize ();

#ifdef USE_TIMER
  /* This triggers a timer event every 20 milliseconds, which will yield
   a maximum framerate of 1000/20 = 50 FPS.  Obviously if it takes longer
   than 20 ms to render a frame the framerate will be lower :-)
   You may wish to tweak this for your own application.  Note that
   this also lets you throttle the CPU usage of your app, because
   the application will yield the CPU and wait for events in the
   time between when it completes rendering the current frame and
   the timer triggers the next frame.
  */

  Pump* p = new Pump();
  p->s = simple;
  p->Start(20);
#endif

  return true;
}

#ifdef USE_IDLE
void MyApp::OnIdle() {
  simple->PushFrame();
}
#endif

int MyApp::OnExit()
{
  simple = 0;
  csInitializer::DestroyApplication (object_reg);
  return 0;
}
