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
  	const csVector3& corner11, const csVector3& corner10)
{
  corners[0] = corner00;
  corners[1] = corner01;
  corners[2] = corner11;
  corners[3] = corner10;
  center = (corner00 + corner11) / 2;
}

//--------------------------------------------------------------------------

void csQuadtree::Build (csQuadtreeNode* node, const csVector3& corner00,
  	const csVector3& corner01, const csVector3& corner11,
	const csVector3& corner10, int depth)
{
  node->SetBox (corner00, corner01, corner11, corner10);
  if (depth <= 0) return;
  const csVector3& center = node->GetCenter ();

  CHK (node->children[0] = new csQuadtreeNode ());
  Build (node->children[0],
  	corner00, (corner00+corner01)/2,
  	center, (corner00+corner10)/2,
	depth-1);

  CHK (node->children[1] = new csQuadtreeNode ());
  Build (node->children[1],
  	(corner00+corner01)/2, corner01,
  	(corner01+corner11)/2, center,
	depth-1);

  CHK (node->children[2] = new csQuadtreeNode ());
  Build (node->children[2],
  	center, (corner01+corner11)/2,
  	corner11, (corner10+corner11)/2,
	depth-1);

  CHK (node->children[3] = new csQuadtreeNode ());
  Build (node->children[3],
  	(corner00+corner10)/2, center,
  	(corner10+corner11)/2, corner10,
	depth-1);
}

csQuadtree::csQuadtree (csVector3* corners, int depth)
{
  CHK (root = new csQuadtreeNode ());
  Build (root, corners[0], corners[1], corners[2], corners[3], depth-1);
  csVector3 pn = (corners[0]+corners[1]+corners[2]+corners[3]).Unit ();
  plane_normal.Set (-pn.x, -pn.y, -pn.z, 0);
}

csQuadtree::~csQuadtree ()
{
  CHK (delete root);
}

bool csQuadtree::InsertPolygon (csQuadtreeNode* node,
	csVector3* verts, int num_verts,
	bool i00, bool i01, bool i11, bool i10)
{
  // If node is completely full already then nothing can happen.
  if (node->GetState () == CS_QUAD_FULL) return false;

  if (i00 && i01 && i11 && i10)
  {
    // This node is completely contained in the polygon.
    // We know the node was not completely full so we make it
    // full and indicate to the caller that the node state has changed.
    node->SetState (CS_QUAD_FULL);
    return true;
  }

  bool vis = false;
  if (i00 || i01 || i11 || i10)
  {
    // At least one vertex of this node is in the polygon.
    // This means the polygon is visible in the node.
    vis = true;
  }
  else
  {
    // None of the vertices of the node are in the polygon.
    // In this case we must perform a more heavy test.

    // If any of the polygon vertices is in the node then
    // the polygon is visible.
    int i;
    for (i = 0 ; i < num_verts ; i++)
      if (csFrustrum::Contains (node->corners, 4, plane_normal, verts[i]))
      {
        vis = true;
	break;
      }

    if (!vis)
      vis = csFrustrum::IsVisibleFull (node->corners, 4, verts, num_verts);
  }

  // If polygon is not visible in node then nothing happens
  // here.
  if (!vis) return false;

  // The polygon partially overlaps with the node.
  // In this case we set the state of the node to CS_QUAD_PARTIAL
  // and traverse to the children (if any).
  bool was_empty = (node->GetState () == CS_QUAD_EMPTY);
  node->SetState (CS_QUAD_PARTIAL);

  csQuadtreeNode** children = node->children;
  if (!children[0])
  {
    // This node has no children. Just return that the state
    // of the node has changed (note that going from
    // CS_QUAD_PARTIAL to CS_QUAD_PARTIAL is also seen as a state
    // change because this means that the polygon may be visible here).
    return true;
  }
  else
  {
    // This node has children.
    if (was_empty)
    {
      children[0]->SetState (CS_QUAD_EMPTY);
      children[1]->SetState (CS_QUAD_EMPTY);
      children[2]->SetState (CS_QUAD_EMPTY);
      children[3]->SetState (CS_QUAD_EMPTY);
    }
    bool i0a = csFrustrum::Contains (verts, num_verts, plane_normal, children[0]->GetCorner (1));
    bool i1a = csFrustrum::Contains (verts, num_verts, plane_normal, children[3]->GetCorner (2));
    bool ia0 = csFrustrum::Contains (verts, num_verts, plane_normal, children[0]->GetCorner (3));
    bool ia1 = csFrustrum::Contains (verts, num_verts, plane_normal, children[1]->GetCorner (2));
    bool iaa = csFrustrum::Contains (verts, num_verts, plane_normal, children[0]->GetCorner (2));
    bool rc0 = InsertPolygon (children[0], verts, num_verts, i00, i0a, iaa, ia0);
    bool rc1 = InsertPolygon (children[1], verts, num_verts, i0a, i01, ia1, iaa);
    bool rc3 = InsertPolygon (children[2], verts, num_verts, iaa, ia1, i11, i1a);
    bool rc2 = InsertPolygon (children[3], verts, num_verts, ia0, iaa, i1a, i10);
    if (children[0]->GetState () == CS_QUAD_FULL &&
        children[1]->GetState () == CS_QUAD_FULL &&
        children[2]->GetState () == CS_QUAD_FULL &&
        children[3]->GetState () == CS_QUAD_FULL)
    {
      // If all children are full then this node also becomes full.
      node->SetState (CS_QUAD_FULL);
    }
    // If one of the children changed state then we have changed state.
    return rc0 || rc1 || rc2 || rc3;
  }

  return true;
}

