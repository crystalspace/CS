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

#include "sysdef.h"
#include "qint.h"
#include "csengine/solidbsp.h"
#include "csengine/pol2d.h"
#include "csengine/world.h"
#include "igraph2d.h"
#include "itxtmgr.h"

//---------------------------------------------------------------------------

csSolidBspNodePool::~csSolidBspNodePool ()
{
  while (alloced)
  {
    PoolObj* n = alloced->next;
    //CHK (delete alloced->pi); @@@ This free is not valid!
    // We should use a ref count on the pool itself so that we
    // now when all objects in the pool are freed and the
    // 'alloced' list will be empty.
    CHK (delete alloced);
    alloced = n;
  }
  while (freed)
  {
    PoolObj* n = freed->next;
    CHK (delete freed->node);
    CHK (delete freed);
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
    CHK (pnew = new PoolObj ());
    CHK (pnew->node = new csSolidBspNode ());
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
  CHK (delete left);
  CHK (delete right);
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

bool csSolidBsp::InsertPolygon (csSolidBspNode* node, csPoly2DEdges* poly)
{
if (ddd)
{
printf ("=== InsertPolygon ===\n");
int i;
for (i = 0 ; i < poly->GetNumEdges () ; i++)
printf ("  %d (%f,%f)-(%f,%f)\n", i, (*poly)[i].v1.x, (*poly)[i].v1.y,
(*poly)[i].v2.x, (*poly)[i].v2.y);
if (node->left) printf ("  children. Splitter=%f,%f,%f\n", node->splitter.A (),
node->splitter.B (), node->splitter.C ());
if (node->solid) printf ("  solid\n");
}
  if (node->solid) { if (ddd) printf ("  FALSE(1)\n"); return false; }
  if (poly->GetNumEdges () == 0) { if (ddd) printf ("  FALSE(2)\n"); return false; }

  if (node->left)
  {
if (ddd) printf ("  BRANCH: children\n");
    // If this node has children then we split to the given
    // splitter in this node.
    csPoly2DEdges* left_poly, * right_poly;
    left_poly = poly_pool.Alloc ();
    right_poly = poly_pool.Alloc ();
    bool onplane;
    poly->Intersect (node->splitter, left_poly, right_poly, onplane);
    bool rc1, rc2;

    if (onplane)
    {
if (ddd) printf ("  BRANCH: onplane\n");
      // If there was an edge on the splitter plane then we know that
      // the polygon can not be both in the left and the right side.
      // If the two polygons are both empty then we mark one of the two
      // nodes as solid depending on the edge from the original polygon.
      if (left_poly->GetNumEdges () == 0 && right_poly->GetNumEdges () == 0)
      {
if (ddd) printf ("  BRANCH: left->edges==0 right->edges==0\n");
        rc1 = rc2 = false;
        csPlane2 edge_plane ((*poly)[0].v1, (*poly)[0].v2);
	if (SIGN (edge_plane.A ()) == SIGN (node->splitter.A ()) &&
	    SIGN (edge_plane.B ()) == SIGN (node->splitter.B ()))
	{
if (ddd) printf ("  BRANCH: planes equal -> right solid\n");
	  // Two planes are equal. This means that the right node
	  // will be solid.
	  if (!node->right->solid)
	  {
if (ddd) printf ("  BRANCH: right solid\n");
            node->right->solid = true;
	    node_pool.Free (node->right->left); node->right->left = NULL;
	    node_pool.Free (node->right->right); node->right->right = NULL;
	    rc2 = true;
	  }
	}
	else
	{
	  // Two planes are negated. This means that the left node
	  // will be solid.
if (ddd) printf ("  BRANCH: planes negated -> left solid\n");
	  if (!node->left->solid)
	  {
if (ddd) printf ("  BRANCH: left solid\n");
            node->left->solid = true;
	    node_pool.Free (node->left->left); node->left->left = NULL;
	    node_pool.Free (node->left->right); node->left->right = NULL;
	    rc1 = true;
	  }
	}
      }
      else
      {
if (ddd) printf ("  BRANCH: left->insert right->insert\n");
        rc1 = InsertPolygon (node->left, left_poly);
        rc2 = InsertPolygon (node->right, right_poly);
      }
    }
    else
    {
if (ddd) printf ("  BRANCH: not onplane\n");
      if (left_poly->GetNumEdges () == 0)
      {
if (ddd) printf ("  BRANCH: left->edges==0\n");
        // Left polygon has no edges. We test if the left node
        // is completely contained in the right polygon. In that
        // case we can clear the subtree and mark it as solid.
        if (!node->left->solid && right_poly->In (node->split_center))
        {
if (ddd) printf ("  BRANCH: left->solid=true\n");
          node->left->solid = true;
	  node_pool.Free (node->left->left); node->left->left = NULL;
	  node_pool.Free (node->left->right); node->left->right = NULL;
	  rc1 = true;
        }
        else rc1 = false;
      }
else
{
if (ddd) printf ("  BRANCH: left->insert\n");
      rc1 = InsertPolygon (node->left, left_poly);
}

      if (right_poly->GetNumEdges () == 0)
      {
if (ddd) printf ("  BRANCH: right->edges==0\n");
        if (!node->right->solid && left_poly->In (node->split_center))
        {
if (ddd) printf ("  BRANCH: right->solid=true\n");
          node->right->solid = true;
	  node_pool.Free (node->right->left); node->right->left = NULL;
	  node_pool.Free (node->right->right); node->right->right = NULL;
	  rc2 = true;
        }
        else rc2 = false;
      }
else
{
if (ddd) printf ("  BRANCH: right->insert\n");
      rc2 = InsertPolygon (node->right, right_poly);
}
    }

    if (node->left->solid && node->right->solid)
    {
if (ddd) printf ("  BRANCH: both are solid\n");
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
if (ddd) printf ("  BRANCH: no children\n");
    // Node has no children so we take a new splitter and
    // create children.
    // @@@ Consider a lazy option where you don't subdivide
    // the polygon until really needed.
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
      n->splitter.Set ((*poly)[i].v1, (*poly)[i].v2);
      n->split_center = ((*poly)[i].v1 + (*poly)[i].v2) / 2;
      n->split_start = (*poly)[i].v1;
      n->split_end = (*poly)[i].v2;
      n->left = node_pool.Alloc ();
      n->right = node_pool.Alloc ();
      n = n->right;
      i1 = i;
    }
    n->solid = true;

    return true;
  }
  return false;
}

void csSolidBsp::InsertPolygonInv (csSolidBspNode* node, csPoly2DEdges* poly)
{
  // Node has no children so we take a new splitter and
  // create children.
  csSolidBspNode* n = node;
  int i;
  for (i = 0 ; i < poly->GetNumEdges () ; i++)
  {
    n->splitter.Set ((*poly)[i].v1, (*poly)[i].v2);
    n->split_center = ((*poly)[i].v1 + (*poly)[i].v2) / 2;
    n->left = node_pool.Alloc ();
    n->right = node_pool.Alloc ();
    n->left->solid = true;
    n = n->right;
  }
}

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
    poly->Intersect (node->splitter, left_poly, right_poly, onplane);

    if (onplane)
    {
      // If there was an edge on the splitter plane then we know that
      // the polygon can not be both in the left and the right side.
      if (left_poly->GetNumEdges () == 0 && right_poly->GetNumEdges () == 0)
      {
        csPlane2 edge_plane ((*poly)[0].v1, (*poly)[0].v2);
	if (SIGN (edge_plane.A ()) == SIGN (node->splitter.A ()) &&
	    SIGN (edge_plane.B ()) == SIGN (node->splitter.B ()))
	{
	  // Two planes are equal.
	  if (!node->right->solid) { rc = true; goto end; }
	}
	else
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
        if (!node->left->solid && right_poly->In (node->split_center))
        { rc = true; goto end; }
      }
      else if (TestPolygon (node->left, left_poly)) { rc = true; goto end; }

      if (right_poly->GetNumEdges () == 0)
      {
        if (!node->right->solid && left_poly->In (node->split_center))
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

csPolygon2D debug_poly2d;

bool csSolidBsp::InsertPolygon (csVector2* verts, int num_verts)
{
  csPoly2DEdges* poly = poly_pool.Alloc ();
  poly->SetNumEdges (0);
  debug_poly2d.SetNumVertices (0);
  ddd = csWorld::ProcessLastPolygon ();
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
    printf ("%s Split_center=%f,%f edge=%f,%f - %f,%f\n", spaces,
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
    for (v.y = poly.GetBoundingBox ().Min ().y ; v.y < poly.GetBoundingBox ().Max ().y ; v.y += 4)
      for (v.x = poly.GetBoundingBox ().Min ().x ; v.x < poly.GetBoundingBox ().Max ().x ; v.x += 4)
        if (poly.In (v))
	  ig2d->DrawPixel ((int)v.x, (int)(height-v.y), red);
  }
  if (depth == 0) return;

  if (!node->left) return;

  csPlane2& sp = node->splitter;
  const csVector2& spc = node->split_center;
  csVector2 v1, v2;

  if (sp.IntersectPolygon (&poly, v1, v2))
  {
    int col = depth == 1 ? yellow : white;
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
  poly.Intersect (sp, &poly_left, &poly_right);

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

//---------------------------------------------------------------------------
