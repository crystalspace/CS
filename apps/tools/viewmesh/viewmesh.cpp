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

/* ViewMesh: tool for displaying mesh objects (3d sprites) */
#include "viewmesh.h"

#include "csutil/cscolor.h"
#include "csutil/common_handlers.h"
#include "csutil/event.h"
#include "csutil/scfstr.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "iutil/eventq.h"
#include "iutil/object.h"
#include "iutil/stringarray.h"
#include "iengine/scenenode.h"
#include "ivideo/graph2d.h"

// Hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#ifdef CS_HAVE_CAL3D
#include <cal3d/animcallback.h>
#include <cal3d/cal3d.h>
#endif

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

#ifdef CS_HAVE_CAL3D
struct vmAnimCallback : public CalAnimationCallback
{
  vmAnimCallback() {}

  virtual void AnimationUpdate (float anim_time, CalModel*, void*)
  {
    csPrintf ("Anim Update at time %.2f.\n",anim_time);
  }

  virtual void AnimationComplete (CalModel*, void*)
  {
    csPrintf ("Anim Completed!\n");
  }
};
#endif

//---------------------------------------------------------------------------

ViewMesh::ViewMesh () : 
  camMode(movenormal), roomsize(5), scale(1), move_sprite_speed(0),
  selectedSocket(0),  selectedCal3dSocket(0), meshTx(0), meshTy(0), 
  meshTz(0), callback(0)
{
  SetApplicationName ("CrystalSpace.ViewMesh");
}

ViewMesh::~ViewMesh ()
{
#ifdef CS_HAVE_CAL3D
  if (cal3dsprite && callback)
  {
    cal3dsprite->RemoveAnimCallback("walk", callback);
    delete callback;
    callback = 0;
  }
#endif
}

void ViewMesh::ProcessFrame()
{
  csTicks elapsed_time = vc->GetElapsedTicks ();
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  iCamera* c = view->GetCamera();

  if (!spritewrapper) camMode = movenormal;

  switch (camMode)
  {
    case movenormal:
    {
      if (kbd->GetKeyState (CSKEY_SHIFT))
      {
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
    	if (kbd->GetKeyState (CSKEY_RIGHT))
	  c->GetTransform ().RotateOther (CS_VEC_ROT_RIGHT, speed);
	if (kbd->GetKeyState (CSKEY_LEFT))
	  c->GetTransform ().RotateOther (CS_VEC_ROT_LEFT, speed);
	if (kbd->GetKeyState (CSKEY_PGUP))
	  c->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
	if (kbd->GetKeyState (CSKEY_PGDN))
	  c->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
        if (kbd->GetKeyState (CSKEY_UP))
          c->Move (CS_VEC_FORWARD * 4 * speed);
        if (kbd->GetKeyState (CSKEY_DOWN))
          c->Move (CS_VEC_BACKWARD * 4 * speed);
      }
      break;
    }
    case moveorigin:
    {
      csVector3 orig = c->GetTransform().GetOrigin();

      csBox3 box;
      box = spritewrapper->GetWorldBoundingBox();
      csVector3 spritepos = box.GetCenter();

      if (kbd->GetKeyState (CSKEY_DOWN))
	c->GetTransform().SetOrigin (orig + CS_VEC_BACKWARD * 4 * speed);
      if (kbd->GetKeyState (CSKEY_UP))
	c->GetTransform().SetOrigin (orig + CS_VEC_FORWARD * 4 * speed);
      if (kbd->GetKeyState (CSKEY_LEFT))
	c->GetTransform().SetOrigin (orig + CS_VEC_LEFT * 4 * speed);
      if (kbd->GetKeyState (CSKEY_RIGHT))
	c->GetTransform().SetOrigin (orig + CS_VEC_RIGHT * 4 * speed);
      if (kbd->GetKeyState (CSKEY_PGUP))
	c->GetTransform().SetOrigin (orig + CS_VEC_UP * 4 * speed);
      if (kbd->GetKeyState (CSKEY_PGDN))
	c->GetTransform().SetOrigin (orig + CS_VEC_DOWN * 4 * speed);
      c->GetTransform().LookAt (spritepos-orig, csVector3(0,1,0) );
      break;
    }
    case rotateorigin:
    {
      csVector3 orig = c->GetTransform().GetOrigin();

      csBox3 box;
      box = spritewrapper->GetWorldBoundingBox();
      csVector3 spritepos = box.GetCenter();

      if (kbd->GetKeyState (CSKEY_LEFT))
        orig = csYRotMatrix3(-speed) * (orig-spritepos) + spritepos;
      if (kbd->GetKeyState (CSKEY_RIGHT))
        orig = csYRotMatrix3(speed) * (orig-spritepos) + spritepos;
      if (kbd->GetKeyState (CSKEY_UP))
        orig = csXRotMatrix3(speed) * (orig-spritepos) + spritepos;
      if (kbd->GetKeyState (CSKEY_DOWN))
        orig = csXRotMatrix3(-speed) * (orig-spritepos) + spritepos;

      c->GetTransform().SetOrigin(orig);

      if (kbd->GetKeyState (CSKEY_PGUP))
        c->Move(CS_VEC_FORWARD * 4 * speed);
      if (kbd->GetKeyState (CSKEY_PGDN))
        c->Move(CS_VEC_BACKWARD * 4 * speed);

      c->GetTransform().LookAt (spritepos-orig, csVector3(0,1,0) );
      break;
    }
    default:
      break;
  }

  if (spritewrapper)
  {
    csRef<iMovable> mov = spritewrapper->GetMovable();
    csVector3 pos = mov->GetFullPosition();    
    mov->MovePosition(csVector3(pos.x,pos.y,-move_sprite_speed*elapsed_time/1000.0f));
    mov->UpdateMove();
    if (pos.z > roomsize) 
    {
      pos.z = -roomsize;
      mov->SetPosition(pos);
    }
    else if (pos.z < -roomsize) 
    {
      pos.z = roomsize;
      mov->SetPosition(pos);
    }
  }

  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();

  if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) 
    return;

  cegui->Render();
}

