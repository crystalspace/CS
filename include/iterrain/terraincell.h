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

#ifndef __CS_ITERRAIN_TERRAINCELL_H__
#define __CS_ITERRAIN_TERRAINCELL_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "csutil/refarr.h"

#include "terrainarray.h"

struct iRenderView;
struct iTerrainCellRenderProperties;
struct iTerrainCellCollisionProperties;
struct iImage;

struct iCollider;

class csVector2;
class csVector3;
class csRect;
class csRefCount;
class csReversibleTransform;

struct csLockedHeightData
{
  float* data;
  unsigned int pitch;
};

struct csLockedMaterialMap
{
  unsigned char* data;
  unsigned int pitch;
};


struct iTerrainCell : public virtual iBase
{
  SCF_INTERFACE (iTerrainCell, 1, 0, 0);

  virtual const char* GetName () const = 0;
  virtual iTerrainCellRenderProperties* GetRenderProperties () const = 0;
  virtual iTerrainCellCollisionProperties* GetCollisionProperties () const = 0;

  virtual csRefCount* GetRenderData () const = 0;
  virtual void SetRenderData (csRefCount* data) = 0;

  virtual csRefCount* GetCollisionData () const = 0;
  virtual void SetCollisionData (csRefCount* data) = 0;

  virtual int GetGridWidth () const = 0;
  virtual int GetGridHeight () const = 0;

  virtual csLockedHeightData GetHeightData () = 0;
  
  virtual csLockedHeightData LockHeightData (const csRect& rectangle) = 0;
  virtual void UnlockHeightData () = 0;

  virtual const csVector2& GetPosition () const = 0;
  virtual const csVector3& GetSize () const = 0;

  virtual int GetMaterialMapWidth () const = 0;
  virtual int GetMaterialMapHeight () const = 0;
  virtual csLockedMaterialMap LockMaterialMap (const csRect& rectangle) = 0;
  virtual void UnlockMaterialMap() = 0;

  virtual void SetMaterialMask (unsigned int material, iImage* image) = 0;
  virtual void SetMaterialMask (unsigned int material, const unsigned char*
                          data, unsigned int width, unsigned int height) = 0;

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

  virtual float GetHeight (int x, int y) const = 0;
  virtual float GetHeight (const csVector2& pos) const = 0;
  
  virtual csVector3 GetTangent (int x, int y) const = 0;
  virtual csVector3 GetTangent (const csVector2& pos) const = 0;

  virtual csVector3 GetBinormal (int x, int y) const = 0;
  virtual csVector3 GetBinormal (const csVector2& pos) const = 0;

  virtual csVector3 GetNormal (int x, int y) const = 0;
  virtual csVector3 GetNormal (const csVector2& pos) const = 0;
};

#endif // __CS_ITERRAIN_TERRAINCELL_H__
