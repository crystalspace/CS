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

#include "phystut.h"

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

// The global pointer to simple
Simple *simple;

Simple::Simple (iObjectRegistry* object_reg)
{
  Simple::object_reg = object_reg;
  objcnt=0;
  solver=0;
  disable=false;
}

Simple::~Simple ()
{
  if (dyn) dyn->RemoveSystem (dynSys);
}

void Simple::SetupFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();

  // Now rotate the camera according to keyboard state
  const float speed = elapsed_time / 1000.0;

  if (kbd->GetKeyState (CSKEY_RIGHT))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
      view->GetCamera()->GetTransform().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP)) {
      //avatar->GetMovable()->MovePosition(avatar->GetMovable()->GetTransform() * CS_VEC_FORWARD * 5 * speed);
      avatarbody->SetLinearVelocity (view->GetCamera()->GetTransform().GetT2O () * csVector3 (0, 0, 5));
  }
  if (kbd->GetKeyState (CSKEY_DOWN)) {
      //avatar->GetMovable()->MovePosition(avatar->GetMovable()->GetTransform() * CS_VEC_BACKWARD * 5 * speed);
      avatarbody->SetLinearVelocity (view->GetCamera()->GetTransform().GetT2O () * csVector3 (0, 0, -5));
  }


  // Take small steps.
  const float maxStep = 0.01f;
  float ta = 0;
  float tb = speed;
  int maxSteps=4;
  while (ta < speed && maxSteps)
  {
    if (tb - ta > maxStep)
      tb = ta + maxStep;

    dyn->Step (tb - ta);
    ta = tb;
    tb = speed;

    view->GetCamera()->GetTransform().SetOrigin(avatar->GetMovable()->GetTransform().GetOrigin());
    //avatarbody->SetTransform(view->GetCamera()->GetTransform());

    maxSteps--;
  }
//  dyn->Step(maxStep);

  view->GetCamera()->GetTransform().SetOrigin(avatar->GetMovable()->GetTransform().GetOrigin());
  //avatar->GetMovable()->SetTransform(view->GetCamera()->GetTransform());

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();

  // Write FPS and other info..
  if(!g3d->BeginDraw (CSDRAW_2DGRAPHICS)) return;
  if( speed != 0.0f) WriteShadow( 10, 400, g2d->FindRGB (255, 150, 100),"FPS: %.2f",1.0f/speed);
  WriteShadow( 10, 410, g2d->FindRGB (255, 150, 100),"%d Objects",objcnt);
  if(solver==0) WriteShadow( 10, 420, g2d->FindRGB (255, 150, 100),"Solver: WorldStep");
  else if(solver==1) WriteShadow( 10, 420, g2d->FindRGB (255, 150, 100),"Solver: StepFast");
  else if(solver==2) WriteShadow( 10, 420, g2d->FindRGB (255, 150, 100),"Solver: QuickStep");
  if(disable) WriteShadow( 10, 430, g2d->FindRGB (255, 150, 100),"AutoDisable ON");
}

void Simple::FinishFrame ()
{
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool Simple::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdProcess)
  {
    simple->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && csCommandEventHelper::GetCode(&ev) == cscmdFinalProcess)
  {
    simple->FinishFrame ();
    return true;
  }
  else if ((ev.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeDown))
  {
    if (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_SPACE)
    {
      if (rand()%2) CreateBox (); else CreateSphere ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'b')
    {
      CreateBox ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 's')
    {
      CreateSphere ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'm')
    {
      CreateMesh ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'j')
    {
      CreateJointed ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'g')
    { // Toggle gravity.
      dynSys->SetGravity (dynSys->GetGravity () == 0 ?
       csVector3 (0,-7,0) : csVector3 (0));
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == 'd')
    { // Toggle autodisable.
      dynSys->EnableAutoDisable (!dynSys->AutoDisableEnabled ());
      //dynSys->SetAutoDisableParams(1.5f,2.5f,6,0.0f);
      disable=dynSys->AutoDisableEnabled ();
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == '1')
    { // Toggle stepfast.
      csRef<iODEDynamicSystemState> osys= SCF_QUERY_INTERFACE (dynSys, iODEDynamicSystemState);
      osys->EnableStepFast (0);
      solver=0;
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == '2')
    { // Toggle stepfast.
      csRef<iODEDynamicSystemState> osys= SCF_QUERY_INTERFACE (dynSys, iODEDynamicSystemState);
      osys->EnableStepFast (1);
      solver=1;
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == '3')
    { // Toggle quickstep.
      csRef<iODEDynamicSystemState> osys= SCF_QUERY_INTERFACE (dynSys, iODEDynamicSystemState);
      //iODEDynamicSystemState osys=(iODEDynamicSystemState*)dynSys->QueryObject();
      osys->EnableQuickStep (1);
      solver=2;
      return true;
    }
    else if (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC)
    {
      csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
      if (q) q->GetEventOutlet()->Broadcast (cscmdQuit);
      return true;
    } 
  }
  else if ((ev.Type == csevKeyboard) && 
    (csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeUp) &&
    ((csKeyEventHelper::GetCookedCode (&ev) == CSKEY_DOWN) ||
    (csKeyEventHelper::GetCookedCode (&ev) == CSKEY_UP)))
  {
    avatarbody->SetLinearVelocity(csVector3 (0, 0, 0));
    avatarbody->SetAngularVelocity (csVector3 (0, 0, 0));
  }

  return false;
}

