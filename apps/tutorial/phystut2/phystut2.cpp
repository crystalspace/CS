#include "cssysdef.h"
#include "csgeom/sphere.h"
#include "imesh/genmesh.h"
#include "imesh/terrain2.h"
#include "cstool/genmeshbuilder.h"
#include "cstool/materialbuilder.h"
#include "phystut2.h"

#define CAMERA_DYNAMIC 1
#define CAMERA_KINEMATIC 2
#define CAMERA_FREE 3

#define ENVIRONMENT_WALLS 1
#define ENVIRONMENT_TERRAIN 2

Simple::Simple()
: DemoApplication ("CrystalSpace.PhysTut2"),
isSoftBodyWorld (true), environment (ENVIRONMENT_TERRAIN), solver (0),
autodisable (false), do_bullet_debug (false), remainingStepDuration (0.0f),
debugMode (false), allStatic (false), pauseDynamic (false), dynamicSpeed (1.0f),
physicalCameraMode (CAMERA_DYNAMIC), dragging (false), softDragging (false)
{
  localTrans.Identity ();
}

Simple::~Simple ()
{
}

void Simple::PrintHelp ()
{
  csCommandLineHelper commandLineHelper;

  // Command line options
  commandLineHelper.AddCommandLineOption
    ("phys_engine", "Specify which physics plugin to use", csVariant ("bullet2"));
  commandLineHelper.AddCommandLineOption
    ("soft", "Enable the soft bodies", csVariant (true));
  commandLineHelper.AddCommandLineOption
    ("terrain", "Start with the terrain environment", csVariant ());

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (GetObjectRegistry (), "phystut2",
    "phystut2 <OPTIONS>",
    "Physics tutorial 2 for Crystal Space.");
}

