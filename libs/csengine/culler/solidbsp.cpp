/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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
#include "qint.h"
#include "csengine/solidbsp.h"
#include "csengine/pol2d.h"
#include "csengine/engine.h"
#include "ivideo/igraph2d.h"
#include "ivideo/itxtmgr.h"

//---------------------------------------------------------------------------

csSolidBspNodePool::~csSolidBspNodePool ()
{
  while (alloced)
  {
    PoolObj* n = alloced->next;
    //delete alloced->pi; @@@ This free is not valid!
    // We should use a ref count on the pool itself so that we
    // now when all objects in the pool are freed and the
    // 'alloced' list will be empty.
    delete alloced;
    alloced = n;
  }
  while (freed)
  {
    PoolObj* n = freed->next;
    delete freed->node;
    delete freed;
    freed = n;
  }
}

csSolidBspNode* csSolidBspNodePool::Alloc ()
{
  PoolObj* pnew;
  if (freed)
  {
    pnew = freed;
    freed = freed->next;
    // If this node has children we unlink them and
    // put them back on the 'freed' list.
    if (pnew->node->left)
    {
      Free (pnew->node->left);
      pnew->node->left = NULL;
    }
    if (pnew->node->right)
    {
      Free (pnew->node->right);
      pnew->node->right = NULL;
    }
    pnew->node->solid = false;
  }
  else
  {
    pnew = new PoolObj ();
    pnew->node = new csSolidBspNode ();
  }
  pnew->next = alloced;
  alloced = pnew;
  return pnew->node;
}

void csSolidBspNodePool::Free (csSolidBspNode* node)
{
  if (!node) return;
  if (alloced)
  {
    PoolObj* po = alloced;
    alloced = alloced->next;
    po->node = node;
    po->next = freed;
    freed = po;
  }
  else
  {
    // Cannot happen!
  }
}

void csSolidBspNodePool::Dump ()
{
  int cnt;
  cnt = 0;
  PoolObj* po = alloced;
  while (po) { cnt++; po = po->next; }
  printf ("PolyInt pool: %d allocated, ", cnt);
  cnt = 0;
  po = freed;
  while (po) { cnt++; po = po->next; }
  printf ("%d freed.\n", cnt);
}

//---------------------------------------------------------------------------

csSolidBspNode::~csSolidBspNode ()
{
  delete left;
  delete right;
}

//---------------------------------------------------------------------------

csSolidBspNodePool csSolidBsp::node_pool;
csPoly2DEdgesPool csSolidBsp::poly_pool;

//---------------------------------------------------------------------------

csSolidBsp::csSolidBsp ()
{
  root = node_pool.Alloc ();
}

csSolidBsp::~csSolidBsp ()
{
  node_pool.Free (root);
}

void csSolidBsp::MakeEmpty ()
{
  node_pool.Free (root->left); root->left = NULL;
  node_pool.Free (root->right); root->right = NULL;
  root->solid = false;
}

bool ddd = false;
static int ddd_level;

