/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __CS_DPMESH_H__
#define __CS_DPMESH_H__

#include "ivideo/graph3d.h"
struct iClipper2D;

void DefaultDrawPolygonMesh (
  G3DPolygonMesh& mesh,
  iGraphics3D* g3d,
  const csReversibleTransform& o2c,
  iClipper2D* clipper, bool lazyclip,
  float aspect,
  int width2,
  int height2);

#endif // __CS_DPMESH_H__
