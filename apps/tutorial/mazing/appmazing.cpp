/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#include <cssysdef.h>
#include "appmazing.h"

CS_IMPLEMENT_APPLICATION

AppMazing::AppMazing() :
	csApplicationFramework(),
	player (this),
	maze (this),
	laser (this)
{
  SetApplicationName("mazing");
}

AppMazing::~AppMazing()
{
}

bool AppMazing::CreateFactories ()
{
  csRef<iGeneralFactoryState> fstate;

  //---------------------------------------------------------------------
  // Adversary factory.
  adversary_factory = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "adversary");
  if (!adversary_factory) return false;

  fstate = scfQueryInterface<iGeneralFactoryState> (
  	adversary_factory->GetMeshObjectFactory ());
  csEllipsoid ellips (
  	csVector3 (0, 0, 0),
	csVector3 (ADVERSARY_DIMENSION, ADVERSARY_DIMENSION,
		ADVERSARY_DIMENSION));
  fstate->GenerateSphere (ellips, 10);

  if (!loader->LoadTexture ("adversary_texture", "/lib/stdtex/misty.jpg"))
    return ReportError ("Error loading 'misty' texture!");
  iMaterialWrapper* adversary_material = engine->GetMaterialList ()
  	->FindByName ("adversary_texture");
  fstate->SetMaterialWrapper (adversary_material);

  //---------------------------------------------------------------------
  // Beam factory.
  laserbeam_factory = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.genmesh", "laserbeam");
  if (!laserbeam_factory) return false;

  fstate = scfQueryInterface<iGeneralFactoryState> (
  	laserbeam_factory->GetMeshObjectFactory ());
  csBox3 laser_box (
  	csVector3 (-LASER_WIDTH, -LASER_WIDTH, 0),
  	csVector3 (LASER_WIDTH, LASER_WIDTH, LASER_LENGTH));
  fstate->GenerateBox (laser_box);
  fstate->SetLighting (false);
  fstate->SetColor (csColor (1.0, 1.0, 1.0));
  // We don't want to hit the player against the laserbeam when it is
  // visible so we disable the collision detection mesh here.
  laserbeam_factory->GetMeshObjectFactory ()->GetObjectModel ()
  	->SetPolygonMeshColldet (0);

  if (!loader->LoadTexture ("laserbeam_texture", "/lib/stdtex/blobby.jpg"))
    return ReportError ("Error loading 'blobby' texture!");
  iMaterialWrapper* laserbeam_material = engine->GetMaterialList ()
  	->FindByName ("laserbeam_texture");
  fstate->SetMaterialWrapper (laserbeam_material);

  //---------------------------------------------------------------------
  // Beam object.
  csRef<iMeshWrapper> laserbeam = engine->CreateMeshWrapper (
  	laserbeam_factory, "laserbeam");
  if (!laserbeam)
    return ReportError ("Error creating laserbeam mesh!");
  // Set our laser beam to NOHITBEAM so that we can use HitBeam() methods
  // to find out what our laser hits without HitBeam() returning the
  // laser itself.
  laserbeam->GetFlags ().Set (CS_ENTITY_NOHITBEAM);
  laser.SetMeshWrapper (laserbeam);

  //---------------------------------------------------------------------
  // Explosion factory.
  explosion_factory = engine->CreateMeshFactory (
  	"crystalspace.mesh.object.particles", "explosion");
  if (!explosion_factory) return false;
  csRef<iParticlesFactoryState> pstate = scfQueryInterface<
  	iParticlesFactoryState> (explosion_factory->GetMeshObjectFactory ());

  if (!loader->LoadTexture ("explosion_texture", "/lib/std/spark.png"))
    return ReportError ("Error loading 'spark' texture!");
  iMaterialWrapper* explosion_material = engine->GetMaterialList ()
  	->FindByName ("explosion_texture");
  pstate->SetMaterial (explosion_material);
  pstate->SetParticleRadius (0.2);
  pstate->SetParticlesPerSecond (50);
  pstate->SetInitialParticleCount (10);
  pstate->SetPointEmitType ();
  pstate->SetRadialForceType (2.0, CS_PART_FALLOFF_LINEAR);
  pstate->SetForce (6.0);
  pstate->SetDiffusion (1.0);
  pstate->SetEmitTime (float (EXPLOSION_EMITTIME) / 1000.0);
  pstate->SetTimeToLive (float (EXPLOSION_PARTTIME) / 1000.0);
  pstate->SetTimeVariation (float (EXPLOSION_PARTVARTIME) / 1000.0);
  pstate->SetConstantColorMethod (csColor4 (1.0, 0.8, 0.5));
  pstate->SetMixMode (CS_FX_ADD);
  pstate->EnableZSort (false);

  return true;
}