void Simple::Frame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  const float speed = elapsed_time / 1000.0;

  // Camera is controlled by a rigid body
  if (physicalCameraMode == CAMERA_DYNAMIC)
  {
    if (kbd->GetKeyState (CSKEY_RIGHT))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_ROT_RIGHT, speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_ROT_LEFT, speed);
    if (kbd->GetKeyState (CSKEY_PGUP))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_TILT_UP, speed);
    if (kbd->GetKeyState (CSKEY_PGDN))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_TILT_DOWN, speed);
    if (kbd->GetKeyState (CSKEY_UP))
    {
      cameraBody->SetLinearVelocity (view->GetCamera()->GetTransform()
        .GetT2O () * csVector3 (0, 0, 5));
    }
    if (kbd->GetKeyState (CSKEY_DOWN))
    {
      cameraBody->SetLinearVelocity (view->GetCamera()->GetTransform()
        .GetT2O () * csVector3 (0, 0, -5));
    }
  }

  // Camera is free
  else
  {
    iCamera* c = view->GetCamera();

    float cameraSpeed = environment == ENVIRONMENT_WALLS ? 4 : 30;
    if (kbd->GetKeyState (CSKEY_SHIFT))
    {
      // If the user is holding down shift, the arrow keys will cause
      // the camera to strafe up, down, left or right from it's
      // current position.
      if (kbd->GetKeyState (CSKEY_RIGHT))
        c->Move (CS_VEC_RIGHT * cameraSpeed * speed);
      if (kbd->GetKeyState (CSKEY_LEFT))
        c->Move (CS_VEC_LEFT * cameraSpeed * speed);
      if (kbd->GetKeyState (CSKEY_UP))
        c->Move (CS_VEC_UP * cameraSpeed * speed);
      if (kbd->GetKeyState (CSKEY_DOWN))
        c->Move (CS_VEC_DOWN * cameraSpeed * speed);
    }
    else
    {
      // left and right cause the camera to rotate on the global Y
      // axis; page up and page down cause the camera to rotate on the
      // _camera's_ X axis (more on this in a second) and up and down
      // arrows cause the camera to go forwards and backwards.
      if (kbd->GetKeyState (CSKEY_RIGHT))
        rotY += speed;
      if (kbd->GetKeyState (CSKEY_LEFT))
        rotY -= speed;
      if (kbd->GetKeyState (CSKEY_PGUP))
        rotX -=speed;
      if (kbd->GetKeyState (CSKEY_PGDN))
        rotX += speed;
      if (kbd->GetKeyState (CSKEY_UP))
        c->Move (CS_VEC_FORWARD * cameraSpeed * speed);
      if (kbd->GetKeyState (CSKEY_DOWN))
        c->Move (CS_VEC_BACKWARD * cameraSpeed * speed);
    }

    // We now assign a new rotation transformation to the camera.
    csQuaternion quaternion;
    quaternion.SetEulerAngles (csVector3 (rotX, rotY, rotZ));
    csOrthoTransform ot (quaternion.GetConjugate ().GetMatrix (), c->GetTransform().GetOrigin ());
    c->SetTransform (ot);
  }

  if (dragging)
  {
    // Keep the drag joint at the same distance to the camera
    csRef<iCamera> camera = view->GetCamera ();
    csVector2 v2d (mouse->GetLastX (), g2d->GetHeight () - mouse->GetLastY ());
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    csVector3 newPosition = endBeam - startBeam;
    newPosition.Normalize ();
    newPosition = camera->GetTransform ().GetOrigin () + newPosition * dragDistance;
    dragJoint->SetPosition (newPosition);
  }

  if (!pauseDynamic)
    physicalSector->Step (speed / dynamicSpeed);

  // Update camera position if it is controlled by a rigid body.
  // (in this mode we want to control the orientation of the camera,
  // so we update the camera position by ourselves instead of using
  // 'cameraBody->AttachCamera (camera)')
  if (physicalCameraMode == CAMERA_DYNAMIC)
    view->GetCamera ()->GetTransform ().SetOrigin
    (cameraBody->GetTransform ().GetOrigin ());

  // Update the demo's state information
  hudManager->GetStateDescriptions ()->Empty ();
  csString txt;

  hudManager->GetStateDescriptions ()->Push (csString ("Physics engine: ") + phys_engine_name);

  txt.Format ("Rigid bodies count: %d", physicalSector->GetRigidBodyCount ());
  hudManager->GetStateDescriptions ()->Push (txt);

  if (isSoftBodyWorld)
  {
    txt.Format ("Soft bodies count: %d", (int) physicalSector->GetSoftBodyCount ());
    hudManager->GetStateDescriptions ()->Push (txt);
  }

  switch (physicalCameraMode)
  {
  case CAMERA_DYNAMIC:
    hudManager->GetStateDescriptions ()->Push (csString ("Camera mode: dynamic"));
    break;

  case CAMERA_FREE:
    hudManager->GetStateDescriptions ()->Push (csString ("Camera mode: free"));
    break;

  case CAMERA_KINEMATIC:
    hudManager->GetStateDescriptions ()->Push (csString ("Camera mode: kinematic"));
    break;

  default:
    break;
  }

  // Default behavior from DemoApplication
  DemoApplication::Frame ();

  // Display debug informations
  if (do_bullet_debug)
    bulletSector->DebugDraw (view);
  else if (isSoftBodyWorld)
    for (size_t i = 0; i < physicalSector->GetSoftBodyCount (); i++)
    {
      CS::Physics::iSoftBody* softBody = physicalSector->GetSoftBody (i);
      csRef<CS::Physics::Bullet::iSoftBody> bulletSoftBody = 
        scfQueryInterface<CS::Physics::Bullet::iSoftBody> (softBody);
      if (!softBody->GetTriangleCount ())
        bulletSoftBody->DebugDraw (view);
    }
}

