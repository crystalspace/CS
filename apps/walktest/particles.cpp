/*
    Copyright (C) 2008 by Jorrit Tyberghein

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
#include "csutil/scanstr.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/sector.h"
#include "imesh/object.h"
#include "imesh/emit.h"
#include "imesh/particles.h"
#include "iengine/camera.h"
#include "iutil/plugin.h"

#include "walktest.h"
#include "particles.h"
#include "splitview.h"

//===========================================================================
// Demo particle system (rain).
//===========================================================================
static void add_particles_rain (WalkTest* Sys, iSector* sector, char* matname, int num,
    float speed, bool do_camera)
{
  iEngine* engine = Sys->Engine;
  // First check if the material exists.
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Can't find material %s in memory!", CS::Quote::Single (matname));
    return;
  }

  csBox3 bbox;
  if (do_camera)
    bbox.Set (-5, -5, -5, 5, 5, 5);
  else
    sector->CalculateSectorBBox (bbox, true);

  csRef<iMeshFactoryWrapper> mfw = engine->CreateMeshFactory (
      "crystalspace.mesh.object.particles", "rain");
  if (!mfw) return;

  csRef<iMeshWrapper> exp = engine->CreateMeshWrapper (mfw, "custom rain",
	sector, csVector3 (0, 0, 0));

  if (do_camera)
  {
    iEngine* e = Sys->Engine;
    CS::Graphics::RenderPriority c = e->GetAlphaRenderPriority ();
    exp->GetFlags ().Set (CS_ENTITY_CAMERA);
    exp->SetRenderPriority (c);
  }
  exp->SetZBufMode(CS_ZBUF_TEST);
  exp->GetMeshObject()->SetMixMode (CS_FX_ADD);
  exp->GetMeshObject()->SetMaterialWrapper (mat);

  csRef<iParticleBuiltinEmitterFactory> emit_factory = 
      csLoadPluginCheck<iParticleBuiltinEmitterFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.emitter", false);
  csRef<iParticleBuiltinEffectorFactory> eff_factory = 
      csLoadPluginCheck<iParticleBuiltinEffectorFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.effector", false);

  csRef<iParticleBuiltinEmitterBox> boxemit = emit_factory->CreateBox ();
  // Time to live depends on height of sector.
  float velocity = 2.84f * speed / 2.0f;
  float seconds_to_live = (bbox.MaxY () - bbox.MinY ()) / velocity;
  csBox3 emit_bbox = bbox;
  emit_bbox.SetMin (1, emit_bbox.MaxY ());
  boxemit->SetBox (emit_bbox);
  boxemit->SetParticlePlacement (CS_PARTICLE_BUILTIN_VOLUME);
  boxemit->SetEmissionRate (float (num) / seconds_to_live);
  boxemit->SetInitialMass (5.0f, 7.5f);
  boxemit->SetUniformVelocity (true);
  boxemit->SetInitialTTL (seconds_to_live, seconds_to_live);
  boxemit->SetInitialVelocity (csVector3 (0, -velocity, 0),
      csVector3 (0));

  csRef<iParticleBuiltinEffectorLinColor> lincol = eff_factory->
    CreateLinColor ();
  lincol->AddColor (csColor4 (.25,.25,.25,1), seconds_to_live);

  csRef<iParticleSystem> partstate =
  	scfQueryInterface<iParticleSystem> (exp->GetMeshObject ());
  partstate->SetMinBoundingBox (bbox);
  partstate->SetParticleSize (csVector2 (0.3f/50.0f, 0.3f));
  partstate->SetParticleRenderOrientation (CS_PARTICLE_ORIENT_COMMON);
  partstate->SetCommonDirection (csVector3 (0, 1, 0));
  partstate->AddEmitter (boxemit);
  partstate->AddEffector (lincol);
}

//===========================================================================
// Demo particle system (snow).
//===========================================================================
static void add_particles_snow (WalkTest* Sys, iSector* sector, char* matname,
    int num, float speed)
{
  iEngine* engine = Sys->Engine;
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->Engine->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material %s in memory!",
		 CS::Quote::Single (matname));
    return;
  }

  csBox3 bbox;
  sector->CalculateSectorBBox (bbox, true);

  csRef<iMeshFactoryWrapper> mfw = engine->CreateMeshFactory (
      "crystalspace.mesh.object.particles", "snow");
  if (!mfw) return;

  csRef<iMeshWrapper> exp = engine->CreateMeshWrapper (mfw, "custom snow",
	sector, csVector3 (0, 0, 0));

  exp->SetZBufMode(CS_ZBUF_TEST);
  exp->GetMeshObject()->SetMixMode (CS_FX_ADD);
  exp->GetMeshObject()->SetMaterialWrapper (mat);

  csRef<iParticleBuiltinEmitterFactory> emit_factory = 
      csLoadPluginCheck<iParticleBuiltinEmitterFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.emitter", false);
  csRef<iParticleBuiltinEffectorFactory> eff_factory = 
      csLoadPluginCheck<iParticleBuiltinEffectorFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.effector", false);

  csRef<iParticleBuiltinEmitterBox> boxemit = emit_factory->CreateBox ();
  // Time to live depends on height of sector.
  float velocity = 2.0f * speed / 2.0f;
  float seconds_to_live = (bbox.MaxY () - bbox.MinY ()) / velocity;
  csBox3 emit_bbox = bbox;
  emit_bbox.SetMin (1, emit_bbox.MaxY ());
  boxemit->SetBox (emit_bbox);
  boxemit->SetParticlePlacement (CS_PARTICLE_BUILTIN_VOLUME);
  boxemit->SetEmissionRate (float (num) / seconds_to_live);
  boxemit->SetInitialMass (5.0f, 7.5f);
  boxemit->SetUniformVelocity (true);
  boxemit->SetInitialTTL (seconds_to_live, seconds_to_live);
  boxemit->SetInitialVelocity (csVector3 (0, -velocity, 0),
      csVector3 (0));

  csRef<iParticleBuiltinEffectorLinColor> lincol = eff_factory->
    CreateLinColor ();
  lincol->AddColor (csColor4 (.25,.25,.25,1), seconds_to_live);

  csRef<iParticleBuiltinEffectorForce> force = eff_factory->
    CreateForce ();
  force->SetRandomAcceleration (csVector3 (1.5f, 0.0f, 1.5f));

  csRef<iParticleSystem> partstate =
  	scfQueryInterface<iParticleSystem> (exp->GetMeshObject ());
  partstate->SetMinBoundingBox (bbox);
  partstate->SetParticleSize (csVector2 (0.07f, 0.07f));
  partstate->AddEmitter (boxemit);
  partstate->AddEffector (lincol);
  partstate->AddEffector (force);
}

//===========================================================================
// Demo particle system (fire).
//===========================================================================
static void add_particles_fire (WalkTest* Sys, iSector* sector, char* matname, int num,
	const csVector3& origin)
{
  iEngine* engine = Sys->Engine;

  // First check if the material exists.
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material %s in memory!",
		 CS::Quote::Single (matname));
    return;
  }

  csRef<iMeshFactoryWrapper> mfw = engine->CreateMeshFactory (
      "crystalspace.mesh.object.particles", "fire");
  if (!mfw) return;

  csRef<iMeshWrapper> exp = engine->CreateMeshWrapper (mfw, "custom fire",
	sector, origin);

  exp->SetZBufMode(CS_ZBUF_TEST);
  exp->GetMeshObject()->SetMixMode (CS_FX_ADD);
  exp->GetMeshObject()->SetMaterialWrapper (mat);

  csRef<iParticleBuiltinEmitterFactory> emit_factory = 
      csLoadPluginCheck<iParticleBuiltinEmitterFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.emitter", false);
  csRef<iParticleBuiltinEffectorFactory> eff_factory = 
      csLoadPluginCheck<iParticleBuiltinEffectorFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.effector", false);

  csRef<iParticleBuiltinEmitterSphere> sphemit = emit_factory->CreateSphere ();
  float velocity = 0.5f;
  float seconds_to_live = 2.0f;
  sphemit->SetRadius (.2f);
  sphemit->SetParticlePlacement (CS_PARTICLE_BUILTIN_VOLUME);
  sphemit->SetEmissionRate (float (num) / seconds_to_live);
  sphemit->SetInitialMass (5.0f, 7.5f);
  sphemit->SetUniformVelocity (true);
  sphemit->SetInitialTTL (seconds_to_live, seconds_to_live);
  sphemit->SetInitialVelocity (csVector3 (0, velocity, 0),
      csVector3 (0));

  csRef<iParticleBuiltinEffectorLinColor> lincol = eff_factory->
    CreateLinColor ();
  lincol->AddColor (csColor4 (0.00f, 0.00f, 0.00f, 1.00f), 2.0000f);
  lincol->AddColor (csColor4 (1.00f, 0.35f, 0.00f, 0.00f), 1.5000f);
  lincol->AddColor (csColor4 (1.00f, 0.22f, 0.00f, 0.10f), 1.3125f);
  lincol->AddColor (csColor4 (1.00f, 0.12f, 0.00f, 0.30f), 1.1250f);
  lincol->AddColor (csColor4 (0.80f, 0.02f, 0.00f, 0.80f), 0.9375f);
  lincol->AddColor (csColor4 (0.60f, 0.00f, 0.00f, 0.90f), 0.7500f);
  lincol->AddColor (csColor4 (0.40f, 0.00f, 0.00f, 0.97f), 0.5625f);
  lincol->AddColor (csColor4 (0.20f, 0.00f, 0.00f, 1.00f), 0.3750f);
  lincol->AddColor (csColor4 (0.00f, 0.00f, 0.00f, 1.00f), 0.1875f);
  lincol->AddColor (csColor4 (0.00f, 0.00f, 0.00f, 1.00f), 0.0000f);

  csRef<iParticleBuiltinEffectorForce> force = eff_factory->
    CreateForce ();
  force->SetRandomAcceleration (csVector3 (1.5f, 1.5f, 1.5f));

  csRef<iParticleSystem> partstate =
  	scfQueryInterface<iParticleSystem> (exp->GetMeshObject ());
  //partstate->SetMinBoundingBox (bbox);
  partstate->SetParticleSize (csVector2 (0.04f, 0.08f));
  partstate->AddEmitter (sphemit);
  partstate->AddEffector (lincol);
  partstate->AddEffector (force);
}

//===========================================================================
// Demo particle system (fountain).
//===========================================================================
static void add_particles_fountain (WalkTest* Sys, iSector* sector, char* matname, int num,
	const csVector3& origin)
{
  iEngine* engine = Sys->Engine;

  // First check if the material exists.
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material %s!",
		 CS::Quote::Single (matname));
    return;
  }

  csRef<iMeshFactoryWrapper> mfw = engine->CreateMeshFactory (
      "crystalspace.mesh.object.particles", "fountain");
  if (!mfw) return;

  csRef<iMeshWrapper> exp = engine->CreateMeshWrapper (mfw, "custom fountain",
	sector, origin);

  exp->SetZBufMode(CS_ZBUF_TEST);
  exp->GetMeshObject()->SetMixMode (CS_FX_ADD);
  exp->GetMeshObject()->SetMaterialWrapper (mat);

  csRef<iParticleBuiltinEmitterFactory> emit_factory = 
      csLoadPluginCheck<iParticleBuiltinEmitterFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.emitter", false);
  csRef<iParticleBuiltinEffectorFactory> eff_factory = 
      csLoadPluginCheck<iParticleBuiltinEffectorFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.effector", false);

  csRef<iParticleBuiltinEmitterCone> conemit = emit_factory->CreateCone ();
  float velocity = 3.0f;
  float seconds_to_live = 1.5f;
  conemit->SetExtent (csVector3 (0, 0.5f, 0));
  conemit->SetConeAngle (0.3f);
  conemit->SetParticlePlacement (CS_PARTICLE_BUILTIN_VOLUME);
  conemit->SetEmissionRate (float (num) / seconds_to_live);
  conemit->SetInitialMass (8.0f, 10.0f);
  conemit->SetInitialTTL (seconds_to_live, seconds_to_live);
  conemit->SetInitialVelocity (csVector3 (0, velocity, 0), csVector3 (0));

  csRef<iParticleBuiltinEffectorLinColor> lincol = eff_factory->
    CreateLinColor ();
  lincol->AddColor (csColor4 (0.25f, 0.35f, 0.55f, 1), seconds_to_live);

  csRef<iParticleBuiltinEffectorForce> force = eff_factory->
    CreateForce ();
  force->SetAcceleration (csVector3 (0.0f, -3.0f, 0.0f));

  csRef<iParticleSystem> partstate =
  	scfQueryInterface<iParticleSystem> (exp->GetMeshObject ());
  partstate->SetParticleSize (csVector2 (0.1f, 0.1f));
  partstate->AddEmitter (conemit);
  partstate->AddEffector (lincol);
  partstate->AddEffector (force);
}

//===========================================================================
// Demo particle system (explosion).
//===========================================================================
static void add_particles_explosion (WalkTest* Sys, iSector* sector, iEngine* engine,
	const csVector3& center, const char* matname)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->Engine->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material %s in memory!",
		 CS::Quote::Single (matname));
    return;
  }

  csRef<iMeshFactoryWrapper> mfw = engine->CreateMeshFactory (
      "crystalspace.mesh.object.particles", "explosion");
  if (!mfw) return;

  csRef<iMeshWrapper> exp = engine->CreateMeshWrapper (mfw, "custom explosion",
	sector, center);

  exp->SetZBufMode(CS_ZBUF_TEST);
  exp->SetRenderPriority (engine->GetAlphaRenderPriority ());
  exp->GetMeshObject()->SetMaterialWrapper (mat);
  exp->GetMeshObject()->SetMixMode (CS_FX_ALPHA);
  exp->GetMeshObject()->SetColor (csColor (1, 1, 0));

  csRef<iParticleBuiltinEmitterFactory> emit_factory = 
      csLoadPluginCheck<iParticleBuiltinEmitterFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.emitter", false);
  csRef<iParticleBuiltinEffectorFactory> eff_factory = 
      csLoadPluginCheck<iParticleBuiltinEffectorFactory> (
        Sys->object_reg, "crystalspace.mesh.object.particles.effector", false);

  csRef<iParticleBuiltinEmitterSphere> sphereemit = emit_factory->
    CreateSphere ();
  sphereemit->SetRadius (0.1f);
  sphereemit->SetParticlePlacement (CS_PARTICLE_BUILTIN_CENTER);
  sphereemit->SetPosition (csVector3 (0, 0, 0));
  sphereemit->SetInitialVelocity (csVector3 (1, 0, 0), csVector3 (3, 3, 3));
  sphereemit->SetUniformVelocity (false);
  sphereemit->SetDuration (0.1f);
  sphereemit->SetEmissionRate (1000.0f);
  sphereemit->SetInitialTTL (1.0f, 1.0f);

  csRef<iParticleBuiltinEffectorLinColor> lincol = eff_factory->
    CreateLinColor ();
  lincol->AddColor (csColor4 (1,1,1,1), 1.0f);
  lincol->AddColor (csColor4 (1,1,1,0), 0.0f);

  csRef<iParticleSystem> partstate =
  	scfQueryInterface<iParticleSystem> (exp->GetMeshObject ());
  partstate->SetParticleSize (csVector2 (0.15f, 0.15f));
  partstate->SetRotationMode (CS_PARTICLE_ROTATE_VERTICES);
  partstate->SetIntegrationMode (CS_PARTICLE_INTEGRATE_BOTH);
  partstate->AddEmitter (sphereemit);
  partstate->AddEffector (lincol);

  Sys->Engine->DelayedRemoveObject (1100, exp);
  Sys->Engine->DelayedRemoveObject (1101, mfw);

  exp->PlaceMesh ();
}

void WalkTestParticleDemos::Rain (WalkTest* Sys, const char* arg)
{
  char txtname[100];
  int cnt = 0;
  /* speed and num must be preset to prevent compiler warnings
   * on some systems. */
  int num = 0;
  float speed = 0;
  if (arg) cnt = csScanStr (arg, "%s,%d,%f", txtname, &num, &speed);
  if (cnt <= 2) speed = 2;
  if (cnt <= 1) num = 500;
  if (cnt <= 0) strcpy (txtname, "raindrop");
  add_particles_rain (Sys, Sys->views->GetCamera ()->GetSector (),
      txtname, num, speed, false);
}

