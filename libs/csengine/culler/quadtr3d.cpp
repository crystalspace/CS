/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Copyright (C) 2000 by Wouter Wijngaards

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
#include "csengine/quadtr3d.h"
#include "csengine/engine.h" /* for CsPrintf() -- bleh */
#include "csgeom/frustum.h"
#include "csgeom/poly2d.h"
#include "isys/isystem.h"

/// computes 2**x
static int Pow2(int x)
{
  int res = 1;
  for(int i=0; i<x; i++)
    res <<= 1;
  return res;
}


/// This contains the info that designates thr position of a particular node.
class q3d_node_pos_info
{
public:
  /// Index into states array; the offset -1 denotes the root node.
  int offset;
  /// Index into byte. [0..3]
  int bytepos;
  /// Depth into tree 1==root
  int depth;
  /// Width and height of square plane of nodes.
  int plane_size;
  /// Index in array where the node's at depth are located
  int plane_start;
  /**
   * Coordinates of the node in it's plane.
   * Integer in the plane of leaves at node_depth.
   * <p>
   * Note that these values are redundant.
   *  <li>offset = plane_start + (node_y * plane_size + node_x) / 4
   *  <li>bytepos = (node_y * plane_size + node_x) % 4
   *  <li>plane_size = 2**(depth-1)
   *  <li>plane_start = (4**(depth-1) -1 ) / 3
   * But it is easier to pass these values along.
   */
  int node_x, node_y;

  /// Bounding box of the node.  Note: not correctly computed at this time.
  csBox3 bounds;
  /// Frustum for the node
  csFrustum *frust;

  /// Construct a pos info, uninitialized.
  q3d_node_pos_info();
  /// Copy constructor
  q3d_node_pos_info(const q3d_node_pos_info&);
  /// Delete the node_pos, delete frustum as well
  ~q3d_node_pos_info();
  /// Clear the q3d_node_pos_info, to point to root node.
  void set_root(csQuadTree3D*);
};

q3d_node_pos_info::q3d_node_pos_info()
{
  // uninited. Call set_root!
  frust = 0;
}


q3d_node_pos_info::~q3d_node_pos_info()
{
  delete frust;
}

void q3d_node_pos_info::set_root(csQuadTree3D *tree)
{
  // init to point to root state.
  offset = -1;
  bytepos = 0;
  depth = 1;
  plane_size = 1;
  plane_start = 0;
  node_x = 0;
  node_y = 0;
  bounds = tree->GetBoundingBox();
  frust = new csFrustum(tree->GetCenter(), tree->GetCorners(), 4);
}

q3d_node_pos_info::q3d_node_pos_info(const q3d_node_pos_info &copy)
{
  offset = copy.offset;
  bytepos = copy.bytepos;
  depth = copy.depth;
  plane_size = copy.plane_size;
  plane_start = copy.plane_start;
  node_x = copy.node_x;
  node_y = copy.node_y;
  bounds = copy.bounds;
  frust = new csFrustum(*copy.frust);
}


csQuadTree3D::csQuadTree3D (const csVector3& start_center,
    const csVector3 startcorners[4],
    const csBox3& bounding_box, int the_depth)
{
  int k, i;
  bbox = bounding_box;
  max_depth = the_depth;
  root_state = CS_QUAD3D_EMPTY;
  center = start_center;

  if(max_depth < 1)
  {
    CsPrintf(MSG_FATAL_ERROR, "QuadTree3D: Depth too small.\n");
    exit(1);
  }

  for(i=0; i<4; i++)
    sizes[i] = new csVector3[max_depth+1];
  for(i=0; i<4; i++)
    corners[i] = startcorners[i];

  /// fill the sizes lookup table.
  for(i=0; i<4; i++)
  {
    int next = (i+1) % 4;
    sizes[i][0] = corners[next] - corners[i];
    for(k = 1; k<=max_depth; k++)
      sizes[i][k] = sizes[i][k-1] / 2.0f;
  }

  /// first calculate the number of nodes.
  /// each depth 4* the nodes at the previous depth are added.
  /// depth 1 has the root node.
  /// nr of nodes in leaves at depth n is: 4**(n-1)
  /// nr of nodes in the tree with depth n: (4**n - 1) / 3
  int nr_leaves = Pow2(2* (max_depth-1) );
  int nr_nodes = ( Pow2( 2*max_depth ) - 1 ) / 3;

  /// 4 nodes per byte, the root node is stored seperately
  state_size = (nr_nodes - 1) / 4;
  printf("QuadTree3D: depth %d, nodes %d, leaves %d, statesize %d"
    " bytes.\n", max_depth, nr_nodes, nr_leaves, state_size);

  if(state_size > 0)
  {
    states = new unsigned char[state_size];
    /// and every node is EMPTY at the start
    memset(states, CS_QUAD3D_ALL_EMPTY, state_size);
  }
  else states = NULL;
}


