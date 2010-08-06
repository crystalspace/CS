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
#include "csutil/scanstr.h"
#include "csutil/scfstr.h"
#include "csutil/stringconv.h"
#include "imesh/animesh.h"
#include "imesh/object.h"
#include "imesh/skeleton2anim.h"
#include "imesh/genmesh.h"
#include "iutil/eventq.h"
#include "iutil/object.h"
#include "iutil/stringarray.h"
#include "iengine/scenenode.h"
#include "iengine/renderloop.h"
#include "ivideo/graph2d.h"
#include "ivideo/material.h"
#include "csutil/scfstringarray.h"

#include "animeshasset.h"
#include "cal3dasset.h"
#include "genmeshasset.h"
#include "particlesasset.h"
#include "sprite3dasset.h"

#include "sockettab.h"
#include "animationtab.h"
#include "morphtargettab.h"
#include "submeshtab.h"
#include "materialtab.h"
#include "generaltab.h"
#include "particlestab.h"

// Hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#define CS_EVENT_HANDLED true
#define CS_EVENT_UNHANDLED false

CS_IMPLEMENT_APPLICATION

//---------------------------------------------------------------------------

ViewMesh::ViewMesh () : 
  camMode(rotateorigin), roomsize(5), scale(1), move_sprite_speed(0), lod_level(0), max_lod_level(0), auto_lod(false)
  {
    SetApplicationName ("CrystalSpace.ViewMesh");
  }

  ViewMesh::~ViewMesh ()
  {
  }

  void ViewMesh::Frame()
  {
    csTicks elapsed_time = vc->GetElapsedTicks ();
    float speed = (elapsed_time / 1000.0) * (0.06 * 20);

    iCamera* c = view->GetCamera();
    csVector3 orig = c->GetTransform().GetOrigin();

    if (asset)
    {
      switch (camMode)
      {
      case movenormal:
        {
          if (kbd->GetKeyState (CSKEY_SHIFT))
          {
            if (kbd->GetKeyState (CSKEY_UP))
              camTarget += c->GetTransform().This2OtherRelative(
              csVector3(0,1,0)) * 4 * speed;
            if (kbd->GetKeyState (CSKEY_DOWN))
              camTarget -= c->GetTransform().This2OtherRelative(
              csVector3(0,1,0)) * 4 * speed;
            if (kbd->GetKeyState (CSKEY_RIGHT))
              camTarget += c->GetTransform().This2OtherRelative(
              csVector3(1,0,0)) * 4 * speed;
            if (kbd->GetKeyState (CSKEY_LEFT))
              camTarget -= c->GetTransform().This2OtherRelative(
              csVector3(1,0,0)) * 4 * speed;
          }
          else
          {
            if (kbd->GetKeyState (CSKEY_UP))
              camTarget += (camTarget - orig).Unit() * 4 * speed;
            if (kbd->GetKeyState (CSKEY_DOWN))
              camTarget -= (camTarget - orig).Unit() * 4 * speed;
          }

          UpdateCamera();
          orig = c->GetTransform().GetOrigin();
          if (!kbd->GetKeyState (CSKEY_SHIFT))
          {
            if (kbd->GetKeyState (CSKEY_RIGHT))
              camYaw += speed;
            if (kbd->GetKeyState (CSKEY_LEFT))
              camYaw -= speed;
          }
          if (kbd->GetKeyState (CSKEY_PGUP))
            camPitch = csMin<float>(3.14159f * 0.5f - 0.01f, camPitch + speed);
          if (kbd->GetKeyState (CSKEY_PGDN))
            camPitch = csMax<float>(-3.14159f * 0.5f + 0.01f, camPitch - speed);

          UpdateCamera();
          csVector3 deltaOrig = c->GetTransform().GetOrigin() - orig;
          camTarget -= deltaOrig;
          UpdateCamera();
          break;
        }
      case moveorigin:
        {
          if (kbd->GetKeyState (CSKEY_DOWN))
            orig.z -= 4 * speed;
          if (kbd->GetKeyState (CSKEY_UP))
            orig.z += 4 * speed;
          if (kbd->GetKeyState (CSKEY_LEFT))
            orig.x -= 4 * speed;
          if (kbd->GetKeyState (CSKEY_RIGHT))
            orig.x += 4 * speed;
          if (kbd->GetKeyState (CSKEY_PGUP))
            orig.y += 4 * speed;
          if (kbd->GetKeyState (CSKEY_PGDN))
            orig.y -= 4 * speed;
          FixCameraForOrigin(orig);
          UpdateCamera();
          break;
        }
      case rotateorigin:
        {
          if (kbd->GetKeyState (CSKEY_LEFT))
            camYaw += speed;
          if (kbd->GetKeyState (CSKEY_RIGHT))
            camYaw -= speed;
          if (kbd->GetKeyState (CSKEY_UP))
            camPitch = csMin<float>(3.14159f * 0.5f - 0.01f, camPitch + speed);
          if (kbd->GetKeyState (CSKEY_DOWN))
            camPitch = csMax<float>(-3.14159f * 0.5f + 0.01f, camPitch - speed);
          if (kbd->GetKeyState (CSKEY_PGUP))
            camDist = csMax<float>(0.01f, camDist - speed * 4);
          if (kbd->GetKeyState (CSKEY_PGDN))
            camDist += speed * 4;
          UpdateCamera();
          break;
        }
      default:
        break;
      }

      if (kbd->GetKeyState (CSKEY_PGDN))
      {
        if (lod_level < max_lod_level)
          lod_level += 1;
      }
	    if (kbd->GetKeyState (CSKEY_PGUP))
      {
        if (lod_level > 0)
          lod_level -= 1;
      }
      
      if (!auto_lod)
      {
        csRef<iMeshObject> mobj = asset->GetMesh()->GetMeshObject();
        assert(mobj);

        csRef<iGeneralMeshState> mstate = scfQueryInterface<iGeneralMeshState>(mobj);
        if (mstate)
        {
            mstate->ForceProgLODLevel(lod_level);
        }
      }

      csRef<iMovable> mov = asset->GetMesh()->GetMovable();
      csVector3 pos = mov->GetFullPosition();    
      mov->MovePosition(csVector3(pos.x, pos.y, -move_sprite_speed*elapsed_time/1000.0f));
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

    if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS
      | CSDRAW_CLEARZBUFFER | CSDRAW_CLEARSCREEN))
      return;

    view->Draw ();

    if (loading)
      LoadSprite(reloadFilename, reloadFilePath);

    cegui->Render();
  }

  void ViewMesh::ResetCamera()
  {
    camTarget.Set(0,0,0);
    if (asset)
    {
      csBox3 box;
      box = asset->GetMesh()->GetWorldBoundingBox();
      camTarget = box.GetCenter();
    }

    camDist = 3.5f;
    camYaw = 0.0f;
    camPitch = -0.2f;
  }

