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
isSoftBodyWorld (true), environment (ENVIRONMENT_WALLS), solver (0),
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
  const csOrthoTransform& tc = cameraBody->GetTransform ();
  csVector3 a = tc.GetOrigin ();
  if (physicalCameraMode == CAMERA_DYNAMIC)
    view->GetCamera ()->GetTransform ().SetOrigin
    (cameraBody->GetTransform ().GetOrigin ());

  // Update the demo's state information
  hudManager->GetStateDescriptions ()->Empty ();
  csString txt;

  hudManager->GetStateDescriptions ()->Push (csString ("Physics engine: ") + phys_engine_name);

  txt.Format ("Collision objects count: %d", collisionSector->GetCollisionObjectCount ());
  hudManager->GetStateDescriptions ()->Push (txt);

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
      CS::Physics2::iSoftBody* softBody = physicalSector->GetSoftBody (i);
      csRef<CS::Physics2::Bullet2::iSoftBody> bulletSoftBody = 
        scfQueryInterface<CS::Physics2::Bullet2::iSoftBody> (softBody);
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
      case 3: SpawnConcaveMesh (); break;
      case 4: SpawnCylinder (); break;
      case 5: SpawnCapsule (); break;
      case 6: SpawnCompound (); break;
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
        CS::Physics2::iRigidBody* body = physicalSector->GetRigidBody (i);
        if (allStatic)
          body->SetState (CS::Physics2::STATE_STATIC);
        else
        {
          body->SetState (CS::Physics2::STATE_DYNAMIC);
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

      CS::Collision2::HitBeamResult hitResult =
        collisionSector->HitBeam (startBeam, endBeam);
      if (hitResult.hasHit
        && hitResult.object->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
      {
        // Remove the body and the mesh from the simulation, and put them in the clipboard
        clipboardBody = scfQueryInterface<CS::Physics2::iPhysicalBody> (hitResult.object);
        if (clipboardBody->GetBodyType () == CS::Physics2::BODY_RIGID)
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
      if (clipboardBody->GetBodyType () == CS::Physics2::BODY_RIGID)
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
    CS::Collision2::HitBeamResult hitResult =
      collisionSector->HitBeam (startBeam, endBeam);
    if (!hitResult.hasHit)
      return false;

    // Add a force at the point clicked
    if (hitResult.object->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
    {
      csVector3 force = endBeam - startBeam;
      force.Normalize ();
      force *= 2.0f;

      csRef<CS::Physics2::iPhysicalBody> physicalBody = scfQueryInterface<CS::Physics2::iPhysicalBody> (hitResult.object);
      if (physicalBody->GetBodyType () == CS::Physics2::BODY_RIGID)
      {
        // Check if the body hit is not static or kinematic
        csRef<CS::Physics2::iRigidBody> bulletBody =
          scfQueryInterface<CS::Physics2::iRigidBody> (hitResult.object);
        if (bulletBody->GetState () != CS::Physics2::STATE_DYNAMIC)
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
    CS::Collision2::HitBeamResult hitResult =
      collisionSector->HitBeam (startBeam, endBeam);
    if (!hitResult.hasHit)
      return false;

    // Check if we hit a rigid body
    if (hitResult.object->GetObjectType () == CS::Collision2::COLLISION_OBJECT_PHYSICAL)
    {
      csRef<CS::Physics2::iPhysicalBody> physicalBody = scfQueryInterface<CS::Physics2::iPhysicalBody> (hitResult.object);
      if (physicalBody->GetBodyType () == CS::Physics2::BODY_RIGID)
      {
        // Create a p2p joint at the point clicked
        dragJoint = physicalSystem->CreateRigidP2PJoint (hitResult.isect - hitResult.object->GetTransform ().GetOrigin ());
        dragJoint->Attach (physicalBody, NULL);

        dragging = true;
        dragDistance = (hitResult.isect - startBeam).Norm ();

        // Set some dampening on the rigid body to have a more stable dragging
        csRef<CS::Physics2::iRigidBody> bulletBody =
          scfQueryInterface<CS::Physics2::iRigidBody> (hitResult.object);
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
    csRef<CS::Physics2::iRigidBody> bulletBody =
      scfQueryInterface<CS::Physics2::iRigidBody> (dragJoint->GetAttachedBody (0));
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

  csBaseEventHandler::Initialize (GetObjectRegistry ());
  if (!RegisterQueue (GetObjectRegistry (), csevAllEvents (GetObjectRegistry ())))
    return ReportError ("Failed to set up event handler!");

  // Checking for choosen dynamic system
  csRef<iCommandLineParser> clp = csQueryRegistry<iCommandLineParser> (GetObjectRegistry ());
  phys_engine_name = clp->GetOption ("phys_engine");
  
  phys_engine_name = "Bullet";
  csRef<iPluginManager> plugmgr = 
    csQueryRegistry<iPluginManager> (GetObjectRegistry ());
  collisionSystem = csLoadPlugin<CS::Collision2::iCollisionSystem> (plugmgr, "crystalspace.physics.bullet2");
  physicalSystem = scfQueryInterface<CS::Physics2::iPhysicalSystem> (collisionSystem);

  // We have some objects of size smaller than 0.035 units, so we scale up the
  // whole world for a better behavior of the dynamic simulation.
  collisionSystem->SetInternalScale (10.0f);

  // Check whether the soft bodies are enabled or not
  isSoftBodyWorld = clp->GetBoolOption ("soft", true);

  // Load the soft body animation control plugin & factory
  if (isSoftBodyWorld)
  {
    csRef<CS::Physics2::iSoftBodyAnimationControlType> softBodyAnimationType =
      csLoadPlugin<CS::Physics2::iSoftBodyAnimationControlType>
      (plugmgr, "crystalspace.physics.softanim2");
    if (!softBodyAnimationType)
      return ReportError ("Could not load soft body animation for genmeshes plugin!");

    csRef<iGenMeshAnimationControlFactory> animationFactory =
      softBodyAnimationType->CreateAnimationControlFactory ();
    softBodyAnimationFactory =
      scfQueryInterface<CS::Physics2::iSoftBodyAnimationControlFactory> (animationFactory);
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
  physicalSector = scfQueryInterface<CS::Physics2::iPhysicalSector> (collisionSector);

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
        cameraBody->SetState (CS::Physics2::STATE_DYNAMIC);

        // Remove the attached camera (in this mode we want to control
        // the orientation of the camera, so we update the camera
        // position by ourselves)
        cameraBody->SetAttachedMovable (0);
      }

      // Create a new rigid body
      else
      {
        cameraBody = physicalSystem->CreateRigidBody ();
        cameraBody->SetDensity (0.3f);
        const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();
        csVector3 a = tc.GetOrigin ();
        cameraBody->SetTransform (tc);
        csRef<CS::Collision2::iColliderSphere> sphere = collisionSystem->CreateColliderSphere (0.8f);
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

      csRef<CS::Collision2::iColliderSphere> sphere = collisionSystem->CreateColliderSphere (0.8f);
      csOrthoTransform localTrans;
      cameraBody->AddCollider (sphere, localTrans);
      cameraBody->SetDensity (1.0f);
      cameraBody->SetElasticity (0.8f);
      cameraBody->SetFriction (10.0f);
      cameraBody->RebuildObject ();
      physicalSector->AddRigidBody (cameraBody);

      // Make it kinematic
      cameraBody->SetState (CS::Physics2::STATE_KINEMATIC);

      // Attach the camera to the body so as to benefit of the default
      // kinematic callback
      cameraBody->SetAttachedMovable (view->GetCamera ()->QuerySceneNode ()->GetMovable ());

      break;
    }

  default:
    break;
  }
}

CS::Physics2::iRigidBody* Simple::SpawnBox ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (boxFact, "box", room));
  meshes.Push (mesh);

  // Create a body and attach the mesh.
  csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  rb->SetTransform (trans);
  rb->SetAttachedMovable (mesh->GetMovable ());

  // Create and attach a box collider.
  csVector3 size (0.4f, 0.8f, 0.4f); // This should be the same size as the mesh
  csRef<CS::Collision2::iColliderBox> box = collisionSystem->CreateColliderBox (size);
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

CS::Physics2::iRigidBody* Simple::SpawnSphere ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the ball mesh factory.
  csRef<iMeshFactoryWrapper> ballFact = engine->CreateMeshFactory(
    "crystalspace.mesh.object.genmesh", "ballFact");
  if (!ballFact)
  {
    ReportError ("Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
    iGeneralFactoryState> (ballFact->GetMeshObjectFactory ());
  const float r (rand()%5/10. + .2);
  csVector3 radius (r, r, r);
  csEllipsoid ellips (csVector3 (0), radius);
  gmstate->GenerateSphere (ellips, 16);

  // We do a hardtransform here to make sure our sphere has an artificial
  // offset. That way we can test if the physics engine supports that.
  csMatrix3 m;
  csVector3 artificialOffset (0, .5, 0);
  csReversibleTransform t = csReversibleTransform (m, artificialOffset);
  ballFact->HardTransform (t);

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (ballFact, "ball", room));
  meshes.Push (mesh);

  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  rb->SetTransform (trans);
  rb->SetAttachedMovable (mesh->GetMovable ());

  // Create and attach a sphere collider.
  csRef<CS::Collision2::iColliderSphere> sphere = collisionSystem->CreateColliderSphere (r);
  trans = localTrans;
  trans.SetOrigin (artificialOffset);
  rb->AddCollider (sphere, trans);
  rb->SetDensity (1.0f);
  rb->SetElasticity (0.8f);
  rb->SetFriction (10.0f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return rb;
}

CS::Physics2::iRigidBody* Simple::SpawnCone ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  const float radius (rand() % 10 / 50. + .2);
  const float length (rand() % 3 / 50. + .7);

  // We do a hardtransform here to make sure our cylinder has an artificial
  // offset. That way we can test if the physics engine supports that.
  csVector3 artificialOffset (3, 3, 3);
  csReversibleTransform hardTransform (csYRotMatrix3 (PI/2.0), artificialOffset);
 
  // Create a body and attach the mesh.
  csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  trans.RotateThis (csXRotMatrix3 (PI / 5.0));
  rb->SetTransform (trans);

  // Create and attach a cone collider.
  csRef<CS::Collision2::iColliderCone> cone = collisionSystem->CreateColliderCone (length, radius);
  csMatrix3 m;
  csReversibleTransform t = csReversibleTransform (m, artificialOffset);
  rb->AddCollider (cone, t);
  rb->SetDensity (1.0f);
  rb->SetElasticity (0.8f);
  rb->SetFriction (10.0f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return rb;
}

CS::Physics2::iRigidBody* Simple::SpawnCylinder ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the cylinder mesh factory.
  csRef<iMeshFactoryWrapper> cylinderFact = engine->CreateMeshFactory(
    "crystalspace.mesh.object.genmesh", "cylinderFact");
  if (!cylinderFact)
  {
    ReportError ("Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
    iGeneralFactoryState> (cylinderFact->GetMeshObjectFactory ());
  const float radius (rand() % 10 / 50. + .2);
  const float length (rand() % 3 / 50. + .7);
  gmstate->GenerateCylinder (length, radius, 10);

  // We do a hardtransform here to make sure our cylinder has an artificial
  // offset. That way we can test if the physics engine supports that.
  csVector3 artificialOffset (3, 3, 3);
  csReversibleTransform hardTransform (csYRotMatrix3 (PI/2.0), artificialOffset);
  cylinderFact->HardTransform (hardTransform);

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
    cylinderFact, "cylinder", room));
  meshes.Push (mesh);

  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  trans.RotateThis (csXRotMatrix3 (PI / 5.0));
  rb->SetTransform (trans);
  rb->SetAttachedMovable (mesh->GetMovable ());

  // Create and attach a cone collider.
  csRef<CS::Collision2::iColliderCylinder> cyliner = collisionSystem->CreateColliderCylinder (length, radius);
  csMatrix3 m;
  csReversibleTransform t = csReversibleTransform (m, artificialOffset);
  rb->AddCollider (cyliner, t);
  rb->SetDensity (1.0f);
  rb->SetElasticity (0.8f);
  rb->SetFriction (10.0f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));
  
  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return rb;
}

CS::Physics2::iRigidBody* Simple::SpawnCapsule (float length, float radius)
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the capsule mesh factory.
  csRef<iMeshFactoryWrapper> capsuleFact = engine->CreateMeshFactory (
    "crystalspace.mesh.object.genmesh", "capsuleFact");
  if (!capsuleFact)
  {
    ReportError ("Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate =
    scfQueryInterface<iGeneralFactoryState> (capsuleFact->GetMeshObjectFactory ());
  gmstate->GenerateCapsule (length, radius, 10);
  capsuleFact->HardTransform (
    csReversibleTransform (csYRotMatrix3 (PI/2), csVector3 (0)));

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
    capsuleFact, "capsule", room));
  meshes.Push (mesh);
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  trans.RotateThis (csXRotMatrix3 (PI / 5.0));
  rb->SetTransform (trans);
  rb->SetAttachedMovable (mesh->GetMovable ());

  // Create and attach a cone collider.
  csRef<CS::Collision2::iColliderCylinder> cyliner = collisionSystem->CreateColliderCylinder (length, radius);
  rb->AddCollider (cyliner, localTrans);
  rb->SetDensity (1.0f);
  rb->SetElasticity (0.8f);
  rb->SetFriction (10.0f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return rb;
}

CS::Collision2::iCollisionObject* Simple::SpawnConcaveMesh ()
{
  // Find the 'star' mesh factory
  csRef<iMeshFactoryWrapper> starFact;
  starFact = engine->FindMeshFactory ("genstar");
  if (!starFact)
  {
    loader->Load ("/lib/std/star.xml");
    starFact = engine->FindMeshFactory ("genstar");
    if (!starFact)
    {
      ReportError ("Error loading %s!", CS::Quote::Single ("star.xml"));
      return NULL;
    }
  }

  // Use the camera transform.
  csOrthoTransform tc = view->GetCamera ()->GetTransform ();
  tc.SetOrigin (tc.This2Other (csVector3 (0, 0, 3)));

  // Create the mesh.
  csRef<iMeshWrapper> star = engine->CreateMeshWrapper (starFact, "star",
    room);
  meshes.Push (star);
  star->GetMovable ()->SetTransform (tc);
  star->GetMovable ()->UpdateMove ();

  csRef<CS::Collision2::iCollisionObject> co = collisionSystem->CreateCollisionObject ();
  csRef<CS::Collision2::iColliderConcaveMesh> starCollider = collisionSystem->CreateColliderConcaveMesh (star);
  co->AddCollider (starCollider, localTrans);
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 2));
  co->SetAttachedMovable (star->GetMovable ());
  co->SetTransform (trans);

  co->RebuildObject ();
  collisionSector->AddCollisionObject (co);
  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return co;
}

CS::Physics2::iRigidBody* Simple::SpawnConvexMesh ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh factory (a capsule in this example)
  csRef<iMeshFactoryWrapper> capsuleFact = engine->CreateMeshFactory(
    "crystalspace.mesh.object.genmesh", "capsuleFact");
  if (!capsuleFact)
  {
    ReportError ("Error creating mesh object factory!");
    return 0;
  }

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
    iGeneralFactoryState> (capsuleFact->GetMeshObjectFactory ());
  const float radius (rand() % 10 / 50. + .2);
  const float length (rand() % 3 / 50. + .7);
  gmstate->GenerateCapsule (length, radius, 10);
  capsuleFact->HardTransform (
    csReversibleTransform (csYRotMatrix3 (PI/2), csVector3 (0)));

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
    capsuleFact, "capsule", room));
  meshes.Push (mesh);
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  rb->SetTransform (trans);
  rb->SetAttachedMovable (mesh->GetMovable ());

  // Create and attach a cone collider.
  csRef<CS::Collision2::iColliderConvexMesh> cyliner = collisionSystem->CreateColliderConvexMesh (mesh);
  rb->AddCollider (cyliner, localTrans);
  rb->SetDensity (1.0f);
  rb->SetElasticity (0.8f);
  rb->SetFriction (10.0f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));
  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return rb;
}

