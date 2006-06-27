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

#include "iterrain/terraincell.h"

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
csArray<csVector3>& points)
{
  const csVector2& pos = cell->GetPosition ();
  const csVector3& size = cell->GetSize ();
  
  if (fabsf (start.x - end.x) < EPSILON && fabsf (start.z - end.z) < EPSILON
    && start.x >= pos.x && start.x <= pos.x + size.x && start.z >= pos.y &&
    start.z <= pos.y + size.y)
  {
    float height = cell->GetHeight(csVector2(start.x, start.z) - cell->GetPosition());
    
    float min_height, max_height;
    
    if (start.y < end.y)
    {
      min_height = start.y;
      max_height = end.y;
    }
    else
    {
      max_height = start.y;
      min_height = end.y;
    }
    
    if (height >= min_height && height <= max_height)
    {
      points.Push (csVector3 (start.x, height, start.z));
      return true;
    }
  }
  
  return false;
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
