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
#include "ivideo/graph2d.h"
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
  main_sequence = loader->GetSequence ("main");
  seqmgr->RunSequence (0, main_sequence);
  // Don't decref main_sequence because we might need it later.
  seqmgr->Resume ();
  suspended = false;
  delete loader;
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

void DemoSequenceManager::Restart (const char* sequenceFileName)
{
  seqmgr->Clear ();
  int i;
  for (i = 0 ; i < paths.Length () ; i++)
  {
    csNamedPath* np = (csNamedPath*)paths[i];
    delete np;
  }
  paths.DeleteAll ();
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    delete pfm;
  }
  pathForMesh.DeleteAll ();
  do_camera_path = false;
  do_fade = false;
  fade_value = 0;
  suspended = true;
  suspend_one_frame = false;
  Setup (sequenceFileName);
}

void DemoSequenceManager::TimeWarp (cs_time dt)
{
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

  seqmgr->TimeWarp (dt, false);
  if (seqmgr->IsEmpty ())
  {
    // If the sequence manager is empty we insert the main sequence
    // again.
    seqmgr->RunSequence (0, main_sequence);
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
    pfm->mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
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

void DemoSequenceManager::DebugDrawPath (csNamedPath* np, bool hi,
	const csVector2& tl, const csVector2& br, int selpoint)
{
  int dim = demo->G2D->GetHeight ()-10;
  int col = demo->col_gray;
  if (hi) col = demo->col_white;
  float r;
  csVector3 p;
  for (r = 0 ; r <= 1 ; r += .001)
  {
    np->Calculate (r);
    np->GetInterpolatedPosition (p);
    int x = int ((p.x-tl.x)*dim / (br.x-tl.x));
    int y = int ((p.z-tl.y)*dim / (br.y-tl.y));
    if (x > 0 && x < dim && y > 0 && y < dim)
      demo->G2D->DrawPixel (x, y, col);
  }
  float* px, * py, * pz;
  px = np->GetDimensionValues (0);
  py = np->GetDimensionValues (1);
  pz = np->GetDimensionValues (2);
  float* fx, * fy, * fz;
  fx = np->GetDimensionValues (6);
  fy = np->GetDimensionValues (7);
  fz = np->GetDimensionValues (8);
  int j;
  for (j = 0 ; j < np->GetNumPoints () ; j++)
  {
    int col = demo->col_red;
    if (hi && selpoint == j) col = demo->col_green;
    int x = int ((px[j]-tl.x)*dim/(br.x-tl.x));
    int y = int ((pz[j]-tl.y)*dim/(br.y-tl.y));
    if (x > 0 && x < dim && y > 0 && y < dim)
    {
      demo->G2D->DrawPixel (x, y, col);
      demo->G2D->DrawPixel (x-1, y, col);
      demo->G2D->DrawPixel (x+1, y, col);
      demo->G2D->DrawPixel (x, y-1, col);
      demo->G2D->DrawPixel (x, y+1, col);
      if (hi && selpoint == j)
      {
        csVector3 forward (fx[j], fy[j], fz[j]);
        forward.Normalize ();
        forward *= 20.;
        demo->G2D->DrawLine (x, y, int (x+forward.x), int (y-forward.z),
      	  demo->col_cyan);
      }
    }
  }
}

void DemoSequenceManager::DrawSelPoint (
	const csVector3& pos, const csVector3& forward,
	const csVector2& tl, const csVector2& br,
	int dim, int col, float fwlen)
{
  int x = int ((pos.x-tl.x)*dim/(br.x-tl.x));
  int y = int ((pos.z-tl.y)*dim/(br.y-tl.y));
  if (x > 0 && x < dim && y > 0 && y < dim)
  {
    demo->G2D->DrawPixel (x, y, col);
    demo->G2D->DrawPixel (x-1, y-1, col);
    demo->G2D->DrawPixel (x-2, y-2, col);
    demo->G2D->DrawPixel (x+1, y+1, col);
    demo->G2D->DrawPixel (x+2, y+2, col);
    demo->G2D->DrawPixel (x+1, y-1, col);
    demo->G2D->DrawPixel (x+2, y-2, col);
    demo->G2D->DrawPixel (x-1, y+1, col);
    demo->G2D->DrawPixel (x-2, y+2, col);
    csVector3 f = forward;
    f.Normalize ();
    f *= fwlen;
    demo->G2D->DrawLine (x, y, int (x+f.x), int (y-f.z), col);
  }
}

void DemoSequenceManager::DebugDrawPaths (cs_time current_time,
	const char* hilight, const csVector2& tl, const csVector2& br,
	int selpoint)
{
  int i;
  int len = pathForMesh.Length ();

  //=====
  // Draw the border around the map.
  //=====
  int dim = demo->G2D->GetHeight ()-10;
  demo->G2D->DrawLine (0, 0, dim, 0, demo->col_cyan);
  demo->G2D->DrawLine (0, dim, dim, dim, demo->col_cyan);
  demo->G2D->DrawLine (0, 0, 0, dim, demo->col_cyan);
  demo->G2D->DrawLine (dim, 0, dim, dim, demo->col_cyan);

  //=====
  // Get the current selected path.
  //=====
  cs_time start, total, seltime = 0;
  csNamedPath* selnp = NULL;
  if (hilight) selnp = GetSelectedPath (hilight, start, total);
  if (selnp)
  {
    // Calculate where we are in time on the selected path.
    float t = selnp->GetTimeValue (selpoint);
    seltime = cs_time (start + total*t);
  }

  //=====
  // Draw all active paths.
  //=====
  i = 0;
  while (i < len)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    bool hi = (pfm->path == selnp);
    DebugDrawPath (pfm->path, hi, tl, br, selpoint);
    i++;
  }
  if (do_camera_path && camera_path)
  {
    bool hi = (camera_path == selnp);
    DebugDrawPath (camera_path, hi, tl, br, selpoint);
  }

  //=====
  // Indicate the current camera point on all paths.
  // In addition also indicate points on the other paths
  // which correspond with the current selected point on the selected path.
  //=====
  i = 0;
  while (i < len)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    bool hi = (pfm->path == selnp);

    // Fetch the current time, make sure we take account of suspension.
    cs_time ct = current_time;
    if (suspended) ct = suspend_time;

    // Calculate where we are on this path at the moment.
    // r should be between 0 and 1.
    float r = float (ct - pfm->start_path_time)
    	/ float (pfm->total_path_time);
    if (r >= 1) r = 1;
    pfm->path->Calculate (r);
    // We are going to show both the position as the forward vector.
    csVector3 pos, forward;
    pfm->path->GetInterpolatedPosition (pos);
    pfm->path->GetInterpolatedForward (forward);
    DrawSelPoint (pos, forward, tl, br, dim, demo->col_yellow, 20);

    // If there is a hilighted path and we are not busy drawing the hilighted
    // path then we will draw an additional point on this path to indicate
    // where this path will be when the selected path is at the selected point.
    if (!hi && selnp)
    {
      r = float (seltime - pfm->start_path_time)
      	/ float (pfm->total_path_time);
      if (r >= 0 && r <= 1)
      {
	pfm->path->Calculate (r);
	pfm->path->GetInterpolatedPosition (pos);
	pfm->path->GetInterpolatedForward (forward);
        DrawSelPoint (pos, forward, tl, br, dim, demo->col_cyan, 10);
      }
    }

    i++;
  }

  if (do_camera_path && camera_path)
  {
    bool hi = (camera_path == selnp);
    cs_time ct = current_time;
    if (suspended) ct = suspend_time;
    float r = GetCameraIndex (ct);
    if (r >= 1) r = 1;
    camera_path->Calculate (r);
    csVector3 pos, forward;
    camera_path->GetInterpolatedPosition (pos);
    camera_path->GetInterpolatedForward (forward);
    int col = demo->col_yellow;
    DrawSelPoint (pos, forward, tl, br, dim, demo->col_yellow, 20);

    // If there is a hilighted path and we are not busy drawing the hilighted
    // path then we will draw an additional point on this path to indicate
    // where this path will be when the selected path is at the selected point.
    if (!hi && selnp)
    {
      r = GetCameraIndex (seltime);
      if (r >= 0 && r <= 1)
      {
	camera_path->Calculate (r);
	camera_path->GetInterpolatedPosition (pos);
	camera_path->GetInterpolatedForward (forward);
        DrawSelPoint (pos, forward, tl, br, dim, demo->col_cyan, 10);
      }
    }
  }
}