CS::Physics2::iRigidBody* Simple::SpawnCompound ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (meshFact, "mesh", room));
  meshes.Push (mesh);

  // Create a body and attach the mesh.
  // Create a body and attach the mesh.
  csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  csOrthoTransform trans = tc;
  trans.SetOrigin (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 2));
  rb->SetTransform (trans);
  rb->SetAttachedMovable (mesh->GetMovable ());

  collisionSystem->DecomposeConcaveMesh (rb, mesh);
  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return rb;
}

CS::Physics2::iJoint* Simple::SpawnJointed ()
{
  // Create and position two rigid bodies
  // Already added to sector.
  CS::Physics2::iRigidBody* rb1 = SpawnBox ();
  csOrthoTransform trans = rb1->GetTransform ();
  trans.SetOrigin (trans.GetOrigin () + csVector3 (1.0f, 2.0f, 0.0f));
  rb1->SetTransform (trans);

  CS::Physics2::iRigidBody* rb2 = SpawnSphere ();
  trans = rb2->GetTransform ();
  csOrthoTransform jointTrans = trans;
  trans.SetOrigin (trans.GetOrigin () + csVector3 (0.0f, -0.5f, 0.0f));
  rb2->SetTransform (trans);
  rb2->SetState (CS::Physics2::STATE_STATIC);

#define SLIDE

#ifdef P2P
  csRef<CS::Physics2::iJoint> joint = physicalSystem->CreateRigidP2PJoint (csVector3(0.2f, -0.4f, -0.2f));
  joint->Attach (rb1, rb2);
#endif

#ifdef HINGE
  // Create a joint and attach the two bodies to it.
    csRef<CS::Physics2::iJoint> joint = physicalSystem->CreateRigidHingeJoint (csVector3(0.2f, -0.2f, 0.2f),
    PI, -PI, 1);
  // Add a motor to the joint
  joint->SetDesiredVelocity (csVector3 (0.0f, -0.5f, 0.0f));
  joint->SetMaxForce (csVector3 (0.0f, 1.0f, 0.0f));
  joint->Attach (rb1, NULL/* rb2, true*/);
#endif

#ifdef SLIDE
  csRef<CS::Physics2::iJoint> joint = physicalSystem->CreateRigidSlideJoint (jointTrans, -1.f, 1.f, 1.f, -1.f, 0);
  joint->Attach (rb2, rb1);
#endif

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();

  return joint;
}