bool Simple::OnKeyboard (iEvent &event)
{
  DemoApplication::OnKeyboard (event);

  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&event);
  if (eventtype == csKeyEventTypeDown)
  {
    if (csKeyEventHelper::GetCookedCode (&event) == CSKEY_SPACE)
    {
      int primitiveCount = 7;
      switch (rand() % primitiveCount)
      {
      case 0: SpawnBox (); break;
      case 1: SpawnSphere (); break;
      case 2: SpawnConvexMesh (); break;
      case 3: SpawnJointed (); break;
      case 4: SpawnCylinder (); break;
      case 5: SpawnCapsule (); break;
      case 6: SpawnRagdoll (); break;
      case 7: SpawnCone (); break;
      case 8: SpawnCompound (); break;
      default: break;
      }
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'b')
    {
      SpawnBox ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 's')
    {
      SpawnSphere ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'c')
    {
      SpawnCylinder ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'a')
    {
      SpawnCapsule ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'n')
    {
      SpawnCone ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'm')
    {
      SpawnConcaveMesh ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'v')
    {
      SpawnConvexMesh ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'q')
    {
      SpawnCompound ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'j')
    {
      SpawnJointed ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'h')
    {
      SpawnChain ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'r')
    {
      SpawnRagdoll ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'y' && isSoftBodyWorld)
    {
      SpawnRope ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'u' && isSoftBodyWorld)
    {
      SpawnCloth ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&event) == 'i' && isSoftBodyWorld)
    {
      SpawnSoftBody ();
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&event) == 'f')
    {
      // Toggle camera mode
      switch (physicalCameraMode)
      {
      case CAMERA_DYNAMIC:
        physicalCameraMode = CAMERA_FREE;
        break;

      case CAMERA_FREE:
          physicalCameraMode = CAMERA_KINEMATIC;
        break;

      case CAMERA_KINEMATIC:
        physicalCameraMode = CAMERA_DYNAMIC;
        break;
      }

      UpdateCameraMode ();
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&event) == 't')
    {
      // Toggle all bodies between dynamic and static
      allStatic = !allStatic;

      if (allStatic)
        printf ("Toggling all bodies to static mode\n");
      else
        printf ("Toggling all bodies to dynamic mode\n");

      for (size_t i = 0; i < physicalSector->GetRigidBodyCount (); i++)
      {
        CS::Physics::iRigidBody* body = physicalSector->GetRigidBody (i);
        if (allStatic)
          body->SetState (CS::Physics::STATE_STATIC);
        else
        {
          body->SetState (CS::Physics::STATE_DYNAMIC);
          body->Enable ();
        }
      }
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&event) == 'p')
    {
      // Toggle pause mode for dynamic simulation
      pauseDynamic = !pauseDynamic;
      if (pauseDynamic)
        printf ("Dynamic simulation paused\n");
      else
        printf ("Dynamic simulation resumed\n");
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&event) == 'o')
    {
      // Toggle speed of dynamic simulation
      if (dynamicSpeed - 1.0 < 0.00001)
      {
        dynamicSpeed = 45.0;
        printf ("Dynamic simulation slowed\n");
      }
      else
      {
        dynamicSpeed = 1.0;
        printf ("Dynamic simulation at normal speed\n");
      }
    }

    else if (csKeyEventHelper::GetCookedCode (&event) == 'd')
    {
      // Toggle dynamic system visual debug mode
      // TODO
      return true;
    }

    else if (csKeyEventHelper::GetCookedCode (&event) == 'g')
    {
      // Toggle gravity.
      collisionSector->SetGravity (collisionSector->GetGravity () == 0 ?
        csVector3 (0.0f, -9.81f, 0.0f) : csVector3 (0));
      return true;
    }

    // Cut operation
    else if (csKeyEventHelper::GetRawCode (&event) == 'x'
      && kbd->GetKeyState (CSKEY_CTRL))
    {
      // Trace a beam to find if a rigid body was under the mouse cursor
      csRef<iCamera> camera = view->GetCamera ();
      csVector2 v2d (mouse->GetLastX (), g2d->GetHeight () - mouse->GetLastY ());
      csVector3 v3d = camera->InvPerspective (v2d, 10000);
      csVector3 startBeam = camera->GetTransform ().GetOrigin ();
      csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

      CS::Collision::HitBeamResult hitResult =
        collisionSector->HitBeam (startBeam, endBeam);
      if (hitResult.hasHit
        && hitResult.object->GetObjectType () == CS::Collision::COLLISION_OBJECT_PHYSICAL)
      {
        // Remove the body and the mesh from the simulation, and put them in the clipboard
        clipboardBody = scfQueryInterface<CS::Physics::iPhysicalBody> (hitResult.object);
        if (clipboardBody->GetBodyType () == CS::Physics::BODY_RIGID)
          physicalSector->RemoveRigidBody (clipboardBody->QueryRigidBody ());
        else
          physicalSector->RemoveSoftBody (clipboardBody->QuerySoftBody ());

        clipboardMovable = hitResult.object->GetAttachedMovable ();
        if (clipboardMovable)
          room->GetMeshes ()->Remove (clipboardMovable->GetSceneNode ()->QueryMesh ());

        // Update the display of the dynamics debugger
        //dynamicsDebugger->UpdateDisplay ();
      }
    }

    // Paste operation
    else if (csKeyEventHelper::GetRawCode (&event) == 'v'
      && kbd->GetKeyState (CSKEY_CTRL)
      && clipboardBody.IsValid ())
    {
      // Compute the new position of the body
      csRef<iCamera> camera = view->GetCamera ();
      csVector2 v2d (mouse->GetLastX (), g2d->GetHeight () - mouse->GetLastY ());
      csVector3 v3d = camera->InvPerspective (v2d, 10000);
      csVector3 startBeam = camera->GetTransform ().GetOrigin ();
      csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

      csVector3 newPosition = endBeam - startBeam;
      newPosition.Normalize ();
      csOrthoTransform newTransform = camera->GetTransform ();
      newTransform.SetOrigin (newTransform.GetOrigin () + newPosition * 1.5f);
      clipboardBody->SetTransform (newTransform);

      // Put back the body from the clipboard to the simulation
      if (clipboardBody->GetBodyType () == CS::Physics::BODY_RIGID)
        physicalSector->AddRigidBody (clipboardBody->QueryRigidBody ());
      else
        physicalSector->AddSoftBody (clipboardBody->QuerySoftBody ());
      //TODO---------------------------------------------------------------------------
      room->GetMeshes ()->Add (clipboardMovable->GetSceneNode ()->QueryMesh ());
      clipboardBody = 0;
      clipboardMovable = 0;

      // Update the display of the dynamics debugger
      //dynamicsDebugger->UpdateDisplay ();
    }

