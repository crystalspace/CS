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
#include "cssysdef.h"

#include "csgeom/plane3.h"
#include "csgeom/sphere.h"
#include "cstool/materialbuilder.h"
#include "csutil/cfgnotifier.h"
#include "deferreddemo.h"
#include "iengine/campos.h"

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include <CEGUIWindowManager.h>
#include <CEGUILogger.h>
#include "csutil/custom_new_enable.h"

const char *DEFAULT_CFG_WORLDDIR = "/data/sponza";  // "/lev/castle";

const char* ballMaterialNames[4] = { "red", "green", "blue", "yellow" };
const csColor ballMaterialColors[4] = { csColor (1.0f, 0.0f, 0.0f),
					csColor (0.0f, 1.0f, 0.0f),
					csColor (0.0f, 0.0f, 1.0f),
					csColor (1.0f, 1.0f, 0.0f) };

//----------------------------------------------------------------------

DeferredDemo::DeferredDemo()
  : DemoApplication ("CrystalSpace.DeferredDemo")
{
  // Sets default cfg values.
  cfgWorldDir = DEFAULT_CFG_WORLDDIR;
  cfgWorldFile = "world";
  
  cfgUseDeferredShading = true;

  downsampleNormalsDepth = false;

  cfgShowHUD = false;
}

//----------------------------------------------------------------------
DeferredDemo::~DeferredDemo()
{}

//----------------------------------------------------------------------
void DeferredDemo::PrintHelp ()
{
  csCommandLineHelper commandLineHelper;

  commandLineHelper.AddCommandLineOption
    ("forward", "Use forward rendering on startup", csVariant ());
  commandLineHelper.AddCommandLineOption
    ("world", "Use given world file", csVariant ("world"));
  commandLineHelper.AddCommandLineOption
    ("worlddir", "Use given VFS path", csVariant ("/data/sponza"));

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (GetObjectRegistry (), "deferreddemo", "deferreddemo", "Crystal Space's deferred renderer demo.");
}

//----------------------------------------------------------------------
bool DeferredDemo::OnInitialize(int argc, char *argv[])
{
  if (!csInitializer::RequestPlugins (GetObjectRegistry(),
      CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
      CS_REQUEST_PLUGIN ("crystalspace.dynamics.debug", CS::Debug::iDynamicsDebuggerManager),
      CS_REQUEST_END))
  {
    return ReportError("Failed to initialize plugins!");
  }

  // Default behavior from DemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::Application()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  if (!(SetupModules() && 
	LoadSettings() && 
	LoadScene() &&
	SetupGui() &&
	SetupScene()))
    return false;

   // Run the application
   Run();

   return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupModules()
{
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
  
  /* NOTE: Config settings for render managers are stored in 'engine.cfg' 
   * and are needed when loading a render manager. Normally these settings 
   * are added by the engine when it loads a render manager. However, since
   * we are loading the deferred render manager manually we must also manually
   * add the proper config file. */
  csRef<iConfigManager> cfg = csQueryRegistry<iConfigManager> (GetObjectRegistry());
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

  globalIllumResolution = config->GetStr ("RenderManager.Deferred.GlobalIllum.BufferResolution", "full");
  depthNormalsResolution = config->GetStr ("RenderManager.Deferred.GlobalIllum.DepthAndNormalsResolution",
    "full");

  SetupDynamicsSystem ();

  return true;
}