void ConstraintJoint (CS::Physics2::iJoint* joint)
{
  // If the axis is constrained. You should set it with axis and pass min and max value to it.
  // Min = max means full constraint; min < max means move in the range. 
  // The translations are fully constrained.
  joint->SetTransConstraints (true, true, true);
  joint->SetMinimumDistance (csVector3 (0,0,0));
  joint->SetMaximumDistance (csVector3 (0,0,0));

  // The rotations are bounded
  joint->SetRotConstraints (true, true, true);
  joint->SetMinimumAngle (csVector3 (-PI/4.0, -PI/6.0, -PI/6.0));
  joint->SetMaximumAngle (csVector3 (PI/4.0, PI/6.0, PI/6.0));
}

void Simple::SpawnChain ()
{
  CS::Physics2::iRigidBody* rb1 = SpawnBox ();
  csOrthoTransform trans = rb1->GetTransform ();
  csVector3 initPos = trans.GetOrigin () + csVector3 (0.0f, 5.0f, 0.0f);
  trans.SetOrigin (initPos);
  rb1->SetState (CS::Physics2::STATE_STATIC);
  rb1->SetTransform (trans);

  csVector3 offset (0.0f, 1.3f, 0.0f);

  CS::Physics2::iRigidBody* rb2 = SpawnCapsule (0.4f, 0.3f);
  rb2->SetLinearVelocity (csVector3 (0.0f));
  rb2->SetAngularVelocity (csVector3 (0.0f));
  trans.SetOrigin (initPos - offset);
  trans.SetO2T (csXRotMatrix3 (PI / 2.0f));
  rb2->SetTransform (trans);

  CS::Physics2::iRigidBody* rb3 = SpawnBox ();
  rb3->SetLinearVelocity (csVector3 (0.0f));
  rb3->SetAngularVelocity (csVector3 (0.0f));
  trans.Identity ();
  trans.SetOrigin (initPos - 2.0f * offset);
  rb3->SetTransform (trans);

  CS::Physics2::iRigidBody* rb4 = SpawnCapsule (0.4f, 0.3f);
  rb4->SetLinearVelocity (csVector3 (0.0f));
  rb4->SetAngularVelocity (csVector3 (0.0f));
  trans.SetOrigin (initPos - 3.0f * offset);
  trans.SetO2T (csXRotMatrix3 (PI / 2.0f));
  rb4->SetTransform (trans);

  CS::Physics2::iRigidBody* rb5 = SpawnBox ();
  rb5->SetLinearVelocity (csVector3 (0.0f));
  rb5->SetAngularVelocity (csVector3 (0.0f));
  trans.Identity ();
  trans.SetOrigin (initPos - 4.0f * offset);
  rb5->SetTransform (trans);

  // Create joints and attach bodies.
  csOrthoTransform jointTransform;
  csRef<CS::Physics2::iJoint> joint;

  joint = physicalSystem->CreateJoint ();
  jointTransform.Identity ();
  jointTransform.SetOrigin (initPos - csVector3 (0.0f, 0.6f, 0.0f));
  jointTransform = jointTransform * rb2->GetTransform ().GetInverse ();
  joint->SetTransform (jointTransform);
  joint->Attach (rb1, rb2, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = physicalSystem->CreateJoint ();
  jointTransform.Identity ();
  jointTransform.SetOrigin (initPos - csVector3 (0.0f, 0.6f, 0.0f) - offset);
  jointTransform = jointTransform * rb3->GetTransform ().GetInverse ();
  joint->SetTransform (jointTransform);
  joint->Attach (rb2, rb3, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = physicalSystem->CreateJoint ();
  jointTransform.Identity ();
  jointTransform.SetOrigin (initPos - csVector3 (0.0f, 0.6f, 0.0f) - 2.0f * offset);
  jointTransform = jointTransform * rb4->GetTransform ().GetInverse ();
  joint->SetTransform (jointTransform);
  joint->Attach (rb3, rb4, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  joint = physicalSystem->CreateJoint ();
  jointTransform.Identity ();
  jointTransform.SetOrigin (initPos - csVector3 (0.0f, 0.6f, 0.0f) - 3.0f * offset);
  jointTransform = jointTransform * rb5->GetTransform ().GetInverse ();
  joint->SetTransform (jointTransform);
  joint->Attach (rb4, rb5, false);
  ConstraintJoint (joint);
  joint->RebuildJoint ();

  // Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();
}

void Simple::LoadRagdoll ()
{
  // Load animesh factory
  csLoadResult rc = loader->Load ("/lib/frankie/frankie.xml");
  if (!rc.success)
  {
    ReportError ("Can't load Frankie!");
    return;
  }

  csRef<iMeshFactoryWrapper> meshfact = engine->FindMeshFactory ("franky_frankie");
  if (!meshfact)
    return;

  csRef<CS::Mesh::iAnimatedMeshFactory> animeshFactory =
    scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
    (meshfact->GetMeshObjectFactory ());
  if (!animeshFactory)
  {
    ReportError ("Can't find Frankie's animesh factory!");
    return;
  }

  // Load bodymesh (animesh's physical properties)
  rc = loader->Load ("/lib/frankie/skelfrankie_body");
  if (!rc.success)
  {
    ReportError ("Can't load Frankie's body description!");
    return;
  }

  csRef<CS::Animation::iBodyManager> bodyManager =
    csQueryRegistry<CS::Animation::iBodyManager> (GetObjectRegistry ());
  CS::Animation::iBodySkeleton* bodySkeleton = bodyManager->FindBodySkeleton ("frankie_body");
  if (!bodySkeleton)
  {
    ReportError ("Can't find Frankie's body description!");
    return;
  }

  // Create bone chain
  CS::Animation::iBodyChain* chain = bodySkeleton->CreateBodyChain
    ("body_chain", animeshFactory->GetSkeletonFactory ()->FindBone ("Frankie_Main"));
  chain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("CTRL_Head"));
  chain->AddSubChain (animeshFactory->GetSkeletonFactory ()->FindBone ("Tail_8"));

  // Create ragdoll animation node factory
  csRef<CS::Animation::iSkeletonRagdollNodeFactory> ragdollFactory =
    ragdollManager->CreateAnimNodeFactory ("frankie_ragdoll");
  ragdollFactory->SetBodySkeleton (bodySkeleton);
  ragdollFactory->AddBodyChain (chain, CS::Animation::STATE_DYNAMIC);

  // Set the ragdoll anim node as the only node of the animation tree
  animeshFactory->GetSkeletonFactory ()->GetAnimationPacket ()
    ->SetAnimationRoot (ragdollFactory);
}

void Simple::SpawnRagdoll ()
{
  //TODO----------------------------------------------------------------
  // Load frankie's factory if not yet done
  //csRef<iMeshFactoryWrapper> meshfact =
  //  engine->FindMeshFactory ("franky_frankie");
  //if (!meshfact)
  //{
  //  LoadRagdoll ();
  //  meshfact = engine->FindMeshFactory ("franky_frankie");
  //}

  //if (!meshfact)
  //  return;

  //// Create animesh
  //ragdollMesh = engine->CreateMeshWrapper (meshfact, "Frankie",
  //  room, csVector3 (0, -4, 0));
  //csRef<CS::Mesh::iAnimatedMesh> animesh =
  //  scfQueryInterface<CS::Mesh::iAnimatedMesh> (ragdollMesh->GetMeshObject ());

  //// Close the eyes of Frankie as he is dead
  //csRef<CS::Mesh::iAnimatedMeshFactory> animeshFactory =
  //  scfQueryInterface<CS::Mesh::iAnimatedMeshFactory>
  //  (meshfact->GetMeshObjectFactory ());

  //animesh->SetMorphTargetWeight (animeshFactory->FindMorphTarget ("eyelids_closed"), 0.7f);

  //// Set the initial position of the body
  //const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();
  //ragdollMesh->QuerySceneNode ()->GetMovable ()->SetPosition (
  //  tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));

  //// Start the ragdoll animation node so that the rigid bodies of the bones are created
  //CS::Animation::iSkeletonAnimNode* root = animesh->GetSkeleton ()->GetAnimationPacket ()->
  //  GetAnimationRoot ();
  //csRef<CS::Animation::iSkeletonRagdollNode> ragdoll =
  //  scfQueryInterfaceSafe<CS::Animation::iSkeletonRagdollNode> (root);
  //ragdoll->SetDynamicSystem (dynamicSystem);
  //ragdoll->Play ();

  //// Fling the body.
  //for (uint i = 0; i < ragdoll->GetBoneCount (CS::Animation::STATE_DYNAMIC); i++)
  //{
  //  CS::Animation::BoneID boneID = ragdoll->GetBone (CS::Animation::STATE_DYNAMIC, i);
  //  iRigidBody* rb = ragdoll->GetBoneRigidBody (boneID);
  //  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  //  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 5, 0));
  //}

  //// Update the display of the dynamics debugger
  //dynamicsDebugger->UpdateDisplay ();
}

void Simple::SpawnRope ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Spawn a box
  csRef<CS::Physics2::iRigidBody> box = SpawnBox ();

  // First example using ropes defined by their extremities
#if 1
  // Spawn a first rope and attach it to the box
  csRef<CS::Physics2::iSoftBody> body = physicalSystem->CreateRope
    (tc.GetOrigin () + tc.GetT2O () * csVector3 (-2, 2, 0),
    tc.GetOrigin () + tc.GetT2O () * csVector3 (-0.2f, 0, 1), 20);
  body->SetMass (2.0f);
  body->SetRigidity (0.95f);
  body->AnchorVertex (0);
  body->AnchorVertex (body->GetVertexCount () - 1, box);
  body->RebuildObject ();
  physicalSector->AddSoftBody (body);

  // Spawn a second rope and attach it to the box
  body = physicalSystem->CreateRope
    (tc.GetOrigin () + tc.GetT2O () * csVector3 (2, 2, 0),
    tc.GetOrigin () + tc.GetT2O () * csVector3 (0.2f, 0, 1), 20);
  body->SetMass (2.0f);
  body->SetRigidity (0.95f);
  body->AnchorVertex (0);
  body->AnchorVertex (body->GetVertexCount () - 1, box);
  body->RebuildObject ();
  physicalSector->AddSoftBody (body);

  // Second example using ropes defined by the position of each of their vertices
#else
  // Spawn a first rope and attach it to the box
  {
    // Define the positions of the vertices
    size_t vertexCount = 10;
    CS_ALLOC_STACK_ARRAY(csVector3, nodes, vertexCount);
    nodes[0] = tc.GetOrigin () + tc.GetT2O () * csVector3 (-2, 2, 0);
    csVector3 step = (tc.GetT2O () * csVector3 (-0.2f, 0, 1) -
      tc.GetT2O () * csVector3 (-2, 2, 0)) / (((float) vertexCount) - 1);
    for (size_t i = 1; i < vertexCount; i++)
      nodes[i] = nodes[0] + ((int) (i % 2)) * csVector3 (-0.2f, 0, 0) + ((int) i) * step;

    // Create the soft body
    CS::Physics2::iSoftBody* body = physicalSystem->CreateRope
      (nodes, vertexCount);
    body->SetMass (2.0f);
    body->SetRigidity (0.95f);
    body->AnchorVertex (0);
    body->AnchorVertex (body->GetVertexCount () - 1, box);
    body->RebuildObject ();
    physicalSector->AddSoftBody (body);
  }

  // Spawn a second rope and attach it to the box
  {
    // Define the positions of the vertices
    size_t vertexCount = 10;
    CS_ALLOC_STACK_ARRAY(csVector3, nodes, vertexCount);
    nodes[0] = tc.GetOrigin () + tc.GetT2O () * csVector3 (2, 2, 0);
    csVector3 step = (tc.GetT2O () * csVector3 (0.2f, 0, 1) -
      tc.GetT2O () * csVector3 (2, 2, 0)) / (((float) vertexCount) - 1);
    for (size_t i = 1; i < vertexCount; i++)
      nodes[i] = nodes[0] + ((int) (i % 2)) * csVector3 (0.2f, 0, 0) + ((int) i) * step;

    // Create the soft body
    CS::Physics2::iSoftBody* body = bulletDynamicSystem->CreateRope
      (nodes, vertexCount);
    body->SetMass (2.0f);
    body->SetRigidity (0.95f);
    body->AnchorVertex (0);
    body->AnchorVertex (body->GetVertexCount () - 1, box);
    body->RebuildObject ();
    physicalSector->AddSoftBody (body);
  }
#endif
}

CS::Physics2::iSoftBody* Simple::SpawnCloth ()
{
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the cloth
  csRef<CS::Physics2::iSoftBody> body = physicalSystem->CreateCloth
    (tc.GetOrigin () + tc.GetT2O () * csVector3 (-2, 2, 1),
    tc.GetOrigin () + tc.GetT2O () * csVector3 (2, 2, 1),
    tc.GetOrigin () + tc.GetT2O () * csVector3 (-2, 0, 1),
    tc.GetOrigin () + tc.GetT2O () * csVector3 (2, 0, 1),
    10, 10, true);
  body->SetMass (5.0f);

  // Attach the two top corners
  body->AnchorVertex (0);
  body->AnchorVertex (9);
  body->RebuildObject ();
  physicalSector->AddSoftBody (body);

  // Create the cloth mesh factory
  csRef<iMeshFactoryWrapper> clothFact =
    CS::Physics2::SoftBodyHelper::CreateClothGenMeshFactory
    (GetObjectRegistry (), "clothFact", body);
  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
    (clothFact->GetMeshObjectFactory ());

  // Create the mesh
  gmstate->SetAnimationControlFactory (softBodyAnimationFactory);
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
    clothFact, "cloth_body", room));
  iMaterialWrapper* mat = CS::Material::MaterialBuilder::CreateColorMaterial
    (GetObjectRegistry (), "cloth", csColor4 (1.0f, 0.0f, 0.0f, 1.0f));
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Init the animation control for the animation of the genmesh
  csRef<iGeneralMeshState> meshState =
    scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ());
  csRef<CS::Physics2::iSoftBodyAnimationControl> animationControl =
    scfQueryInterface<CS::Physics2::iSoftBodyAnimationControl> (meshState->GetAnimationControl ());
  animationControl->SetSoftBody (body, true);

  return body;
}