#ifdef CS_HAVE_BULLET_SERIALIZER
    // Save a .bullet file
    else if (csKeyEventHelper::GetRawCode (&ev) == 's'
      && kbd->GetKeyState (CSKEY_CTRL))
    {
      const char* filename = "phystut_world.bullet";
      if (bulletDynamicSystem->SaveBulletWorld (filename))
        printf ("Dynamic world successfully saved as file %s\n", filename);
      else
        printf ("Problem saving dynamic world to file %s\n", filename);

      return true;
    }
#endif

    else if (csKeyEventHelper::GetRawCode (&event) == 'i'
      && kbd->GetKeyState (CSKEY_CTRL))
    {
      printf ("Starting profile...\n");
      bulletSector->StartProfile ();
      return true;
    }

    else if (csKeyEventHelper::GetRawCode (&event) == 'o'
      && kbd->GetKeyState (CSKEY_CTRL))
    {
      printf ("Stopping profile...\n");
      bulletSector->StopProfile ();
      return true;
    }

    else if (csKeyEventHelper::GetRawCode (&event) == 'p'
      && kbd->GetKeyState (CSKEY_CTRL))
    {
      bulletSector->DumpProfile ();
      return true;
    }
  }

  // Slow down the camera's body
  else if (physicalCameraMode == CAMERA_DYNAMIC
    && (eventtype == csKeyEventTypeUp)
    && ((csKeyEventHelper::GetCookedCode (&event) == CSKEY_DOWN) 
    || (csKeyEventHelper::GetCookedCode (&event) == CSKEY_UP)))
  {
    cameraBody->SetLinearVelocity(csVector3 (0, 0, 0));
    cameraBody->SetAngularVelocity (csVector3 (0, 0, 0));
  }

  return false;
}

// This method updates the position of the dragging for soft bodies
csVector3 MouseAnchorAnimationControl::GetAnchorPosition () const
{
  // Keep the drag joint at the same distance to the camera
  csRef<iCamera> camera = simple->view->GetCamera ();
  csVector2 v2d (simple->mouse->GetLastX (), simple->g2d->GetHeight () - simple->mouse->GetLastY ());
  csVector3 v3d = camera->InvPerspective (v2d, 10000);
  csVector3 startBeam = camera->GetTransform ().GetOrigin ();
  csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

  csVector3 newPosition = endBeam - startBeam;
  newPosition.Normalize ();
  newPosition = camera->GetTransform ().GetOrigin () + newPosition * simple->dragDistance;
  return newPosition;
}