bool csQuadtree::InsertPolygon (csVector3* verts, int num_verts)
{
  bool i00 = csFrustrum::Contains (verts, num_verts, plane_normal, root->GetCorner (0));
  bool i01 = csFrustrum::Contains (verts, num_verts, plane_normal, root->GetCorner (1));
  bool i11 = csFrustrum::Contains (verts, num_verts, plane_normal, root->GetCorner (2));
  bool i10 = csFrustrum::Contains (verts, num_verts, plane_normal, root->GetCorner (3));
  return InsertPolygon (root, verts, num_verts, i00, i01, i11, i10);
}

bool csQuadtree::TestPolygon (csQuadtreeNode* node,
	csVector3* verts, int num_verts,
	bool i00, bool i01, bool i11, bool i10)
{
  // If node is completely full then polygon is not
  // visible in this node.
  if (node->GetState () == CS_QUAD_FULL) return false;

  if (i00 && i01 && i11 && i10)
  {
    // This node is completely contained in the polygon.
    // Since the node is not full we have to assume that the
    // polygon can be visible in the node.
    return true;
  }

  bool vis = false;
  if (i00 || i01 || i11 || i10)
  {
    // At least one vertex of this node is in the polygon.
    // This means the polygon is visible in the node.
    vis = true;
  }
  else
  {
    // None of the vertices of the node are in the polygon.
    // In this case we must perform a more heavy test.
    vis = csFrustrum::IsVisible (node->corners, 4, verts, num_verts);
  }

  // If polygon is not visible in node then nothing happens
  // here.
  if (!vis) return false;

  // The polygon partially overlaps with the node.
  // In this case we check the state in the children (if any).

  csQuadtreeNode** children = node->children;
  if (!children[0])
  {
    // This node has no children. This means that the polygon
    // is visible in this node.
    return true;
  }
  else
  {
    // This node has children.
    bool i0a = csFrustrum::Contains (verts, num_verts, plane_normal, children[0]->GetCorner (1));
    bool i1a = csFrustrum::Contains (verts, num_verts, plane_normal, children[3]->GetCorner (2));
    bool ia0 = csFrustrum::Contains (verts, num_verts, plane_normal, children[0]->GetCorner (3));
    bool ia1 = csFrustrum::Contains (verts, num_verts, plane_normal, children[1]->GetCorner (2));
    bool iaa = csFrustrum::Contains (verts, num_verts, plane_normal, children[0]->GetCorner (2));
    if (TestPolygon (children[0], verts, num_verts, i00, i0a, iaa, ia0)) return true;
    if (TestPolygon (children[1], verts, num_verts, i0a, i01, ia1, iaa)) return true;
    if (TestPolygon (children[2], verts, num_verts, iaa, ia1, i11, i1a)) return true;
    if (TestPolygon (children[3], verts, num_verts, ia0, iaa, i1a, i10)) return true;
    // Polygon is not visible in any of the children.
    return false;
  }

  return true;
}

bool csQuadtree::TestPolygon (csVector3* verts, int num_verts)
{
  bool i00 = csFrustrum::Contains (verts, num_verts, plane_normal, root->GetCorner (0));
  bool i01 = csFrustrum::Contains (verts, num_verts, plane_normal, root->GetCorner (1));
  bool i10 = csFrustrum::Contains (verts, num_verts, plane_normal, root->GetCorner (2));
  bool i11 = csFrustrum::Contains (verts, num_verts, plane_normal, root->GetCorner (3));
  return TestPolygon (root, verts, num_verts, i00, i01, i11, i10);
}

