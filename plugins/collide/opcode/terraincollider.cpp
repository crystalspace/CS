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

#include "cssysdef.h"

#include "csgeom/transfrm.h"
#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refcount.h"
#include "imesh/terrain2.h"
#include "ivaria/collider.h"

#include "segmentcell.h"
#include "terraincollider.h"
#include "CSopcodecollider.h"

CS_PLUGIN_NAMESPACE_BEGIN(csOpcode)
{

SCF_IMPLEMENT_FACTORY (csTerrainCollider)

csTerrainCellCollisionProperties::csTerrainCellCollisionProperties ()
  : scfImplementationType (this), collidable (true)
{
}

csTerrainCellCollisionProperties::csTerrainCellCollisionProperties (
  csTerrainCellCollisionProperties& other)
  : scfImplementationType (this), collidable (other.collidable)
{
}

csTerrainCellCollisionProperties::~csTerrainCellCollisionProperties ()
{
}

bool csTerrainCellCollisionProperties::GetCollidable () const
{
  return collidable;
}

void csTerrainCellCollisionProperties::SetCollidable (bool value)
{
  collidable = value;
}

void csTerrainCellCollisionProperties::SetParameter (const char* name,
  const char* value)
{
  if (!strcmp (name, "collideable"))
    collidable = !strcmp (value, "true");
}

size_t csTerrainCellCollisionProperties::GetParameterCount()
{
  return 1;
}

const char* csTerrainCellCollisionProperties::GetParameterName (size_t index)
{
  switch (index)
  {
    case 0: return "collideable";
    default: return 0;
  }
}

const char* csTerrainCellCollisionProperties::GetParameterValue (size_t index)
{
  return GetParameterValue (GetParameterName (index));
}

const char* csTerrainCellCollisionProperties::GetParameterValue (const char* name)
{
  if (strcmp (name, "collideable") == 0)
    return collidable ? "true" : "false";
  else
    return 0;
}

csPtr<iTerrainCellCollisionProperties> csTerrainCellCollisionProperties::Clone ()
{
  return csPtr<iTerrainCellCollisionProperties> (
    new csTerrainCellCollisionProperties (*this)); 
}


csTerrainCollider::csTerrainCollider (iBase* parent)
  : scfImplementationType (this, parent)
{
  TreeCollider.SetFirstContact (false);
  TreeCollider.SetFullBoxBoxTest (false);
  TreeCollider.SetFullPrimBoxTest (false);
  // TreeCollider.SetFullPrimPrimTest (true);
  TreeCollider.SetTemporalCoherence (true);
}


csTerrainCollider::~csTerrainCollider ()
{
}

csPtr<iTerrainCellCollisionProperties> csTerrainCollider::CreateProperties ()
{
  return csPtr<iTerrainCellCollisionProperties> (
      new csTerrainCellCollisionProperties);
}

csTerrainColliderCollideSegmentResult csTerrainCollider::CollideSegment (
      iTerrainCell* cell, const csVector3& start, const csVector3& end)
{
  csTerrainColliderCollideSegmentResult rc;

  csTerrainSegmentCellCollider collider (cell, start, end);

  csVector2 cell_result (0, 0);
  int rv;
  
  while ((rv = collider.GetIntersection (rc.isect, cell_result)) >= 0)
  {
    if (rv == 1)
    {
      unsigned int width = cell->GetGridWidth ();
      unsigned int height = cell->GetGridHeight ();
  
      const csVector2& pos = cell->GetPosition ();
      const csVector3& size = cell->GetSize ();

      float scale_u = size.x / (width - 1);
      float scale_v = size.z / (height - 1);

      rc.hit = true;
      if (cell_result.x >= width - 1 - EPSILON) 
        cell_result.x = width - 1 - EPSILON;
      if (cell_result.x < 0)
        cell_result.x = 0;
            
      if (cell_result.y >= height - 1 - EPSILON) 
        cell_result.y = height - 1 - EPSILON;
      if (cell_result.y < 0)
        cell_result.y = 0;

      int x = (int)floorf(cell_result.x);
      int y = (int)floorf(cell_result.y);

      float frac = (cell_result.x - floorf(cell_result.x)) +
		   (cell_result.y - floorf(cell_result.y));
      bool half = (frac >= 1);
      if (!half)
      {
        rc.a = csVector3(x, cell->GetHeight (x, y), height-y-1);
        rc.b = csVector3(x+1, cell->GetHeight (x+1, y), height-y-1);
        rc.c = csVector3(x, cell->GetHeight (x, y+1), height-y-2);
      }
      else
      {
        rc.a = csVector3(x+1, cell->GetHeight (x+1, y+1), height-y-2);
        rc.b = csVector3(x, cell->GetHeight (x, y+1), height-y-2);
        rc.c = csVector3(x+1, cell->GetHeight (x+1, y), height-y-1);
      }
        
      rc.a.x *= scale_u; rc.a.x += pos.x;
      rc.b.x *= scale_u; rc.b.x += pos.x;
      rc.c.x *= scale_u; rc.c.x += pos.x;
        
      rc.a.z *= scale_v; rc.a.z += pos.y;
      rc.b.z *= scale_v; rc.b.z += pos.y;
      rc.c.z *= scale_v; rc.c.z += pos.y;

      return rc;
    }
  }

  rc.hit = false;
  return rc;
}

bool csTerrainCollider::CollideSegment (iTerrainCell* cell,
  const csVector3& start, const csVector3& end, bool oneHit,
  iTerrainVector3Array* points)
{
  size_t points_size = points->GetSize ();
  
  csTerrainSegmentCellCollider collider (cell, start, end);
  
  csVector3 result;
  csVector2 cell_result;
  int rv;
  
  while ((rv = collider.GetIntersection (result, cell_result)) >= 0)
  {
    if (rv == 1) points->Push (result);
  }
  
  return points_size != points->GetSize ();
}

bool csTerrainCollider::CollideSegment (iTerrainCell* cell, const csVector3& start,
					const csVector3& end,
					csVector3& hitPoint)
{
  csTerrainSegmentCellCollider collider (cell, start, end);
  
  csVector3 result;
  csVector2 cell_result;
  int rv;
  
  while ((rv = collider.GetIntersection (result, cell_result)) >= 0)
  {
    if (rv == 1)
    {
      hitPoint = result;
      return true;
    }
  }
  
  return false;
}

struct csTerrainTriangle
{
  unsigned int x, y; // quad coords, 0..width/height-2
  bool half; // triangle, representing half of the quad
  
  bool operator==(const csTerrainTriangle& tri) const
  {
    return (x == tri.x && y == tri.y && half == tri.half);
  }
};

bool csTerrainCollider::CollideTriangles (iTerrainCell* cell,
                       const csVector3* vertices, size_t tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform& trans,
                       bool oneHit, iTerrainCollisionPairArray* pairs)
{
  bool result = false;
  unsigned int width = cell->GetGridWidth ();
  unsigned int height = cell->GetGridHeight ();
  
  const csVector2& pos = cell->GetPosition ();
  const csVector3& size = cell->GetSize ();

  float scale_u = size.x / (width - 1);
  float scale_v = size.z / (height - 1);

  for (unsigned int i = 0; i < tri_count; ++i)
  {
    csArray<csTerrainTriangle> tris;
    
    csVector3 triv[3];
    
    for (unsigned int j = 0; j < 3; ++j)
    {
      triv[j] = vertices[indices[i*3 + j]];
      triv[j] = trans.This2Other(triv[j]);
    }
    
    for (unsigned int edge = 0; edge < 3; ++edge)
    {
      const csVector3& start = triv[edge];
      const csVector3& end = triv[CS::Math::NextModulo3(edge)];
      
      csTerrainSegmentCellCollider collider (cell, start, end);
  
      csVector3 vresult;
      csVector2 cell_result (0, 0);
      int rv;
  
      while ((rv = collider.GetIntersection (vresult, cell_result)) >= 0)
      {
        if (rv == 1)
        {
          csTerrainTriangle tri;
          
          if (cell_result.x >= width - 1 - EPSILON) 
            cell_result.x = width - 1 - EPSILON;
          if (cell_result.x <= 0)
            cell_result.x = 0;
            
          if (cell_result.y >= height - 1 - EPSILON) 
            cell_result.y = height - 1 - EPSILON;
          if (cell_result.y <= 0)
            cell_result.y = 0;
          
          tri.x = (int)floorf(cell_result.x);
          tri.y = (int)floorf(cell_result.y);
          
          float frac = (cell_result.x - floorf(cell_result.x)) +
                       (cell_result.y - floorf(cell_result.y));
          
          tri.half = (frac >= 1);
          
          tris.Push (tri);
        }
      }
    }
 
    if (!tris.IsEmpty ())
    {
      result = true;
      
      csCollisionPair p;
      
      p.a1 = vertices[indices[i*3 + 0]];
      p.b1 = vertices[indices[i*3 + 1]];
      p.c1 = vertices[indices[i*3 + 2]];
    
      for (size_t j = 0; j < tris.GetSize (); ++j)
      {
        const csTerrainTriangle& tri = tris[j];
        
        if (!tri.half)
        {
          p.a2 = csVector3(tri.x, cell->GetHeight (tri.x, tri.y), height-tri.y-1);
          p.b2 = csVector3(tri.x+1, cell->GetHeight (tri.x+1, tri.y), height-tri.y-1);
          p.c2 = csVector3(tri.x, cell->GetHeight (tri.x, tri.y+1), height-tri.y-2);
        }
        else
        {
          p.a2 = csVector3(tri.x+1, cell->GetHeight (tri.x+1, tri.y+1), height-tri.y-2);
          p.b2 = csVector3(tri.x, cell->GetHeight (tri.x, tri.y+1), height-tri.y-2);
          p.c2 = csVector3(tri.x+1, cell->GetHeight (tri.x+1, tri.y), height-tri.y-1);
        }
        
        p.a2.x *= scale_u; p.a2.x += pos.x;
        p.b2.x *= scale_u; p.b2.x += pos.x;
        p.c2.x *= scale_u; p.c2.x += pos.x;
        
        p.a2.z *= scale_v; p.a2.z += pos.y;
        p.b2.z *= scale_v; p.b2.z += pos.y;
        p.c2.z *= scale_v; p.c2.z += pos.y;

        pairs->Push (p);
      }
    }
  }
  
  return result;
}

class csOPCODETerrainCell: public csRefCount
{
  iTerrainCell* cell;

  Opcode::MeshInterface opcMeshInt;

  Opcode::OPCODECREATE OPCC;

public:
  csDirtyAccessArray<unsigned int> indices;
  csDirtyAccessArray<Point> vertices;

  IceMaths::Matrix4x4 transform;
  Opcode::Model* opcode_model;

  csOPCODETerrainCell (iTerrainCell* cell)
  {
    this->cell = cell;
  
    opcMeshInt.SetCallback (&MeshCallback, this); 

    transform.m[0][0] = 1;
    transform.m[1][0] = 0;
    transform.m[2][0] = 0;
    transform.m[3][0] = 0;  

    transform.m[0][1] = 0;
    transform.m[1][1] = 1;
    transform.m[2][1] = 0;
    transform.m[3][1] = 0;  

    transform.m[0][2] = 0;
    transform.m[1][2] = 0;
    transform.m[2][2] = 1;
    transform.m[3][2] = 0;  

    transform.m[0][3] = 0;
    transform.m[1][3] = 0;
    transform.m[2][3] = 0;
    transform.m[3][3] = 1;

    unsigned int width = cell->GetGridWidth ();
    unsigned int height = cell->GetGridHeight ();
  
    vertices.SetSize (width * height);
    indices.SetSize (3 * 2 * (width-1) * (height-1));

    opcode_model = new Opcode::Model;

    opcMeshInt.SetNbTriangles (2 * (width-1) * (height-1));
    opcMeshInt.SetNbVertices ((udword)vertices.GetSize());

    // Mesh data
    OPCC.mIMesh = &opcMeshInt;
    OPCC.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS |
       Opcode::SPLIT_GEOM_CENTER;
    OPCC.mNoLeaf = true;
    OPCC.mQuantized = true;
    OPCC.mKeepOriginal = false;
    OPCC.mCanRemap = true;

    float offset_x = cell->GetPosition ().x;
    float offset_y = cell->GetPosition ().y;

    float scale_x = cell->GetSize ().x / (width - 1);
    float scale_z = cell->GetSize ().z / (height - 1);

    for (unsigned int y = 0 ; y < height ; y++)
    {
      for (unsigned int x = 0 ; x < width ; x++)
      {
        int index = y*width + x;
        
        vertices[index].Set (x * scale_x + offset_x,
                             cell->GetHeight(x, y),
                             y * scale_z + offset_y);
      }
    }
  
    int i = 0;
    for (unsigned int y = 0 ; y < height-1 ; y++)
    {
      int yr = y * width;
      for (unsigned int x = 0 ; x < width-1 ; x++)
      {
        indices[i++] = yr + x;
        indices[i++] = yr+width + x;
        indices[i++] = yr + x+1;
        indices[i++] = yr + x+1;
        indices[i++] = yr+width + x;
        indices[i++] = yr+width + x+1;
      }
    }

    opcode_model->Build (OPCC);

  }

  ~csOPCODETerrainCell ()
  {
    if (opcode_model)
      delete opcode_model;
  }

  static void MeshCallback (udword triangle_index, 
    Opcode::VertexPointers& triangle, void* user_data)
  {
    csOPCODETerrainCell* cell = (csOPCODETerrainCell*)user_data;
    udword *tri_array = cell->indices.GetArray ();
    Point *vertholder = cell->vertices.GetArray ();
    int index = 3 * triangle_index;
    triangle.Vertex[0] = &vertholder [tri_array[index]] ;
    triangle.Vertex[1] = &vertholder [tri_array[index + 1]];
    triangle.Vertex[2] = &vertholder [tri_array[index + 2]];
  }
};

namespace
{
  void CopyCollisionPairs (Opcode::AABBTreeCollider& TreeCollider,
                           csOPCODECollider* col1,
                           csOPCODETerrainCell* cell,
                           iTerrainCollisionPairArray* pairs)
  {
    int size = (int) (udword(TreeCollider.GetNbPairs ()));
    if (size == 0) return;
    int N_pairs = size;
    const Pair* colPairs=TreeCollider.GetPairs ();
    Point* vertholder0 = col1->vertholder;
    if (!vertholder0) return;
    Point* vertholder1 = cell->vertices.GetArray ();
    if (!vertholder1) return;
    udword* indexholder0 = col1->indexholder;
    if (!indexholder0) return;
    udword* indexholder1 = cell->indices.GetArray ();
    if (!indexholder1) return;
    Point* current;
    int i, j;

    size_t oldlen = pairs->GetSize ();
    pairs->SetSize (oldlen + N_pairs);

    for (i = 0 ; i < N_pairs ; i++)
    {
      j = 3 * colPairs[i].id0;
      csCollisionPair& pair = pairs->Get (oldlen);

      current = &vertholder0[indexholder0[j]];		
      pair.a1 = csVector3 (current->x, current->y, current->z);
      current = &vertholder0[indexholder0[j + 1]];		
      pair.b1 = csVector3 (current->x, current->y, current->z);
      current = &vertholder0[indexholder0[j + 2]];		
      pair.c1 = csVector3 (current->x, current->y, current->z);

      j = 3 * colPairs[i].id1;
      current = &vertholder1[indexholder1[j]];		
      pair.a2 = csVector3 (current->x, current->y, current->z);
      current = &vertholder1[indexholder1[j + 1 ]];		
      pair.b2 = csVector3 (current->x, current->y, current->z);
      current = &vertholder1[indexholder1[j + 2 ]];		
      pair.c2 = csVector3 (current->x, current->y, current->z);

      oldlen++;
    }
  }
};

bool csTerrainCollider::Collide (iTerrainCell* cell, iCollider* collider,
                       float radius, const csReversibleTransform& trans,
                       bool oneHit, iTerrainCollisionPairArray* pairs)
{
  csOPCODECollider* col1 = (csOPCODECollider*)collider;

  csRef<csOPCODETerrainCell> cell_data = (csOPCODETerrainCell*)
    cell->GetCollisionData ();
  
  if (!cell_data)
  {
  	cell_data.AttachNew(new csOPCODETerrainCell (cell));
  	
  	cell->SetCollisionData (cell_data);
  }

  ColCache.Model0 = col1->m_pCollisionModel;
  ColCache.Model1 = cell_data->opcode_model;

  csMatrix3 m1 = trans.GetT2O ();
  csVector3 u = trans.GetO2TTranslation ();

  IceMaths::Matrix4x4 transform1;
  transform1.m[0][3] = 0;
  transform1.m[1][3] = 0;
  transform1.m[2][3] = 0;
  transform1.m[3][3] = 1;
  u = m1.Row1 ();
  transform1.m[0][0] = u.x;
  transform1.m[1][0] = u.y;
  transform1.m[2][0] = u.z;
  u = m1.Row2 ();
  transform1.m[0][1] = u.x;
  transform1.m[1][1] = u.y;
  transform1.m[2][1] = u.z;
  u = m1.Row3 ();
  transform1.m[0][2] = u.x;
  transform1.m[1][2] = u.y;
  transform1.m[2][2] = u.z;

  transform1.m[3][0] = u.x;
  transform1.m[3][1] = u.y;
  transform1.m[3][2] = u.z;

  TreeCollider.SetFirstContact(oneHit);

  bool isOk = TreeCollider.Collide (ColCache, &transform1,
  	&cell_data->transform);
  if (isOk)
  {
    bool status = (TreeCollider.GetContactStatus () != FALSE);
    if (status)
    {
      CopyCollisionPairs (TreeCollider, col1, cell_data, pairs);
    }
    return status;
  }
  else
  {
    return false;
  }
}

bool csTerrainCollider::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  return true;
}


}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
