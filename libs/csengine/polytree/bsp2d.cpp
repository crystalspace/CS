/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
  
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
#include "csengine/bsp2d.h"
#include "isystem.h"

//---------------------------------------------------------------------------

csBspNode2D::csBspNode2D () : segments (10, 10)
{
  front = back = NULL;
}

csBspNode2D::~csBspNode2D ()
{
  CHK (delete front);
  CHK (delete back);
}

void csBspNode2D::AddSegment (csSegment2* seg)
{
  segments.Push (seg);
}

//---------------------------------------------------------------------------

csBspTree2D::csBspTree2D ()
{
  root = NULL;
}

csBspTree2D::~csBspTree2D ()
{
  CHK (delete root);
}

void csBspTree2D::Build (csSegment2** segments, int num)
{
  CHK (root = new csBspNode2D);
  Build (root, segments, num);
}

int csBspTree2D::SelectSplitter (csSegment2** segments, int num)
{
  int i, j, poly_idx;

  poly_idx = -1;

  // Several modes in the end come down to BSP_BALANCE_AND_SPLITS
  // with a different balance_factor and split_factor. Those two
  // factors determine how important the balance or split quality
  // is for the total quality factor.
  float balance_factor, split_factor;
  int cur_mode = BSP_MINIMIZE_SPLITS;
  if (cur_mode == BSP_BALANCED)
  {
    cur_mode = BSP_BALANCE_AND_SPLITS;
    balance_factor = 1;
    split_factor = .1;	// Don't completely ignore splitting.
  }
  else if (cur_mode == BSP_ALMOST_BALANCED)
  {
    cur_mode = BSP_ALMOST_BALANCE_AND_SPLITS;
    balance_factor = 1;
    split_factor = .1;
  }
  else if (cur_mode == BSP_MINIMIZE_SPLITS)
  {
    cur_mode = BSP_BALANCE_AND_SPLITS;
    balance_factor = 0;
    split_factor = 1;
  }
  else if (cur_mode == BSP_ALMOST_MINIMIZE_SPLITS)
  {
    cur_mode = BSP_ALMOST_BALANCE_AND_SPLITS;
    balance_factor = 0;
    split_factor = 1;
  }
  else
  {
    balance_factor = 1;
    split_factor = 4;
  }

  if (cur_mode == BSP_RANDOM)
  {
    poly_idx = rand () % num;
  }
  else if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS || cur_mode == BSP_BALANCE_AND_SPLITS)
  {
    // Combine balancing and least nr of splits.
    float least_penalty = 1000000;
    int n = num;
    if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS && n > 20) n = 20;
    int ii, jj;
    for (i = 0 ; i < n ; i++)
    {
      if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS)
        ii = rand () % num;
      else
        ii = i;
      int front = 0, back = 0;
      int splits = 0;
      csPlane2 split_plane (*segments[ii]);
      for (j = 0 ; j < n ; j++)
      {
        if (cur_mode == BSP_ALMOST_BALANCE_AND_SPLITS)
          jj = rand () % num;
	else
	  jj = j;
	float c1 = split_plane.Classify (segments[jj]->Start ());
	float c2 = split_plane.Classify (segments[jj]->End ());
	if (c1 < 0 && c2 < 0) back++;
	else if (c1 > 0 && c2 > 0) front++;
	else splits++;
      }
      // balance_penalty is 0 for a very good balanced tree and
      // 1 for a very bad one.
      float balance_penalty = ((float)ABS (front+splits-back))/(float)num;
      // split_penalty is 0 for a very good tree with regards to splitting
      // and 1 for a very bad one.
      float split_penalty = (float)splits/(float)num;
      // Total penalty is a combination of both penalties. 0 is very good,
      float penalty = balance_factor * balance_penalty + split_factor * split_penalty;

      if (penalty < least_penalty)
      {
        least_penalty = penalty;
	poly_idx = ii;
      }
    }
  }
  return poly_idx;
}