CS::Physics2::iSoftBody* Simple::SpawnSoftBody ()
{
  // Create the ball mesh factory.
  csRef<iMeshFactoryWrapper> ballFact = engine->CreateMeshFactory(
    "crystalspace.mesh.object.genmesh", "ballFact");
  if (!ballFact)
  {
    ReportError ("Error creating mesh object factory!");
    return NULL;
  }

  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
    iGeneralFactoryState> (ballFact->GetMeshObjectFactory ());
  const float r (rand()%5/10. + .4);
  csVector3 radius (r, r, r);
  csEllipsoid ellips (csVector3 (0), radius);
  gmstate->GenerateSphere (ellips, 16);

  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the soft body
  csRef<CS::Physics2::iSoftBody> body = physicalSystem->CreateSoftBody(gmstate,
    csOrthoTransform (csMatrix3 (), csVector3 (0.0f, 0.0f, 1.0f)) * tc);
  // This would have worked too
  //iBulletSoftBody* body = bulletDynamicSystem->CreateSoftBody
  //  (gmstate->GetVertices (), gmstate->GetVertexCount (),
  //   gmstate->GetTriangles (), gmstate->GetTriangleCount ());
  body->SetDensity (0.1f);
  body->SetRigidity (0.8f);
  csRef<CS::Physics2::Bullet2::iSoftBody> bulletbody = 
    scfQueryInterface<CS::Physics2::Bullet2::iSoftBody> (body);
  bulletbody->SetBendingConstraint (true);

  // Create the mesh
  gmstate->SetAnimationControlFactory (softBodyAnimationFactory);
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (
    ballFact, "soft_body", room));
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");
  mesh->GetMeshObject ()->SetMaterialWrapper (mat);

  // Init the animation control for the animation of the genmesh
  csRef<iGeneralMeshState> meshState =
    scfQueryInterface<iGeneralMeshState> (mesh->GetMeshObject ());
  csRef<CS::Physics2::iSoftBodyAnimationControl> animationControl =
    scfQueryInterface<CS::Physics2::iSoftBodyAnimationControl> (meshState->GetAnimationControl ());
  animationControl->SetSoftBody (body);

  // Fling the body.
  body->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  body->RebuildObject ();
  physicalSector->AddSoftBody (body);

  // This would have worked too
  //for (size_t i = 0; i < body->GetVertexCount (); i++)
  //  body->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5), i);
  return body;
}

