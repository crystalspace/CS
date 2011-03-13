/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "hairtest.h"
#include "krystal.h"
#include "frankie.h"

#define MODEL_FRANKIE 1
#define MODEL_KRYSTAL 2

#define CS_EVENT_HANDLED true
#define CS_EVENT_UNHANDLED false

HairTest::HairTest ()
: DemoApplication ("CrystalSpace.HairTest"),
  avatarScene (0), avatarSceneType(MODEL_KRYSTAL), 
  furMeshEnabled (true), dynamicsDebugMode (DYNDEBUG_NONE)
{
  // Use a default rotate camera
  cameraManager.SetCameraMode (CS::Demo::CAMERA_ROTATE);

  // Don't display the available keys
  hudManager.keyDescriptions.DeleteAll ();
}

HairTest::~HairTest ()
{
  delete avatarScene;
}

void HairTest::PrintHelp ()
{
  csCommandLineHelper commandLineHelper;

  // Command line options
  commandLineHelper.AddCommandLineOption
    ("scene", "Set the starting scene", csVariant ("krystal"));

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (GetObjectRegistry (), "hairtest", "hairtest <OPTIONS>", "Tests on the fur mesh.");
}

void HairTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsedTime = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  const float speed = elapsedTime / 1000.0f;

  // Step the dynamic simulation (we slow down artificially the simulation in
  // order to achieve a 'slow motion' effect)
  if (physicsEnabled)
    dynamics->Step (speed * avatarScene->GetSimulationSpeed ());

  // Update the target of the camera
  cameraManager.SetCameraTarget (avatarScene->GetCameraTarget ());

  // Update the information on the current state of the application
  avatarScene->UpdateStateDescription ();

  // Default behavior from csDemoApplication
  DemoApplication::Frame ();

  // Display the Bullet debug information
  if (avatarScene->HasPhysicalObjects ()
    && dynamicsDebugMode == DYNDEBUG_BULLET)
    bulletDynamicSystem->DebugDraw (view);

  cegui->Render ();
}

bool HairTest::OnExitButtonClicked (const CEGUI::EventArgs&)
{
  csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
  return true;
}

bool HairTest::OnSaveButtonClicked (const CEGUI::EventArgs&)
{
  if (!avatarScene)
    return false;

  avatarScene->SaveFur();
  return true;
}

bool HairTest::OnCollidersButtonClicked (const CEGUI::EventArgs&)
{
  SwitchDynamics();
  return true;
}

bool HairTest::OnSceneButtonClicked (const CEGUI::EventArgs&)
{
  SwitchScenes();
  return true;
}

bool HairTest::OnKillButtonClicked (const CEGUI::EventArgs&)
{
  if (!avatarScene)
    return false;

  avatarScene->KillAvatar();
  return true;
}

bool HairTest::OnResetButtonClicked (const CEGUI::EventArgs&)
{
  if (!avatarScene)
    return false;

  avatarScene->ResetScene();
  return true;
}

// RGB hair color
bool HairTest::OnEventThumbTrackEndedR (const CEGUI::EventArgs&)
{
  CS::ShaderVarName objColor (svStrings, "fur color");	
  
  csVector4 color; 
  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objColor)->GetValue(color);
  
  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objColor)->
    SetValue(csVector4( sliderR->getScrollPosition(), color.y, color.z, color.w ) );

  return true;
}

bool HairTest::OnEventThumbTrackEndedG (const CEGUI::EventArgs&)
{
  CS::ShaderVarName objColor (svStrings, "fur color");	

  csVector4 color; 
  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objColor)->GetValue(color);

  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objColor)->
    SetValue(csVector4( color.x, sliderG->getScrollPosition(), color.z, color.w ) );

  return true;
}

bool HairTest::OnEventThumbTrackEndedB (const CEGUI::EventArgs&)
{
  CS::ShaderVarName objColor (svStrings, "fur color");	

  csVector4 color; 
  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objColor)->GetValue(color);

  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objColor)->
    SetValue(csVector4( color.x, color.y , sliderB->getScrollPosition(), color.w) );

  return true;
}

