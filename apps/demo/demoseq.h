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
class csVector3;
class csVector2;
struct iGraphics3D;
struct iGraphics2D;
struct iCamera;
struct iSector;
struct iMeshWrapper;
struct iParticle;

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
  iMeshWrapper* mesh;	// If NULL this path controls a camera.
  cs_time total_path_time;
  cs_time start_path_time;
};

/**
 * A rotation for a mesh as a particle.
 */
class MeshRotation
{
public:
  iMeshWrapper* mesh;
  iParticle* particle;
  cs_time total_time;
  cs_time start_time;
  float angle_speed;
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

  //=====
  // For rotation.
  //=====
  csVector meshRotation;

private:
  void DebugDrawPath (csNamedPath* np, bool hi,
	const csVector2& tl, const csVector2& br, int selpoint);
  void DrawSelPoint (
	const csVector3& pos, const csVector3& forward,
	const csVector2& tl, const csVector2& br,
	int dim, int col, float fwlen);
  void Clear ();

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

  /**
   * Time warp. If restart==true we restart the sequence manager
   * when we go backwards in time.
   */
  void TimeWarp (cs_time dt, bool restart = false);

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
  void Draw3DEffects (iGraphics3D* g3d);

  /**
   * Perform 2D special effects operations. This should
   * be called while in 2D mode.
   */
  void Draw2DEffects (iGraphics2D* g2d);

  /**
   * Control all paths (including the one for the camera).
   * This should be called from within NextFrame().
   */
  void ControlPaths (iCamera* camera, cs_time elapsed_time);

  /**
   * Correctly position all objects (including the camera)
   * while in suspended mode.
   */
  void DebugPositionObjects (iCamera* camera);

  /**
   * For debugging, draw all active paths on screen. Draw the
   * path with the given name highlighted.
   */
  void DebugDrawPaths (iCamera* camera,
	const char* hilight, const csVector2& tl,
	const csVector2& br, int selpoint);

  /**
   * Get the selected path.
   */
  csNamedPath* GetSelectedPath (const char* hilight);

  /**
   * Get the start and total time for the selected path.
   */
  csNamedPath* GetSelectedPath (const char* hilight, cs_time& start,
  	cs_time& total);

  /**
   * Select first path.
   */
  void SelectFirstPath (char* hilight);

  /**
   * Select last path.
   */
  void SelectLastPath (char* hilight);

  /**
   * Select previous path.
   */
  void SelectPreviousPath (char* hilight);

  /**
   * Select next path.
   */
  void SelectNextPath (char* hilight);

  /**
   * Start a rotating particle effect.
   */
  void SetupRotatePart (iMeshWrapper* mesh, float angle_speed,
  	cs_time total_rotate_time, cs_time already_elapsed);
  /**
   * Start a fade effect.
   */
  void SetupFade (float start_fade, float end_fade,
  	cs_time total_fade_time, cs_time already_elapsed);

  /**
   * Setup a path. If mesh == NULL we are setting up a path for the camera.
   */
  void SetupPath (csNamedPath* path, iMeshWrapper* mesh,
  	cs_time total_path_time,
  	cs_time already_elapsed);

  /**
   * See if some path is already running. If so then replace the object
   * that is attached to the running path with this one (mesh can be NULL
   * for the camera).
   */
  void ReplacePathObject (csNamedPath* path, iMeshWrapper* mesh);

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
};

#endif // DEMOSEQ_H

