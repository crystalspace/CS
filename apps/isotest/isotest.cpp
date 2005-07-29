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
#include "isotest.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "csutil/cscolor.h"
#include "csutil/event.h"
#include "csutil/sysfunc.h"
#include "iengine/camera.h"
#include "iengine/campos.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "iengine/texture.h"
#include "igraphic/imageio.h"
#include "imap/loader.h"
#include "imesh/object.h"
#include "imesh/sprite3d.h"
#include "iutil/csinput.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/virtclk.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "ivideo/fontserv.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"
#include "imesh/genmesh.h"
#include "imesh/gmeshskel.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to isotest
IsoTest *isotest;

IsoTest::IsoTest (iObjectRegistry* object_reg)
{
  IsoTest::object_reg = object_reg;

  current_view = 0;
  views[0].SetOrigOffset (csVector3 (-4, 4, -4)); // true isometric perspective.
  views[1].SetOrigOffset (csVector3 (-9, 9, -9)); // zoomed out.
  views[2].SetOrigOffset (csVector3 (4, 3, -4)); // diablo style perspective.
  views[3].SetOrigOffset (csVector3 (0, 4, -4)); // zelda style perspective.

  actor_is_walking = false;
}

IsoTest::~IsoTest ()
{
}

void IsoTest::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 90);

  if (kbd->GetModifierState (CSKEY_SHIFT_LEFT) 
    || kbd->GetModifierState (CSKEY_SHIFT_RIGHT))
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      views[current_view].angle += speed*15.f;
    if (kbd->GetKeyState (CSKEY_LEFT))
      views[current_view].angle -= speed*15.f;
    if (kbd->GetKeyState (CSKEY_UP))
      views[current_view].distance -= 0.25f*speed;
    if (kbd->GetKeyState (CSKEY_DOWN))
      views[current_view].distance += 0.25f*speed;
    SetupIsoView(views[current_view]);
  }
  else
  {
    float facing = 0.f; // in degrees
    bool moved = false;
    if (kbd->GetKeyState (CSKEY_RIGHT))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (speed, 0, 0));
      facing = 270.f;
    }
    if (kbd->GetKeyState (CSKEY_LEFT))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (-speed, 0, 0));
      facing = 90.f;
    }
    if (kbd->GetKeyState (CSKEY_UP))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (0, 0, speed));
      facing = 0.f;
    }
    if (kbd->GetKeyState (CSKEY_DOWN))
    {
      moved = true;
      actor->GetMovable ()->MovePosition (csVector3 (0, 0, -speed));
      facing = 180.f;
    }

    if(kbd->GetKeyState (CSKEY_DOWN) && kbd->GetKeyState (CSKEY_LEFT))
      facing = 135;
    if(kbd->GetKeyState (CSKEY_DOWN) && kbd->GetKeyState (CSKEY_RIGHT))
      facing = 225;
    if(kbd->GetKeyState (CSKEY_UP) && kbd->GetKeyState (CSKEY_LEFT))
      facing = 45;
    if(kbd->GetKeyState (CSKEY_UP) && kbd->GetKeyState (CSKEY_RIGHT))
      facing = 315;

    if(moved)
    {
      csYRotMatrix3 r(facing*PI/180.0);
      actor->GetMovable ()->SetTransform(r);
    }
    // update animation state
    csRef<iGeneralMeshState> spstate (
      SCF_QUERY_INTERFACE (actor->GetMeshObject (), iGeneralMeshState));
    csRef<iGenMeshSkeletonControlState> animcontrol (
      SCF_QUERY_INTERFACE (spstate->GetAnimationControl (), 
      iGenMeshSkeletonControlState));
    if(actor_is_walking && !moved)
    {
      animcontrol->StopAll();
      animcontrol->Execute("idle");
    }
    if(!actor_is_walking && moved)
    {
      animcontrol->StopAll();
      animcontrol->Execute("walk");
    }
    actor_is_walking = moved;
  }

  // Make sure actor is constant distance above plane.
  csVector3 actor_pos = actor->GetMovable ()->GetPosition ();
  actor_pos.y += 10.0;	// Make sure we start beam high enough.
  csVector3 end_pos, isect;
  end_pos = actor_pos; end_pos.y -= 100.0;
  float r;
  plane->HitBeamObject (actor_pos, end_pos, isect, &r);
  actor_pos.y = isect.y + .8;

  actor->GetMovable ()->SetPosition (actor_pos);
  actor->GetMovable ()->UpdateMove ();

  // Move the light.
  actor_light->SetCenter (actor_pos+csVector3 (0, 2, -1));

  CameraIsoLookat(view->GetCamera(), views[current_view], actor_pos); 
  
  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
    return;

  csVector2 lpos(0,0);
  view->GetCamera()->Perspective(
    view->GetCamera()->GetTransform ().Other2This(csVector3 (-4.7f, 1.0f, 5.5f)),
    lpos);
  // display a helpful little text.
  int txtw=0, txth=0;
  font->GetMaxSize(txtw, txth);
  if(txth == -1) txth = 20;
  int white = g3d->GetDriver2D ()->FindRGB (255, 255, 255);
  g3d->GetDriver2D ()->DrawBox((int)lpos.x-2,
    g3d->GetDriver2D ()->GetHeight()-(int)lpos.y-2,4,4,white);
  int ypos = g3d->GetDriver2D ()->GetHeight () - txth*4 - 1;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "Isometric demo keys (esc to exit):");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   arrow keys: move around");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   shift+arrow keys: rotate/zoom camera");
  ypos += txth;
  g3d->GetDriver2D ()->Write (font, 1, ypos, white, -1, 
    "   tab key: cycle through camera presets");
}