bool Simple::SimpleEventHandler (iEvent& ev)
{
  return simple->HandleEvent (ev);
}

bool Simple::Initialize ()
{
  if (!csInitializer::RequestPlugins (object_reg,
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_PLUGIN ("crystalspace.dynamics.ode", iDynamics),
    CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
    "Can't initialize plugins!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, SimpleEventHandler))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
    "Can't initialize event handler!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    return false;
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);
  if (vc == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
    "Can't find the virtual clock!");
    return false;
  }

  // Find the pointer to engine plugin
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (engine == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
    "No iEngine plugin!");
    return false;
  }

  loader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (loader == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "No iLoader plugin!");
    return false;
  }

  g3d = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (g3d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "No iGraphics3D plugin!");
    return false;
  }

  g2d = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (g2d == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "No iGraphics2D plugin!");
    return false;
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (kbd == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "No iKeyboardDriver plugin!");
    return false;
  }

  dyn = CS_QUERY_REGISTRY (object_reg, iDynamics);
  if (!dyn)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "No iDynamics plugin!");
    return false;
  }
  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error opening system!");
    return false;
  }

  csRef<iFontServer> fs = g3d->GetDriver2D()->GetFontServer ();
  if (fs)
    courierFont = fs->LoadFont (CSFONT_COURIER);
  else
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error getting FontServer!");
    return false;
  };

  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  cdsys = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.collisiondetection.opcode" , iCollideSystem);
  if (!cdsys)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error getting Collision Detection System!");
    return false;
  };
  
  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  walls = engine->CreateSectorWallsMesh (room, "walls");
  csRef<iThingState> ws =
    SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-5, -5, -5), csVector3 (5, 5, 5));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight (0, csVector3 (3, 0, 0), 8,
    csColor (1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (-3, 0,  0), 8,
    csColor (0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 0, 3), 8,
    csColor (0, 1, 0));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, -3, 0), 8,
    csColor (1, 1, 0));
  ll->Add (light);

  engine->Prepare ();

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -4.5));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  iTextureManager* txtmgr = g3d->GetTextureManager ();

  iTextureWrapper* txt = loader->LoadTexture ("spark",
    "/lib/std/spark.png", CS_TEXTURE_3D, txtmgr, true);
  if (txt == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error loading texture!");
    return false;
  }

  // Load the box mesh factory.
  boxFact = loader->LoadMeshObjectFactory ("/lib/std/sprite1");
  if (boxFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error loading mesh object factory!");
    return false;
  }
  // Double the size.
  csMatrix3 m; m *= .5;
  csReversibleTransform t = csReversibleTransform (m, csVector3 (0));
  boxFact->HardTransform (t);

  // Load the mesh factory.
  meshFact = loader->LoadMeshObjectFactory ("/varia/physmesh");
  if (meshFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error loading mesh object factory!");
    return false;
  }

  // Create the ball mesh factory.
  ballFact = engine->CreateMeshFactory("crystalspace.mesh.object.ball",
   "ballFact");
  if (ballFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error creating mesh object factory!");
    return false;
  }


  // Create the dynamic system.
  dynSys = dyn->CreateSystem ();
  if (dynSys == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
        "crystalspace.application.phystut",
        "Error creating dynamic system!");
    return false;
  }

  dynSys->SetGravity (csVector3 (0,-7,0));

  CreateWalls (csVector3 (5));

  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the avatar.
  avatar = engine->CreateMeshWrapper (boxFact, "box", room);

  // Create a body and attach the mesh.
  avatarbody = dynSys->CreateBody ();
  avatarbody->SetProperties (1, csVector3 (0), csMatrix3 ());
  avatarbody->SetPosition (tc.GetOrigin ());
  avatarbody->AttachMesh (avatar);

  // Create and attach a box collider.
  // const csMatrix3 tmm;
  // const csVector3 tvv (0);
  // csOrthoTransform tt (tmm, tvv);
  // csVector3 size (0.4f, 0.8f, 0.4f); // This should be same size as mesh.
  // avatarbody->AttachColliderBox (size, tt, 10, 1, 0.8f);
  avatarbody->AttachColliderSphere (1.5, csVector3 (0), 10, 1, 0.8f);

  return true;
}

iRigidBody* Simple::CreateBox (void)
{
  objcnt++;
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (boxFact, "box", room));

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (1, csVector3 (0), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  rb->AttachMesh (mesh);

  // Create and attach a box collider.
  const csMatrix3 tm;
  const csVector3 tv (0);
  csOrthoTransform t (tm, tv);
  csVector3 size (0.4f, 0.8f, 0.4f); // This should be the same size as the mesh.
  rb->AttachColliderBox (size, t, 10, 1, 0.8f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  return rb;
}

iRigidBody* Simple::CreateMesh (void)
{
  objcnt++;
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (meshFact, "mesh", room));

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (1, csVector3 (0), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 2));
  rb->AttachMesh (mesh);

  // Create and attach a trimesh collider.
  const csMatrix3 tm;
  const csVector3 tv (0);
  csOrthoTransform t (tm, tv);

  rb->AttachColliderMesh (mesh, t, 10, 1, 0.8f);
     
  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 5));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  return rb;
}

