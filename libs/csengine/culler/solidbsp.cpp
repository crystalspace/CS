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
    poly->Intersect (node->splitter, left_poly, right_poly, onplane);
    bool rc1, rc2;

    if (left_poly->GetNumEdges () == 0 && !onplane)
    {
      // Left polygon has no edges. We test if the left node
      // is completely contained in the right polygon. In that
      // case we can clear the subtree and mark it as solid.
      if (!node->left->solid && right_poly->In (node->split_center))
      {
        node->left->solid = true;
	node_pool.Free (node->left->left); node->left->left = NULL;
	node_pool.Free (node->left->right); node->left->right = NULL;
	rc1 = true;
      }
      else rc1 = false;
    }
    else rc1 = InsertPolygon (node->left, left_poly);

    if (right_poly->GetNumEdges () == 0 && !onplane)
    {
      if (!node->right->solid && left_poly->In (node->split_center))
      {
        node->right->solid = true;
	node_pool.Free (node->right->left); node->right->left = NULL;
	node_pool.Free (node->right->right); node->right->right = NULL;
	rc2 = true;
      }
      else rc2 = false;
    }
    else rc2 = InsertPolygon (node->right, right_poly);

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
    // @@@ Consider a lazy option where you don't subdivide
    // the polygon until really needed.
    csSolidBspNode* n = node;
    int i;
    for (i = 0 ; i < poly->GetNumEdges () ; i++)
    {
      n->splitter = csPlane2 ((*poly)[i].v1, (*poly)[i].v2);
      n->split_center = ((*poly)[i].v1 + (*poly)[i].v2) / 2;
      n->left = node_pool.Alloc ();
      n->right = node_pool.Alloc ();
      n = n->right;
    }
    n->solid = true;

    return true;
  }
  return false;
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

    if (left_poly->GetNumEdges () == 0 && !onplane)
    {
      // Left polygon has no edges. We test if the left node
      // is completely contained in the right polygon. In that
      // case the solid state of that node is important for visibility.
      if (!node->left->solid && right_poly->In (node->split_center))
      { rc = true; goto end; }
    }
    else if (TestPolygon (node->left, left_poly)) { rc = true; goto end; }

    if (right_poly->GetNumEdges () == 0 && !onplane)
    {
      if (!node->right->solid && left_poly->In (node->split_center))
      { rc = true; goto end; }
    }
    else if (TestPolygon (node->right, right_poly)) { rc = true; goto end; }

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

bool csSolidBsp::InsertPolygon (csVector2* verts, int num_verts)
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
  bool rc = InsertPolygon (root, poly);
  poly_pool.Free (poly);
  return rc;
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
	  ig2d->DrawPixel (v.x, height-v.y, red);
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

    ig2d->DrawPixel ((int)spc.x, (int)height-spc.y, blue);
    ig2d->DrawPixel ((int)spc.x-1, (int)height-spc.y-1, blue);
    ig2d->DrawPixel ((int)spc.x-2, (int)height-spc.y-2, blue);
    ig2d->DrawPixel ((int)spc.x+1, (int)height-spc.y-1, blue);
    ig2d->DrawPixel ((int)spc.x+2, (int)height-spc.y-2, blue);
    ig2d->DrawPixel ((int)spc.x-1, (int)height-spc.y+1, blue);
    ig2d->DrawPixel ((int)spc.x-2, (int)height-spc.y+2, blue);
    ig2d->DrawPixel ((int)spc.x+1, (int)height-spc.y+1, blue);
    ig2d->DrawPixel ((int)spc.x+2, (int)height-spc.y+2, blue);
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