bool HairTest::OnEventThumbTrackEndedA (const CEGUI::EventArgs&)
{
  CS::ShaderVarName objColor (svStrings, "fur color");	

  csVector4 color; 
  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objColor)->GetValue(color);

  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objColor)->
    SetValue(csVector4( color.x, color.y , color.z, sliderA->getScrollPosition()) );

  return true;
}

bool HairTest::OnEventThumbTrackEndedPointiness (const CEGUI::EventArgs&)
{
  csRef<CS::Mesh::iFurMeshState> ifms = 
    scfQueryInterface<CS::Mesh::iFurMeshState>(avatarScene->furMesh);

  ifms->SetPointiness(sliderPointiness->getScrollPosition());

  return true;
}

bool HairTest::OnEventThumbTrackEndedStrandWidth (const CEGUI::EventArgs&)
{
  csRef<CS::Mesh::iFurMeshState> ifms = 
    scfQueryInterface<CS::Mesh::iFurMeshState>(avatarScene->furMesh);

  ifms->SetStrandWidth(sliderStrandWidth->getScrollPosition() * 0.01f);

  return true;
}

bool HairTest::OnEventThumbTrackEndedControlPointsDeviation 
  (const CEGUI::EventArgs&)
{
  csRef<CS::Mesh::iFurMeshState> ifms = 
    scfQueryInterface<CS::Mesh::iFurMeshState>(avatarScene->furMesh);

  ifms->SetControlPointsDeviation
    (sliderControlPointsDeviation->getScrollPosition() * 0.05f);

  return true;
}

bool HairTest::OnEventThumbTrackEndedGuideLOD (const CEGUI::EventArgs&)
{
  avatarScene->furMesh->SetGuideLOD(sliderGuideLOD->getScrollPosition());

  return true;
}

bool HairTest::OnEventThumbTrackEndedStrandLOD (const CEGUI::EventArgs&)
{
  avatarScene->furMesh->SetStrandLOD(sliderStrandLOD->getScrollPosition());

  return true;
}

bool HairTest::OnEventThumbTrackEndedControlPointsLOD (const CEGUI::EventArgs&)
{
  avatarScene->furMesh->
    SetControlPointsLOD(sliderControlPointsLOD->getScrollPosition());

  return true;
}

bool HairTest::OnEventThumbTrackEndedOverallLOD (const CEGUI::EventArgs&)
{
  avatarScene->furMesh->SetLOD(sliderOverallLOD->getScrollPosition());

  return true;
}

bool HairTest::OnPhysicsButtonClicked (const CEGUI::EventArgs&)
{
  avatarScene->SwitchFurPhysics();

  return true;
}

bool HairTest::OnEnableButtonClicked (const CEGUI::EventArgs&)
{
  furMeshEnabled = !furMeshEnabled;
  
  if (!avatarScene->furMesh)
    return false;

  if (furMeshEnabled)
    avatarScene->furMesh->EnableMesh();
  else
    avatarScene->furMesh->DisableMesh();

  return true;
}

bool HairTest::OnEventListSelectionChanged (const CEGUI::EventArgs&)
{
  CS::ShaderVarName diffuseType (svStrings, "diffuse type");	

  int type; 
  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(diffuseType)->GetValue(type);

  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(diffuseType)->
    SetValue((int)(objectComboBox->getSelectedItem()->getID()));  

  return true;
}

void HairTest::LoadTexture (const char* filename, const char* path, 
                            const char* shaderVariable)
{
  if (path)
    vfs->ChDir(path);
  
  CS::ShaderVarName objTexture (svStrings, shaderVariable);	

  csRef<iTextureHandle> texture = loader->LoadTexture (filename);

  if(texture)
    avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    GetVariableAdd(objTexture)->SetValue(texture);
  else 
    csPrintfErr("Can't open file: %s\n", filename);
}

//---------------------------------------------------------------------------

void HairTest::StdDlgUpdateLists(const char* filename)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* dirlist = 
    (CEGUI::Listbox*)winMgr->getWindow("HairTest/StdDlg/DirSelect");
  CEGUI::Listbox* filelist = 
    (CEGUI::Listbox*)winMgr->getWindow("HairTest/StdDlg/FileSelect");

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