bool Simple::OnMouseDown (iEvent &event)
{
  if (csMouseEventHelper::GetButton (&event) == 0)
  {
    // Find the rigid body that was clicked on
    // Compute the end beam points
    csRef<iCamera> camera = view->GetCamera ();
    csVector2 v2d (mouse->GetLastX (), g2d->GetHeight () - mouse->GetLastY ());
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    // Trace the physical beam
    CS::Collision::HitBeamResult hitResult =
      collisionSector->HitBeam (startBeam, endBeam);
    if (!hitResult.hasHit)
      return false;

    // Add a force at the point clicked
    if (hitResult.object->GetObjectType () == CS::Collision::COLLISION_OBJECT_PHYSICAL)
    {
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      force *= 2.0f;

      csRef<CS::Physics::iPhysicalBody> physicalBody = scfQueryInterface<CS::Physics::iPhysicalBody> (hitResult.object);
      if (physicalBody->GetBodyType () == CS::Physics::BODY_RIGID)
      {
        // Check if the body hit is not static or kinematic
        csRef<CS::Physics::iRigidBody> bulletBody =
          scfQueryInterface<CS::Physics::iRigidBody> (hitResult.object);
        if (bulletBody->GetState () != CS::Physics::STATE_DYNAMIC)
          return false;

        physicalBody->QueryRigidBody ()->AddForceAtPos (force, hitResult.isect);

        // This would work too
        //csOrthoTransform transform (hitResult.body->QueryRigidBody ()->GetTransform ());
        //csVector3 relativePosition = transform.Other2This (hitResult.isect);
        //hitResult.body->QueryRigidBody ()->AddForceAtRelPos (force, relativePosition);
      }
      else
      {
        csVector3 force = endBeam - startBeam;
        force.Normalize ();
        force *= 2.0f;
        physicalBody->QuerySoftBody ()->AddForce (force, hitResult.vertexIndex);
      }
    }
    else
      return false;
    return true;
  }

  // Right mouse button: dragging
  else if (csMouseEventHelper::GetButton (&event) == 1)
  {
    // Find the rigid body that was clicked on
    // Compute the end beam points
    csRef<iCamera> camera = view->GetCamera ();
    csVector2 v2d (mouse->GetLastX (), g2d->GetHeight () - mouse->GetLastY ());
    csVector3 v3d = camera->InvPerspective (v2d, 10000);
    csVector3 startBeam = camera->GetTransform ().GetOrigin ();
    csVector3 endBeam = camera->GetTransform ().This2Other (v3d);

    // Trace the physical beam
    CS::Collision::HitBeamResult hitResult =
      collisionSector->HitBeam (startBeam, endBeam);
    if (!hitResult.hasHit)
      return false;

    // Check if we hit a rigid body
    if (hitResult.object->GetObjectType () == CS::Collision::COLLISION_OBJECT_PHYSICAL)
    {
      csRef<CS::Physics::iPhysicalBody> physicalBody = scfQueryInterface<CS::Physics::iPhysicalBody> (hitResult.object);
      if (physicalBody->GetBodyType () == CS::Physics::BODY_RIGID)
      {
        // Create a p2p joint at the point clicked
        dragJoint = physicalSystem->CreateRigidP2PJoint (hitResult.isect - hitResult.object->GetTransform ().GetOrigin ());
        dragJoint->Attach (physicalBody, NULL);

        dragging = true;
        dragDistance = (hitResult.isect - startBeam).Norm ();

        // Set some dampening on the rigid body to have a more stable dragging
        csRef<CS::Physics::iRigidBody> bulletBody =
          scfQueryInterface<CS::Physics::iRigidBody> (hitResult.object);
        linearDampening = bulletBody->GetLinearDampener ();
        angularDampening = bulletBody->GetRollingDampener ();
        bulletBody->SetLinearDampener (0.9f);
        bulletBody->SetRollingDampener (0.9f);
      }
      else
      {
        softDragging = true;
        draggedBody = physicalBody->QuerySoftBody ();
        draggedVertex = hitResult.vertexIndex;
        dragDistance = (hitResult.isect - startBeam).Norm ();
        grabAnimationControl.AttachNew (new MouseAnchorAnimationControl (this));
        physicalBody->QuerySoftBody ()->AnchorVertex (hitResult.vertexIndex, grabAnimationControl);
      }
    }
    else 
      return false;
    return true;
  }

  return false;
}

