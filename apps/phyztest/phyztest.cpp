/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#include "cssysdef.h"
#include "cssys/sysfunc.h"
#include "apps/phyztest/phyztest.h"
#include "csengine/engine.h"
#include "cstool/initapp.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "ivaria/polymesh.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/eventq.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/virtclk.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/fontserv.h"
#include "imesh/sprite3d.h"
#include "imap/parser.h"
#include "igraphic/imageio.h"
#include "cstool/collider.h"
#include "cstool/csview.h"
#include "csutil/cmdhelp.h"
#include "csphyzik/phyziks.h"
#include "csgeom/math3d.h"
#include "csengine/meshobj.h"
#include "cstso.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"

// PHYZTEST DEMO
// hit del key to create a swinging chain
// hit tab key for a mass on a spring

//------------------------------------------------- We need the 3D engine -----

CS_IMPLEMENT_APPLICATION

#if !defined(CS_STATIC_LINKED)
SCF_REGISTER_STATIC_LIBRARY (engine)
#endif

//-----------------------------------------------------------------------------

// The physics world.  Main object for physics stuff
  ctWorld phyz_world;
Phyztest *System;

// data for mass on spring demo
iMeshWrapper *bot = NULL;
ctRigidBody *rb_bot = NULL;
  
// data for swinging chain demo
bool chain_added = false;

// holds all data needed to represent a swinging chain
class ChainLink
{
public:
  ChainLink (iMeshWrapper *psprt, ctRigidBody *prb, ctArticulatedBody *pab)
  {
    sprt = psprt;
    rb = prb;
    ab = pab;   
  }

  iMeshWrapper *sprt;         // mesh that represents a link
  ctRigidBody *rb;          // rigidbody object for this link
  ctArticulatedBody * ab;   // articulated body (jointed body) for this link
};

// increasing to more will cause instability for low frame-rates
#define NUM_LINKS 5

// all the links of the chain
ChainLink *chain[NUM_LINKS];

// create a rigidbody at pos with some preset mass and Inertia tensor
ctRigidBody *add_test_body (ctVector3 ppos)
{

  ctRigidBody *arb;

  arb = ctRigidBody::new_ctRigidBody ();
  arb->set_m (2.0);
  arb->set_pos (ctVector3 (ppos[0],ppos[1],ppos[2]));
  arb->calc_simple_I_tensor (.1,0.2,.1);
  return arb;
}

iMeshWrapper *add_test_mesh (iMeshFactoryWrapper *tmpl, iSector *aroom, iView *view)
{
  iMeshWrapper *tsprt;
  
  tsprt = view->GetEngine ()->CreateMeshWrapper (tmpl, NULL);
  tsprt->GetMovable ()->SetSector (aroom);
  csXScaleMatrix3 m (2);
  tsprt->GetMovable ()->SetTransform (m);
  tsprt->GetMovable ()->SetPosition (csVector3 (0, 0, 0));    // only matters for root in chain demo
  tsprt->GetMovable ()->UpdateMove ();

  iSprite3DState* state = SCF_QUERY_INTERFACE (tsprt->GetMeshObject (), iSprite3DState);
  state->SetAction ("default");
  state->DecRef ();
  tsprt->DecRef ();
  return tsprt;
}

//-------------- Phyztest

Phyztest::Phyztest ()
{
  view = NULL;
  engine = NULL;
  dynlight = NULL;
  motion_flags = 0;
  cdsys = NULL;
  courierFont = NULL;
  LevelLoader = NULL;
  myG2D = NULL;
  myG3D = NULL;
  kbd = NULL;
  room_collwrap = NULL;
  bot_sto = NULL;
  vc = NULL;
}

Phyztest::~Phyztest ()
{
  delete bot_sto;
  SCF_DEC_REF (vc);
  SCF_DEC_REF (room_collwrap);
  SCF_DEC_REF (cdsys);
  SCF_DEC_REF (view);
  SCF_DEC_REF (courierFont);
  SCF_DEC_REF (LevelLoader);
  SCF_DEC_REF (myG2D);
  SCF_DEC_REF (myG3D);
  SCF_DEC_REF (engine);
  SCF_DEC_REF (kbd);
}

