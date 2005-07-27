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

#include "cssysdef.h"

#include "csgeom/path.h"
#include "cstool/csfxscr.h"
#include "csutil/cscolor.h"
#include "csutil/flags.h"

#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "ivaria/conout.h"
#include "ivaria/reporter.h"
#include "ivaria/view.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"

#include "demo.h"
#include "demoseq.h"
#include "demoop.h"

//-----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE (StandardOp)
  SCF_IMPLEMENTS_INTERFACE (iSequenceOperation)
SCF_IMPLEMENT_IBASE_END

//-----------------------------------------------------------------------------

void FadeOp::Do (csTicks dt, iBase*)
{
  DemoSequenceManager::demoseq->SetupFade (start_fade,
  	end_fade, total_fade_time, dt);
}

RotatePartOp::RotatePartOp (const char* meshName, csTicks total,
	float aspeed) : total_rotate_time (total), angle_speed (aspeed)
{
  mesh = DemoSequenceManager::demo->engine->GetMeshes ()->FindByName (meshName);
  if (!mesh)
  {
    DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Can't find mesh '%s'", meshName);
    exit (0);
  }
}

void RotatePartOp::Do (csTicks dt, iBase*)
{
  DemoSequenceManager::demoseq->SetupRotatePart (mesh, angle_speed,
  	total_rotate_time, dt);
}

AttachOp::AttachOp (const char* meshName, const char* pathName)
{
  if (meshName)
  {
    mesh = DemoSequenceManager::demo->engine->GetMeshes ()
    	->FindByName (meshName);
    if (!mesh)
    {
      DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	  "Can't find mesh '%s'", meshName);
      exit (0);
    }
  }
  else
    mesh = 0;
  path = DemoSequenceManager::demoseq->FindPath (pathName);
  if (!path)
  {
    DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Can't find path '%s'", pathName);
    exit (0);
  }
}

void AttachOp::Do (csTicks /*dt*/, iBase*)
{
  DemoSequenceManager::demoseq->ReplacePathObject (path, mesh);
}

PathOp::PathOp (csTicks t, const char* meshName, const char* pathName)
{
  total_path_time = t;
  if (meshName)
  {
    mesh = DemoSequenceManager::demo->engine->GetMeshes ()
    	->FindByName (meshName);
    if (!mesh)
    {
      DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	  "Can't find mesh '%s'", meshName);
      exit (0);
    }
  }
  else
    mesh = 0;
  path = DemoSequenceManager::demoseq->FindPath (pathName);
  if (!path)
  {
    DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Can't find path '%s'", pathName);
    exit (0);
  }
}

void PathOp::Do (csTicks dt, iBase*)
{
  DemoSequenceManager::demoseq->SetupPath (path, mesh,
  	total_path_time, dt);
}

SetupMeshOp::SetupMeshOp (const char* meshName, const char* sectName,
	const csVector3& p)
{
  pos = p;
  mesh = DemoSequenceManager::demo->engine->GetMeshes ()
  	->FindByName (meshName);
  if (!mesh)
  {
    DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Can't find mesh '%s'", meshName);
    exit (0);
  }
  sector = DemoSequenceManager::demo->engine->GetSectors ()
  	->FindByName (sectName);
  if (!sector)
  {
    DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Can't find sector '%s'", sectName);
    exit (0);
  }
}

void SetupMeshOp::Do (csTicks /*dt*/, iBase*)
{
  if (mesh)
  {
    iMovable* movable = mesh->GetMovable ();
    movable->SetSector (sector);
    movable->SetPosition (pos);
    movable->UpdateMove ();
  }
}

ShowMeshOp::ShowMeshOp (const char* meshName)
{
  mesh = DemoSequenceManager::demo->engine->GetMeshes ()
  	->FindByName (meshName);
  if (!mesh)
  {
    DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Can't find mesh '%s'", meshName);
    exit (0);
  }
}

void ShowMeshOp::Do (csTicks /*dt*/, iBase*)
{
  if (mesh)
  {
    mesh->GetFlags ().Reset (CS_ENTITY_INVISIBLE);
  }
}

HideMeshOp::HideMeshOp (const char* meshName)
{
  mesh = DemoSequenceManager::demo->engine->GetMeshes ()
  	->FindByName (meshName);
  if (!mesh)
  {
    DemoSequenceManager::demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Can't find mesh '%s'", meshName);
    exit (0);
  }
}

void HideMeshOp::Do (csTicks /*dt*/, iBase*)
{
  if (mesh)
  {
    mesh->GetFlags ().Set (CS_ENTITY_INVISIBLE);
  }
}

void TestOp::Do (csTicks dt, iBase*)
{
  csPrintf ("dt=%ld fps=%g\n", (long)dt,
  	DemoSequenceManager::demoseq->GetFPS ()); fflush (stdout);
}

RecurseOp::RecurseOp (iSequence* sequence, csRef<iSequenceManager> manager)
{
  seq = sequence;
  seqmgr = manager;
}

void RecurseOp::Do (csTicks dt, iBase*)
{
  seqmgr->RunSequence (0, seq);
}
//-----------------------------------------------------------------------------