bool HairTest::StdDlgOkButton (const CEGUI::EventArgs& e)
{
  form->show();
  stddlg->hide();

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* inputpath = winMgr->getWindow("HairTest/StdDlg/Path");
  CEGUI::String path = inputpath->getProperty("Text");
  if (path.empty()) return CS_EVENT_HANDLED;

  vfs->ChDir (path.c_str());

  CEGUI::Window* inputfile = winMgr->getWindow("HairTest/StdDlg/File");
  CEGUI::String file = inputfile->getProperty("Text");
  if (path.empty()) return CS_EVENT_HANDLED;

  CEGUI::String purpose = stddlg->getUserString("Purpose");

  LoadTexture(file.c_str(), path.c_str(), purpose.c_str());

  return CS_EVENT_HANDLED;
}

bool HairTest::StdDlgCancleButton (const CEGUI::EventArgs& e)
{
  form->show();
  stddlg->hide();
  return CS_EVENT_HANDLED;
}

bool HairTest::StdDlgFileSelect (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = 
    (CEGUI::Listbox*) winMgr->getWindow("HairTest/StdDlg/FileSelect");
  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  if (!item) return CS_EVENT_HANDLED;

  CEGUI::String text = item->getText();
  if (text.empty()) return CS_EVENT_HANDLED;

  CEGUI::Window* file = winMgr->getWindow("HairTest/StdDlg/File");
  file->setProperty("Text", text);
  return CS_EVENT_HANDLED;
}

bool HairTest::StdDlgDirSelect (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = 
    (CEGUI::Listbox*) winMgr->getWindow("HairTest/StdDlg/DirSelect");
  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  if (!item) return CS_EVENT_HANDLED;

  CEGUI::String text = item->getText();
  if (text.empty()) return CS_EVENT_HANDLED;

  csPrintf("cd %s\n",text.c_str());

  CEGUI::Window* inputpath = winMgr->getWindow("HairTest/StdDlg/Path");
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

bool HairTest::StdDlgDirChange (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* inputpath = winMgr->getWindow("HairTest/StdDlg/Path");
  CEGUI::String path = inputpath->getProperty("Text");
  if (path.empty()) return CS_EVENT_HANDLED;

  csPrintf("cd %s\n",path.c_str());

  vfs->ChDir (path.c_str ());

  inputpath->setProperty("Text", path.c_str());
  StdDlgUpdateLists(path.c_str());
  return CS_EVENT_HANDLED;
}

bool HairTest::LoadDiffuseMap (const CEGUI::EventArgs& e)
{
  form->hide();
  stddlg->show();
  stddlg->setUserString("Purpose", "diffuse map");
  return true;
}

bool HairTest::UnloadDiffuseMap (const CEGUI::EventArgs& e)
{
  CS::ShaderVarName objTexture (svStrings, "diffuse map");	

  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    RemoveVariable(objTexture);

  return true;
}

bool HairTest::LoadTextureMap (const CEGUI::EventArgs& e)
{
  form->hide();
  stddlg->show();
  stddlg->setUserString("Purpose", "texture map");
  return true;
}

bool HairTest::UnloadTextureMap (const CEGUI::EventArgs& e)
{
  CS::ShaderVarName objTexture (svStrings, "texture map");	

  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    RemoveVariable(objTexture);

  return true;
}

bool HairTest::LoadColorMap (const CEGUI::EventArgs& e)
{
  form->hide();
  stddlg->show();
  stddlg->setUserString("Purpose", "color map");
  return true;
}

bool HairTest::UnloadColorMap (const CEGUI::EventArgs& e)
{
  CS::ShaderVarName objTexture (svStrings, "color map");	

  avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
    RemoveVariable(objTexture);

  return true;
}


void HairTest::SwitchDynamics()
{
  csRef<iMeshObject> animeshObject = 
    scfQueryInterface<iMeshObject> (avatarScene->animesh);

  if (dynamicsDebugMode == DYNDEBUG_NONE)
  {
    dynamicsDebugMode = DYNDEBUG_MIXED;
    dynamicsDebugger->SetDebugDisplayMode (true);
    animeshObject->GetMeshWrapper ()->GetFlags ().Reset (CS_ENTITY_INVISIBLEMESH);
  }

  else if (dynamicsDebugMode == DYNDEBUG_MIXED)
  {
    dynamicsDebugMode = DYNDEBUG_COLLIDER;
    dynamicsDebugger->SetDebugDisplayMode (true);
    animeshObject->GetMeshWrapper ()->GetFlags ().Set (CS_ENTITY_INVISIBLEMESH);
  }

  else if (dynamicsDebugMode == DYNDEBUG_COLLIDER)
  {
    dynamicsDebugMode = DYNDEBUG_BULLET;
    dynamicsDebugger->SetDebugDisplayMode (false);
    animeshObject->GetMeshWrapper ()->GetFlags ().Reset (CS_ENTITY_INVISIBLEMESH);
  }

  else if (dynamicsDebugMode == DYNDEBUG_BULLET)
  {
    dynamicsDebugMode = DYNDEBUG_NONE;
    dynamicsDebugger->SetDebugDisplayMode (false);
    animeshObject->GetMeshWrapper ()->GetFlags ().Reset (CS_ENTITY_INVISIBLEMESH);
  }
}

void HairTest::SwitchScenes()
{
  delete avatarScene;

  if (avatarSceneType == MODEL_FRANKIE)
  {
    avatarSceneType = MODEL_KRYSTAL;
    avatarScene = new KrystalScene (this);
  }

  else if (avatarSceneType == MODEL_KRYSTAL)
  {
    avatarSceneType = MODEL_FRANKIE;
    avatarScene = new FrankieScene (this);
  }

  if (!avatarScene->CreateAvatar ())
  {
    csPrintfErr ("Problem loading scene. Exiting.\n");
    csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (GetObjectRegistry ()));
    if (q) q->GetEventOutlet()->Broadcast (csevQuit (GetObjectRegistry ()));
    return;
  }

  // Re-initialize camera position
  cameraManager.SetCameraTarget (avatarScene->GetCameraTarget ());
  cameraManager.ResetCamera ();

  furMeshEnabled = true;
}

