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
#include "demoldr.h"
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

Demo* DemoSequenceManager::demo;
DemoSequenceManager* DemoSequenceManager::demoseq;

DemoSequenceManager::DemoSequenceManager (Demo* demo)
{
  DemoSequenceManager::demo = demo;
  demoseq = this;
  seqmgr = LOAD_PLUGIN (demo, "crystalspace.utilities.sequence",
  	"Sequence", iSequenceManager);
  if (!seqmgr)
  {
    demo->Printf (MSG_FATAL_ERROR, "Could not load sequence manager plugin!\n");
    exit (0);
  }

  do_camera_path = false;
  do_fade = false;
  fade_value = 0;
  suspended = true;
  suspend_one_frame = false;
}

DemoSequenceManager::~DemoSequenceManager ()
{
  if (seqmgr) seqmgr->DecRef ();
  int i;
  for (i = 0 ; i < paths.Length () ; i++)
  {
    csNamedPath* np = (csNamedPath*)paths[i];
    delete np;
  }
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    delete pfm;
  }
}

void DemoSequenceManager::Setup (const char* sequenceFileName)
{
  DemoSequenceLoader* loader = new DemoSequenceLoader (
  	DemoSequenceManager::demo, this, seqmgr, sequenceFileName);
  iSequence* seq = loader->GetSequence ("main");
  seqmgr->RunSequence (0, seq);
  seq->DecRef ();
  seqmgr->Resume ();
  suspended = false;
}

void DemoSequenceManager::Suspend ()
{
  if (!suspended)
  {
    suspended = true;
    seqmgr->Suspend ();
    suspend_time = demo->GetTime ();
  }
}

void DemoSequenceManager::Resume ()
{
  if (suspended)
  {
    suspended = false;
    seqmgr->Resume ();
    // Now we correct all time information in the sequencer
    // so that it appears as if we just go on from here.
    cs_time dt = demo->GetTime ()-suspend_time;
    start_fade_time += dt;
    start_camera_path_time += dt;
    int i;
    for (i = 0 ; i < pathForMesh.Length () ; i++)
    {
      PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
      pfm->start_path_time += dt;
    }
  }
}

void DemoSequenceManager::TimeWarp (cs_time dt)
{
  seqmgr->TimeWarp (dt, false);
  // Now we correct all time information in the sequencer
  // so that it appears as if we just go on from here.
  start_fade_time -= dt;
  start_camera_path_time -= dt;
  int i;
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    pfm->start_path_time -= dt;
  }
  if (suspended)
  {
    Resume ();
    suspend_one_frame = true;
  }
}


void DemoSequenceManager::Draw3DEffects (iGraphics3D* g3d, cs_time current_time)
{
  if (!suspended)
  {
    if (do_fade)
    {
      float r = float (current_time - start_fade_time)
      	/ float (total_fade_time);
      if (r >= 1)
      {
        r = 1;
        do_fade = false;
      }
      fade_value = start_fade * (1-r) + end_fade * r;
      if (fade_value < 0) fade_value = 0;
      if (fade_value > 1) fade_value = 1;
    }
  }
  if (fade_value > .001) csfxFadeOut (g3d, fade_value);
}

void DemoSequenceManager::Draw2DEffects (iGraphics2D* /*g2d*/,
	cs_time /*current_time*/)
{
}

void DemoSequenceManager::SetupFade (float start_fade, float end_fade,
  	cs_time total_fade_time, cs_time already_elapsed)
{
  DemoSequenceManager::start_fade = start_fade;
  DemoSequenceManager::end_fade = end_fade;
  DemoSequenceManager::total_fade_time = total_fade_time;
  start_fade_time = demo->GetTime ()-already_elapsed;
  if (already_elapsed >= total_fade_time)
  {
    // The fading is already done so we just set the fade
    // result to the final value.
    fade_value = end_fade;
    do_fade = false;
  }
  else
  {
    do_fade = true;
  }
}

void DemoSequenceManager::SetupCameraPath (csNamedPath* path,
	cs_time total_camera_path_time,
  	cs_time already_elapsed)
{
  DemoSequenceManager::total_camera_path_time = total_camera_path_time;
  start_camera_path_time = demo->GetTime ()-already_elapsed;
  camera_path = path;
  do_camera_path = true;
}

void DemoSequenceManager::SetupMeshPath (csNamedPath* path,
	iMeshWrapper* mesh,
	cs_time total_path_time,
  	cs_time already_elapsed)
{
  PathForMesh* pfm = new PathForMesh ();
  pfm->path = path;
  pfm->mesh = mesh;
  pfm->total_path_time = total_path_time;
  pfm->start_path_time = demo->GetTime ()-already_elapsed;
  pathForMesh.Push (pfm);
}

void DemoSequenceManager::ControlPaths (iCamera* camera, cs_time current_time)
{
  if (suspended) return;
  int i = 0;
  int len = pathForMesh.Length ();
  while (i < len)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    float r = float (current_time - pfm->start_path_time)
    	/ float (pfm->total_path_time);
    bool do_path = true;
    if (r >= 1)
    {
      r = 1;
      do_path = false;
    }
    pfm->path->Calculate (r);
    csVector3 pos, up, forward;
    pfm->path->GetInterpolatedPosition (pos);
    pfm->path->GetInterpolatedUp (up);
    pfm->path->GetInterpolatedForward (forward);
    iMovable* movable = pfm->mesh->GetMovable ();
    movable->SetPosition (pos);
    movable->GetTransform ().LookAt (forward.Unit (), up.Unit ());
    movable->UpdateMove ();
    if (!do_path)
    {
      delete pfm;
      pathForMesh.Delete (i);
      len--;
    }
    else i++;
  }

  if (do_camera_path)
  {
    float r = GetCameraIndex (current_time);
    if (r >= 1)
    {
      r = 1;
      do_camera_path = false;
    }
    camera_path->Calculate (r);
    csVector3 pos, up, forward;
    camera_path->GetInterpolatedPosition (pos);
    camera_path->GetInterpolatedUp (up);
    camera_path->GetInterpolatedForward (forward);
    camera->SetPosition (pos);
    camera->GetTransform ().LookAt (forward.Unit (), up.Unit ());
  }
  if (suspend_one_frame) { Suspend (); suspend_one_frame = false; }
}

//-----------------------------------------------------------------------------