bool Simple::OnMouseUp (iEvent &event)
{
  if (csMouseEventHelper::GetButton (&event) == 1
    && dragging)
  {
    dragging = false;

    // Put back the original dampening on the rigid body
    csRef<CS::Physics::iRigidBody> bulletBody =
      scfQueryInterface<CS::Physics::iRigidBody> (dragJoint->GetAttachedBody (0));
    bulletBody->SetLinearDampener (linearDampening);
    bulletBody->SetRollingDampener (angularDampening);

    // Remove the drag joint
    physicalSector->RemoveJoint (dragJoint);
    dragJoint = NULL;
    return true;
  }

  if (csMouseEventHelper::GetButton (&event) == 1
    && softDragging)
  {
    softDragging = false;
    draggedBody->RemoveAnchor (draggedVertex);
    draggedBody = 0;
  }

  return false;
}

bool Simple::OnInitialize (int argc, char* argv[])
{
  // Default behavior from DemoApplication
  if (!DemoApplication::OnInitialize (argc, argv))
    return false;

  // Request plugins
  /*if (!csInitializer::RequestPlugins (GetObjectRegistry (),
    CS_REQUEST_PLUGIN ("crystalspace.dynamics.debug",
    CS::Debug::iDynamicsDebuggerManager),
    CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");*/

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  // Checking for choosen dynamic system
  csRef<iCommandLineParser> clp = csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());
  phys_engine_name = clp->GetOption ("phys_engine");
  
  phys_engine_name = "Bullet";
  csRef<iPluginManager> plugmgr = 
    csQueryRegistry<iPluginManager> (GetObjectRegistry ());
  collisionSystem = csLoadPlugin<CS::Collision::iCollisionSystem> (plugmgr, "crystalspace.dynamics.bullet2");
  physicalSystem = scfQueryInterface<CS::Physics::iPhysicalSystem> (collisionSystem);

  // We have some objects of size smaller than 0.035 units, so we scale up the
  // whole world for a better behavior of the dynamic simulation.
  collisionSystem->SetInternalScale (10.0f);

  // Check whether the soft bodies are enabled or not
  isSoftBodyWorld = clp->GetBoolOption ("soft", true);

  // Load the soft body animation control plugin & factory
  if (isSoftBodyWorld)
  {
    csRef<CS::Animation::iSoftBodyAnimationControlType> softBodyAnimationType =
      csLoadPlugin<CS::Animation::iSoftBodyAnimationControlType>
      (plugmgr, "crystalspace.dynamics.softanim");
    if (!softBodyAnimationType)
      return ReportError ("Could not load soft body animation for genmeshes plugin!");

    csRef<iGenMeshAnimationControlFactory> animationFactory =
      softBodyAnimationType->CreateAnimationControlFactory ();
    softBodyAnimationFactory =
      scfQueryInterface<CS::Animation::iSoftBodyAnimationControlFactory> (animationFactory);
  }

  // Load the ragdoll plugin
  ragdollManager = csLoadPlugin<CS::Animation::iSkeletonRagdollNodeManager>
    (plugmgr, "crystalspace.mesh.animesh.animnode.ragdoll");
  if (!ragdollManager)
    return ReportError ("Failed to locate ragdoll manager!");

  // Check which environment has to be loaded
  if (clp->GetBoolOption ("terrain", false))
    environment = ENVIRONMENT_TERRAIN;


  if (!collisionSystem)
    return ReportError ("No bullet system plugin!");

  return true;
}