void HairTest::SaveObject(iMeshWrapper* meshwrap, const char * filename)
{
  csPrintf("Saving object to file %s...\n", filename);

  csRef<iDocumentSystem> xml(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();

  iMeshFactoryWrapper* meshfactwrap = meshwrap->GetFactory();
  iMeshObjectFactory*  meshfact = meshfactwrap->GetMeshObjectFactory();
  iMeshObject* obj = meshwrap->GetMeshObject();

  //Create the Tag for the MeshObj
  csRef<iDocumentNode> factNode = root->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  factNode->SetValue("meshobj");

  //Add the mesh's name to the MeshObj tag
  const char* name = meshwrap->QueryObject()->GetName();
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
  csReplaceAll(loadername, pluginname, ".object.", ".loader.",
    sizeof(loadername));

  pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(loadername);
  csRef<iPluginManager> plugin_mgr = 
    csQueryRegistry<iPluginManager> (object_reg);

  char savername[128] = "";

  csReplaceAll(savername, pluginname, ".object.", ".saver.",
    sizeof(savername));

  csRef<iSaverPlugin> saver =  csLoadPluginCheck<iSaverPlugin> (
    plugin_mgr, savername);
  if (saver) 
    saver->WriteDown(obj, factNode, 0/*ssource*/);

  scfString str;
  doc->Write(&str);
  vfs->WriteFile(filename, str.GetData(), str.Length());
}

void HairTest::SaveFactory(iMeshFactoryWrapper* meshfactwrap, const char * filename)
{
  csPrintf("Saving factory to file %s...\n", filename);

  csRef<iDocumentSystem> xml(new csTinyDocumentSystem());
  csRef<iDocument> doc = xml->CreateDocument();
  csRef<iDocumentNode> root = doc->CreateRoot();

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

  pluginNode->CreateNodeBefore(CS_NODE_TEXT)->SetValue(loadername);
  csRef<iPluginManager> plugin_mgr = 
    csQueryRegistry<iPluginManager> (object_reg);

  char savername[128] = "";

  csReplaceAll(savername, pluginname, ".object.", ".saver.factory.",
    sizeof(savername));

  csRef<iSaverPlugin> saver =  csLoadPluginCheck<iSaverPlugin> (
    plugin_mgr, savername);
  if (saver) 
    saver->WriteDown(meshfact, factNode, 0/*ssource*/);

  scfString str;
  doc->Write(&str);
  vfs->WriteFile(filename, str.GetData(), str.Length());
}

bool HairTest::OnKeyboard (iEvent &ev)
{
  // Default behavior from csDemoApplication
  DemoApplication::OnKeyboard (ev);

  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // Toggle the debug mode of the dynamic system
    if (csKeyEventHelper::GetCookedCode (&ev) == 'd'
      && physicsEnabled && avatarScene->HasPhysicalObjects ())
    {
      SwitchDynamics();
      return true;
    }

    // Toggle physics control
    if (csKeyEventHelper::GetCookedCode (&ev) == 'e'
      && physicsEnabled && avatarScene->HasPhysicalObjects ())
    {
      avatarScene->SwitchFurPhysics();
      return true;
    }

    // Kill avatar
    if (csKeyEventHelper::GetCookedCode (&ev) == 'k')
    {
      avatarScene->KillAvatar();
      return true;
    }

    // Reset scene
    if (csKeyEventHelper::GetCookedCode (&ev) == 'r')
    {
      avatarScene->ResetScene();
      return true;
    }

    // Save fur
    if (csKeyEventHelper::GetCookedCode (&ev) == 's')
    {
      avatarScene->SaveFur();
      return true;
    }

    // Check for switching of scene
    if (csKeyEventHelper::GetCookedCode (&ev) == 'n')
    {
      SwitchScenes();
      return true;
    }

    // Enable / disable fur mesh
    if (csKeyEventHelper::GetCookedCode (&ev) == 'q'  && avatarScene->furMesh)
    {
      furMeshEnabled = !furMeshEnabled;

      if (furMeshEnabled)
        avatarScene->furMesh->EnableMesh();
      else
        avatarScene->furMesh->DisableMesh();

      return true;
    }

  }
  return false;
}

