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

#include "csutil/scanstr.h"
#include "imesh/animesh.h"
#include "imesh/animnode/skeleton2anim.h"
#include "imesh/genmesh.h"
#include "iutil/object.h"
#include "ivideo/material.h"

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
  DemoApplication ("CrystalSpace.ViewMesh"), lightMode (THREE_POINT), scale (1.0f),
  lod_level (0), max_lod_level (0), auto_lod (false), mouseMove (false)
{
}

ViewMesh::~ViewMesh ()
{
}

void ViewMesh::Frame()
{
  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  if (loading)
    LoadSprite(reloadFilename, reloadFilePath);

  cegui->Render();
}

bool ViewMesh::OnKeyboard(iEvent& ev)
{
  // Default behavior from DemoApplication
  if (DemoApplication::OnKeyboard (ev))
    return true;

  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // Change progressive LOD
    bool needLODUpdate = false;
    if (csKeyEventHelper::GetCookedCode (&ev) == '-'
	&& !auto_lod
	&& lod_level < max_lod_level)
    {
      lod_level += max_lod_level / 20;
      needLODUpdate = true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == '+'
	&& !auto_lod
	&& lod_level > 0)
    {
      lod_level -= max_lod_level / 20;
      needLODUpdate = true;
    }

    if (needLODUpdate)
    {
      csRef<iMeshObject> mobj = asset->GetMesh()->GetMeshObject();
      csRef<iGeneralMeshState> mstate = scfQueryInterface<iGeneralMeshState>(mobj);
      if (mstate)
      {
	mstate->ForceProgLODLevel(lod_level);
      }
    }
  }
  return false;
}

bool ViewMesh::OnMouseDown (iEvent &event)
{
  // We start here a mouse interaction with the camera, therefore
  // we must take precedence in the mouse events over the CeGUI
  // window. In order to do that, we re-register to the event
  // queue, but with a different priority.
  if (mouseMove) return false;
  mouseMove = true;

  // Re-register to the event queue
  csBaseEventHandler::UnregisterQueue ();
  RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ()));

  return false;
}

bool ViewMesh::OnMouseUp (iEvent &event)
{
  // We finish here a mouse interaction with the camera, therefore
  // the CeGUI window should again take precedence in the mouse events.
  if (!mouseMove) return false;
  mouseMove = false;

  // Re-register to the event queue
  csBaseEventHandler::UnregisterQueue ();
  RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ()));
  return false;
}

void ViewMesh::PrintHelp ()
{
  csCommandLineHelper commandLineHelper;

  // Usage examples
  commandLineHelper.AddCommandLineExample ("viewmesh data/frankie/frankie.xml");
  commandLineHelper.AddCommandLineExample ("viewmesh -R=data/kwartz.zip kwartz.lib");
  commandLineHelper.AddCommandLineExample ("viewmesh -R=data/seymour.zip Seymour.dae");

  // Command line options
  commandLineHelper.AddCommandLineOption
    ("R", "Real path to be mounted in VFS", csVariant (""));
  commandLineHelper.AddCommandLineOption
    ("C", "VFS directory where to find the files", csVariant ("/"));
  commandLineHelper.AddCommandLineOption
    ("L", "Load a library file (for textures/materials)", csVariant (""));
  commandLineHelper.AddCommandLineOption
    ("scale", "Scale the Object", csVariant (1.0f));

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (GetObjectRegistry (), "viewmesh", "viewmesh [OPTIONS] [filename]",
     "Crystal Space's tool for the visualization and the manipulation of meshes\n\n"
     "If provided, it will load the given file from the specified VFS directory."
     " If no VFS directory is provided then it will assume the one of the file. "
     "An additional real path can be provided to be mounted before loading the file."
     " This is useful for example to mount an archive in VFS before accessing the"
     " files in it.");
}

void ViewMesh::HandleCommandLine ()
{
  csRef<iCommandLineParser> cmdline =
    csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());

  const char* libname;
  for (int i=0; (libname = cmdline->GetOption ("L",i)); i++)
  {
    if (!loader->LoadLibraryFile (libname))
    {
      ReportError("Couldn't load lib %s.\n", libname);
    }
  }

  csString meshfilename = cmdline->GetName (0);
  const char* texturefilename = cmdline->GetName (1);
  const char* texturename = cmdline->GetName (2);
  const char* scaleTxt = cmdline->GetOption("Scale");
  const char* realPath = cmdline->GetOption("R");

  csString vfsDir = cmdline->GetOption("C");

  if (realPath)
  {
    vfs->Mount ("/tmp/viewmesh", realPath);
    vfs->ChDir ("/tmp/viewmesh");
  }

  if (vfsDir.IsEmpty() && meshfilename)
  {
    size_t index = meshfilename.FindLast ('/');
    if (index != (size_t) -1)
    {
      vfsDir = meshfilename.Slice (0, index);
      meshfilename = meshfilename.Slice (index + 1);
    }
  }

  if (!vfsDir.IsEmpty())
  {
    if (!vfs->ChDir (vfsDir))
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
      ReportError("Cannot load texture %s from file %s.\n",
		  CS::Quote::Single (name), CS::Quote::Single (file));
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

bool ViewMesh::OnInitialize(int argc, char* argv [])
{
  // Default behavior from DemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
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

  return true;
}

bool ViewMesh::Application()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  // Initialize the HUD manager
  hudManager->GetKeyDescriptions ()->Empty ();

  // Find references to the plugins of the animation nodes
  tloader = csQueryRegistry<iThreadedLoader> (GetObjectRegistry());
  if (!tloader) return ReportError("Failed to locate threaded Loader!");

  saver = csQueryRegistry<iSaver> (GetObjectRegistry());
  if (!saver) return ReportError("Failed to locate Saver!");

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!cegui) return ReportError("Failed to locate CEGUI plugin");
  
  collection = engine->CreateCollection ("viewmesh_region");
  reloadFilename = "";

  if (!CreateRoom())
    return false;
  if (!CreateGui ())
    return false;

  HandleCommandLine();

  Run();

  return true;
}

