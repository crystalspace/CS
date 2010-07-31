/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

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

#include "csutil/custom_new_disable.h"
#include <CEGUI.h>
#include <CEGUIWindowManager.h>
#include <CEGUILogger.h>
#include "csutil/custom_new_enable.h"

#include "hairtest.h"
#include "krystal.h"

HairTest::HairTest ()
: csDemoApplication ("CrystalSpace.HairTest", "hairtest",
                     "hairtest <OPTIONS>",
                     "Tests on the animation of objects iAnimatedMesh."),
                     avatarScene (0), dynamicsDebugMode (DYNDEBUG_NONE)
{
  // We manage the camera by ourselves
  SetCameraMode (CSDEMO_CAMERA_NONE);
  SetGUIDisplayed(false);
}

HairTest::~HairTest ()
{
  delete avatarScene;
}

void HairTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsedTime = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  const float speed = elapsedTime / 1000.0f;

  // Rotate by angle
  const float angle = 4 * speed;

  // Compute camera and animesh position
  iCamera* camera = view->GetCamera ();
  csVector3 cameraPosition = camera->GetTransform ().GetOrigin ();
  csVector3 cameraTarget = avatarScene->GetCameraTarget ();
  float minimumDistance = avatarScene->GetCameraMinimumDistance ();

  float radius = sqrt ( ( cameraTarget.x - cameraPosition.x ) * ( cameraTarget.x - cameraPosition.x ) + 
    ( cameraTarget.y - cameraPosition.y ) * ( cameraTarget.y - cameraPosition.y ) + 
    ( cameraTarget.z - cameraPosition.z ) * ( cameraTarget.z - cameraPosition.z ));
  float lateral = sin(angle) * radius;
  float straight = cos(angle) * radius;

  // Move camera
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the Up/Down arrow keys will cause
    // the camera to go forwards and backwards (forward only allowed if camera 
    // not too close). Left/Right arrows work also when shift is hold.
    if (kbd->GetKeyState (CSKEY_UP)
      && (cameraPosition - cameraTarget).Norm () > minimumDistance)
      camera->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      camera->Move (CS_VEC_BACKWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_RIGHT))
      camera->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      camera->Move (CS_VEC_LEFT * 4 * speed);
  }
  else
  {
    // Left and right arrows cause the camera to strafe on the X axis; up and 
    // down arrows cause the camera to strafe on the Y axis
    if (kbd->GetKeyState (CSKEY_RIGHT))
    {
      camera ->GetTransform().SetOrigin(csVector3(cameraTarget));
      camera ->Move(CS_VEC_BACKWARD * straight);
      camera ->Move(CS_VEC_RIGHT * lateral);
    }
    if (kbd->GetKeyState (CSKEY_LEFT))
    {
      camera ->GetTransform().SetOrigin(csVector3(cameraTarget));
      camera ->Move(CS_VEC_BACKWARD * straight);
      camera ->Move(CS_VEC_LEFT * lateral);
    }
    // Avoid gimbal lock of camera
    cameraPosition.Normalize ();
    float cameraDot = cameraPosition * csVector3 (0.0f, 1.0f, 0.0f);
    if (kbd->GetKeyState (CSKEY_UP)
      && cameraDot < 0.98f)
      camera->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN)
      && cameraDot > -0.98f)
      camera->Move (CS_VEC_DOWN * 4 * speed);
  }

  // Make the camera look at the animesh
  camera->GetTransform ().LookAt (cameraTarget - camera->GetTransform ().GetOrigin (),
    csVector3 (0.0f, 1.0f, 0.0f) );

  // Step the dynamic simulation (we slow down artificially the simulation in
  // order to achieve a 'slow motion' effect)
  if (physicsEnabled)
    dynamics->Step (speed * avatarScene->GetSimulationSpeed ());

  // Update the information on the current state of the application
  avatarScene->UpdateStateDescription ();

  // Default behavior from csDemoApplication
  csDemoApplication::Frame ();

  // Display the Bullet debug information
  if (avatarScene->HasPhysicalObjects ()
    && dynamicsDebugMode == DYNDEBUG_BULLET)
    bulletDynamicSystem->DebugDraw (view);
  /*
  for (size_t i = 0; i < bulletDynamicSystem->GetSoftBodyCount (); i++)
  {
  iBulletSoftBody* softBody = bulletDynamicSystem->GetSoftBody (i);
  // Ropes are characterized by the fact that they have no triangle
  if (!softBody->GetTriangleCount ())
  softBody->DebugDraw (view);
  }
  */
  cegui->Render ();
}