#if 0
bool csSolidBsp::InsertPolygon (csSolidBspNode* node, csPoly2DEdges* poly)
{
  if (node->solid) return false;
  if (poly->GetNumEdges () == 0) return false;

  if (node->left)
  {
    // If this node has children then we split to the given
    // splitter in this node.
    csPoly2DEdges* left_poly, * right_poly;
    left_poly = poly_pool.Alloc ();
    right_poly = poly_pool.Alloc ();
    bool onplane;
    poly->Intersect (node->splitter, *left_poly, *right_poly, onplane);
    bool rc1, rc2;

    if (onplane)
    {
      rc1 = InsertPolygon (node->left, left_poly);
      rc2 = InsertPolygon (node->right, right_poly);
    }
    else
    {
      if (left_poly->GetNumEdges () == 0)
      {
        // Left polygon has no edges. We test if the left node
        // is completely contained in the right polygon. In that
        // case we can clear the subtree and mark it as solid.
        if (!node->left->solid && right_poly->In (node->left->split_center))
        {
          node->left->solid = true;
	  node_pool.Free (node->left->left); node->left->left = NULL;
	  node_pool.Free (node->left->right); node->left->right = NULL;
	  rc1 = true;
        }
        else rc1 = false;
      }
      rc1 = InsertPolygon (node->left, left_poly);

      if (right_poly->GetNumEdges () == 0)
      {
        if (!node->right->solid && left_poly->In (node->right->split_center))
        {
          node->right->solid = true;
	  node_pool.Free (node->right->left); node->right->left = NULL;
	  node_pool.Free (node->right->right); node->right->right = NULL;
	  rc2 = true;
        }
        else rc2 = false;
      }
      rc2 = InsertPolygon (node->right, right_poly);
    }

    if (node->left->solid && node->right->solid)
    {
      node_pool.Free (node->left); node->left = NULL;
      node_pool.Free (node->right); node->right = NULL;
      node->solid = true;
    }

    poly_pool.Free (left_poly);
    poly_pool.Free (right_poly);
    return rc1 || rc2;
  }
  else
  {
    // Node has no children so we take a new splitter and
    // create children.

    // This flag indicates wether or not we added an edge.
    // If false then we ignored everything.
    bool edge_added = false;

    csSolidBspNode* n = node;
    int i, i1;
    i1 = poly->GetNumEdges ()-1;
    for (i = 0 ; i < poly->GetNumEdges () ; i++)
    {
      // If we have more than one edge then we test if this edge
      // is colinear with the previous one. In that case we simply
      // ignore the edge.
      if (i1 != i)
        if (ABS (csMath2::Area2 ((*poly)[i1].v1, (*poly)[i1].v2,
		(*poly)[i].v2)) < EPSILON &&
            ABS (csMath2::Area2 ((*poly)[i1].v1, (*poly)[i1].v2,
		(*poly)[i].v1)) < EPSILON)
	{
	  i1 = i;
	  continue;
	}
      edge_added = true;
      csVector2 start = (*poly)[i].v1;
      csVector2 end = (*poly)[i].v2;
      n->splitter.Set (start, end);
      n->split_center = (start + end) / 2;
     n->split_start = start;
     n->split_end = end;
      n->left = node_pool.Alloc ();
      n->right = node_pool.Alloc ();
      // @@@ This can potentially go wrong if we have a very thin node.
      // In that case the calculated split_center might be outside the node.
      // So we should have a more robust way to calculate this.
      n->left->split_center = n->split_center-n->splitter.norm / 20.;
      n->right->split_center = n->split_center+n->splitter.norm / 20.;
      n = n->right;
      i1 = i;
    }
    if (edge_added) n->solid = true;

    return true;
  }
  return false;
}
#else
bool csSolidBsp::InsertPolygon (csSolidBspNode* node, csPoly2DEdges* poly)
{
char ddd_spaces[200];
if (ddd)
{
int i;
for (i = 0 ; i < ddd_level ; i++) ddd_spaces[i] = ' ';
ddd_spaces[ddd_level] = 0;
printf ("%s === InsertPolygon ===\n", ddd_spaces);
for (i = 0 ; i < poly->GetNumEdges () ; i++)
printf ("%s   %d (%f,%f)-(%f,%f)\n",
ddd_spaces, i, (*poly)[i].Start ().x, (*poly)[i].Start ().y,
(*poly)[i].End ().x, (*poly)[i].End ().y);
if (node->left)
printf ("%s   children. Splitter=%f,%f,%f\n", ddd_spaces, node->splitter.A (),
node->splitter.B (), node->splitter.C ());
if (node->solid) printf ("%s   solid\n", ddd_spaces);
}
  if (node->solid) {
    if (ddd) printf ("%s   FALSE(1)\n", ddd_spaces); return false;
  }
  if (poly->GetNumEdges () == 0)
  {
    if (ddd) printf ("%s   FALSE(2)\n", ddd_spaces); return false;
  }

  if (node->left)
  {
if (ddd) printf ("%s   BRANCH: children\n", ddd_spaces);
    // If this node has children then we split to the given
    // splitter in this node.
    csPoly2DEdges* left_poly, * right_poly;
    left_poly = poly_pool.Alloc ();
    right_poly = poly_pool.Alloc ();
    bool onplane;
    poly->Intersect (node->splitter, *left_poly, *right_poly, onplane);
    bool rc1, rc2;

    if (onplane)
    {
if (ddd) printf ("%s   BRANCH: onplane\n", ddd_spaces);
      // If there was an edge on the splitter plane then we know that
      // the polygon can not be both in the left and the right side.
      // If the two polygons are both empty then we mark one of the two
      // nodes as solid depending on the edge from the original polygon.
      if (left_poly->GetNumEdges () == 0 && right_poly->GetNumEdges () == 0)
      {
if (ddd) printf ("%s   BRANCH: left->edges==0 right->edges==0\n", ddd_spaces);
        rc1 = rc2 = false;
	// Here we need to take care of the case that the edges in the
	// original polygon are not coplanar themselves. This can happen
	// if we have a very small polygon. In that case we just ignore
	// the polygon.
	bool left_solid = false, right_solid = false;
	int i;
	for (i = 0 ; i < poly->GetNumEdges () ; i++)
	{
          csPlane2 edge_plane ((*poly)[0]);
	  if (SIGN (edge_plane.A ()) == SIGN (node->splitter.A ()) &&
	      SIGN (edge_plane.B ()) == SIGN (node->splitter.B ()))
	    right_solid = true;
	  else
	    left_solid = true;
	}

	if (right_solid && !left_solid)
	{
if (ddd) printf ("%s   BRANCH: planes equal -> right solid\n", ddd_spaces);
	  // Two planes are equal. This means that the right node
	  // will be solid.
	  if (!node->right->solid)
	  {
if (ddd) printf ("%s   BRANCH: right solid\n", ddd_spaces);
            node->right->solid = true;
	    node_pool.Free (node->right->left); node->right->left = NULL;
	    node_pool.Free (node->right->right); node->right->right = NULL;
	    rc2 = true;
	  }
	}
	else if (left_solid && !right_solid)
	{
	  // Two planes are negated. This means that the left node
	  // will be solid.
if (ddd) printf ("%s   BRANCH: planes negated -> left solid\n", ddd_spaces);
	  if (!node->left->solid)
	  {
if (ddd) printf ("%s   BRANCH: left solid\n", ddd_spaces);
            node->left->solid = true;
	    node_pool.Free (node->left->left); node->left->left = NULL;
	    node_pool.Free (node->left->right); node->left->right = NULL;
	    rc1 = true;
	  }
	}
      }
      else
      {
if (ddd)
printf ("%s   BRANCH: BEFORE LEFT left->insert right->insert\n", ddd_spaces);
        rc1 = InsertPolygon (node->left, left_poly);
if (ddd)
printf ("%s   BRANCH: BEFORE RIGHT left->insert right->insert\n", ddd_spaces);
        rc2 = InsertPolygon (node->right, right_poly);
if (ddd)
printf ("%s   BRANCH: END left->insert right->insert\n", ddd_spaces);
      }
    }
    else
    {
if (ddd) printf ("%s   BRANCH: not onplane\n", ddd_spaces);
      if (left_poly->GetNumEdges () == 0)
      {
if (ddd) printf ("%s   BRANCH: left->edges==0\n", ddd_spaces);
        // Left polygon has no edges. We test if the left node
        // is completely contained in the right polygon. In that
        // case we can clear the subtree and mark it as solid.
        if (!node->left->solid && right_poly->In (node->left->split_center))
        {
if (ddd) printf ("%s   BRANCH: left->solid=true\n", ddd_spaces);
          node->left->solid = true;
	  node_pool.Free (node->left->left); node->left->left = NULL;
	  node_pool.Free (node->left->right); node->left->right = NULL;
	  rc1 = true;
        }
        else rc1 = false;
      }
else
{
if (ddd) printf ("%s   BRANCH: START left->insert\n", ddd_spaces);
      rc1 = InsertPolygon (node->left, left_poly);
if (ddd) printf ("%s   BRANCH: END left->insert\n", ddd_spaces);
}

      if (right_poly->GetNumEdges () == 0)
      {
if (ddd) printf ("%s   BRANCH: right->edges==0\n", ddd_spaces);
        if (!node->right->solid && left_poly->In (node->right->split_center))
        {
if (ddd) printf ("%s   BRANCH: right->solid=true\n", ddd_spaces);
          node->right->solid = true;
	  node_pool.Free (node->right->left); node->right->left = NULL;
	  node_pool.Free (node->right->right); node->right->right = NULL;
	  rc2 = true;
        }
        else rc2 = false;
      }
else
{
if (ddd) printf ("%s   BRANCH: START right->insert\n", ddd_spaces);
      rc2 = InsertPolygon (node->right, right_poly);
if (ddd) printf ("%s   BRANCH: END right->insert\n", ddd_spaces);
}
    }

    if (node->left->solid && node->right->solid)
    {
if (ddd) printf ("%s   BRANCH: both are solid\n", ddd_spaces);
      node_pool.Free (node->left); node->left = NULL;
      node_pool.Free (node->right); node->right = NULL;
      node->solid = true;
    }

    poly_pool.Free (left_poly);
    poly_pool.Free (right_poly);
    return rc1 || rc2;
  }
  else
  {
if (ddd) printf ("%s   BRANCH: no children\n", ddd_spaces);
    // Node has no children so we take a new splitter and
    // create children.

    // This flag indicates wether or not we added an edge.
    // If false then we ignored everything.
    bool edge_added = false;

    csSolidBspNode* n = node;
    int i, i1;
    i1 = poly->GetNumEdges ()-1;
    for (i = 0 ; i < poly->GetNumEdges () ; i++)
    {
      // If we have more than one edge then we test if this edge
      // is colinear with the previous one. In that case we simply
      // ignore the edge.
      if (i1 != i)
        if (ABS (csMath2::Area2 ((*poly)[i1].Start (), (*poly)[i1].End (),
		(*poly)[i].End ())) < EPSILON &&
            ABS (csMath2::Area2 ((*poly)[i1].Start (), (*poly)[i1].End (),
		(*poly)[i].Start ())) < EPSILON)
	{
	  i1 = i;
	  continue;
	}
      edge_added = true;
      csVector2 start = (*poly)[i].Start ();
      csVector2 end = (*poly)[i].End ();
      n->splitter.Set (start, end);
      n->split_center = (start + end) / 2;
     n->split_start = start;
     n->split_end = end;
      n->left = node_pool.Alloc ();
      n->right = node_pool.Alloc ();
      // @@@ This can potentially go wrong if we have a very thin node.
      // In that case the calculated split_center might be outside the node.
      // So we should have a more robust way to calculate this.
      n->left->split_center = n->split_center-n->splitter.norm / 20.;
      n->right->split_center = n->split_center+n->splitter.norm / 20.;
      n = n->right;
      i1 = i;
    }
    if (edge_added) n->solid = true;

    return true;
  }
  return false;
}
#endif

