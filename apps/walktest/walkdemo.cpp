/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "walktest.h"
#include "bot.h"
#include "infmaze.h"
#include "command.h"
#include "ivaria/view.h"
#include "ivaria/engseq.h"
#include "iengine/light.h"
#include "iengine/campos.h"
#include "iengine/region.h"
#include "iengine/material.h"
#include "iengine/sector.h"
#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/sharevar.h"
#include "csgeom/poly3d.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "csutil/scanstr.h"
#include "csgeom/math3d.h"
#include "igeom/polymesh.h"
#include "igeom/objmodel.h"
#include "cstool/collider.h"
#include "cstool/cspixmap.h"
#include "csqint.h"
#include "isound/handle.h"
#include "isound/source.h"
#include "isound/listener.h"
#include "isound/renderer.h"
#include "isound/wrapper.h"
#include "ivideo/graph3d.h"
#include "ivaria/collider.h"
#include "ivaria/reporter.h"
#include "imesh/lighting.h"
#include "imesh/partsys.h"
#include "imesh/fountain.h"
#include "imesh/explode.h"
#include "imesh/fire.h"
#include "imesh/snow.h"
#include "imesh/rain.h"
#include "imesh/spiral.h"
#include "imesh/sprite3d.h"
#include "imap/parser.h"

extern WalkTest* Sys;


void RandomColor (float& r, float& g, float& b)
{
  float sig = (float)(900+(rand () % 100))/1000.;
  float sm1= (float)(rand () % 1000)/1000.;
  float sm2 = (float)(rand () % 1000)/1000.;
  switch ((rand ()>>3) % 3)
  {
    case 0: r = sig; g = sm1; b = sm2; break;
    case 1: r = sm1; g = sig; b = sm2; break;
    case 2: r = sm1; g = sm2; b = sig; break;
  }
}

extern iMeshWrapper* add_meshobj (const char* tname, char* sname, iSector* where,
	csVector3 const& pos, float size);
extern void move_mesh (iMeshWrapper* sprite, iSector* where,
	csVector3 const& pos);

//===========================================================================
// Demo particle system (rain).
//===========================================================================
void add_particles_rain (iSector* sector, char* matname, int num, float speed,
	bool do_camera)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csBox3 bbox;
  if (do_camera)
    bbox.Set (-5, -5, -5, 5, 5, 5);
  else
    sector->CalculateSectorBBox (bbox, true);

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.rain", "rain"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom rain", sector,
					  csVector3 (0, 0, 0)));
  if (do_camera)
  {
    iEngine* e = Sys->view->GetEngine ();
    int c = e->GetAlphaRenderPriority ();
    exp->GetFlags ().Set (CS_ENTITY_CAMERA);
    exp->SetRenderPriority (c);
  }
  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iRainState> rainstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iRainState));
  rainstate->SetMaterialWrapper (mat);
  rainstate->SetMixMode (CS_FX_ADD);
  rainstate->SetColor (csColor (.25,.25,.25));
  rainstate->SetParticleCount (num);
  rainstate->SetDropSize (0.3f/50.0f, 0.3f);
  rainstate->SetLighting (false);
  rainstate->SetBox (bbox.Min (), bbox.Max ());
  rainstate->SetFallSpeed (csVector3 (0, -speed, 0));
}

//===========================================================================
// Demo particle system (snow).
//===========================================================================
void add_particles_snow (iSector* sector, char* matname, int num, float speed)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csBox3 bbox;
  sector->CalculateSectorBBox (bbox, true);

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.snow", "snow"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom snow", sector,
	csVector3 (0, 0, 0)));

  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);
  partstate->SetColor (csColor (.25,.25,.25));

  csRef<iSnowState> snowstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iSnowState));
  snowstate->SetParticleCount (num);
  snowstate->SetDropSize (0.07f, 0.07f);
  snowstate->SetLighting (false);
  snowstate->SetBox (bbox.Min (), bbox.Max ());
  snowstate->SetFallSpeed (csVector3 (0, -speed, 0));
  snowstate->SetSwirl (0.2f);
}

