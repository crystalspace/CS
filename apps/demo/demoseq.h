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

#ifndef DEMOSEQ_H
#define DEMOSEQ_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/path.h"
#include "ivaria/sequence.h"

class Demo;
struct iGraphics3D;
struct iGraphics2D;
struct iCamera;
struct iSector;
struct iMeshWrapper;

/**
 * A subclass of csPath so that paths can have a name.
 */
class csNamedPath : public csPath
{
private:
  char* name;

public:
  csNamedPath (int p, const char* name) : csPath (p)
  {
    csNamedPath::name = strnew (name);
  }
  virtual ~csNamedPath () { delete[] name; }
  char* GetName () const { return name; }
};

/**
 * A path connected with a mesh.
 */
class PathForMesh
{
public:
  csNamedPath* path;
  iMeshWrapper* mesh;
  cs_time total_path_time;
  cs_time start_path_time;
};

/**
 * The Demo Sequence Manager.
 */
class DemoSequenceManager
{
public:
  static Demo* demo;
  static DemoSequenceManager* demoseq;

private:
  iSequenceManager* seqmgr;
  iSequence* main_sequence;
  bool suspended;
  bool suspend_one_frame;
  cs_time suspend_time;

  //=====
  // For fading.
  //=====
  bool do_fade;
  float start_fade;
  float end_fade;
  float fade_value;	// Current fade value.
  cs_time total_fade_time;
  cs_time start_fade_time;

  //=====
  // For path handling.
  //=====
  csVector paths;
  csVector pathForMesh;	// PathForMesh objects.
  // Camera path handling.
  bool do_camera_path;
  csNamedPath* camera_path;
  cs_time total_camera_path_time;
  cs_time start_camera_path_time;

private:
  void DebugDrawPath (csNamedPath* np, bool hi,
	const csVector2& tl, const csVector2& br, int selpoint);

public:
  DemoSequenceManager (Demo* demo);
  virtual ~DemoSequenceManager ();

  /// Suspend everything.
  void Suspend ();

  /// Is everything suspended?
  bool IsSuspended () { return suspended; }

  /// Resume.
  void Resume ();

  /// Restart the sequence manager from zero.
  void Restart (const char* sequenceFileName);

  /// Time warp.
  void TimeWarp (cs_time dt);

  /**
   * Initialize the sequence manager and setup all sequences
   * from the given file.
   */
  void Setup (const char* sequenceFileName);

  /**
   * Perform 3D special effects operations. This should
   * be called while in 3D mode and after drawing the
   * world (view->Draw()).
   */
  void Draw3DEffects (iGraphics3D* g3d, cs_time current_time);

  /**
   * Perform 2D special effects operations. This should
   * be called while in 2D mode.
   */
  void Draw2DEffects (iGraphics2D* g2d, cs_time current_time);

  /**
   * Control all paths (including the one for the camera).
   * This should be called from within NextFrame().
   */
  void ControlPaths (iCamera* camera, cs_time current_time);

  /**
   * For debugging, draw all active paths on screen. Draw the
   * path with the given name highlighted.
   */
  void DebugDrawPaths (cs_time current_time,
	const char* hilight, const csVector2& tl,
	const csVector2& br, int selpoint);

  /**
   * Get the selected path.
   */
  csNamedPath* GetSelectedPath (const char* hilight);

  /**
   * Select previous path.
   */
  void SelectPreviousPath (char* hilight);

  /**
   * Select next path.
   */
  void SelectNextPath (char* hilight);

  /**
   * Start a fade effect.
   */
  void SetupFade (float start_fade, float end_fade,
  	cs_time total_fade_time, cs_time already_elapsed);

  /**
   * Setup the camera path.
   */
  void SetupCameraPath (csNamedPath* path, cs_time total_camera_path_time,
  	cs_time already_elapsed);
  
  /**
   * Setup a path for a mesh.
   */
  void SetupMeshPath (csNamedPath* path, iMeshWrapper* mesh,
  	cs_time total_path_time,
  	cs_time already_elapsed);
  
  /**
   * Register a named path with the sequencer.
   */
  void RegisterPath (csNamedPath* path)
  {
    paths.Push (path);
  }

  /**
   * Find a path by name.
   */
  csNamedPath* FindPath (const char* name)
  {
    int i;
    for (i = 0 ; i < paths.Length () ; i++)
    {
      csNamedPath* p = (csNamedPath*)paths[i];
      if (!strcmp (p->GetName (), name)) return p;
    }
    return NULL;
  }

  /**
   * Get the path which is currently controlling the camera.
   */
  csNamedPath* GetCameraPath () { return camera_path; }

  /**
   * Get the current index of the camera path (0..1).
   */
  float GetCameraIndex (cs_time current_time)
  {
    return float (current_time - start_camera_path_time)
    	/ float (total_camera_path_time);
  }

  /**
   * Get the absolute time point which corresponds to the camera path
   * index (0..1).
   */
  cs_time GetCameraTimePoint (float r)
  {
    return cs_time (r * total_camera_path_time + start_camera_path_time);
  }
};

#endif // DEMOSEQ_H