csQuadTree3D::~csQuadTree3D ()
{
  delete[] states;
  int i;
  for(i=0; i<4; i++)
    delete[] sizes[i];
}

void csQuadTree3D::SetMainFrustum (const csVector3& start_center,
    const csVector3 startcorners[4])
{
  int k, i;
  center = start_center;

  for(i=0; i<4; i++)
    corners[i] = startcorners[i];

  /// fill the sizes lookup table.
  for(i=0; i<4; i++)
  {
    int next = (i+1) % 4;
    sizes[i][0] = corners[next] - corners[i];
    for(k = 1; k<=max_depth; k++)
      sizes[i][k] = sizes[i][k-1] / 2.0f;
  }
}


void csQuadTree3D::CallChildren(quad_traverse_func func, csQuadTree3D* pObj,
  q3d_node_pos_info *parent_pos, void *data)
{
  int retval[4];
  pObj->CallChildren(func, pObj, parent_pos, data, retval);
}


void csQuadTree3D::CallChildren(quad_traverse_func func, csQuadTree3D* pObj,
  q3d_node_pos_info *parent_pos, void *data, int retval[4])
{
  if(parent_pos->depth >= pObj->max_depth)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree3D: leaf trying to recurse.\n");
    return;
  }
  // call func for each child of parent node with (offset, node_nr)
  csVector3 child_u, child_v;
  /// this depends on the fact that rectangles have 90 degree corners.
  child_u = sizes[0][parent_pos->depth];
  child_v = sizes[1][parent_pos->depth];

  // make a copy of parent position, and adjust values.
  // during recursion only depth times will there be a q3d_node_pos_info on
  // stack.  say 20 deep, times 28 bytes -> only 560 bytes.
  q3d_node_pos_info child_pos(*parent_pos);
  child_pos.depth = parent_pos->depth+1;
  child_pos.plane_size = parent_pos->plane_size << 1;
  child_pos.plane_start = parent_pos->plane_start +
    (parent_pos->plane_size * parent_pos->plane_size) / 4;
  int child_x = parent_pos->node_x << 1;
  int child_y = parent_pos->node_y << 1;

  // for ease
  csVector3* child_verts = child_pos.frust->GetVertices();
  csVector3* parent_verts = parent_pos->frust->GetVertices();

  /// counted in nr of nodes, how manyeth node is the topleft node
  /// of the children from it's plane_start
  int nodenr = child_pos.plane_size * child_y + child_x;
  int childnr = 0;

  for(int y = 0; y < 2; y++)
  {
    for(int x = 0; x < 2; x++)
    {
      // compute array position
      child_pos.node_x = child_x + x;
      child_pos.node_y = child_y + y;
      child_pos.offset = child_pos.plane_start + (nodenr >> 2);
      child_pos.bytepos = nodenr & 0x3;

      // compute new bounding box.
      //   X = parent vertice
      //           --> child_u
      //       | X0---------X1
      //       V  | 0  | 1  |
      //  child_v |----+----|
      //          | 2  | 3  |
      //         X3---------X2
      // such that:
      //   X1 = X0 + child_u + child_u
      //   X2 = X0 + child_u + child_u + child_v + child_v
      //   X3 = X0 + child_v + child_v
      //
      // first vertice [0] of the child rectangle is determined.
      // Then the rest is filled in.
      switch(childnr)
      {
        case 0 /*topleft*/ :
          child_verts[0] = parent_verts[0]; break;
        case 1 /*topright*/ :
	  child_verts[0] = parent_verts[0] + child_u; break;
        case 2 /*bottomleft*/ :
	  child_verts[0] = parent_verts[0] + child_v; break;
        case 3 /*bottomright*/ :
	  child_verts[0] = parent_verts[0] + child_u + child_v; break;
        default: CsPrintf(MSG_FATAL_ERROR, "QuadTree3D: Unknown child.\n");
      }
      child_verts[1] = child_verts[0] + child_u;
      child_verts[2] = child_verts[1] + child_v;
      child_verts[3] = child_verts[0] + child_v;

      int childstate = pObj->GetNodeState(&child_pos);
      retval[childnr] = (this->*func)(pObj, childstate, &child_pos, data);

      nodenr += 1; // next node in this row
      childnr++; // next child.
    }
    // done all in the row
    // go down a row. that is plane_size bytes, subtract what was
    // added while processing the row.
    nodenr += child_pos.plane_size - 2;
  }
}


