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
#include "demo.h"
#include "demoseq.h"
#include "demoop.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivaria/conout.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/polygon.h"
#include "iengine/thing.h"
#include "iengine/light.h"
#include "iengine/view.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/object.h"
#include "csutil/cscolor.h"
#include "csgeom/path.h"
#include "csfx/csfxscr.h"

//-----------------------------------------------------------------------------

void FadeOp::Do (cs_time dt)
{
  DemoSequenceManager::demoseq->SetupFade (start_fade,
  	end_fade, total_fade_time, dt);
}

CameraPathOp::CameraPathOp (cs_time t, const char* pathName) :
	total_camera_path_time (t)
{
  path = DemoSequenceManager::demoseq->FindPath (pathName);
  if (!path)
  {
    DemoSequenceManager::demo->Printf (MSG_FATAL_ERROR,
    	"Can't find path '%s'\n", pathName);
    exit (0);
  }
}

void CameraPathOp::Do (cs_time dt)
{
  DemoSequenceManager::demoseq->SetupCameraPath (path,
  	total_camera_path_time, dt);
}

MeshPathOp::MeshPathOp (cs_time t, const char* meshName, const char* pathName)
{
  total_path_time = t;
  mesh = DemoSequenceManager::demo->engine->FindMeshObject (meshName);
  if (!mesh)
  {
    DemoSequenceManager::demo->Printf (MSG_FATAL_ERROR,
    	"Can't find mesh '%s'\n", meshName);
    exit (0);
  }
  path = DemoSequenceManager::demoseq->FindPath (pathName);
  if (!path)
  {
    DemoSequenceManager::demo->Printf (MSG_FATAL_ERROR,
    	"Can't find path '%s'\n", pathName);
    exit (0);
  }
}

void MeshPathOp::Do (cs_time dt)
{
  DemoSequenceManager::demoseq->SetupMeshPath (path, mesh,
  	total_path_time, dt);
}

ShowMeshOp::ShowMeshOp (const char* meshName, const char* sectName,
	const csVector3& p)
{
  pos = p;
  mesh = DemoSequenceManager::demo->engine->FindMeshObject (meshName);
  if (!mesh)
  {
    DemoSequenceManager::demo->Printf (MSG_FATAL_ERROR,
    	"Can't find mesh '%s'\n", meshName);
    exit (0);
  }
  sector = DemoSequenceManager::demo->engine->FindSector (sectName);
  if (!sector)
  {
    DemoSequenceManager::demo->Printf (MSG_FATAL_ERROR,
    	"Can't find sector '%s'\n", sectName);
    exit (0);
  }
}

void ShowMeshOp::Do (cs_time dt)
{
  if (mesh)
  {
    iMovable* movable = mesh->GetMovable ();
    movable->SetSector (sector);
    movable->SetPosition (pos);
    movable->UpdateMove ();
  }
}

//-----------------------------------------------------------------------------
