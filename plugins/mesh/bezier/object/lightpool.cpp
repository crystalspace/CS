/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csgeom/frustum.h"
#include "lightpool.h"
#include "curvebase.h"
#include "iengine/shadows.h"

csBezierLightPatch::csBezierLightPatch ()
{
  next = prev = 0;
  num_vertices = 0;
  max_vertices = 0;
  vertices = 0;
  light = 0;
}

csBezierLightPatch::~csBezierLightPatch ()
{
  delete[] vertices;
  RemovePatch ();
}

void csBezierLightPatch::RemovePatch ()
{
  if (curve) curve->UnlinkLightPatch (this);
  shadows->DeleteShadows ();
  light_frustum = 0;
}

void csBezierLightPatch::Initialize (int n)
{
  if (n > max_vertices)
  {
    delete[] vertices;
    max_vertices = n;
    vertices = new csVector3[max_vertices];
  }

  num_vertices = n;
}

void csBezierLightPatch::SetShadowBlock (iShadowBlock* bl)
{
  shadows = bl;
}