/// masks and shifts for GetNodeState and SetNodeState
static const int node_masks[4] = {0xC0, 0x30, 0x0C, 0x03};
static const int node_shifts[4] = {6, 4, 2, 0};

int csQuadTree3D::GetNodeState(int offset, int bytepos) const
{
  if(offset > state_size)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree3D: state out of range.\n");
    return 0;
  }
  if(offset == -1)
    return root_state;
  unsigned char bits = states[offset];
  bits &= node_masks[bytepos];
  bits >>= node_shifts[bytepos];
  return bits;
}


int csQuadTree3D::GetNodeState(q3d_node_pos_info *pos) const
{
  return GetNodeState(pos->offset, pos->bytepos);
}


void csQuadTree3D::SetNodeState(int offset, int bytepos, int newstate)
{
  if(offset > state_size)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree3D: setstate out of range.\n");
    return;
  }
  if(offset == -1)
  {
    root_state = newstate;
    return;
  }
  unsigned char bits = states[offset];      // get all bits
  bits &= ~node_masks[bytepos];             // purge this node's bits
  bits |= newstate << node_shifts[bytepos]; // insert new bits
  states[offset] = bits;                    // store bits
}


void csQuadTree3D::SetNodeState(q3d_node_pos_info *pos, int newstate)
{
  SetNodeState(pos->offset, pos->bytepos, newstate);
}


void csQuadTree3D::MakeEmpty()
{
  root_state = CS_QUAD3D_EMPTY;
  if(states)
    memset(states, CS_QUAD3D_ALL_EMPTY, state_size);
}


int csQuadTree3D::mark_node_func (csQuadTree3D* pObj,
  int /*node_state*/, q3d_node_pos_info *node_pos, void* data)
{
  pObj->SetNodeState(node_pos, (int)data);
  return (int)data;
}