void ViewMesh::UpdateCamera()
{
  csVector3 camPos;

  camPos.x = camTarget.x - camDist * (float)cos(camPitch) * (float)sin(camYaw);
  camPos.y = camTarget.y - camDist * (float)sin(camPitch);
  camPos.z = camTarget.z - camDist * (float)cos(camPitch) * (float)cos(camYaw);

  iCamera * c = view->GetCamera();
  c->GetTransform().SetOrigin(camPos);
  c->GetTransform().LookAt(camTarget - camPos, csVector3(0,1,0));
}

void ViewMesh::FixCameraForOrigin(const csVector3 & desiredOrigin)
{
  // calculate distance, yaw, and pitch values that will put the origin at the desired origin

  camDist = (camTarget - desiredOrigin).Norm();

  camPitch = (float)asin((camTarget.y - desiredOrigin.y) / camDist);

  camYaw = (float)asin((camTarget.x - desiredOrigin.x)
      / (camDist * (float)cos(camPitch)));
  if ((camTarget.z - desiredOrigin.z) / (camDist * (float)cos(camPitch)) < 0.0f)
      camYaw = 3.14159f - camYaw;
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
        csQueryRegistry<iEventQueue> (GetObjectRegistry());
      if (q.IsValid())
	q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
    }
  }
  return false;
}

