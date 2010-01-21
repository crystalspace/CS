/*
  Copyright (C) 2009-10 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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

#include "avatartest.h"

#define LOOKAT_CAMERA 1
#define LOOKAT_POSITION 2
#define LOOKAT_NOTHING 3

#define ROTATION_SLOW 1
#define ROTATION_NORMAL 2
#define ROTATION_IMMEDIATE 3

CS_IMPLEMENT_APPLICATION

AvatarTest::AvatarTest ()
  : targetReached (false), lookAtListener (this)
{
  SetApplicationName ("CrystalSpace.AvatarTest");
}

AvatarTest::~AvatarTest ()
{
  lookAtNode->RemoveListener (&lookAtListener);
}

void AvatarTest::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  const float speed = elapsed_time / 1000.0f;

  // Compute camera and animesh position
  iCamera* c = view->GetCamera ();
  csVector3 cameraPosition = c->GetTransform ().GetOrigin ();
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  csVector3 avatarPosition = animeshObject->GetMeshWrapper ()->QuerySceneNode ()
    ->GetMovable ()->GetTransform ().GetOrigin () + csVector3 (0.0f, 0.35f, 0.0f);
  avatarPosition.y = 0.5f;

  // Move camera
  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the Up/Down arrow keys will cause
    // the camera to go forwards and backwards (forward only allowed if camera 
    // not too close). Left/Right arrows work also when shift is hold.
    if (kbd->GetKeyState (CSKEY_UP)
	&& (cameraPosition - avatarPosition).Norm () > 0.5f)
      c->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);
  }
  else
  {
    // Left and right arrows cause the camera to strafe on the X axis; up and 
    // down arrows cause the camera to strafe on the Y axis
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);

    // Avoid gimbal lock of camera
    cameraPosition.Normalize ();
    float cameraDot = cameraPosition * csVector3 (0.0f, 1.0f, 0.0f);
    if (kbd->GetKeyState (CSKEY_UP)
	&& cameraDot < 0.98f)
      c->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN)
	&& cameraDot > -0.98f)
      c->Move (CS_VEC_DOWN * 4 * speed);
  }

  // Make the camera look at the animesh
  c->GetTransform ().LookAt (avatarPosition - c->GetTransform ().GetOrigin (),
			     csVector3 (0,1,0) );

  // Step the dynamic simulation (we slow down artificially the simulation in
  // order to achieve a 'slow motion' effect)
  if (physicsEnabled)
    dynamics->Step (speed / 4.0f);

  // Update the morph state (frankie smiles sadistically if no target in view)
  if (targetReached)
    smileWeight -= (float) elapsed_time / 250.0f;
  else 
    smileWeight += (float) elapsed_time / 1500.0f;

  if (smileWeight > 1.0f)
    smileWeight = 1.0f;
  else if (smileWeight < 0.0f)
    smileWeight = 0.0f;

  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("smile.B"),
				 smileWeight);
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyebrows_down.B"),
				 smileWeight);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Write FPS and other info
  if(!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  int y = 480;
  int lineSize = 18;

  if (targetMode == LOOKAT_CAMERA)
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100),
		 "Watch out, Frankie is looking at you!");
  else if (targetMode == LOOKAT_POSITION)
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100),
		 "Frankie is looking at something");
  else if (targetMode == LOOKAT_NOTHING)
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100),
		 "Frankie doesn't care about anything");
  y += lineSize;

  if (alwaysRotate)
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100), "Always rotate: ON");
  else
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100), "Always rotate: OFF");
  y += lineSize;

  if (rotationSpeed == ROTATION_SLOW)
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100), "Rotation speed: really slow");
  else if (rotationSpeed == ROTATION_NORMAL)
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100), "Rotation speed: normal");
  else if (rotationSpeed == ROTATION_IMMEDIATE)
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100), "Rotation speed: infinite");
  y += lineSize;

  WriteShadow (20, y, g2d->FindRGB (255, 150, 100), "Walk speed: %.1f",
	      ((float) currentSpeed) / 10.0f);
  y += lineSize;

  if (speed != 0.0f)
  {
    WriteShadow (20, y, g2d->FindRGB (255, 150, 100), "FPS: %.2f",
		 1.0f / speed);
    y += lineSize;
  }

  // Write available keys
  DisplayKeys ();
}

bool AvatarTest::OnKeyboard (iEvent &ev)
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // Toggle the target mode of the 'LookAt' controller
    if (csKeyEventHelper::GetCookedCode (&ev) == 't')
    {
      if (targetMode == LOOKAT_CAMERA)
      {
	lookAtNode->SetTarget (view->GetCamera ()->GetTransform ().GetOrigin ());
	targetMode = LOOKAT_POSITION;
      }

      else if (targetMode == LOOKAT_POSITION)
      {
	lookAtNode->RemoveTarget ();
	targetMode = LOOKAT_NOTHING;
      }

      else
      {
	lookAtNode->SetTarget (view->GetCamera (), csVector3 (0.0f));
	targetMode = LOOKAT_CAMERA;
      }

      return true;
    }

    // Toggle 'always rotate' option of the 'LookAt' controller
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'a')
    {
      alwaysRotate = !alwaysRotate;
      lookAtNode->SetAlwaysRotate (alwaysRotate);
      return true;
    }

    // Toggle rotation speed of the 'LookAt' controller
    else if (csKeyEventHelper::GetCookedCode (&ev) == 's')
    {
      if (rotationSpeed == ROTATION_SLOW)
      {
	rotationSpeed = ROTATION_NORMAL;
	lookAtNode->SetMaximumSpeed (5.0f);
      }

      else if (rotationSpeed == ROTATION_NORMAL)
      {
	rotationSpeed = ROTATION_IMMEDIATE;
	lookAtNode->SetMaximumSpeed (0.0f);
      }

      else if (rotationSpeed == ROTATION_IMMEDIATE)
      {
	rotationSpeed = ROTATION_SLOW;
	lookAtNode->SetMaximumSpeed (0.5f);
      }

      return true;
    }

    // Update walk speed of the 'speed' controller
    else if (csKeyEventHelper::GetCookedCode (&ev) == '+')
    {
      if (currentSpeed < 58)
      {
	currentSpeed += 1;
	speedNode->SetSpeed (((float) currentSpeed) / 10.0f);
      }
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&ev) == '-')
    {
      if (currentSpeed > 0)
      {
	currentSpeed -= 1;
	speedNode->SetSpeed (((float) currentSpeed) / 10.0f);
      }
      return true;
    }

    // Reset of the scene
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'r')
    {
      ResetScene ();
      return true;
    }

    // Check for ESC key
    else if (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC)
    {
      csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (GetObjectRegistry ()));
      if (q) q->GetEventOutlet()->Broadcast (csevQuit (GetObjectRegistry ()));
      return true;
    }
  }

  return false;
}

bool AvatarTest::OnMouseDown (iEvent& ev)
{
  if (csMouseEventHelper::GetButton (&ev) == 0
      && physicsEnabled)
  {
    // Trying to kill Frankie

    // Compute the beam points to check what was hit
    int mouseX = csMouseEventHelper::GetX (&ev);
    int mouseY = csMouseEventHelper::GetY (&ev);

    csRef<iCamera> camera = view->GetCamera ();
    csVector2 v2d (mouseX, camera->GetShiftY () * 2 - mouseY);
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    // If Frankie is already dead, simply check for adding a force on him
    if (frankieDead)
    {
      // Trace a physical beam to find if a rigid body was hit
      csRef<iBulletDynamicSystem> bulletSystem =
	scfQueryInterface<iBulletDynamicSystem> (dynamicSystem);
      csBulletHitBeamResult physicsResult = bulletSystem->HitBeam (startBeam, endBeam);

      // Apply a big force at the point clicked by the mouse
      if (physicsResult.body)
      {
	csVector3 force = endBeam - startBeam;
	force.Normalize ();
	physicsResult.body->AddForceAtPos (physicsResult.isect, force * 10.0f);
      }

      return true;
    }

    // At first, test with a sector HitBeam if we clicked on an animated mesh
    csSectorHitBeamResult sectorResult = camera->GetSector ()->HitBeam
      (startBeam, endBeam, true);
    if (!sectorResult.mesh)
      return false;

    csRef<iAnimatedMesh> animesh =
      scfQueryInterface<iAnimatedMesh> (sectorResult.mesh->GetMeshObject ());
    if (!animesh)
      return false;

    // OK, it's an animesh, it must be Frankie, start the ragdoll
    frankieDead = true;

    // Close the eyes of Frankie as he is dead
    animesh->SetMorphTargetWeight
      (animeshFactory->FindMorphTarget ("eyelids_closed"), 0.7f);

    // Set the ragdoll animation node as the active state of the Finite State Machine
    // (start the ragdoll node so that the rigid bodies are created)
    FSMNode->SwitchToState (ragdollFSMState);
    FSMNode->GetStateNode (ragdollFSMState)->Play ();

    // Fling the body a bit
    const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();
    for (uint i = 0; i < ragdollNode->GetBoneCount (RAGDOLL_STATE_DYNAMIC); i++)
    {
      BoneID boneID = ragdollNode->GetBone (RAGDOLL_STATE_DYNAMIC, i);
      iRigidBody* rb = ragdollNode->GetBoneRigidBody (boneID);
      rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0.0f, 0.0f, 0.1f));
    }

    // Trace a physical beam to find which rigid body was hit
    csRef<iBulletDynamicSystem> bulletSystem =
      scfQueryInterface<iBulletDynamicSystem> (dynamicSystem);
    csBulletHitBeamResult physicsResult = bulletSystem->HitBeam (startBeam, endBeam);

    // Apply a big force at the point clicked by the mouse
    if (physicsResult.body)
    {
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      physicsResult.body->AddForceAtPos (physicsResult.isect, force * 1.0f);
      physicsResult.body->SetLinearVelocity (tc.GetT2O () * csVector3 (0.0f, 0.0f, 1.0f));
    }

    return true;
  }

  return false;
}

bool AvatarTest::OnInitialize (int /*argc*/, char* /*argv*/ [])
{
  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (GetObjectRegistry ()))
  {
    csPrintf ("Usage: avatartest\n");
    csPrintf ("Tests on animesh animation\n\n");
    csPrintf ("Options for avatartest:\n");
    csPrintf ("  -no_physics:       disable physical animations\n");
    csCommandLineHelper::Help (GetObjectRegistry ());
    return false;
  }

  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.controllers.lookat",
		       iSkeletonLookAtManager2),
    CS_REQUEST_PLUGIN ("crystalspace.mesh.animesh.controllers.basic",
		       iSkeletonBasicNodesManager2),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  // Check if physics effects are enabled
  csRef<iCommandLineParser> clp =
    csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());
  physicsEnabled = !clp->GetBoolOption ("no_physics", false);

  if (physicsEnabled)
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
    }
  }

  return true;
}

