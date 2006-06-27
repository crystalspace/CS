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

#ifndef __CS_ITERRAIN_TERRAINCOLLIDER_H__
#define __CS_ITERRAIN_TERRAINCOLLIDER_H__

#include "csutil/scf.h"
#include "csutil/array.h"

struct iTerrainCellCollisionProperties;
struct iTerrainCell;

class csVector3;
class csRect;

struct iTerrainCollider : public virtual iBase
{
  SCF_INTERFACE (iTerrainCollider, 1, 0, 0);

  virtual csPtr<iTerrainCellCollisionProperties> CreateProperties () = 0;
  
  virtual bool CollideSegment (iTerrainCell* cell, const csVector3& start,
           const csVector3& end, bool oneHit, csArray<csVector3>& points) = 0;

  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle,
                               const float* data, unsigned int pitch) = 0;
};

#endif // __CS_ITERRAIN_TERRAINCOLLIDER_H__
