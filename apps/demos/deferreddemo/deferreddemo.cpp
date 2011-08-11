/*
  Copyright (C) 2010 by Joe Forte

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

#include "deferreddemo.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include <CEGUIWindowManager.h>
#include <CEGUILogger.h>
#include "csutil/custom_new_enable.h"

const char *DEFAULT_CFG_WORLDDIR = "/lev/castle";
const char *DEFAULT_CFG_LOGOFILE = "/lib/std/cslogo2.png";

const char* ballMaterialNames[4] = { "red", "green", "blue", "yellow" };
const csColor ballMaterialColors[4] = { csColor (1.0f, 0.0f, 0.0f),
					csColor (0.0f, 1.0f, 0.0f),
					csColor (0.0f, 0.0f, 1.0f),
					csColor (1.0f, 1.0f, 0.0f) };

//----------------------------------------------------------------------
DeferredDemo::DeferredDemo()
:
viewRotX(0.0f),
viewRotY(0.0f),
shouldShutdown(false)
{
  SetApplicationName ("CrystalSpace.DeferredDemo");

  // Sets default cfg values.
  cfgWorldDir = DEFAULT_CFG_WORLDDIR;
  cfgWorldFile = "world";
  
  cfgDrawLogo = true;
  cfgUseDeferredShading = true;
}

//----------------------------------------------------------------------
DeferredDemo::~DeferredDemo()
{}

//----------------------------------------------------------------------
bool DeferredDemo::OnInitialize(int argc, char *argv[])
{
  if (!csInitializer::RequestPlugins (GetObjectRegistry(),
      CS_REQUEST_VFS,
      CS_REQUEST_OPENGL3D,
      CS_REQUEST_ENGINE,
      CS_REQUEST_FONTSERVER,
      CS_REQUEST_IMAGELOADER,
      CS_REQUEST_LEVELLOADER,
      CS_REQUEST_REPORTER,
      CS_REQUEST_REPORTERLISTENER,
      CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
      CS_REQUEST_PLUGIN ("crystalspace.dynamics.debug", CS::Debug::iDynamicsDebuggerManager),
      CS_REQUEST_END))
  {
    return ReportError("Failed to initialize plugins!");
  }

  csBaseEventHandler::Initialize (GetObjectRegistry());
  csEventID events[] = 
  {
    /* List of events to listen to. */

    csevQuit (GetObjectRegistry()),
    csevFrame (GetObjectRegistry()),
    csevKeyboardEvent (GetObjectRegistry()),
    csevCommandLineHelp (GetObjectRegistry()),
    csevMouseEvent (GetObjectRegistry()),
    CS_EVENTLIST_END
  };

  if (!RegisterQueue (GetObjectRegistry(), events))
  {
    return ReportError("Failed to set up event handler!");
  }

  // Setup default values.
  viewRotX = 0.0f;
  viewRotY = 0.0f;

  shouldShutdown = false;
  
  // Setup chached event names.
  quitEventID = csevQuit (GetObjectRegistry());
  cmdLineHelpEventID = csevCommandLineHelp (GetObjectRegistry());

  // Load deferred demo config file.
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  cfg->AddDomain ("/config/deferreddemo.cfg", vfs, iConfigManager::ConfigPriorityPlugin);

  csRef<iBugPlug> bugPlug = csQueryRegistry<iBugPlug> (GetObjectRegistry());  

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupModules()
{
  eventQueue = csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (!eventQueue) 
    return ReportError("Failed to locate Event Queue!");

  graphics2D = csQueryRegistry<iGraphics2D> (GetObjectRegistry());
  if (!graphics2D) 
    return ReportError("Failed to locate 2D renderer!");

  graphics3D = csQueryRegistry<iGraphics3D> (GetObjectRegistry());
  if (!graphics3D) 
    return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry());
  if (!engine) 
    return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry());
  if (!vc) 
    return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry());
  if (!kbd) 
    return ReportError("Failed to locate Keyboard Driver!");

  md = csQueryRegistry<iMouseDriver> (GetObjectRegistry());
  if (!md) 
    return ReportError("Failed to locate Mouse Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry());
  if (!loader) 
    return ReportError("Failed to locate Loader!");

  csRef<iShaderManager> shaderManager = csQueryRegistry<iShaderManager> (GetObjectRegistry());
  if (!shaderManager)
    return ReportError("Failed to locate shader manager!");

  svStringSet = shaderManager->GetSVNameStringset();

  cegui = csQueryRegistry<iCEGUI> (GetObjectRegistry());
  if (!loader) 
    return ReportError("Failed to locate CEGUI!");

  dynamicsDebuggerManager = csQueryRegistry<CS::Debug::iDynamicsDebuggerManager> (GetObjectRegistry());
  if (!dynamicsDebuggerManager)
    return ReportError ("Failed to locate Dynamic's Debugger Manager!");
  
  csRef<iPluginManager> pluginManager = csQueryRegistry<iPluginManager> (GetObjectRegistry ());
  if (!pluginManager)
    return ReportError ("Failed to locate Plugin Manager!");

  hudManager = csLoadPlugin<CS::Utility::iHUDManager>(pluginManager, "crystalspace.utilities.texthud");
  if (!hudManager)
    return ReportError ("Failed to locate HUD manager!");

  // Load the screenshot configuration
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  screenshotFormat = cfg->GetStr ("Deferreddemo.Settings.Screenshot.ImageFormat", "jpg");
  csString screenshotMask = cfg->GetStr ("Deferreddemo.Settings.Screenshot.FilenameFormat",
					    "/tmp/CS_DeferredDemo_0000");
  screenshotHelper.SetMask (screenshotMask + "." + screenshotFormat);

  /* NOTE: Config settings for render managers are stored in 'engine.cfg' 
   * and are needed when loading a render manager. Normally these settings 
   * are added by the engine when it loads a render manager. However, since
   * we are loading the deferred render manager manually we must also manually
   * add the proper config file. */
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());  
  cfg->AddDomain ("/config/engine.cfg", vfs, iConfigManager::ConfigPriorityPlugin);

  rm = csLoadPlugin<iRenderManager> (GetObjectRegistry(), "crystalspace.rendermanager.deferred");
  if (!rm)
    return ReportError("Failed to load deferred Render Manager!");

  cfg->RemoveDomain ("/config/engine.cfg");

  rm_debug = scfQueryInterface<iDebugHelper> (rm);
  if (!rm_debug)
    return ReportError("Failed to query the deferred Render Manager debug helper!");

  rm_default = engine->GetRenderManager ();

  rmGlobalIllum = scfQueryInterface<iRenderManagerGlobalIllum> (rm);
  if (!rmGlobalIllum)
    return ReportError ("Failed to query the deferred Render Manager global illumination interface!");

  SetupDynamicsSystem (pluginManager);

  return true;
}