void csSolidBsp::InsertPolygonInv (csSolidBspNode* node, csPoly2DEdges* poly)
{
  // Node has no children so we take a new splitter and
  // create children.
  csSolidBspNode* n = node;
  int i;
  for (i = 0 ; i < poly->GetNumEdges () ; i++)
  {
    n->splitter.Set ((*poly)[i]);
    n->split_center = ((*poly)[i].Start () + (*poly)[i].End ()) / 2;
    n->left = node_pool.Alloc ();
    n->right = node_pool.Alloc ();
    // @@@ This can potentially go wrong if we have a very thin node.
    // In that case the calculated split_center might be outside the node.
    // So we should have a more robust way to calculate this.
    n->left->split_center = n->split_center-n->splitter.norm / 20.;
    n->right->split_center = n->split_center+n->splitter.norm / 20.;
    n->left->solid = true;
    n = n->right;
  }
}

#if 0
bool csSolidBsp::TestPolygon (csSolidBspNode* node, csPoly2DEdges* poly)
{
  if (node->solid) return false;
  if (poly->GetNumEdges () == 0) return false;
  bool rc;

  if (node->left)
  {
    // If this node has children then we split to the given
    // splitter in this node.
    csPoly2DEdges* left_poly, * right_poly;
    left_poly = poly_pool.Alloc ();
    right_poly = poly_pool.Alloc ();
    bool onplane;
    poly->Intersect (node->splitter, *left_poly, *right_poly, onplane);

    if (onplane)
    {
      if (TestPolygon (node->left, left_poly)) { rc = true; goto end; }
      if (TestPolygon (node->right, right_poly)) { rc = true; goto end; }
    }
    else
    {
      if (left_poly->GetNumEdges () == 0)
      {
        // Left polygon has no edges. We test if the left node
        // is completely contained in the right polygon. In that
        // case the solid state of that node is important for visibility.
        if (!node->left->solid && right_poly->In (node->left->split_center))
        { rc = true; goto end; }
      }
      else if (TestPolygon (node->left, left_poly)) { rc = true; goto end; }

      if (right_poly->GetNumEdges () == 0)
      {
        if (!node->right->solid && left_poly->In (node->right->split_center))
        { rc = true; goto end; }
      }
      else if (TestPolygon (node->right, right_poly)) { rc = true; goto end; }
    }

    rc = false;

    end:
    poly_pool.Free (left_poly);
    poly_pool.Free (right_poly);
    return rc;
  }
  else
  {
    return true;
  }
  return false;
}
#else
bool csSolidBsp::TestPolygon (csSolidBspNode* node, csPoly2DEdges* poly)
{
  if (node->solid) return false;
  if (poly->GetNumEdges () == 0) return false;
  bool rc;

  if (node->left)
  {
    // If this node has children then we split to the given
    // splitter in this node.
    csPoly2DEdges* left_poly, * right_poly;
    left_poly = poly_pool.Alloc ();
    right_poly = poly_pool.Alloc ();
    bool onplane;
    poly->Intersect (node->splitter, *left_poly, *right_poly, onplane);

    if (onplane)
    {
      // If there was an edge on the splitter plane then we know that
      // the polygon can not be both in the left and the right side.
      if (left_poly->GetNumEdges () == 0 && right_poly->GetNumEdges () == 0)
      {
	// Here we need to take care of the case that the edges in the
	// original polygon are not coplanar themselves. This can happen
	// if we have a very small polygon. In that case we just ignore
	// the polygon.
	bool left_solid = false, right_solid = false;
	int i;
	for (i = 0 ; i < poly->GetNumEdges () ; i++)
	{
          csPlane2 edge_plane ((*poly)[0]);
	  if (SIGN (edge_plane.A ()) == SIGN (node->splitter.A ()) &&
	      SIGN (edge_plane.B ()) == SIGN (node->splitter.B ()))
	    right_solid = true;
	  else
	    left_solid = true;
	}
	if (right_solid && !left_solid)
	{
	  // Two planes are equal.
	  if (!node->right->solid) { rc = true; goto end; }
	}
	else if (left_solid && !right_solid)
	{
	  // Two planes are negated.
	  if (!node->left->solid) { rc = true; goto end; }
	}
      }
      else
      {
        if (TestPolygon (node->left, left_poly)) { rc = true; goto end; }
        if (TestPolygon (node->right, right_poly)) { rc = true; goto end; }
      }
    }
    else
    {
      if (left_poly->GetNumEdges () == 0)
      {
        // Left polygon has no edges. We test if the left node
        // is completely contained in the right polygon. In that
        // case the solid state of that node is important for visibility.
        if (!node->left->solid && right_poly->In (node->left->split_center))
        { rc = true; goto end; }
      }
      else if (TestPolygon (node->left, left_poly)) { rc = true; goto end; }

      if (right_poly->GetNumEdges () == 0)
      {
        if (!node->right->solid && left_poly->In (node->right->split_center))
        { rc = true; goto end; }
      }
      else if (TestPolygon (node->right, right_poly)) { rc = true; goto end; }
    }

    rc = false;

    end:
    poly_pool.Free (left_poly);
    poly_pool.Free (right_poly);
    return rc;
  }
  else
  {
    return true;
  }
  return false;
}
#endif

