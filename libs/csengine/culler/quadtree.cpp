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

#include "cssysdef.h"
#include "csgeom/frustum.h"
#include "csgeom/poly2d.h"
#include "csengine/quadtree.h"
#include "csengine/world.h"
#include "isystem.h"


bool BoxEntirelyInPolygon (csVector2* verts, int num_verts, const csBox2& bbox)
{
  return (csPoly2D::In (verts, num_verts, bbox.GetCorner (0)) &&
          csPoly2D::In (verts, num_verts, bbox.GetCorner (1)) &&
          csPoly2D::In (verts, num_verts, bbox.GetCorner (2)) &&
          csPoly2D::In (verts, num_verts, bbox.GetCorner (3)));
}


/// computes 2**x
static int Pow2(int x)
{
  int res = 1;
  for(int i=0; i<x; i++)
    res <<= 1;
  return res;
}


csQuadTree :: csQuadTree (const csBox2& the_box, int the_depth)
{
  bbox = the_box;
  max_depth= the_depth;
  root_state = CS_QUAD_EMPTY;
  if(depth < 1)
  {
    CsPrintf(MSG_FATAL_ERROR, "QuadTree: Depth too small\n");
    exit(1);
  }
  /// first calculate the number of nodes.
  /// each depth 4* the nodes at the previous depth are added.
  /// depth 1 has the root node.
  /// nr of nodes in leaves at depth n is: 4**(n-1)
  /// nr of nodes in the tree with depth n: (4**n - 1) / 3
  int nr_leaves = Pow2(2* (max_depth-1) );
  int nr_nodes = ( Pow2( 2*n ) - 1 ) / 3;

  /// 4 nodes per byte, the root node is stored seperately
  state_size = (nr_nodes - 1) / 4;
  CsPrintf(MSG_STDOUT, "QuadTree: depth %d, nodes %d, statesize %d bytes\n", 
    max_depth, nr_nodes, state_size);

  if(state_size > 0)
  {
    states = new unsigned char[state_size];
    /// and every node is EMPTY at the start
    memset(states, CS_QUAD_ALL_EMPTY, state_size);  
  }
  else states = NULL;
}


csQuadTree :: ~csQuadTree ()
{
  delete[] states;
}


void csQuadTree :: CallChildren(int (csQuadTree::*func)(
    const csBox2& node_bbox, int node_depth, int node_state, int offset, 
    int node_nr, void* data), 
  const csBox2& box, int depth, int offset, int node_nr, void *data)
{
  int retval[4];
  CallChildren(func, box, depth, offset, node_nr, data, retval);
}

void csQuadTree :: CallChildren(int (csQuadTree::*func)(
    const csBox2& node_bbox, int node_depth, int node_state, int offset, 
    int node_nr, void* data), 
  const csBox2& box, int depth, int offset, int node_nr, 
  void *data, int retval[4])
{
  if(depth >= max_depth)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: leaf trying to recurse.\n");
    return;
  }
  // call func for each child of parent node with (offset, node_nr)
  csBox2 childbox;
  csVector2 center = box.GetCenter();
  int childstate, childoffset, childnr;

  /*
   * states are ordered like this:
   * root has children in byte 0.
   * nodes in byte 0 have children in byte 0+node_nr+1(1,2,3,4).
   * nodes in byte 1 have children in byte 4+node_nr+1.
   * nodes in byte n have children in byte 4*n+node_nr+1
   * So for byte n, take 4*n + node_nr+1 as the new byte
   * that new byte has the states of it's four children.
  */
  if(offset == -1)
    childoffset = 0; // root node's children
  else childoffset = 4 * offset + node_nr + 1;
  for(childnr=0; childnr<4; childnr++)
  {
    // compute new bounding box.
    switch(childnr)
    {
      case 0 /*topleft*/ : 
        childbox.Set(box.Min(), center); break;
      case 1 /*topright*/ : 
        childbox.Set(center.x, box.MinY(), box.MaxX(), center.y); break;
      case 2 /*bottomright*/ : 
        childbox.Set(center, box.Max()); break;
      case 3 /*bottomleft*/ : 
        childbox.Set(box.MinX(), center.y, center.x, box.MaxY()); break;
      default: CsPrintf(MSG_FATAL_ERROR, "QuadTree: Unknown child\n");
    }
    childstate = GetNodeState(childoffset, childnr);
    retval[childnr] = func(childbox, depth+1, childstate, childoffset, 
      childnr, data);
  }
}

