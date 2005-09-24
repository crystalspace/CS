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
#include "plugins/engine/3d/meshlod.h"
#include "iengine/sharevar.h"



csStaticLODMesh::csStaticLODMesh ()
  : scfImplementationType (this), lod_m (0), lod_a (1)
{
}

csStaticLODMesh::~csStaticLODMesh ()
{
}

void csStaticLODMesh::ClearLODListeners ()
{
  if (lod_varm)
  {
    lod_varm->RemoveListener (lod_varm_listener);
    lod_varm_listener = 0;
    lod_varm = 0;
  }
  if (lod_vara)
  {
    lod_vara->RemoveListener (lod_vara_listener);
    lod_vara_listener = 0;
    lod_vara = 0;
  }
}

void csStaticLODMesh::SetLOD (float m, float a)
{
  ClearLODListeners ();
  lod_m = m;
  lod_a = a;
}

void csStaticLODMesh::GetLOD (float& m, float& a) const
{
  m = lod_m;
  a = lod_a;
}

void csStaticLODMesh::SetLOD (iSharedVariable* varm, iSharedVariable* vara)
{
  ClearLODListeners ();
  lod_varm = varm;
  lod_vara = vara;
  lod_varm_listener = csPtr<csLODListener> (new csLODListener (&lod_m));
  lod_varm->AddListener (lod_varm_listener);
  lod_vara_listener = csPtr<csLODListener> (new csLODListener (&lod_a));
  lod_vara->AddListener (lod_vara_listener);
  lod_m = varm->Get ();
  lod_a = vara->Get ();
}

int csStaticLODMesh::GetLODPolygonCount (float lod) const
{
  // @@@ Not implemented yet.
  return 0;
}

//----------------------------------------------------------------------------


csStaticLODFactoryMesh::csStaticLODFactoryMesh ()
  : scfImplementationType (this), lod_m (0), lod_a (1)
{
}

csStaticLODFactoryMesh::~csStaticLODFactoryMesh ()
{
}

void csStaticLODFactoryMesh::SetLOD (float m, float a)
{
  lod_m = m;
  lod_a = a;
  lod_varm = 0;
  lod_vara = 0;
}

void csStaticLODFactoryMesh::GetLOD (float& m, float& a) const
{
  m = lod_m;
  a = lod_a;
}

void csStaticLODFactoryMesh::SetLOD (iSharedVariable* varm,
	iSharedVariable* vara)
{
  lod_varm = varm;
  lod_vara = vara;
  lod_m = varm->Get ();
  lod_a = vara->Get ();
}


