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
#include "ivaria/view.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/light.h"
#include "iengine/camera.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/thing.h"
#include "imesh/object.h"
#include "imesh/particle.h"
#include "ivaria/reporter.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "csutil/cscolor.h"
#include "csgeom/path.h"
#include "cstool/csfxscr.h"
#include "iutil/plugin.h"

//-----------------------------------------------------------------------------

Demo* DemoSequenceManager::demo;
DemoSequenceManager* DemoSequenceManager::demoseq;

DemoSequenceManager::DemoSequenceManager (Demo* demo)
{
  DemoSequenceManager::demo = demo;
  demoseq = this;
  iObjectRegistry* object_reg = demo->object_reg;
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  seqmgr = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.utilities.sequence",
  	iSequenceManager);
  if (!seqmgr)
  {
    demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Could not load sequence manager plugin!");
    exit (0);
  }

  do_fade = false;
  fade_value = 0;
  suspended = true;
  suspend_one_frame = false;
}

DemoSequenceManager::~DemoSequenceManager ()
{
  Clear ();
}

void DemoSequenceManager::Clear ()
{
  seqmgr->Clear ();
  pathForMesh.DeleteAll ();
  meshRotation.DeleteAll ();
}

void DemoSequenceManager::Setup (const char* sequenceFileName)
{
  DemoSequenceLoader* loader = new DemoSequenceLoader (
  	DemoSequenceManager::demo, this, seqmgr, sequenceFileName);
  main_sequence = loader->GetSequence ("main");
  seqmgr->RunSequence (0, main_sequence);
  seqmgr->Resume ();
  suspended = false;
  main_start_time = seqmgr->GetMainTime ();
  num_frames = 0;
  delete loader;
}

void DemoSequenceManager::Suspend ()
{
  if (!suspended)
  {
    suspended = true;
    seqmgr->Suspend ();
  }
}

void DemoSequenceManager::Resume ()
{
  if (suspended)
  {
    suspended = false;
    seqmgr->Resume ();
  }
}

void DemoSequenceManager::Restart (const char* sequenceFileName)
{
  Clear ();
  paths.DeleteAll ();
  main_sequence = 0;
  do_fade = false;
  fade_value = 0;
  suspended = true;
  suspend_one_frame = false;
  Setup (sequenceFileName);
}

void DemoSequenceManager::TimeWarp (csTicks dt, bool restart)
{
  // Temporarily resume everything to make sure our data is ok.
  bool sus = suspended;
  Resume ();

  // If we were suspended we say to the sequence manager to
  // suspend after one frame again. But we will have to go on
  // one frame to update the screen.
  if (sus) suspend_one_frame = true;

  if (seqmgr->GetMainTime () + dt <= main_start_time)
  {
    Clear ();
    seqmgr->RunSequence (0, main_sequence);
    main_start_time = seqmgr->GetMainTime ();
    num_frames = 0;
    return;
  }

  if (restart)
  {
    dt = seqmgr->GetMainTime () + dt - main_start_time;
    Clear ();
    seqmgr->RunSequence (0, main_sequence);
    main_start_time = seqmgr->GetMainTime ();
    num_frames = 0;
    seqmgr->TimeWarp (dt, false);
    return;
  }

  seqmgr->TimeWarp (dt, false);
  if (seqmgr->IsEmpty ())
  {
    // If the sequence manager is empty we insert the main sequence
    // again.
    seqmgr->RunSequence (0, main_sequence);
  }
}


