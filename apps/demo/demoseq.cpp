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
#include "imesh/thing/polygon.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
#include "imesh/particle.h"
#include "isys/plugin.h"
#include "iutil/objreg.h"
#include "csutil/cscolor.h"
#include "csgeom/path.h"
#include "cstool/csfxscr.h"

//-----------------------------------------------------------------------------

Demo* DemoSequenceManager::demo;
DemoSequenceManager* DemoSequenceManager::demoseq;

DemoSequenceManager::DemoSequenceManager (Demo* demo)
{
  DemoSequenceManager::demo = demo;
  demoseq = this;
  iObjectRegistry* object_reg = demo->GetObjectRegistry ();
  iPluginManager* plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  seqmgr = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.utilities.sequence",
  	"Sequence", iSequenceManager);
  if (!seqmgr)
  {
    demo->Printf (CS_MSG_FATAL_ERROR, "Could not load sequence manager plugin!\n");
    exit (0);
  }

  do_fade = false;
  fade_value = 0;
  suspended = true;
  suspend_one_frame = false;
  main_sequence = NULL;
}

DemoSequenceManager::~DemoSequenceManager ()
{
  Clear ();
  int i;
  for (i = 0 ; i < paths.Length () ; i++)
  {
    csNamedPath* np = (csNamedPath*)paths[i];
    delete np;
  }
  paths.DeleteAll ();
  if (seqmgr) seqmgr->DecRef ();
  if (main_sequence) main_sequence->DecRef ();
}

void DemoSequenceManager::Clear ()
{
  seqmgr->Clear ();
  int i;
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    delete pfm;
  }
  pathForMesh.DeleteAll ();
  for (i = 0 ; i < meshRotation.Length () ; i++)
  {
    MeshRotation* mrot = (MeshRotation*)meshRotation[i];
    if (mrot->particle) mrot->particle->DecRef ();
    delete mrot;
  }
  meshRotation.DeleteAll ();
}

void DemoSequenceManager::Setup (const char* sequenceFileName)
{
  if (main_sequence) main_sequence->DecRef ();
  DemoSequenceLoader* loader = new DemoSequenceLoader (
  	DemoSequenceManager::demo, this, seqmgr, sequenceFileName);
  main_sequence = loader->GetSequence ("main");
  main_sequence->IncRef ();
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
  int i;
  for (i = 0 ; i < paths.Length () ; i++)
  {
    csNamedPath* np = (csNamedPath*)paths[i];
    delete np;
  }
  paths.DeleteAll ();
  if (main_sequence) { main_sequence->DecRef (); main_sequence = NULL; }
  do_fade = false;
  fade_value = 0;
  suspended = true;
  suspend_one_frame = false;
  Setup (sequenceFileName);
}

void DemoSequenceManager::TimeWarp (csTime dt, bool restart)
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
  csTime current_time = seqmgr->GetMainTime ();
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
  	csTime total_fade_time, csTime already_elapsed)
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
  int i;
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    if (pfm->path == path)
    {
      pfm->mesh = mesh;
      return;
    }
  }
}

void DemoSequenceManager::SetupPath (csNamedPath* path,
	iMeshWrapper* mesh,
	csTime total_path_time,
  	csTime already_elapsed)
{
  PathForMesh* pfm = new PathForMesh ();
  pfm->path = path;
  pfm->mesh = mesh;
  pfm->total_path_time = total_path_time;
  pfm->start_path_time = seqmgr->GetMainTime ()-already_elapsed;
  pathForMesh.Push (pfm);
}

void DemoSequenceManager::ControlPaths (iCamera* camera, csTime elapsed_time)
{
  if (suspended) return;
  csTime current_time = seqmgr->GetMainTime ();
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
    if (pfm->mesh)
    {
      iMovable* movable = pfm->mesh->GetMovable ();
      movable->SetPosition (pos);
      movable->GetTransform ().LookAt (forward.Unit (), up.Unit ());
      movable->UpdateMove ();
      pfm->mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
    }
    else
    {
      camera->GetTransform ().SetOrigin (pos);
      camera->GetTransform ().LookAt (forward.Unit (), up.Unit ());
    }
    if (!do_path)
    {
      delete pfm;
      pathForMesh.Delete (i);
      len--;
    }
    else i++;
  }

  i = 0;
  len = meshRotation.Length ();
  while (i < len)
  {
    MeshRotation* mrot = (MeshRotation*)meshRotation[i];
    if (current_time > mrot->start_time + mrot->total_time)
    {
      if (mrot->particle) mrot->particle->DecRef ();
      delete mrot;
      meshRotation.Delete (i);
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
    csTime debug_time)
{
  int i = 0;
  int len = pathForMesh.Length ();
  while (i < len)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
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
        pfm->mesh->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
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
  for (r = 0 ; r <= 1 ; r += .001)
  {
    np->Calculate (r);
    np->GetInterpolatedPosition (p);
    int x = int ((p.x-tl.x)*dim / (br.x-tl.x));
    int y = int ((p.z-tl.y)*dim / (br.y-tl.y));
    if (x > 0 && x < dim && y > 0 && y < dim)
      demo->myG2D->DrawPixel (x, y, col);
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
  int i;
  int len = pathForMesh.Length ();
  csTime current_time = seqmgr->GetMainTime ();

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
  csTime start = 0, total = 0, seltime = 0;
  csNamedPath* selnp = NULL;
  if (hilight) selnp = GetSelectedPath (hilight, start, total);
  if (selnp)
  {
    // Calculate where we are in time on the selected path.
    float t = selnp->GetTimeValue (selpoint);
    seltime = csTime (start + total*t);
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

    // Fetch the current time.
    csTime ct = current_time;

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
    strcpy (hilight, ((PathForMesh*)pathForMesh[0])->path->GetName ());
  }
}

void DemoSequenceManager::SelectLastPath (char* hilight)
{
  if (pathForMesh.Length () > 0)
  {
    strcpy (hilight, ((PathForMesh*)pathForMesh[
      pathForMesh.Length ()-1])->path->GetName ());
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
  int i;
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    if (pfm->path == np)
    {
      if (i == 0)
	return;
      else
      {
        pfm = (PathForMesh*)pathForMesh[i-1];
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
  int i;
  for (i = 0 ; i < pathForMesh.Length () ; i++)
  {
    PathForMesh* pfm = (PathForMesh*)pathForMesh[i];
    if (pfm->path == np)
    {
      if (i < pathForMesh.Length ()-1)
      {
        pfm = (PathForMesh*)pathForMesh[i+1];
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
  csTime s, t;
  return GetSelectedPath (hilight, s, t);
}

csNamedPath* DemoSequenceManager::GetSelectedPath (const char* hilight,
	csTime& start, csTime& total)
{
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

void DemoSequenceManager::SetupRotatePart (iMeshWrapper* mesh,
	float angle_speed, csTime total_rotate_time, csTime already_elapsed)
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
  csTime cur_time = seqmgr->GetMainTime ();
  csTime dt = cur_time-main_start_time;
  return (float (num_frames) / float (dt)) * 1000.;
}

//-----------------------------------------------------------------------------