int csQuadTree3D::insert_polygon_func (csQuadTree3D* pObj,
  int node_state, q3d_node_pos_info *node_pos, void* data)
{
  struct poly_info& info = *(struct poly_info*)data;
  if(node_state == CS_QUAD3D_UNKNOWN)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree3D: insertpoly quad_unknown.\n");
    Print();
    return CS_QUAD3D_NOCHANGE;
  }
  if(node_state == CS_QUAD3D_FULL) // full already, skip it.
    return CS_QUAD3D_NOCHANGE;

  /// Classify the polygon with repect to its frustum.
  int covered = csFrustum::Classify(node_pos->frust->GetVertices(), 4,
    info.verts, info.num_verts);

  if(covered < 0 || covered > 3)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree3D: Classify out of range.\n");
    return CS_QUAD3D_NOCHANGE;
  }

  if(0)
  printf("quadtree3d at %d Checking node %d+%d %d,%d (%d)\n", node_pos->depth,
    node_pos->offset, node_pos->bytepos, node_pos->node_x, node_pos->node_y,
    node_state);
  if(0)
  printf("quadtree3d rect (%f,%f,%f), (%f,%f,%f), (%f,%f,%f), (%f,%f,%f).\n",
    node_pos->frust->GetVertices()[0].x,
    node_pos->frust->GetVertices()[0].y,
    node_pos->frust->GetVertices()[0].z,
    node_pos->frust->GetVertices()[1].x,
    node_pos->frust->GetVertices()[1].y,
    node_pos->frust->GetVertices()[1].z,
    node_pos->frust->GetVertices()[2].x,
    node_pos->frust->GetVertices()[2].y,
    node_pos->frust->GetVertices()[2].z,
    node_pos->frust->GetVertices()[3].x,
    node_pos->frust->GetVertices()[3].y,
    node_pos->frust->GetVertices()[3].z );
  if(0)
  printf("quadtree3d classified as %d\n", covered);

  /// perhaps none of the node is covered?
  if (covered == CS_FRUST_OUTSIDE)
    return CS_QUAD3D_NOCHANGE;

  ///----------------------------------------------------
  /// So, the polygon bbox overlaps this node. How much?
  ///----------------------------------------------------

  /// is the whole node covered?
  /// // first check bounding boxes then precisely.
  /// // info.pol_bbox->Contains(node_bbox) &&
  if( covered == CS_FRUST_COVERED )
  {
    if(!info.test_only)
    {
      pObj->SetNodeState(node_pos, CS_QUAD3D_FULL);
      /// mark children (if any) as unknown, since they should not be reached.
      if(node_pos->depth < pObj->max_depth)
        pObj->CallChildren(&csQuadTree3D::mark_node_func, pObj,
          node_pos, (void*)CS_QUAD3D_UNKNOWN);
    }
    return CS_QUAD3D_CERTAINCHANGE;
  }

  /// So we are sure that a part of the node is covered.
  int old_node_state = node_state;
  // so it overlaps a bit.

  // if only testing and this is empty we already know a certain change.
  if(node_state == CS_QUAD3D_EMPTY && info.test_only)
    return CS_QUAD3D_CERTAINCHANGE;

  if(node_state == CS_QUAD3D_EMPTY && node_pos->depth < pObj->max_depth)
  {
    // mark children as empty now, since they should be empty, and
    // can be reached now...
    pObj->CallChildren(&csQuadTree3D::mark_node_func, pObj,
      node_pos, (void*)CS_QUAD3D_EMPTY);
  }
  // this node is partially covered.
  if(!info.test_only)
    pObj->SetNodeState(node_pos, CS_QUAD3D_PARTIAL);
  // if any children they can process the polygon too. And they determine
  // the return value.
  if(node_pos->depth < pObj->max_depth)
  {
    int retval[4];
    pObj->CallChildren(&csQuadTree3D::insert_polygon_func, pObj,
      node_pos, data, retval);
    /// @@@ the polygon could have caused a child to become FULL and thus
    /// all children could be FULL now, in which case we should be FULL too.
    return pObj->GetPolygonResult(retval);
  }
  /// no children, return value for change, either
  /// EMPTY->PARTIAL (certain change) or
  /// PARTIAL->PARTIAL (possible change)
  if(old_node_state == CS_QUAD3D_EMPTY)
    return CS_QUAD3D_CERTAINCHANGE;
  return CS_QUAD3D_POSSIBLECHANGE;
}


int csQuadTree3D::GetPolygonResult(int retval[4]) const
{
  // returns the max change.
  // since NOCHANGE(0) < POSSIBLECHANGE(1) < CERTAINCHANGE(2)
  int res = CS_QUAD3D_NOCHANGE;
  for(int i=0; i<4; i++)
    if(retval[i] > res)
      res = retval[i];
  return res;
}


int csQuadTree3D::InsertPolygon (csVector3* verts, int num_verts,
  const csBox3& pol_bbox)
{
  struct poly_info info;
  info.verts = verts;
  info.num_verts = num_verts;
  info.pol_bbox = &pol_bbox;
  info.test_only = false;
  q3d_node_pos_info start_pos;
  start_pos.set_root(this);
  return insert_polygon_func(this, root_state, &start_pos, (void*)&info);
}


