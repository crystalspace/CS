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

#include "simplecollider.h"

#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csgeom/transfrm.h"

#include "ivaria/collider.h"

#include "iterrain/terraincell.h"

#include "csutil/set.h"

#include "segmentcell.h"

CS_PLUGIN_NAMESPACE_BEGIN(ImprovedTerrain)
{

SCF_IMPLEMENT_FACTORY (csTerrainSimpleCollider)

csTerrainSimpleCellCollisionProperties::csTerrainSimpleCellCollisionProperties
 (iBase* parent)
  : scfImplementationType (this, parent)
{
  collideable = true;
}

csTerrainSimpleCellCollisionProperties::~csTerrainSimpleCellCollisionProperties
()
{
}

bool csTerrainSimpleCellCollisionProperties::GetCollideable () const
{
  return collideable;
}

void csTerrainSimpleCellCollisionProperties::SetCollideable (bool value)
{
  collideable = value;
}

csTerrainSimpleCollider::csTerrainSimpleCollider (iBase* parent)
  : scfImplementationType (this, parent)
{
}


csTerrainSimpleCollider::~csTerrainSimpleCollider ()
{
}

csPtr<iTerrainCellCollisionProperties> csTerrainSimpleCollider::
CreateProperties ()
{
  return new csTerrainSimpleCellCollisionProperties(NULL);
}

bool csTerrainSimpleCollider::CollideSegment (iTerrainCell* cell,
const csVector3& start, const csVector3& end, bool oneHit,
iTerrainVector3Array& points)
{
  size_t points_size = points.GetSize ();
  
  csTerrainSegmentCellCollider collider (cell, start, end);
  
  csVector3 result;
  csVector2 cell_result;
  int rv;
  
  while ((rv = collider.GetIntersection (result, cell_result)) >= 0)
  {
    if (rv == 1) points.Push (result);
  }
  
  return points_size != points.GetSize ();
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

bool csTerrainSimpleCollider::CollideTriangles (iTerrainCell* cell,
                       const csVector3* vertices, unsigned int tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs)
{
  bool result = false;
  unsigned int width = cell->GetGridWidth ();
  unsigned int height = cell->GetGridHeight ();
  
  const csVector2& pos = cell->GetPosition ();
  const csVector3& size = cell->GetSize ();

  float scale_u = size.x / (width - 1);
  float scale_v = size.y / (height - 1);

  for (unsigned int i = 0; i < tri_count; ++i)
  {
    csArray<csTerrainTriangle> tris;
    
    csVector3 triv[3];
    
    for (unsigned int j = 0; j < 3; ++j)
    {
      triv[j] = vertices[indices[i*3 + j]];
      triv[j] = trans->This2Other(triv[j]);
    }
    
    for (unsigned int edge = 0; edge < 3; ++edge)
    {
      const csVector3& start = triv[edge];
      const csVector3& end = triv[(edge + 1)%3];
      
      csTerrainSegmentCellCollider collider (cell, start, end);
  
      csVector3 result;
      csVector2 cell_result;
      int rv;
  
      while ((rv = collider.GetIntersection (result, cell_result)) >= 0)
      {
        if (rv == 1)
        {
          csTerrainTriangle tri;
          
          if (cell_result.x >= width - 1 - EPSILON) 
            cell_result.x = width - 1 - EPSILON;
            
          if (cell_result.y >= height - 1 - EPSILON) 
            cell_result.y = height - 1 - EPSILON;
          
          tri.x = floor(cell_result.x);
          tri.y = floor(cell_result.y);
          
          float frac = (cell_result.x - floor(cell_result.x)) +
                       (cell_result.y - floor(cell_result.y));
          
          tri.half = (frac >= 1);
          
          tris.Push (tri);
        }
      }
    }
 
    if (!tris.IsEmpty ())
    {
      result = true;
      
      csCollisionPair p;
      
      p.a1 = triv[0];
      p.b1 = triv[1];
      p.c1 = triv[2];
    
      for (size_t i = 0; i < tris.GetSize (); ++i)
      {
        const csTerrainTriangle& tri = tris[i];
        
        if (!tri.half)
        {
          p.a2 = csVector3(tri.x, cell->GetHeight (tri.x, tri.y), tri.y);
          p.c2 = csVector3(tri.x+1, cell->GetHeight (tri.x+1, tri.y), tri.y);
          p.b2 = csVector3(tri.x, cell->GetHeight (tri.x, tri.y+1), tri.y+1);
        }
        else
        {
          p.a2 = csVector3(tri.x+1, cell->GetHeight (tri.x+1, tri.y+1), tri.y+1);
          p.c2 = csVector3(tri.x, cell->GetHeight (tri.x, tri.y+1), tri.y+1);
          p.b2 = csVector3(tri.x+1, cell->GetHeight (tri.x+1, tri.y), tri.y);
        }
        
        p.a2.x *= scale_u; p.a2.x += pos.x;
        p.b2.x *= scale_u; p.b2.x += pos.x;
        p.c2.x *= scale_u; p.c2.x += pos.x;
        
        p.a2.z *= scale_v; p.a2.z += pos.y;
        p.b2.z *= scale_v; p.b2.z += pos.y;
        p.c2.z *= scale_v; p.c2.z += pos.y;

        pairs.Push (p);
      }
    }
  }
  
  return result;
}

bool csTerrainSimpleCollider::Initialize (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
  return true;
}

void csTerrainSimpleCollider::OnHeightUpdate (iTerrainCell* cell, const csRect&
  rectangle, const float* data, unsigned int pitch)
{
}

}
CS_PLUGIN_NAMESPACE_END(ImprovedTerrain)