void WalkTestParticleDemos::FollowRain (WalkTest* Sys, const char* arg)
{
  char txtname[100];
  int cnt = 0;
  /* speed and num must be preset to prevent compiler warnings
   * on some systems. */
  int num = 0;
  float speed = 0;
  if (arg) cnt = csScanStr (arg, "%s,%d,%f", txtname, &num, &speed);
  if (cnt <= 2) speed = 2;
  if (cnt <= 1) num = 500;
  if (cnt <= 0) strcpy (txtname, "raindrop");
  add_particles_rain (Sys, Sys->views->GetCamera ()->GetSector (),
      txtname, num, speed, true);
}

void WalkTestParticleDemos::Snow (WalkTest* Sys, const char* arg)
{
  char txtname[100];
  int cnt = 0;
  /* speed and num must be preset to prevent compiler warnings
   * on some systems. */
  int num = 0;
  float speed = 0;
  if (arg) cnt = csScanStr (arg, "%s,%d,%f", txtname, &num, &speed);
  if (cnt <= 2) speed = 0.3f;
  if (cnt <= 1) num = 500;
  if (cnt <= 0) strcpy (txtname, "snow");
  add_particles_snow (Sys, Sys->views->GetCamera ()->GetSector (),
      txtname, num, speed);
}

void WalkTestParticleDemos::Flame (WalkTest* Sys, const char* arg)
{
  char txtname[100];
  int cnt = 0;
  int num = 0;
  if (arg) cnt = csScanStr (arg, "%s,%d", txtname, &num);
  if (cnt <= 1) num = 200;
  if (cnt <= 0) strcpy (txtname, "raindrop");
  add_particles_fire (Sys, Sys->views->GetCamera ()->GetSector (),
      txtname, num, Sys->views->GetCamera ()->GetTransform ().GetOrigin ()-
      csVector3 (0, Sys->cfg_body_height, 0));
}