int csQuadTree3D::TestPolygon (csVector3* verts, int num_verts,
  const csBox3& pol_bbox)
{
  struct poly_info info;
  info.verts = verts;
  info.num_verts = num_verts;
  info.pol_bbox = &pol_bbox;
  info.test_only = true;
  q3d_node_pos_info start_pos;
  start_pos.set_root(this);
  return insert_polygon_func(this, root_state, &start_pos, (void*)&info);
}


int csQuadTree3D::GetTestPointResult(int retval[4]) const
{
  /// returns UNKNOWN if all retval are UNKNOWN.
  /// returns PARTIAL if at least one is PARTIAL, or
  ///   both FULL and EMPTY are present.
  /// returns FULL when all non-unknown values are FULL
  /// returns EMPTY when all non-unknown values are EMPTY
  int res = CS_QUAD3D_UNKNOWN;
  for(int i=0; i<4; i++)
    if(retval[i] != CS_QUAD3D_UNKNOWN)
    {
      if(res == CS_QUAD3D_UNKNOWN)
        res = retval[i];
      else if(res == CS_QUAD3D_PARTIAL || retval[i] == CS_QUAD3D_PARTIAL)
        res = CS_QUAD3D_PARTIAL;
      else // res must be EMPTY or FULL now. so must retval[i].
        if(res != retval[i])
          res = CS_QUAD3D_PARTIAL;
    }
  return res;
}


int csQuadTree3D::test_point_func (csQuadTree3D* pObj,
  int node_state, q3d_node_pos_info *node_pos, void* data)
{
  if(node_state == CS_QUAD3D_UNKNOWN)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree3D: testpoint quad_unknown.\n");
    return CS_QUAD3D_UNKNOWN;
  }
  if(!node_pos->frust->Contains(*(csVector3*)data))
    return CS_QUAD3D_UNKNOWN;
  if(node_state != CS_QUAD3D_PARTIAL || node_pos->depth == pObj->max_depth)
    return node_state;
  // for a partial covered node with children, call the children
  int retval[4];
  CallChildren(&csQuadTree3D::test_point_func, pObj, node_pos, data, retval);
  return pObj->GetTestPointResult(retval);
}


int csQuadTree3D::TestPoint (const csVector3& point)
{
  q3d_node_pos_info start_pos;
  start_pos.set_root(this);
  return test_point_func(this, root_state, &start_pos, (void*)&point);
}


void csQuadTree3D::Print()
{
  printf("csQuadTree3D depth %d, statesize %d, root_state %d\n",
    max_depth, state_size, root_state);
  if(!states)
    return;
  for(int i=0; i<state_size; i++)
    printf("%2.2x ", states[i]);
  printf("\n");
  // characters to represent the node states.
  char *display = "./?X"; // EMPTY, PARTIAL, UNKOWN, FULL
  for(int dep=1; dep<=max_depth; dep++)
  {
    int dep_prevnodes = ( Pow2( 2*(dep-1) ) - 1 ) / 3;
    // dep_prevnodes is the number of nodes in dep-1 tree.
    int plane_start = dep_prevnodes / 4; // offset of start of plane in array
    int plane_size = Pow2( (dep-1) ); // is sqrt(nr_leaves)
    int nodenr = 0; // nr of nodes after plane_start
    printf("Depth %d (size %d, start %d)\n", dep, plane_size, plane_start);
    for(int y=0;y<plane_size;y++)
    {
      for(int x=0;x<plane_size;x++,nodenr++)
      {
        int offset;
        if(dep==1) offset = -1;
        else offset = plane_start + (nodenr >> 2);
	int bytepos = nodenr & 0x3;
	int state = GetNodeState(offset, bytepos);
	printf("%c", display[state]);
      }
      printf("\n");
    }
    // did depth dep.
  }
}