bool ViewMesh::OnMouseDown (iEvent& e)
{
  const float mouseWheelZoomAmount = 0.25f;

  if (!asset)
    return false;

  uint button = csMouseEventHelper::GetButton(&e);
  switch (button)
  {
  case 0:
    camModePan = true;
    break;
  case 1:
    camModeRotate = true;
    break;
  case 2:
    camModeZoom = true;
    break;
  case 3:
    camDist = csMax<float>(0.1f, camDist - mouseWheelZoomAmount);
    UpdateCamera();
    break;
  case 4:
    camDist = csMax<float>(0.1f, camDist + mouseWheelZoomAmount);
    UpdateCamera();
    break;
  }
  return false;
}

bool ViewMesh::OnMouseUp (iEvent& e)
{
  if (!asset)
    return false;

  uint button = csMouseEventHelper::GetButton(&e);
  switch (button)
  {
  case 0:
    camModePan = false;
    break;
  case 1:
    camModeRotate = false;
    break;
  case 2:
    camModeZoom = false;
    break;
  }
  return false;
}

bool ViewMesh::OnMouseMove (iEvent& e)
{
  int x = csMouseEventHelper::GetX(&e);
  int y = csMouseEventHelper::GetY(&e);
  float dx = (float)(x - lastMouseX) * 0.02f;
  float dy = (float)(y - lastMouseY) * -0.02f;
  iCamera * c = view->GetCamera();

  lastMouseX = x;
  lastMouseY = y;

  if (camModePan)
  {
    camTarget += c->GetTransform().This2OtherRelative(csVector3(1,0,0)) * dx 
               + c->GetTransform().This2OtherRelative(csVector3(0,1,0)) * dy;
  }
  if (camModeRotate)
  {
    camYaw += dx;
    camPitch += dy;
  }
  if (camModeZoom)
  {
    camDist = csMax<float>(0.1f, camDist - (dx + dy));
  }

  if (camModePan || camModeRotate || camModePan)
    UpdateCamera();

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
  csPrintf ("\n");
  csPrintf ("Example:\n");
  csPrintf ("  viewmesh -C=data/frankie/ frankie.xml\n");
  csPrintf ("\n");
}

void ViewMesh::HandleCommandLine()
{
  csRef<iCommandLineParser> cmdline =
    csQueryRegistry<iCommandLineParser> (GetObjectRegistry());

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

  csString vfsDir = cmdline->GetOption("C");

  if (realPath)
  {
    vfs->Mount ("/tmp/viewmesh", realPath);
    vfs->ChDir ("/tmp/viewmesh");
  }

  if(vfsDir.IsEmpty() && meshfilename)
  {
    vfsDir = csString(meshfilename).Slice(0, csString(meshfilename).FindLast('/'));
  }

  if (!vfsDir.IsEmpty())
  {
    if(!vfs->ChDir (vfsDir))
    {
      ReportError("Cannot change to path: %s\n", vfsDir.GetData ());
    }
    else
    {
      // Update StdDlg path.
      CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();
      CEGUI::Window* window = winMgr->getWindow("StdDlg/Path");
      window->setProperty("Text", vfs->GetCwd());
      StdDlgUpdateLists(vfs->GetCwd());
    }
  }

  if (texturefilename && texturename)
  {
    LoadTexture(texturefilename, texturename);
  }

  if (meshfilename)
  {
    LoadSprite(meshfilename);
  }

  if (roomSize) roomsize = CS::Utility::strtof(roomSize);

  if (scaleTxt != 0)
  {
    float newScale;
    csScanStr (scaleTxt, "%f", &newScale);
    ScaleSprite(newScale);
  }
}

