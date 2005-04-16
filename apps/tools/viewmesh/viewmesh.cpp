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
#include "csutil/scfstr.h"

// Hack: work around problems caused by #defining 'new'
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#ifdef CS_HAVE_CAL3D
#include <cal3d/animcallback.h>
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

ViewMesh::ViewMesh ()
: camMode(movenormal), roomsize(5), scale(1), selectedSocket(0),
  selectedCal3dSocket(0), meshTx(0), meshTy(0), meshTz(0), callback(0)
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

void ViewMesh::ProcessFrame ()
{
  csTicks elapsed_time = vc->GetElapsedTicks ();
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  iGraphics2D* g2d = g3d->GetDriver2D ();

  if (g2d->GetHeight() != y || g2d->GetWidth() != x)
  {
    x = g2d->GetWidth();
    y = g2d->GetHeight();
    aws->SetupCanvas(0, g3d->GetDriver2D (), g3d);
  }

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
      spritewrapper->GetWorldBoundingBox(box);
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
      spritewrapper->GetWorldBoundingBox(box);
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

  aws->Redraw ();
  aws->Print (g3d, 64);

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
      if (q.IsValid()) q->GetEventOutlet()->Broadcast(cscmdQuit);
    }
  }
  return false;
}

bool ViewMesh::HandleEvent (iEvent &event)
{
  if (aws)
    if (aws->HandleEvent (event))
      return true;
  return csBaseEventHandler::HandleEvent(event);;
}

void ViewMesh::Help ()
{
  csPrintf ("Options for ViewMesh:\n");
  csPrintf ("  -L=<file>          Load a library file (for textures/materials)\n");
  csPrintf ("  -Scale=<ratio>     Scale the Object\n");
  csPrintf ("  -RoomSize=<units>  Radius and height (4*) of the room (default 5)\n");
  csPrintf ("  <file>             Load the specified mesh object (meshfact or library)\n");
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

  if (texturefilename && texturename)
  {
    csString file(texturefilename);
    size_t split = file.FindLast('/');
    LoadTexture(file, file.Slice(split+1, file.Length()-split-1), texturename);
  }

  if (meshfilename)
  {
    csString file(meshfilename);
    size_t split = file.FindLast('/');    
    LoadSprite(file, file.Slice(split+1, file.Length()-split-1));
  }

  if (roomSize) roomsize = atof(roomSize);

  if (scaleTxt != 0)
  {
    float newScale;
    sscanf (scaleTxt, "%f", &newScale);
    ScaleSprite(newScale);
  }

}

void ViewMesh::LoadTexture(const char* path, const char* file, const char* name)
{
  if (path) ParseDir(path);

  if (file && name)
  {
    iTextureWrapper* txt = loader->LoadTexture (name, file);
    if (txt == 0)
    {
      ReportError("Cannot load texture '%s' from file '%s'.\n", name, file);
      return;
    }
    txt->Register (g3d->GetTextureManager ());
    iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName (name);
    engine->PrepareTextures();
  }
}

void ViewMesh::LoadLibrary(const char* path, const char* file)
{
  if (path) ParseDir(path);
  loader->LoadLibraryFile(file);
}

bool ViewMesh::OnInitialize(int argc, char* argv[])
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
    CS_REQUEST_PLUGIN("crystalspace.window.alternatemanager", iAws),
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  if (!RegisterQueue(GetObjectRegistry()))
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

  aws = CS_QUERY_REGISTRY(GetObjectRegistry(), iAws);
  if (!aws) return ReportError("Failed to locate AWS!");
  
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
  csRef<iThingState> ws =
    SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
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