bool DeferredDemo::SetupDynamicsSystem(iPluginManager *pluginManager)
{
  // Setup physics subsystem
  dynamics = csLoadPlugin<iDynamics> (pluginManager, "crystalspace.dynamics.bullet");
  if (!dynamics)
    return ReportError ("Failed to load iDynamics plugin!");

  dynamicSystem = dynamics->CreateSystem ();
  if (!dynamicSystem) 
    return ReportError ("Failed to create dynamic system!");

  // Set some linear and angular dampening in order to have a reduction of
  // the movements of the objects
  dynamicSystem->SetLinearDampener(0.1f);
  dynamicSystem->SetRollingDampener(0.1f);

  bulletDynamicSystem = scfQueryInterface<CS::Physics::Bullet::iDynamicSystem> (dynamicSystem);

  // Scale up the whole world for a better behavior of the dynamic simulation.
  bulletDynamicSystem->SetInternalScale (10.0f);
    
  dynamicsDebugger = dynamicsDebuggerManager->CreateDebugger();
  dynamicsDebugger->SetDynamicSystem (dynamicSystem);

  // Don't display static colliders as the z-fighting with the original mesh is very ugly
  dynamicsDebugger->SetStaticBodyMaterial (nullptr);

  isBulletEnabled = true;
  doBulletDebug = false;
  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadScene()
{
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs->ChDir (cfgWorldDir))
    return ReportError("Could not navigate to level directory %s!",
		       CS::Quote::Single (cfgWorldDir.GetDataSafe ()));

  if (!loader->LoadMapFile (cfgWorldFile))
     return ReportError("Could not load level!");

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadAppData()
{
  LoadLogo ();

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadSettings()
{
  const char *val = NULL;

  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (GetObjectRegistry());

  val = cmdline->GetOption ("worlddir");
  if (val)
    cfgWorldDir = val;
  else
    cfgWorldDir = cfg->GetStr ("Deferreddemo.Settings.WorldDirectory", DEFAULT_CFG_WORLDDIR);

  cfgWorldFile = cfg->GetStr ("Deferreddemo.Settings.WorldFile", "world");
  cfgLogoFile = cfg->GetStr ("Deferreddemo.Settings.LogoFile", DEFAULT_CFG_LOGOFILE);


  if (cmdline->GetOption ("nologo"))
    cfgDrawLogo = false;
  else
    cfgDrawLogo = cfg->GetBool ("Deferreddemo.Settings.DrawLogo", true);

  if (cmdline->GetOption ("forward"))
    cfgUseDeferredShading = false;
  else
    cfgUseDeferredShading = !cfg->GetBool ("Deferreddemo.Settings.UseForward", false);

  cfgShowGui = cfg->GetBool ("Deferreddemo.Settings.ShowGui", true);

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupGui(bool reload)
{
  // Initialize the HUD manager
  hudManager->GetKeyDescriptions()->Empty ();

  configEventNotifier.AttachNew(new CS::Utility::ConfigEventNotifier(GetObjectRegistry()));

  occlusionStrengthListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.OcclusionStrength", occlusionStrength));
  sampleRadiusListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.SampleRadius", sampleRadius));
  detailSampleRadiusListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.DetailSampleRadius", detailSampleRadius));
  aoPassesListener.AttachNew (new CS::Utility::ConfigListener<int>(GetObjectRegistry(), 
    "DeferredDemo.NumPasses", aoPasses));
  maxOccluderDistListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.MaxOccluderDist", maxOccluderDistance));  
  selfOcclusionListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.SelfOcclusion", selfOcclusion));
  occAngleBiasListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.AngleBias", occAngleBias));
  bounceStrengthListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.BounceStrength", bounceStrength));
  blurKernelSizeListener.AttachNew (new CS::Utility::ConfigListener<int>(GetObjectRegistry(), 
    "DeferredDemo.KernelSize", blurKernelSize));
  blurPositionThresholdListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.PositionThreshold", blurPositionThreshold));
  blurNormalThresholdListener.AttachNew (new CS::Utility::ConfigListener<float>(GetObjectRegistry(), 
    "DeferredDemo.NormalThreshold", blurNormalThreshold));

  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());

  if (!reload)
  {
    cegui->Initialize ();
    cegui->GetLoggerPtr ()->setLoggingLevel (CEGUI::Informative);

    // Load the ice skin.
    vfs->ChDir ("/cegui/");
    cegui->GetSchemeManagerPtr ()->create ("ice.scheme");
    cegui->GetSystemPtr ()->setDefaultMouseCursor ("ice", "MouseArrow");

    cegui->GetFontManagerPtr ()->createFreeTypeFont ("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");
  }

  // Load layout and set as root
  CEGUI::WindowManager *winMgr = cegui->GetWindowManagerPtr ();

  if (reload)
  {
    winMgr->getWindow ("root")->destroy ();
  }
  
  vfs->ChDir ("/data/deferreddemo/");
  cegui->GetSystemPtr ()->setGUISheet (winMgr->loadWindowLayout("deferreddemo.layout"));

  guiRoot             = winMgr->getWindow ("root");
  guiDeferred         = static_cast<CEGUI::RadioButton*>(winMgr->getWindow ("Deferred"));
  guiForward          = static_cast<CEGUI::RadioButton*>(winMgr->getWindow ("Forward"));
  guiShowGBuffer      = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("ShowGBuffer"));
  guiDrawLightVolumes = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("DrawLightVolumes"));
  guiDrawLogo         = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("DrawLogo"));  
  guiEnableAO         = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableAO"));
  guiEnableBlur       = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableBlur"));
  guiEnableDetailSamples = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableDetailSamples"));
  guiEnableGlobalIllum   = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableGlobalIllum"));
  guiEnableIndirectLight = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableIndirectLight"));
  //CEGUI::Combobox *guiCombo = static_cast<CEGUI::Combobox*>(winMgr->getWindow ("RenderBuffer"));  
  //guiCombo->addItem (new CEGUI::ListboxTextItem ("Ambient Occlusion"));

  if (!guiRoot || 
      !guiDeferred || !guiForward || !guiEnableGlobalIllum ||
      !guiShowGBuffer || !guiDrawLightVolumes || !guiDrawLogo ||
      !guiEnableAO || !guiEnableBlur || !guiEnableIndirectLight || !guiEnableDetailSamples)
  {
    return ReportError("Could not load GUI!");
  }

  if (cfgUseDeferredShading)
    guiDeferred->setSelected (true);
  else
    guiForward->setSelected (true);

  guiShowGBuffer->setSelected (false);
  guiDrawLightVolumes->setSelected (false);
  guiDrawLogo->setSelected (cfgDrawLogo);
  guiEnableAO->setSelected (true);
  guiEnableBlur->setSelected (true);
  guiEnableIndirectLight->setSelected (true);
  guiEnableDetailSamples->setSelected (true);
  guiEnableGlobalIllum->setSelected (true);
  
  occlusionStrength = 1.4f;
  sampleRadius = 0.25f;
  detailSampleRadius = 0.05f;
  aoPasses = 2;
  maxOccluderDistance = 2.0f;
  selfOcclusion = 0.0f;
  occAngleBias = 0.0f;
  bounceStrength = 6.5f;
  blurKernelSize = 3;
  blurPositionThreshold = 0.5f;
  blurNormalThreshold = 0.2f;

  showGBuffer = false;
  drawLightVolumes = false;
  showAmbientOcclusion = false;
  showGlobalIllumination = false;
  enableGlobalIllum = true;

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupScene()
{
  // Setup camera  
  csVector3 pos (0, 0, 0);
  if (engine->GetCameraPositions ()->GetCount () > 0)
  {
    iCameraPosition* campos = engine->GetCameraPositions ()->Get (0);
    room = engine->GetSectors ()->FindByName (campos->GetSector ());
    pos = campos->GetPosition ();
  }
  else
  {
    // There are no valid starting point for the camera so we just start at the origin.
    room = engine->GetSectors ()->FindByName ("room");
    pos = csVector3 (0, 0, 0);
  }

  if (!room)
    return ReportError("Can't find a valid starting position!");

  view.AttachNew ( new csView (engine, graphics3D) );
  view->SetRectangle (0, 0, graphics2D->GetWidth (), graphics2D->GetHeight ());
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (pos);
  
  csPlane3 *farPlane = new csPlane3 (0, 0, -1, 1000);
  view->GetCamera()->SetFarPlane (farPlane);
  view->GetPerspectiveCamera()->SetNearClipDistance (0.2f);
  delete farPlane;

  // Checks for support of at least 4 color buffer attachment points.
  const csGraphics3DCaps *caps = graphics3D->GetCaps();
  if (caps->MaxRTColorAttachments < 3)
    return ReportError("Graphics3D does not support at least 3 color buffer attachments!");
  else
    ReportInfo("Graphics3D supports %d color buffer attachments.", caps->MaxRTColorAttachments);  
  
  // Create 5 ball factories with different sizes
  for (int i=0; i < 6; i++)
  {
    ballFact[i] = engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh", 
      csString ("ballFact") + csString ((char)('0' + i)));
    if (!ballFact[i])
      ReportError ("Failed to create mesh object factory!");

    csRef<iGeneralFactoryState> factoryState = 
      scfQueryInterface<iGeneralFactoryState> (ballFact[i]->GetMeshObjectFactory());

    float r = i * 0.1f + 0.3f;
    if (i == 5) r = 0.1f;
    csVector3 radius (r, r, r);
    csEllipsoid ellips (csVector3 (0.0f), radius);
    factoryState->GenerateSphere (ellips, 16);
  }
  
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  vfs->ChDir ("/lib/std");
  csRef<iMaterialWrapper> matR = CS::Material::MaterialBuilder::CreateColorMaterial
    (GetObjectRegistry (), ballMaterialNames[0], ballMaterialColors[0]);
  csRef<iMaterialWrapper> matG = CS::Material::MaterialBuilder::CreateColorMaterial
    (GetObjectRegistry (), ballMaterialNames[1], ballMaterialColors[1]);
  csRef<iMaterialWrapper> matB = CS::Material::MaterialBuilder::CreateColorMaterial
    (GetObjectRegistry (), ballMaterialNames[2], ballMaterialColors[2]);
  csRef<iMaterialWrapper> matY = CS::Material::MaterialBuilder::CreateColorMaterial
    (GetObjectRegistry (), ballMaterialNames[3], ballMaterialColors[3]);

  CreateColliders();
    
  dynamicsDebugger->SetDebugSector (room);

  engine->Prepare ();

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::CreateColliders()
{
  ReportInfo ("Creating colliders...");

  /*csPlane3 mainFloor (csVector3 (0.0f, 1.0f, 0.0f), 0.55f);
  dynamicSystem->AttachColliderPlane (mainFloor, 200.0f, 0.0f);
  
  csOrthoTransform t;
  // Walls
  csVector3 wallsSize (46.0f, 22.0f, 1.0f);

  t.SetOrigin(csVector3(0.0f, 10.0f, 9.9f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 100.0f, 0.0f);

  t.SetOrigin(csVector3(0.0f, 10.0f, -9.8f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 100.0f, 0.0f);

  wallsSize.Set (1.0f, 22.0f, 20.0f);

  t.SetOrigin(csVector3(22.0f, 10.0f, 0.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 100.0f, 0.0f);

  t.SetOrigin(csVector3(-21.1f, 10.0f, 0.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 100.0f, 0.0f);

  // Second level floor
  wallsSize.Set (46.0f, 0.25f, 6.0f);

  t.SetOrigin(csVector3(0.0f, 6.2f, 7.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 200.0f, 0.0f);

  t.SetOrigin(csVector3(0.0f, 6.2f, -7.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 200.0f, 0.0f);

  // Third level floor
  t.SetOrigin(csVector3(0.0f, 13.25f, 7.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 200.0f, 0.0f);

  t.SetOrigin(csVector3(0.0f, 13.25f, -7.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 200.0f, 0.0f);

  // Second level floor
  wallsSize.Set (7.0f, 0.25f, 20.0f);

  t.SetOrigin(csVector3(18.5f, 6.8f, 0.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 200.0f, 0.0f);

  t.SetOrigin(csVector3(-17.5f, 6.8f, 0.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 200.0f, 0.0f);

  // Third level floor
  t.SetOrigin(csVector3(18.5f, 13.85f, 0.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 200.0f, 0.0f);

  t.SetOrigin(csVector3(-17.5f, 13.85f, 0.0f));
  dynamicSystem->AttachColliderBox (wallsSize, t, 200.0f, 0.0f);*/

  /*CreateMeshBBoxCollider ("lucy");
  CreateMeshBBoxCollider ("happy_vrip");
  CreateMeshBBoxCollider ("xyzrgb_dragon");
  CreateMeshBBoxCollider ("bunny_Mesh");
  CreateMeshBBoxCollider ("dragon_vrip");
  //CreateMeshBBoxCollider ("krystal");

  CreateMeshColliders ("Cube.00", 6);
  CreateMeshColliders ("pillar", 5);
  CreateMeshColliders ("arcs", 8);
  CreateMeshColliders ("object", 11);
  CreateMeshColliders ("walls", 3);
  CreateMeshColliders ("floors", 1);
  CreateMeshColliders ("ceiling", 1);*/

  for (int i=0; i < engine->GetMeshes()->GetCount(); i++)
  {
    iMeshWrapper *mesh = engine->GetMeshes()->Get(i);
    const csReversibleTransform &fullTransform = mesh->GetMovable()->GetFullTransform();
    csOrthoTransform t (fullTransform.GetO2T(), fullTransform.GetOrigin());    
    dynamicSystem->AttachColliderMesh (mesh, t, 10.0f, 0.0f); 
  }
}