void ViewMesh::LoadTexture(const char* file, const char* name)
{
  if (file && name)
  {
    iTextureWrapper* txt = loader->LoadTexture (name, file, CS_TEXTURE_3D, 0, true, true, true, collection);
    if (txt == 0)
    {
      ReportError("Cannot load texture '%s' from file '%s'.\n", name, file);
      return;
    }
    engine->PrepareTextures();
  }
}

void ViewMesh::LoadLibrary(const char* file, bool record)
{
  if(record)
    reloadLibraryFilenames.Push(file);

  loader->LoadLibraryFile(file, collection);
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
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) return ReportError("Failed to locate 3D engine!");
  engine->SetSaveableFlag(true);

  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_LEVELSAVER,
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize(GetObjectRegistry());

  if (!RegisterQueue(GetObjectRegistry(), csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void ViewMesh::OnExit()
{
  printer.Invalidate ();
}

bool ViewMesh::Application()
{
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return ReportError("Failed to locate Virtual FileSystem!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) return ReportError("Failed to locate Loader!");

  tloader = csQueryRegistry<iThreadedLoader> (GetObjectRegistry());
  if (!tloader) return ReportError("Failed to locate threaded Loader!");

  saver = csQueryRegistry<iSaver> (GetObjectRegistry());
  if (!saver) return ReportError("Failed to locate Saver!");

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!cegui) return ReportError("Failed to locate CEGUI plugin");
  
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  collection = engine->CreateCollection ("viewmesh_region");
  reloadFilename = "";

  if (!CreateRoom())
    return false;
  if (!CreateGui ())
    return false;

  HandleCommandLine();

  ResetCamera();
  UpdateCamera();

  camModePan = false;
  camModeRotate = false;
  camModeZoom = false;

  x = g3d->GetDriver2D ()->GetWidth ();
  y = g3d->GetDriver2D ()->GetHeight ();

  printer.AttachNew (new FramePrinter (object_reg));

  Run();

  return true;
}

bool ViewMesh::CreateRoom ()
{
  room = engine->CreateSector ("room");
  view->GetCamera ()->SetSector (room);

  csRef<iLight> light;
  light = engine->CreateLight
    (0, csVector3(-roomsize/2, roomsize/2, 0), 2*roomsize, csColor(1, 1, 1));
  room->GetLights ()->Add (light);

  light = engine->CreateLight
    (0, csVector3(roomsize/2, roomsize/2,  0), 2*roomsize, csColor(1, 1, 1));
  room->GetLights ()->Add (light);

  light = engine->CreateLight
    (0, csVector3(0, roomsize/2, -roomsize/2), 2*roomsize, csColor(1, 1, 1));
  room->GetLights ()->Add (light);

  return true;
}

bool ViewMesh::CreateGui()
{
  // Initialize CEGUI wrapper
  cegui->Initialize ();

  // Set the logging level
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

  vfs->ChDir ("/cegui/");

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->create("ice.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("ice", "MouseArrow");

  cegui->GetFontManagerPtr ()->createFreeTypeFont("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  vfs->ChDir ("/viewmesh/");
  // Load layout and set as root
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("viewmesh.layout"));

  form = winMgr->getWindow("Form");
  stddlg = winMgr->getWindow("StdDlg");

  CEGUI::Window* btn = 0;
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

  btn = winMgr->getWindow("StdDlg/Path");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ViewMesh::StdDlgDirChange, this));

  // ------------------------------------------------------------------------

  vfs->ChDir ("/this/");
  btn = winMgr->getWindow("StdDlg/Path");
  btn->setProperty("Text", vfs->GetCwd());
  StdDlgUpdateLists(vfs->GetCwd());

  // Create default tab.
  generalTab.AttachNew(new GeneralTab(this, GetObjectRegistry(), 0));

  return true;
}

void ViewMesh::LoadSprite (const char* filename, const char* path)
{
  reloadFilename = filename;
  if (path)
    vfs->ChDir(path);

  reloadFilePath = vfs->GetCwd();

  if (asset)
  {
    generalTab->SetAsset(0);
    materialTab.Invalidate();
    UnRegisterTabs ();
    asset.Invalidate();
    lod_level = 0;
    max_lod_level = 0;
    auto_lod = false;
  }

  if (!loading)
  {
    printf ("Loading model '%s' from vfs dir '%s'\n",
      filename, vfs->GetCwd ()); fflush (stdout);

    loading = tloader->LoadFile (vfs->GetCwd(), filename, collection);
  }

  if (!loading->IsFinished())
  {
    // Write loading message.
    csRef<iFontServer> fs = g3d->GetDriver2D()->GetFontServer ();
    if (fs)
    {
      if (g3d->BeginDraw (CSDRAW_2DGRAPHICS))
      {
        csRef<iFont> courierFont = fs->LoadFont (CSFONT_COURIER);
        int fg = g3d->GetDriver2D()->FindRGB (255, 150, 100);
        int x = g3d->GetWidth() - 80;
        int y = g3d->GetHeight() - 20;
        g3d->GetDriver2D()->Write(courierFont, x, y, fg, -1, "Loading...");
      }
    }

    return;
  }

  if (!loading->WasSuccessful())
  {
    loading.Invalidate();
    return;
  }

  csRef<iMeshWrapper> spritewrapper;
  csRef<iMeshFactoryWrapper> factwrap;

  if (!loading->GetResultRefPtr().IsValid())
  {
    // Library file. Find the first factory in our region.
    iMeshFactoryList* factories = engine->GetMeshFactories ();
    int i;
    for (i = 0 ; i < factories->GetCount () ; i++)
    {
      iMeshFactoryWrapper* f = factories->Get (i);
      if (collection->IsParentOf (f->QueryObject ()))
      {
        factwrap = f;
        break;
      }
    }
  }
  else
  {
    factwrap = scfQueryInterface<iMeshFactoryWrapper> (loading->GetResultRefPtr());
    if(!factwrap)
    {
      spritewrapper = scfQueryInterface<iMeshWrapper> (loading->GetResultRefPtr());
      if (spritewrapper)
        factwrap = spritewrapper->GetFactory();
    }
  }

  if (!factwrap)
    return;

  csRef<iMeshObjectFactory> fact = factwrap->GetMeshObjectFactory();
  if (fact)
  {
    csVector3 v(0, 0, 0);

    if(!spritewrapper)
      spritewrapper = engine->CreateMeshWrapper(factwrap, "MySprite", room, v);
    else
    {
      spritewrapper->GetMovable()->SetSector(room);
      spritewrapper->GetMovable()->SetPosition(v);
      spritewrapper->GetMovable()->UpdateMove();
    }

    csRef<iGeneralFactoryState> fstate = scfQueryInterface<iGeneralFactoryState>(factwrap->GetMeshObjectFactory());
    if (fstate)
    {
      max_lod_level = fstate->GetNumProgLODLevels();
    }

    if (AnimeshAsset::Support(spritewrapper))
    {
      asset.AttachNew(new AnimeshAsset(GetObjectRegistry(), spritewrapper));
    }
#ifdef CS_HAVE_CAL3D
    else if (Cal3DAsset::Support(spritewrapper))
    {
      asset.AttachNew(new Cal3DAsset(GetObjectRegistry(), spritewrapper));
    }
#endif
    else if (Sprite3DAsset::Support(spritewrapper))
    {
      asset.AttachNew(new Sprite3DAsset(GetObjectRegistry(), spritewrapper));
    }
    else if (GenmeshAsset::Support(spritewrapper))
    {
      asset.AttachNew(new GenmeshAsset(GetObjectRegistry(), spritewrapper));
    }
    else if (ParticlesAsset::Support(spritewrapper))
    {
      asset.AttachNew(new ParticlesAsset(GetObjectRegistry(), spritewrapper));
    }
    else
    {
      return;
    }
  }

  ScaleSprite (scale);

  if (asset)
  {
    csBox3 box;
    box = asset->GetMesh()->GetWorldBoundingBox();
    csVector3 sprpos = box.GetCenter();
    csVector3 campos = view->GetCamera ()->GetTransform ().GetOrigin();
    view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (campos.x, sprpos.y, campos.z));
  }

  generalTab->SetAsset(asset);

  materialTab.AttachNew(new MaterialTab(GetObjectRegistry(), 0));
  materialTab->SetAsset(asset);

  if (asset->SupportsSockets())
  {
    RegisterTab<SocketTab>();
  }

  if (asset->SupportsAnimations())
  {
    RegisterTab<AnimationTab>();
  }

  if (asset->SupportsMorphTargets())
  {
    RegisterTab<MorphTargetTab>();
  }

  if (asset->SupportsSubMeshes())
  {
    RegisterTab<SubMeshTab>();
    materialTab->Update(true);
  }
  else
    materialTab->Update(false);

  if (asset->SupportsParticles())
  {
    RegisterTab<ParticlesTab>();
  }

  ResetCamera();
  loading.Invalidate();
}

