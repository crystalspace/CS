/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#ifndef __CS_TERRAIN_FACTORY_H__
#define __CS_TERRAIN_FACTORY_H__

#include "csutil/scf_implementation.h"

#include "iterrain/terrainfactory.h"

#include "imesh/object.h"

#include "csutil/flags.h"

#include "terrainsystem.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

class csTerrainFactory :
  public scfImplementation2<csTerrainFactory,
                            iMeshObjectFactory,
                            iTerrainFactory>
{
  csRef<csTerrainSystem> terrain;

  iTerrainRenderer* renderer;
  iTerrainCollider* collider;
  
  iMeshFactoryWrapper* logparent;
  csFlags flags;

  csRef<iMeshObjectType> type;

public:
  csTerrainFactory (iMeshObjectType *pParent);

  virtual ~csTerrainFactory ();

  // ------------ iMeshObjectFactory implementation ------------

  virtual csFlags& GetFlags ();

  virtual csPtr<iMeshObject> NewInstance ();

  virtual csPtr<iMeshObjectFactory> Clone ();

  virtual void HardTransform (const csReversibleTransform& t);

  virtual bool SupportsHardTransform () const;

  virtual void SetMeshFactoryWrapper (iMeshFactoryWrapper* lp);

  virtual iMeshFactoryWrapper* GetMeshFactoryWrapper () const;

  virtual iMeshObjectType* GetMeshObjectType () const;

  virtual iObjectModel* GetObjectModel ();

  virtual bool SetMaterialWrapper (iMaterialWrapper* material);

  virtual iMaterialWrapper* GetMaterialWrapper () const;

  virtual void SetMixMode (uint mode);
  virtual uint GetMixMode () const;

  // ------------ iTerrainFactory implementation ------------

  virtual void SetRenderer (iTerrainRenderer* renderer);
  virtual void SetCollider (iTerrainCollider* collider);
  virtual iTerrainCell* AddCell (const char* name, int grid_width,
                        int grid_height, int material_width,
                        int material_height, bool material_persistent,
                        const csVector2& position,
                        const csVector3& size, iTerrainDataFeeder* feeder);
};

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)

#endif // __CS_TERRAIN_FACTORY_H__