//===========================================================================
// Demo particle system (fire).
//===========================================================================
void add_particles_fire (iSector* sector, char* matname, int num,
	const csVector3& origin)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.fire", "fire"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom fire", sector,
	origin));

  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);

  csRef<iFireState> firestate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iFireState));
  firestate->SetParticleCount (num);
  //firestate->SetDropSize (.02, .04);
  firestate->SetDropSize (0.04f, 0.08f);
  firestate->SetLighting (false);
  firestate->SetOrigin (csBox3(-csVector3(0.2f, 0, 0.2f),
    csVector3(0.2f, 0.2f)));
  firestate->SetDirection (csVector3 (0, 1.0f, 0));
  firestate->SetSwirl (1.6f);
  firestate->SetColorScale (0.2f);
}

//===========================================================================
// Demo particle system (fountain).
//===========================================================================
void add_particles_fountain (iSector* sector, char* matname, int num,
	const csVector3& origin)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.fountain", "fountain"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom fountain",
	sector, origin));
  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_ADD);
  partstate->SetColor (csColor (0.25f, 0.35f, 0.55f));
  partstate->SetChangeRotation (7.5f);

  csRef<iFountainState> fountstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iFountainState));
  fountstate->SetParticleCount (num);
  fountstate->SetDropSize (0.1f, 0.1f);
  fountstate->SetOrigin (csVector3 (0, 0, 0));
  fountstate->SetAcceleration (csVector3 (0, -1.0f, 0));
  fountstate->SetFallTime (5.0f);
  fountstate->SetSpeed (3.0f);
  fountstate->SetElevation (3.1415926f/2.0f);
  fountstate->SetAzimuth (0);
  fountstate->SetOpening (0.2f);
}

//===========================================================================
// Demo particle system (explosion).
//===========================================================================
void add_particles_explosion (iSector* sector, iEngine* engine,
	const csVector3& center, char* matname)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.explosion", "explosion"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom explosion",
	sector, center));

  exp->SetZBufMode(CS_ZBUF_TEST);
  exp->SetRenderPriority (engine->GetAlphaRenderPriority ());

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_SETALPHA (0.50));
  partstate->SetColor (csColor (1, 1, 0));
  partstate->SetChangeRotation (5.0);
  partstate->SetChangeSize (1.25);
  partstate->SetSelfDestruct (3000);

  csRef<iExplosionState> expstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iExplosionState));
  expstate->SetParticleCount (100);
  expstate->SetCenter (csVector3 (0, 0, 0));
  expstate->SetPush (csVector3 (0, 0, 0));
  expstate->SetNrSides (6);
  expstate->SetPartRadius (0.15f);
  expstate->SetLighting (true);
  expstate->SetSpreadPos (0.6f);
  expstate->SetSpreadSpeed (2.0f);
  expstate->SetSpreadAcceleration (2.0f);
  expstate->SetFadeSprites (500);

  exp->PlaceMesh ();
}

//===========================================================================
// Demo particle system (spiral).
//===========================================================================
void add_particles_spiral (iSector* sector, const csVector3& bottom,
	char* matname)
{
  // First check if the material exists.
  iMaterialWrapper* mat = Sys->view->GetEngine ()->GetMaterialList ()->
  	FindByName (matname);
  if (!mat)
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY, "Can't find material '%s' in memory!", matname);
    return;
  }

  csRef<iMeshFactoryWrapper> mfw (Sys->view->GetEngine ()->
    CreateMeshFactory ("crystalspace.mesh.object.spiral", "spiral"));
  if (!mfw) return;

  csRef<iMeshWrapper> exp (
  	Sys->view->GetEngine ()->CreateMeshWrapper (mfw, "custom spiral",
	sector, bottom));

  exp->SetZBufMode(CS_ZBUF_TEST);

  csRef<iParticleState> partstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iParticleState));
  partstate->SetMaterialWrapper (mat);
  partstate->SetMixMode (CS_FX_SETALPHA (0.50));
  partstate->SetColor (csColor (1, 1, 0));
  partstate->SetChangeColor (csColor(+0.01f, 0.0f, -0.012f));

  csRef<iSpiralState> spirstate (
  	SCF_QUERY_INTERFACE (exp->GetMeshObject (), iSpiralState));
  spirstate->SetParticleCount (500);
  spirstate->SetSource (csVector3 (0, 0, 0));
}