void IsoTest::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool IsoTest::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    isotest->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdFinalProcess)
  {
    isotest->FinishFrame ();
    return true;
  }
  else if ((ev.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeDown))
  {
    utf32_char c = csKeyEventHelper::GetCookedCode (&ev);
    if (c == CSKEY_ESC)
    {
      csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
      if (q)
	q->GetEventOutlet()->Broadcast (cscmdQuit);
      return true;
    }
    else if (c == CSKEY_TAB)
    {
      current_view++;
      if (current_view >= 4) current_view = 0;
    }
  }

  return false;
}

void IsoTest::CameraIsoLookat(csRef<iCamera> cam, const IsoView& isoview,
    const csVector3& lookat)
{
  // Let the camera look at the actor.
  // so the camera is set to look at 'actor_pos'
  //int isofactor = 50; // 98.3% isometric (=GetFovAngle()/180.0)
  //int isofactor = 100; // 99.2% isometric (=GetFovAngle()/180.0)
  int isofactor = 200; // 99.6% isometric (=GetFovAngle()/180.0)

  // set center and lookat
  csOrthoTransform& cam_trans = cam->GetTransform ();
  cam_trans.SetOrigin (lookat + float(isofactor)*isoview.camera_offset);
  cam_trans.LookAt (lookat-cam_trans.GetOrigin (), csVector3 (0, 1, 0));
  // set fov more isometric, could be done in initialisation once.
  cam->SetFOV (g3d->GetHeight()*isofactor, g3d->GetWidth());

  // due to moving the camera so far away, depth buffer accuracy is
  // impaired, repair that by using smaller coordinate system
  csOrthoTransform repair_trans = cam->GetTransform();
  repair_trans.SetT2O (repair_trans.GetT2O()/repair_trans.GetOrigin().Norm());
  cam->SetTransform (repair_trans);
}

void IsoTest::SetupIsoView(IsoView& isoview)
{
  // clamp
  if(isoview.angle < 0.f) isoview.angle += 360.f;
  if(isoview.angle > 360.f) isoview.angle -= 360.f;
  if(isoview.distance < 0.05f) isoview.distance = 0.05f;
  if(views[current_view].distance > 10.f) isoview.distance = 10.f;
  // setup
  csYRotMatrix3 r(isoview.angle * PI / 180.0);
  isoview.camera_offset = (r*isoview.original_offset)*isoview.distance;
}

bool IsoTest::IsoTestEventHandler (iEvent& ev)
{
  return isotest->HandleEvent (ev);
}

bool IsoTest::LoadMap ()
{
  // First disable the lighting cache. Our map uses stencil
  // lighting.
  engine->SetLightingCacheMode (0);

  // Set VFS current directory to the level we want to load.
  csRef<iVFS> VFS (CS_QUERY_REGISTRY (object_reg, iVFS));
  VFS->ChDir ("/lev/isomap");
  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"Couldn't load level!");
    return false;
  }

  // Find the starting position in this level.
  csVector3 pos (0, 0, 0);
  if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    // There is a valid starting position defined in the level file.
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    room = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
  }
  else
  {
    // We didn't find a valid starting position. So we default
    // to going to room called 'room' at position (0,0,0).
    room = engine->GetSectors ()->FindByName ("room");
  }
  if (!room)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
      	"Can't find a valid starting position!");
    return false;
  }

  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);

  iLightList* ll = room->GetLights ();
  actor_light = engine->CreateLight (0, csVector3 (-3, 5, 0), 5,
    csColor (1, 1, 1));
  ll->Add (actor_light);

  csRef<iLight> statuelight = engine->CreateLight ("statuelight",
    csVector3 (-4.7f, 1.0f, 5.5f), 4, csColor(1.2f,0.2f,0.2f));
  statuelight->CreateNovaHalo (1278, 15, 0.3f);
  ll->Add (statuelight);

  plane = engine->FindMeshObject ("Plane");

  return true;
}