bool HairTest::OnExitButtonClicked (const CEGUI::EventArgs&)
{
  csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (GetObjectRegistry());
  if (q.IsValid()) q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
  return true;
}

bool HairTest::OnCollidersButtonClicked (const CEGUI::EventArgs&)
{
  SwitchDynamics();
  return true;
}

// Surface properties
bool HairTest::OnEventThumbTrackEndedShiftR (const CEGUI::EventArgs&)
{
  CS::ShaderVarName aR (svStrings, "aR");	
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(aR)->SetValue(-5  * ( 1 + sliderShiftR->getScrollPosition() ) );
  avatarScene->furMaterial->GetFurStrandGenerator()->Invalidate();

  return true;
}

bool HairTest::OnEventThumbTrackEndedWidthR (const CEGUI::EventArgs&)
{
  CS::ShaderVarName bR (svStrings, "bR");	
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(bR)->SetValue(5  * ( 1 + sliderWidthR->getScrollPosition() ) );
  avatarScene->furMaterial->GetFurStrandGenerator()->Invalidate();

  return true;
}

// Fiber properties
bool HairTest::OnEventThumbTrackEndedAbsorption (const CEGUI::EventArgs&)
{
  CS::ShaderVarName absorption (svStrings, "absorption");	
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(absorption)->SetValue(0.2f + 10 * sliderAbsorption->getScrollPosition() );
  avatarScene->furMaterial->GetFurStrandGenerator()->Invalidate();

  return true;
}

bool HairTest::OnEventThumbTrackEndedEccentricity (const CEGUI::EventArgs&)
{
  CS::ShaderVarName eccentricity (svStrings, "eccentricity");	
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(eccentricity)->SetValue(0.85f + 0.15f * sliderEccentricity->getScrollPosition() );
  avatarScene->furMaterial->GetFurStrandGenerator()->Invalidate();

  return true;
}

// Glints
bool HairTest::OnEventThumbTrackEndedGlintScale (const CEGUI::EventArgs&)
{
  CS::ShaderVarName kG (svStrings, "kG");	
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(kG)->SetValue(0.5f + 4.5f * sliderGlintScale->getScrollPosition() );
  avatarScene->furMaterial->GetFurStrandGenerator()->Invalidate();

  return true;
}

bool HairTest::OnEventThumbTrackEndedCausticWidth (const CEGUI::EventArgs&)
{
  CS::ShaderVarName wc (svStrings, "wc");	
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(wc)->SetValue(10 + 15 * sliderCausticWidth->getScrollPosition() );
  avatarScene->furMaterial->GetFurStrandGenerator()->Invalidate();

  return true;
}

bool HairTest::OnEventThumbTrackEndedCausticMerge (const CEGUI::EventArgs&)
{
  CS::ShaderVarName Dh0 (svStrings, "Dh0");	
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(Dh0)->SetValue(0.2f + 0.2f * sliderCausticMerge->getScrollPosition() );
  avatarScene->furMaterial->GetFurStrandGenerator()->Invalidate();

  return true;
}

// RGB hair color
bool HairTest::OnEventThumbTrackEndedR (const CEGUI::EventArgs&)
{
  CS::ShaderVarName objColor (svStrings, "hair color");	
  
  csVector3 color; 
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(objColor)->GetValue(color);
  
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(objColor)->SetValue(csVector3( 
    sliderR->getScrollPosition(), color.y, color.z ) );

  return true;
}

