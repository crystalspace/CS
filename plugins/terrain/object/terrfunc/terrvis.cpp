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
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/poly2d.h"
#include "csgeom/vector3.h"
#include "csutil/garray.h"
#include "isys/system.h"
#include "terrvis.h"
#include "qint.h"

//------------------------------------------------------------------------

int csTerrainQuad::global_visnr = 0;

csTerrainQuad::csTerrainQuad ()
{
  children[0] = NULL;
  children[1] = NULL;
  children[2] = NULL;
  children[3] = NULL;
  visnr = -1;
}

csTerrainQuad::~csTerrainQuad ()
{
  delete children[0];
  delete children[1];
  delete children[2];
  delete children[3];
}

void csTerrainQuad::Build (int depth)
{
  if (depth <= 0) return;
  children[0] = new csTerrainQuad ();
  children[0]->Build (depth-1);
  children[1] = new csTerrainQuad ();
  children[1]->Build (depth-1);
  children[2] = new csTerrainQuad ();
  children[2]->Build (depth-1);
  children[3] = new csTerrainQuad ();
  children[3]->Build (depth-1);
}

//------------------------------------------------------------------------