csPolygon2D debug_poly2d;

bool csSolidBsp::InsertPolygon (csVector2* verts, int num_verts)
{
  csPoly2DEdges* poly = poly_pool.Alloc ();
  poly->SetNumEdges (0);
  debug_poly2d.SetNumVertices (0);
  ddd = csEngine::ProcessLastPolygon ();
if (ddd)
{
printf ("====================================\n");
Dump();
printf ("------------------------------------\n");
}
  int i, i1;
  i1 = num_verts-1;
  for (i = 0 ; i < num_verts ; i++)
  {
    debug_poly2d.AddVertex (verts[i]);
    poly->AddEdge (verts[i1], verts[i]);
    i1 = i;
  }
  bool rc = InsertPolygon (root, poly);
  poly_pool.Free (poly);
  return rc;
}

void csSolidBsp::InsertPolygonInv (csVector2* verts, int num_verts)
{
  csPoly2DEdges* poly = poly_pool.Alloc ();
  poly->SetNumEdges (0);
  int i, i1;
  i1 = num_verts-1;
  for (i = 0 ; i < num_verts ; i++)
  {
    poly->AddEdge (verts[i1], verts[i]);
    i1 = i;
  }
  InsertPolygonInv (root, poly);
  poly_pool.Free (poly);
}