void ViewMesh::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool ViewMesh::OnKeyboard(iEvent& ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      csRef<iEventQueue> q = 
        CS_QUERY_REGISTRY(GetObjectRegistry(), iEventQueue);
      if (q.IsValid())
	q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

void ViewMesh::Help ()
{
  csPrintf ("Options for ViewMesh:\n");
  csPrintf ("  -L=<file>          Load a library file (for textures/materials)\n");
  csPrintf ("  -Scale=<ratio>     Scale the Object\n");
  csPrintf ("  -RoomSize=<units>  Radius and height (4*) of the room (default 5)\n");
  csPrintf ("  -R=<realpath>      Real path from where to load the model\n");
  csPrintf ("  -C=<vfsdir>        Current VFS directory\n");
  csPrintf ("  <file>             Load the specified mesh object from the VFS path (meshfact or library)\n");
}

void ViewMesh::HandleCommandLine()
{
  csRef<iCommandLineParser> cmdline =
    CS_QUERY_REGISTRY(GetObjectRegistry(), iCommandLineParser);

  const char* libname;
  for (int i=0; (libname=cmdline->GetOption("L",i)); i++)
  {
    if (!loader->LoadLibraryFile(libname))
    {
      ReportError("Couldn't load lib %s.\n", libname);
    }
  }

  const char* meshfilename = cmdline->GetName (0);
  const char* texturefilename = cmdline->GetName (1);
  const char* texturename = cmdline->GetName (2);
  const char* scaleTxt = cmdline->GetOption("Scale");
  const char* roomSize = cmdline->GetOption("RoomSize");
  const char* realPath = cmdline->GetOption("R");
  const char* vfsDir = cmdline->GetOption("C");

  if (realPath)
  {
    vfs->Mount ("/tmp/viewmesh", realPath);
    vfs->ChDir ("/tmp/viewmesh");
  }
  if (vfsDir)
  {
    vfs->ChDir (vfsDir);
  }

  if (texturefilename && texturename)
  {
    LoadTexture(texturefilename, texturename);
  }

  if (meshfilename)
  {
    LoadSprite(meshfilename);
  }

  if (roomSize) roomsize = atof(roomSize);

  if (scaleTxt != 0)
  {
    float newScale;
    sscanf (scaleTxt, "%f", &newScale);
    ScaleSprite(newScale);
  }

}

void ViewMesh::LoadTexture(const char* file, const char* name)
{
  if (file && name)
  {
    iTextureWrapper* txt = loader->LoadTexture (name, file);
    if (txt == 0)
    {
      ReportError("Cannot load texture '%s' from file '%s'.\n", name, file);
      return;
    }
    engine->PrepareTextures();
  }
}

void ViewMesh::LoadLibrary(const char* file)
{
  loader->LoadLibraryFile(file);
}

bool ViewMesh::OnInitialize(int /*argc*/, char* /*argv*/ [])
{
  if (csCommandLineHelper::CheckHelp (GetObjectRegistry()))
  {
    ViewMesh::Help();
    csCommandLineHelper::Help(GetObjectRegistry());
    return 0;
  }

  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_LEVELSAVER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());

  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void ViewMesh::OnExit()
{
}

bool ViewMesh::Application()
{
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  g3d = CS_QUERY_REGISTRY(GetObjectRegistry(), iGraphics3D);
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = CS_QUERY_REGISTRY(GetObjectRegistry(), iEngine);
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = CS_QUERY_REGISTRY(GetObjectRegistry(), iVirtualClock);
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  vfs = CS_QUERY_REGISTRY(GetObjectRegistry(), iVFS);
  if (!vfs) return ReportError("Failed to locate Virtual FileSystem!");

  kbd = CS_QUERY_REGISTRY(GetObjectRegistry(), iKeyboardDriver);
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = CS_QUERY_REGISTRY(GetObjectRegistry(), iLoader);
  if (!loader) return ReportError("Failed to locate Loader!");

  saver = CS_QUERY_REGISTRY(GetObjectRegistry(), iSaver);
  if (!saver) return ReportError("Failed to locate Saver!");

  cegui = CS_QUERY_REGISTRY(GetObjectRegistry(), iCEGUI);
  if (!cegui) return ReportError("Failed to locate CEGUI plugin");
  
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  engine->SetLightingCacheMode (0);

  CreateRoom();
  CreateGui ();

  HandleCommandLine();

  engine->Prepare ();

  rotY = rotX = 0;

  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 1, -3));

  x = g3d->GetDriver2D ()->GetWidth ();
  y = g3d->GetDriver2D ()->GetHeight ();

  Run();

  return true;
}

void ViewMesh::CreateRoom ()
{
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError("Error loading 'stone4' texture!");

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");

  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingFactoryState> walls_state = 
    scfQueryInterface<iThingFactoryState> (walls->GetMeshObject ()->GetFactory());
  walls_state->AddInsideBox (csVector3 (-roomsize, -roomsize/2, -roomsize),
    csVector3 (roomsize, 3*roomsize/2, roomsize));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight
    (0, csVector3(-roomsize/2, roomsize/2, 0), 2*roomsize, csColor(1, 1, 1));
  ll->Add (light);

  light = engine->CreateLight
    (0, csVector3(roomsize/2, roomsize/2,  0), 2*roomsize, csColor(1, 1, 1));
  ll->Add (light);

  light = engine->CreateLight
    (0, csVector3(0, roomsize/2, -roomsize/2), 2*roomsize, csColor(1, 1, 1));
  ll->Add (light);
}