/// masks and shifts for GetNodeState and SetNodeState
static const int node_masks[4] = {0xC0, 0x30, 0x0C, 0x03};
static const int node_shifts[4] = {6, 4, 2, 0};

int csQuadTree :: GetNodeState(int offset, int nodenr)
{
  if(offset > state_size)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: state out of range\n");
    return 0;
  }
  if(offset == -1)
    return root_state;
  unsigned char bits = states[offset];
  bits &= node_masks[nodenr];
  bits >>= node_shifts[nodenr];
  return bits;
}


void csQuadTree :: SetNodeState(int offset, int nodenr, int newstate)
{
  if(offset > state_size)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: setstate out of range\n");
    return;
  }
  if(offset == -1)
  {
    root_state = newstate;
    return;
  }
  unsigned char bits = states[offset];     // get all bits
  bits &= ~node_masks[nodenr];             // purge this node's bits
  bits |= newstate << node_shifts[nodenr]; // insert new bits
  states[offset] = bits;                   // store bits
}


void csQuadTree :: MakeEmpty()
{
  root_state = CS_QUAD_EMPTY;
  if(states)
    memset(states, CS_QUAD_ALL_EMPTY, state_size);  
}


int csQuadTree :: mark_node_func (const csBox2& node_bbox,
  int node_depth, int node_state, int offset, int node_nr, void* data)
{
  SetNodeState(offset, node_nr, (int)data);
}


int csQuadTree :: insert_polygon_func (const csBox2& node_bbox,
  int node_depth, int node_state, int offset, int node_nr, void* data)
{
  struct poly_info info& = *(struct poly_info*)data;
  if(node_state == CS_QUAD_UNKNOWN)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: insertpoly quad_unknown.\n");
    return CS_QUAD_NOCHANGE;
  }
  if(node_state == CS_QUAD_FULL) // full already, skip it.
    return CS_QUAD_NOCHANGE;

  /// perhaps none of the node is covered?
  if (!node_bbox.Overlap (info.pol_bbox)) 
    return CS_QUAD_NOCHANGE;

  ///----------------------------------------------------
  /// So, the polygon bbox overlaps this node. How much?
  ///----------------------------------------------------

  /// is the whole node covered?
  /// first check bounding boxes then precisely.
  if(info.pol_bbox.Contains(node_bbox) &&
    BoxEntirelyInPolygon(info.verts, info.num_verts, node_bbox))
  {
    if(!info.test_only)
      SetNodeState(offset, node_nr, CS_QUAD_FULL);
    /// mark children (if any) as unknown, since they should not be reached.
    if(node_depth < max_depth)
      CallChildren(mark_node_func, node_bbox, node_depth, node_offset, 
        node_nr, (void*)CS_QUAD_UNKNOWN);
    return CS_QUAD_CERTAINCHANGE;
  }
  
  /// So a part of the node may be covered, perhaps none.
  /// could test some more points here.
  if( csPoly2D::In(info.verts, info.num_verts, node_bbox->GetCenter())
    || node_bbox.Intersect(info.verts, info.num_verts))
  { 
    int old_node_state = node_state;
    // so it overlaps a bit.
    if(node_state == CS_QUAD_EMPTY && node_depth < max_depth)
    {
      // mark children as empty now, since they should be empty, and
      // can be reached now...
      CallChildren(mark_node_func, node_bbox, node_depth, node_offset, 
        node_nr, (void*)CS_QUAD_EMPTY);
    }
    // this node is partially covered.
    if(!info.test_only)
      SetNodeState(offset, node_nr, CS_QUAD_PARTIAL);
    // if any children they can process the polygon too. And they determine
    // the return value.
    if(node_depth < max_depth)
    {
      int retval[4];
      CallChildren(insert_polygon_func, node_bbox, node_depth, node_offset, 
        node_nr, data, retval);
      return GetPolygonResult(retval);
    }
    /// no children, return value for change, either 
    /// EMPTY->PARTIAL (certain change) or 
    /// PARTIAL->PARTIAL (possible change)
    if(old_node_state == CS_QUAD_EMPTY)
      return CS_QUAD_CERTAINCHANGE;
    return CS_QUAD_POSSIBLECHANGE;
  }
  /// polygon bound overlaps, but polygon itself does not intersect us
  /// i.e. the polygon is not in this node. No change, nothing added here.
  return CS_QUAD_NOCHANGE;
}


