/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include "sysdef.h"
#include "csgeom/quadtree.h"
#include "csgeom/frustrum.h"

csQuadtreeNode::csQuadtreeNode ()
{
  children[0] = NULL;
  children[1] = NULL;
  children[2] = NULL;
  children[3] = NULL;
  state = CS_QUAD_EMPTY;
}

csQuadtreeNode::~csQuadtreeNode ()
{
  CHK (delete children[0]);
  CHK (delete children[1]);
  CHK (delete children[2]);
  CHK (delete children[3]);
}

void csQuadtreeNode::SetBox (const csVector3& corner00, const csVector3& corner01,
  	const csVector3& corner10, const csVector3& corner11)
{
  corners[0] = corner00;
  corners[1] = corner01;
  corners[2] = corner10;
  corners[3] = corner11;
  center = (corner00 + corner11) / 2;
}

//--------------------------------------------------------------------------

void csQuadtree::Build (csQuadtreeNode* node, const csVector3& corner00,
  	const csVector3& corner01, const csVector3& corner10,
	const csVector3& corner11, int depth)
{
  node->SetBox (corner00, corner01, corner10, corner11);
  if (depth <= 0) return;
  const csVector3& center = node->GetCenter ();

  CHK (node->children[0] = new csQuadtreeNode ());
  Build (node->children[0],
  	corner00, (corner00+corner01)/2,
  	(corner00+corner10)/2, center,
	depth-1);

  CHK (node->children[1] = new csQuadtreeNode ());
  Build (node->children[1],
  	(corner00+corner01)/2, corner01,
  	center, (corner01+corner11)/2,
	depth-1);

  CHK (node->children[2] = new csQuadtreeNode ());
  Build (node->children[2],
  	(corner00+corner10)/2, center,
  	corner10, (corner10+corner11)/2,
	depth-1);

  CHK (node->children[3] = new csQuadtreeNode ());
  Build (node->children[3],
  	center, (corner01+corner11)/2,
  	(corner10+corner11)/2, corner11,
	depth-1);
}

csQuadtree::csQuadtree (csVector3* corners, int depth)
{
  CHK (root = new csQuadtreeNode ());
  Build (root, corners[0], corners[1], corners[2], corners[3], depth-1);
}

csQuadtree::~csQuadtree ()
{
  CHK (delete root);
}

bool csQuadtree::InsertPolygon (csQuadtreeNode* node,
	csVector3* verts, int num_verts,
	bool i00, bool i01, bool i10, bool i11)
{
  bool was_full = (node->GetState () == CS_QUAD_FULL);

  if (i00 && i01 && i10 && i11)
  {
    // This node is completely contained in the polygon.
    node->SetState (CS_QUAD_FULL);
    return !was_full;
  }

  if (!i00 && !i01 && !i10 && !i11)
  {
    // This node is completely outside the polygon.
    return false;
  }

  // The node is only partially in the polygon.
  if (!node->children[0])
  {
    // This node has no children.
    if (node->GetState () == CS_QUAD_FULL) return false;
    else if (node->GetState () == CS_QUAD_EMPTY)
    {
      node->SetState (CS_QUAD_PARTIAL);
      return true;
    }
    else // CS_QUAD_PARTIAL
    {
      return true;
    }
  }
  else
  {
    // This node has children.
  }
  return false;
}

bool csQuadtree::InsertPolygon (csVector3* verts, int num_verts)
{
  bool i00 = csFrustrum::Contains (verts, num_verts, root->GetCorner (0));
  bool i01 = csFrustrum::Contains (verts, num_verts, root->GetCorner (1));
  bool i10 = csFrustrum::Contains (verts, num_verts, root->GetCorner (2));
  bool i11 = csFrustrum::Contains (verts, num_verts, root->GetCorner (3));
  return InsertPolygon (root, verts, num_verts, i00, i01, i10, i11);
}