void ViewMesh::CreateGui()
{
  // Initialize CEGUI wrapper
  cegui->Initialize ();

  // Set the logging level
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

#if (CEGUI_VERSION_MAJOR == 0) && (CEGUI_VERSION_MINOR >= 5)
  // Use the 0.5 version of the skin
  vfs->ChDir ("/ceguitest/0.5/");
#else
  // Use the old version of the skin
  vfs->ChDir ("/ceguitest/");
#endif

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->loadScheme("ice.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("ice", "MouseArrow");

#if (CEGUI_VERSION_MAJOR == 0) && (CEGUI_VERSION_MINOR >= 5)
  CEGUI::Font* font = cegui->GetFontManagerPtr ()->createFont("FreeType",
    "Vera", "/fonts/ttf/Vera.ttf");
  font->setProperty("PointSize", "10");
  font->load();
#else
  cegui->GetFontManagerPtr ()->createFont("Vera", "/fonts/ttf/Vera.ttf", 10,
    CEGUI::Default);
#endif

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("viewmesh.layout"));

  form = winMgr->getWindow("Form");
  stddlg = winMgr->getWindow("StdDlg");

  CEGUI::Window* btn = 0;
  // ----[ GENERAL ]---------------------------------------------------------

  btn = winMgr->getWindow("Tab1/SaveButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::SaveButton, this));

  btn = winMgr->getWindow("Tab1/LoadButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::LoadButton, this));

  btn = winMgr->getWindow("Tab1/SaveBinaryButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::SaveBinaryButton, this));

  btn = winMgr->getWindow("Tab1/LoadLibButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::LoadLibButton, this));

  btn = winMgr->getWindow("Tab1/NormalMovementRadio");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&ViewMesh::CameraModeMoveNormal, this));

  btn = winMgr->getWindow("Tab1/LooktooriginRadio");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&ViewMesh::CameraModeMoveOrigin, this));

  btn = winMgr->getWindow("Tab1/RotateRadio");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&ViewMesh::CameraModeRotate, this));

  btn = winMgr->getWindow("Tab1/ScaleSprite");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ViewMesh::SetScaleSprite, this));

  // ----[ ANIMATION ]-------------------------------------------------------

  btn = winMgr->getWindow("Tab2/ReverseAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::ReversAnimation, this));

  btn = winMgr->getWindow("Tab2/StopAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::StopAnimation, this));

  btn = winMgr->getWindow("Tab2/SlowerAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::SlowerAnimation, this));

  btn = winMgr->getWindow("Tab2/AddAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::AddAnimation, this));

  btn = winMgr->getWindow("Tab2/FasterAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::FasterAnimation, this));

  btn = winMgr->getWindow("Tab2/SetAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::SetAnimation, this));

  btn = winMgr->getWindow("Tab2/RemoveAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::RemoveAnimation, this));

  btn = winMgr->getWindow("Tab2/ClearAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::ClearAnimation, this));

  btn = winMgr->getWindow("Tab2/List");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&ViewMesh::SelAnimation, this));

  // ----[ SOCKET ]----------------------------------------------------------

  btn = winMgr->getWindow("Tab3/RotX/Input");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ViewMesh::SetRotX, this));

  btn = winMgr->getWindow("Tab3/RotY/Input");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ViewMesh::SetRotY, this));

  btn = winMgr->getWindow("Tab3/RotZ/Input");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ViewMesh::SetRotZ, this));

  btn = winMgr->getWindow("Tab3/Mesh/Input");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ViewMesh::SetMesh, this));

  btn = winMgr->getWindow("Tab3/Sub/Input");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ViewMesh::SetSubMesh, this));

  btn = winMgr->getWindow("Tab3/Tria/Input");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ViewMesh::SetTriangle, this));

  btn = winMgr->getWindow("Tab3/AttachButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::AttachButton, this));

  btn = winMgr->getWindow("Tab3/DetachButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::DetachButton, this));

  btn = winMgr->getWindow("Tab3/AddSocket");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::AddSocket, this));

  btn = winMgr->getWindow("Tab3/DelSocket");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::DelSocket, this));

  btn = winMgr->getWindow("Tab3/List");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&ViewMesh::SelSocket, this));

  btn = winMgr->getWindow("Tab3/RenameSocket");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::RenameSocket, this));

  // ----[ Morph ]----------------------------------------------------------

  btn = winMgr->getWindow("Tab4/List");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&ViewMesh::SelMorph, this));

  btn = winMgr->getWindow("Tab4/BlendButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::BlendButton, this));

  btn = winMgr->getWindow("Tab4/ClearButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::ClearButton, this));

  // ----[ STDDLG ]----------------------------------------------------------

  btn = winMgr->getWindow("StdDlg/OkButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::StdDlgOkButton, this));

  btn = winMgr->getWindow("StdDlg/CancleButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ViewMesh::StdDlgCancleButton, this));

  btn = winMgr->getWindow("StdDlg/FileSelect");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&ViewMesh::StdDlgFileSelect, this));

  btn = winMgr->getWindow("StdDlg/DirSelect");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&ViewMesh::StdDlgDirSelect, this));

  // ------------------------------------------------------------------------

  btn = winMgr->getWindow("StdDlg/Path");
  btn->setProperty("Text", vfs->GetCwd());
  StdDlgUpdateLists(vfs->GetCwd());
}

void ViewMesh::LoadSprite (const char* filename)
{
  if (spritewrapper)
  {
    if (sprite)
    {
      for (int i = 0; i < sprite->GetSocketCount(); i++)
      {
        iMeshWrapper* meshWrapOld = sprite->GetSocket(i)->GetMeshWrapper();
        engine->RemoveObject(meshWrapOld);
        engine->RemoveObject(meshWrapOld->GetFactory());
        delete meshWrapOld;
      }
    }
    else if (cal3dsprite)
    {
      for (int i = 0; i < cal3dsprite->GetSocketCount(); i++)
      {
        iMeshWrapper* meshWrapOld = 
          cal3dsprite->GetSocket(i)->GetMeshWrapper();

        if (meshWrapOld)
        {
          engine->RemoveObject(meshWrapOld);
          engine->RemoveObject(meshWrapOld->GetFactory());
          delete meshWrapOld; 
        }
      }
    }
#ifdef CS_HAVE_CAL3D
    if (cal3dsprite && callback)
    {
      cal3dsprite->RemoveAnimCallback("walk", callback);
      delete callback;
      callback = 0;
    }
#endif
    engine->RemoveObject(spritewrapper);
    engine->RemoveObject(spritewrapper->GetFactory());
    spritewrapper = 0;
    sprite = 0;
    cal3dsprite = 0;
    state = 0;
    cal3dstate = 0;
    selectedSocket = 0;
    selectedCal3dSocket = 0;
    selectedAnimation = 0;
    selectedMorphTarget = 0;
    meshTx = meshTy = meshTz = 0;
  }

  iBase* result;
  printf ("Loading model '%s' from vfs dir '%s'\n",
		  filename, vfs->GetCwd ()); fflush (stdout);
  iRegion* region = engine->CreateRegion ("viewmesh_region");
  bool rc = loader->Load (filename, result, region, false, true);

  if (!rc)
    return;

  csRef<iMeshFactoryWrapper> wrap;
  if (result == 0)
  {
    // Library file. Find the first factory in our region.
    iMeshFactoryList* factories = engine->GetMeshFactories ();
    int i;
    for (i = 0 ; i < factories->GetCount () ; i++)
    {
      iMeshFactoryWrapper* f = factories->Get (i);
      if (region->IsInRegion (f->QueryObject ()))
      {
        wrap = f;
        break;
      }
    }
  }
  else
  {
    wrap = SCF_QUERY_INTERFACE (result,iMeshFactoryWrapper);
  }

  if (!wrap) return;

  if (wrap) 
  {
    csRef<iMeshObjectFactory> fact = wrap->GetMeshObjectFactory();
    if (fact)
    {
      csVector3 v(0, 0, 0);
      spritewrapper = engine->CreateMeshWrapper(wrap, "MySprite", room, v);

      cal3dsprite = SCF_QUERY_INTERFACE(fact, iSpriteCal3DFactoryState);
      sprite = SCF_QUERY_INTERFACE(fact, iSprite3DFactoryState);
      if (cal3dsprite || sprite)
      {
        iMeshObject* mesh = spritewrapper->GetMeshObject();
        cal3dstate = SCF_QUERY_INTERFACE(mesh, iSpriteCal3DState);
        state = SCF_QUERY_INTERFACE(mesh, iSprite3DState);
      }
      if (cal3dstate)
      {
#ifdef CS_HAVE_CAL3D
        vmAnimCallback *callback = new vmAnimCallback;
        cal3dsprite->RegisterAnimCallback("walk",callback,.5);
#endif
      }
    }
  }

  ScaleSprite (scale);

  if (spritewrapper)
  {
    csBox3 box;
    box = spritewrapper->GetWorldBoundingBox();
    csVector3 sprpos = box.GetCenter();
    csVector3 campos = view->GetCamera ()->GetTransform ().GetOrigin();
    view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (campos.x, sprpos.y, campos.z));
    camMode = rotateorigin;
  }

  UpdateSocketList();
  UpdateAnimationList();
  UpdateMorphList ();
}

