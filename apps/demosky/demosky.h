/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#ifndef DEMOSKY_H
#define DEMOSKY_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

class csProcSky;
class csProcSkyTexture;
class Flock;
struct iSector;
struct iSector;
struct iView;
struct iEngine;
struct iDynLight;
struct iMaterialWrapper;
struct iPolygon3D;
struct iFont;
struct iMeshWrapper;
struct iMaterialWrapper;
struct iLoader;

class Simple : public SysSystemDriver
{
  typedef SysSystemDriver superclass;
private:
  iSector* room;
  iView* view;
  iEngine* engine;
  iMaterialWrapper* matPlasma;
  iFont *font;
  iLoader *LevelLoader;
  iGraphics2D *myG2D;
  iGraphics3D *myG3D;

  /// the flock of birds
  Flock *flock;

  /// the sky 
  csProcSky *sky;
  /// the six sides (front, back, left, right, up, down)
  csProcSkyTexture *sky_f, *sky_b, *sky_l, *sky_r, *sky_u, *sky_d;

  /** set texture space of poly, a size x size texture , 
   * given orig,u,ulen,v,vlen,  so that you get no ugly
   * edges (connecting to other polygons
   */
  void SetTexSpace(csProcSkyTexture *skytex, iPolygon3D *poly, int size, 
    const csVector3& orig, const csVector3& upt, float ulen, 
    const csVector3& vpt, float vlen);

public:
  Simple ();
  virtual ~Simple ();

  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame ();
  virtual bool HandleEvent (iEvent &Event);
};


/** A flock of birds, a collection of sprites*/
class Flock {
  /// nr of sprites
  int nr;
  /// the sprites
  iMeshWrapper **spr;
  /// sprite speed, accel
  csVector3 *speed, *accel;

  /// flock focus position
  csVector3 focus;
  /// focus speed, accel
  csVector3 foc_speed, foc_accel;

public:
  /// create, nr , texture
  Flock(iEngine *engine, int num, iMaterialWrapper *mat, iSector *sector);
  ///
  ~Flock();
  /// call each frame to move birds
  void Update(cs_time elapsed_time);
};

#endif // DEMOSKY_H