bool csSolidBsp::TestPolygon (csVector2* verts, int num_verts)
{
  csPoly2DEdges* poly = poly_pool.Alloc ();
  poly->SetNumEdges (0);
  int i, i1;
  i1 = num_verts-1;
  for (i = 0 ; i < num_verts ; i++)
  {
    poly->AddEdge (verts[i1], verts[i]);
    i1 = i;
  }
  bool rc = TestPolygon (root, poly);
  poly_pool.Free (poly);
  return rc;
}

void csSolidBsp::Dump (csSolidBspNode* node, int level)
{
  if (!node) return;
  int i;
  char spaces[100];
  for (i = 0 ; i < level ; i++) spaces[i] = ' ';
  spaces[i] = 0;
  if (node->solid)
    printf ("%s solid\n", spaces);
  if (node->left)
  {
    printf ("%s children. Splitter=%f,%f,%f\n", spaces,
        node->splitter.A (), node->splitter.B (), node->splitter.C ());
    printf ("%s Sp_cent=%f,%f e=%f,%f - %f,%f\n", spaces,
        node->split_center.x, node->split_center.y,
	node->split_start.x, node->split_start.y,
	node->split_end.x, node->split_end.y);
    printf ("%s left side:\n", spaces);
    Dump (node->left, level+1);
    printf ("%s right side:\n", spaces);
    Dump (node->right, level+1);
  }
  else
    printf ("%s no children\n", spaces);
}