void ViewMesh::SaveSprite (const char* filename, bool binary)
{
  csRef<iDocumentSystem> xml(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();

  iMeshFactoryWrapper* meshfactwrap = spritewrapper->GetFactory();
  iMeshObjectFactory*  meshfact = meshfactwrap->GetMeshObjectFactory();

  //Create the Tag for the MeshObj
  csRef<iDocumentNode> factNode = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  factNode->SetValue("meshfact");

  //Add the mesh's name to the MeshObj tag
  const char* name = meshfactwrap->QueryObject()->GetName();
  if (name && *name)
    factNode->SetAttribute("name", name);

  csRef<iFactory> factory = 
    SCF_QUERY_INTERFACE(meshfact->GetMeshObjectType(), iFactory);

  const char* pluginname = factory->QueryClassID();

  if (!(pluginname && *pluginname)) return;

  csRef<iDocumentNode> pluginNode = factNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  pluginNode->SetValue("plugin");

  //Add the plugin tag
  char loadername[128] = "";
  csReplaceAll(loadername, pluginname, ".object.", ".loader.factory.",
    sizeof(loadername));

  if (binary)
    strcat(loadername, ".binary");

  pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(loadername);
  csRef<iPluginManager> plugin_mgr = 
    csQueryRegistry<iPluginManager> (GetObjectRegistry ());

  char savername[128] = "";

  csReplaceAll(savername, pluginname, ".object.", ".saver.factory.",
    sizeof(savername));

  if (binary)
    strcat(savername, ".binary");

  //Invoke the iSaverPlugin::WriteDown
  if (binary)
  {
    csRef<iString> fname (new scfString(filename));
    fname->Append(".binary", 7);

    csRef<iFile> file (vfs->Open(*fname, VFS_FILE_WRITE));

    csRef<iDocumentNode> paramsNode = 
      factNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    paramsNode->SetValue("paramsfile");

    csRef<iDocumentNode> paramsdataNode = 
      paramsNode->CreateNodeBefore(CS_NODE_TEXT, 0);

    paramsdataNode->SetValue(*fname);

    csRef<iBinarySaverPlugin> saver = csLoadPluginCheck<iBinarySaverPlugin> (
    	plugin_mgr, savername);
    if (saver)
      saver->WriteDown(meshfact, file, 0/*ssource*/);
  }
  else
  {
    csRef<iSaverPlugin> saver =  csLoadPluginCheck<iSaverPlugin> (
        plugin_mgr, savername);
    if (saver) 
      saver->WriteDown(meshfact, factNode, 0/*ssource*/);
  }
  scfString str;
  doc->Write(&str);
  vfs->WriteFile(filename, str.GetData(), str.Length());
}

void ViewMesh::AttachMesh (const char* file)
{
  if (selectedSocket)
  {
    csRef<iMeshWrapper> meshWrapOld = selectedSocket->GetMeshWrapper();
    if ( meshWrapOld )
    {
      meshWrapOld->QuerySceneNode ()->SetParent (0);
      selectedSocket->SetMeshWrapper( 0 );
    }
  }
  else if (selectedCal3dSocket)
  {
    csRef<iMeshWrapper> meshWrapOld = selectedCal3dSocket->GetMeshWrapper();
    if ( meshWrapOld )
    {
      meshWrapOld->QuerySceneNode ()->SetParent (0);
      selectedCal3dSocket->SetMeshWrapper( 0 );    
    }
  }

  iBase* result;
  iRegion* region = engine->CreateRegion ("viewmesh_region");
  bool rc = loader->Load (file, result, region, false, true);

  if (!rc)
    return;

  csRef<iMeshFactoryWrapper> factory;
  if (result == 0)
  {
    // Library file. Find the first factory in our region.
    iMeshFactoryList* factories = engine->GetMeshFactories ();
    int i;
    for (i = 0 ; i < factories->GetCount () ; i++)
    {
      iMeshFactoryWrapper* f = factories->Get (i);
      if (region->IsInRegion (f->QueryObject ()))
      {
        factory = f;
        break;
      }
    }
  }
  else
  {
    factory = SCF_QUERY_INTERFACE (result,iMeshFactoryWrapper);
  }

  if (!factory) return;

  csRef<iMeshWrapper> meshWrap = engine->CreateMeshWrapper(factory, file);
  csReversibleTransform t;

  if (selectedSocket)
  {
    meshWrap->QuerySceneNode ()->SetParent (spritewrapper->QuerySceneNode ());
    selectedSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
  }
  else if (selectedCal3dSocket)
  {
    selectedCal3dSocket->SetTransform(t);
    meshWrap->QuerySceneNode ()->SetParent (spritewrapper->QuerySceneNode ());
    selectedCal3dSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
  }
}

void ViewMesh::UpdateSocketList ()
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Tab3/List");

  list->resetList();

  if (sprite)
  {
    for (int i = 0; i < sprite->GetSocketCount(); i++)
    {
      iSpriteSocket* sock = sprite->GetSocket(i);
      if (!sock) continue;

      if (i==0) SelectSocket(sock->GetName());

      CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(sock->GetName());
      item->setTextColours(CEGUI::colour(0,0,0));
      item->setSelectionBrushImage("ice", "TextSelectionBrush");
      item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
      list->addItem(item);
    }
  }
  else if (cal3dsprite)
  {
    for (int i = 0; i < cal3dsprite->GetSocketCount(); i++)
    {
      iSpriteCal3DSocket* sock = cal3dsprite->GetSocket(i);
      if (!sock) continue;

      if (i==0) SelectSocket(sock->GetName());

      CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(sock->GetName());
      item->setTextColours(CEGUI::colour(0,0,0));
      item->setSelectionBrushImage("ice", "TextSelectionBrush");
      item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
      list->addItem(item);
    }
  }
}