//----------------------------------------------------------------------
void DeferredDemo::CreateMeshBBoxCollider(const char *meshName)
{
  csOrthoTransform t;
  iMeshWrapper *mesh = engine->FindMeshObject (meshName); 
  if (mesh)
  {
    t.SetOrigin (mesh->GetWorldBoundingBox().GetCenter());
    dynamicSystem->AttachColliderBox (mesh->GetWorldBoundingBox().GetSize(), t, 10.0f, 0.0f);
  }
}

//----------------------------------------------------------------------
void DeferredDemo::CreateMeshColliders(const char *baseMeshName, int numMeshes)
{
  for (int i=0; i < numMeshes; i++)
  {
    csOrthoTransform t;
    csString meshName = csString (baseMeshName) + csString ((char)('0' + i));
    iMeshWrapper *mesh = engine->FindMeshObject (meshName);
    if (mesh)
    {
      t.SetOrigin (mesh->GetMovable()->GetFullPosition());
      dynamicSystem->AttachColliderMesh (mesh, t, 10.0f, 0.0f);
    }
  }
}

//----------------------------------------------------------------------
void DeferredDemo::Help()
{
  csRef<iConfigManager> cfg (csQueryRegistry<iConfigManager> (object_reg));

  csPrintf ("Options for DeferredDemo:\n");
  csPrintf ("  -nologo            do not draw logo.\n");
  csPrintf ("  -forward           use forward rendering on startup.\n");
  csPrintf ("  -world=<file>      use given world file instead of %s\n",
	    CS::Quote::Single ("world"));
  csPrintf ("  -worlddir=<path>   load map from VFS <path> (default %s)\n", 
    CS::Quote::Single (cfg->GetStr ("Walktest.Settings.WorldFile",
				    DEFAULT_CFG_WORLDDIR)));
}