bool ViewMesh::CreateRoom ()
{
  room = engine->CreateSector ("room");

  // Initialize the camera
  view->GetCamera ()->SetSector (room);
  cameraManager->SetCameraMode (CS::Utility::CAMERA_ROTATE);
  cameraManager->SetCamera (view->GetCamera ());

  csRef<iLight> light;
  light = engine->CreateLight
    (0, csVector3 (0.0f), 100.0f, csColor(1, 1, 1));
  room->GetLights ()->Add (light);

  light = engine->CreateLight
    (0, csVector3 (0.0f), 100.0f, csColor(1, 1, 1));
  room->GetLights ()->Add (light);

  light = engine->CreateLight
    (0, csVector3(0.0f), 100.0f, csColor(1, 1, 1));
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
    printf ("Loading model %s from vfs dir %s\n",
      CS::Quote::Single (filename), CS::Quote::Single (vfs->GetCwd ()));
    fflush (stdout);

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
      lod_level = 0;
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

  // Reset the camera and light positions
  if (asset)
  {
    csBox3 bbox = asset->GetMesh ()->GetWorldBoundingBox ();
    cameraManager->SetCameraTarget (bbox.GetCenter ());
    float boxSize = bbox.GetSize ().Norm ();
    cameraManager->SetStartPosition (bbox.GetCenter () + csVector3 (0.0f, 0.0f, - boxSize));
    cameraManager->SetMotionSpeed (boxSize * 5.0f);
    cameraManager->ResetCamera ();

    SetLightMode (lightMode);
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

  loading.Invalidate();
}

void ViewMesh::SaveSprite (const char* filename, bool binary)
{
  if (!asset) return;

  csRef<iDocumentSystem> xml = csPtr<iDocumentSystem>(new csTinyDocumentSystem());
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
    csRef<iString> fname (csPtr<iString> (new scfString(filename)));
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
  csRef<iString> valueMesh (csPtr<iString> (new scfString()));
  valueMesh->Format("%.2f", scale);
  component->setProperty("Text", valueMesh->GetData());
}

void ViewMesh::SetLightMode (LightMode lightMode)
{
  iLightList* ll = room->GetLights ();
  if (ll->GetCount () < 3)
    ReportError("MoveLights () has less lights than expected!");

  float roomSize = 5.0f;
  if (asset)
    roomSize = asset->GetMesh ()->GetWorldBoundingBox ().GetSize ().Norm () * 5.0f;

  switch (lightMode)
    {
    case THREE_POINT:
      ll->Get (0)->GetMovable()->SetPosition (csVector3 (-roomSize / 2.0f, roomSize / 2.0f, 0.0f));
      ll->Get (1)->GetMovable()->SetPosition (csVector3 (roomSize / 2.0f,  -roomSize / 2.0f, 0.0f));
      ll->Get (2)->GetMovable()->SetPosition (csVector3 (0.0f, 0.0f, -roomSize / 2.0f));
      break;

    case FRONT_BACK_TOP:
      ll->Get (0)->GetMovable()->SetPosition (csVector3 (0.0f, 0.0f, roomSize / 4.0f));
      ll->Get (1)->GetMovable()->SetPosition (csVector3 (0.0f, 0.0f, -roomSize / 4.0f));
      ll->Get (2)->GetMovable()->SetPosition (csVector3 (0.0f, roomSize / 2.0f, 0.0f));
      break;

    case UNLIT:
      ll->Get (0)->GetMovable()->SetPosition (csVector3 (0.0f));
      ll->Get (1)->GetMovable()->SetPosition (csVector3 (0.0f,  -roomSize / 4.0f, 0.0f));
      ll->Get (2)->GetMovable()->SetPosition (csVector3 (0.0f, roomSize / 2.0f, -roomSize / 2.0f));
      break;
    }

  ll->Get (0)->SetCutoffDistance (roomSize);
  ll->Get (1)->SetCutoffDistance (roomSize);
  ll->Get (2)->SetCutoffDistance (roomSize);

  ll->Get (0)->GetMovable()->UpdateMove();
  ll->Get (1)->GetMovable()->UpdateMove();
  ll->Get (2)->GetMovable()->UpdateMove();

  this->lightMode = lightMode;
}

void ViewMesh::MoveLights (const csVector3 &a, const csVector3 &b,
    const csVector3 &c)
{

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
