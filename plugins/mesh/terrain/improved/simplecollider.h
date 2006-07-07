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

#ifndef __CS_TERRAIN_SIMPLECOLLIDER_H__
#define __CS_TERRAIN_SIMPLECOLLIDER_H__

#include "csutil/scf_implementation.h"

#include "iterrain/terraincollider.h"
#include "iterrain/terraincellcollisionproperties.h"

#include "iutil/comp.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

class csTerrainSimpleCellCollisionProperties :
  public scfImplementation1<csTerrainSimpleCellCollisionProperties,
                            iTerrainCellCollisionProperties>
{
private:
  bool collideable;

public:
  csTerrainSimpleCellCollisionProperties (iBase* parent);

  virtual ~csTerrainSimpleCellCollisionProperties ();

  virtual bool GetCollideable () const;
  virtual void SetCollideable (bool value);
};

class csTerrainSimpleCollider :
  public scfImplementation2<csTerrainSimpleCollider,
                            iTerrainCollider,
                            iComponent>
{
  iObjectRegistry* object_reg;
public:
  csTerrainSimpleCollider (iBase* parent);

  virtual ~csTerrainSimpleCollider ();

  // ------------ iTerrainCollider implementation ------------

  virtual csPtr<iTerrainCellCollisionProperties> CreateProperties ();

  virtual bool CollideSegment (iTerrainCell* cell, const csVector3& start,
            const csVector3& end, bool oneHit, iTerrainVector3Array& points);

  virtual bool CollideTriangles (iTerrainCell* cell, const csVector3* vertices,
                       unsigned int tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs);

  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle,
                               const float* data, unsigned int pitch);

  // ------------ iComponent implementation ------------
  virtual bool Initialize (iObjectRegistry* object_reg);
};

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)

#endif // __CS_TERRAIN_SIMPLECOLLIDER_H__