bool Simple::Application ()
{
  // Default behavior from DemoApplication
  if (!DemoApplication::Application ())
    return false;

  // Find references to main objects
  /*debuggerManager =
    csQueryRegistry<CS::Debug::iDynamicsDebuggerManager> (GetObjectRegistry ());
  if (!debuggerManager)
    return ReportError ("Failed to locate dynamic's debug manager!");*/

  // Create the dynamic system
  collisionSector = collisionSystem->CreateCollisionSector ();
  if (!collisionSector) return ReportError ("Error creating collision sector!");
  physicalSector = scfQueryInterface<CS::Physics::iPhysicalSector> (collisionSector);

  // Set some linear and angular dampening in order to have a reduction of
  // the movements of the objects
  physicalSector->SetLinearDampener(0.1f);
  physicalSector->SetRollingDampener(0.1f);

  // Enable soft bodies
  if (isSoftBodyWorld)
    physicalSector->SetSoftBodyEnabled (true);


  //// Create the dynamic's debugger
  //dynamicsDebugger = debuggerManager->CreateDebugger ();
  //dynamicsDebugger->SetDynamicSystem (dynamicSystem);

  //// Don't display static colliders as the z-fighting with the original mesh
  //// is very ugly
  //dynamicsDebugger->SetStaticBodyMaterial (0);

  // Create the environment
  if (environment == ENVIRONMENT_WALLS)
  {
    CreateWalls (csVector3 (5));
    view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -3));
  }
  else
  {
    CreateTerrain ();
    view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 30, -3));
  }

  // Initialize the dynamics debugger
  //dynamicsDebugger->SetDebugSector (room);

  // Preload some meshes and materials
  iTextureWrapper* txt = loader->LoadTexture ("spark",
    "/lib/std/spark.png");
  if (!txt) return ReportError ("Error loading texture!");

  // Load the box mesh factory.
  boxFact = loader->LoadMeshObjectFactory ("/lib/std/sprite1");
  if (!boxFact) return ReportError ("Error loading mesh object factory!");

  // Double the size.
  csMatrix3 m; m *= .5;
  csReversibleTransform t = csReversibleTransform (m, csVector3 (0));
  boxFact->HardTransform (t);

  // Load the mesh factory.
  meshFact = loader->LoadMeshObjectFactory ("/varia/physmesh");
  if (!meshFact) return ReportError ("Error loading mesh object factory!");

  // Disable the camera manager
  cameraManager->SetCameraMode (CS::Utility::CAMERA_NO_MOVE);
  cameraManager->SetMouseMoveEnabled (false);

  // Initialize the camera
  UpdateCameraMode ();

  // Initialize the HUD manager
  hudManager->GetKeyDescriptions ()->Empty ();
  hudManager->GetKeyDescriptions ()->Push ("b: spawn a box");
  hudManager->GetKeyDescriptions ()->Push ("s: spawn a sphere");
  
  hudManager->GetKeyDescriptions ()->Push ("c: spawn a cylinder");
  hudManager->GetKeyDescriptions ()->Push ("a: spawn a capsule");
  hudManager->GetKeyDescriptions ()->Push ("n: spawn a cone");
 
  hudManager->GetKeyDescriptions ()->Push ("v: spawn a convex mesh");
  hudManager->GetKeyDescriptions ()->Push ("m: spawn a static concave mesh");
  
  hudManager->GetKeyDescriptions ()->Push ("q: spawn a compound body");
  hudManager->GetKeyDescriptions ()->Push ("j: spawn a joint with motor");
  
  hudManager->GetKeyDescriptions ()->Push ("h: spawn a chain");
  hudManager->GetKeyDescriptions ()->Push ("r: spawn a Frankie's ragdoll");

  if (isSoftBodyWorld)
  {
    hudManager->GetKeyDescriptions ()->Push ("y: spawn a rope");
    hudManager->GetKeyDescriptions ()->Push ("u: spawn a cloth");
    hudManager->GetKeyDescriptions ()->Push ("i: spawn a soft body");
  }
  hudManager->GetKeyDescriptions ()->Push ("SPACE: spawn random object");
  
  hudManager->GetKeyDescriptions ()->Push ("left mouse: fire!");
  hudManager->GetKeyDescriptions ()->Push ("right mouse: drag object");
  hudManager->GetKeyDescriptions ()->Push ("CTRL-x: cut selected object");
  hudManager->GetKeyDescriptions ()->Push ("CTRL-v: paste object");

  hudManager->GetKeyDescriptions ()->Push ("f: toggle camera modes");
  hudManager->GetKeyDescriptions ()->Push ("t: toggle all bodies dynamic/static");
  hudManager->GetKeyDescriptions ()->Push ("p: pause the simulation");
  hudManager->GetKeyDescriptions ()->Push ("o: toggle speed of simulation");
  hudManager->GetKeyDescriptions ()->Push ("d: toggle Bullet debug display");
  
  hudManager->GetKeyDescriptions ()->Push ("?: toggle display of collisions");
  hudManager->GetKeyDescriptions ()->Push ("g: toggle gravity");
  hudManager->GetKeyDescriptions ()->Push ("I: toggle autodisable");
  
#ifdef CS_HAVE_BULLET_SERIALIZER
  if (phys_engine_id == BULLET_ID)
    hudManager->GetKeyDescriptions ()->Push ("CTRL-s: save the dynamic world");