void WalkTestParticleDemos::Fountain (WalkTest* Sys, const char* arg)
{
  char txtname[100];
  int cnt = 0;
  int num = 0;
  if (arg) cnt = csScanStr (arg, "%s,%d", txtname, &num);
  if (cnt <= 1) num = 400;
  if (cnt <= 0) strcpy (txtname, "spark");
  add_particles_fountain (Sys, Sys->views->GetCamera ()->GetSector (),
      txtname, num, Sys->views->GetCamera ()->GetTransform ().GetOrigin ()-
      csVector3 (0, Sys->cfg_body_height, 0));
}

void WalkTestParticleDemos::Explosion (WalkTest* Sys, const char* arg)
{
  char txtname[100];
  int cnt = 0;
  if (arg) cnt = csScanStr (arg, "%s", txtname);
  if (cnt != 1)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Expected parameter %s!", CS::Quote::Single ("texture"));
  }
  else
    add_particles_explosion (Sys, Sys->views->GetCamera ()->GetSector (),
      Sys->Engine,
      Sys->views->GetCamera ()->GetTransform ().GetOrigin (), txtname);
}

void WalkTestParticleDemos::Explosion (WalkTest* Sys, iSector* sector, const csVector3& center,
      const char* materialname)
{
  add_particles_explosion (Sys, sector, Sys->Engine, center, materialname);
}