//===========================================================================
// Everything for bots.
//===========================================================================

bool do_bots = false;

// Add a bot with some size at the specified positin.
void WalkTest::add_bot (float size, iSector* where, csVector3 const& pos,
	float dyn_radius)
{
  csRef<iLight> dyn;
  if (dyn_radius)
  {
    float r, g, b;
    RandomColor (r, g, b);
    dyn = Sys->view->GetEngine ()->CreateLight ("",
    	pos, dyn_radius, csColor(r, g, b), CS_LIGHT_DYNAMICTYPE_DYNAMIC);
    where->GetLights ()->Add (dyn);
    dyn->Setup ();
    //@@@ BUG! Calling twice is needed.
    dyn->Setup ();
    Sys->dynamic_lights.Push (dyn);
  }
  iMeshFactoryWrapper* tmpl = Sys->view->GetEngine ()->GetMeshFactories ()
  	->FindByName ("bot");
  if (!tmpl) return;
  csRef<iMeshObject> botmesh (tmpl->GetMeshObjectFactory ()->NewInstance ());
  csRef<iMeshWrapper> botWrapper = Engine->CreateMeshWrapper (botmesh, "bot",
    where);

  csMatrix3 m; m.Identity (); m = m * size;
  botWrapper->GetMovable ()->SetTransform (m);
  
  botWrapper->GetMovable ()->UpdateMove ();
  csRef<iSprite3DState> state (SCF_QUERY_INTERFACE (botmesh, iSprite3DState));
  state->SetAction ("default");
  
  Bot* bot = new Bot (Sys->view->GetEngine(), botWrapper);
  bot->set_bot_move (pos);
  bot->set_bot_sector (where);
  bots.Push (bot);
}

void WalkTest::del_bot ()
{
  if (bots.Length () > 0)
    bots.DeleteIndex (0);
}

void WalkTest::move_bots (csTicks elapsed_time)
{
  size_t i;
  for (i = 0; i < bots.Length(); i++)
  {
    bots[i]->move (elapsed_time);
  }
}

//===========================================================================
// Everything for the missile.
//===========================================================================

#define DYN_TYPE_MISSILE 1
#define DYN_TYPE_RANDOM 2
#define DYN_TYPE_EXPLOSION 3

struct LightStruct
{
  int type;
};

struct MissileStruct
{
  int type;		// type == DYN_TYPE_MISSILE
  csOrthoTransform dir;
  csRef<iMeshWrapper> sprite;
  csRef<iSoundSource> snd;
};

struct ExplosionStruct
{
  int type;		// type == DYN_TYPE_EXPLOSION
  float radius;
  int dir;
};

struct RandomLight
{
  int type;		// type == DYN_TYPE_RANDOM
  float dyn_move_dir;
  float dyn_move;
  float dyn_r1, dyn_g1, dyn_b1;
};

