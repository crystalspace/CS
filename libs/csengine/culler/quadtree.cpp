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
#include "csgeom/frustum.h"
#include "csgeom/poly2d.h"
#include "csengine/quadtree.h"
#include "csengine/engine.h"
#include "isys/system.h"


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


csQuadTree :: node_pos_info :: node_pos_info(void)
{
  set_root();
}

void csQuadTree :: node_pos_info :: set_root(void)
{
  // init to point to root state.
  offset = -1;
  bytepos = 0;
  depth = 1;
  plane_size = 1;
  plane_start = 0;
  node_x = 0;
  node_y = 0;
}

csQuadTree :: node_pos_info :: node_pos_info(const node_pos_info &copy)
{
  offset = copy.offset;
  bytepos = copy.bytepos;
  depth = copy.depth;
  plane_size = copy.plane_size;
  plane_start = copy.plane_start;
  node_x = copy.node_x;
  node_y = copy.node_y;
}


csQuadTree :: csQuadTree (const csBox2& the_box, int the_depth)
{
  bbox = the_box;
  max_depth= the_depth;
  root_state = CS_QUAD_EMPTY;
  if(max_depth < 1)
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
  int nr_nodes = ( Pow2( 2*max_depth ) - 1 ) / 3;

  /// 4 nodes per byte, the root node is stored seperately
  state_size = (nr_nodes - 1) / 4;
  printf("QuadTree: depth %d, nodes %d, leaves %d, statesize %d"
    " bytes\n", max_depth, nr_nodes, nr_leaves, state_size);

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


void csQuadTree :: CallChildren(quad_traverse_func func, csQuadTree* pObj, 
  const csBox2& box, node_pos_info *parent_pos, void *data)
{
  int retval[4];
  pObj->CallChildren(func, pObj, box, parent_pos, data, retval);
}

  /*
   * OLD:
   * states are ordered like this:
   * root has children in byte 0.
   * nodes in byte 0 have children in byte 0+node_nr+1(1,2,3,4).
   * nodes in byte 1 have children in byte 4+node_nr+1.
   * nodes in byte n have children in byte 4*n+node_nr+1
   * So for byte n, take 4*n + node_nr+1 as the new byte
   * that new byte has the states of it's four children.
   * BUT: finding node at x,y is too hard.
   *
   * NEW:
   * states orderd like this:
   * node at depth 1, then all nodes at depth 2, ...
   * nodes at a depth are stored in y*width + x ordering.
   * width at depth d is: 2 ** (d-1)
   * nodes at depth d start after: (4 ** (d-1) -1) /3 nodes in the array.
   * 
   * To find children of a node, do the following.
   * a position is offset*4 + node_nr.
   * parent node, position pos, coords px, py, depth pd.
   * pos = 2**(pd-1) * py + px + (4**(pd-1) -1 ) / 3
   *
   * The topleft child is at 4*pos - 2*px + 1.
   * is has coords 2*px, 2*py, depth pd+1, width 2**(pd+1-1)
   * childpos = 2**(pd+1-1) * 2*py + 2*px + (4**(pd+1-1) -1)/3
   *
   * but since:
   * 4 * pos = 4* 2**(pd-1)*py + 4*px + 4* (4**(pd-1) -1)/3
   *   = 2**(pd+1-1) * 2*py + 2* 2*px + (4**(pd+1-1))/3 - 4/3
   *   = 2**(pd+1-1) * 2*py + 2* 2*px + (4**(pd+1-1))/3 - 1/3 - 1
   *   = 2**(pd+1-1) * 2*py + 2* 2*px + (4**(pd+1-1) - 1)/3 - 1
   *   = childpos + 2*px - 1
   * Thus childpos = 4*pos - 2*px + 1
   * 
   * This is not yet used. Because I am not sure if the above will
   * work, first the simple version.
  */
void csQuadTree :: CallChildren(quad_traverse_func func, csQuadTree* pObj,  
  const csBox2& box, node_pos_info *parent_pos, void *data, int retval[4])
{
  if(parent_pos->depth >= pObj->max_depth)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: leaf trying to recurse.\n");
    return;
  }
  // call func for each child of parent node with (offset, node_nr)
  csBox2 childbox;
  csVector2 center = box.GetCenter();
  // make a copy of parent position, and adjust values.
  // during recursion only depth times will there be a node_pos_info on stack.
  // say 20 deep, times 28 bytes -> only 560 bytes.
  node_pos_info child_pos(*parent_pos);
  child_pos.depth = parent_pos->depth+1;
  child_pos.plane_size = parent_pos->plane_size << 1;
  child_pos.plane_start = parent_pos->plane_start + 
    (parent_pos->plane_size * parent_pos->plane_size) / 4;
  int child_x = parent_pos->node_x << 1;
  int child_y = parent_pos->node_y << 1;

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
      switch(childnr)
      {
        case 0 /*topleft*/ : 
          childbox.Set(box.Min(), center); break;
        case 1 /*topright*/ : 
          childbox.Set(center.x, box.MinY(), box.MaxX(), center.y); break;
        case 2 /*bottomleft*/ : 
          childbox.Set(box.MinX(), center.y, center.x, box.MaxY()); break;
        case 3 /*bottomright*/ : 
          childbox.Set(center, box.Max()); break;
        default: CsPrintf(MSG_FATAL_ERROR, "QuadTree: Unknown child\n");
      }
      int childstate = pObj->GetNodeState(&child_pos);
      retval[childnr] = (this->*func)(pObj, childbox, childstate, 
        &child_pos, data);

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

int csQuadTree :: GetNodeState(int offset, int bytepos)
{
  if(offset > state_size)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: state out of range\n");
    return 0;
  }
  if(offset == -1)
    return root_state;
  unsigned char bits = states[offset];
  bits &= node_masks[bytepos];
  bits >>= node_shifts[bytepos];
  return bits;
}


