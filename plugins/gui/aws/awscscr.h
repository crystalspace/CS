/*
    Copyright (C) 2000-2001 by Christopher Nelson

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

#ifndef __CS_AWS_CSCR_H__
#define __CS_AWS_CSCR_H__

#include "cstool/proctex.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "csgeom/transfrm.h"

class awsScreenCanvas : public iAwsCanvas
{
private:
  /// Context to the screen's g2d.
  iGraphics2D *rG2D;

  /// Context to the screen's g3d.
  iGraphics3D *rG3D;
public:
  SCF_DECLARE_IBASE;

  /// Initialize this canvas with custom pointers to a valid g2d and g3d.
  awsScreenCanvas (iGraphics2D *g2d, iGraphics3D *g3d) :
    rG2D(g2d),
    rG3D(g3d)
  {
    SCF_CONSTRUCT_IBASE (0);
    g2d->IncRef ();
    g3d->IncRef ();
  }

  /// Destruct, release references.
  virtual ~awsScreenCanvas ()
  {
    rG2D->DecRef ();
    rG3D->DecRef ();
    SCF_DESTRUCT_IBASE ();
  }

  virtual void Animate (csTicks current_time)
  {
    (void)current_time; // Do nothing.
  }

  iGraphics2D *G2D () { return rG2D; }
  iGraphics3D *G3D () { return rG3D; }

  virtual void Show (
    csRect *area = 0,
    iGraphics3D *g3d = 0,
    uint8 Alpha = 0)
  {
    (void)area;
    (void)g3d;
    (void)Alpha;        // Do nothing.
  }
};

#endif // __CS_AWS_CSCR_H__
