/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_SOLIDSPACE_H__
#define __CS_SOLIDSPACE_H__

#include "csextern.h"
#include "csgeom/box.h"
#include "csgeom/pmtools.h"

class csSolidSpaceNode;
struct iPolygonMesh;

/**
 * This structure keeps track of solid space. Basically it represents
 * a 3 dimensional sparse bit array which is 1 where space is solid
 * and 0 where not.
 */
class CS_CRYSTALSPACE_EXPORT csSolidSpace
{
private:
  csBox3 root_bbox;
  csSolidSpaceNode* root;
  csVector3 minsize;

  bool CheckBox (const csBox3& bbox, csSolidSpaceNode* node,
	csVector3* vertices,
	csTriangleMinMax* tris, int tri_count, 
	csPlane3* planes);

public:
  /**
   * Create a solid space for the given box and minimum node size.
   */
  csSolidSpace (const csBox3& bbox, const csVector3& minsize);

  ~csSolidSpace ();

  /**
   * Add a closed object as solid space. This polygon mesh
   * must be closed. This function will not check for that.
   */
  void AddClosedObject (iPolygonMesh* object);
};

#endif // __CS_SOLIDSPACE_H__