void ViewMesh::UpdateAnimationList ()
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Tab2/List");

  list->resetList();

  CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem("default");
  item->setTextColours(CEGUI::colour(0,0,0));
  item->setSelectionBrushImage("ice", "TextSelectionBrush");
  item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
  list->addItem(item);

  if (sprite)
  {
    for (int i = 0; i < sprite->GetActionCount(); i++)
    {
      iSpriteAction* action = sprite->GetAction(i);
      if (!action) continue;

      item = new CEGUI::ListboxTextItem(action->GetName());
      item->setTextColours(CEGUI::colour(0,0,0));
      item->setSelectionBrushImage("ice", "TextSelectionBrush");
      item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
      list->addItem(item);
    }
  }
  else if (cal3dsprite)
  {
    for (int i = 0; i < cal3dstate->GetAnimCount(); i++)
    {
      const char* animname = cal3dstate->GetAnimName(i);
      if (!animname) continue;

      item = new CEGUI::ListboxTextItem(animname);
      item->setTextColours(CEGUI::colour(0,0,0));
      item->setSelectionBrushImage("ice", "TextSelectionBrush");
      item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
      list->addItem(item);
    }
  }
}

void ViewMesh::UpdateMorphList ()
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Tab4/List");

  list->resetList();

  CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem("default");
  item->setTextColours(CEGUI::colour(0,0,0));
  item->setSelectionBrushImage("ice", "TextSelectionBrush");
  item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
  list->addItem(item);

  if (cal3dsprite)
  {
    for (int i = 0; i < cal3dsprite->GetMorphAnimationCount(); i++)
    {
      const char* morphname = cal3dsprite->GetMorphAnimationName(i);
      if (!morphname) continue;

      item = new CEGUI::ListboxTextItem(morphname);
      item->setTextColours(CEGUI::colour(0,0,0));
      item->setSelectionBrushImage("ice", "TextSelectionBrush");
      item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
      list->addItem(item);
    }
  }
}

void ViewMesh::SelectSocket (const char* newsocket)
{
  if (state)
  {
    iSpriteSocket* sock = state->FindSocket(newsocket);
    if (selectedSocket == sock) return;
    selectedSocket = sock;
  }
  else if (cal3dstate)
  {
    iSpriteCal3DSocket* sock = cal3dstate->FindSocket(newsocket);
    if (selectedCal3dSocket == sock) return;
    selectedCal3dSocket = sock;
  }
  UpdateSocket();
}

void ViewMesh::UpdateSocket ()
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  if (selectedSocket)
  {
    CEGUI::Window* InputName = winMgr->getWindow("Tab3/RenameSocket/Input");
    const char* name = selectedSocket->GetName();
    InputName->setProperty("Text", name);

    CEGUI::Window* InputTriangle = winMgr->getWindow("Tab3/Tria/Input");
    csRef<iString> valueTriangle(new scfString());
    valueTriangle->Format("%d", selectedSocket->GetTriangleIndex());
    InputTriangle->setProperty("Text", valueTriangle->GetData());
  }
  else if (selectedCal3dSocket)
  {
    CEGUI::Window* InputName = winMgr->getWindow("Tab3/RenameSocket/Input");
    const char* name = selectedCal3dSocket->GetName();
    InputName->setProperty("Text", name);

    CEGUI::Window* InputMesh = winMgr->getWindow("Tab3/Mesh/Input");
    csRef<iString> valueMesh(new scfString());
    valueMesh->Format("%d", selectedCal3dSocket->GetMeshIndex());
    InputMesh->setProperty("Text", valueMesh->GetData());

    CEGUI::Window* InputSubMesh = winMgr->getWindow("Tab3/Sub/Input");
    csRef<iString> valueSubmesh(new scfString());
    valueSubmesh->Format("%d", selectedCal3dSocket->GetSubmeshIndex());
    InputSubMesh->setProperty("Text", valueSubmesh->GetData());

    CEGUI::Window* InputTriangle = winMgr->getWindow("Tab3/Tria/Input");
    csRef<iString> valueTriangle(new scfString());
    valueTriangle->Format("%d", selectedCal3dSocket->GetTriangleIndex());
    InputTriangle->setProperty("Text", valueTriangle->GetData());
  }
}

void ViewMesh::ScaleSprite (float newScale)
{
  csMatrix3 scalingHt; scalingHt.Identity(); scalingHt *= scale/newScale;
  csReversibleTransform rTH;
  rTH.SetT2O (scalingHt);
  if (spritewrapper)
    spritewrapper->HardTransform (rTH);

  csMatrix3 scaling; scaling.Identity(); scaling /= newScale;
  csReversibleTransform rT;
  rT.SetT2O (scaling);
  if (spritewrapper)
    spritewrapper->GetMovable()->SetTransform(rT);

  scale = newScale;

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab1/ScaleSprite");
  csRef<iString> valueMesh(new scfString());
  valueMesh->Format("%.2f", scale);
  component->setProperty("Text", valueMesh->GetData());
}