void csQuadTree :: SetNodeState(int offset, int bytepos, int newstate)
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
  unsigned char bits = states[offset];      // get all bits
  bits &= ~node_masks[bytepos];             // purge this node's bits
  bits |= newstate << node_shifts[bytepos]; // insert new bits
  states[offset] = bits;                    // store bits
}


void csQuadTree :: MakeEmpty()
{
  root_state = CS_QUAD_EMPTY;
  if(states)
    memset(states, CS_QUAD_ALL_EMPTY, state_size);  
}


int csQuadTree :: mark_node_func (csQuadTree* pObj, const csBox2& node_bbox,
  int node_state, node_pos_info *node_pos, void* data)
{
  (void)node_bbox;
  (void)node_state;
  pObj->SetNodeState(node_pos, (int)data);
  return (int)data;
}


int csQuadTree :: insert_polygon_func (csQuadTree* pObj, 
  const csBox2& node_bbox, int node_state, node_pos_info *node_pos, void* data)
{
  struct poly_info& info = *(struct poly_info*)data;
  if(node_state == CS_QUAD_UNKNOWN)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: insertpoly quad_unknown.\n");
    return CS_QUAD_NOCHANGE;
  }
  if(node_state == CS_QUAD_FULL) // full already, skip it.
    return CS_QUAD_NOCHANGE;

  /// perhaps none of the node is covered?
  if (!node_bbox.Overlap (*info.pol_bbox)) 
    return CS_QUAD_NOCHANGE;

  ///----------------------------------------------------
  /// So, the polygon bbox overlaps this node. How much?
  ///----------------------------------------------------

  if(0)
  printf("Depth %d Checking node %d+%d %d,%d (%d)\n", node_pos->depth, 
    node_pos->offset, node_pos->bytepos, node_pos->node_x, node_pos->node_y,
    node_state);

  /// is the whole node covered?
  /// first check bounding boxes then precisely.
  if(
    info.pol_bbox->Contains(node_bbox) &&
    BoxEntirelyInPolygon(info.verts, info.num_verts, node_bbox))
  {
    if(!info.test_only)
    {
      pObj->SetNodeState(node_pos, CS_QUAD_FULL);
      /// mark children (if any) as unknown, since they should not be reached.
      if(node_pos->depth < pObj->max_depth)
        pObj->CallChildren(&csQuadTree::mark_node_func, pObj, node_bbox, 
          node_pos, (void*)CS_QUAD_UNKNOWN);
    }
    return CS_QUAD_CERTAINCHANGE;
  }
  
  /// So a part of the node may be covered, perhaps none.
  /// could test some more points here.
  /// note that the test if the node is inside the poly must be before
  /// the box::intersect test.
  if( csPoly2D::In(info.verts, info.num_verts, node_bbox.GetCenter())
    || node_bbox.Intersect(info.verts, info.num_verts))
  { 
    int old_node_state = node_state;
    // so it overlaps a bit.

    // if only testing, and empty we already know a certain change now.
    if(node_state == CS_QUAD_EMPTY && info.test_only)
      return CS_QUAD_CERTAINCHANGE;

    if(node_state == CS_QUAD_EMPTY && node_pos->depth < pObj->max_depth)
    {
      // mark children as empty now, since they should be empty, and
      // can be reached now...
      pObj->CallChildren(&csQuadTree::mark_node_func, pObj, node_bbox, 
        node_pos, (void*)CS_QUAD_EMPTY);
    }
    // this node is partially covered.
    if(!info.test_only)
      pObj->SetNodeState(node_pos, CS_QUAD_PARTIAL);
    // if any children they can process the polygon too. And they determine
    // the return value.
    if(node_pos->depth < pObj->max_depth)
    {
      int retval[4];
      pObj->CallChildren(&csQuadTree::insert_polygon_func, pObj, node_bbox, 
        node_pos, data, retval);
      /// @@@ the polygon could have caused a child to become FULL and thus
      /// all children could be FULL now, in which case we should be FULL too.
      return pObj->GetPolygonResult(retval);
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
  info.pol_bbox = &pol_bbox;
  info.test_only = false;
  node_pos_info start_pos;
  start_pos.set_root();
  return insert_polygon_func(this, bbox, root_state, &start_pos, (void*)&info);
}


int csQuadTree :: TestPolygon (csVector2* verts, int num_verts,
  const csBox2& pol_bbox)
{
  struct poly_info info;
  info.verts = verts;
  info.num_verts = num_verts;
  info.pol_bbox = &pol_bbox;
  info.test_only = true;
  node_pos_info start_pos;
  start_pos.set_root();
  return insert_polygon_func(this, bbox, root_state, &start_pos, (void*)&info);
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


int csQuadTree :: test_point_func (csQuadTree* pObj, const csBox2& node_bbox,
  int node_state, node_pos_info *node_pos, void* data)
{
  if(node_state == CS_QUAD_UNKNOWN)
  {
    CsPrintf(MSG_INTERNAL_ERROR, "QuadTree: testpoint quad_unknown.\n");
    return CS_QUAD_UNKNOWN;
  }
  if(!node_bbox.In(*(csVector2*)data))
    return CS_QUAD_UNKNOWN;
  if(node_state != CS_QUAD_PARTIAL || node_pos->depth == pObj->max_depth)
    return node_state;
  // for a partial covered node with children, call the children
  int retval[4];
  CallChildren(&csQuadTree::test_point_func, pObj, node_bbox, node_pos,
    data, retval);
  return pObj->GetTestPointResult(retval);
}


int csQuadTree :: TestPoint (const csVector2& point)
{
  node_pos_info start_pos;
  start_pos.set_root();
  return test_point_func(this, bbox, root_state, &start_pos, (void*)&point);
}


void csQuadTree :: Print(void)
{
  printf("csQuadTree depth %d, statesize %d, root_state %d\n", 
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


int csQuadTree :: propagate_func (csQuadTree* pObj, 
  const csBox2& node_bbox, int node_state, node_pos_info *node_pos, void* data)
{
  int newstate = (int)data;
  if(newstate != CS_QUAD_UNKNOWN)
    pObj->SetNodeState(node_pos, newstate);
  else if(node_state == CS_QUAD_FULL || node_state == CS_QUAD_EMPTY)
    newstate = node_state;
  if(node_pos->depth < pObj->max_depth)
    CallChildren(&csQuadTree::propagate_func, pObj, node_bbox, node_pos,
    (void*)newstate);
  return 0;
}


void csQuadTree :: Propagate(void)
{
  node_pos_info start_pos;
  start_pos.set_root();
  propagate_func(this, bbox, root_state, &start_pos, 
    (void*)CS_QUAD_UNKNOWN);
}


int csQuadTree :: sift_func (csQuadTree* pObj, const csBox2& node_bbox, 
  int node_state, node_pos_info *node_pos, void* data)
{
  if(node_pos->depth >= pObj->max_depth) // this is a leaf
    return node_state;
  // ask my children what value should me
  int retval[4];
  CallChildren(&csQuadTree::sift_func, pObj, node_bbox, node_pos,
    data, retval);
  int newstate = pObj->GetTestPointResult(retval);
  if(node_state != newstate)
    pObj->SetNodeState(node_pos, newstate);
  return newstate;
}


void csQuadTree :: Sift(void)
{
  node_pos_info start_pos;
  start_pos.set_root();
  sift_func(this, bbox, root_state, &start_pos, 0);
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
    //// These values depend of CS_QUAD_FULL==3
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
    if(states[offset] != CS_QUAD_ALL_FULL) // 0xFF
      return false;
  // now to check the trailing nodes to be sure.
  // got w%4 trailing nodes.
  // offset already points to the right byte.
  // in trail_check is the value to check for, the last value is never used.
  //// These values depend of CS_QUAD_FULL==3
  const int trail_check[5] = {0x00, 0xC0, 0xF0, 0xFC, 0xFF};
  int trail_amt = w%4;
  if(trail_amt != 0) // have to check, prevents array overflow
    if( (states[offset] & trail_check[trail_amt]) != trail_check[trail_amt] )
      return false;
  // passed all tests
  return true;
}

bool csQuadTree :: TestRectangle(int depth, int x, int y, int w, int h)
{
  if(depth==1) // if this is root node
    return (root_state == CS_QUAD_FULL);
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


/*
 * 2 bytes form a 4x4 rectangle.
 * Bit coordinate x,y corresponds with bitnr y*4+x
 * And then 1<<bitnr is used.
 */
