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
#include "csgeom/polyint.h"
#include "csgeom/bsp.h"

//---------------------------------------------------------------------------

csBspNode::csBspNode ()
{
  front = back = NULL;
  polygons = NULL;
  num = max = 0;
}

csBspNode::~csBspNode ()
{
  CHK (delete [] polygons);
}

void csBspNode::AddPolygon (csPolygonInt* poly)
{
  if (!polygons)
  {
    max = 3;
    CHK (polygons = new csPolygonInt* [max]);
  }

  if (num >= max)
  {
    CHK (csPolygonInt** pp = new csPolygonInt* [max+2]);
    memcpy (pp, polygons, sizeof (csPolygonInt*)*max);
    max += 2;
    CHK (delete [] polygons);
    polygons = pp;
  }

  polygons[num] = poly;
  num++;
}

//---------------------------------------------------------------------------

csBspTree::csBspTree (csPolygonParentInt* pset)
{
  csBspTree::pset = pset;

  int i;
  int num = pset->GetNumPolygons ();
  CHK (csPolygonInt** polygons = new csPolygonInt* [num]);
  for (i = 0 ; i < num ; i++) polygons[i] = pset->GetPolygon (i);

  CHK (root = new csBspNode);

  Build (root, polygons, num);

  CHK (delete [] polygons);
}

csBspTree::csBspTree (csPolygonParentInt* pset, csPolygonInt** polygons, int num)
{
  csBspTree::pset = pset;
  CHK (root = new csBspNode);
  Build (root, polygons, num);
}

csBspTree::~csBspTree ()
{
  Clear (root);
}

void csBspTree::Clear (csBspNode* node)
{
  if (!node) return;
  Clear (node->front);
  Clear (node->back);
  CHK (delete node);
}

// Set this to 1 to optimize the BSP tree based on the number of
// splits. Otherwise the splitter will be chosen which has the
// highest number of polygons on the same plane as the splitter.
// @@@ This should be a parameter to the BSP tree building process.
#define BSP_MINIMAL_SPLIT 1

void csBspTree::Build (csBspNode* node, csPolygonInt** polygons, int num)
{
  int i, j, poly_idx;

#if BSP_MINIMAL_SPLIT
  // Choose the polygon which generates the least number of splits.
  int min_splits = 32767;
  poly_idx = -1;
  for (i = 0 ; i < num ; i++)
  {
    int cnt = 0;
    for (j = 0 ; j < num ; j++)
      if (polygons[j]->Classify (polygons[i]) == POL_SPLIT_NEEDED) cnt++;
    if (cnt < min_splits) { min_splits = cnt; poly_idx = i; }
  }
#else
  // First choose a polygon which shares its plane with the highest
  // number of other polygons.
  int same_poly = 0;
  poly_idx = -1;
  for (i = 0 ; i < num ; i++)
  {
    int cnt = 1;
    for (j = i+1 ; j < num ; j++)
      if (polygons[i]->SamePlane (polygons[j])) cnt++;
    if (cnt > same_poly) { same_poly = cnt; poly_idx = i; }
  }
#endif //BSP_MINIMAL_SPLIT

  csPlane* plane = polygons[poly_idx]->GetPolyPlane ();

  // Now we split the node according to the plane of that polygon.
  CHK (csPolygonInt** front_poly = new csPolygonInt* [num]);
  CHK (csPolygonInt** back_poly = new csPolygonInt* [num]);
  int front_idx = 0, back_idx = 0;

  for (i = 0 ; i < num ; i++)
  {
    int c = polygons[i]->Classify (polygons[poly_idx]);
    switch (c)
    {
      case POL_SAME_PLANE: node->AddPolygon (polygons[i]); break;
      case POL_FRONT: front_poly[front_idx++] = polygons[i]; break;
      case POL_BACK: back_poly[back_idx++] = polygons[i]; break;
      case POL_SPLIT_NEEDED:
	{
	  csVector3* front_v, * back_v;
	  int front_n, back_n, j;
	  int n = polygons[i]->GetNumVertices ();
  	  CHK (front_v = new csVector3 [n*2]);
  	  CHK (back_v = new csVector3 [n*2]);
	  polygons[i]->SplitWithPlane (front_v, front_n, back_v, back_n, *plane);

	  csPolygonInt* np1 = polygons[i]->Clone ();
	  csPolygonInt* np2 = polygons[i]->Clone ();	// polygons[i]@@@
	  np1->Reset ();
	  np2->Reset ();

	  for (j = 0 ; j < front_n ; j++)
	    np2->AddVertex (front_v[j]);
	  for (j = 0 ; j < back_n ; j++)
	    np1->AddVertex (back_v[j]);
	  polygons[i]->GetParent ()->AddPolygon (np1);
	  polygons[i]->GetParent ()->AddPolygon (np2);

	  np1->Finish ();
	  np2->Finish ();

	  CHK (delete [] front_v);
	  CHK (delete [] back_v);

	  front_poly[front_idx++] = np2;
	  back_poly[back_idx++] = np1;
	}
	break;

    }
  }

  if (front_idx)
  {
    CHK (node->front = new csBspNode);
    Build (node->front, front_poly, front_idx);
  }
  if (back_idx)
  {
    CHK (node->back = new csBspNode);
    Build (node->back, back_poly, back_idx);
  }

  CHK (delete [] front_poly);
  CHK (delete [] back_poly);
}

void* csBspTree::Back2Front (const csVector3& pos, csBspVisitFunc* func, void* data)
{
  return Back2Front (root, pos, func, data);
}

void* csBspTree::Front2Back (const csVector3& pos, csBspVisitFunc* func, void* data)
{
  return Front2Back (root, pos, func, data);
}

void* csBspTree::Back2Front (csBspNode* node, const csVector3& pos, csBspVisitFunc* func, void* data)
{
  if (!node) return NULL;
  void* rc;

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if ( csMath3::Visible(pos, *(node->polygons[0]->GetPolyPlane())) )
  {
    // Front.
    rc = Back2Front (node->back, pos, func, data);
    if (rc) return rc;
    rc = func (pset, node->polygons, node->num, data);
    if (rc) return rc;
    rc = Back2Front (node->front, pos, func, data);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Back2Front (node->front, pos, func, data);
    if (rc) return rc;
    rc = func (pset, node->polygons, node->num, data);
    if (rc) return rc;
    rc = Back2Front (node->back, pos, func, data);
    if (rc) return rc;
  }
  return NULL;
}

void* csBspTree::Front2Back (csBspNode* node, const csVector3& pos, csBspVisitFunc* func, void* data)
{
  if (!node) return NULL;
  void* rc;

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if ( csMath3::Visible(pos, *(node->polygons[0]->GetPolyPlane())) )
  {
    // Front.
    rc = Front2Back (node->front, pos, func, data);
    if (rc) return rc;
    rc = func (pset, node->polygons, node->num, data);
    if (rc) return rc;
    rc = Front2Back (node->back, pos, func, data);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Front2Back (node->back, pos, func, data);
    if (rc) return rc;
    rc = func (pset, node->polygons, node->num, data);
    if (rc) return rc;
    rc = Front2Back (node->front, pos, func, data);
    if (rc) return rc;
  }
  return NULL;
}

//---------------------------------------------------------------------------