void Simple::CreateWalls (const csVector3& radius)
{
  // Default behavior from DemoApplication for the creation of the scene
  if (!DemoApplication::CreateRoom ())
    return;

  // First we make a primitive for our geometry.
  using namespace CS::Geometry;
  DensityTextureMapper mapper (0.3f);
  TesselatedBox box (csVector3 (-5, -5, -5), csVector3 (5, 5, 5));
  box.SetLevel (3);
  box.SetMapper (&mapper);
  box.SetFlags (Primitives::CS_PRIMBOX_INSIDE);

  // Now we make a factory and a mesh at once.
  walls = GeneralMeshBuilder::CreateFactoryAndMesh (
    engine, room, "walls", "walls_factory", &box);

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportWarning ("Could not load texture %s",
    CS::Quote::Single ("stone"));
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
  walls->GetMeshObject ()->SetMaterialWrapper (tm);

  csOrthoTransform t;

#if 0
  // Enabling this will work, however, mesh<->mesh collision
  // requires a lot of hand tuning. When this is enabled,
  // mesh objects created with 'm' will either sink through
  // the floor, or stick in it.

  // Some hints to make mesh<->mesh work better:
  //  * Decrease the time step. 1/300th of a second minimum
  //  * Slow down objects
  //  * Play with softness, cfm, etc.
#endif

#if 0
  // With ODE, mesh <-> plane doesn't work yet, so we will use boxes for each
  // wall for now
  for (int i = 0; i < walls_state->GetPolygonCount (); i++)
  {
    rb->AttachColliderPlane (walls_state->GetPolygonObjectPlane (i), 10, 0, 0);
  }
#endif

  csVector3 size (10.0f, 10.0f, 10.0f); // This should be the same size as the mesh.
  t.SetOrigin(csVector3(10.0f,0.0f,0.0f));

  // Just to make sure everything works we create half of the colliders
  // using dynsys->CreateCollider() and the other half using
  // dynsys->AttachColliderBox().
  csRef<CS::Collision2::iCollisionObject> co = collisionSystem->CreateCollisionObject ();
  csRef<CS::Collision2::iColliderBox> collider = collisionSystem->CreateColliderBox (size);
  co->AddCollider (collider, localTrans);
  co->SetTransform (t);
  co->RebuildObject ();
  collisionSector->AddCollisionObject (co);

  t.SetOrigin(csVector3(-10.0f, 0.0f, 0.0f));
  co = collisionSystem->CreateCollisionObject ();
  collider = collisionSystem->CreateColliderBox (size);
  co->AddCollider (collider, localTrans);
  co->SetTransform (t);
  co->RebuildObject ();
  collisionSector->AddCollisionObject (co);

  t.SetOrigin(csVector3(0.0f, 10.0f, 0.0f));
  co = collisionSystem->CreateCollisionObject ();
  collider = collisionSystem->CreateColliderBox (size);
  co->AddCollider (collider, localTrans);
  co->SetTransform (t);
  co->RebuildObject ();
  collisionSector->AddCollisionObject (co);

  // If we use the Bullet plugin, then use a plane collider for the floor
  // Also, soft bodies don't work well with planes, so use a box in this case
  csRef<CS::Collision2::iColliderPlane> planeCollider = 
    collisionSystem->CreateColliderPlane (csPlane3 (
    csVector3 (0.0f, 1.0f, 0.0f), -5.0f));
  csRef<CS::Physics2::iRigidBody> floorBody = physicalSystem->CreateRigidBody ();
  floorBody->AddCollider (planeCollider, localTrans);
  floorBody->SetFriction (10.0f);
  floorBody->SetElasticity (0.0f);
  floorBody->RebuildObject ();
  physicalSector->AddRigidBody (floorBody);
  //You should set the state after the body is added to a sector.
  floorBody->SetState (CS::Physics2::STATE_STATIC);

  t.SetOrigin(csVector3(0.0f, 0.0f, 10.0f));
  collider = collisionSystem->CreateColliderBox (size);
  csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
  rb->AddCollider (collider, localTrans);
  rb->SetTransform (t);
  rb->SetFriction (10.0f);
  rb->SetElasticity (0.0f);
  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);
  rb->SetState (CS::Physics2::STATE_STATIC);

  

  t.SetOrigin(csVector3(0.0f, 0.0f, -10.0f));
  collider = collisionSystem->CreateColliderBox (size);
  rb = physicalSystem->CreateRigidBody ();
  rb->AddCollider (collider, localTrans);
  rb->SetTransform (t);
  rb->RebuildObject ();
  physicalSector->AddRigidBody (rb);
  rb->SetState (CS::Physics2::STATE_STATIC);

  // Set our own lights
  room->SetDynamicAmbientLight (csColor (0.3, 0.3, 0.3));

  csRef<iLight> light;
  iLightList* lightList = room->GetLights ();
  lightList->RemoveAll ();

  light = engine->CreateLight(0, csVector3(10), 9000, csColor (1));
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 0, 0), 8, csColor (1, 0, 0));
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (-3, 0,  0), 8, csColor (0, 0, 1));
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 0, 3), 8, csColor (0, 1, 0));
  lightList->Add (light);

  light = engine->CreateLight (0, csVector3 (0, -3, 0), 8, csColor (1, 1, 0));
  lightList->Add (light);

  engine->Prepare ();
  CS::Lighting::SimpleStaticLighter::ShineLights (room, engine, 4);
}