void Phyztest::Report (int severity, const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);
  iReporter* rep = CS_QUERY_REGISTRY (System->object_reg, iReporter);
  if (rep)
  {
    rep->ReportV (severity, "crystalspace.application.phyztest", msg, arg);
    rep->DecRef ();
  }
  else
  {
    csPrintfV (msg, arg);
    csPrintf ("\n");
  }
  va_end (arg);
}

void Cleanup ()
{
  csPrintf ("Cleaning up...\n");
  iObjectRegistry* object_reg = System->object_reg;
  delete System; System = NULL;
  csInitializer::DestroyApplication (object_reg);
}

static bool PhyzEventHandler (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    System->SetupFrame ();
    return true;
  }
  else if (ev.Type == csevBroadcast && ev.Command.Code == cscmdFinalProcess)
  {
    System->FinishFrame ();
    return true;
  }
  else
  {
    return System ? System->HandleEvent (ev) : false;
  }
}

bool Phyztest::Initialize (int argc, const char* const argv[],
	const char *iConfigName)
{
  object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg) return false;

  if (!csInitializer::SetupConfigManager (object_reg, iConfigName))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't initialize app!");
    return false;
  }

  if (!csInitializer::RequestPlugins (object_reg,
  	CS_REQUEST_VFS,
	CS_REQUEST_SOFTWARE3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
	CS_REQUEST_END))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  if (!csInitializer::SetupEventHandler (object_reg, PhyzEventHandler))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Couldn't init app!");
    return false;
  }

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper::Help (object_reg);
    exit (0);
  }

  // The virtual clock.
  vc = CS_QUERY_REGISTRY (object_reg, iVirtualClock);

  // Find the pointer to engine plugin
  myG3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!myG3D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics3D plugin!");
    exit (-1);
  }

  myG2D = CS_QUERY_REGISTRY (object_reg, iGraphics2D);
  if (!myG2D)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iGraphics2D plugin!");
    exit (-1);
  }

  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine plugin!");
    exit (-1);
  }

  LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
  if (!LevelLoader)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iLoader plugin!");
    exit (-1);
  }

  kbd = CS_QUERY_REGISTRY (object_reg, iKeyboardDriver);
  if (!kbd)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iKeyboardDriver!");
    exit (-1);
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "Error opening system!");
    Cleanup ();
    exit (1);
  }

  iFontServer *fs = myG3D->GetDriver2D ()->GetFontServer ();
  if (fs)
    courierFont = fs->LoadFont (CSFONT_COURIER);
  else
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No font plugin!");
    Cleanup ();
    exit (1);
  }

  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  cdsys = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.collisiondetection.rapid",
  	iCollideSystem);
  plugin_mgr->DecRef ();

  // Some commercials...
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Phyztest Crystal Space Application version 0.1.");
  iTextureManager* txtmgr = myG3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");


  if (!LevelLoader->LoadLibraryFile ("/lib/std/library"))
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, "LIBRARY NOT LOADED!...");
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q)
    {
      q->GetEventOutlet ()->Broadcast (cscmdQuit);
      q->DecRef ();
    }
    return false;
  }
  LevelLoader->LoadTexture ("stone", "/lib/std/stone4.gif");
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");
 
  room = engine->CreateSector ("room");
  iMeshWrapper *wallmw = engine->CreateSectorWallsMesh (room,"walls");
  iThingState* walls = SCF_QUERY_INTERFACE (wallmw->GetMeshObject (), iThingState);
  csVector3 
    f1 (-5,  5,  5),
    f2 (5,  5,  5), 
    f3 (5, -5,  5), 
    f4 (-5, -5,  5), 
    b1 (-5,  5, -5),
    b2 (5,  5, -5), 
    b3 (5, -5, -5), 
    b4 (-5, -5, -5);

  iPolygon3D* p = walls->CreatePolygon ("back");
  p->SetMaterial (tm);
  p->CreateVertex (b4);
  p->CreateVertex (b3);
  p->CreateVertex (b2);
  p->CreateVertex (b1);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("front");
  p->SetMaterial (tm);
  p->CreateVertex (f1);
  p->CreateVertex (f2);
  p->CreateVertex (f3);
  p->CreateVertex (f4);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("top");
  p->SetMaterial (tm);
  p->CreateVertex (b1);
  p->CreateVertex (b2);
  p->CreateVertex (f2);
  p->CreateVertex (f1);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("right");
  p->SetMaterial (tm);
  p->CreateVertex (f2);
  p->CreateVertex (b2);
  p->CreateVertex (b3);
  p->CreateVertex (f3);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("left");
  p->SetMaterial (tm);
  p->CreateVertex (f1);
  p->CreateVertex (f4);
  p->CreateVertex (b4);
  p->CreateVertex (b1);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  p = walls->CreatePolygon ("bottom");
  p->SetMaterial (tm);
  p->CreateVertex (f4);
  p->CreateVertex (f3);
  p->CreateVertex (b3);
  p->CreateVertex (b4);
  p->SetTextureSpace (p->GetVertex (0), p->GetVertex (1), 3);

  walls->DecRef ();
  wallmw->DecRef ();

  iStatLight* light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (NULL, csVector3 (-3, -4, 0), 10, csColor (1, 0, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();
  light = engine->CreateLight (NULL, csVector3 (3, -4, 0), 10, csColor (0, 0, 1), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();
  light = engine->CreateLight (NULL, csVector3 (0, -4, -3), 10, csColor (0, 1, 0), false);
  ll->Add (light->QueryLight ());
  light->DecRef ();
  
  iMeshWrapper *mw = room->GetMeshes ()->Get (0);
  iPolygonMesh* mesh = SCF_QUERY_INTERFACE (mw->GetMeshObject (), iPolygonMesh);
  room_collwrap = new csColliderWrapper (mw->QueryObject (), cdsys, mesh);
  mesh->DecRef ();
  
  engine->Prepare ();

  Report (CS_REPORTER_SEVERITY_NOTIFY, "--------------------------------------");

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (engine, myG3D);
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, -4));
  view->SetRectangle (2, 2, myG2D->GetWidth () - 4, myG2D->GetHeight () - 4);

  txtmgr->SetPalette ();
  write_colour = txtmgr->FindRGB (255, 150, 100);
  return true;
}