bool HairTest::OnMouseDown (iEvent& ev)
{
  // Default behavior from csDemoApplication
//   if (DemoApplication::OnMouseDown (ev))
//     return true;

  return false;
}

bool HairTest::OnInitialize (int argc, char* argv[])
{
  // Default behavior from csDemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.lookat",
    CS::Animation::iSkeletonLookAtNodeManager),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.animnode.speed",
    CS::Animation::iSkeletonSpeedNodeManager),
    CS_REQUEST_PLUGIN("crystalspace.mesh.object.furmesh", CS::Mesh::iFurMeshType),
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) return ReportError("Failed to locate 3D engine!");
  engine->SetSaveableFlag(true);

  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_LEVELSAVER,
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  // Shader variables
  svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
    object_reg, "crystalspace.shader.variablenameset");

  if (!svStrings) 
  {
    ReportError ("No SV names string set!\n");
    return false;
  }

  // Level saver
  saver = csQueryRegistry<iSaver> (GetObjectRegistry ());
  if (!saver) return ReportError("Failed to locate Saver!");

  // Check if physical effects are enabled
  csRef<iCommandLineParser> clp =
    csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());
  physicsEnabled = true;

  while (physicsEnabled)
  {
    // Load the Bullet plugin
    csRef<iPluginManager> plugmgr = 
      csQueryRegistry<iPluginManager> (GetObjectRegistry ());
    dynamics = csLoadPlugin<iDynamics> (plugmgr, "crystalspace.dynamics.bullet");

    if (!dynamics)
    {
      ReportWarning
        ("Can't load Bullet plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    // Load the dynamics debugger
    debuggerManager = csLoadPlugin<CS::Debug::iDynamicsDebuggerManager>
      (plugmgr, "crystalspace.dynamics.debug");

    if (!debuggerManager)
    {
      ReportWarning
        ("Can't load Dynamics Debugger plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    // Load the ragdoll plugin
    ragdollManager = csLoadPlugin<CS::Animation::iSkeletonRagdollNodeManager>
      (plugmgr, "crystalspace.mesh.animesh.animnode.ragdoll");

    if (!ragdollManager)
    {
      ReportWarning
        ("Can't load ragdoll plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    break;
  }

  // Read which scene to display at first
  csString sceneName = clp->GetOption ("scene");
  if (!sceneName.IsEmpty ())
  {
    if (sceneName == "krystal")
      avatarSceneType = MODEL_KRYSTAL;
    else if (sceneName == "frankie")
      avatarSceneType = MODEL_FRANKIE;
    else
      printf ("Given scene (%s) is not one of {%s, %s}. Falling back to Krystal\n", 
        CS::Quote::Single (sceneName.GetData ()),
	CS::Quote::Single ("krystal"),
	CS::Quote::Single ("frankie"));
  }

  return true;
}

bool HairTest::Application ()
{
  // Default behavior from csDemoApplication
  if (!DemoApplication::Application ())
    return false;

  if(!engine) return ReportError("Failed to locate Engine!");

  engine->SetSaveableFlag(true);

  // Find references to the plugins of the animation nodes
  lookAtManager = 
    csQueryRegistry<CS::Animation::iSkeletonLookAtNodeManager> (GetObjectRegistry ());
  if (!lookAtManager) 
    return ReportError("Failed to locate iLookAtNodeManager plugin!");

  basicNodesManager =
    csQueryRegistry<CS::Animation::iSkeletonSpeedNodeManager> (GetObjectRegistry ());
  if (!basicNodesManager)
    return ReportError("Failed to locate CS::Animation::iSkeletonSpeedNodeManager plugin!");

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!cegui) return ReportError("Failed to locate CEGUI plugin!");

  vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return ReportError("Failed to locate VFS!");

  // Initialize GUI

  // Initialize CEGUI wrapper
  cegui->Initialize ();

  // Set the logging level
  cegui->GetLoggerPtr ()->setLoggingLevel(CEGUI::Informative);

  vfs->ChDir ("/cegui/");

  // Load the ice skin (which uses Falagard skinning system)
  cegui->GetSchemeManagerPtr ()->create("ice.scheme");

  cegui->GetSystemPtr ()->setDefaultMouseCursor("ice", "MouseArrow");

  cegui->GetFontManagerPtr ()->
    createFreeTypeFont("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  vfs->ChDir ("/lib/hairtest/");
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("hairtest.layout"));

  // Load the two windows
  form = winMgr->getWindow("HairTest/MainWindow");
  stddlg = winMgr->getWindow("HairTest/StdDlg");

  // Subscribe to the clicked event for the exit button
  CEGUI::Window* btn = winMgr->getWindow("HairTest/MainWindow/Tab/Page1/Quit");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::OnExitButtonClicked, this));

  winMgr->getWindow("HairTest/MainWindow/Tab/Page1/Save")-> 
    subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::OnSaveButtonClicked, this));

  winMgr->getWindow("HairTest/MainWindow/Tab/Page1/Colliders")-> 
    subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HairTest::OnCollidersButtonClicked, this));

  winMgr->getWindow("HairTest/MainWindow/Tab/Page1/Scene")-> 
    subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::OnSceneButtonClicked, this));

  winMgr->getWindow("HairTest/MainWindow/Tab/Page1/Kill")-> 
    subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::OnKillButtonClicked, this));

  winMgr->getWindow("HairTest/MainWindow/Tab/Page1/Reset")-> 
    subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::OnResetButtonClicked, this));

  winMgr->getWindow("HairTest/MainWindow/Tab/Page2/Physics")-> 
    subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HairTest::OnPhysicsButtonClicked, this));

  winMgr->getWindow("HairTest/MainWindow/Tab/Page2/Enable")-> 
    subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::OnEnableButtonClicked, this));

  // ----[ STDDLG ]----------------------------------------------------------

  btn = winMgr->getWindow("HairTest/StdDlg/OkButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::StdDlgOkButton, this));

  btn = winMgr->getWindow("HairTest/StdDlg/CancleButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::StdDlgCancleButton, this));

  btn = winMgr->getWindow("HairTest/StdDlg/FileSelect");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&HairTest::StdDlgFileSelect, this));

  btn = winMgr->getWindow("HairTest/StdDlg/DirSelect");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&HairTest::StdDlgDirSelect, this));

  btn = winMgr->getWindow("HairTest/StdDlg/Path");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&HairTest::StdDlgDirChange, this));

  // ------------------------------------------------------------------------
  vfs->ChDir ("/lib/hairtest/");
  btn = winMgr->getWindow("HairTest/StdDlg/Path");
  btn->setProperty("Text", vfs->GetCwd());
  StdDlgUpdateLists(vfs->GetCwd());

  // Default behavior from csDemoApplication for the creation of the scene
  if (!DemoApplication::CreateRoom ())
    return false;

  // Create the dynamic system
  if (physicsEnabled)
  {
    dynamicSystem = dynamics->CreateSystem ();
    if (!dynamicSystem) 
    {
      ReportWarning
        ("Can't create dynamic system, continuing with reduced functionalities");
      physicsEnabled = false;
    }

    else
    {
      // Find the Bullet interface of the dynamic system
      bulletDynamicSystem =
        scfQueryInterface<CS::Physics::Bullet::iDynamicSystem> (dynamicSystem);

      // We have some objects of size smaller than 0.035 units, so we scale up the
      // whole world for a better behavior of the dynamic simulation.
      bulletDynamicSystem->SetInternalScale (10.0f);

      // The ragdoll model of Krystal is rather complex, and the model of Frankie
      // is unstable because of the overlap of its colliders. We therefore use high
      // accuracy/low performance parameters for a better behavior of the dynamic
      // simulation.
      bulletDynamicSystem->SetStepParameters (0.008f, 150, 10);

      // Create the dynamic's debugger
      dynamicsDebugger = debuggerManager->CreateDebugger ();
      dynamicsDebugger->SetDynamicSystem (dynamicSystem);
      dynamicsDebugger->SetDebugSector (room);

      bulletDynamicSystem->SetSoftBodyWorld (true);

      // Set up the physical collider for the roof (soft bodies don't like plane
      // colliders, so use a box instead)
      csOrthoTransform t;
      t.SetOrigin(csVector3(0.0f, -50.0f, 0.0f));
      dynamicSystem->AttachColliderBox (csVector3 (100.0f), t, 10.0f, 0.0f);
    }
  }

  // Set lights
  room->GetLights()->RemoveAll();

  // This light is for the background
  csRef<iLight> light = 
    engine->CreateLight(0, csVector3(10, 10, -10), 9000, csColor (1));
  light->SetAttenuationMode (CS_ATTN_NONE);
  room->GetLights()->Add (light);

  // Create avatar
  if (avatarSceneType == MODEL_KRYSTAL)
    avatarScene = new KrystalScene (this);
  else if (avatarSceneType == MODEL_FRANKIE)
    avatarScene = new FrankieScene (this);
  
  if (!avatarScene->CreateAvatar ())
    return false;

  // Set default color sliders
  CS::ShaderVarName objColor (svStrings, "hair color");	

  csVector3 color; 
  if (avatarScene->furMesh && avatarScene->furMesh->GetFurMeshProperties() && 
      avatarScene->furMesh->GetFurMeshProperties()->GetMaterial())
  {
    
    winMgr->getWindow("HairTest/MainWindow/Tab/Page3/LoadDiffuseMap")->
      subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HairTest::LoadDiffuseMap, this));

    winMgr->getWindow("HairTest/MainWindow/Tab/Page3/UnloadDiffuseMap")->
      subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HairTest::UnloadDiffuseMap, this));    

    winMgr->getWindow("HairTest/MainWindow/Tab/Page3/LoadTextureMap")->
      subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HairTest::LoadTextureMap, this));

    winMgr->getWindow("HairTest/MainWindow/Tab/Page3/UnloadTextureMap")->
      subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HairTest::UnloadTextureMap, this));    

    winMgr->getWindow("HairTest/MainWindow/Tab/Page3/LoadColorMap")->
      subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HairTest::LoadColorMap, this));

    winMgr->getWindow("HairTest/MainWindow/Tab/Page3/UnloadDiffuseMap")->
      subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&HairTest::UnloadDiffuseMap, this));    

    // RGB hair color
    CS::ShaderVarName objColor (svStrings, "fur color");	

    csVector4 color; 
    avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
      GetVariableAdd(objColor)->GetValue(color);

    sliderR = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page3/Slider1");

    sliderR->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedR, this));  

    sliderR->setScrollPosition(color.x);

    sliderG = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page3/Slider2");

    sliderG->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedG, this));  

    sliderG->setScrollPosition(color.y);

    sliderB = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page3/Slider3");

    sliderB->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedB, this));  

    sliderB->setScrollPosition(color.z);

    sliderA = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page3/Slider4");

    sliderA->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedA, this));  

    sliderA->setScrollPosition(color.w);

    sliderPointiness = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page1/Slider1");

    sliderPointiness->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedPointiness, this)); 

    sliderStrandWidth = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page1/Slider2");

    sliderStrandWidth->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedStrandWidth, this)); 

    sliderControlPointsDeviation = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page1/Slider3");

    sliderControlPointsDeviation->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedControlPointsDeviation, this)); 

    csRef<CS::Mesh::iFurMeshState> ifms = 
      scfQueryInterface<CS::Mesh::iFurMeshState>(avatarScene->furMesh);

    sliderPointiness->setScrollPosition(ifms->GetPointiness());    

    sliderStrandWidth->setScrollPosition(ifms->GetStrandWidth() * 100.0f);

    sliderControlPointsDeviation->setScrollPosition(ifms->GetStrandWidth() * 50.0f);

    avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()
      ->GetVariableAdd(objColor)->GetValue(color);
    sliderR->setScrollPosition(color.x);
    sliderG->setScrollPosition(color.y);
    sliderB->setScrollPosition(color.z);

    objectComboBox = (CEGUI::Combobox*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page3/Combobox");

    CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem( "Disabled", 0 );
    objectComboBox->addItem( item );
    item = new CEGUI::ListboxTextItem( "CS", 1 );
    objectComboBox->addItem( item );
    item = new CEGUI::ListboxTextItem( "NVidia", 2 );
    objectComboBox->addItem( item ); 
    item = new CEGUI::ListboxTextItem( "Kajiya&Kay", 3 );
    objectComboBox->addItem( item ); 
    item = new CEGUI::ListboxTextItem( "Ambient", 4 );
    objectComboBox->addItem( item ); 

    objectComboBox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
      CEGUI::Event::Subscriber(&HairTest::OnEventListSelectionChanged, this));

    CS::ShaderVarName diffuseType (svStrings, "diffuse type");	

    int type; 
    avatarScene->furMesh->GetFurMeshProperties()->GetMaterial()->
      GetVariableAdd(diffuseType)->GetValue(type);

    objectComboBox->setText(objectComboBox->getListboxItemFromIndex(type)->getText() );
  }

  sliderGuideLOD = (CEGUI::Scrollbar*)winMgr->
    getWindow("HairTest/MainWindow/Tab/Page2/Slider1");

  sliderGuideLOD->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
    CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedGuideLOD, this));  

  sliderStrandLOD = (CEGUI::Scrollbar*)winMgr->
    getWindow("HairTest/MainWindow/Tab/Page2/Slider2");

  sliderStrandLOD->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
    CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedStrandLOD, this));  

  sliderStrandLOD->setScrollPosition(1.0f);

  sliderControlPointsLOD = (CEGUI::Scrollbar*)winMgr->
    getWindow("HairTest/MainWindow/Tab/Page2/Slider3");

  sliderControlPointsLOD->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
    CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedControlPointsLOD, this)); 

  sliderOverallLOD = (CEGUI::Scrollbar*)winMgr->
    getWindow("HairTest/MainWindow/Tab/Page2/Slider4");

  sliderOverallLOD->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
    CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedOverallLOD, this)); 

  // Initialize camera position
  cameraManager.SetCameraTarget (avatarScene->GetCameraTarget ());
  cameraManager.ResetCamera ();

  // Run the application
  Run();

  return true;
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return HairTest ().Main (argc, argv);
}