void ViewMesh::CreateGui ()
{
  aws->SetupCanvas(0, g3d->GetDriver2D (), g3d);

  iAwsSink* sink;

  //GENERAL
  sink = aws->GetSinkMgr ()->CreateSink ((intptr_t)this);
  sink->RegisterTrigger ("CameraMode", &CameraMode);
  sink->RegisterTrigger ("LoadButton", &LoadButton);
  sink->RegisterTrigger ("LoadLibButton", &LoadLibButton);
  sink->RegisterTrigger ("SaveButton", &SaveButton);
  sink->RegisterTrigger ("SaveBinaryButton", &SaveBinaryButton);
  sink->RegisterTrigger ("ScaleSprite", &SetScaleSprite);
  aws->GetSinkMgr ()->RegisterSink ("General", sink);

  //ANIMATION
  sink = aws->GetSinkMgr ()->CreateSink ((intptr_t)this);
  sink->RegisterTrigger ("ReversAnimation", &ReversAnimation);
  sink->RegisterTrigger ("StopAnimation", &StopAnimation);
  sink->RegisterTrigger ("SlowerAnimation", &SlowerAnimation);
  sink->RegisterTrigger ("AddAnimation", &AddAnimation);
  sink->RegisterTrigger ("FasterAnimation", &FasterAnimation);
  sink->RegisterTrigger ("SetAnimation", &SetAnimation);
  sink->RegisterTrigger ("RemoveAnimation", &RemoveAnimation);
  sink->RegisterTrigger ("ClearAnimation", &ClearAnimation);
  sink->RegisterTrigger ("SelAnimation", &SelAnimation);
  aws->GetSinkMgr ()->RegisterSink ("Anim", sink);

  //SOCKET
  sink = aws->GetSinkMgr ()->CreateSink ((intptr_t)this);
  sink->RegisterTrigger ("SetMesh", &SetMesh);
  sink->RegisterTrigger ("SetSubMesh", &SetSubMesh);
  sink->RegisterTrigger ("SetTriangle", &SetTriangle);
  sink->RegisterTrigger ("SetRotX", &SetRotX);
  sink->RegisterTrigger ("SetRotY", &SetRotY);
  sink->RegisterTrigger ("SetRotZ", &SetRotZ);
  sink->RegisterTrigger ("AttachButton", &AttachButton);
  sink->RegisterTrigger ("DetachButton", &DetachButton);
  sink->RegisterTrigger ("AddSocket", &AddSocket);
  sink->RegisterTrigger ("DelSocket", &DelSocket);
  sink->RegisterTrigger ("SelSocket", &SelSocket);
  sink->RegisterTrigger ("RenameSocket", &RenameSocket);
  aws->GetSinkMgr ()->RegisterSink ("Socket", sink);

  //SOCKET
  sink = aws->GetSinkMgr ()->CreateSink ((intptr_t)this);
  sink->RegisterTrigger ("SelMorph", &SelMorph);
  sink->RegisterTrigger ("BlendButton", &BlendButton);
  sink->RegisterTrigger ("ClearButton", &ClearButton);
  aws->GetSinkMgr ()->RegisterSink ("Morph", sink);

  //STDDLG
  sink = aws->GetSinkMgr ()->CreateSink ((intptr_t)this);
  sink->RegisterTrigger ("OkButton", &StdDlgOkButton);
  sink->RegisterTrigger ("CancleButton", &StdDlgCancleButton);
  sink->RegisterTrigger ("FileSelect", &StdDlgFileSelect);
  sink->RegisterTrigger ("DirSelect", &StdDlgDirSelect);
  aws->GetSinkMgr ()->RegisterSink ("StdDlg", sink);

  if (!aws->GetPrefMgr()->Load ("/aws/windows_skin.def"))
    ReportError("couldn't load skin definition file!");
  if (!aws->GetPrefMgr()->Load ("/varia/viewmesh.def"))
    ReportError("couldn't load ViewMesh AWS definition file!");
  if (!aws->GetPrefMgr()->Load ("/aws/stddlg.def"))
    ReportError("couldn't load Standard Dialog AWS definition file!");
  aws->GetPrefMgr ()->SelectDefaultSkin ("Windows");

  form = aws->CreateWindowFrom ("Form1");
  stddlg = aws->CreateWindowFrom ("StdDlg");
  form->Show();

  iAwsComponent* InputPath = stddlg->FindChild("InputPath");
  csRef<iString> valuePath(new scfString(vfs->GetCwd()));
  if (InputPath) InputPath->SetProperty("Text",(intptr_t)(iString*)valuePath);

  StdDlgUpdateLists(valuePath->GetData());
}

void ViewMesh::LoadSprite (const char* path, const char* filename)
{
  if (path) ParseDir(path);

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
        engine->RemoveObject(meshWrapOld);
        engine->RemoveObject(meshWrapOld->GetFactory());
        delete meshWrapOld;
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
    spritewrapper->GetWorldBoundingBox(box);
    csVector3 sprpos = box.GetCenter();
    csVector3 campos = view->GetCamera ()->GetTransform ().GetOrigin();
    view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (campos.x, sprpos.y, campos.z));
    camMode = rotateorigin;
  }

  UpdateSocketList();
  UpdateAnimationList();
  UpdateMorphList ();
}

void ViewMesh::SaveSprite (const char* path, const char* filename, bool binary)
{
  ParseDir(path);

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
    CS_QUERY_REGISTRY (object_reg, iPluginManager);

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

    csRef<iBinarySaverPlugin> saver =
      CS_QUERY_PLUGIN_CLASS(plugin_mgr, savername, iBinarySaverPlugin);
    if (!saver)
      saver = CS_LOAD_PLUGIN(plugin_mgr, savername, iBinarySaverPlugin);
    if (saver)
      saver->WriteDown(meshfact, file);
  }
  else
  {
    csRef<iSaverPlugin> saver = 
      CS_QUERY_PLUGIN_CLASS(plugin_mgr, savername, iSaverPlugin);
    if (!saver) 
      saver = CS_LOAD_PLUGIN(plugin_mgr, savername, iSaverPlugin);
    if (saver) 
      saver->WriteDown(meshfact, factNode);
  }
  scfString str;
  doc->Write(&str);
  vfs->WriteFile(filename, str.GetData(), str.Length());
}

