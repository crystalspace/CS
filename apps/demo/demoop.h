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

#ifndef __DEMOOP_H__
#define __DEMOOP_H__

#include <stdarg.h>
#include "csutil/scf.h"
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
public:
  SCF_DECLARE_IBASE;
  StandardOp () { SCF_CONSTRUCT_IBASE (0); }
  virtual void CleanupSequences () { }
protected:
  virtual ~StandardOp () { SCF_DESTRUCT_IBASE(); }
};

/**
 * A test operation for the sequence manager.
 */
class TestOp : public StandardOp
{
public:
  virtual void Do (csTicks dt, iBase* params);
};

/**
 * A fade operation for the sequence manager.
 */
class FadeOp : public StandardOp
{
private:
  float start_fade, end_fade;
  csTicks total_fade_time;

public:
  FadeOp (float sf, float ef, csTicks tft) :
  	start_fade (sf), end_fade (ef), total_fade_time (tft)
  {
  }

  virtual void Do (csTicks dt, iBase* params);
};

/**
 * A rotate particle operation.
 */
class RotatePartOp : public StandardOp
{
private:
  csTicks total_rotate_time;
  float angle_speed;
  iMeshWrapper* mesh;

public:
  RotatePartOp (const char* meshname, csTicks total, float aspeed);

  virtual void Do (csTicks dt, iBase* params);
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
  virtual void Do (csTicks dt, iBase* params);
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
  virtual void Do (csTicks dt, iBase* params);
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
  virtual void Do (csTicks dt, iBase* params);
};

/**
 * An operation to attach an object to an already running path.
 */
class AttachOp : public StandardOp
{
private:
  iMeshWrapper* mesh;
  csNamedPath* path;

public:
  AttachOp (const char* meshName, const char* pathName);
  virtual void Do (csTicks dt, iBase* params);
};

/**
 * An operation to attach a path to an object (mesh or camera).
 */
class PathOp : public StandardOp
{
private:
  iMeshWrapper* mesh;
  csNamedPath* path;
  csTicks total_path_time;

public:
  PathOp (csTicks t, const char* meshName, const char* pathName);
  virtual void Do (csTicks dt, iBase* params);
};

/**
 * Recurse a sequence. Won't IncRef() it so clean-up works
 * properly.
 */
class RecurseOp : public StandardOp
{
private:
  /**
   * Sequence to run.
   * IncRef() is avoided intentionally. Otherwise a recursing
   * sequence will own itself and prevent proper clean-up.
   */
  iSequence* seq;
  /// sequence manager
  csRef<iSequenceManager> seqmgr;
public:
  RecurseOp (iSequence* sequence, csRef<iSequenceManager> manager);
  virtual void Do (csTicks dt, iBase* params);
};

#endif // __DEMOOP_H__

