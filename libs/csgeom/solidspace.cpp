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
#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csgeom/solidspace.h"
#include "csgeom/pmtools.h"
#include "csgeom/plane3.h"
#include "igeom/polymesh.h"

//---------------------------------------------------------------------------

/**
 * This structure keeps track of solid space. Basically it represents
 * a 3 dimensional sparse bit array which is 1 where space is solid
 * and 0 where not.
 */
class csSolidSpaceNode
{
public:
  // Coordinate space for bits and children is 4x2x4. This makes
  // the assumption that most levels are more horizontally oriented.
#define CS_SOLIDSPACE_COORD(x,y,z) (((x)<<3)+((y)<<2)+(z))

  // Status of the 32 children. If 'children' is 0 then the meaning
  // of the bits is as follows:
  //   0: empty
  //   1: solid
  // If 'children' is not 0 then the meaning is as follows:
  //   0: unknown state, check child
  //   1: solid
  uint32 bits;

  // Pointer to an array of 32 children or null if this is a leaf.
  csSolidSpaceNode* children;

public:
  /// Create a new empty node.
  csSolidSpaceNode () : bits (0), children (0) { }
  /// Destroy the BSP-tree.
  ~csSolidSpaceNode () { delete[] children; }
};

//---------------------------------------------------------------------------

csSolidSpace::csSolidSpace (const csBox3& bbox, const csVector3& minsize)
{
  root = new csSolidSpaceNode ();
  root_bbox = bbox;
  csSolidSpace::minsize = minsize;
}

csSolidSpace::~csSolidSpace ()
{
  delete root;
}

bool csSolidSpace::CheckBox (const csBox3& bbox, csSolidSpaceNode* node,
	csVector3* vertices,
	csTriangleMinMax* tris, int tri_count, 
	csPlane3* planes)
{
  // If node is already completely solid we don't have to proceed.
  if (node->bits == (uint32)~0) return true;

  bool box_intersects = !csPolygonMeshTools::BoxInClosedMesh (
  	bbox, vertices, tris, tri_count, planes);

  // If the box intersects with the mesh we don't have sufficient
  // information and we have to traverse to the children.
  if (box_intersects)
  {
    if (!node)
    {
      // We are not calculating for a node. Just return false here.
      return false;
    }

    csVector3 dbox (
    	(bbox.MaxX ()-bbox.MinX ()) / 4.0f,
    	(bbox.MaxY ()-bbox.MinY ()) / 2.0f,
    	(bbox.MaxZ ()-bbox.MinZ ()) / 4.0f);
    if (dbox.x < minsize.x && dbox.y < minsize.y && dbox.z < minsize.z)
    {
      // Our boxes are below the minimum size so we stop dividing further.
      // In this case we still mark solid and non solid nodes but we don't
      // create children.
    }
    else
    {
      // Check if there are already children.
      if (!node->children)
        node->children = new csSolidSpaceNode[32];
    }

    int x, y, z;
    csBox3 b;
    for (z = 0 ; z < 4 ; z++)
    {
      b.SetMin (CS_AXIS_Z, bbox.MinZ ()+float (z)*dbox.z);
      b.SetMax (CS_AXIS_Z, bbox.MinZ ()+float (z+1)*dbox.z);
      for (y = 0 ; y < 2 ; y++)
      {
        b.SetMin (CS_AXIS_Y, bbox.MinY ()+float (y)*dbox.y);
        b.SetMax (CS_AXIS_Y, bbox.MinY ()+float (y+1)*dbox.y);
        for (x = 0 ; x < 4 ; x++)
	{
          b.SetMin (CS_AXIS_X, bbox.MinX ()+float (x)*dbox.x);
          b.SetMax (CS_AXIS_X, bbox.MinX ()+float (x+1)*dbox.x);
	  int c = CS_SOLIDSPACE_COORD (x, y, z);
	  bool solid = CheckBox (bbox,
	  	node->children ? &node->children[c] : 0,
		vertices, tris, tri_count, planes);
	  if (solid)
	    node->bits |= 1 << c;
	}
      }
    }
    if (node->bits == (uint32)~0)
    {
      // Node is fully solid. We just delete the children again then.
      delete[] node->children;
      node->children = 0;
      return true;
    }

    return false;
  }

  // The box does not intersect with the mesh. In that case we
  // test if a single point of the box is inside the mesh. If so then
  // we know the entire box is in the mesh and the box can be marked
  // as solid.
  bool point_closed = csPolygonMeshTools::PointInClosedMesh (
  	bbox.Min (), vertices, tris, tri_count, planes);
  if (point_closed)
  {
    // This node can be marked solid.
    if (node)
    {
      delete[] node->children;
      node->children = 0;
      node->bits = (uint32)~0;
    }
    return true;
  }
  return false;
}

void csSolidSpace::AddClosedObject (iPolygonMesh* object)
{
  csTriangleMinMax* tris;
  int tri_count;
  csPlane3* planes;
  csPolygonMeshTools::SortTrianglesX (object, tris, tri_count, planes);

  CheckBox (root_bbox, root, object->GetVertices (),
  	tris, tri_count, planes);

  delete[] tris;
  delete[] planes;
}

//---------------------------------------------------------------------------

