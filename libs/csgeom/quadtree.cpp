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
#include "csgeom/poly2d.h"

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

//--------------------------------------------------------------------------

void csQuadtree::Build (csQuadtreeNode* node, const csBox& box, int depth)
{
  node->SetCenter ((box.Min () + box.Max ())/2);
  if (depth <= 0) return;
  const csVector2& center = node->GetCenter ();

  csBox childbox;

  CHK (node->children[0] = new csQuadtreeNode ());
  childbox.Set (box.Min (), center);
  Build (node->children[0], childbox, depth-1);

  CHK (node->children[1] = new csQuadtreeNode ());
  childbox.Set (center.x, box.MinY (), box.MaxX (), center.y);
  Build (node->children[1], childbox, depth-1);

  CHK (node->children[2] = new csQuadtreeNode ());
  childbox.Set (center, box.Max ());
  Build (node->children[2], childbox, depth-1);

  CHK (node->children[3] = new csQuadtreeNode ());
  childbox.Set (box.MinX (), center.y, center.x, box.MaxY ());
  Build (node->children[3], childbox, depth-1);
}

csQuadtree::csQuadtree (const csBox& box, int depth)
{
  bbox = box;
  CHK (root = new csQuadtreeNode ());
  Build (root, box, depth-1);
}

csQuadtree::~csQuadtree ()
{
  CHK (delete root);
}

bool IsVisibleFull (
	csVector2* frustrum, int num_frust,
  	csVector2* poly, int num_poly)
{
  int i1, j1, i, j;

  // Here is the difficult case. We need to see if there is an
  // edge from the polygon which intersects a frustrum plane.
  // If so then polygon is visible. Otherwise not.
  csVector2 isect;
  float dist;
  j1 = num_frust-1;
  for (j = 0 ; j < num_frust ; j++)
  {
    i1 = num_poly-1;
    for (i = 0 ; i < num_poly ; i++)
    {
      if (csIntersect2::Segments (poly[i], poly[i1],
      	frustrum[j], frustrum[j1], isect, dist))
      {
        return true;
      }
      i1 = i;
    }
    j1 = j;
  }
  return false;
}

// Test if a bounding box is completely inside a convex polygon.
bool BoxEntirelyInPolygon (csVector2* verts, int num_verts, const csBox& bbox)
{
  return (csPoly2D::In (verts, num_verts, bbox.GetCorner (0)) &&
          csPoly2D::In (verts, num_verts, bbox.GetCorner (1)) &&
          csPoly2D::In (verts, num_verts, bbox.GetCorner (2)) &&
          csPoly2D::In (verts, num_verts, bbox.GetCorner (3)));
}