void Phyztest::SetupFrame ()
{
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  int i;
  csMatrix3 m; 
  ctMatrix3 M;

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.) * (0.03 * 20);

  if (kbd->GetKeyState (CSKEY_RIGHT))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
  if (kbd->GetKeyState (CSKEY_LEFT))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
  if (kbd->GetKeyState (CSKEY_PGUP))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
  if (kbd->GetKeyState (CSKEY_PGDN))
    view->GetCamera ()->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
  if (kbd->GetKeyState (CSKEY_UP))
    view->GetCamera ()->Move (CS_VEC_FORWARD * 4.0f * speed);
  if (kbd->GetKeyState (CSKEY_DOWN))
    view->GetCamera ()->Move (CS_VEC_BACKWARD * 4.0f * speed);

  // add a chain
  if (kbd->GetKeyState (CSKEY_DEL) && !chain_added)
  {
    // Report (CS_REPORTER_SEVERITY_DEBUG, "adding chain");
    // use box template

    iMeshFactoryWrapper* bxtmpl = view->GetEngine ()->GetMeshFactories ()
      ->FindByName ("box");
    if (!bxtmpl)
    {  
      Report (CS_REPORTER_SEVERITY_NOTIFY, "couldn't load template 'box'");
      return;
    }

    // root of chain.  invisible (no mesh)
    // this body doesn't rotate or translate if it is rooted. 
    iMeshWrapper *sprt;
    ctArticulatedBody *ab_parent;
    ctArticulatedBody *ab_child;
    // each link of chain has a rigid body 
    ctRigidBody *rb = add_test_body (ctVector3 (0.0, 0.0, 0.0));
    // which is used in the creation of an articulated body 
    // (linked to others via a joint)
    ab_parent = new ctArticulatedBody (rb);
    // the world only needs to have a pointer to the root of the 
    // articulated body tree.
    phyz_world.add_entity (ab_parent);
    // make the root fixed to the world. can be non-rooted as well
    ab_parent->make_grounded (); 
  
    // add all the links that will be seen swinging.
    for (i = 0; i < NUM_LINKS; i++)
    {
      // position is irrelevent. 
      // It will be determined by root offset and joint angles
      rb = add_test_body (ctVector3 (0.0, 0.0, 0.0));  
      ab_child = new ctArticulatedBody (rb);
      // link this body to the previous one.  first 2 vectors are joint offsets, 
      // the 3rd is the line the joint revolves around
      ctVector3 joint_offset (0, -0.1, 0);
      ctVector3 joint_action (0, 0, 1);
      ab_parent->link_revolute (ab_child, joint_offset, joint_offset, joint_action);
  
      // make something to draw
      sprt = add_test_mesh (bxtmpl, room, view);
      chain[i] = new ChainLink (sprt, rb, ab_child);
      
      // this will be parent of next body
      ab_parent = ab_child;
    }
    
    // rotate them so we can see some action.
    chain[0]->ab->rotate_around_axis (degree_to_rad (80));
    //!me uncomment if you have a good frame-rate
    chain[2]->ab->rotate_around_axis (degree_to_rad (60));
    chain_added = true;
  }

  // simple mass on a spring demo

  if (kbd->GetKeyState (CSKEY_TAB))
  {
    if  (bot == NULL)
    {
      // add a mesh
      iMeshFactoryWrapper* tmpl = view->GetEngine ()->GetMeshFactories ()
      	->FindByName ("box");
      if (!tmpl)
      {     
	Report (CS_REPORTER_SEVERITY_NOTIFY, "couldn't load template 'box'");
	return;
      }

      bot = view->GetEngine ()->CreateMeshWrapper (tmpl, NULL);
      bot->GetMovable ()->SetSector (room);
      iSprite3DState* state = SCF_QUERY_INTERFACE (bot->GetMeshObject (), iSprite3DState);
      state->SetAction ("default");
      state->DecRef ();


      // add the rigidbody physics object
      rb_bot = ctRigidBody::new_ctRigidBody ();
      rb_bot->calc_simple_I_tensor (0.2, 0.4, 0.2);
      phyz_world.add_entity (rb_bot);
      bot_sto = new csRigidSpaceTimeObj (cdsys, bot, rb_bot);
      bot->DecRef ();
    }

    csXScaleMatrix3 m (10);
    bot->GetMovable ()->SetTransform (m);
    bot->GetMovable ()->SetPosition (csVector3 (0, 0, 0));
    bot->GetMovable ()->UpdateMove ();
    rb_bot->set_m (15.0);
    rb_bot->set_pos (ctVector3 (0.0, 0.0, 0));
    rb_bot->set_v (ctVector3 (0.4, 0.0, 0));

    // create a spring force object and add it to our test body
    ctSpringF *sf = new ctSpringF (rb_bot, ctVector3 (0, 0.2, 0) , NULL, ctVector3 (0,12, 0));
    sf->set_rest_length (2);
    sf->set_magnitude (50.0);
    rb_bot->add_force (sf);
    ctVector3 rotaxisz (0, 0, 1);
    ctVector3 rotaxisy (0, 1, 0);
 
    rb_bot->rotate_around_line (rotaxisy, degree_to_rad (45));  
    rb_bot->rotate_around_line (rotaxisz, degree_to_rad (60));
    rb_bot->set_angular_v (ctVector3 (1.0, 0, 0));
  }
  
  // Move the dynamic light around.
  /*  angle += elapsed_time * 0.4 / 1000.;
      while (angle >= 2.*3.1415926) angle -= 2.*3.1415926;
      dynlight->Move (room, cos (angle)*3, 17, sin (angle)*3);
      dynlight->Setup ();
  */

  // evolve the physics world by time step.  Slowed down by 4x due to speed of demo objects
  //!me phyz_world.evolve (0, 0.25*elapsed_time / 1000.0);  //!me .25 needed to balance test samples..
  csRigidSpaceTimeObj::evolve_system (0, 0.25*elapsed_time / 1000.0, &phyz_world, engine);

  // Add a boost
  if (rb_bot && kbd->GetKeyState (CSKEY_ENTER))
    rb_bot->apply_impulse (ctVector3 (0,1,0), ctVector3 (0,10,0));

  // evolve the physics world by time step.  
  // Slowed down by 4x due to speed of demo objects
  // !me .25 needed to balance test samples..
  // !me phyz_world.evolve (0, 0.25*elapsed_time / 1000.0);  

  csRigidSpaceTimeObj::evolve_system
    (0, 0.25*elapsed_time / 1000.0, &phyz_world, engine);



  // if we have a spring and mass demo started
  if (bot)
  {
    csVector3 new_p = rb_bot->get_pos ();
    iLight* lights[2];
    int num_lights = engine->GetNearbyLights (room, new_p, 
		       CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, lights, 2);
    bot->UpdateLighting (lights, num_lights);  
  }
 
  // if we have a swinging chain demo started
  if (chain_added == true)
  {
    iLight* lights[2];
    csVector3 new_p;
    int num_lights;

    // update position and orientation of all meshes for the chain.
    // queries the physics world for rigidbody data then sets the meshes
    // properties accordingly
    for (i = 0; i < NUM_LINKS; i++)
    {
      if (chain[i] != NULL)
      {
        //  get the position of this link
        new_p = chain[i]->rb->get_pos ();
	//  Report (CS_REPORTER_SEVERITY_DEBUG, "chain pos %d = %f, %f, %f",
	//            i, new_p.x, new_p.y, new_p.z);
        chain[i]->sprt->GetMovable ()->SetPosition (new_p);
        
        M = chain[i]->rb->get_R ();   // get orientation for this link
        // ctMatrix3 and csMatrix3 not directly compatable yet
        m.Set (M[0][0], M[0][1], M[0][2],
               M[1][0], M[1][1], M[1][2],
               M[2][0], M[2][1], M[2][2]);    // set orientation of mesh
        csMatrix3 M_scale;   // chain is half size of box
        M_scale.Identity ();
        M_scale *= 0.5;
        m *= M_scale;
        chain[i]->sprt->GetMovable ()->SetTransform (m);
	chain[i]->sprt->GetMovable ()->UpdateMove ();

        num_lights = engine->GetNearbyLights (room, new_p, 
			   CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, lights, 2);
        chain[i]->sprt->UpdateLighting (lights, num_lights); 
      }
    }
  }

  // Tell 3D driver we're going to display 3D things.
  if (!myG3D->BeginDraw (CSDRAW_3DGRAPHICS)) return;

  view->Draw ();

  if (rb_bot && myG3D->BeginDraw (CSDRAW_2DGRAPHICS))
  {
    ctVector3 p = rb_bot->get_pos ();
    ctVector3 F = rb_bot->get_F ();
    ctVector3 av = rb_bot->get_angular_v ();
    ctVector3 lv = rb_bot->get_v ();
    ctVector3 T = rb_bot->get_torque ();
    ctVector3 P = rb_bot->get_P ();
    ctVector3 L = rb_bot->get_angular_P ();
    WriteShadow (ALIGN_LEFT, 10, 10, write_colour, 
		 "pos: %.2f, %.2f, %.2f", p.x, p.y, p.z);
    WriteShadow (ALIGN_LEFT, 10, 20, write_colour, 
		 "Frc: %.2f, %.2f, %.2f", F.x, F.y, F.z);
    WriteShadow (ALIGN_LEFT, 10, 30, write_colour, 
		 "Tqe: %.2f, %.2f, %.2f", T.x, T.y, T.z);
    WriteShadow (ALIGN_LEFT, 10, 40, write_colour, 
		 "  P: %.2f, %.2f, %.2f", P.x, P.y, P.z);
    WriteShadow (ALIGN_LEFT, 10, 50, write_colour, 
		 " lv: %.2f, %.2f, %.2f", lv.x, lv.y, lv.z);
    WriteShadow (ALIGN_LEFT, 10, 60, write_colour, 
		 "  L: %.2f, %.2f, %.2f", L.x, L.y, L.z);
    WriteShadow (ALIGN_LEFT, 10, 70, write_colour, 
		 " av: %.2f, %.2f, %.2f", av.x, av.y, av.z);

  }
  // Add in some help text.
  WriteShadow (ALIGN_LEFT,10, 100, write_colour,"Press the <DEL> key to start");
  WriteShadow (ALIGN_LEFT,10, 110, write_colour,"the chain object");
  WriteShadow (ALIGN_LEFT,10, 120, write_colour,"Press the <TAB> key to start");
  WriteShadow (ALIGN_LEFT,10, 130, write_colour,"the spring object");
  WriteShadow (ALIGN_LEFT,10, 140, write_colour,"Press the <ENTER> key to add");
  WriteShadow (ALIGN_LEFT,10, 150, write_colour,"an impulse to the spring object");
}