void AvatarTest::OnExit ()
{
  printer.Invalidate ();
}

bool AvatarTest::Application ()
{
  if (!OpenApplication (GetObjectRegistry ()))
    return ReportError ("Error opening system!");

  g3d = csQueryRegistry<iGraphics3D> (GetObjectRegistry ());
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (GetObjectRegistry ());
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = csQueryRegistry<iVirtualClock> (GetObjectRegistry ());
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = csQueryRegistry<iKeyboardDriver> (GetObjectRegistry ());
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = csQueryRegistry<iLoader> (GetObjectRegistry ());
  if (!loader) return ReportError("Failed to locate Loader!");

  g2d = csQueryRegistry<iGraphics2D> (GetObjectRegistry ());
  if (!g2d) return ReportError("Failed to locate 2D renderer!");

  lookAtManager = csQueryRegistry<iSkeletonLookAtManager2> (GetObjectRegistry ());
  if (!lookAtManager) return ReportError("Failed to locate iLookAtManager plugin!");

  basicNodesManager =
    csQueryRegistry<iSkeletonBasicNodesManager2> (GetObjectRegistry ());
  if (!basicNodesManager)
    return ReportError("Failed to locate iSkeletonBasicNodesManager2 plugin!");

  printer.AttachNew (new FramePrinter (GetObjectRegistry ()));

  csRef<iFontServer> fs = g3d->GetDriver2D()->GetFontServer ();
  if (fs)
    courierFont = fs->LoadFont (CSFONT_COURIER);
  else return ReportError ("Failed to locate font server!");

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
      // Load the ragdoll plugin
      csRef<iPluginManager> plugmgr = 
	csQueryRegistry<iPluginManager> (GetObjectRegistry ());
      ragdollManager = csLoadPlugin<iSkeletonRagdollManager2>
	(plugmgr, "crystalspace.mesh.animesh.controllers.ragdoll");
      if (!ragdollManager)
      {
	ReportWarning
	  ("Can't load ragdoll plugin, continuing with reduced functionalities");
	physicsEnabled = false;
      }
    }
  }

  // Create sector
  room = engine->CreateSector ("room");

  // Initialize camera
  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0.0f, 0.0f, -1.25f));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // Create scene
  CreateRoom ();
  if (!CreateAvatar ())
    return false;

  // Run the application
  Run();

  return true;
}