iRigidBody* Simple::CreateSphere (void)
{
  objcnt++;
  // Use the camera transform.
  const csOrthoTransform& tc = view->GetCamera ()->GetTransform ();


  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (ballFact, "ball", room));

  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName ("spark");

  // Set the ball mesh properties.
  csRef<iBallState> s (
    SCF_QUERY_INTERFACE (mesh->GetMeshObject (), iBallState));
  const float r (rand()%5/10. + .1);
  const csVector3 radius (r, r, r);
  s->SetRadius (radius.x, radius.y, radius.z);
  s->SetRimVertices (16);
  s->SetMaterialWrapper (mat);

  // Create a body and attach the mesh.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetProperties (radius.Norm()/2, csVector3 (0), csMatrix3 ());
  rb->SetPosition (tc.GetOrigin () + tc.GetT2O () * csVector3 (0, 0, 1));
  rb->AttachMesh (mesh);

  // Create and attach a sphere collider.
  rb->AttachColliderSphere (radius.Norm()/2, csVector3 (0), 10, 1, 0.8f);

  // Fling the body.
  rb->SetLinearVelocity (tc.GetT2O () * csVector3 (0, 0, 6));
  rb->SetAngularVelocity (tc.GetT2O () * csVector3 (5, 0, 0));

  return rb;
}

iJoint* Simple::CreateJointed (void)
{
  objcnt++;
  // Create and position objects.
  iRigidBody* rb1 = CreateBox();
  rb1->SetPosition (rb1->GetPosition () +
   rb1->GetOrientation () * csVector3 (-.5, 0, 0));
  iRigidBody* rb2 = CreateSphere();
  rb2->SetPosition (rb2->GetPosition () +
   rb2->GetOrientation () * csVector3 (.5, 0, 0));

  // Create a joint and attach bodies.
  csRef<iJoint> joint = dynSys->CreateJoint ();
  joint->Attach (rb1, rb2);

  // Constrain translation.
  joint->SetMinimumDistance (csVector3 (1, 1, 1));
  joint->SetMaximumDistance (csVector3 (1, 1, 1));
  joint->SetTransConstraints (true, true, true);

  // Constrain rotation.
  joint->SetMinimumAngle (csVector3 (0, 0, 0));
  joint->SetMaximumAngle (csVector3 (0, 0, 0));
  joint->SetRotConstraints (true, true, true);

  return joint;
}

iRigidBody* Simple::CreateWalls (const csVector3& radius)
{
  // Create a body for the room.
  csRef<iRigidBody> rb = dynSys->CreateBody ();
  rb->SetMoveCallback(0);
  rb->SetPosition (csVector3 (0));
  rb->MakeStatic ();

  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();

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
  dynSys->AttachColliderMesh (walls, t,10,1);
#endif
#if 0
  // mesh <-> plane doesn't work yet, so we will use boxes for each 
  // wall for now
  for(int i = 0; i < walls_state->GetPolygonCount(); i++)
  {
      rb->AttachColliderPlane(walls_state->GetPolygonObjectPlane(i), 10, 0, 0);
  }
#endif
#if 1
  csVector3 size (10.0f, 10.0f, 10.0f); // This should be the same size as the mesh.
  t.SetOrigin(csVector3(10.0f,0.0f,0.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);
  t.SetOrigin(csVector3(-10.0f,0.0f,0.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);
  t.SetOrigin(csVector3(0.0f,10.0f,0.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);
  t.SetOrigin(csVector3(0.0f,-10.0f,0.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);
  t.SetOrigin(csVector3(0.0f,0.0f,10.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);
  t.SetOrigin(csVector3(0.0f,0.0f,-10.0f));
  dynSys->AttachColliderBox (size, t, 10, 0);

#endif
  return rb;
}

void Simple::Start ()
{
  csDefaultRunLoop (object_reg);
}

void Simple::WriteShadow (int x,int y,int fg,const char *str,...) 
{
  csString buf;

  va_list arg;
  va_start (arg, str);
  buf.FormatV (str, arg);
  va_end (arg);

  Write (x+1, y-1, 0, -1, "%s", buf.GetData());
  Write (x, y, fg, -1, "%s", buf.GetData());
}

void Simple::Write(int x,int y,int fg,int bg,const char *str,...) 
{
  va_list arg;
  csString buf;

  va_start (arg,str);
  buf.FormatV (str, arg);
  va_end (arg);

  g2d->Write (courierFont, x, y, fg, bg, buf);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);

  simple = new Simple (object_reg);
  if (simple->Initialize ())
    simple->Start ();
  delete simple;

  csInitializer::DestroyApplication (object_reg);
  return 0;
}