void ViewMesh::AttachMesh (const char* path, const char* file)
{
  if (selectedSocket)
  {
    csRef<iMeshWrapper> meshWrapOld = selectedSocket->GetMeshWrapper();
    if ( meshWrapOld )
    {
      spritewrapper->GetChildren()->Remove( meshWrapOld );
      selectedSocket->SetMeshWrapper( NULL );    
    }
  }
  else if (selectedCal3dSocket)
  {
    csRef<iMeshWrapper> meshWrapOld = selectedCal3dSocket->GetMeshWrapper();
    if ( meshWrapOld )
    {
      spritewrapper->GetChildren()->Remove( meshWrapOld );
      selectedCal3dSocket->SetMeshWrapper( NULL );    
    }
  }

  ParseDir(path);

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
    spritewrapper->GetChildren()->Add( meshWrap );
    selectedSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
  }
  else if (selectedCal3dSocket)
  {
    selectedCal3dSocket->SetTransform(t);
    spritewrapper->GetChildren()->Add( meshWrap );
    selectedCal3dSocket->SetMeshWrapper( meshWrap );
    spritewrapper->GetMovable()->UpdateMove();
  }
}

void ViewMesh::UpdateSocketList ()
{
  iAwsComponent* list = form->FindChild("SocketList");
  iAwsParmList* pl = aws->CreateParmList();
  if (!list) return;

  list->Execute("ClearList", pl);

  if (sprite)
  {
    for (int i = 0; i < sprite->GetSocketCount(); i++)
    {
      iSpriteSocket* sock = sprite->GetSocket(i);
      if (!sock) continue;

      if (i==0) SelectSocket(sock->GetName());

      pl->AddString("text0", sock->GetName());
      list->Execute("InsertItem", pl);
      pl->Clear();
    }
  }
  else if (cal3dsprite)
  {
    for (int i = 0; i < cal3dsprite->GetSocketCount(); i++)
    {
      iSpriteCal3DSocket* sock = cal3dsprite->GetSocket(i);
      if (!sock) continue;

      if (i==0) SelectSocket(sock->GetName());

      pl->AddString("text0", sock->GetName());
      list->Execute("InsertItem", pl);
      pl->Clear();
    }
  }
}

void ViewMesh::UpdateAnimationList ()
{
  iAwsComponent* list = form->FindChild("AnimList");
  iAwsParmList* pl = aws->CreateParmList();
  if (!list) return;

  list->Execute("ClearList", pl);

  pl->AddString("text0", "default");
  list->Execute("InsertItem", pl);
  pl->Clear();

  if (sprite)
  {
    for (int i = 0; i < sprite->GetActionCount(); i++)
    {
      iSpriteAction* action = sprite->GetAction(i);
      if (!action) continue;

      pl->AddString("text0", action->GetName ());
      list->Execute("InsertItem", pl);
      pl->Clear();
    }
  }
  else if (cal3dsprite)
  {
    for (int i = 0; i < cal3dstate->GetAnimCount(); i++)
    {
      const char* animname = cal3dstate->GetAnimName(i);
      if (!animname) continue;

      pl->AddString("text0", animname);
      list->Execute("InsertItem", pl);
      pl->Clear();
    }
  }
}