void DemoSequenceManager::SelectPreviousPath (char* hilight)
{
  csNamedPath* np = GetSelectedPath (hilight);
  if (!np)
  {
    if (camera_path) strcpy (hilight, camera_path->GetName ());
    return;
  }
  if (np == camera_path) return;
  else
  {
    int i;
    for (i = 0 ; i < pathForMesh.Length () ; i++)
    {
      PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
      if (pfm->path == np)
      {
        if (i == 0)
	{
	  if (camera_path) strcpy (hilight, camera_path->GetName ());
	  return;
	}
	else
	{
          pfm = (PathForMesh*)pathForMesh[i-1];
	  strcpy (hilight, pfm->path->GetName ());
	}
	return;
      }
    }
    // We can't find the path. Switch back to camera.
    if (camera_path) strcpy (hilight, camera_path->GetName ());
    return;
  }
}

void DemoSequenceManager::SelectNextPath (char* hilight)
{
  csNamedPath* np = GetSelectedPath (hilight);
  if (!np)
  {
    if (camera_path) strcpy (hilight, camera_path->GetName ());
    return;
  }
  if (np == camera_path)
  {
    // Fetch the first non-camera path.
    if (pathForMesh.Length () <= 0) return;	// Do nothing.
    PathForMesh* pfm = (PathForMesh*)pathForMesh[0];
    strcpy (hilight, pfm->path->GetName ());
    return;
  }
  else
  {
    int i;
    for (i = 0 ; i < pathForMesh.Length () ; i++)
    {
      PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
      if (pfm->path == np)
      {
        if (i != pathForMesh.Length ()-1)
	{
          pfm = (PathForMesh*)pathForMesh[i+1];
	  strcpy (hilight, pfm->path->GetName ());
	}
	return;
      }
    }
    // We can't find the path. Switch back to camera.
    if (camera_path) strcpy (hilight, camera_path->GetName ());
    return;
  }
}

csNamedPath* DemoSequenceManager::GetSelectedPath (const char* hilight)
{
  cs_time s, t;
  return GetSelectedPath (hilight, s, t);
}

csNamedPath* DemoSequenceManager::GetSelectedPath (const char* hilight,
	cs_time& start, cs_time& total)
{
  if (do_camera_path && camera_path)
  {
    bool hi = (hilight && !strcmp (camera_path->GetName (), hilight));
    if (hi)
    {
      start = start_camera_path_time;
      total = total_camera_path_time;
      return camera_path;
    }
  }

  int i = 0;
  int len = pathForMesh.Length ();
  while (i < len)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    csNamedPath* np = pfm->path;
    bool hi = (hilight && !strcmp (np->GetName (), hilight));
    if (hi)
    {
      start = pfm->start_path_time;
      total = pfm->total_path_time;
      return np;
    }
    i++;
  }
  return NULL;
}

//-----------------------------------------------------------------------------
