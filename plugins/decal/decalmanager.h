/*
    Copyright (C) 2006 by Andrew Robberts

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

#ifndef __CS_DECAL_MANAGER_H__
#define __CS_DECAL_MANAGER_H__

#include "iutil/comp.h"
#include "ivaria/decal.h"
#include "csutil/scf_implementation.h"
#include "csgeom/vector3.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "csgeom/poly3d.h"
#include "csgeom/tri.h"
#include "ivaria/collider.h"
#include "csgfx/renderbuffer.h"
#include "ivideo/rendermesh.h"
#include "iutil/array.h"
#include "iutil/eventh.h"
#include "csutil/eventnames.h"
#include "iutil/virtclk.h"
#include "csutil/blockallocator.h"

struct iObjectRegistry;
class csDecal;

class csDecalManager : public scfImplementation3<csDecalManager,
  iDecalManager, iComponent, iEventHandler>
{
  friend class csDecal;

private:
  iObjectRegistry* objectReg;
  csRef<iEngine> engine;
  csArray<csDecal*> decals;
  csRef<iEventHandler> weakEventHandler;
  csRef<iVirtualClock> vc;
  csBlockAllocator<csRenderMesh> renderMeshAllocator;

public:
  csDecalManager (iBase* parent);
  virtual ~csDecalManager ();

  virtual bool Initialize (iObjectRegistry* objectReg);

  virtual iDecal* CreateDecal (iDecalTemplate* decalTemplate, 
    iSector* sector, const csVector3& pos, const csVector3& up, 
    const csVector3& normal, float width, float height, iDecal* oldDecal);
  virtual iDecal* CreateDecal (iDecalTemplate* decalTemplate, 
    iMeshWrapper* mesh, const csVector3& pos, const csVector3& up, 
    const csVector3& normal, float width = 1.0f, float height = 1.0f,
    iDecal* oldDecal = 0);

  virtual csRef<iDecalTemplate> CreateDecalTemplate (
    iMaterialWrapper* material);

  virtual void DeleteDecal (const iDecal* decal);
  virtual size_t GetDecalCount () const;
  virtual iDecal* GetDecal (size_t idx) const;

  // event handler stuff
  virtual bool HandleEvent (iEvent& ev);

  CS_EVENTHANDLER_PHASE_LOGIC ("crystalspace.decals")
  CS_DECLARE_EVENT_SHORTCUTS;
  
private:
  bool EnsureEngineReference ();

  void RemoveDecalFromList (csDecal* decal);
};  

#endif // __CS_DECAL_MANAGER_H__