bool HairTest::OnEventThumbTrackEndedG (const CEGUI::EventArgs&)
{
  CS::ShaderVarName objColor (svStrings, "hair color");	

  csVector3 color; 
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(objColor)->GetValue(color);

  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(objColor)->SetValue(csVector3( 
    color.x, sliderG->getScrollPosition(), color.z ) );

  return true;
}

bool HairTest::OnEventThumbTrackEndedB (const CEGUI::EventArgs&)
{
  CS::ShaderVarName objColor (svStrings, "hair color");	

  csVector3 color; 
  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(objColor)->GetValue(color);

  avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
    ->GetVariableAdd(objColor)->SetValue(csVector3( 
    color.x, color.y , sliderB->getScrollPosition()) );

  return true;
}

bool HairTest::OnEventThumbTrackEndedGuideLOD (const CEGUI::EventArgs&)
{
  avatarScene->furMaterial->SetGuideLOD(sliderGuideLOD->getScrollPosition());

  return true;
}

bool HairTest::OnEventThumbTrackEndedStrandLOD (const CEGUI::EventArgs&)
{
  avatarScene->furMaterial->SetStrandLOD(sliderStrandLOD->getScrollPosition());

  return true;
}

bool HairTest::OnEventThumbTrackEndedOverallLOD (const CEGUI::EventArgs&)
{
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

bool HairTest::OnKeyboard (iEvent &ev)
{
  // Default behavior from csDemoApplication
  csDemoApplication::OnKeyboard (ev);

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
  }

  return false;
}

bool HairTest::OnInitialize (int argc, char* argv[])
{
  // Default behavior from csDemoApplication
  if (!csDemoApplication::OnInitialize (argc, argv))
    return false;

  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.controllers.lookat",
    iSkeletonLookAtManager2),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.controllers.basic",
    iSkeletonBasicNodesManager2),
    CS_REQUEST_PLUGIN("crystalspace.material.furmaterial", iFurMaterialType),
    CS_REQUEST_PLUGIN("crystalspace.physics.furphysics", iFurPhysicsControl),
    CS_REQUEST_PLUGIN("crystalspace.material.furstrandmaterial", iFurStrandGenerator),
    CS_REQUEST_PLUGIN ("crystalspace.cegui.wrapper", iCEGUI),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  // Shader variables
  svStrings = csQueryRegistryTagInterface<iShaderVarStringSet> (
    object_reg, "crystalspace.shader.variablenameset");

  if (!svStrings) 
  {
    printf ("No SV names string set!\n");
    return false;
  }

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
    debuggerManager = csLoadPlugin<iDynamicsDebuggerManager>
      (plugmgr, "crystalspace.dynamics.debug");

    if (!debuggerManager)
    {
      ReportWarning
        ("Can't load Dynamics Debugger plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    // Load the ragdoll plugin
    ragdollManager = csLoadPlugin<iSkeletonRagdollManager2>
      (plugmgr, "crystalspace.mesh.animesh.controllers.ragdoll");

    if (!ragdollManager)
    {
      ReportWarning
        ("Can't load ragdoll plugin, continuing with reduced functionalities");
      physicsEnabled = false;
      break;
    }

    break;
  }

  return true;
}