bool AppMazing::CreateAdversary (int x, int y, int z)
{
  float sx = float (x) * ROOM_DIMENSION;
  float sy = float (y) * ROOM_DIMENSION;
  float sz = float (z) * ROOM_DIMENSION;
  csRef<iMeshWrapper> adversary = engine->CreateMeshWrapper (adversary_factory,
  	"adversary", maze.GetSector (x, y, z), csVector3 (sx, sy, sz));
  if (!adversary)
    return ReportError ("Couldn't create adversary mesh!");

  RoomCoordinate rc (x, y, z);
  Adversary* adv = new Adversary (this, adversary, rc);
  adversaries.Push (adv);
  adversary->QueryObject ()->ObjAdd ((iObject*)adv);
  adv->DecRef ();
  
  return true;
}

bool AppMazing::InitCollisionDetection ()
{
  csColliderHelper::InitializeCollisionWrappers (cdsys, engine, 0);
  return player.InitCollisionDetection ();
}

bool AppMazing::SetupGame ()
{
  if (!maze.CreateGeometry ())
    return ReportError("Error creating the geometry!");

  if (!CreateFactories ())
    return ReportError ("Error creating mesh factories!");

  engine->Prepare ();

  // Create a view.
  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (engine->FindSector ("room_0_0_0"));
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 0, 0));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  if (!InitCollisionDetection ())
    return false;

  if (!CreateAdversary (0, 0, 2)) return false;
  if (!CreateAdversary (1, 1, 1)) return false;
  if (!CreateAdversary (1, 0, 2)) return false;
  if (!CreateAdversary (2, 2, 2)) return false;
  return true;
}

void AppMazing::ExplodeAdversary (Adversary* adv)
{
  iMeshWrapper* mesh = adv->GetMesh ();
  StartExplosion (mesh->GetMovable ()->GetSectors ()->Get (0),
      	mesh->GetMovable ()->GetTransform ().GetOrigin ());
  engine->RemoveObject (mesh);
  adversaries.Delete (adv);
}

void AppMazing::StartExplosion (iSector* sector, const csVector3& pos)
{
  csRef<iMeshWrapper> explo = engine->CreateMeshWrapper (explosion_factory,
  	"explosion", sector, pos);
  if (!explo)
  {
    ReportError ("Error creating explosion mesh!");
    return;
  }
  explo->SetZBufMode (CS_ZBUF_TEST);
  Explosion exp (explo, EXPLOSION_TIME);
  explosions.Push (exp);
}

void AppMazing::HandleExplosions (csTicks elapsed_ticks)
{
  size_t i = 0;
  while (i < explosions.Length ())
  {
    if (explosions[i].Handle (elapsed_ticks)) i++;
    else
    {
      engine->RemoveObject (explosions[i].GetMesh ());
      explosions.DeleteIndex (i);
    }
  }
}

void AppMazing::ProcessFrame()
{
  // First we move the camera.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  float elapsed_seconds = float (elapsed_time) / 1000.0;

  // Handle the laser.
  laser.Handle (elapsed_time);

  // Handle explosions.
  HandleExplosions (elapsed_time);

  // Move the camera.
  player.MoveAndRotateCamera (elapsed_seconds);

  // Let all the adversaries think about what to do.
  size_t i;
  for (i = 0 ; i < adversaries.Length () ; i++)
    adversaries[i]->ThinkAndMove (elapsed_seconds);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  view->Draw ();
}

void AppMazing::FinishFrame()
{
  g3d->FinishDraw();
  g3d->Print(0);
}