bool HandleDynLight (iLight* dyn, iEngine* engine)
{
  LightStruct* ls = (LightStruct*)(WalkDataObject::GetData(dyn->QueryObject ()));
  switch (ls->type)
  {
    case DYN_TYPE_MISSILE:
    {
      MissileStruct* ms = (MissileStruct*)(WalkDataObject::GetData(
      	dyn->QueryObject ()));
      csVector3 v (0, 0, 2.5);
      csVector3 old = dyn->GetCenter ();
      v = old + ms->dir.GetT2O () * v;
      iSector* s = dyn->GetSector ();
      bool mirror = false;
      csVector3 old_v = v;
      s = s->FollowSegment (ms->dir, v, mirror);
      if (ABS (v.x-old_v.x) > SMALL_EPSILON ||
      	ABS (v.y-old_v.y) > SMALL_EPSILON ||
	ABS (v.z-old_v.z) > SMALL_EPSILON)
      {
        v = old;
        if (ms->sprite)
      	{
          if ((rand () & 0x3) == 1)
	  {
	    int i;
	    if (do_bots)
	      for (i = 0 ; i < 40 ; i++)
            Sys->add_bot (1, dyn->GetSector (), dyn->GetCenter (), 0);
	  }
	  ms->sprite->GetMovable ()->ClearSectors ();
	  Sys->view->GetEngine ()->GetMeshes ()->Remove (ms->sprite);
	}
	csRef<WalkDataObject> ido (
		CS_GET_CHILD_OBJECT (dyn->QueryObject (), WalkDataObject));
        dyn->QueryObject ()->ObjRemove (ido);
        if (ms->snd)
        {
          ms->snd->Stop();
        }
        delete ms;
        if (Sys->mySound)
        {
          csRef<iSoundSource> sndsrc (
	  	Sys->wMissile_boom->CreateSource (SOUND3D_ABSOLUTE));
          if (sndsrc)
          {
            sndsrc->SetPosition (v);
            sndsrc->Play();
          }
        }
        ExplosionStruct* es = new ExplosionStruct;
        es->type = DYN_TYPE_EXPLOSION;
        es->radius = 2;
        es->dir = 1;
        WalkDataObject* esdata = new WalkDataObject (es);
        dyn->QueryObject ()->ObjAdd (esdata);
	esdata->DecRef ();
        add_particles_explosion (dyn->GetSector (),
		engine, dyn->GetCenter (), "explo");
        return false;
      }
      else ms->dir.SetOrigin (v);
      if (dyn->GetSector () != s)
      {
	dyn->IncRef ();
        dyn->GetSector ()->GetLights ()->Remove (dyn);
        s->GetLights ()->Add (dyn);
	dyn->DecRef ();
      }
      dyn->SetCenter (v);
      dyn->Setup ();
      if (ms->sprite) move_mesh (ms->sprite, s, v);
      if (Sys->mySound && ms->snd) ms->snd->SetPosition (v);
      break;
    }
    case DYN_TYPE_EXPLOSION:
    {
      ExplosionStruct* es = (ExplosionStruct*)(WalkDataObject::GetData(
      	dyn->QueryObject ()));
      if (es->dir == 1)
      {
        es->radius += 3;
	if (es->radius > 6) es->dir = -1;
      }
      else
      {
        es->radius -= 2;
	if (es->radius < 1)
	{
	  csRef<WalkDataObject> ido (
	  	CS_GET_CHILD_OBJECT (dyn->QueryObject (), WalkDataObject));
	  dyn->QueryObject ()->ObjRemove (ido);
	  delete es;
	  dyn->GetSector ()->GetLights ()->Remove (dyn);
	  return true;
	}
      }
      dyn->SetCutoffDistance (es->radius);
      dyn->Setup ();
      break;
    }
    case DYN_TYPE_RANDOM:
    {
      RandomLight* rl = (RandomLight*)(WalkDataObject::GetData(
      	dyn->QueryObject ()));
      rl->dyn_move += rl->dyn_move_dir;
      if (rl->dyn_move < 0 || rl->dyn_move > 2)
      	rl->dyn_move_dir = -rl->dyn_move_dir;
      if (ABS (rl->dyn_r1-dyn->GetColor ().red) < .01 &&
      	  ABS (rl->dyn_g1-dyn->GetColor ().green) < .01 &&
	  ABS (rl->dyn_b1-dyn->GetColor ().blue) < .01)
        RandomColor (rl->dyn_r1, rl->dyn_g1, rl->dyn_b1);
      else
        dyn->SetColor (csColor ((rl->dyn_r1+7.*dyn->GetColor ().red)/8.,
		(rl->dyn_g1+7.*dyn->GetColor ().green)/8.,
		(rl->dyn_b1+7.*dyn->GetColor ().blue)/8.));
      dyn->SetCenter (dyn->GetCenter () + csVector3 (0, rl->dyn_move_dir, 0));
      dyn->Setup ();
      break;
    }
  }
  return false;
}