void Phyztest::FinishFrame ()
{
  myG3D->FinishDraw ();
  myG3D->Print (NULL);
}

bool Phyztest::HandleEvent (iEvent &Event)
{
  if ((Event.Type == csevKeyDown) && (Event.Key.Code == CSKEY_ESC))
  {
    iEventQueue* q = CS_QUERY_REGISTRY (object_reg, iEventQueue);
    if (q)
    {
      q->GetEventOutlet ()->Broadcast (cscmdQuit);
      q->DecRef ();
    }
    return true;
  }
  if ((Event.Type == csevBroadcast) && 
      (Event.Command.Code == cscmdContextResize))
    view->GetCamera ()->SetPerspectiveCenter (myG3D->GetWidth ()/2, myG3D->GetHeight ()/2);
  return false;
}
void Phyztest::Write (int align,int x,int y,int fg,int bg,char *str,...)
{
  va_list arg;
  char b[256], *buf = b;

  va_start (arg,str);
  int l = vsprintf (buf, str, arg);
  va_end (arg);

  if (align != ALIGN_LEFT)
  {
    int rb = 0;

    if (align == ALIGN_CENTER)
    {
      int where;
      sscanf (buf, "%d%n", &rb,&where);
      buf += where + 1;
      l -= where + 1;
    }

    int w, h;
    courierFont->GetDimensions (buf, w, h);

    switch (align)
    {
    case ALIGN_RIGHT:  x -= w; break;
    case ALIGN_CENTER: x = (x + rb - w) / 2; break;
    }
  }

  myG2D->Write (courierFont, x, y, fg, bg, buf);
}

