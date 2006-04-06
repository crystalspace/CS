/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_TERRAFORMER_COLLIDER_H__
#define __CS_TERRAFORMER_COLLIDER_H__

#include "csutil/scf.h"
#include "ivaria/collider.h"
#include "ivaria/terraform.h"
#include "csutil/scf_implementation.h"
#include "csgeom/tri.h"
#include "csutil/dirtyaccessarray.h"
#include "Opcode.h"

struct csTriangle;

class csTerraFormerCollider : public scfImplementation1<csTerraFormerCollider, iCollider>
{
  csRef<iTerraFormer> former;
  csStringID stringHeights;
  csStringID stringVertices;
  iObjectRegistry *object_reg;

  unsigned int resolution;

  CS::Plugins::Opcode::Opcode::MeshInterface opcMeshInt;

  CS::Plugins::Opcode::Opcode::OPCODECREATE OPCC;

  void InitOPCODEModel ();

public:

  void UpdateOPCODEModel (const csVector3 &other_pos, float resolution);
  unsigned int* indexholder;
  csDirtyAccessArray<CS::Plugins::Opcode::Point> vertices;

  CS::Plugins::Opcode::IceMaths::Matrix4x4 transform;
  CS::Plugins::Opcode::Opcode::Model* opcode_model;

  csTerraFormerCollider (iTerraFormer* terraformer, iObjectRegistry* object_reg);
  float SampleFloat (float x, float z);
  csColliderType GetColliderType () {return CS_TERRAFORMER_COLLIDER;}
  virtual ~csTerraFormerCollider ();

  static void MeshCallback (CS::Plugins::Opcode::udword triangle_index, 
    CS::Plugins::Opcode::Opcode::VertexPointers& triangle, void* user_data);

  CS::Plugins::Opcode::Opcode::Model* GetOPCODEModel ();

};

#endif // __CS_TERRAFORMER_COLLIDER_H__