//---------------------------------------------------------------------------

bool ViewMesh::ReversAnimation (const CEGUI::EventArgs& e)
{

  if (cal3dstate)
  {
    cal3dstate->SetAnimationTime(-1);
  }
  else if (state)
  {
    state->SetReverseAction(state->GetReverseAction()^true);
  }
  return true;
}

bool ViewMesh::StopAnimation (const CEGUI::EventArgs& e)
{
  move_sprite_speed = 0;
  return true;
}

bool ViewMesh::SlowerAnimation (const CEGUI::EventArgs& e)
{
  move_sprite_speed -= 0.5f;
  return true;
}

bool ViewMesh::AddAnimation (const CEGUI::EventArgs& e)
{
  if (cal3dstate)
  {
    if (!selectedAnimation) return false;
    int anim = cal3dstate->FindAnim(selectedAnimation);
    cal3dstate->AddAnimCycle(anim,1,3);
  }
  return true;
}

bool ViewMesh::FasterAnimation (const CEGUI::EventArgs& e)
{
  move_sprite_speed += 0.5f;
  return true;
}

bool ViewMesh::SetAnimation (const CEGUI::EventArgs& e)
{
  if (cal3dstate)
  {
    if (!selectedAnimation) return false;
    int anim = cal3dstate->FindAnim(selectedAnimation);
    cal3dstate->SetAnimAction(anim,1,1);
  }
  else if (state)
  {
    if (!selectedAnimation) return false;
    state->SetAction(selectedAnimation);
  }
  return true;
}

bool ViewMesh::RemoveAnimation (const CEGUI::EventArgs& e)
{
  //TODO: Implement it.

  ReportWarning("Removal of Animation is not yet implemented");
  return true;
}

bool ViewMesh::ClearAnimation (const CEGUI::EventArgs& e)
{
  if (cal3dstate)
  {
    if (!selectedAnimation) return false;
    int anim = cal3dstate->FindAnim(selectedAnimation);
    cal3dstate->ClearAnimCycle(anim,3);
  }
  return true;
}

bool ViewMesh::SelAnimation (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Tab2/List");

  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  const CEGUI::String& text = item->getText();
  if (text.empty()) return false;

  selectedAnimation = text.c_str();
  return true;
}

//---------------------------------------------------------------------------

bool ViewMesh::SetMesh (const CEGUI::EventArgs& e)
{
  if (!selectedCal3dSocket) return false;

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab3/Mesh/Input");
  CEGUI::String text = component->getProperty("Text");

  if (!text.c_str()) return false;

  int i;
  if (sscanf(text.c_str(),"%d", &i) != 1) return false;

  selectedCal3dSocket->SetMeshIndex(i);
  UpdateSocket();
  return true;
}

bool ViewMesh::SetSubMesh (const CEGUI::EventArgs& e)
{
  if (!selectedCal3dSocket) return false;

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab3/Sub/Input");
  CEGUI::String text = component->getProperty("Text");

  if (!text.c_str()) return false;

  int i;
  if (sscanf(text.c_str(),"%d", &i) != 1) return false;

  selectedCal3dSocket->SetSubmeshIndex(i);
  UpdateSocket();
  return true;
}

bool ViewMesh::SetTriangle (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab3/Tria/Input");
  CEGUI::String text = component->getProperty("Text");

  if (!text.c_str()) return false;

  int i;
  if (sscanf(text.c_str(),"%d", &i) != 1) return false;

  if (selectedCal3dSocket)
    selectedCal3dSocket->SetTriangleIndex(i);
  else if (selectedSocket)
    selectedSocket->SetTriangleIndex(i);

  UpdateSocket();
  return true;
}

bool ViewMesh::SetRotX (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab3/RotX/Input");
  CEGUI::String text = component->getProperty("Text");

  if (!text.c_str()) return false;

  float f;
  if (sscanf(text.c_str(),"%f", &f) != 1) return false;

  if (selectedCal3dSocket && selectedCal3dSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = selectedCal3dSocket->GetMeshWrapper();
    meshWrap->QuerySceneNode ()->SetParent (0);
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-meshTz);
    Tr.RotateOther(csVector3(0,1,0),-meshTy);
    Tr.RotateOther(csVector3(1,0,0),-meshTx);
    Tr.RotateOther(csVector3(1,0,0),f);
    Tr.RotateOther(csVector3(0,1,0),meshTy);
    Tr.RotateOther(csVector3(0,0,1),meshTz);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    meshWrap->QuerySceneNode ()->SetParent (spritewrapper
    	->QuerySceneNode ());
    selectedCal3dSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
    meshTx = f;
  }
  else if (selectedSocket && selectedSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = selectedSocket->GetMeshWrapper();
    meshWrap->QuerySceneNode ()->SetParent (0);
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-meshTz);
    Tr.RotateOther(csVector3(0,1,0),-meshTy);
    Tr.RotateOther(csVector3(1,0,0),-meshTx);
    Tr.RotateOther(csVector3(1,0,0),f);
    Tr.RotateOther(csVector3(0,1,0),meshTy);
    Tr.RotateOther(csVector3(0,0,1),meshTz);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    meshWrap->QuerySceneNode ()->SetParent (spritewrapper
    	->QuerySceneNode ());
    selectedSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
    meshTx = f;
  }
  return true;
}

bool ViewMesh::SetRotY (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab3/RotY/Input");
  CEGUI::String text = component->getProperty("Text");

  if (!text.c_str()) return false;

  float f;
  if (sscanf(text.c_str(),"%f", &f) != 1) return false;

  if (selectedCal3dSocket && selectedCal3dSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = selectedCal3dSocket->GetMeshWrapper();
    meshWrap->QuerySceneNode ()->SetParent (0);
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-meshTz);
    Tr.RotateOther(csVector3(0,1,0),-meshTy);
    Tr.RotateOther(csVector3(1,0,0),-meshTx);
    Tr.RotateOther(csVector3(1,0,0),meshTx);
    Tr.RotateOther(csVector3(0,1,0),f);
    Tr.RotateOther(csVector3(0,0,1),meshTz);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    meshWrap->QuerySceneNode ()->SetParent (spritewrapper
    	->QuerySceneNode ());
    selectedCal3dSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
    meshTy = f;
  }
  else if (selectedSocket && selectedSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = selectedSocket->GetMeshWrapper();
    meshWrap->QuerySceneNode ()->SetParent (0);
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-meshTz);
    Tr.RotateOther(csVector3(0,1,0),-meshTy);
    Tr.RotateOther(csVector3(1,0,0),-meshTx);
    Tr.RotateOther(csVector3(1,0,0),meshTx);
    Tr.RotateOther(csVector3(0,1,0),f);
    Tr.RotateOther(csVector3(0,0,1),meshTz);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    meshWrap->QuerySceneNode ()->SetParent (spritewrapper
    	->QuerySceneNode ());
    selectedSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
    meshTy = f;
  }
  return true;
}