void Phyztest::WriteShadow (int align,int x,int y,int fg,char *str,...)
{
  char buf[256];

  va_list arg;
  va_start (arg, str);
  vsprintf (buf, str, arg);
  va_end (arg);

  Write (align, x+1, y-1, 0, -1, buf);
  Write (align, x, y, fg, -1, buf);
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // add gravity to the world.  enviro forces affect all bodies in the world
  //ctGravityF *gf = new ctGravityF (9.81 / M_PER_WORLDUNIT);
  ctGravityF *gf = new ctGravityF (2.0 / M_PER_WORLDUNIT);
  phyz_world.add_enviro_force (gf);
  // add air resistance
  ctAirResistanceF *af = new ctAirResistanceF ();
  phyz_world.add_enviro_force (af);

  // register collision detection catastrophe manager
  ctLameCollisionCatastrophe *cdm = new ctLameCollisionCatastrophe ();
  phyz_world.register_catastrophe_manager (cdm);

  // Create our main class.
  System = new Phyztest ();

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!System->Initialize (argc, argv, NULL))
  {
    System->Report (CS_REPORTER_SEVERITY_ERROR, "Error initializing system!");
    Cleanup ();
    exit (1);
  }

  // Main loop.
  csDefaultRunLoop(System->object_reg);

  // Cleanup.
  Cleanup ();

  return 0;
}