void show_lightning ()
{
  csRef<iEngineSequenceManager> seqmgr(CS_QUERY_REGISTRY (Sys->object_reg,
  	iEngineSequenceManager));
  if (seqmgr)
  {
    // This finds the light L1 (the colored light over the stairs) and
    // makes the lightning restore this color back after it runs.
    iLight *light = Sys->view->GetEngine ()->FindLight("l1");
    iSharedVariable *var = Sys->view->GetEngine ()->GetVariableList()
    	->FindByName("Lightning Restore Color");
    if (light && var)
    {
      var->SetColor (light->GetColor ());
    }
    seqmgr->RunSequenceByName ("seq_lightning", 0);
  }
  else
  {
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	             "Could not find engine sequence manager!");
  }
}

void fire_missile ()
{
  csVector3 dir (0, 0, 0);
  csVector3 pos = Sys->view->GetCamera ()->GetTransform ().This2Other (dir);
  float r, g, b;
  RandomColor (r, g, b);
  csRef<iLight> dyn =
  	Sys->view->GetEngine ()->CreateLight ("", pos, 4, csColor (r, g, b),
		CS_LIGHT_DYNAMICTYPE_DYNAMIC);
  Sys->view->GetCamera ()->GetSector ()->GetLights ()->Add (dyn);
  dyn->Setup ();
  // @@@ BUG!!! Calling twice is needed.
  dyn->Setup ();
  Sys->dynamic_lights.Push (dyn);

  MissileStruct* ms = new MissileStruct;
  ms->snd = 0;
  if (Sys->mySound)
  {
    ms->snd = Sys->wMissile_whoosh->CreateSource (SOUND3D_ABSOLUTE);
    if (ms->snd)
    {
      ms->snd->SetPosition (pos);
      ms->snd->Play();
    }
  }
  ms->type = DYN_TYPE_MISSILE;
  ms->dir = (csOrthoTransform)(Sys->view->GetCamera ()->GetTransform ());
  ms->sprite = 0;
  WalkDataObject* msdata = new WalkDataObject(ms);
  dyn->QueryObject ()->ObjAdd(msdata);
  msdata->DecRef ();

  csString misname;
  misname.Format ("missile%d", ((rand () >> 3) & 1)+1);

  iMeshFactoryWrapper *tmpl = Sys->view->GetEngine ()->GetMeshFactories ()
  	->FindByName (misname);
  if (!tmpl)
    Sys->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Could not find '%s' sprite factory!", misname.GetData());
  else
  {
    csRef<iMeshWrapper> sp (
    	Sys->view->GetEngine ()->CreateMeshWrapper (tmpl,
	"missile",Sys->view->GetCamera ()->GetSector (), pos));

    ms->sprite = sp;
    csMatrix3 m = ms->dir.GetT2O ();
    sp->GetMovable ()->SetTransform (m);
    sp->GetMovable ()->UpdateMove ();
  }
}

void AttachRandomLight (iLight* light)
{
  RandomLight* rl = new RandomLight;
  rl->type = DYN_TYPE_RANDOM;
  rl->dyn_move_dir = 0.2f;
  rl->dyn_move = 0;
  rl->dyn_r1 = rl->dyn_g1 = rl->dyn_b1 = 1;
  WalkDataObject* rldata = new WalkDataObject (rl);
  light->QueryObject ()->ObjAdd (rldata);
  rldata->DecRef ();
}