void ViewMesh::UpdateMorphList ()
{
  iAwsComponent* list = form->FindChild("MorphList");
  iAwsParmList* pl = aws->CreateParmList();
  if (!list) return;

  list->Execute("ClearList", pl);

  pl->AddString("text0", "default");
  list->Execute("InsertItem", pl);
  pl->Clear();

  if (cal3dsprite)
  {
    for (int i = 0; i < cal3dsprite->GetMorphAnimationCount(); i++)
    {
      const char* morphname = cal3dsprite->GetMorphAnimationName(i);
      if (!morphname) continue;

      pl->AddString("text0", morphname);
      list->Execute("InsertItem", pl);
      pl->Clear();
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
  if (selectedSocket)
  {
    iAwsComponent* InputName = form->FindChild("InputName");
    const char* name = selectedSocket->GetName();
    csRef<iString> valueName(new scfString(name));
    InputName->SetProperty("Text",(intptr_t)(iString*)valueName);

    iAwsComponent* InputTriangle = form->FindChild("InputTriangle");
    csRef<iString> valueTriangle(new scfString());
    valueTriangle->Format("%d", selectedSocket->GetTriangleIndex());
    InputTriangle->SetProperty("Text",(intptr_t)(iString*)valueTriangle);
  }
  else if (selectedCal3dSocket)
  {
    iAwsComponent* InputName = form->FindChild("InputName");
    const char* name = selectedCal3dSocket->GetName();
    csRef<iString> valueName(new scfString(name));
    InputName->SetProperty("Text",(intptr_t)(iString*)valueName);

    iAwsComponent* InputMesh = form->FindChild("InputMesh");
    csRef<iString> valueMesh(new scfString());
    valueMesh->Format("%d", selectedCal3dSocket->GetMeshIndex());
    InputMesh->SetProperty("Text",(intptr_t)(iString*)valueMesh);

    iAwsComponent* InputSubMesh = form->FindChild("InputSubMesh");
    csRef<iString> valueSubmesh(new scfString());
    valueSubmesh->Format("%d", selectedCal3dSocket->GetSubmeshIndex());
    InputSubMesh->SetProperty("Text",(intptr_t)(iString*)valueSubmesh);

    iAwsComponent* InputTriangle = form->FindChild("InputTriangle");
    csRef<iString> valueTriangle(new scfString());
    valueTriangle->Format("%d", selectedCal3dSocket->GetTriangleIndex());
    InputTriangle->SetProperty("Text",(intptr_t)(iString*)valueTriangle);
  }
}

void ViewMesh::ScaleSprite (float newScale)
{
  csMatrix3 scalingHt; scalingHt.Identity(); scalingHt *= newScale/scale;
  csReversibleTransform rTH;
  rTH.SetT2O (scalingHt);
  if (spritewrapper)
    spritewrapper->HardTransform (rTH);

  csMatrix3 scaling; scaling.Identity(); scaling *= newScale;
  csReversibleTransform rT;
  rT.SetT2O (scaling);
  if (spritewrapper)
    spritewrapper->GetMovable()->SetTransform(rT);

  scale = newScale;

  iAwsComponent* InputMesh = form->FindChild("InputScale");
  csRef<iString> valueMesh(new scfString());
  valueMesh->Format("%.2f", scale);
  InputMesh->SetProperty("Text",(intptr_t)(iString*)valueMesh);
}

//---------------------------------------------------------------------------

void ViewMesh::ReversAnimation (intptr_t awst, iAwsSource *source)
{
  ViewMesh* tut = (ViewMesh*)awst;
  if (tut->cal3dstate)
  {
    tut->cal3dstate->SetAnimationTime(-1);
  }
  else if (tut->state)
  {
    tut->state->SetReverseAction(tut->state->GetReverseAction()^true);
  }
}
void ViewMesh::StopAnimation (intptr_t awst, iAwsSource *source)
{
  ViewMesh* tut = (ViewMesh*)awst;
  tut->move_sprite_speed = 0;
}
void ViewMesh::SlowerAnimation (intptr_t awst, iAwsSource *source)
{
  ViewMesh* tut = (ViewMesh*)awst;
  tut->move_sprite_speed -= 0.5f;
}
void ViewMesh::AddAnimation (intptr_t awst, iAwsSource *source)
{
  ViewMesh* tut = (ViewMesh*)awst;
  if (tut->cal3dstate)
  {
    if (!tut->selectedAnimation) return;
    int anim = tut->cal3dstate->FindAnim(tut->selectedAnimation);
    tut->cal3dstate->AddAnimCycle(anim,1,3);
  }
}
void ViewMesh::FasterAnimation (intptr_t awst, iAwsSource *source)
{
  ViewMesh* tut = (ViewMesh*)awst;
  tut->move_sprite_speed += 0.5f;
}
void ViewMesh::SetAnimation (intptr_t awst, iAwsSource *source)
{
  ViewMesh* tut = (ViewMesh*)awst;
  if (tut->cal3dstate)
  {
    if (!tut->selectedAnimation) return;
    int anim = tut->cal3dstate->FindAnim(tut->selectedAnimation);
    tut->cal3dstate->SetAnimAction(anim,1,1);
  }
  else if (tut->state)
  {
    if (!tut->selectedAnimation) return;
    tut->state->SetAction(tut->selectedAnimation);
  }
}
void ViewMesh::RemoveAnimation (intptr_t awst, iAwsSource *source)
{
  //TODO: Implement it.
  ViewMesh* tut = (ViewMesh*)awst;
  tut->ReportWarning("Removal of Animation is not yet implemented");
}
void ViewMesh::ClearAnimation (intptr_t awst, iAwsSource *source)
{
  ViewMesh* tut = (ViewMesh*)awst;
  if (tut->cal3dstate)
  {
    if (!tut->selectedAnimation) return;
    int anim = tut->cal3dstate->FindAnim(tut->selectedAnimation);
    tut->cal3dstate->ClearAnimCycle(anim,3);
  }
}
void ViewMesh::SelAnimation (intptr_t awst, iAwsSource *source)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  iAwsComponent* list = tut->form->FindChild("AnimList");
  iAwsParmList* pl = tut->aws->CreateParmList();
  if (!list) return;

  pl->AddString("text0","");
  list->Execute("GetSelectedItem", pl);
  pl->GetString("text0",&text);

  if (!text->GetData()) return;

  tut->selectedAnimation = text->GetData();
}

//---------------------------------------------------------------------------

void ViewMesh::SetMesh (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  if (!tut->selectedCal3dSocket) return;

  iString* text;
  if (!s->GetComponent()->GetProperty("Text",(intptr_t*)&text)) return;

  if (!text->GetData()) return;

  int i;
  if (sscanf(text->GetData(),"%d", &i) != 1) return;

  tut->selectedCal3dSocket->SetMeshIndex(i);
  tut->UpdateSocket();
}

void ViewMesh::SetSubMesh (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  if (!tut->selectedCal3dSocket) return;

  iString* text;
  if (!s->GetComponent()->GetProperty("Text",(intptr_t*)&text)) return;

  if (!text->GetData()) return;

  int i;
  if (sscanf(text->GetData(),"%d", &i) != 1) return;

  tut->selectedCal3dSocket->SetSubmeshIndex(i);
  tut->UpdateSocket();
}

void ViewMesh::SetTriangle (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  if (!s->GetComponent()->GetProperty("Text",(intptr_t*)&text)) return;

  if (!text->GetData()) return;

  int i;
  if (sscanf(text->GetData(),"%d", &i) != 1) return;

  if (tut->selectedCal3dSocket)
    tut->selectedCal3dSocket->SetTriangleIndex(i);
  else if (tut->selectedSocket)
    tut->selectedSocket->SetTriangleIndex(i);

  tut->UpdateSocket();
}

void ViewMesh::SetRotX (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  if (!s->GetComponent()->GetProperty("Text",(intptr_t*)&text)) return;

  if (!text->GetData()) return;

  float f;
  if (sscanf(text->GetData(),"%f", &f) != 1) return;

  if (tut->selectedCal3dSocket && tut->selectedCal3dSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = tut->selectedCal3dSocket->GetMeshWrapper();
    tut->spritewrapper->GetChildren()->Remove( meshWrap );
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-tut->meshTz);
    Tr.RotateOther(csVector3(0,1,0),-tut->meshTy);
    Tr.RotateOther(csVector3(1,0,0),-tut->meshTx);
    Tr.RotateOther(csVector3(1,0,0),f);
    Tr.RotateOther(csVector3(0,1,0),tut->meshTy);
    Tr.RotateOther(csVector3(0,0,1),tut->meshTz);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    tut->spritewrapper->GetChildren()->Add( meshWrap );
    tut->selectedCal3dSocket->SetMeshWrapper( meshWrap );
    tut->spritewrapper->GetMovable()->UpdateMove();
    tut->meshTx = f;
  }
  else if (tut->selectedSocket && tut->selectedSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = tut->selectedSocket->GetMeshWrapper();
    tut->spritewrapper->GetChildren()->Remove( meshWrap );
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-tut->meshTz);
    Tr.RotateOther(csVector3(0,1,0),-tut->meshTy);
    Tr.RotateOther(csVector3(1,0,0),-tut->meshTx);
    Tr.RotateOther(csVector3(1,0,0),f);
    Tr.RotateOther(csVector3(0,1,0),tut->meshTy);
    Tr.RotateOther(csVector3(0,0,1),tut->meshTz);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    tut->spritewrapper->GetChildren()->Add( meshWrap );
    tut->selectedSocket->SetMeshWrapper( meshWrap );
    tut->spritewrapper->GetMovable()->UpdateMove();
    tut->meshTx = f;
  }
}

