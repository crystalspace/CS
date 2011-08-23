/*
  Copyright (C) 2011 by Liu Lu

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

#ifndef __CS_OPCODE_COLLIDER_H__
#define __CS_OPCODE_COLLIDER_H__

#include "imesh/terrain2.h"
#include "ivaria/collision2.h"
#include "Opcode.h"
#include "csOpcode2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Opcode2)
{
class csOpcodeCollider : public scfImplementation1<
  csOpcodeCollider, CS::Collision2::iColliderConcaveMesh>
{
  friend class csOpcodeCollisionObject;
  friend class csOpcodeCollisionSector;
private:
  float volume;
  Opcode::Model* model;
  iMeshWrapper* mesh;
  Opcode::MeshInterface opcMeshInt;
  unsigned int* indexholder;
  Point *vertholder;
  csVector3 scale;
  csBox3 aabbox;
  csOpcodeCollisionSystem* system;

  static void MeshCallback (udword triangle_index, 
    Opcode::VertexPointers& triangle, void* user_data);
public:
  csOpcodeCollider (iMeshWrapper* mesh, csOpcodeCollisionSystem* sys);
  virtual ~csOpcodeCollider();
  virtual CS::Collision2::ColliderType GetGeometryType () const {return CS::Collision2::COLLIDER_CONCAVE_MESH;}
  virtual iMeshWrapper* GetMesh () {return mesh;}
  virtual void SetLocalScale (const csVector3& scale) {}
  virtual const csVector3& GetLocalScale () const {return scale;}
  virtual void SetMargin (float margin) {}
  virtual float GetMargin () const {return 0;}
  virtual float GetVolume () const {return volume;}
};

class TerrainCellCollider
{
  friend class csOpcodeCollisionObject;

  static void MeshCallback (udword triangle_index, 
    Opcode::VertexPointers& triangle, void* user_data);

public:
  Opcode::MeshInterface opcMeshInt;
  Opcode::Model* model;
  iTerrainCell* cell;
  unsigned int* indexholder;
  Point *vertholder;
  csOrthoTransform cellTransform;
  TerrainCellCollider(iTerrainCell* cell, csOrthoTransform trans);
  virtual ~TerrainCellCollider();
  //Do not support update.
};

class csOpcodeColliderTerrain:
  public scfImplementation2<csOpcodeColliderTerrain,
  CS::Collision2::iColliderTerrain, iTerrainCellLoadCallback>
{
  friend class csOpcodeCollisionObject;
  friend class csOpcodeCollisionSector;

  csArray<TerrainCellCollider*> colliders;
  iTerrainSystem* terrainSystem;
  csOpcodeCollisionSystem* system;
  csOrthoTransform terrainTransform;
  float volume;
  bool unload;
  csVector3 scale;

public:
  csOpcodeColliderTerrain(iTerrainSystem* terrain, csOpcodeCollisionSystem* sys);
  virtual ~csOpcodeColliderTerrain();
  virtual CS::Collision2::ColliderType GetGeometryType() const {return CS::Collision2::COLLIDER_TERRAIN;}
  virtual iTerrainSystem* GetTerrain() const {return terrainSystem;}
  virtual void SetLocalScale (const csVector3& scale) {}
  virtual const csVector3& GetLocalScale () const {return scale;}
  virtual void SetMargin (float margin) {}
  virtual float GetMargin () const {return 0;}
  virtual float GetVolume () const {return volume;}

  virtual void OnCellLoad (iTerrainCell *cell);
  virtual void OnCellPreLoad (iTerrainCell *cell);
  virtual void OnCellUnload (iTerrainCell *cell);
  Opcode::Model* GetColliderModel (size_t index) {return colliders[index]->model;}
  csOrthoTransform GetColliderTransform (size_t index) {return colliders[index]->cellTransform;}
  Point* GetVertexHolder (size_t index) {return colliders[index]->vertholder;}
  udword* GetIndexHolder (size_t index) {return colliders[index]->indexholder;}
  void LoadCellToCollider (iTerrainCell* cell);
};
}
CS_PLUGIN_NAMESPACE_END (Opcode2)
#endif