void AvatarTest::CreateRoom ()
{
  // Creating the background
  // First we make a primitive for our geometry.
  CS::Geometry::DensityTextureMapper bgMapper (0.3f);
  CS::Geometry::TesselatedBox bgBox (csVector3 (-4000), csVector3 (4000));
  bgBox.SetMapper (&bgMapper);
  bgBox.SetFlags (CS::Geometry::Primitives::CS_PRIMBOX_INSIDE);
  
  // Now we make a factory and a mesh at once.
  csRef<iMeshWrapper> background =
    CS::Geometry::GeneralMeshBuilder::CreateFactoryAndMesh (engine, room,
				   "background", "background_factory", &bgBox);

  csRef<iMaterialWrapper> bgMaterial =
    CS::Material::MaterialBuilder::CreateColorMaterial
    (GetObjectRegistry (), "background", csColor (0.398f));
  background->GetMeshObject()->SetMaterialWrapper (bgMaterial);

  // Set up of the physical collider for the roof
  if (physicsEnabled)
    dynamicSystem->AttachColliderPlane (csPlane3 (csVector3 (0.0f, 1.0f, 0.0f), 0.0f),
					10.0f, 0.0f);
  // Creating lights
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  // This light is for the background
  // TODO: putting instead the following line creates a black background, otherwise it is grey
  // the behavior doesn't persist if the other lights are removed
  //light = engine->CreateLight(0, csVector3(1, 1, -1), 9000, csColor (1));
  light = engine->CreateLight(0, csVector3(1, 1, 0), 9000, csColor (1));
  light->SetAttenuationMode (CS_ATTN_NONE);
  ll->Add (light);

  // Other lights
  light = engine->CreateLight (0, csVector3 (3, 0, 0), 8, csColor (1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (-3, 0,  0), 8, csColor (1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 0, -3), 8, csColor (1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, -3, 0), 8, csColor (1));
  light->SetAttenuationMode (CS_ATTN_REALISTIC);
  ll->Add (light);

  engine->Prepare ();

  CS::Lighting::SimpleStaticLighter::ShineLights (room, engine, 4);
}

bool AvatarTest::CreateAvatar ()
{
  // Load animesh factory
  csLoadResult rc = loader->Load ("/lib/frankie/frankie.xml");
  if (!rc.success)
    return ReportError ("Can't load Frankie library file!");

  csRef<iMeshFactoryWrapper> meshfact = engine->FindMeshFactory ("franky_frankie");
  if (!meshfact)
    return ReportError ("Can't find Frankie's mesh factory!");

  animeshFactory = scfQueryInterface<iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
    return ReportError ("Can't find Frankie's animesh factory!");

  // Load bodymesh (animesh's physical properties)
  rc = loader->Load ("/lib/frankie/skelfrankie_body");
  if (!rc.success)
    return ReportError ("Can't load frankie's body mesh file!");

  csRef<iBodyManager> bodyManager = csQueryRegistry<iBodyManager> (GetObjectRegistry ());
  csRef<iBodySkeleton> bodySkeleton = bodyManager->FindBodySkeleton ("frankie_body");
  if (!bodySkeleton)
    return ReportError ("Can't find Frankie's body mesh description!");

  // Create a new animation tree. The structure of the tree is:
  //   + Finite State Machine node (root node)
  //     + 'LookAt' controller node
  //       + 'speed' controller node
  //         + animation nodes for all speeds
  //     + ragdoll controller node
  csRef<iSkeletonAnimPacketFactory2> animPacketFactory =
    animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ();

  // Create the Finite State Machine node
  csRef<iSkeletonFSMNodeFactory2> FSMNodeFactory =
    animPacketFactory->CreateFSMNode ("fsm");
  animPacketFactory->SetAnimationRoot (FSMNodeFactory);

  // Create the 'LookAt' controller
  csRef<iSkeletonLookAtNodeFactory2> lookAtNodeFactory =
    lookAtManager->CreateAnimNodeFactory ("lookat", bodySkeleton);
  mainFSMState = FSMNodeFactory->AddState
    ("main_state", lookAtNodeFactory);
  FSMNodeFactory->SetStartState (mainFSMState);

  // Create the 'idle' animation node
  csRef<iSkeletonAnimationNodeFactory2> idleNodeFactory =
    animPacketFactory->CreateAnimationNode ("idle");
  idleNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Idle1"));

  // Create the 'walk_slow' animation node
  csRef<iSkeletonAnimationNodeFactory2> walkSlowNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk_slow");
  walkSlowNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_WalkSlow"));

  // Create the 'walk' animation node
  csRef<iSkeletonAnimationNodeFactory2> walkNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk");
  walkNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Walk"));

  // Create the 'walk_fast' animation node
  csRef<iSkeletonAnimationNodeFactory2> walkFastNodeFactory =
    animPacketFactory->CreateAnimationNode ("walk_fast");
  walkFastNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_WalkFast"));

  // Create the 'footing' animation node
  csRef<iSkeletonAnimationNodeFactory2> footingNodeFactory =
    animPacketFactory->CreateAnimationNode ("footing");
  footingNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Runs"));

  // Create the 'run_slow' animation node
  csRef<iSkeletonAnimationNodeFactory2> runSlowNodeFactory =
    animPacketFactory->CreateAnimationNode ("run_slow");
  runSlowNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_RunSlow"));

  // Create the 'run' animation node
  csRef<iSkeletonAnimationNodeFactory2> runNodeFactory =
    animPacketFactory->CreateAnimationNode ("run");
  runNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_Run"));

  // Create the 'run_fast' animation node
  csRef<iSkeletonAnimationNodeFactory2> runFastNodeFactory =
    animPacketFactory->CreateAnimationNode ("run_fast");
  runFastNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_RunFaster"));

  // Create the 'run_jump' animation node
  csRef<iSkeletonAnimationNodeFactory2> runJumpNodeFactory =
    animPacketFactory->CreateAnimationNode ("run_jump");
  runJumpNodeFactory->SetAnimation
    (animPacketFactory->FindAnimation ("Frankie_RunFast2Jump"));

  // Create the 'speed' controller (and add all animations of Frankie moving at different speeds)
  // Unfortunately, the Frankie animations from 'walk fast' to 'footing'
  // do not blend well together, but this is just an example...
  csRef<iSkeletonSpeedNodeFactory2> speedNodeFactory =
    basicNodesManager->CreateSpeedNodeFactory ("speed");
  speedNodeFactory->AddNode (idleNodeFactory, 0.0f);
  speedNodeFactory->AddNode (walkSlowNodeFactory, 0.4f);
  speedNodeFactory->AddNode (walkNodeFactory, 0.6f);
  speedNodeFactory->AddNode (walkFastNodeFactory, 1.2f);
  speedNodeFactory->AddNode (footingNodeFactory, 1.6f);
  speedNodeFactory->AddNode (runSlowNodeFactory, 2.6f);
  speedNodeFactory->AddNode (runNodeFactory, 3.4f);
  speedNodeFactory->AddNode (runFastNodeFactory, 5.0f);
  speedNodeFactory->AddNode (runJumpNodeFactory, 5.8f);

  lookAtNodeFactory->SetChildNode (speedNodeFactory);

  if (physicsEnabled)
  {
    // Create the ragdoll controller
    csRef<iSkeletonRagdollNodeFactory2> ragdollNodeFactory =
      ragdollManager->CreateAnimNodeFactory ("ragdoll",
					     bodySkeleton, dynamicSystem);
    ragdollFSMState = FSMNodeFactory->AddState
      ("ragdoll_state", ragdollNodeFactory);

    // Create bone chain
    iBodyChain* chain = bodySkeleton->CreateBodyChain
      ("body_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Frankie_Main"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"),
       animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_8"), 0);
    ragdollNodeFactory->AddBodyChain (chain, RAGDOLL_STATE_DYNAMIC);
  }

  // Create the animated mesh
  csRef<iMeshWrapper> avatarMesh = engine->CreateMeshWrapper (meshfact, "Frankie",
					   room, csVector3 (0.0f));
  animesh = scfQueryInterface<iAnimatedMesh> (avatarMesh->GetMeshObject ());

  // When the animated mesh is created, the animation nodes are created too.
  // We can therefore set them up
  iSkeletonAnimNode2* rootNode =
    animesh->GetSkeleton ()->GetAnimationPacket ()->GetAnimationRoot ();

  // Setup of the FSM node
  FSMNode = scfQueryInterface<iSkeletonFSMNode2> (rootNode->FindNode ("fsm"));

  // Setup of the LookAt controller
  lookAtNode = scfQueryInterface<iSkeletonLookAtNode2> (rootNode->FindNode ("lookat"));
  lookAtNode->AddListener (&lookAtListener);
  lookAtNode->SetAnimatedMesh (animesh);
  lookAtNode->SetBone (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"));
  lookAtNode->SetListenerDelay (0.6f);

  // Setup of the speed controller
  speedNode = scfQueryInterface<iSkeletonSpeedNode2> (rootNode->FindNode ("speed"));

  // Setup of the ragdoll controller
  if (physicsEnabled)
  {
    ragdollNode =
      scfQueryInterface<iSkeletonRagdollNode2> (rootNode->FindNode ("ragdoll"));
    ragdollNode->SetAnimatedMesh (animesh);
  }

  // Reset the scene so as to put the parameters of the animation nodes in a default state
  ResetScene ();

  // Start animation
  rootNode->Play ();

  return true;
}

void AvatarTest::LookAtListener::TargetReached ()
{
  printf ("'LookAt' target reached\n");
  avatarTest->targetReached = true;
}

void AvatarTest::LookAtListener::TargetLost ()
{
  printf ("'LookAt' target lost\n");
  avatarTest->targetReached = false;
}

void AvatarTest::ResetScene ()
{
  // Reset animesh position
  csRef<iMeshObject> animeshObject = scfQueryInterface<iMeshObject> (animesh);
  animeshObject->GetMeshWrapper ()->QuerySceneNode ()->GetMovable ()->SetTransform
    (csOrthoTransform (csMatrix3 (), csVector3 (0.0f)));

  // Reset initial Finite State Machine state
  FSMNode->SwitchToState (mainFSMState);

  // The FSM doesn't stop the child nodes
  if (physicsEnabled)
    ragdollNode->Stop ();

  // Reset 'LookAt' controller
  alwaysRotate = false;
  lookAtNode->SetAlwaysRotate (alwaysRotate);
  targetMode = LOOKAT_CAMERA;
  lookAtNode->SetTarget (view->GetCamera(), csVector3 (0.0f));
  rotationSpeed = ROTATION_NORMAL;
  lookAtNode->SetMaximumSpeed (5.0f);

  // Reset 'speed' controller
  currentSpeed = 0;
  speedNode->SetSpeed (((float) currentSpeed) / 10.0f);

  // Reset morphing
  smileWeight = 1.0f;
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("smile.B"), 1.0f);
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyebrows_down.B"), 1.0f);
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("wings_in"), 1.0f);
  animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyelids_closed"), 0.0f);

  frankieDead = false;
}

void AvatarTest::WriteShadow (int x, int y, int fg, const char *str,...)
{
  csString buf;

  va_list arg;
  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  Write (x+1, y-1, 0, -1, "%s", buf.GetData());
  Write (x, y, fg, -1, "%s", buf.GetData());
}

void AvatarTest::Write(int x, int y, int fg, int bg, const char *str,...)
{
  va_list arg;
  csString buf;

  va_start (arg,str);
  buf.FormatV (str, arg);
  va_end (arg);

  g2d->Write (courierFont, x, y, fg, bg, buf);
}

void AvatarTest::DisplayKeys ()
{
  int x = 20;
  int y = 20;
  int fg = g2d->FindRGB (255, 150, 100);
  int lineSize = 18;

  WriteShadow (x - 5, y, fg, "Keys available:");
  y += lineSize;

  WriteShadow (x, y, fg, "arrow keys: move camera");
  y += lineSize;

  WriteShadow (x, y, fg, "SHIFT-up/down keys: camera closer/farther");
  y += lineSize;

  WriteShadow (x, y, fg, "+/-: walk faster/slower");
  y += lineSize;

  WriteShadow (x, y, fg, "t: toggle 'LookAt' target mode");
  y += lineSize;

  WriteShadow (x, y, fg, "a: toggle 'LookAt: always rotate' mode");
  y += lineSize;

  WriteShadow (x, y, fg, "s: toggle 'LookAt: rotation speed'");
  y += lineSize;

  if (physicsEnabled)
  {
    WriteShadow (x, y, fg, "left mouse: kill Frankie");
    y += lineSize;
  }

  WriteShadow (x, y, fg, "r: reset scene");
  y += lineSize;
}

//---------------------------------------------------------------------------

int main (int argc, char* argv[])
{
  return AvatarTest ().Main (argc, argv);
}