void DemoSequenceManager::Draw3DEffects (iGraphics3D* g3d)
{
  num_frames++;
  csTicks current_time = seqmgr->GetMainTime ();
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

void DemoSequenceManager::Draw2DEffects (iGraphics2D* /*g2d*/)
{
}

void DemoSequenceManager::SetupFade (float start_fade, float end_fade,
  	csTicks total_fade_time, csTicks already_elapsed)
{
  DemoSequenceManager::start_fade = start_fade;
  DemoSequenceManager::end_fade = end_fade;
  DemoSequenceManager::total_fade_time = total_fade_time;
  start_fade_time = seqmgr->GetMainTime ()-already_elapsed;
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

void DemoSequenceManager::ReplacePathObject (csNamedPath* path,
	iMeshWrapper* mesh)
{
  size_t i;
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = pathForMesh[i];
    if (pfm->path == path)
    {
      pfm->mesh = mesh;
      return;
    }
  }
}

void DemoSequenceManager::SetupPath (csNamedPath* path,
	iMeshWrapper* mesh,
	csTicks total_path_time,
  	csTicks already_elapsed)
{
  PathForMesh* pfm = new PathForMesh ();
  pfm->path = path;
  pfm->mesh = mesh;
  pfm->total_path_time = total_path_time;
  pfm->start_path_time = seqmgr->GetMainTime ()-already_elapsed;
  pathForMesh.Push (pfm);
}

void DemoSequenceManager::ControlPaths (iCamera* camera, csTicks elapsed_time)
{
  if (suspended) return;
  csTicks current_time = seqmgr->GetMainTime ();
  size_t i = 0;
  size_t len = pathForMesh.Length ();
  while (i < len)
  {
    PathForMesh* pfm = pathForMesh[i];
    float r = float (current_time - pfm->start_path_time)
    	/ float (pfm->total_path_time);
    bool do_path = true;
    if (r >= 1)
    {
      r = 1;
      do_path = false;
    }
    pfm->path->Calculate (r);
    csVector3 oldpos;
    iSector* oldsector;
    csVector3 pos, up, forward;
    if (pfm->mesh)
    {
      iMovable* movable = pfm->mesh->GetMovable ();
      oldpos = movable->GetPosition ();
      oldsector = movable->GetSectors ()->Get (0);
    }
    else
    {
      oldpos = camera->GetTransform ().GetOrigin ();
      oldsector = camera->GetSector ();
    }

    pfm->path->GetInterpolatedPosition (pos);
    pfm->path->GetInterpolatedUp (up);
    pfm->path->GetInterpolatedForward (forward);

    // See if we have to go to another sector when going through a
    // portal.
    csReversibleTransform trans;
    trans.SetOrigin (oldpos);
    bool mirror = false;
    iSector* newsector = oldsector->FollowSegment (trans,
    	pos, mirror, true);

    if (pfm->mesh)
    {
      iMovable* movable = pfm->mesh->GetMovable ();
      movable->SetPosition (pos);
      if (oldsector != newsector)
        movable->SetSector (newsector);
      movable->GetTransform ().LookAt (forward.Unit (), up.Unit ());
      movable->UpdateMove ();
    }
    else
    {
      if (oldsector != newsector)
        camera->SetSector (newsector);
      camera->GetTransform ().SetOrigin (pos);
      camera->GetTransform ().LookAt (forward.Unit (), up.Unit ());
    }
    if (!do_path)
    {
      pathForMesh.DeleteIndex (i);
      len--;
    }
    else i++;
  }

  i = 0;
  len = meshRotation.Length ();
  while (i < len)
  {
    MeshRotation* mrot = meshRotation[i];
    if (current_time > mrot->start_time + mrot->total_time)
    {
      meshRotation.DeleteIndex (i);
      len--;
    }
    else
    {
      mrot->particle->Rotate (mrot->angle_speed * float (elapsed_time/1000.));
      i++;
    }
  }

  if (suspend_one_frame) { Suspend (); suspend_one_frame = false; }
}

void DemoSequenceManager::DebugPositionObjects (iCamera* camera,
    csTicks debug_time)
{
  size_t i = 0;
  size_t len = pathForMesh.Length ();
  while (i < len)
  {
    PathForMesh* pfm = pathForMesh[i];
    float r = float (debug_time - pfm->start_path_time)
    	/ float (pfm->total_path_time);
    if (r >= 0 && r <= 1)
    {
      pfm->path->Calculate (r);
      csVector3 pos, up, forward;
      pfm->path->GetInterpolatedPosition (pos);
      pfm->path->GetInterpolatedUp (up);
      pfm->path->GetInterpolatedForward (forward);
      if (pfm->mesh)
      {
        iMovable* movable = pfm->mesh->GetMovable ();
        movable->SetPosition (pos);
        movable->GetTransform ().LookAt (forward.Unit (), up.Unit ());
        movable->UpdateMove ();
      }
      else
      {
        camera->GetTransform ().SetOrigin (pos);
        camera->GetTransform ().LookAt (forward.Unit (), up.Unit ());
      }
    }
    i++;
  }
}

void DemoSequenceManager::DebugDrawPath (csNamedPath* np, bool hi,
	const csVector2& tl, const csVector2& br, int selpoint)
{
  int dim = demo->myG2D->GetHeight ()-10;
  int col = demo->col_gray;
  if (hi) col = demo->col_white;
  float r;
  csVector3 p;
  for (r = 0 ; r <= 1 ; r += 0.001f)
  {
    np->Calculate (r);
    np->GetInterpolatedPosition (p);
    int x = int ((p.x-tl.x)*dim / (br.x-tl.x));
    int y = int ((p.z-tl.y)*dim / (br.y-tl.y));
    if (x > 0 && x < dim && y > 0 && y < dim)
      demo->myG2D->DrawPixel (x, y, col);
  }
  const float* px, * py, * pz;
  px = np->GetDimensionValues (0);
  py = np->GetDimensionValues (1);
  pz = np->GetDimensionValues (2);
  const float* fx, * fy, * fz;
  fx = np->GetDimensionValues (6);
  fy = np->GetDimensionValues (7);
  fz = np->GetDimensionValues (8);
  int j;
  for (j = 0 ; j < np->GetPointCount () ; j++)
  {
    int col = demo->col_red;
    if (hi && selpoint == j) col = demo->col_green;
    int x = int ((px[j]-tl.x)*dim/(br.x-tl.x));
    int y = int ((pz[j]-tl.y)*dim/(br.y-tl.y));
    if (x > 0 && x < dim && y > 0 && y < dim)
    {
      demo->myG2D->DrawPixel (x, y, col);
      demo->myG2D->DrawPixel (x-1, y, col);
      demo->myG2D->DrawPixel (x+1, y, col);
      demo->myG2D->DrawPixel (x, y-1, col);
      demo->myG2D->DrawPixel (x, y+1, col);
      if (hi && selpoint == j)
      {
        csVector3 forward (fx[j], fy[j], fz[j]);
        forward.Normalize ();
        forward *= 20.;
        demo->myG2D->DrawLine (x, y, int (x+forward.x), int (y-forward.z),
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
    demo->myG2D->DrawPixel (x, y, col);
    demo->myG2D->DrawPixel (x-1, y-1, col);
    demo->myG2D->DrawPixel (x-2, y-2, col);
    demo->myG2D->DrawPixel (x+1, y+1, col);
    demo->myG2D->DrawPixel (x+2, y+2, col);
    demo->myG2D->DrawPixel (x+1, y-1, col);
    demo->myG2D->DrawPixel (x+2, y-2, col);
    demo->myG2D->DrawPixel (x-1, y+1, col);
    demo->myG2D->DrawPixel (x-2, y+2, col);
    csVector3 f = forward;
    f.Normalize ();
    f *= fwlen;
    demo->myG2D->DrawLine (x, y, int (x+f.x), int (y-f.z), col);
  }
}

void DemoSequenceManager::DebugDrawPaths (iCamera* camera,
	const char* hilight, const csVector2& tl, const csVector2& br,
	int selpoint)
{
  size_t i;
  size_t len = pathForMesh.Length ();
  csTicks current_time = seqmgr->GetMainTime ();

  //=====
  // Draw the border around the map.
  //=====
  int dim = demo->myG2D->GetHeight ()-10;
  demo->myG2D->DrawLine (0, 0, dim, 0, demo->col_cyan);
  demo->myG2D->DrawLine (0, dim, dim, dim, demo->col_cyan);
  demo->myG2D->DrawLine (0, 0, 0, dim, demo->col_cyan);
  demo->myG2D->DrawLine (dim, 0, dim, dim, demo->col_cyan);

  //=====
  // Draw the current camera.
  //=====
  csVector3 campos = camera->GetTransform ().GetOrigin ();
  csVector3 camfwd = camera->GetTransform ().This2Other (csVector3 (0, 0, 1)) -
  	campos;
  DrawSelPoint (campos, camfwd, tl, br, dim, demo->col_green, 20);

  //=====
  // Get the current selected path.
  //=====
  csTicks start = 0, total = 0, seltime = 0;
  csNamedPath* selnp = 0;
  if (hilight) selnp = GetSelectedPath (hilight, start, total);
  if (selnp)
  {
    // Calculate where we are in time on the selected path.
    float t = selnp->GetTimeValue (selpoint);
    seltime = csTicks (start + total*t);
  }

  //=====
  // Draw all active paths.
  //=====
  i = 0;
  while (i < len)
  {
    PathForMesh* pfm = pathForMesh[i];
    bool hi = (pfm->path == selnp);
    DebugDrawPath (pfm->path, hi, tl, br, selpoint);
    i++;
  }

  //=====
  // Indicate the current camera point on all paths.
  // In addition also indicate points on the other paths
  // which correspond with the current selected point on the selected path.
  //=====
  i = 0;
  while (i < len)
  {
    PathForMesh* pfm = pathForMesh[i];
    bool hi = (pfm->path == selnp);

    // Fetch the current time.
    csTicks ct = current_time;

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
}

void DemoSequenceManager::SelectFirstPath (char* hilight)
{
  if (pathForMesh.Length () > 0)
  {
    strcpy (hilight, pathForMesh[0]->path->GetName ());
  }
}

void DemoSequenceManager::SelectLastPath (char* hilight)
{
  if (pathForMesh.Length () > 0)
  {
    strcpy (hilight, pathForMesh[pathForMesh.Length ()-1]->path->GetName ());
  }
}

void DemoSequenceManager::SelectPreviousPath (char* hilight)
{
  csNamedPath* np = GetSelectedPath (hilight);
  if (!np)
  {
    SelectLastPath (hilight);
    return;
  }
  size_t i;
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = pathForMesh[i];
    if (pfm->path == np)
    {
      if (i == 0)
	return;
      else
      {
        pfm = pathForMesh[i-1];
	strcpy (hilight, pfm->path->GetName ());
      }
      return;
    }
  }
  // We can't find the path. Switch back to last path.
  SelectLastPath (hilight);
  return;
}

void DemoSequenceManager::SelectNextPath (char* hilight)
{
  csNamedPath* np = GetSelectedPath (hilight);
  if (!np)
  {
    SelectFirstPath (hilight);
    return;
  }
  size_t i;
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = pathForMesh[i];
    if (pfm->path == np)
    {
      if (i < pathForMesh.Length ()-1)
      {
        pfm = pathForMesh[i+1];
	strcpy (hilight, pfm->path->GetName ());
      }
      return;
    }
  }
  // We can't find the path. Switch back to first path.
  SelectFirstPath (hilight);
  return;
}

csNamedPath* DemoSequenceManager::GetSelectedPath (const char* hilight)
{
  csTicks s, t;
  return GetSelectedPath (hilight, s, t);
}

csNamedPath* DemoSequenceManager::GetSelectedPath (const char* hilight,
	csTicks& start, csTicks& total)
{
  size_t i = 0;
  size_t len = pathForMesh.Length ();
  while (i < len)
  {
    PathForMesh* pfm = pathForMesh[i];
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
  return 0;
}

void DemoSequenceManager::SetupRotatePart (iMeshWrapper* mesh,
	float angle_speed, csTicks total_rotate_time, csTicks already_elapsed)
{
  MeshRotation* mrot = new MeshRotation ();
  mrot->particle = SCF_QUERY_INTERFACE (mesh->GetMeshObject (), iParticle);
  if (!mrot->particle)
  {
    delete mrot;
    return;
  }
  mrot->mesh = mesh;
  mrot->total_time = total_rotate_time;
  mrot->start_time = seqmgr->GetMainTime ()-already_elapsed;
  mrot->angle_speed = angle_speed;
  meshRotation.Push (mrot);
}

float DemoSequenceManager::GetFPS ()
{
  csTicks cur_time = seqmgr->GetMainTime ();
  csTicks dt = cur_time-main_start_time;
  return (float (num_frames) / float (dt)) * 1000.0f;
}

//-----------------------------------------------------------------------------