//----------------------------------------------------------------------
bool DeferredDemo::Application()
{
  if (!OpenApplication (GetObjectRegistry()))
  {
    return ReportError("Error opening system!");
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (GetObjectRegistry()))
  {
    csCommandLineHelper::Help (GetObjectRegistry());
    return true;
  }

  if (SetupModules() && 
      LoadSettings() && 
      LoadAppData() &&
      LoadScene() &&
      SetupGui() &&
      SetupScene())
  {
    RunDemo();
  }

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::RunDemo()
{
  // Fetch the minimal elapsed ticks per second.
  csConfigAccess cfgacc (GetObjectRegistry(), "/config/system.cfg");
  csTicks min_elapsed = (csTicks)cfgacc->GetInt ("System.MinimumElapsedTicks", 0);

  while (!ShouldShutdown())
  {
    vc->Advance ();

    csTicks previous = csGetTicks ();

    eventQueue->Process ();

    // Limit fps.
    csTicks elapsed = csGetTicks () - previous;
    if (elapsed < min_elapsed)
      csSleep (min_elapsed - elapsed);
  }
}

//----------------------------------------------------------------------
void DeferredDemo::UpdateCamera(float deltaTime)
{
  const float MOVE_SPEED = 5.0f;
  const float ROTATE_SPEED = 2.0f;  

  // Handles camera movement.
  iCamera *c = view->GetCamera ();
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * MOVE_SPEED * deltaTime);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * MOVE_SPEED * deltaTime);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * MOVE_SPEED * deltaTime);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * MOVE_SPEED * deltaTime);
  }
  else
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      viewRotY += ROTATE_SPEED * deltaTime;
    if (kbd->GetKeyState (CSKEY_LEFT))
      viewRotY -= ROTATE_SPEED * deltaTime;
    if (kbd->GetKeyState (CSKEY_PGUP))
      viewRotX += ROTATE_SPEED * deltaTime;
    if (kbd->GetKeyState (CSKEY_PGDN))
      viewRotX -= ROTATE_SPEED * deltaTime;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * MOVE_SPEED * deltaTime);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * MOVE_SPEED * deltaTime);
  }

  csMatrix3 Rx = csXRotMatrix3 (viewRotX);
  csMatrix3 Ry = csYRotMatrix3 (viewRotY);
  csOrthoTransform V (Rx * Ry, c->GetTransform ().GetOrigin ());

  c->SetTransform (V);
}