bool DeferredDemo::SetupDynamicsSystem()
{
  csRef<iPluginManager> pluginManager = csQueryRegistry<iPluginManager> (GetObjectRegistry ());

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
  if (!vfs->ChDir (cfgWorldDir))
    return ReportError("Could not navigate to level directory %s!",
		       CS::Quote::Single (cfgWorldDir.GetDataSafe ()));

  if (!loader->LoadMapFile (cfgWorldFile))
     return ReportError("Could not load level!");

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::LoadSettings()
{
  const char *val = NULL;

  csRef<iCommandLineParser> cmdline = csQueryRegistry<iCommandLineParser> (GetObjectRegistry());

  val = cmdline->GetOption ("worlddir");
  if (val)
    cfgWorldDir = val;
  else
    cfgWorldDir = config->GetStr ("Deferreddemo.Settings.WorldDirectory", DEFAULT_CFG_WORLDDIR);

  cfgWorldFile = config->GetStr ("Deferreddemo.Settings.WorldFile", "world");

  if (cmdline->GetOption ("nologo"))
    cfgDrawLogo = false;
  else
    cfgDrawLogo = config->GetBool ("Deferreddemo.Settings.DrawLogo", true);

  if (cmdline->GetOption ("forward"))
    cfgUseDeferredShading = false;
  else
    cfgUseDeferredShading = !config->GetBool ("Deferreddemo.Settings.UseForward", false);

  cfgShowGui = config->GetBool ("Deferreddemo.Settings.ShowGui", true);

  return true;
}

//----------------------------------------------------------------------
bool DeferredDemo::SetupGui(bool reload)
{
  // Initialize the HUD manager
  hudManager->GetKeyDescriptions()->Empty();

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
  guiEnableAO         = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableAO"));
  guiEnableBlur       = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableBlur"));
  guiEnableDetailSamples = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableDetailSamples"));
  guiEnableGlobalIllum   = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableGlobalIllum"));
  guiEnableIndirectLight = static_cast<CEGUI::Checkbox*>(winMgr->getWindow ("EnableIndirectLight"));
  //CEGUI::Combobox *guiCombo = static_cast<CEGUI::Combobox*>(winMgr->getWindow ("RenderBuffer"));  
  //guiCombo->addItem (new CEGUI::ListboxTextItem ("Ambient Occlusion"));

  if (!guiRoot || 
      !guiDeferred || !guiForward || !guiEnableGlobalIllum ||
      !guiShowGBuffer || !guiDrawLightVolumes ||
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
  // Find a strating sector
  iCameraPositionList* positions = engine->GetCameraPositions ();
  if (positions->GetCount ())
    room = engine->FindSector (positions->Get (0)->GetSector ());

  else room = engine->GetSectors ()->Get (0);

  if (!room)
    return ReportError ("Could not find a valid starting sector");

  // Setup the camera
  view->GetCamera ()->SetSector (room);
  cameraManager->SetCamera (view->GetCamera ());
  cameraManager->SetCameraMode (CS::Utility::CAMERA_MOVE_FREE);

  // TODO: explain this
  csPlane3 *farPlane = new csPlane3 (0, 0, -1, 100);
  view->GetCamera()->SetFarPlane (farPlane);
  view->GetPerspectiveCamera()->SetNearClipDistance (0.2f);
  delete farPlane;

  // Checks for support of at least 4 color buffer attachment points.
  const csGraphics3DCaps *caps = g3d->GetCaps();
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

    float r = i * 0.1f + 0.4f;
    if (i == 5) r = 0.1f;
    csVector3 radius (r, r, r);
    csEllipsoid ellips (csVector3 (0.0f), radius);
    factoryState->GenerateSphere (ellips, 16);
  }
  
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

  printf ("Precaching data...\n");
  engine->Prepare ();
  printf ("Ready!\n");

  return true;
}

//----------------------------------------------------------------------
void DeferredDemo::CreateColliders()
{
  ReportInfo ("Creating colliders...");  

  for (int i=0; i < engine->GetMeshes()->GetCount(); i++)
  {
    iMeshWrapper *mesh = engine->GetMeshes()->Get(i);
    const csReversibleTransform &fullTransform = mesh->GetMovable()->GetFullTransform();
    csOrthoTransform t (fullTransform.GetO2T(), fullTransform.GetOrigin());    
    dynamicSystem->AttachColliderMesh (mesh, t, 10.0f, 0.0f); 
  }
}

//----------------------------------------------------------------------
void DeferredDemo::UpdateDynamics(float deltaTime)
{
  if (isBulletEnabled)
    dynamics->Step (deltaTime);
}

//----------------------------------------------------------------------
void DeferredDemo::UpdateGui()
{
  guiRoot->setVisible (cfgShowGui);

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

  hudManager->GetStateDescriptions()->Empty();
  csString msg ("Number of lights: ");
  msg.Append (room->GetLights()->GetCount());  
  hudManager->GetStateDescriptions()->Push (msg);

  msg = csString ("SSGI resolution: ");
  msg.Append (globalIllumResolution);
  hudManager->GetStateDescriptions()->Push (msg);

  msg = csString ("Depth/Normals resolution: ");
  msg.Append (depthNormalsResolution);
  hudManager->GetStateDescriptions()->Push (msg);
}

//----------------------------------------------------------------------
void DeferredDemo::Frame ()
{
  float dt = (float)(vc->GetElapsedTicks () / 1000.0f);

  UpdateDynamics (dt);
  UpdateGui ();

  if (cfgUseDeferredShading)
    engine->SetRenderManager (rm);
  else
    engine->SetRenderManager (rm_default);

  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  if (doBulletDebug)
    bulletDynamicSystem->DebugDraw (view);

  cegui->Render ();

  // TODO: enable/disable logo
}

//----------------------------------------------------------------------
bool DeferredDemo::OnKeyboard(iEvent &event)
{
  // Default behavior from DemoApplication
  DemoApplication::OnKeyboard (event);

  csKeyEventType eventtype = csKeyEventHelper::GetEventType (&event);
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&event);
    if (code == CSKEY_F8)
    {
      cfgShowHUD = !cfgShowHUD;

      if (cfgShowHUD)
      {
	hudManager->GetKeyDescriptions()->Empty();
	hudManager->GetKeyDescriptions()->Push ("z: Change SSGI resolution");
	hudManager->GetKeyDescriptions()->Push ("x: Change depth/normals resolution");
	hudManager->GetKeyDescriptions()->Push ("Space: Throw ball");
	hudManager->GetKeyDescriptions()->Push ("Alt+Space: Throw ball w/light");
	hudManager->GetKeyDescriptions()->Push ("n: Pause/Resume physics simulation");
	hudManager->GetKeyDescriptions()->Push ("g: Show/Hide GUI");
  hudManager->GetKeyDescriptions()->Push ("F8: Show/Hide Key Descriptions");
	hudManager->GetKeyDescriptions()->Push ("F9: Show/Hide HUD");
	hudManager->GetKeyDescriptions()->Push ("F12: Screenshot");  
	hudManager->GetKeyDescriptions()->Push ("1-9: Visualize deferred buffers");
	hudManager->GetKeyDescriptions()->Push ("0: Visualize final rendered image");
      }

      else
	hudManager->GetKeyDescriptions()->Empty();

      return true;
    }
    else if (code == CSKEY_F2)
    {
      g2d->SetFullScreen (!g2d->GetFullScreen());
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
    else if (code == '8')
    {
      //showAmbientOcclusion = !showAmbientOcclusion;
      //showGlobalIllumination = false;
      //if (showAmbientOcclusion)
      {
        rm_debug->DebugCommand ("toggle_visualize_ambient_occlusion");
      }
      return true;
    }
    else if (code == '9')
    {
      //showGlobalIllumination = !showGlobalIllumination;
      //showAmbientOcclusion = false;
      //if (showGlobalIllumination)
      {
        rm_debug->DebugCommand ("toggle_visualize_color_bleeding");
      }
      return true;
    }
    else if (code == '0')
    {
      rm_debug->DebugCommand ("toggle_visualize_backbuffer");
      return true;
    } 
    else if (code == '1')
    {
      rm_debug->DebugCommand ("toggle_visualize_diffusebuffer");
      return true;
    }
    else if (code == '2')
    {
      rm_debug->DebugCommand ("toggle_visualize_normalbuffer");
      return true;
    }
    else if (code == '3')
    {
      rm_debug->DebugCommand ("toggle_visualize_ambientbuffer");
      return true;
    } 
    else if (code == '4')
    {
      rm_debug->DebugCommand ("toggle_visualize_depthbuffer");
      return true;
    }
    else if (code == '5')
    {
      rm_debug->DebugCommand ("toggle_visualize_specularbuffer");
      return true;
    }
    else if (code == '6')
    {
      rm_debug->DebugCommand ("toggle_visualize_vertexnormalsbuffer");
      return true;
    }
    else if (code == '7')
    {
      rm_debug->DebugCommand ("toggle_visualize_lineardepthbuffer");
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
    else if (code == 'p')
    {
      doBulletDebug = !doBulletDebug;
      return true;
    }
    else if (code == 'z')
    {
      if (globalIllumResolution.CompareNoCase ("full"))
        globalIllumResolution = "half";
      else if (globalIllumResolution.CompareNoCase ("half"))
        globalIllumResolution = "quarter";
      else if (globalIllumResolution.CompareNoCase ("quarter"))
        globalIllumResolution = "full";

      rmGlobalIllum->ChangeBufferResolution (globalIllumResolution.GetDataSafe());
      return true;
    }
    else if (code == 'x')
    {
      if (depthNormalsResolution.CompareNoCase ("full"))
        depthNormalsResolution = "half";
      else if (depthNormalsResolution.CompareNoCase ("half"))
        depthNormalsResolution = "quarter";
      else if (depthNormalsResolution.CompareNoCase ("quarter"))
        depthNormalsResolution = "full";

      rmGlobalIllum->ChangeNormalsAndDepthResolution (depthNormalsResolution.GetDataSafe());
      return true;
    }
    else if (code == 'l')
    {
      SpawnSphere (true);      
      return true;
    } 
    else if (code == CSKEY_SPACE)
    {      
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
bool DeferredDemo::OnMouseDown (iEvent &event)
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

//----------------------------------------------------------------------
bool DeferredDemo::OnMouseUp (iEvent &event)
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

//----------------------------------------------------------------------
void DeferredDemo::SpawnSphere(bool attachLight)
{  
  const csOrthoTransform& cameraTransform = view->GetCamera()->GetTransform();  
  int ballIndex = rand() % 5;    
  csRef<iRigidBody> body = dynamicSystem->CreateBody();
  float radius = ballIndex * 0.1f + 0.4f;
  iSector *currentSector = view->GetCamera()->GetSector();

  int materialIndex = rand() % 4;

  if (attachLight)
  {    
    light = engine->CreateLight ("light", body->GetPosition(), 5.0f, ballMaterialColors[materialIndex], 
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

  body->AttachMesh (mesh);
  body->SetProperties (0.1f, csVector3 (0.0f), csMatrix3());
  body->SetPosition (cameraTransform.GetOrigin() + cameraTransform.GetT2O() * csVector3 (0, 0, 1));
  body->AttachColliderSphere (radius, csVector3(0.0f), 1, 1, 0.01f);    
  body->SetLinearVelocity (cameraTransform.GetT2O() * csVector3 (0, 0, 7));
  body->SetAngularVelocity (cameraTransform.GetT2O() * csVector3 (7, 0, 0));  
    
  dynamicsDebugger->UpdateDisplay();
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return DeferredDemo ().Main (argc, argv);
}
