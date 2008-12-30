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
#include "iengine/sector.h"
#include "iengine/movable.h"
#include "iengine/camera.h"

#include "walktest.h"
#include "lights.h"
#include "splitview.h"
#include "particles.h"
#include "missile.h"

extern void move_mesh (iMeshWrapper* sprite, iSector* where, csVector3 const& pos);

static void RandomColor (float& r, float& g, float& b)
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


static void AttachRandomLight (iLight* light)
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


WalkTestLights::WalkTestLights (WalkTest* walktest) : walktest (walktest)
{
}

void WalkTestLights::DelLight ()
{
  iLightList* ll = walktest->views->GetCamera ()->GetSector ()->GetLights ();
  int i;
  for (i = 0 ; i < ll->GetCount () ; i++)
  {
    iLight* l = ll->Get (i);
    if (l->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_DYNAMIC)
    {
      ll->Remove (l);
      size_t j;
      for (j = 0 ; j < dynamic_lights.GetSize () ; j++)
      {
	if (dynamic_lights[j] == l)
	{
	  dynamic_lights.DeleteIndex (j);
	  break;
	}
      }
      walktest->Report (CS_REPORTER_SEVERITY_NOTIFY, "Dynamic light removed.");
      break;
    }
  }
}

void WalkTestLights::DelLights ()
{
  iLightList* ll = walktest->views->GetCamera ()->GetSector ()->GetLights ();
  int i;
  for (i = 0 ; i < ll->GetCount () ; i++)
  {
    iLight* l = ll->Get (i);
    if (l->GetDynamicType () == CS_LIGHT_DYNAMICTYPE_DYNAMIC)
    {
      ll->Remove (l);
      size_t j;
      for (j = 0 ; j < dynamic_lights.GetSize () ; j++)
      {
	if (dynamic_lights[j] == l)
	{
	  dynamic_lights.DeleteIndex (j);
	  break;
	}
      }
      i--;
    }
  }
  walktest->Report (CS_REPORTER_SEVERITY_NOTIFY, "All dynamic lights deleted.");
}

void WalkTestLights::AddLight (const char* arg)
{
  csVector3 dir (0,0,0);
  csVector3 pos = walktest->views->GetCamera ()->GetTransform ().This2Other (dir);
  csRef<iLight> dyn;

  bool rnd;
  float r, g, b, radius;
  if (arg && csScanStr (arg, "%f,%f,%f,%f", &r, &g, &b, &radius) == 4)
  {
    dyn = walktest->Engine->CreateLight ("", pos,
      radius, csColor (r, g, b), CS_LIGHT_DYNAMICTYPE_DYNAMIC);
    rnd = false;
  }
  else
  {
    dyn = walktest->Engine->CreateLight ("", pos,
      6, csColor (1, 1, 1), CS_LIGHT_DYNAMICTYPE_DYNAMIC);
    rnd = true;
  }
  iLightList* ll = walktest->views->GetCamera ()->GetSector ()->GetLights ();
  ll->Add (dyn);
  dynamic_lights.Push (dyn);
  if (rnd)
    AttachRandomLight (dyn);
  walktest->Report (CS_REPORTER_SEVERITY_NOTIFY, "Dynamic light added.");
}

csRef<iLight> WalkTestLights::CreateRandomLight (const csVector3& pos, iSector* where,
    float dyn_radius)
{
  csRef<iLight> dyn;
  float r, g, b;
  RandomColor (r, g, b);
  dyn = walktest->Engine->CreateLight ("",
    	pos, dyn_radius, csColor(r, g, b), CS_LIGHT_DYNAMICTYPE_DYNAMIC);
  where->GetLights ()->Add (dyn);
  dynamic_lights.Push (dyn);
  return dyn;
}

void WalkTestLights::HandleDynLights ()
{
  size_t i;
  for (i = 0 ; i < dynamic_lights.GetSize () ; i++)
  {
    iLight* dyn = dynamic_lights[i];
    csRef<WalkDataObject> dao (
        CS::GetChildObject<WalkDataObject>(dyn->QueryObject()));
    if (dao)
      if (HandleDynLight (dyn))
      {
        dynamic_lights.DeleteIndex (i);
	i--;
      }
  }
}

bool WalkTestLights::HandleDynLight (iLight* dyn)
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
	  ms->sprite->GetMovable ()->ClearSectors ();
	  Sys->Engine->GetMeshes ()->Remove (ms->sprite);
	}
	csRef<WalkDataObject> ido (
		CS::GetChildObject<WalkDataObject>(dyn->QueryObject()));
        dyn->QueryObject ()->ObjRemove (ido);
        if (ms->snd)
        {
          ms->snd_stream->Pause();
        }
        delete ms;
        if (Sys->mySound)
        {
	  if (Sys->wMissile_boom)
	  {
	    csRef<iSndSysStream> st = Sys->mySound->CreateStream (
		Sys->wMissile_boom->GetData (), CS_SND3D_ABSOLUTE);
	    csRef<iSndSysSource> sndsource = Sys->mySound->
	      	CreateSource (st);
	    if (sndsource)
	    {
	      csRef<iSndSysSource3D> sndsource3d
		= scfQueryInterface<iSndSysSource3D> (sndsource);

	      sndsource3d->SetPosition (v);
	      sndsource->SetVolume (1.0f);
	      st->SetLoopState (CS_SNDSYS_STREAM_DONTLOOP);
	      st->Unpause ();
	    }
	  }
        }
        ExplosionStruct* es = new ExplosionStruct;
        es->type = DYN_TYPE_EXPLOSION;
        es->radius = 2;
        es->dir = 1;
        WalkDataObject* esdata = new WalkDataObject (es);
        dyn->QueryObject ()->ObjAdd (esdata);
	esdata->DecRef ();
	WalkTestParticleDemos::Explosion (Sys, dyn->GetSector (),
		dyn->GetCenter (), "explo");
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
      if (ms->sprite) move_mesh (ms->sprite, s, v);
      if (Sys->mySound && ms->snd)
      {
	csRef<iSndSysSource3D> sndsource3d
		= scfQueryInterface<iSndSysSource3D> (ms->snd);
	sndsource3d->SetPosition (v);
	ms->snd->SetVolume (1.0f);
      }
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
		CS::GetChildObject<WalkDataObject>(dyn->QueryObject()));
	  dyn->QueryObject ()->ObjRemove (ido);
	  delete es;
	  dyn->GetSector ()->GetLights ()->Remove (dyn);
	  return true;
	}
      }
      dyn->SetCutoffDistance (es->radius);
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
      break;
    }
  }
  return false;
}