bool AppMazing::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  if (csKeyEventHelper::GetEventType(&ev) == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    switch (code)
    {
      case CSKEY_ESC:
        {
          // The user pressed escape, so terminate the application.  The proper
	  // way to terminate a Crystal Space application is by broadcasting a
          // csevQuit event.  That will cause the main run loop to stop.  To do
          // so we retrieve the event queue from the object registry and then
	  // post the event.
          csRef<iEventQueue> q =
            csQueryRegistry<iEventQueue> (GetObjectRegistry());
          if (q.IsValid())
            q->GetEventOutlet()->Broadcast(csevQuit(GetObjectRegistry()));
        }
        return true;
      case 'e':
	player.StartMovement (csVector3 (0, 1, 0));
	return true;
      case 'q':
	player.StartMovement (csVector3 (0, -1, 0));
	return true;
      case 'a':
	player.StartMovement (csVector3 (-1, 0, 0));
	return true;
      case 'd':
	player.StartMovement (csVector3 (1, 0, 0));
	return true;
      case 'w':
	player.StartMovement (csVector3 (0, 0, 1));
	return true;
      case 's':
	player.StartMovement (csVector3 (0, 0, -1));
	return true;
      case CSKEY_UP:
	player.StartRotation (csVector3 (-1, 0, 0));
	return true;
      case CSKEY_DOWN:
	player.StartRotation (csVector3 (1, 0, 0));
	return true;
      case CSKEY_LEFT:
	player.StartRotation (csVector3 (0, -1, 0));
	return true;
      case CSKEY_RIGHT:
	player.StartRotation (csVector3 (0, 1, 0));
	return true;
      case ' ':
	laser.Start ();
	return true;
    }
  }
  return false;
}

bool AppMazing::OnInitialize(int argc, char* argv[])
{
  iObjectRegistry* r = GetObjectRegistry();

  // Load application-specific configuration file.
  if (!csInitializer::SetupConfigManager(r, 0, GetApplicationName()))
    return ReportError("Failed to initialize configuration manager!");

  // RequestPlugins() will load all plugins we specify.  In addition it will
  // also check if there are plugins that need to be loaded from the
  // configuration system (both the application configuration and CS or global
  // configurations).  It also supports specifying plugins on the command line
  // via the --plugin= option.
  if (!csInitializer::RequestPlugins(r,
	CS_REQUEST_VFS,
	CS_REQUEST_OPENGL3D,
	CS_REQUEST_ENGINE,
	CS_REQUEST_FONTSERVER,
	CS_REQUEST_IMAGELOADER,
	CS_REQUEST_LEVELLOADER,
        CS_REQUEST_PLUGIN ("crystalspace.collisiondetection.opcode",
		iCollideSystem),
	CS_REQUEST_REPORTER,
	CS_REQUEST_REPORTERLISTENER,
	CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  // "Warm up" the event handler so it can interact with the world
  csBaseEventHandler::Initialize(r);
 
  // Set up an event handler for the application.  Crystal Space is fully
  // event-driven.  Everything (except for this initialization) happens in
  // response to an event.
  if (!RegisterQueue (r, csevAllEvents(GetObjectRegistry())))
    return ReportError("Failed to set up event handler!");

  return true;
}

void AppMazing::OnExit()
{
}

bool AppMazing::Application()
{
  iObjectRegistry* r = GetObjectRegistry();

  // Open the main system. This will open all the previously loaded plugins
  // (i.e. all windows will be opened).
  if (!OpenApplication(r))
    return ReportError("Error opening system!");

  // Now get the pointer to various modules we need.  We fetch them from the
  // object registry.  The RequestPlugins() call we did earlier registered all
  // loaded plugins with the object registry.  It is also possible to load
  // plugins manually on-demand.
  g3d = csQueryRegistry<iGraphics3D> (r);
  if (!g3d)
    return ReportError("Failed to locate 3D renderer!");

  engine = csQueryRegistry<iEngine> (r);
  if (!engine)
    return ReportError("Failed to locate 3D engine!");

  loader = csQueryRegistry<iLoader> (r);
  if (!loader)
    return ReportError("Failed to locate the map loader!");

  cdsys = csQueryRegistry<iCollideSystem> (r);
  if (!cdsys)
    return ReportError("Failed to locate the collision detection system!");

  vc = csQueryRegistry<iVirtualClock> (r);
  if (!vc)
    return ReportError("Failed to locate the virtual clock!");

  // Setup game.
  if (!SetupGame ())
    return false;

  // Start the default run/event loop.  This will return only when some code,
  // such as OnKeyboard(), has asked the run loop to terminate.
  Run();

  return true;
}

int main(int argc, char** argv)
{
  csPrintf ("mazing version 1.0 by Jorrit Tyberghein.\n");

  /* Runs the application.  
   *
   * csApplicationRunner<> cares about creating an application instance 
   * which will perform initialization and event handling for the entire game. 
   *
   * The underlying csApplicationFramework also performs some core 
   * initialization.  It will set up the configuration manager, event queue, 
   * object registry, and much more.  The object registry is very important, 
   * and it is stored in your main application class (again, by 
   * csApplicationFramework). 
   */
  return csApplicationRunner<AppMazing>::Run (argc, argv);
}