int csQuadTree3D::propagate_func (csQuadTree3D* pObj,
  int node_state, q3d_node_pos_info *node_pos, void* data)
{
  int newstate = (int)data;
  if(newstate != CS_QUAD3D_UNKNOWN)
    pObj->SetNodeState(node_pos, newstate);
  else if(node_state == CS_QUAD3D_FULL || node_state == CS_QUAD3D_EMPTY)
    newstate = node_state;
  if(node_pos->depth < pObj->max_depth)
    CallChildren(&csQuadTree3D::propagate_func, pObj, node_pos,
    (void*)newstate);
  return 0;
}


void csQuadTree3D::Propagate()
{
  q3d_node_pos_info start_pos;
  start_pos.set_root(this);
  propagate_func(this, root_state, &start_pos, (void*)CS_QUAD3D_UNKNOWN);
}


int csQuadTree3D::sift_func (csQuadTree3D* pObj,
  int node_state, q3d_node_pos_info *node_pos, void* data)
{
  if(node_pos->depth >= pObj->max_depth) // this is a leaf
    return node_state;
  // ask my children what value should me
  int retval[4];
  CallChildren(&csQuadTree3D::sift_func, pObj, node_pos, data, retval);
  int newstate = pObj->GetTestPointResult(retval);
  if(node_state != newstate)
    pObj->SetNodeState(node_pos, newstate);
  return newstate;
}


void csQuadTree3D::Sift()
{
  q3d_node_pos_info start_pos;
  start_pos.set_root(this);
  sift_func(this, root_state, &start_pos, 0);
}


static bool CheckRow(int plane_start, int start_nr, int w,
  unsigned char *states)
// checks a sequence of nodes if all are FULL
// plane start is start of plane,
// start_nr is the node nr of the start node
// w is width to check.
{
  /// it is sure this is not the root node.
  int offset = plane_start + (start_nr >> 2);
  int bytepos = start_nr & 0x3;
  /// checking 4 nodes at a time (1 byte) is very fast.
  if(bytepos != 0)
  {
    /// so first check the leading nodes. since bytepos!=0,
    /// startnr was not a multiple of 4.
    /// if bytepos == 0, we were not called,
    /// if bytepos>0, check using lead_check[bytepos]
    //// These values depend of CS_QUAD3D_FULL==3
    const int lead_check[4] = {0xFF, 0x3F, 0x0F, 0x03};
    if( (states[offset] & lead_check[bytepos]) != lead_check[bytepos] )
      return false;
    w -= 4-bytepos; // checked this many nodes.
    // update values to get correct start position.
    offset ++;
    bytepos = 0;
  }
  // got a correct start position now, can check bytes for w/4 times.
  int runlen = w/4;
  for(int i=0; i<runlen; i++, offset++)
    if(states[offset] != CS_QUAD3D_ALL_FULL) // 0xFF
      return false;
  // now to check the trailing nodes to be sure.
  // got w%4 trailing nodes.
  // offset already points to the right byte.
  // in trail_check is the value to check for, the last value is never used.
  //// These values depend of CS_QUAD3D_FULL==3
  const int trail_check[5] = {0x00, 0xC0, 0xF0, 0xFC, 0xFF};
  int trail_amt = w%4;
  if(trail_amt != 0) // have to check, prevents array overflow
    if( (states[offset] & trail_check[trail_amt]) != trail_check[trail_amt] )
      return false;
  // passed all tests
  return true;
}

bool csQuadTree3D::TestRectangle(int depth, int x, int y, int w, int h)
{
  if(depth==1) // if this is root node
    return (root_state == CS_QUAD3D_FULL);
  int dep_prevnodes = ( Pow2( 2*(depth-1) ) - 1 ) / 3;
  // dep_prevnodes is the number of nodes in dep-1 tree.
  int plane_start = dep_prevnodes / 4; // offset of start of plane in array
  int plane_size = Pow2( (depth-1) ); // is sqrt(nr_leaves)
  int nodenr = plane_size * y + x; // nr of nodes after plane_start
  for(int ty=0;ty<h;ty++,nodenr+=plane_size)
    if(!CheckRow(plane_start, nodenr, w, states))
      return false;
  return true;
}
