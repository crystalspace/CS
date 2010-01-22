/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_DEBUGDYN_H__
#define __CS_DEBUGDYN_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "csutil/leakguard.h"
#include "csutil/weakref.h"
#include "csutil/refarr.h"
#include "ivaria/dynamicsdebug.h"

struct iDynamicSystem;
struct iSector;
struct iMeshWrapper;
struct iMaterialWrapper;
struct csOrthoTransform;
struct csBox3;
struct csSphere;
class DynamicsDebugger;

CS_PLUGIN_NAMESPACE_BEGIN(DebugDynamics)
{

  class DebuggerManager : public scfImplementation2<DebuggerManager,
    iDynamicsDebuggerManager, iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(DebuggerManager);

    DebuggerManager (iBase* parent);

    //-- iDynamicsDebuggerManager
    virtual iDynamicSystemDebugger* CreateDebugger ();

    //-- iComponent
    virtual bool Initialize (iObjectRegistry*);

    //-- Error reporting
    void Report (int severity, const char* msg, ...);

  private:
    iObjectRegistry* object_reg;
    csRefArray<iDynamicSystemDebugger> debuggers;

    friend class DynamicsDebugger;
  };

  class DynamicsDebugger : public scfImplementation1<DynamicsDebugger,
    iDynamicSystemDebugger>
  {
  public:
    CS_LEAKGUARD_DECLARE(DynamicsDebugger);

    DynamicsDebugger (DebuggerManager* manager);

    //-- iDynamicSystemDebugger
    virtual void SetDynamicSystem (iDynamicSystem* system);
    virtual void SetDebugSector (iSector* sector);
    virtual void SetDebugDisplayMode (bool debugMode);

  private:
    csRef<iMeshWrapper> CreateBoxMesh (csBox3 box,
				       iMaterialWrapper* material,
				       csOrthoTransform transform,
				       iSector* sector);

    csRef<iMeshWrapper> CreateSphereMesh (csSphere sphere,
					  iMaterialWrapper* material,
					  iSector* sector);

    csRef<iMeshWrapper> CreateCylinderMesh (float length, float radius,
					    iMaterialWrapper* material,
					    csOrthoTransform transform,
					    iSector* sector);

    csRef<iMeshWrapper> CreateCapsuleMesh (float length, float radius,
					   iMaterialWrapper* material,
					   csOrthoTransform transform,
					   iSector* sector);

  private:

    struct MeshData
    {
      csRef<iMeshWrapper> mesh;
      //csOrthoTransform transform;
    };

    DebuggerManager* manager;
    csRef<iDynamicSystem> system;
    csRef<iSector> sector;
    csRef<iMaterialWrapper> material;
    bool debugMode;
    csHash<MeshData, csPtrKey<iRigidBody> > storedMeshes;
  };

}
CS_PLUGIN_NAMESPACE_END(DebugDynamics)

#endif // __CS_DEBUGDYN_H__
