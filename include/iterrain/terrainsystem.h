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

#ifndef __CS_ITERRAIN_TERRAINSYSTEM_H__
#define __CS_ITERRAIN_TERRAINSYSTEM_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "csutil/refarr.h"

#include "terrainarray.h"

struct iRenderView;
struct iTerrainCell;
struct iMaterialWrapper;
struct iMovable;
struct iCollider;

class csVector2;
class csVector3;

class csReversibleTransform;

struct iTerrainSystem : public virtual iBase
{
  SCF_INTERFACE (iTerrainSystem, 1, 0, 0);

  virtual iTerrainCell* GetCell (const char* name) = 0;
  virtual iTerrainCell* GetCell (const csVector2& pos) = 0;

  virtual const csRefArray<iMaterialWrapper>& GetMaterialPalette () const = 0;
  virtual void SetMaterialPalette (const csRefArray<iMaterialWrapper>& array)
                                                                          = 0;

  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
                           bool oneHit, iTerrainVector3Array& points) = 0;
  virtual bool CollideTriangles (const csVector3* vertices,
                       unsigned int tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs) = 0;
  virtual bool Collide (iCollider* collider, float radius,
                       const csReversibleTransform* trans, bool oneHit,
                       iTerrainCollisionPairArray& pairs) = 0;

  virtual float GetVirtualViewDistance () const = 0;
  virtual void SetVirtualViewDistance (float distance) = 0;

  virtual bool GetAutoPreLoad () const = 0;
  virtual void SetAutoPreLoad (bool mode) = 0;
  virtual void PreLoadCells (iRenderView* rview, iMovable* movable) = 0;
  
  virtual float GetHeight (const csVector2& pos) = 0;
  virtual csVector3 GetTangent (const csVector2& pos) = 0;
  virtual csVector3 GetBinormal (const csVector2& pos) = 0;
  virtual csVector3 GetNormal (const csVector2& pos) = 0;
};

#endif // __CS_ITERRAIN_TERRAINSYSTEM_H__