//----------------------------------------------------------------------
void DeferredDemo::UpdateDynamics(float deltaTime)
{
  dynamicsDebugger->SetDebugSector (view->GetCamera()->GetSector());

  if (isBulletEnabled)
    dynamics->Step (deltaTime);
}

//----------------------------------------------------------------------
void DeferredDemo::UpdateGui()
{
  guiRoot->setVisible (cfgShowGui);
  cfgDrawLogo = guiDrawLogo->isSelected ();

  if (showGBuffer != guiShowGBuffer->isSelected ())
  {
    showGBuffer = !showGBuffer;
    rm_debug->DebugCommand ("toggle_visualize_gbuffer");
  }
  if (drawLightVolumes != guiDrawLightVolumes->isSelected ())
  {
    drawLightVolumes = !drawLightVolumes;
    rm_debug->DebugCommand ("toggle_visualize_lightvolumes");
  }
  if (cfgUseDeferredShading != guiDeferred->isSelected ())
  {
     cfgUseDeferredShading = guiDeferred->isSelected ();
  }
  if (enableGlobalIllum != guiEnableGlobalIllum->isSelected())
  {
    enableGlobalIllum = !enableGlobalIllum;
    rmGlobalIllum->EnableGlobalIllumination (enableGlobalIllum);
  }
  // By setting the AO wide radius to 0 it is disabled
  if (!guiEnableDetailSamples->isSelected())
  {
    detailSampleRadius = 0.0f;
  }

  rmGlobalIllum->EnableBlurPass (guiEnableBlur->isSelected());

  rmGlobalIllum->GetGlobalIllumVariableAdd ("enable ambient occlusion")->
      SetValue (guiEnableAO->isSelected() ? 1.0f : 0.0f);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("enable indirect light")->
      SetValue (guiEnableIndirectLight->isSelected() ? 1.0f : 0.0f);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("occlusion strength")->SetValue (occlusionStrength);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("sample radius")->SetValue (sampleRadius);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("detail sample radius")->SetValue (detailSampleRadius);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("num passes")->SetValue (aoPasses);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("max occluder distance")->SetValue (maxOccluderDistance);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("self occlusion")->SetValue (selfOcclusion);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("occluder angle bias")->SetValue (occAngleBias);
  rmGlobalIllum->GetGlobalIllumVariableAdd ("bounce strength")->SetValue (bounceStrength);  
  rmGlobalIllum->GetBlurVariableAdd ("ssao blur kernelsize")->SetValue (blurKernelSize);
  rmGlobalIllum->GetBlurVariableAdd ("ssao blur position threshold")->SetValue (blurPositionThreshold);
  rmGlobalIllum->GetBlurVariableAdd ("ssao blur normal threshold")->SetValue (blurNormalThreshold);

  char *msg = new char[50];
  cs_snprintf (msg, 50, "%s: %d", "Number of lights", room->GetLights()->GetCount());
  hudManager->GetStateDescriptions()->Empty();
  hudManager->GetStateDescriptions()->Push (msg);
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadLogo()
{
  logoTex = loader->LoadTexture (cfgLogoFile.GetDataSafe (), CS_TEXTURE_2D, NULL);

  if (!logoTex.IsValid ())
  {
    return ReportError("Could not load logo %s!",
		       CS::Quote::Single (cfgLogoFile.GetDataSafe ()));
  }

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::DrawLogo()
{
  if (!cfgDrawLogo || !logoTex.IsValid ())
    return;

  int w, h;
  logoTex->GetRendererDimensions (w, h);

  int screenW = graphics2D->GetWidth ();

  // Margin to the edge of the screen, as a fraction of screen width
  const float marginFraction = 0.01f;
  const int margin = (int)screenW * marginFraction;

  // Width of the logo, as a fraction of screen width
  const float widthFraction = 0.2f;
  const int width = (int)screenW * widthFraction;
  const int height = width * h / w;

  graphics3D->BeginDraw (CSDRAW_2DGRAPHICS);
  graphics3D->DrawPixmap (logoTex, 
                          screenW - width - margin, 
                          margin,
                          width,
                          height,
                          0,
                          0,
                          w,
                          h,
                          0);
}

//----------------------------------------------------------------------
void DeferredDemo::Frame ()
{
  float dt = (float)(vc->GetElapsedTicks () / 1000.0f);

  UpdateCamera (dt);
  UpdateDynamics (dt);
  UpdateGui ();

  if (cfgUseDeferredShading)
    engine->SetRenderManager (rm);
  else
    engine->SetRenderManager (rm_default);

  view->Draw ();

  if (doBulletDebug)
    bulletDynamicSystem->DebugDraw (view);

  cegui->Render ();

  DrawLogo ();

  graphics3D->FinishDraw ();
  graphics3D->Print (NULL);
}

//----------------------------------------------------------------------
bool DeferredDemo::OnUnhandledEvent (iEvent &event)
{
  if (event.Name == quitEventID)
  {
    return OnQuit (event);
  }
  else if (event.Name == cmdLineHelpEventID)
  {
    Help ();
    return true;
  }

  return false;
}

//----------------------------------------------------------------------
bool DeferredDemo::OnKeyboard(iEvent &event)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&event);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&event);
    if (code == CSKEY_ESC)
    {
      if (eventQueue.IsValid ()) 
      {
        eventQueue->GetEventOutlet ()->Broadcast( csevQuit(GetObjectRegistry()) );
        return true;
      }
    }        
    else if (code == CSKEY_F12) // Screenshot key
    {      
      return TakeScreenShot();
    }
    else if (code == CSKEY_F9)
    {
      hudManager->SetEnabled (!hudManager->GetEnabled());
      return true;
    }
    else if (code == CSKEY_F2)
    {
      graphics2D->SetFullScreen (!graphics2D->GetFullScreen());
      return true;
    }
    else if (code == 'f')
    {
      cfgUseDeferredShading = false;
      guiForward->setSelected (true);
      return true;
    }
    else if (code == 'd')
    {
      cfgUseDeferredShading = true;
      guiDeferred->setSelected (true);
      return true;
    }
    else if (code == 'g')
    {
      cfgShowGui = !cfgShowGui;
      return true;
    }
    else if (code == 'a')
    {
      showAmbientOcclusion = !showAmbientOcclusion;
      showGlobalIllumination = false;
      if (showAmbientOcclusion)
      {
        rm_debug->DebugCommand ("toggle_visualize_ambient_occlusion");
      }  
      return true;
    }
    else if (code == 's')
    {
      showGlobalIllumination = !showGlobalIllumination;
      showAmbientOcclusion = false;
      if (showGlobalIllumination)
      {
        rm_debug->DebugCommand ("toggle_visualize_global_illumination");
      }
      return true;
    }
    else if (code == 'e')
    {
      enableGlobalIllum = !enableGlobalIllum;
      guiEnableGlobalIllum->setSelected (enableGlobalIllum);
      rmGlobalIllum->EnableGlobalIllumination (enableGlobalIllum);
      return true;
    }
    else if (code == 'n')
    {
      isBulletEnabled = !isBulletEnabled;
    }
    else if (code == 'b')
    {
      doBulletDebug = !doBulletDebug;
      return true;
    }
    else if (code == '1')
    {
      rmGlobalIllum->ChangeBufferResolution ("full");
    }
    else if (code == '2')
    {
      rmGlobalIllum->ChangeBufferResolution ("half");
    }
    else if (code == '3')
    {
      rmGlobalIllum->ChangeBufferResolution ("quarter");
    }
    else if (code == CSKEY_SPACE)
    {
      if (kbd->GetKeyState (CSKEY_CTRL))
        SpawnSphere(true);
      else
        SpawnSphere();
      
      return true;
    } 