void ViewMesh::SaveSprite (const char* filename, bool binary)
{
  if (!asset) return;

  csRef<iDocumentSystem> xml(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();

  iMeshFactoryWrapper* meshfactwrap = asset->GetMesh()->GetFactory();
  iMeshObjectFactory*  meshfact = meshfactwrap->GetMeshObjectFactory();

  //Create the Tag for the MeshObj
  csRef<iDocumentNode> factNode = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  factNode->SetValue("meshfact");

  //Add the mesh's name to the MeshObj tag
  const char* name = meshfactwrap->QueryObject()->GetName();
  if (name && *name)
    factNode->SetAttribute("name", name);

  csRef<iFactory> factory = 
    scfQueryInterface<iFactory> (meshfact->GetMeshObjectType());

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

void ViewMesh::UnRegisterTabs ()
{
  tabs.DeleteAll();
}

void ViewMesh::AttachMesh (const char* file)
{
  asset->AttachMesh(asset->GetSelectedSocket(), 0);

  iCollection* collection = engine->CreateCollection ("viewmesh_region");
  csLoadResult rc = loader->Load (file, collection, false, true);

  if (!rc.success)
    return;

  csRef<iMeshFactoryWrapper> factory;
  if (rc.result == 0)
  {
    // Library file. Find the last factory in our region.
    iMeshFactoryList* factories = engine->GetMeshFactories ();
    int i;
    for (i = factories->GetCount ()-1 ; i >= 0 ; --i)
    {
      iMeshFactoryWrapper* f = factories->Get (i);
      if (collection->IsParentOf (f->QueryObject ()))
      {
        factory = f;
        break;
      }
    }
  }
  else
  {
    factory = scfQueryInterface<iMeshFactoryWrapper> (rc.result);
  }

  if (!factory) return;

  csRef<iMeshWrapper> meshWrap = engine->CreateMeshWrapper(factory, file);
  csReversibleTransform t;

  asset->AttachMesh(asset->GetSelectedSocket(), meshWrap);
}

void ViewMesh::ScaleSprite (float newScale)
{
  csMatrix3 scalingHt; scalingHt.Identity(); scalingHt *= scale/newScale;
  csReversibleTransform rTH;
  rTH.SetT2O (scalingHt);
  if (asset->GetMesh())
    asset->GetMesh()->HardTransform (rTH);

  csMatrix3 scaling; scaling.Identity(); scaling /= newScale;
  csReversibleTransform rT;
  rT.SetT2O (scaling);
  if (asset->GetMesh())
    asset->GetMesh()->GetMovable()->SetTransform(rT);

  scale = newScale;

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("General/ScaleSprite");
  csRef<iString> valueMesh(new scfString());
  valueMesh->Format("%.2f", scale);
  component->setProperty("Text", valueMesh->GetData());
}

void ViewMesh::MoveLights (const csVector3 &a, const csVector3 &b,
    const csVector3 &c)
{
  iLightList* ll = room->GetLights ();
  if (ll->GetCount () < 3)
    ReportError("MoveLights () has less lights than expected!");

  ll->Get (0)->GetMovable()->SetPosition (a);
  ll->Get (1)->GetMovable()->SetPosition (b);
  ll->Get (2)->GetMovable()->SetPosition (c);

  ll->Get (0)->GetMovable()->UpdateMove();
  ll->Get (1)->GetMovable()->UpdateMove();
  ll->Get (2)->GetMovable()->UpdateMove();
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
  files->Sort (false);
  
  for (size_t i = 0; i < files->GetSize(); i++)
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
  if (path.empty()) return CS_EVENT_HANDLED;

  vfs->ChDir (path.c_str());

  CEGUI::Window* inputfile = winMgr->getWindow("StdDlg/File");
  CEGUI::String file = inputfile->getProperty("Text");
  if (path.empty()) return CS_EVENT_HANDLED;

  CEGUI::String purpose = stddlg->getUserString("Purpose");
  if (purpose == "Save")
  {
    SaveSprite(file.c_str(), false);
  }
  else if (purpose == "SaveBinary")
  {
    SaveSprite(file.c_str(), true);
  }
  else if (purpose == "Load")
  {
    LoadSprite(file.c_str());
  }
  else if (purpose == "LoadLib")
  {
    LoadLibrary(file.c_str());
  }
  else if (purpose == "Attach")
  {
    AttachMesh(file.c_str());
  }

  return CS_EVENT_HANDLED;
}

bool ViewMesh::StdDlgCancleButton (const CEGUI::EventArgs& e)
{
  form->show();
  stddlg->hide();
  return CS_EVENT_HANDLED;
}

bool ViewMesh::StdDlgFileSelect (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*) winMgr->getWindow("StdDlg/FileSelect");
  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  if (!item) return CS_EVENT_HANDLED;

  CEGUI::String text = item->getText();
  if (text.empty()) return CS_EVENT_HANDLED;

  CEGUI::Window* file = winMgr->getWindow("StdDlg/File");
  file->setProperty("Text", text);
  return CS_EVENT_HANDLED;
}

bool ViewMesh::StdDlgDirSelect (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*) winMgr->getWindow("StdDlg/DirSelect");
  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  if (!item) return CS_EVENT_HANDLED;

  CEGUI::String text = item->getText();
  if (text.empty()) return CS_EVENT_HANDLED;

  csPrintf("cd %s\n",text.c_str());

  CEGUI::Window* inputpath = winMgr->getWindow("StdDlg/Path");
  CEGUI::String path = inputpath->getProperty("Text");
  if (path.empty()) return CS_EVENT_HANDLED;

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
  return CS_EVENT_HANDLED;
}

bool ViewMesh::StdDlgDirChange (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* inputpath = winMgr->getWindow("StdDlg/Path");
  CEGUI::String path = inputpath->getProperty("Text");
  if (path.empty()) return CS_EVENT_HANDLED;

  csPrintf("cd %s\n",path.c_str());

  vfs->ChDir (path.c_str ());

  inputpath->setProperty("Text", path.c_str());
  StdDlgUpdateLists(path.c_str());
  return CS_EVENT_HANDLED;
}

//---------------------------------------------------------------------------

int main(int argc, char** argv)
{
  return ViewMesh().Main(argc, argv);
}
