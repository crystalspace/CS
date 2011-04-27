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

#include "imesh/terrain2.h"

#include "iutil/comp.h"

#include "Opcode.h"

CS_PLUGIN_NAMESPACE_BEGIN(csOpcode)
{

class csTerrainCellCollisionProperties :
  public scfImplementation1<csTerrainCellCollisionProperties,
                            iTerrainCellCollisionProperties>
{
private:
  bool collidable;

public:
  csTerrainCellCollisionProperties ();
  csTerrainCellCollisionProperties (csTerrainCellCollisionProperties& other);

  virtual ~csTerrainCellCollisionProperties ();

  virtual bool GetCollidable () const;
  virtual void SetCollidable (bool value);
  
  virtual void SetParameter (const char* name, const char* value);
  virtual size_t GetParameterCount();
  virtual const char* GetParameterName (size_t index);
  virtual const char* GetParameterValue (size_t index);
  virtual const char* GetParameterValue (const char* name);

  virtual csPtr<iTerrainCellCollisionProperties> Clone ();
};

class csTerrainCollider :
  public scfImplementation2<csTerrainCollider,
                            iTerrainCollider,
                            iComponent>
{
  iObjectRegistry* object_reg;
  
  Opcode::AABBTreeCollider TreeCollider;
  Opcode::BVTCache ColCache;

public:
  csTerrainCollider (iBase* parent);

  virtual ~csTerrainCollider ();

  // ------------ iTerrainCollider implementation ------------

  virtual csPtr<iTerrainCellCollisionProperties> CreateProperties ();

  virtual bool CollideSegment (iTerrainCell* cell, const csVector3& start,
            const csVector3& end, bool oneHit, iTerrainVector3Array* points);
  virtual csTerrainColliderCollideSegmentResult CollideSegment (
      iTerrainCell* cell, const csVector3& start, const csVector3& end);
  virtual bool CollideSegment (iTerrainCell* cell, const csVector3& start,
                               const csVector3& end,
			       csVector3& hitPoint);

  virtual bool CollideTriangles (iTerrainCell* cell, const csVector3* vertices,
                       size_t tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform& trans,
                       bool oneHit, iTerrainCollisionPairArray* pairs);

  virtual bool Collide (iTerrainCell* cell, iCollider* collider,
                       float radius, const csReversibleTransform& trans,
                       bool oneHit, iTerrainCollisionPairArray* pairs);


  // ------------ iComponent implementation ------------
  virtual bool Initialize (iObjectRegistry* object_reg);
};

}
CS_PLUGIN_NAMESPACE_END(csOpcode)

#endif // __CS_TERRAIN_SIMPLECOLLIDER_H__