bool ViewMesh::SetRotZ (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab3/RotZ/Input");
  CEGUI::String text = component->getProperty("Text");

  if (!text.c_str()) return false;

  float f;
  if (sscanf(text.c_str(),"%f", &f) != 1) return false;

  if (selectedCal3dSocket && selectedCal3dSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = selectedCal3dSocket->GetMeshWrapper();
    meshWrap->QuerySceneNode ()->SetParent (0);
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-meshTz);
    Tr.RotateOther(csVector3(0,1,0),-meshTy);
    Tr.RotateOther(csVector3(1,0,0),-meshTx);
    Tr.RotateOther(csVector3(1,0,0),meshTx);
    Tr.RotateOther(csVector3(0,1,0),meshTy);
    Tr.RotateOther(csVector3(0,0,1),f);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    meshWrap->QuerySceneNode ()->SetParent (spritewrapper
    	->QuerySceneNode ());
    selectedCal3dSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
    meshTz = f;
  }
  else if (selectedSocket && selectedSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = selectedSocket->GetMeshWrapper();
    meshWrap->QuerySceneNode ()->SetParent (0);
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-meshTz);
    Tr.RotateOther(csVector3(0,1,0),-meshTy);
    Tr.RotateOther(csVector3(1,0,0),-meshTx);
    Tr.RotateOther(csVector3(1,0,0),meshTx);
    Tr.RotateOther(csVector3(0,1,0),meshTy);
    Tr.RotateOther(csVector3(0,0,1),f);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    meshWrap->QuerySceneNode ()->SetParent (spritewrapper
    	->QuerySceneNode ());
    selectedSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
    meshTz = f;
  }
  return true;
}

bool ViewMesh::AttachButton (const CEGUI::EventArgs& e)
{
  form->hide();
  stddlg->show();
  stddlgPurpose=attach;
  return true;
}

bool ViewMesh::DetachButton (const CEGUI::EventArgs& e)
{
  csRef<iMeshWrapper> meshWrapOld;
  if (selectedCal3dSocket)
    meshWrapOld = selectedCal3dSocket->GetMeshWrapper();
  else if (selectedSocket)
    meshWrapOld = selectedSocket->GetMeshWrapper();
  
  if (!meshWrapOld ) return false;

  meshWrapOld->QuerySceneNode ()->SetParent (0);

  engine->RemoveObject(meshWrapOld);
  engine->RemoveObject(meshWrapOld->GetFactory());

  if (selectedCal3dSocket)
    selectedCal3dSocket->SetMeshWrapper( 0 );    
  else if (selectedSocket)
    selectedSocket->SetMeshWrapper( 0 );    
  return true;
}

bool ViewMesh::AddSocket (const CEGUI::EventArgs& e)
{
  ReportWarning("Adding sockets is not yet implemented");

  if (cal3dsprite)
  {
    //cal3dsprite->AddSocket()->SetName("NewSocket");
    //cal3dstate->AddSocket()->SetName("NewSocket");
    //SelectSocket("NewSocket");
  }
  else if (sprite)
  {
    //iSpriteSocket* newsocket = sprite->AddSocket();
    //newsocket->SetName("NewSocket");
    //SelectSocket(newsocket->GetName());
  }
  UpdateSocketList();
  return true;
}

bool ViewMesh::DelSocket (const CEGUI::EventArgs& e)
{
  //Change API of iSpriteCal3DFactoryState to enable this!

  ReportWarning("Deleting sockets is not yet implemented");
  //socket->DelSocket(selectedCal3dSocket);
  //selectedCal3dSocket = 0;
  UpdateSocketList();
  return true;
}

bool ViewMesh::SelSocket (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Tab3/List");

  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  const CEGUI::String& text = item->getText();
  if (text.empty()) return false;

  SelectSocket(text.c_str());
  return true;
}


bool ViewMesh::RenameSocket (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();
  CEGUI::Window* textfield = winMgr->getWindow("Tab3/RenameSocket/Input");

  CEGUI::String text = textfield->getProperty("Text");

  if (text.empty()) return false;

  if (selectedSocket)
  {
    selectedSocket->SetName(text.c_str());
  }
  else if (selectedCal3dSocket)
  {
    const char* name = selectedCal3dSocket->GetName();
    cal3dsprite->FindSocket(name)->SetName(text.c_str());
    cal3dstate->FindSocket(name)->SetName(text.c_str());
    selectedCal3dSocket = cal3dsprite->FindSocket(text.c_str());
  }

  UpdateSocketList();
  return true;
}

//---------------------------------------------------------------------------

bool ViewMesh::CameraModeRotate (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("Tab1/RotateRadio");

  if (radio->getSelectedButtonInGroup() == radio)
    camMode = rotateorigin;
  return true;
}

bool ViewMesh::CameraModeMoveOrigin (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("Tab1/LooktooriginRadio");

  if (radio->getSelectedButtonInGroup() == radio)
    camMode = moveorigin;
  return true;
}

bool ViewMesh::CameraModeMoveNormal (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("Tab1/NormalMovementRadio");

  if (radio->getSelectedButtonInGroup() == radio)
    camMode = movenormal;
  return true;
}

bool ViewMesh::LoadButton (const CEGUI::EventArgs& e)
{
  form->hide();
  stddlg->show();
  stddlgPurpose=load;
  return true;
}

bool ViewMesh::LoadLibButton (const CEGUI::EventArgs& e)
{
  form->hide();
  stddlg->show();
  stddlgPurpose=loadlib;
  return true;
}

bool ViewMesh::SaveButton (const CEGUI::EventArgs& e)
{
  form->hide();
  stddlg->show();
  stddlgPurpose=save;
  return true;
}