bool csQuadtree::InsertPolygon (csQuadtreeNode* node,
	csVector2* verts, int num_verts,
	const csBox& cur_bbox, const csBox& pol_bbox)
{
  // If node is completely full already then nothing can happen.
  if (node->GetState () == CS_QUAD_FULL) return false;

  csQuadtreeNode** children = node->children;
  // If there are no children then this node is set to state partial.
  if (!children[0])
  {
    node->SetState (CS_QUAD_PARTIAL);
    return true;
  }

  // If there are children and we are empty then we propagate
  // the empty state to the children. This is because our
  // state is going to change so the children need to be valid.
  if (node->GetState () == CS_QUAD_EMPTY)
  {
    children[0]->SetState (CS_QUAD_EMPTY);
    children[1]->SetState (CS_QUAD_EMPTY);
    children[2]->SetState (CS_QUAD_EMPTY);
    children[3]->SetState (CS_QUAD_EMPTY);
  }

  csBox childbox;
  const csVector2& center = node->GetCenter ();
  bool vis, rc1, rc2, rc3, rc4;
  csVector2 v;
  int i;
  rc1 = rc2 = rc3 = rc4 = false;

  // center_vis contains visibility info about the visibility
  // of the center inside the polygon.
  bool center_vis = csPoly2D::In (verts, num_verts, center);
  // Visibility information for the four central points of the sides
  // of every edge of the total area. We precompute this because
  // we're going to need this information anyway.
  v.x = cur_bbox.MinX ();
  v.y = center.y;
  bool left_vis = csPoly2D::In (verts, num_verts, v);
  v.x = cur_bbox.MaxX ();
  bool right_vis = csPoly2D::In (verts, num_verts, v);
  v.x = center.x;
  v.y = cur_bbox.MinY ();
  bool top_vis = csPoly2D::In (verts, num_verts, v);
  v.y = cur_bbox.MaxY ();
  bool bottom_vis = csPoly2D::In (verts, num_verts, v);

  // Check the bounding box of the polygon against all four
  // child areas (by comparing the bounding box against the center).
  // If the bounding box overlaps the child area then we continue
  // the check to see if the polygon also overlaps the child area.

  // Child 0 (top/left).
  if (children[0]->GetState () != CS_QUAD_FULL &&
  	pol_bbox.MinX () <= center.x && pol_bbox.MinY () <= center.y)
  {
    vis = false;
    // If any of the three corners is in polygon then polygon is visible.
    if (center_vis || left_vis || top_vis) vis = true;
    // If bbox is entirely in child area then polygon is visible.
    else if (pol_bbox.MaxX () <= center.x && pol_bbox.MaxY () <= center.y) vis = true;
    else if (pol_bbox.MaxX () <= center.x)
    {
      // If bbox crosses child area vertically but does not cross
      // it horizontally (both left and right) then polygon is visible.
      if (pol_bbox.MinX () >= cur_bbox.MinX ()) vis = true;
      // Else we have a difficult case. Here we know the bottom-left corner
      // of the child area is inside the bounding box. We test every vertex
      // of the polygon and see if any is inside the child area. In that
      // case the polygon is visible.
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].y <= center.y && verts[i].x >= cur_bbox.MinX ())
	  {
	    vis = true;
	    break;
	  }
    }
    else if (pol_bbox.MaxY () <= center.y)
    {
      if (pol_bbox.MinY () >= cur_bbox.MinY ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].x <= center.x && verts[i].y >= cur_bbox.MinY ())
	  {
	    vis = true;
	    break;
	  }
    }
    else
      for (i = 0 ; i < num_verts ; i++)
	if (verts[i].x <= center.x && verts[i].y <= center.y)
	{
	  vis = true;
	  break;
	}

    if (vis)
    {
      // We already calculated wether or not three of the four corners
      // of the child area are inside the polygon. We only need
      // to test the fourth one. If all are in the polygon then
      // the node is full and we can stop recursion.
      if (center_vis && left_vis && top_vis && csPoly2D::In (verts, num_verts, cur_bbox.Min ()))
      {
        children[0]->SetState (CS_QUAD_FULL);
	rc1 = true;
      }
      else
      {
        childbox.Set (cur_bbox.Min (), center);
        rc1 = InsertPolygon (children[0], verts, num_verts, childbox, pol_bbox);
      }
    }
  }

  // Child 1 (top/right).
  if (children[1]->GetState () != CS_QUAD_FULL &&
  	pol_bbox.MaxX () > center.x && pol_bbox.MinY () <= center.y)
  {
    vis = false;
    if (center_vis || right_vis || top_vis) vis = true;
    else if (pol_bbox.MinX () >= center.x && pol_bbox.MaxY () <= center.y) vis = true;
    else if (pol_bbox.MinX () >= center.x)
    {
      if (pol_bbox.MaxX () <= cur_bbox.MaxX ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].y <= center.y && verts[i].x <= cur_bbox.MaxX ())
	  {
	    vis = true;
	    break;
	  }
    }
    else if (pol_bbox.MaxY () <= center.y)
    {
      if (pol_bbox.MinY () >= cur_bbox.MinY ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].x >= center.x && verts[i].y >= cur_bbox.MinY ())
	  {
	    vis = true;
	    break;
	  }
    }
    else
      for (i = 0 ; i < num_verts ; i++)
	if (verts[i].x >= center.x && verts[i].y <= center.y)
	{
	  vis = true;
	  break;
	}

    if (vis)
    {
      v.x = cur_bbox.MaxX ();
      v.y = cur_bbox.MinY ();
      if (center_vis && right_vis && top_vis && csPoly2D::In (verts, num_verts, v))
      {
        children[1]->SetState (CS_QUAD_FULL);
	rc2 = true;
      }
      else
      {
        childbox.Set (center.x, cur_bbox.MinY (), cur_bbox.MaxX (), center.y);
        rc2 = InsertPolygon (children[1], verts, num_verts, childbox, pol_bbox);
      }
    }
  }

  // Child 2 (bottom/right).
  if (children[2]->GetState () != CS_QUAD_FULL &&
  	pol_bbox.MaxX () > center.x && pol_bbox.MaxY () > center.y)
  {
    vis = false;
    if (center_vis || right_vis || bottom_vis) vis = true;
    else if (pol_bbox.MinX () >= center.x && pol_bbox.MinY () >= center.y) vis = true;
    else if (pol_bbox.MinX () >= center.x)
    {
      if (pol_bbox.MaxX () <= cur_bbox.MaxX ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].y >= center.y && verts[i].x <= cur_bbox.MaxX ())
	  {
	    vis = true;
	    break;
	  }
    }
    else if (pol_bbox.MinY () >= center.y)
    {
      if (pol_bbox.MaxY () <= cur_bbox.MaxY ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].x >= center.x && verts[i].y <= cur_bbox.MaxY ())
	  {
	    vis = true;
	    break;
	  }
    }
    else
      for (i = 0 ; i < num_verts ; i++)
	if (verts[i].x >= center.x && verts[i].y >= center.y)
	{
	  vis = true;
	  break;
	}

    if (vis)
    {
      if (center_vis && right_vis && top_vis && csPoly2D::In (verts, num_verts, cur_bbox.Max ()))
      {
        children[2]->SetState (CS_QUAD_FULL);
	rc3 = true;
      }
      else
      {
        childbox.Set (center, cur_bbox.Max ());
        rc3 = InsertPolygon (children[2], verts, num_verts, childbox, pol_bbox);
      }
    }
  }

  // Child 3 (bottom/left).
  if (children[3]->GetState () != CS_QUAD_FULL &&
  	pol_bbox.MinX () <= center.x && pol_bbox.MaxY () > center.y)
  {
    vis = false;
    if (center_vis || left_vis || bottom_vis) vis = true;
    else if (pol_bbox.MaxX () <= center.x && pol_bbox.MinY () >= center.y) vis = true;
    else if (pol_bbox.MaxX () <= center.x)
    {
      if (pol_bbox.MinX () >= cur_bbox.MinX ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].y >= center.y && verts[i].x >= cur_bbox.MinX ())
	  {
	    vis = true;
	    break;
	  }
    }
    else if (pol_bbox.MinY () >= center.y)
    {
      if (pol_bbox.MaxY () <= cur_bbox.MaxY ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].x <= center.x && verts[i].y <= cur_bbox.MaxY ())
	  {
	    vis = true;
	    break;
	  }
    }
    else
      for (i = 0 ; i < num_verts ; i++)
	if (verts[i].x >= center.x && verts[i].y >= center.y)
	{
	  vis = true;
	  break;
	}

    if (vis)
    {
      v.x = cur_bbox.MinX ();
      v.y = cur_bbox.MaxY ();
      if (center_vis && right_vis && top_vis && csPoly2D::In (verts, num_verts, v))
      {
        children[3]->SetState (CS_QUAD_FULL);
	rc4 = true;
      }
      else
      {
  	childbox.Set (cur_bbox.MinX (), center.y, center.x, cur_bbox.MaxY ());
        rc4 = InsertPolygon (children[3], verts, num_verts, childbox, pol_bbox);
      }
    }
  }

  if (children[0]->GetState () == CS_QUAD_FULL &&
      children[1]->GetState () == CS_QUAD_FULL &&
      children[2]->GetState () == CS_QUAD_FULL &&
      children[3]->GetState () == CS_QUAD_FULL)
  {
    node->SetState (CS_QUAD_FULL);
    return true;
  }

  node->SetState (CS_QUAD_PARTIAL);
  return true;
}