void ViewMesh::SetRotY (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  if (!s->GetComponent()->GetProperty("Text",(intptr_t*)&text)) return;

  if (!text->GetData()) return;

  float f;
  if (sscanf(text->GetData(),"%f", &f) != 1) return;

  if (tut->selectedCal3dSocket && tut->selectedCal3dSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = tut->selectedCal3dSocket->GetMeshWrapper();
    tut->spritewrapper->GetChildren()->Remove( meshWrap );
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-tut->meshTz);
    Tr.RotateOther(csVector3(0,1,0),-tut->meshTy);
    Tr.RotateOther(csVector3(1,0,0),-tut->meshTx);
    Tr.RotateOther(csVector3(1,0,0),tut->meshTx);
    Tr.RotateOther(csVector3(0,1,0),f);
    Tr.RotateOther(csVector3(0,0,1),tut->meshTz);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    tut->spritewrapper->GetChildren()->Add( meshWrap );
    tut->selectedCal3dSocket->SetMeshWrapper( meshWrap );
    tut->spritewrapper->GetMovable()->UpdateMove();
    tut->meshTy = f;
  }
  else if (tut->selectedSocket && tut->selectedSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = tut->selectedSocket->GetMeshWrapper();
    tut->spritewrapper->GetChildren()->Remove( meshWrap );
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-tut->meshTz);
    Tr.RotateOther(csVector3(0,1,0),-tut->meshTy);
    Tr.RotateOther(csVector3(1,0,0),-tut->meshTx);
    Tr.RotateOther(csVector3(1,0,0),tut->meshTx);
    Tr.RotateOther(csVector3(0,1,0),f);
    Tr.RotateOther(csVector3(0,0,1),tut->meshTz);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    tut->spritewrapper->GetChildren()->Add( meshWrap );
    tut->selectedSocket->SetMeshWrapper( meshWrap );
    tut->spritewrapper->GetMovable()->UpdateMove();
    tut->meshTy = f;
  }
}