bool ViewMesh::SaveBinaryButton (const CEGUI::EventArgs& e)
{
  form->hide();
  stddlg->show();
  stddlgPurpose=savebinary;
  return true;
}

bool ViewMesh::SetScaleSprite (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Editbox* component = (CEGUI::Editbox*)winMgr->getWindow("Tab1/ScaleSprite");
  const CEGUI::String& text = component->getText();

  if (text.empty()) return false;

  float f;
  if (sscanf(text.c_str(),"%f", &f) != 1) return false;

  ScaleSprite(f);
  return true;
}

//---------------------------------------------------------------------------
bool ViewMesh::SelMorph (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Tab4/List");

  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  const CEGUI::String& text = item->getText();
  if (text.empty()) return false;

  selectedMorphTarget = text.c_str();
  return true;
}

bool ViewMesh::BlendButton (const CEGUI::EventArgs& e)
{
  if (!cal3dstate) return false;

  float weight=1, delay=1;

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab4/WeightInput");
  CEGUI::String Sweight = component->getProperty("Text");

  if (! Sweight.empty())
  {
    if(sscanf(Sweight.c_str(), "%f", &weight) != 1) weight = 1;
  }

  component = winMgr->getWindow("Tab4/DelayInput");
  CEGUI::String Sdelay = component->getProperty("Text");
  if (! Sdelay.empty())
  {
    if(sscanf(Sdelay.c_str(), "%f", &delay) != 1) delay = 1;
  }

  int target =
    cal3dsprite->FindMorphAnimationName(selectedMorphTarget);

  if (target == -1) return false;

  cal3dstate->BlendMorphTarget(target, weight, delay);
  return true;
}

bool ViewMesh::ClearButton (const CEGUI::EventArgs& e)
{
  if (!cal3dstate) return false;

  float weight=1;

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("Tab4/WeightInput");
  CEGUI::String Sweight = component->getProperty("Text");

  if (! Sweight.empty())
  {
    if(sscanf(Sweight.c_str(), "%f", &weight) != 1) weight = 1;
  }

  int target =
    cal3dsprite->FindMorphAnimationName(selectedMorphTarget);

  if (target == -1) return false;

  cal3dstate->ClearMorphTarget(target, weight);
  return true;
}
//---------------------------------------------------------------------------

void ViewMesh::StdDlgUpdateLists(const char* filename)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* dirlist = (CEGUI::Listbox*)winMgr->getWindow("StdDlg/DirSelect");
  CEGUI::Listbox* filelist = (CEGUI::Listbox*)winMgr->getWindow("StdDlg/FileSelect");

  dirlist->resetList();
  filelist->resetList();

  CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem("..");
  item->setTextColours(CEGUI::colour(0,0,0));
  //item->setSelectionBrushImage("ice", "TextSelectionBrush");
  //item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
  dirlist->addItem(item);

  csRef<iStringArray> files = vfs->FindFiles(filename);
  
  for (size_t i = 0; i < files->Length(); i++)
  {
    char* file = (char*)files->Get(i);
    if (!file) continue;

    size_t dirlen = strlen(file);
    if (dirlen)
      dirlen--;
    while (dirlen && file[dirlen-1]!= '/')
      dirlen--;
    file=file+dirlen;

    if (file[strlen(file)-1] == '/')
    {
      file[strlen(file)-1]='\0';
      CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(file);
      item->setTextColours(CEGUI::colour(0,0,0));
      //item->setSelectionBrushImage("ice", "TextSelectionBrush");
      //item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
      dirlist->addItem(item);
    }
    else
    {
      CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem(file);
      item->setTextColours(CEGUI::colour(0,0,0));
      //item->setSelectionBrushImage("ice", "TextSelectionBrush");
      //item->setSelectionColours(CEGUI::colour(0.5f,0.5f,1));
      filelist->addItem(item);
    }
  }
}

//---------------------------------------------------------------------------

bool ViewMesh::StdDlgOkButton (const CEGUI::EventArgs& e)
{
  form->show();
  stddlg->hide();

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* inputpath = winMgr->getWindow("StdDlg/Path");
  CEGUI::String path = inputpath->getProperty("Text");
  if (path.empty()) return false;

  vfs->ChDir (path.c_str());

  CEGUI::Window* inputfile = winMgr->getWindow("StdDlg/File");
  CEGUI::String file = inputfile->getProperty("Text");
  if (path.empty()) return false;

  switch (stddlgPurpose)
  {
  case save:
    SaveSprite(file.c_str(), false);
    break;
  case savebinary:
    SaveSprite(file.c_str(), true);
    break;
  case load:
    LoadSprite(file.c_str());
    break;
  case loadlib:
    LoadLibrary(file.c_str());
    break;
  case attach:
    AttachMesh(file.c_str());
    break;
  }
  return true;
}

bool ViewMesh::StdDlgCancleButton (const CEGUI::EventArgs& e)
{
  form->show();
  stddlg->hide();
  return true;
}

bool ViewMesh::StdDlgFileSelect (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*) winMgr->getWindow("StdDlg/FileSelect");
  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  CEGUI::String text = item->getText();
  if (text.empty()) return false;

  CEGUI::Window* file = winMgr->getWindow("StdDlg/File");
  file->setProperty("Text", text);
  return true;
}

bool ViewMesh::StdDlgDirSelect (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*) winMgr->getWindow("StdDlg/DirSelect");
  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  CEGUI::String text = item->getText();
  if (text.empty()) return false;

  csPrintf("cd %s\n",text.c_str());

  CEGUI::Window* inputpath = winMgr->getWindow("StdDlg/Path");
  CEGUI::String path = inputpath->getProperty("Text");
  if (path.empty()) return false;

  csString newpath(path.c_str());

  if (csString("..") == text.c_str())
  {
    size_t i = newpath.Slice(0,newpath.Length()-1).FindLast('/')+1;
    csPrintf("%zu", i);
    newpath = newpath.Slice(0,i);
  }
  else
  {
    newpath.Append(text.c_str());
    newpath.Append("/");
  }

  if (!newpath.GetData()) newpath.Append("/");
  vfs->ChDir (newpath.GetData ());

  inputpath->setProperty("Text", newpath.GetData());
  StdDlgUpdateLists(newpath.GetData());
  return true;
}

//---------------------------------------------------------------------------

int main(int argc, char** argv)
{
  return ViewMesh().Main(argc, argv);
}