bool csQuadtree::InsertPolygon (csVector2* verts, int num_verts, const csBox& pol_bbox)
{
  // If root is already full then there is nothing that can happen further.
  if (root->GetState () == CS_QUAD_FULL) return false;

  // If the bounding box of the tree does not overlap with the bounding box of
  // the polygon then we can return false here.
  if (!bbox.Overlap (pol_bbox)) return false;

  // If bounding box of tree is completely inside bounding box of polygon then
  // it is possible that tree is completely in polygon. We test that condition
  // further.
  if (bbox < pol_bbox)
  {
    if (BoxEntirelyInPolygon (verts, num_verts, bbox))
    {
      // Polygon completely covers tree. In that case set state
      // of tree to full and return true.
      root->SetState (CS_QUAD_FULL);
      return true;
    }
  }

  return InsertPolygon (root, verts, num_verts, bbox, pol_bbox);
}

bool csQuadtree::TestPolygon (csQuadtreeNode* node,
	csVector2* verts, int num_verts,
	const csBox& cur_bbox, const csBox& pol_bbox)
{
  // If node is completely full already then nothing can happen.
  if (node->GetState () == CS_QUAD_FULL) return false;
  // If node is completely empty then polygon is always visible.
  if (node->GetState () == CS_QUAD_EMPTY) return true;

  csQuadtreeNode** children = node->children;
  // If there are no children then we assume polygon is not visible.
  // This is an optimization which is not entirely correct@@@
  if (!children[0]) return true;

  csBox childbox;
  const csVector2& center = node->GetCenter ();
  bool vis;
  csVector2 v;
  int i;

  // center_vis contains visibility info about the visibility
  // of the center inside the polygon.
  bool center_vis = csPoly2D::In (verts, num_verts, center);
  // Visibility information for the four central points of the sides
  // of every edge of the total area. We precompute this because
  // we're going to need this information anyway.
  v.x = cur_bbox.MinX ();
  v.y = center.y;
  bool left_vis = csPoly2D::In (verts, num_verts, v);
  v.x = cur_bbox.MaxX ();
  bool right_vis = csPoly2D::In (verts, num_verts, v);
  v.x = center.x;
  v.y = cur_bbox.MinY ();
  bool top_vis = csPoly2D::In (verts, num_verts, v);
  v.y = cur_bbox.MaxY ();
  bool bottom_vis = csPoly2D::In (verts, num_verts, v);

  // Check the bounding box of the polygon against all four
  // child areas (by comparing the bounding box against the center).
  // If the bounding box overlaps the child area then we continue
  // the check to see if the polygon also overlaps the child area.

  // Child 0 (top/left).
  if (children[0]->GetState () != CS_QUAD_FULL &&
  	pol_bbox.MinX () <= center.x && pol_bbox.MinY () <= center.y)
  {
    vis = false;
    // If any of the three corners is in polygon then polygon is visible.
    if (center_vis || left_vis || top_vis) vis = true;
    // If bbox is entirely in child area then polygon is visible.
    else if (pol_bbox.MaxX () <= center.x && pol_bbox.MaxY () <= center.y) vis = true;
    else if (pol_bbox.MaxX () <= center.x)
    {
      // If bbox crosses child area vertically but does not cross
      // it horizontally (both left and right) then polygon is visible.
      if (pol_bbox.MinX () >= cur_bbox.MinX ()) vis = true;
      // Else we have a difficult case. Here we know the bottom-left corner
      // of the child area is inside the bounding box. We test every vertex
      // of the polygon and see if any is inside the child area. In that
      // case the polygon is visible.
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].y <= center.y && verts[i].x >= cur_bbox.MinX ())
	  {
	    vis = true;
	    break;
	  }
    }
    else if (pol_bbox.MaxY () <= center.y)
    {
      if (pol_bbox.MinY () >= cur_bbox.MinY ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].x <= center.x && verts[i].y >= cur_bbox.MinY ())
	  {
	    vis = true;
	    break;
	  }
    }
    else
      for (i = 0 ; i < num_verts ; i++)
	if (verts[i].x <= center.x && verts[i].y <= center.y)
	{
	  vis = true;
	  break;
	}

    if (vis)
    {
      // We already calculated wether or not three of the four corners
      // of the child area are inside the polygon. We only need
      // to test the fourth one. If all are in the polygon then
      // we cover the node and we can stop recursion. Polygon is
      // visible in that case becase node is not full.
      if (center_vis && left_vis && top_vis && csPoly2D::In (verts, num_verts, cur_bbox.Min ()))
      {
        return true;
      }
      else
      {
        childbox.Set (cur_bbox.Min (), center);
        if (TestPolygon (children[0], verts, num_verts, childbox, pol_bbox)) return true;
      }
    }
  }

  // Child 1 (top/right).
  if (children[1]->GetState () != CS_QUAD_FULL &&
  	pol_bbox.MaxX () > center.x && pol_bbox.MinY () <= center.y)
  {
    vis = false;
    if (center_vis || right_vis || top_vis) vis = true;
    else if (pol_bbox.MinX () >= center.x && pol_bbox.MaxY () <= center.y) vis = true;
    else if (pol_bbox.MinX () >= center.x)
    {
      if (pol_bbox.MaxX () <= cur_bbox.MaxX ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].y <= center.y && verts[i].x <= cur_bbox.MaxX ())
	  {
	    vis = true;
	    break;
	  }
    }
    else if (pol_bbox.MaxY () <= center.y)
    {
      if (pol_bbox.MinY () >= cur_bbox.MinY ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].x >= center.x && verts[i].y >= cur_bbox.MinY ())
	  {
	    vis = true;
	    break;
	  }
    }
    else
      for (i = 0 ; i < num_verts ; i++)
	if (verts[i].x >= center.x && verts[i].y <= center.y)
	{
	  vis = true;
	  break;
	}

    if (vis)
    {
      v.x = cur_bbox.MaxX ();
      v.y = cur_bbox.MinY ();
      if (center_vis && right_vis && top_vis && csPoly2D::In (verts, num_verts, v))
      {
        return true;
      }
      else
      {
        childbox.Set (center.x, cur_bbox.MinY (), cur_bbox.MaxX (), center.y);
        if (TestPolygon (children[1], verts, num_verts, childbox, pol_bbox)) return true;
      }
    }
  }

  // Child 2 (bottom/right).
  if (children[2]->GetState () != CS_QUAD_FULL &&
  	pol_bbox.MaxX () > center.x && pol_bbox.MaxY () > center.y)
  {
    vis = false;
    if (center_vis || right_vis || bottom_vis) vis = true;
    else if (pol_bbox.MinX () >= center.x && pol_bbox.MinY () >= center.y) vis = true;
    else if (pol_bbox.MinX () >= center.x)
    {
      if (pol_bbox.MaxX () <= cur_bbox.MaxX ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].y >= center.y && verts[i].x <= cur_bbox.MaxX ())
	  {
	    vis = true;
	    break;
	  }
    }
    else if (pol_bbox.MinY () >= center.y)
    {
      if (pol_bbox.MaxY () <= cur_bbox.MaxY ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].x >= center.x && verts[i].y <= cur_bbox.MaxY ())
	  {
	    vis = true;
	    break;
	  }
    }
    else
      for (i = 0 ; i < num_verts ; i++)
	if (verts[i].x >= center.x && verts[i].y >= center.y)
	{
	  vis = true;
	  break;
	}

    if (vis)
    {
      if (center_vis && right_vis && top_vis && csPoly2D::In (verts, num_verts, cur_bbox.Max ()))
      {
        return true;
      }
      else
      {
        childbox.Set (center, cur_bbox.Max ());
        if (TestPolygon (children[2], verts, num_verts, childbox, pol_bbox)) return true;
      }
    }
  }

  // Child 3 (bottom/left).
  if (children[3]->GetState () != CS_QUAD_FULL &&
  	pol_bbox.MinX () <= center.x && pol_bbox.MaxY () > center.y)
  {
    vis = false;
    if (center_vis || left_vis || bottom_vis) vis = true;
    else if (pol_bbox.MaxX () <= center.x && pol_bbox.MinY () >= center.y) vis = true;
    else if (pol_bbox.MaxX () <= center.x)
    {
      if (pol_bbox.MinX () >= cur_bbox.MinX ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].y >= center.y && verts[i].x >= cur_bbox.MinX ())
	  {
	    vis = true;
	    break;
	  }
    }
    else if (pol_bbox.MinY () >= center.y)
    {
      if (pol_bbox.MaxY () <= cur_bbox.MaxY ()) vis = true;
      else
        for (i = 0 ; i < num_verts ; i++)
	  if (verts[i].x <= center.x && verts[i].y <= cur_bbox.MaxY ())
	  {
	    vis = true;
	    break;
	  }
    }
    else
      for (i = 0 ; i < num_verts ; i++)
	if (verts[i].x >= center.x && verts[i].y >= center.y)
	{
	  vis = true;
	  break;
	}

    if (vis)
    {
      v.x = cur_bbox.MinX ();
      v.y = cur_bbox.MaxY ();
      if (center_vis && right_vis && top_vis && csPoly2D::In (verts, num_verts, v))
      {
        return true;
      }
      else
      {
  	childbox.Set (cur_bbox.MinX (), center.y, center.x, cur_bbox.MaxY ());
        if (TestPolygon (children[3], verts, num_verts, childbox, pol_bbox)) return true;
      }
    }
  }

  return false;
}

bool csQuadtree::TestPolygon (csVector2* verts, int num_verts, const csBox& pol_bbox)
{
  // If root is already full then there is nothing that can happen further.
  if (root->GetState () == CS_QUAD_FULL) return false;

  // If the bounding box of the tree does not overlap with the bounding box of
  // the polygon then we can return false here.
  if (!bbox.Overlap (pol_bbox)) return false;

  // If bounding box of tree is completely inside bounding box of polygon then
  // it is possible that tree is completely in polygon. We test that condition
  // further.
  if (bbox < pol_bbox)
  {
    if (BoxEntirelyInPolygon (verts, num_verts, bbox))
    {
      // Polygon completely covers tree. In that case return
      // true because polygon will be visible (node is not full).
      return true;
    }
  }

  return TestPolygon (root, verts, num_verts, bbox, pol_bbox);
}