int csQuadTree :: GetPolygonResult(int retval[4])
{
  // returns the max change.
  // since NOCHANGE(0) < POSSIBLECHANGE(1) < CERTAINCHANGE(2)
  int res = CS_QUAD_NOCHANGE;
  for(int i=0; i<4; i++)
    if(retval[i] > res)
      res = retval[i];
  return res;
}


int csQuadTree :: InsertPolygon (csVector2* verts, int num_verts,
  const csBox2& pol_bbox)
{
  struct poly_info info;
  info.verts = verts;
  info.num_verts = num_verts;
  info.pol_bbox = pol_bbox;
  info.test_only = false;
  return insert_polygon_func(bbox, 1, root_state, -1, 0, (void*)&info);
}


int csQuadTree :: TestPolygon (csVector2* verts, int num_verts,
  const csBox2& pol_bbox)
{
  struct poly_info info;
  info.verts = verts;
  info.num_verts = num_verts;
  info.pol_bbox = pol_bbox;
  info.test_only = true;
  return insert_polygon_func(bbox, 1, root_state, -1, 0, (void*)&info);
}


int csQuadTree :: GetTestPointResult(int retval[4])
{
  /// returns UNKNOWN if all retval are UNKNOWN.
  /// returns PARTIAL if at least one is PARTIAL, or
  ///   both FULL and EMPTY are present.
  /// returns FULL when all non-unknown values are FULL
  /// returns EMPTY when all non-unknown values are EMPTY
  int res = CS_QUAD_UNKNOWN;
  for(int i=0; i<4; i++)
    if(retval[i] != CS_QUAD_UNKNOWN)
    {
      if(res == CS_QUAD_UNKNOWN)
        res = retval[i];
      else if(res == CS_QUAD_PARTIAL || retval[i] == CS_QUAD_PARTIAL)
        res = CS_QUAD_PARTIAL;
      else // res must be EMPTY or FULL now. so must retval[i].
        if(res != retval[i])
          res = CS_QUAD_PARTIAL;
    }
  return res;
}


int csQuadTree :: test_point_func (const csBox2& node_bbox,
  int node_depth, int node_state, int offset, int node_nr, void* data)
{
  if(node_state == CS_QUAD_UNKNOWN)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: testpoint quad_unknown.\n");
    return CS_QUAD_UNKNOWN;
  }
  if(!node_bbox.In(*(csVector2*)data))
    return CS_QUAD_UNKNOWN;
  if(node_state != CS_QUAD_PARTIAL || node_depth == max_depth)
    return node_state;
  // for a partial covered node with children, call the children
  int retval[4];
  CallChildren(test_point_func, node_bbox, node_depth, node_offset, node_nr,
    data, retval);
  return GetTestPointResult(retval);
}


int csQuadTree :: TestPoint (const csVector2& point)
{
  return test_point_func(bbox, 1, root_state, -1, 0, (void*)&point);
}

