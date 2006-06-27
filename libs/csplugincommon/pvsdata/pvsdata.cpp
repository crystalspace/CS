/*
    Copyright (C) 2006 by Jorrit Tyberghein and Benjamin Stover

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

#include "csplugincommon/pvsdata/pvsdata.h"

csPVSNodeData::csPVSNodeData (csString* names, int total)
{
  if (total > 0)
  {
    pvs = new void*[total];
    memset ((void*) pvs, 0, sizeof(void*) * total);
    pvsnames = names;
    numRegistered = 0;
    numTotal = total;
  }
  else
  {
    pvs = NULL;
    pvsnames = NULL;
    numRegistered = numTotal = 0;
  }
}

csPVSNodeData::~csPVSNodeData ()
{
  delete[] pvs;
  delete[] pvsnames;
}

bool csPVSNodeData::PVSNamesContains(const char* name)
{
  //iVisibilityObject *vis = reg->visobj;
  //csFlags flags = reg->GetMeshWrapper ()->GetMeshObject()->GetFlags ();
  // If mesh is not staticpos or staticshape, we will not find it in
  // the pvsnames list.
  //if (!(flags & (CS_MESH_STATICPOS | CS_MESH_STATICSHAPE)))
  //  return false;
  // If pvsnames is NULL, reg's name is not in
  if (!pvsnames)
    return false;
  if (pvsnames) 
  {
    for (int i = 0; i < numTotal; i++)
    {
      if (pvsnames[i] == name)
        return true;
    }
  }
  return false;
}

void csPVSNodeData::RemoveFromPVS(void *object)
{
  for (int i = 0; i < numRegistered; i++)
  {
    if (pvs[i] == object)
    {
      pvs[i] = pvs[(numRegistered--) - 1];
      break;
    }
  }
}