#endif
  /*
  if (phys_engine_id == BULLET_ID)
    hudManager->GetKeyDescriptions ()->Push ("CTRL-n: next environment");
  */
  
  hudManager->GetKeyDescriptions ()->Push ("CTRL-i: start profiling");
  hudManager->GetKeyDescriptions ()->Push ("CTRL-o: stop profiling");
  hudManager->GetKeyDescriptions ()->Push ("CTRL-p: dump profile");
  
  // Pre-load the animated mesh and the ragdoll animation node data
  
    LoadRagdoll ();

  // Run the application
  Run();

  return true;
}

void Simple::UpdateCameraMode ()
{
  switch (physicalCameraMode)
  {
    // The camera is controlled by a rigid body
  case CAMERA_DYNAMIC:
    {
      // Check if there is already a rigid body created for the 'kinematic' mode
      if (cameraBody)
      {
        cameraBody->SetState (CS::Physics::STATE_DYNAMIC);

        // Remove the attached camera (in this mode we want to control
        // the orientation of the camera, so we update the camera
        // position by ourselves)
        cameraBody->SetAttachedMovable (0);
      }

      // Create a new rigid body
      else
      {
        cameraBody = physicalSystem->CreateRigidBody ();
        cameraBody->SetDensity (1.0f);
        cameraBody->SetTransform (view->GetCamera ()->GetTransform ());
        csRef<CS::Collision::iColliderSphere> sphere = collisionSystem->CreateColliderSphere (0.8f);
        cameraBody->AddCollider (sphere, localTrans);
        cameraBody->SetElasticity (0.8f);
        cameraBody->SetFriction (10.0f);
        cameraBody->RebuildObject ();
        physicalSector->AddRigidBody (cameraBody);
      }

      break;
    }

    // The camera is free
  case CAMERA_FREE:
    {
      physicalSector->RemoveRigidBody (cameraBody);
      cameraBody = 0;

      // Update rotX, rotY, rotZ
      csQuaternion quaternion;
      quaternion.SetMatrix
        (((csReversibleTransform) view->GetCamera ()->GetTransform ()).GetT2O ());
      csVector3 eulerAngles = quaternion.GetEulerAngles ();
      rotX = eulerAngles.x;
      rotY = eulerAngles.y;
      rotZ = eulerAngles.z;

      // Update the display of the dynamics debugger
      //dynamicsDebugger->UpdateDisplay ();

      break;
    }

    // The camera is kinematic
  case CAMERA_KINEMATIC:
    {
      // Create a body
      cameraBody = physicalSystem->CreateRigidBody ();
      cameraBody->SetTransform (view->GetCamera ()->GetTransform ());

      csRef<CS::Collision::iColliderSphere> sphere = collisionSystem->CreateColliderSphere (0.8f);
      csOrthoTransform localTrans;
      cameraBody->AddCollider (sphere, localTrans);
      cameraBody->SetDensity (1.0f);
      cameraBody->SetElasticity (0.8f);
      cameraBody->SetFriction (10.0f);
      cameraBody->RebuildObject ();
      physicalSector->AddRigidBody (cameraBody);

      // Make it kinematic
      cameraBody->SetState (CS::Physics::STATE_KINEMATIC);

      // Attach the camera to the body so as to benefit of the default
      // kinematic callback
      cameraBody->SetAttachedMovable (view->GetCamera ()->QuerySceneNode ()->GetMovable ());

      break;
    }

  default:
    break;
  }
}

CS::Physics::iRigidBody* Simple::SpawnBox ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (boxFact, "box", room));

  // Create a body and attach the mesh.
  csRef<CS::Physics::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  rb->SetTransform (trans);
  rb->SetAttachedMovable (mesh->GetMovable ());

  // Create and attach a box collider.
  const csMatrix3 tm;
  const csVector3 tv (0);
  csOrthoTransform t (tm, tv);
  csVector3 size (0.4f, 0.8f, 0.4f); // This should be the same size as the mesh
  csRef<CS::Collision::iColliderBox> box = collisionSystem->CreateColliderBox (size);
  rb->AddCollider (box, localTrans);
  rb->SetDensity (1.0f);
  rb->SetElasticity (0.8f);
  rb->SetFriction (10.0f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return rb;
}