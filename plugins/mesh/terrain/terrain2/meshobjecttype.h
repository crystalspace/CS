/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#ifndef __CS_TERRAIN_MESHOBJECTTYPE_H__
#define __CS_TERRAIN_MESHOBJECTTYPE_H__

#include "csutil/scf_implementation.h"

#include "imesh/object.h"
#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

class csTerrainMeshObjectType :
  public scfImplementation2<csTerrainMeshObjectType,
                            iMeshObjectType,
                            iComponent>
{
  iObjectRegistry* object_reg;
public:
  csTerrainMeshObjectType (iBase* parent);

  virtual ~csTerrainMeshObjectType ();

  // ------------ iMeshObjectType implementation ------------

  virtual csPtr<iMeshObjectFactory> NewFactory ();
  
  // ------------ iComponent implementation ------------
  virtual bool Initialize (iObjectRegistry* object_reg);
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CS_TERRAIN_MESHOBJECTTYPE_H__