void csBspTree2D::Build (csBspNode2D* node, csSegment2** segments,
	int num)
{
  int i;

  csSegment2* split_segment = segments[SelectSplitter (segments, num)];
  csPlane2 split_plane (*split_segment);
  node->splitter = split_plane;

  // Now we split the node according to the plane of that polygon.
  CHK (csSegment2** front_seg = new csSegment2* [num]);
  CHK (csSegment2** back_seg = new csSegment2* [num]);
  int front_idx = 0, back_idx = 0;

  for (i = 0 ; i < num ; i++)
  {
    float c1 = split_plane.Classify (segments[i]->Start ());
    float c2 = split_plane.Classify (segments[i]->End ());
    if (ABS (c1) < EPSILON) c1 = 0;
    if (ABS (c2) < EPSILON) c2 = 0;
    if (c1 == 0 && c2 == 0)
    {
      // Same plane.
      node->AddSegment (segments[i]);
      segments[i] = NULL;
    }
    else if (c1 > 0 && c2 > 0)
    {
      // Front.
      front_seg[front_idx++] = segments[i];
      segments[i] = NULL;
    }
    else if (c1 < 0 && c2 < 0)
    {
      // Back.
      back_seg[back_idx++] = segments[i];
      segments[i] = NULL;
    }
    else
    {
      // Split needed.
      csVector2 isect;
      float dist;
      csIntersect2::Plane (*segments[i], split_plane, isect, dist);
      CHK (csSegment2* segf = new csSegment2 ());
      CHK (csSegment2* segb = new csSegment2 ());
      if (c1 < 0)
      {
        segb->Set (segments[i]->Start (), isect);
        segf->Set (isect, segments[i]->End ());
      }
      else
      {
        segf->Set (segments[i]->Start (), isect);
        segb->Set (isect, segments[i]->End ());
      }
      front_seg[front_idx++] = segf;
      back_seg[back_idx++] = segb;
    }
  }

  if (front_idx)
  {
    CHK (node->front = new csBspNode2D);
    Build (node->front, front_seg, front_idx);
  }
  if (back_idx)
  {
    CHK (node->back = new csBspNode2D);
    Build (node->back, back_seg, back_idx);
  }

  CHK (delete [] front_seg);
  CHK (delete [] back_seg);
}

void* csBspTree2D::Back2Front (const csVector2& pos, csTree2DVisitFunc* func,
	void* data)
{
  return Back2Front (root, pos, func, data);
}

void* csBspTree2D::Front2Back (const csVector2& pos, csTree2DVisitFunc* func,
	void* data)
{
  return Front2Back (root, pos, func, data);
}

void* csBspTree2D::Back2Front (csBspNode2D* node, const csVector2& pos,
	csTree2DVisitFunc* func, void* data)
{
  if (!node) return NULL;
  void* rc;

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (csMath2::Visible (pos, node->splitter))
  {
    // Front.
    rc = Back2Front (node->back, pos, func, data);
    if (rc) return rc;
    rc = func (node->segments.GetArray (), node->segments.Length (), data);
    if (rc) return rc;
    rc = Back2Front (node->front, pos, func, data);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Back2Front (node->front, pos, func, data);
    if (rc) return rc;
    rc = func (node->segments.GetArray (), node->segments.Length (), data);
    if (rc) return rc;
    rc = Back2Front (node->back, pos, func, data);
    if (rc) return rc;
  }
  return NULL;
}

void* csBspTree2D::Front2Back (csBspNode2D* node, const csVector2& pos,
	csTree2DVisitFunc* func, void* data)
{
  if (!node) return NULL;
  void* rc;

  // Check if some polygon (just take the first) of the polygons array
  // is visible from the given point. If so, we are in front of this node.
  if (csMath2::Visible (pos, node->splitter))
  {
    // Front.
    rc = Front2Back (node->front, pos, func, data);
    if (rc) return rc;
    rc = func (node->segments.GetArray (), node->segments.Length (), data);
    if (rc) return rc;
    rc = Front2Back (node->back, pos, func, data);
    if (rc) return rc;
  }
  else
  {
    // Back.
    rc = Front2Back (node->back, pos, func, data);
    if (rc) return rc;
    rc = func (node->segments.GetArray (), node->segments.Length (), data);
    if (rc) return rc;
    rc = Front2Back (node->front, pos, func, data);
    if (rc) return rc;
  }
  return NULL;
}

//---------------------------------------------------------------------------
