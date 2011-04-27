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
#include "missile.h"
#include "splitview.h"
#include "particles.h"
#include "lights.h"

extern void move_mesh (iMeshWrapper* sprite, iSector* where, csVector3 const& pos);


WalkTestMissileLauncher::WalkTestMissileLauncher (WalkTest* walktest) : walktest (walktest)
{
}

void WalkTestMissileLauncher::FireMissile ()
{
  csVector3 dir (0, 0, 0);
  csVector3 pos = walktest->views->GetCamera ()->GetTransform ().This2Other (dir);
  csRef<iLight> dyn = walktest->lights->CreateRandomLight (pos, walktest->views->GetCamera ()->GetSector (), 4);

  MissileStruct* ms = new MissileStruct;
  ms->snd = 0;
  if (walktest->mySound)
  {
    iSndSysData* snddata = walktest->wMissile_whoosh->GetData ();
    ms->snd_stream = walktest->mySound->CreateStream (snddata, CS_SND3D_ABSOLUTE);
    ms->snd = walktest->mySound->CreateSource (ms->snd_stream);
    if (ms->snd)
    {
      csRef<iSndSysSource3D> sndsource3d
		= scfQueryInterface<iSndSysSource3D> (ms->snd);

      sndsource3d->SetPosition (pos);
      ms->snd->SetVolume (1.0f);
      ms->snd_stream->Unpause ();
    }
  }
  ms->type = DYN_TYPE_MISSILE;
  ms->dir = (csOrthoTransform)(walktest->views->GetCamera ()->GetTransform ());
  ms->sprite = 0;
  WalkDataObject* msdata = new WalkDataObject(ms);
  dyn->QueryObject ()->ObjAdd(msdata);
  msdata->DecRef ();

  csString misname;
  misname.Format ("missile%d", ((rand () >> 3) & 1)+1);

  iMeshFactoryWrapper *tmpl = walktest->Engine->GetMeshFactories ()
  	->FindByName (misname);
  if (!tmpl)
    walktest->Report (CS_REPORTER_SEVERITY_NOTIFY,
    	"Could not find %s sprite factory!", CS::Quote::Single (misname.GetData()));
  else
  {
    csRef<iMeshWrapper> sp (
    	walktest->Engine->CreateMeshWrapper (tmpl,
	"missile",walktest->views->GetCamera ()->GetSector (), pos));

    ms->sprite = sp;
    csMatrix3 m = ms->dir.GetT2O ();
    sp->GetMovable ()->SetTransform (m);
    sp->GetMovable ()->UpdateMove ();
  }
}