bool IsoTest::CreateActor ()
{
  // Load a texture for our sprite.
  iTextureManager* txtmgr = g3d->GetTextureManager ();
  iTextureWrapper* txt = loader->LoadTexture ("vedette_fashion",
    "/lib/std/ugly_woman.jpg", CS_TEXTURE_3D, txtmgr, false, false);
  if (txt == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.isotest",
        "Error loading texture!");
    return false;
  }
  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
  csRef<iShaderManager> shader_mgr = CS_QUERY_REGISTRY (object_reg,
  	iShaderManager);
  if (shader_mgr == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.isotest",
        "Couldn't find shader manager! This application requires new renderer!"
	);
    return false;
  }
  iShader* ambient_shader = shader_mgr->GetShader ("ambient");
  iShader* light_shader = shader_mgr->GetShader ("light");
  if (ambient_shader == 0 || light_shader == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.isotest",
        "Couldn't find shaders!");
    return false;
  }
  csRef<iMaterial> fash_material = engine->CreateBaseMaterial (txt);
  fash_material->SetShader (strings->Request ("ambient"), ambient_shader);
  fash_material->SetShader (strings->Request ("diffuse"), light_shader);
  engine->GetMaterialList ()->NewMaterial (fash_material, "vedette_fashion");

  // Load a sprite template from disk.
  csRef<iMeshFactoryWrapper> imeshfact (
    loader->LoadMeshObjectFactory ("/lev/isomap/vedette.spr"));
  if (imeshfact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.isotest",
        "Error loading mesh object factory!");
    return false;
  }
  csMatrix3 m; m.Identity (); m *= 1.10f; // scaling factor
  imeshfact->HardTransform (csReversibleTransform (m, csVector3 (0)));

  // Create the sprite and add it to the engine.
  actor = engine->CreateMeshWrapper (
    imeshfact, "MySprite", room, csVector3 (-3, 2, 3));
  actor->GetMovable ()->UpdateMove ();
  csRef<iGeneralMeshState> spstate (
    SCF_QUERY_INTERFACE (actor->GetMeshObject (), iGeneralMeshState));
  csRef<iGenMeshSkeletonControlState> animcontrol (
    SCF_QUERY_INTERFACE (spstate->GetAnimationControl (), 
    iGenMeshSkeletonControlState));
  animcontrol->StopAll();
  animcontrol->Execute("idle");

  // The following two calls are not needed since CS_ZBUF_USE and
  // Object render priority are the default but they show how you
  // can do this.
  actor->SetZBufMode (CS_ZBUF_USE);
  actor->SetRenderPriority (engine->GetObjectRenderPriority ());
  return true;
}

bool IsoTest::Initialize ()
{
  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_PLUGIN("crystalspace.font.server.multiplexer", iFontServer),
	"crystalspace.font.server.freetype2", "iFontServer.1", 
	  scfInterfaceTraits<iFontServer>::GetID(), 
	  scfInterfaceTraits<iFontServer>::GetVersion(),
	"crystalspace.font.server.default", "iFontServer.2", 
	  scfInterfaceTraits<iFontServer>::GetID(), 
	  scfInterfaceTraits<iFontServer>::GetVersion(),
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
	"Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, IsoTestEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
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
    	"crystalspace.application.isotest",
	"Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
	"No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!g3d)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"No iGraphics3D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"No iKeyboardDriver plugin!");
    return false;
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
    	"crystalspace.application.isotest",
    	"Error opening system!");
    return false;
  }

  view = csPtr<iView> (new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  font = g3d->GetDriver2D ()->GetFontServer()->LoadFont
    ("/fonts/ttf/Vera.ttf", 10);
  if(!font) // fallback
    font = g3d->GetDriver2D ()->GetFontServer()->LoadFont(CSFONT_LARGE);

  if (!LoadMap ()) return false;
  if (!CreateActor ()) return false;
  engine->Prepare ();

  return true;
}

void IsoTest::Start ()
{
  csDefaultRunLoop (object_reg);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return -1;
  isotest = new IsoTest (object_reg);

  if (isotest->Initialize ())
    isotest->Start ();

  delete isotest;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}