void ViewMesh::SetRotZ (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  if (!s->GetComponent()->GetProperty("Text",(intptr_t*)&text)) return;

  if (!text->GetData()) return;

  float f;
  if (sscanf(text->GetData(),"%f", &f) != 1) return;

  if (tut->selectedCal3dSocket && tut->selectedCal3dSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = tut->selectedCal3dSocket->GetMeshWrapper();
    tut->spritewrapper->GetChildren()->Remove( meshWrap );
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-tut->meshTz);
    Tr.RotateOther(csVector3(0,1,0),-tut->meshTy);
    Tr.RotateOther(csVector3(1,0,0),-tut->meshTx);
    Tr.RotateOther(csVector3(1,0,0),tut->meshTx);
    Tr.RotateOther(csVector3(0,1,0),tut->meshTy);
    Tr.RotateOther(csVector3(0,0,1),f);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    tut->spritewrapper->GetChildren()->Add( meshWrap );
    tut->selectedCal3dSocket->SetMeshWrapper( meshWrap );
    tut->spritewrapper->GetMovable()->UpdateMove();
    tut->meshTz = f;
  }
  else if (tut->selectedSocket && tut->selectedSocket->GetMeshWrapper())
  {
    csRef<iMeshWrapper> meshWrap = tut->selectedSocket->GetMeshWrapper();
    tut->spritewrapper->GetChildren()->Remove( meshWrap );
    csReversibleTransform Tr;
    Tr.RotateOther(csVector3(0,0,1),-tut->meshTz);
    Tr.RotateOther(csVector3(0,1,0),-tut->meshTy);
    Tr.RotateOther(csVector3(1,0,0),-tut->meshTx);
    Tr.RotateOther(csVector3(1,0,0),tut->meshTx);
    Tr.RotateOther(csVector3(0,1,0),tut->meshTy);
    Tr.RotateOther(csVector3(0,0,1),f);
    meshWrap->GetMeshObject()->HardTransform(Tr);
    meshWrap->GetFactory()->GetMeshObjectFactory()->HardTransform(Tr);
    tut->spritewrapper->GetChildren()->Add( meshWrap );
    tut->selectedSocket->SetMeshWrapper( meshWrap );
    tut->spritewrapper->GetMovable()->UpdateMove();
    tut->meshTz = f;
  }
}

void ViewMesh::AttachButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;
  tut->form->Hide();
  tut->stddlg->Show();
  tut->stddlgPurpose=attach;
}

void ViewMesh::DetachButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  csRef<iMeshWrapper> meshWrapOld;
  if (tut->selectedCal3dSocket)
    meshWrapOld = tut->selectedCal3dSocket->GetMeshWrapper();
  else if (tut->selectedSocket)
    meshWrapOld = tut->selectedSocket->GetMeshWrapper();
  
  if (!meshWrapOld ) return;

  tut->spritewrapper->GetChildren()->Remove( meshWrapOld );

  tut->engine->RemoveObject(meshWrapOld);
  tut->engine->RemoveObject(meshWrapOld->GetFactory());

  if (tut->selectedCal3dSocket)
    tut->selectedCal3dSocket->SetMeshWrapper( 0 );    
  else if (tut->selectedSocket)
    tut->selectedSocket->SetMeshWrapper( 0 );    
}

void ViewMesh::AddSocket (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  tut->ReportWarning("Adding sockets is not yet implemented");

  if (tut->cal3dsprite)
  {
    //tut->cal3dsprite->AddSocket()->SetName("NewSocket");
    //tut->cal3dstate->AddSocket()->SetName("NewSocket");
    //tut->SelectSocket("NewSocket");
  }
  else if (tut->sprite)
  {
    //iSpriteSocket* newsocket = tut->sprite->AddSocket();
    //newsocket->SetName("NewSocket");
    //tut->SelectSocket(newsocket->GetName());
  }
  tut->UpdateSocketList();
}

void ViewMesh::DelSocket (intptr_t awst, iAwsSource *s)
{
  //Change API of iSpriteCal3DFactoryState to enable this!
  ViewMesh* tut = (ViewMesh*)awst;
  tut->ReportWarning("Deleting sockets is not yet implemented");
  //tut->socket->DelSocket(selectedCal3dSocket);
  //selectedCal3dSocket = 0;
  tut->UpdateSocketList();
}

void ViewMesh::SelSocket (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  iAwsComponent* list = tut->form->FindChild("SocketList");
  iAwsParmList* pl = tut->aws->CreateParmList();
  if (!list) return;

  pl->AddString("text0","");
  list->Execute("GetSelectedItem", pl);
  pl->GetString("text0",&text);

  if (!text->GetData()) return;

  tut->SelectSocket(text->GetData());
}