static int white, red, blue, green, yellow, black;

void csSolidBsp::GfxDump (csSolidBspNode* node, iGraphics2D* ig2d, int depth,
	csPoly2D& poly)
{
  if (!node) return;
  if (depth < 0) return;

//   int width = ig2d->GetWidth ();
  int height = ig2d->GetHeight ();

  if (node->solid)
  {
    csVector2 v;
    for (v.y = poly.GetBoundingBox ().Min ().y;
         v.y < poly.GetBoundingBox ().Max ().y; v.y += 4)
      for (v.x = poly.GetBoundingBox ().Min ().x;
           v.x < poly.GetBoundingBox ().Max ().x ; v.x += 4)
        if (poly.In (v))
	  ig2d->DrawPixel ((int)v.x, (int)(height-v.y), red);
  }
  if (depth == 0) return;

  if (!node->left) return;

  csPlane2& sp = node->splitter;
  const csVector2& spc = node->split_center;
  csSegment2 seg;

  if (csIntersect2::IntersectPolygon (sp, &poly, seg))
  {
    int col = depth == 1 ? yellow : white;
    const csVector2& v1 = seg.Start ();
    const csVector2& v2 = seg.End ();
    ig2d->DrawLine (v1.x, height-v1.y, v2.x, height-v2.y, col);
    csVector2 dir = sp.norm;
    dir /= dir.Norm ();
    dir *= 8;
    ig2d->DrawLine (spc.x, height-spc.y, spc.x+dir.x, height-(spc.y+dir.y),
    	col);

    ig2d->DrawPixel ((int)spc.x, (int)(height-spc.y), blue);
    ig2d->DrawPixel ((int)spc.x-1, (int)(height-spc.y-1), blue);
    ig2d->DrawPixel ((int)spc.x-2, (int)(height-spc.y-2), blue);
    ig2d->DrawPixel ((int)spc.x+1, (int)(height-spc.y-1), blue);
    ig2d->DrawPixel ((int)spc.x+2, (int)(height-spc.y-2), blue);
    ig2d->DrawPixel ((int)spc.x-1, (int)(height-spc.y+1), blue);
    ig2d->DrawPixel ((int)spc.x-2, (int)(height-spc.y+2), blue);
    ig2d->DrawPixel ((int)spc.x+1, (int)(height-spc.y+1), blue);
    ig2d->DrawPixel ((int)spc.x+2, (int)(height-spc.y+2), blue);
  }

  csPoly2D poly_left, poly_right;
  poly.Intersect (sp, poly_left, poly_right);

  GfxDump (node->left, ig2d, depth-1, poly_left);
  GfxDump (node->right, ig2d, depth-1, poly_right);
}

void csSolidBsp::GfxDump (iGraphics2D* ig2d, iTextureManager* itxtmgr,
  int depth)
{
  if (root && root->solid) return;
  white = itxtmgr->FindRGB (255, 255, 255);
  red = itxtmgr->FindRGB (255, 0, 0);
  green = itxtmgr->FindRGB (0, 255, 0);
  blue = itxtmgr->FindRGB (0, 0, 255);
  yellow = itxtmgr->FindRGB (255, 255, 0);
  black = itxtmgr->FindRGB (0, 0, 0);
  int width = ig2d->GetWidth ();
  int height = ig2d->GetHeight ();
  csPoly2D poly;
  poly.AddVertex (0, height);
  poly.AddVertex (width, height);
  poly.AddVertex (width, 0);
  poly.AddVertex (0, 0);
  GfxDump (root, ig2d, depth, poly);
}