#ifdef CS_DEBUG
    else if (code == 'r')
    {
      SetupGui (true);
      return true;
    }
#endif
  }

  return false;
}

//----------------------------------------------------------------------
bool DeferredDemo::TakeScreenShot()
{
  csRef<iImage> screenshot = graphics2D->ScreenShot();

  // Convert the screenshot to the target image format
  csRef<iImageIO> imageIO = csQueryRegistry<iImageIO> (GetObjectRegistry ());
  if (!screenshot || !imageIO)
	  return false;

  csRef<iDataBuffer> data =
	  imageIO->Save (screenshot, csString().Format ("image/%s", screenshotFormat.GetData()));

  if (!data)
  {
	  ReportError ("Could not export screenshot image to format %s!",
		  CS::Quote::Single (screenshotFormat.GetData ()));
	  return false;
  }

  // Save the file
  csRef<iVFS> vfs = csQueryRegistry<iVFS> (GetObjectRegistry());
  if (!vfs) return false;

  csString filename = screenshotHelper.FindNextFilename (vfs);
  if (data && vfs->WriteFile (filename, data->GetData (), data->GetSize()))
  {
	  csRef<iDataBuffer> path = vfs->GetRealPath (filename.GetData ());
	  ReportInfo ("Screenshot saved to %s...", CS::Quote::Single (path->GetData()));
  }

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::SpawnSphere(bool attachLight)
{  
  const csOrthoTransform& cameraTransform = view->GetCamera()->GetTransform();  
  int ballIndex = rand() % 5;    
  csRef<iRigidBody> body = dynamicSystem->CreateBody();
  float radius = ballIndex * 0.1f + 0.3f;
  iSector *currentSector = view->GetCamera()->GetSector();

  int materialIndex = rand() % 4;

  if (attachLight)
  {
    light = engine->CreateLight ("light", body->GetPosition(), 8.0f, ballMaterialColors[materialIndex], 
        CS_LIGHT_DYNAMICTYPE_DYNAMIC);
    //light->SetType (CS_LIGHT_SPOTLIGHT);
    //light->SetSpotLightFalloff (25.0f, 30.0f);
    currentSector->GetLights()->Add (light);
    body->AttachLight (light);
    radius = 0.15f;
    ballIndex = 5;
  }

  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (ballFact[ballIndex], "ball", currentSector));
  iMaterialWrapper* mat = engine->GetMaterialList()->FindByName (ballMaterialNames[materialIndex]);    
  mesh->GetMeshObject()->SetMaterialWrapper (mat);

  body->SetProperties (0.01f, csVector3 (0.0f), csMatrix3());
  body->SetPosition (cameraTransform.GetOrigin() + cameraTransform.GetT2O() * csVector3 (0, 0, 1));
  body->AttachMesh (mesh);    
  body->AttachColliderSphere (radius, csVector3(0.0f), 1, 1, 0.01f);    
  body->SetLinearVelocity (cameraTransform.GetT2O() * csVector3 (0, 0, 7));
  body->SetAngularVelocity (cameraTransform.GetT2O() * csVector3 (7, 0, 0));  
    
  dynamicsDebugger->UpdateDisplay();
}

//----------------------------------------------------------------------
bool DeferredDemo::OnQuit (iEvent &event)
{
  shouldShutdown = true;
  return true;
}