void Simple::CreateTerrain ()
{
  printf ("Loading terrain...\n");

  // Load the level file
  csRef<iVFS> VFS (csQueryRegistry<iVFS> (GetObjectRegistry ()));
  VFS->ChDir ("/lev/terraini");

  if (!loader->LoadMapFile ("world"))
  {
    ReportError("Error couldn't load terrain level!");
    return;
  }

  // Setup the sector
  room = engine->FindSector ("room");
  view->GetCamera ()->SetSector (room);
  engine->Prepare ();

  // Find the terrain mesh
  csRef<iMeshWrapper> terrainWrapper = engine->FindMeshObject ("Terrain");
  if (!terrainWrapper)
  {
    ReportError("Error cannot find the terrain mesh!");
    return;
  }

  csRef<iTerrainSystem> terrain =
    scfQueryInterface<iTerrainSystem> (terrainWrapper->GetMeshObject ());
  if (!terrain)
  {
    ReportError("Error cannot find the terrain interface!");
    return;
  }

  // Create a terrain collider for each cell of the terrain
  csRef<CS::Collision2::iColliderTerrain> terrainCollider = collisionSystem->CreateColliderTerrain (terrain);
  csRef<CS::Collision2::iCollisionObject> co = collisionSystem->CreateCollisionObject ();
  co->AddCollider (terrainCollider, localTrans);
  co->RebuildObject ();
  collisionSector->AddCollisionObject (co);
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

int main (int argc, char* argv[])
{
  return Simple ().Main(argc, argv);
}