bool HairTest::Application ()
{
  // Default behavior from csDemoApplication
  if (!csDemoApplication::Application ())
    return false;

  // Find references to the plugins of the animation nodes
  lookAtManager = csQueryRegistry<iSkeletonLookAtManager2> (GetObjectRegistry ());
  if (!lookAtManager) return ReportError("Failed to locate iLookAtManager plugin!");

  basicNodesManager =
    csQueryRegistry<iSkeletonBasicNodesManager2> (GetObjectRegistry ());
  if (!basicNodesManager)
    return ReportError("Failed to locate iSkeletonBasicNodesManager2 plugin!");

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

  cegui->GetFontManagerPtr ()->createFreeTypeFont("DejaVuSans", 10, true, "/fonts/ttf/DejaVuSans.ttf");

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  // Load layout and set as root
  vfs->ChDir ("/hairtest/");
  cegui->GetSystemPtr ()->setGUISheet(winMgr->loadWindowLayout("ice.layout"));

  // Subscribe to the clicked event for the exit button
  CEGUI::Window* btn = winMgr->getWindow("HairTest/MainWindow/Tab/Page1/Quit");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::OnExitButtonClicked, this));

  winMgr->getWindow("HairTest/MainWindow/Tab/Page1/Colliders") 
    -> subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&HairTest::OnCollidersButtonClicked, this));

  // Default behavior from csDemoApplication for the creation of the scene
  if (!csDemoApplication::CreateRoom ())
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
        scfQueryInterface<iBulletDynamicSystem> (dynamicSystem);

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

  // Create avatar
  avatarScene = new KrystalScene (this);

  if (!avatarScene->CreateAvatar ())
    return false;

  // Set default color sliders
  CS::ShaderVarName objColor (svStrings, "hair color");	

  csVector3 color; 
  if (avatarScene->furMaterial->GetFurStrandGenerator() && 
      avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial())
  {

    // Initialized GUI for Marschner

    // Surface properties
    sliderShiftR = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page2/Slider1");

    sliderShiftR->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedShiftR, this));

    sliderWidthR = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page2/Slider2");

    sliderWidthR->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedWidthR, this));

    // Fiber properties
    sliderAbsorption = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page2/Slider3");

    sliderAbsorption->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedAbsorption, this));

    sliderEccentricity = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page2/Slider4");

    sliderEccentricity->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedEccentricity, this));  

    // Glints
    sliderGlintScale = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page2/Slider5");

    sliderGlintScale->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedGlintScale, this));  

    sliderCausticWidth = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page2/Slider6");

    sliderCausticWidth->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedCausticWidth, this));  

    sliderCausticMerge = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page2/Slider7");

    sliderCausticMerge->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedCausticMerge, this));  

    // RGB hair color
    sliderR = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page1/Slider1");

    sliderR->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedR, this));  

    sliderG = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page1/Slider2");

    sliderG->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedG, this));  

    sliderB = (CEGUI::Scrollbar*)winMgr->
      getWindow("HairTest/MainWindow/Tab/Page1/Slider3");

    sliderB->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
      CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedB, this));  

    avatarScene->furMaterial->GetFurStrandGenerator()->GetMaterial()
      ->GetVariableAdd(objColor)->GetValue(color);
    sliderR->setScrollPosition(color.x);
    sliderG->setScrollPosition(color.y);
    sliderB->setScrollPosition(color.z);
  }

  sliderGuideLOD = (CEGUI::Scrollbar*)winMgr->
    getWindow("HairTest/MainWindow/Tab/Page3/Slider1");

  sliderGuideLOD->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
    CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedGuideLOD, this));  

  sliderStrandLOD = (CEGUI::Scrollbar*)winMgr->
    getWindow("HairTest/MainWindow/Tab/Page3/Slider2");

  sliderStrandLOD->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
    CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedStrandLOD, this));  

  sliderStrandLOD->setScrollPosition(1.0f);

  sliderOverallLOD = (CEGUI::Scrollbar*)winMgr->
    getWindow("HairTest/MainWindow/Tab/Page3/Slider3");

  sliderOverallLOD->subscribeEvent(CEGUI::Scrollbar::EventThumbTrackEnded,
    CEGUI::Event::Subscriber(&HairTest::OnEventThumbTrackEndedOverallLOD, this)); 

  // Initialize camera position
  view->GetCamera ()->GetTransform ().SetOrigin (avatarScene->GetCameraStart ());

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