void ViewMesh::RenameSocket (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iAwsComponent* textfield = tut->form->FindChild("InputName");
  if (!textfield) return;

  iString* text;
  if (!textfield->GetComponent()->GetProperty("Text",(intptr_t*)&text)) return;

  if (!text->GetData()) return;

  if (tut->selectedSocket)
  {
    tut->selectedSocket->SetName(*text);
  }
  else if (tut->selectedCal3dSocket)
  {
    const char* name = tut->selectedCal3dSocket->GetName();
    tut->cal3dsprite->FindSocket(name)->SetName(*text);
    tut->cal3dstate->FindSocket(name)->SetName(*text);
    tut->selectedCal3dSocket = tut->cal3dsprite->FindSocket(*text);
  }

  tut->UpdateSocketList();
}

//---------------------------------------------------------------------------

void ViewMesh::CameraMode (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  bool* state;
  if (!s->GetComponent()->GetProperty("State",(intptr_t*)&state)) return;

  if (!state) return;

  iString* caption;
  if (!s->GetComponent()->GetProperty("Caption",(intptr_t*)&caption)) return;

  if (!caption->GetData()) return;

  if (!strcmp (*caption, "Rotate"))
    tut->camMode = rotateorigin;
  else if (!strcmp (*caption, "Look to Origin"))
    tut->camMode = moveorigin;
  else if (!strcmp (*caption, "Normal Movement"))
    tut->camMode = movenormal;
}

void ViewMesh::LoadButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;
  tut->form->Hide();
  tut->stddlg->Show();
  tut->stddlgPurpose=load;
}

void ViewMesh::LoadLibButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;
  tut->form->Hide();
  tut->stddlg->Show();
  tut->stddlgPurpose=loadlib;
}

void ViewMesh::SaveButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;
  tut->form->Hide();
  tut->stddlg->Show();
  tut->stddlgPurpose=save;
}

void ViewMesh::SaveBinaryButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;
  tut->form->Hide();
  tut->stddlg->Show();
  tut->stddlgPurpose=savebinary;
}

void ViewMesh::SetScaleSprite (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  if (!s->GetComponent()->GetProperty("Text",(intptr_t*)&text)) return;

  if (!text->GetData()) return;

  float f;
  if (sscanf(text->GetData(),"%f", &f) != 1) return;

  tut->ScaleSprite(f);
}

//---------------------------------------------------------------------------
void ViewMesh::SelMorph (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  iAwsComponent* list = tut->form->FindChild("MorphList");
  iAwsParmList* pl = tut->aws->CreateParmList();
  if (!list) return;

  pl->AddString("text0","");
  list->Execute("GetSelectedItem", pl);
  pl->GetString("text0",&text);

  if (!text->GetData()) return;

  tut->selectedMorphTarget = text->GetData();
}

void ViewMesh::BlendButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;
  if (!tut->cal3dstate) return;

  iString *Sweight=0, *Sdelay=0;
  float weight=1, delay=1;

  iAwsComponent* component;
  component = tut->stddlg->FindChild("InputWeight");
  if (component) component->GetProperty("Text",(intptr_t*)&Sweight);
  if (Sweight && Sweight->GetData())
  {
    if(sscanf(Sweight->GetData(), "%f", &weight) != 1) weight = 1;
  }

  component = tut->stddlg->FindChild("InputDelay");
  if (component) component->GetProperty("Text",(intptr_t*)&Sdelay);
  if (Sdelay && Sdelay->GetData())
  {
    if(sscanf(Sdelay->GetData(), "%f", &delay) != 1) delay = 1;
  }

  int target =
    tut->cal3dsprite->FindMorphAnimationName(tut->selectedMorphTarget);

  if (target == -1) return;

  tut->cal3dstate->BlendMorphTarget(target, weight, delay);
}

void ViewMesh::ClearButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;
  if (!tut->cal3dstate) return;

  iString *Sweight=0;
  float weight=1;

  iAwsComponent* component;
  component = tut->stddlg->FindChild("InputWeight");
  if (component) component->GetProperty("Text",(intptr_t*)&Sweight);
  if (Sweight && Sweight->GetData())
  {
    if(sscanf(Sweight->GetData(), "%f", &weight) != 1) weight = 1;
  }

  int target =
    tut->cal3dsprite->FindMorphAnimationName(tut->selectedMorphTarget);

  if (target == -1) return;

  tut->cal3dstate->ClearMorphTarget(target, weight);
}
//---------------------------------------------------------------------------

