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

#ifndef DEMOOP_H
#define DEMOOP_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/path.h"
#include "ivaria/sequence.h"

class Demo;
class csNamedPath;
struct iGraphics3D;
struct iGraphics2D;
struct iCamera;
struct iSector;
struct iMeshWrapper;

/**
 * The superclass of all sequence operations.
 */
class StandardOp : public iSequenceOperation
{
private:
  int ref_count;

protected:
  virtual ~StandardOp () { }

public:
  StandardOp () : ref_count (1) { }
  virtual void IncRef () { ref_count++; }
  virtual void DecRef () { if (--ref_count <= 0) delete this; }
};

/**
 * A test operation for the sequence manager.
 */
class TestOp : public StandardOp
{
public:
  virtual void Do (cs_time dt)
  {
    printf ("dt=%ld\n", dt); fflush (stdout);
  }
};

/**
 * A fade operation for the sequence manager.
 */
class FadeOp : public StandardOp
{
private:
  float start_fade, end_fade;
  cs_time total_fade_time;

public:
  FadeOp (float sf, float ef, cs_time tft) :
  	start_fade (sf), end_fade (ef), total_fade_time (tft)
  {
  }

  virtual void Do (cs_time dt);
};

/**
 * A rotate particle operation.
 */
class RotatePartOp : public StandardOp
{
private:
  cs_time total_rotate_time;
  float angle_speed;
  iMeshWrapper* mesh;

public:
  RotatePartOp (const char* meshname, cs_time total, float aspeed);

  virtual void Do (cs_time dt);
};

/**
 * An operation to attach a path to the camera.
 */
class CameraPathOp : public StandardOp
{
private:
  csNamedPath* path;
  cs_time total_camera_path_time;

public:
  CameraPathOp (cs_time t, const char* pathName);
  virtual void Do (cs_time dt);
};

/**
 * An operation to show and move a mesh object.
 */
class SetupMeshOp : public StandardOp
{
private:
  iMeshWrapper* mesh;
  iSector* sector;
  csVector3 pos;

public:
  SetupMeshOp (const char* meshName, const char* sectName, const csVector3& p);
  virtual void Do (cs_time dt);
};

/**
 * An operation to show a mesh object.
 */
class ShowMeshOp : public StandardOp
{
private:
  iMeshWrapper* mesh;

public:
  ShowMeshOp (const char* meshName);
  virtual void Do (cs_time dt);
};

/**
 * An operation to hide a mesh object.
 */
class HideMeshOp : public StandardOp
{
private:
  iMeshWrapper* mesh;

public:
  HideMeshOp (const char* meshName);
  virtual void Do (cs_time dt);
};

/**
 * An operation to attach a path to a mesh object.
 */
class MeshPathOp : public StandardOp
{
private:
  iMeshWrapper* mesh;
  csNamedPath* path;
  cs_time total_path_time;

public:
  MeshPathOp (cs_time t, const char* meshName, const char* pathName);
  virtual void Do (cs_time dt);
};

#endif // DEMOOP_H

