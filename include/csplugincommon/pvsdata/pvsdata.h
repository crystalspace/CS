/*
    Copyright (C) 2002 by Jorrit Tyberghein and Benjamin Stover

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

#ifndef __CS_PVSDATA_H
#define __CS_PVSDATA_H

#include "cssysdef.h"
#include "csutil/array.h"
#include "csutil/csstring.h"

class csStaticKDTree;
class iObjectRegistry;

/// Data at every node for the static KD tree
class csPVSNodeData
{
public:
  csPVSNodeData (csString* names, int total);
  ~csPVSNodeData ();
  bool PVSNamesContains (const char* name);
  void RemoveFromPVS(void *object);

  // Set of all objects potentially visible from this node
  void** pvs;
  // Set of all names of objects we expect to be in the PVS.
  csString* pvsnames;
  
  int numRegistered;  // Number of registered objects from pvsnames
  int numTotal;  // Total number of objects in pvsnames
};

class csPVSID
{
public:
  csString name;
  csArray<csPVSNodeData*> nodes;

  csPVSID(const csString& name);
  csPVSID(int numnodes, const csString& name);
  /// Iterates through every node and adds a new visibility object.
  void Register(void* data);
  /// Iterates through every node and removes the visibility object.
  void Unregister(void* data);
};

void csSavePVSDataFile(iObjectRegistry* registry, const char* filename,
    csStaticKDTree* tree);
void csSavePVSDataFile(iObjectRegistry* registry, const char* filename,
    const csStaticKDTree* tree, csArray<csPVSID>& idlist);
csStaticKDTree* csLoadPVSDataFile(iObjectRegistry* registry, 
    const char* filename, csArray<csPVSID>& idlist);

#endif