bool ViewMesh::ParseDir(const char* filename)
{
  const char* colon = strchr (filename, ':');
  char* path;
  CS_ALLOC_STACK_ARRAY(char, fn, strlen(filename) + 1);
  if (colon)
  {
    int pathlen = colon-filename;
    path = new char[pathlen+1];
    strcpy (fn, colon+1);
    strncpy (path, filename, pathlen);
    path[pathlen] = 0;
    if (!vfs->ChDirAuto (path, 0, 0, colon+1))
    {
      ReportError ("Couldn't find '%s' in '%s'!", colon+1, path);
      delete[] path;
      return false;
    }
    // If there is a path after the colon we skip every entry in that
    // path until we end up with only the filename.
    char* slash = strpbrk (fn, "/\\");
    while (slash)
    {
      char rs = *slash;
      *slash = 0;
      if (!vfs->ChDir (fn))
        return false;
      *slash = rs;
      strcpy (fn, slash+1);
      slash = strpbrk (fn, "/\\");
    }
    return true;
  }
  else
  {
    // grab the directory.
    path = new char[strlen(filename)+1];
    strcpy (path, filename);
    strcpy (fn, path);
    char* slash = strrchr (path, '/');
    char* dir;
    if (slash)
    {
      strcpy (fn, slash + 1);
      *slash = 0;
      dir = path;
    }
    else
      dir = "/";
    if (!vfs->ChDir(dir))
      return false;
    return true;
  }
}

void ViewMesh::StdDlgUpdateLists(const char* filename)
{
  iAwsComponent* dirlist = stddlg->FindChild("DirList");
  iAwsComponent* filelist = stddlg->FindChild("FileList");
  iAwsParmList* pl = aws->CreateParmList();
  if (!dirlist || !filelist) return;

  dirlist->Execute("ClearList", pl);
  filelist->Execute("ClearList", pl);

  pl->AddString("text0", "..");
  dirlist->Execute("InsertItem", pl);
  pl->Clear();

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
      pl->AddString("text0", file);
      dirlist->Execute("InsertItem", pl);
      pl->Clear();
    }
    else
    {
      pl->AddString("text0", file);
      filelist->Execute("InsertItem", pl);
      pl->Clear();
    }
  }
}

//---------------------------------------------------------------------------

void ViewMesh::StdDlgOkButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* path=0;
  iString* file=0;

  tut->form->Show();
  tut->stddlg->Hide();

  iAwsComponent* inputpath = tut->stddlg->FindChild("InputPath");
  if (inputpath) inputpath->GetProperty("Text",(intptr_t*)&path);
  if (!path || !path->GetData()) return;

  iAwsComponent* inputfile = tut->stddlg->FindChild("InputFile");
  if (inputfile) inputfile->GetProperty("Text",(intptr_t*)&file);
  if (!file || !file->GetData()) return;

  switch (tut->stddlgPurpose)
  {
  case save:
    tut->SaveSprite(*path, *file, false);
    break;
  case savebinary:
    tut->SaveSprite(*path, *file, true);
    break;
  case load:
    tut->LoadSprite(*path, *file);
    break;
  case loadlib:
    tut->LoadLibrary(*path, *file);
    break;
  case attach:
    tut->AttachMesh(*path, *file);
    break;
  }
}

void ViewMesh::StdDlgCancleButton (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  tut->form->Show();
  tut->stddlg->Hide();
}

void ViewMesh::StdDlgFileSelect (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  iAwsComponent* list = tut->stddlg->FindChild("FileList");
  iAwsParmList* pl = tut->aws->CreateParmList();
  if (!list) return;

  pl->AddString("text0","");
  list->Execute("GetSelectedItem", pl);
  pl->GetString("text0",&text);

  if (!text->GetData()) return;

  iAwsComponent* InputPath = tut->stddlg->FindChild("InputFile");
  InputPath->SetProperty("Text", (intptr_t)text);
}

void ViewMesh::StdDlgDirSelect (intptr_t awst, iAwsSource *s)
{
  ViewMesh* tut = (ViewMesh*)awst;

  iString* text;
  iAwsComponent* list = tut->stddlg->FindChild("DirList");
  iAwsParmList* pl = tut->aws->CreateParmList();
  if (!list) return;

  pl->AddString("text0","");
  list->Execute("GetSelectedItem", pl);
  pl->GetString("text0",&text);

  if (!text->GetData()) return;

  csPrintf("cd %s\n",text->GetData());

  iString* path = 0;
  iAwsComponent* inputpath = tut->stddlg->FindChild("InputPath");
  if (inputpath) inputpath->GetProperty("Text",(intptr_t*)&path);
  if (!path || !path->GetData()) return;

  csRef<iString> newpath = path;

  if (csString("..") == *text)
  {
    size_t i = newpath->Slice(0,newpath->Length()-1)->FindLast('/')+1;
    csPrintf("%zu", i);
    newpath = newpath->Slice(0,i);
  }
  else
  {
     newpath->Append(text);
     newpath->Append("/");
  }

  if (!newpath->GetData()) newpath->Append("/");

  tut->ParseDir(newpath->GetData());

  iAwsComponent* InputPath = tut->stddlg->FindChild("InputPath");
  if (InputPath) InputPath->SetProperty("Text", (intptr_t)(iString*)newpath);
  tut->StdDlgUpdateLists(newpath->GetData());
}

//---------------------------------------------------------------------------

int main(int argc, char** argv)
{
  return ViewMesh().Main(argc, argv);
}