//===========================================================================

static csPtr<iMeshWrapper> CreateMeshWrapper (const char* name)
{
  csRef<iPluginManager> plugin_mgr = CS_QUERY_REGISTRY (Sys->object_reg,
  	iPluginManager);
  csRef<iMeshObjectType> ThingType = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.thing", iMeshObjectType);
  if (!ThingType)
    ThingType = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.mesh.object.thing", iMeshObjectType);

  csRef<iMeshObjectFactory> thing_fact = ThingType->NewFactory ();
  csRef<iMeshObject> mesh_obj = thing_fact->NewInstance ();

  csRef<iMeshWrapper> mesh_wrap =
  	Sys->Engine->CreateMeshWrapper (mesh_obj, name);
  return csPtr<iMeshWrapper> (mesh_wrap);
}

static csPtr<iMeshWrapper> CreatePortalThing (const char* name, iSector* room,
    	iMaterialWrapper* tm, int& portalPoly)
{
  csRef<iMeshWrapper> thing = CreateMeshWrapper (name);
  csRef<iThingState> thing_state =
  	SCF_QUERY_INTERFACE (thing->GetMeshObject (),
  	iThingState);
  csRef<iThingFactoryState> thing_fact_state = thing_state->GetFactory ();
  thing->GetMovable ()->SetSector (room);
  float dx = 1, dy = 3, dz = 0.3f;
  float border = 0.3f; // width of border around the portal

  // bottom
  thing_fact_state->AddQuad (
    csVector3 (-dx, 0, -dz),
    csVector3 (dx, 0, -dz),
    csVector3 (dx, 0, dz),
    csVector3 (-dx, 0, dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.25, 0.875),
      csVector2 (0.75, 0.875),
      csVector2 (0.75, 0.75));

  // top
  thing_fact_state->AddQuad (
    csVector3 (-dx, dy, dz),
    csVector3 (dx, dy, dz),
    csVector3 (dx, dy, -dz),
    csVector3 (-dx, dy, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.25, 0.25),
      csVector2 (0.75, 0.25),
      csVector2 (0.75, 0.125));

  // back
  thing_fact_state->AddQuad (
    csVector3 (-dx, 0, dz),
    csVector3 (dx, 0, dz),
    csVector3 (dx, dy, dz),
    csVector3 (-dx, dy, dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.25, 0.75),
      csVector2 (0.75, 0.75),
      csVector2 (0.75, 0.25));

  // right
  thing_fact_state->AddQuad (
    csVector3 (dx, 0, dz),
    csVector3 (dx, 0, -dz),
    csVector3 (dx, dy, -dz),
    csVector3 (dx, dy, dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.75, 0.75),
      csVector2 (0.875, 0.75),
      csVector2 (0.875, 0.25));

  // left
  thing_fact_state->AddQuad (
    csVector3 (-dx, 0, -dz),
    csVector3 (-dx, 0, dz),
    csVector3 (-dx, dy, dz),
    csVector3 (-dx, dy, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 0.75),
      csVector2 (0.25, 0.75),
      csVector2 (0.25, 0.25));

  // front border
  // border top
  thing_fact_state->AddQuad (
    csVector3 (-dx+border, dy, -dz),
    csVector3 (dx-border, dy, -dz),
    csVector3 (dx-border, dy-border, -dz),
    csVector3 (-dx+border, dy-border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 0.125),
      csVector2 (0.875, 0.125),
      csVector2 (0.875, 0.0));
  // border right
  thing_fact_state->AddQuad (
    csVector3 (dx-border, dy-border, -dz),
    csVector3 (dx, dy-border, -dz),
    csVector3 (dx, border, -dz),
    csVector3 (dx-border, border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (1.0, 0.125),
      csVector2 (0.875, 0.125),
      csVector2 (0.875, 0.875));
  // border bottom
  thing_fact_state->AddQuad (
    csVector3 (-dx+border, border, -dz),
    csVector3 (+dx-border, border, -dz),
    csVector3 (+dx-border, 0.0, -dz),
    csVector3 (-dx+border, 0.0, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 1.0),
      csVector2 (0.875, 1.0),
      csVector2 (0.875, 0.875));
  // border left
  thing_fact_state->AddQuad (
    csVector3 (-dx, dy-border, -dz),
    csVector3 (-dx+border, dy-border, -dz),
    csVector3 (-dx+border, border, -dz),
    csVector3 (-dx, border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 0.125),
      csVector2 (0.0, 0.125),
      csVector2 (0.0, 0.875));
  // border topleft
  thing_fact_state->AddQuad (
    csVector3 (-dx, dy, -dz),
    csVector3 (-dx+border, dy, -dz),
    csVector3 (-dx+border, dy-border, -dz),
    csVector3 (-dx, dy-border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 0.125),
      csVector2 (0.0, 0.125),
      csVector2 (0.0, 0.0));
  // border topright
  thing_fact_state->AddQuad (
    csVector3 (dx-border, dy, -dz),
    csVector3 (dx, dy, -dz),
    csVector3 (dx, dy-border, -dz),
    csVector3 (dx-border, dy-border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (1.0, 0.125),
      csVector2 (0.875, 0.125),
      csVector2 (0.875, 0.0));
  // border botright
  thing_fact_state->AddQuad (
    csVector3 (dx-border, border, -dz),
    csVector3 (dx, border, -dz),
    csVector3 (dx, 0.0, -dz),
    csVector3 (dx-border, 0.0, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (1.0, 1.0),
      csVector2 (0.875, 1.0),
      csVector2 (0.875, 0.875));
  // border botleft
  thing_fact_state->AddQuad (
    csVector3 (-dx, border, -dz),
    csVector3 (-dx+border, border, -dz),
    csVector3 (-dx+border, 0.0, -dz),
    csVector3 (-dx, 0.0, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0.125, 1.0),
      csVector2 (0.0, 1.0),
      csVector2 (0.0, 0.875));

  // front - the portal
  portalPoly = thing_fact_state->AddQuad (
    csVector3 (dx-border, border, -dz),
    csVector3 (-dx+border, border, -dz),
    csVector3 (-dx+border, dy-border, -dz),
    csVector3 (dx-border, dy-border, -dz));
  thing_fact_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST,
      csVector2 (0, 0),
      csVector2 (1, 0),
      csVector2 (1, 1));

  thing_fact_state->SetPolygonMaterial (CS_POLYRANGE_ALL, tm);
  thing_fact_state->SetPolygonFlags (CS_POLYRANGE_ALL, CS_POLY_COLLDET);

  csRef<iLightingInfo> linfo (SCF_QUERY_INTERFACE (thing->GetMeshObject (),
    iLightingInfo));
  linfo->InitializeDefault (true);
  room->ShineLights (thing);
  linfo->PrepareLighting ();

  return csPtr<iMeshWrapper> (thing);
}

void OpenPortal (iLoader *LevelLoader, iView* view, char* lev)
{
  iSector* room = view->GetCamera ()->GetSector ();
  csVector3 pos = view->GetCamera ()->GetTransform ().This2Other (
  	csVector3 (0, 0, 1));
  iMaterialWrapper* tm = Sys->Engine->GetMaterialList ()->
  	FindByName ("portal");

  int portalPoly;
  csRef<iMeshWrapper> thing = CreatePortalThing ("portalTo", room, tm,
  	portalPoly);
  csRef<iThingState> thing_state = SCF_QUERY_INTERFACE (
  	thing->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> thing_fact_state = thing_state->GetFactory ();
csPrintf ("b\n"); fflush (stdout);

  bool regionExists = (Sys->Engine->GetRegions ()->FindByName (lev) != 0);
  iRegion* cur_region = Sys->Engine->CreateRegion (lev);
  // If the region did not already exist then we load the level in it.
  if (!regionExists)
  {
    // @@@ No error checking!
    csString buf;
    buf.Format ("/lev/%s", lev);
    Sys->myVFS->ChDir (buf);
    LevelLoader->LoadMapFile ("world", false, cur_region, true);
    cur_region->Prepare ();
  }

  iMovable* tmov = thing->GetMovable ();
  tmov->SetPosition (pos + csVector3 (0, Sys->cfg_legs_offset, 0));
  tmov->Transform (view->GetCamera ()->GetTransform ().GetT2O ());
  tmov->UpdateMove ();

  // First make a portal to the new level.
  iCameraPosition* cp = cur_region->FindCameraPosition ("Start");
  const char* room_name;
  csVector3 topos;
  if (cp) { room_name = cp->GetSector (); topos = cp->GetPosition (); }
  else { room_name = "room"; topos.Set (0, 0, 0); }
  topos.y -= Sys->cfg_eye_offset;
  iSector* start_sector = cur_region->FindSector (room_name);
  if (start_sector)
  {
    csPoly3D poly;
    poly.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 0));
    poly.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 1));
    poly.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 2));
    poly.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 3));
    iPortal* portal;
    csRef<iMeshWrapper> portalMesh = Sys->Engine->CreatePortal (
    	"new_portal", tmov->GetSectors ()->Get (0), csVector3 (0),
	start_sector, poly.GetVertices (), (int)poly.GetVertexCount (),
	portal);
    //iPortal* portal = portalPoly->CreatePortal (start_sector);
    portal->GetFlags ().Set (CS_PORTAL_ZFILL);
    portal->GetFlags ().Set (CS_PORTAL_CLIPDEST);
    portal->SetWarp (view->GetCamera ()->GetTransform ().GetT2O (), topos, pos);

    if (!regionExists)
    {
      // Only if the region did not already exist do we create a portal
      // back. So even if multiple portals go to the region we only have
      // one portal back.
      int portalPolyBack;
      csRef<iMeshWrapper> thingBack = CreatePortalThing ("portalFrom",
	  	start_sector, tm, portalPolyBack);
      thing_state = SCF_QUERY_INTERFACE (thingBack->GetMeshObject (),
      	iThingState);
      thing_fact_state = thing_state->GetFactory ();
      iMovable* tbmov = thingBack->GetMovable ();
      tbmov->SetPosition (topos + csVector3 (0, Sys->cfg_legs_offset, -0.1f));
      tbmov->Transform (csYRotMatrix3 (PI));//view->GetCamera ()->GetW2C ());
      tbmov->UpdateMove ();
      iPortal* portalBack;
      csPoly3D polyBack;
      polyBack.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 0));
      polyBack.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 1));
      polyBack.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 2));
      polyBack.AddVertex (thing_fact_state->GetPolygonVertex (portalPoly, 3));
      csRef<iMeshWrapper> portalMeshBack = Sys->Engine->CreatePortal (
    	  "new_portal_back", tbmov->GetSectors ()->Get (0), csVector3 (0),
	  tmov->GetSectors ()->Get (0), polyBack.GetVertices (),
	  (int)polyBack.GetVertexCount (),
	  portalBack);
      //iPortal* portalBack = portalPolyBack->CreatePortal (room);
      portalBack->GetFlags ().Set (CS_PORTAL_ZFILL);
      portalBack->GetFlags ().Set (CS_PORTAL_CLIPDEST);
      portalBack->SetWarp (view->GetCamera ()->GetTransform ().GetO2T (),
      	-pos, -topos);
    }
  }

  if (!regionExists)
    Sys->InitCollDet (Sys->Engine, cur_region);
}

