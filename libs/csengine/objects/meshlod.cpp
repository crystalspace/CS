/*
    Copyright (C) 2003 by Jorrit Tyberghein

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
#include "csengine/meshlod.h"


SCF_IMPLEMENT_IBASE(csStaticLODMesh)
  SCF_IMPLEMENTS_INTERFACE(iLODControl)
SCF_IMPLEMENT_IBASE_END

csStaticLODMesh::csStaticLODMesh ()
{
  SCF_CONSTRUCT_IBASE (0);
  lod_m = 0;
  lod_a = 1;
}

csStaticLODMesh::~csStaticLODMesh ()
{
}

void csStaticLODMesh::SetLOD (float m, float a)
{
  lod_m = m;
  lod_a = a;
}

void csStaticLODMesh::GetLOD (float& m, float& a) const
{
  m = lod_m;
  a = lod_a;
}

int csStaticLODMesh::GetLODPolygonCount (float lod) const
{
  // @@@ Not implemented yet.
  return 0;
}

