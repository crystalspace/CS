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

#ifndef SIMPLE_H
#define SIMPLE_H

#include <stdarg.h>
#include "cssys/sysdriv.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"

class csSector;
class csView;
class csEngine;
class csDynLight;
class csMaterialWrapper;
class csProcSky;
class csProcSkyTexture;
class csPolygon3D;

class Simple : public SysSystemDriver
{
  typedef SysSystemDriver superclass;
private:
  csSector* room;
  csView* view;
  csEngine* engine;
  csMaterialWrapper* matPlasma;

  /// the sky 
  csProcSky *sky;
  /// the six sides (front, back, left, right, up, down)
  csProcSkyTexture *sky_f, *sky_b, *sky_l, *sky_r, *sky_u, *sky_d;

  /** set texture space of poly, a size x size texture , 
   * given orig,u,ulen,v,vlen,  so that you get no ugly
   * edges (connecting to other polygons
   */
  void SetTexSpace(csPolygon3D *poly, int size, const csVector3& orig,
    const csVector3& upt, float ulen, const csVector3& vpt, float vlen);

public:
  Simple ();
  virtual ~Simple ();

  virtual bool Initialize (int argc, const char* const argv[],
    const char *iConfigName);
  virtual void NextFrame ();
  virtual bool HandleEvent (iEvent &Event);
};

#endif // SIMPLE_H

