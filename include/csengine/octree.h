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

#ifndef OCTREE_H
#define OCTREE_H

#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "csengine/polytree.h"
#include "csengine/bsp.h"
#include "csengine/arrays.h"

class csPolygonInt;
class csOctree;
class csOctreeNode;
class csBspTree;
class csCBuffer;
class csThing;
class csPVS;
class Dumper;
struct iVFS;
struct iFile;

#define OCTREE_FFF 0
#define OCTREE_FFB 1
#define OCTREE_FBF 2
#define OCTREE_FBB 3
#define OCTREE_BFF 4
#define OCTREE_BFB 5
#define OCTREE_BBF 6
#define OCTREE_BBB 7

/**
 * A visibility info node for one octree node.
 * This node represents a visible octree node and possibly
 * all visible polygons (if the node is a leaf).
 */
class csOctreeVisible
{
  friend class csPVS;

private:
  // Next visibility entry.
  csOctreeVisible* next;
  // The visible polygons.
  csPolygonArrayNoFree polygons;
  // The visible node.
  csOctreeNode* node;
  
public:
  csOctreeVisible() : next (NULL), polygons (8, 16), node (NULL) {}
  /// Set octree node.
  void SetOctreeNode (csOctreeNode* onode) { node = onode; }
  /// Get octree node.
  csOctreeNode* GetOctreeNode () { return node; }
  /// Get the polygon array.
  csPolygonArrayNoFree& GetPolygons () { return polygons; }
};

/**
 * The PVS itself.
 */
class csPVS
{
private:
  // Linked list of visible nodes (with polygons attached).
  csOctreeVisible* visible;

public:
  /// Constructor.
  csPVS () : visible (NULL) { }
  /// Destructor.
  ~csPVS ();
  /// Clear the PVS.
  void Clear ();

  /// Add a new visibility struct.
  csOctreeVisible* Add ();

  /// Get the first visibility struct.
  csOctreeVisible* GetFirst () { return visible; }
  /// Get the next one.
  csOctreeVisible* GetNext (csOctreeVisible* ovis) { return ovis->next; }
};

/**
 * An octree node.
 * @@@ We should have seperate leaf/non-leaf structures as they
 * are considerably different.
 */
class csOctreeNode : public csPolygonTreeNode
{
  friend class csOctree;
  friend class Dumper;

private:
  /// Children.
  csPolygonTreeNode* children[8];
  /// Bounding box;
  csBox3 bbox;
  /// Center point for this node.
  csVector3 center;

  /**
   * If true then this is a leaf.
   * If a node has no polygons then it will also be a leaf
   * but there will be no mini-bsp.
   */
  bool leaf;

  /**
   * A list of all polygons in this node. These are the original
   * unsplit polygons. Further subdivision of this node will
   * cause these polygons to be split into the children but
   * the list here reflects the unsplit polygons.
   */
  csPolygonIntArray unsplit_polygons;

  /// Mini-bsp tree (in this case there are no children).
  csBspTree* minibsp;

  /**
   * If there is a mini-bsp tree this array contains the indices
   * of all vertices that are used by the polygons in the tree.
   * This can be used to optimize the world->camera transformation
   * process because only the minibsp nodes that are really used
   * need to be traversed.
   */
  int* minibsp_verts;

  /// Number of vertices in minibsp_verts.
  int minibsp_numverts;

  /**
   * The PVS for this node.
   * This list contains all octree nodes and polygons that
   * are visible from this node.
   */
  csPVS pvs;

  /**
   * Visibility number. If equal to csOctreeNode::pvs_cur_vis_nr then
   * this object is visible.
   */
  ULong pvs_vis_nr;

public:
  /**
   * Current visibility number. All objects (i.e. octree
   * nodes and polygons) which have the same number as this one
   * are visible. All others are not.
   */
  static ULong pvs_cur_vis_nr;

private:
  /// Make an empty octree node.
  csOctreeNode ();

  /**
   * Destroy this octree node.
   */
  virtual ~csOctreeNode ();

  /// Set box.
  void SetBox (const csVector3& bmin, const csVector3& bmax)
  {
    bbox.Set (bmin, bmax);
    center = (bmin + bmax) / 2;
  }

  /// Set mini-bsp tree.
  void SetMiniBsp (csBspTree* mbsp);

  /// Build vertex tables.
  void BuildVertexTables ();

public:
  /// Return true if node is empty.
  bool IsEmpty ();

  /// Return true if node is visible according to PVS.
  bool IsVisible () { return pvs_vis_nr == pvs_cur_vis_nr; }

  /// Mark visible (used by PVS).
  void MarkVisible () { pvs_vis_nr = pvs_cur_vis_nr; }

  /// Return true if node is leaf.
  bool IsLeaf () { return leaf; }

  /// Get center.
  const csVector3& GetCenter () const { return center; }

  /// Get minimum coordinate of box.
  const csVector3& GetMinCorner () const { return bbox.Min (); }

  /// Get maximum coordinate of box.
  const csVector3& GetMaxCorner () const { return bbox.Max (); }

  /// Get box.
  const csBox3& GetBox () { return bbox; }

  /// Get a child.
  csOctreeNode* GetChild (int i) { return (csOctreeNode*)children[i]; }

  /**
   * Get the list of all unsplit polygons in this node.
   * These are the original unsplit polygons. Further
   * subdivision of this node will have caused these polygons to
   * be split into the children but the list here reflects
   * the unsplit polygons.
   */
  csPolygonIntArray& GetUnsplitPolygons () { return unsplit_polygons; }

  /// Get mini-bsp tree.
  csBspTree* GetMiniBsp () const { return minibsp; }

  /// Get indices of vertices used in the mini-bsp of this leaf.
  int* GetMiniBspVerts () const { return minibsp_verts; }

  /// Get number of vertices.
  int GetMiniBspNumVerts () const { return minibsp_numverts; }

  /// Return type (NODE_???).
  int Type () { return NODE_OCTREE; }

  /// Get the PVS.
  csPVS& GetPVS () { return pvs; }
};

/**
 * The octree.
 */
class csOctree : public csPolygonTree
{
  friend class Dumper;

private:
  /// The main bounding box for the octree.
  csBox3 bbox;
  /// The number of polygons at which we revert to a bsp tree.
  int bsp_num;
  /// The mode for the mini-bsp trees.
  int mode;

private:
  /// Build the tree from the given node and number of polygons.
  void Build (csOctreeNode* node, const csVector3& bmin, const csVector3& bmax,
  	csPolygonInt** polygons, int num);

  /// Traverse the tree from back to front starting at 'node' and 'pos'.
  void* Back2Front (csOctreeNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata);
  /// Traverse the tree from front to back starting at 'node' and 'pos'.
  void* Front2Back (csOctreeNode* node, const csVector3& pos,
  	csTreeVisitFunc* func, void* data, csTreeCullFunc* cullfunc,
	void* culldata);

  /**
   * Process all todo stubs in a node and add new
   * todo stubs to the children of this node.
   */
  void ProcessTodo (csOctreeNode* node);

  /**
   * Try to find the best center possible and update the node.
   */
  void ChooseBestCenter (csOctreeNode* node, csPolygonInt** polygons, int num);

  /**
   * Gather statistics info about this tree.
   */
  void Statistics (csOctreeNode* node, int depth,
  	int* num_oct_nodes, int* max_oct_depth, int* num_bsp_trees,
  	int* tot_bsp_nodes, int* min_bsp_nodes, int* max_bsp_nodes,
	int* tot_bsp_leaves, int* min_bsp_leaves, int* max_bsp_leaves,
	int* tot_max_depth, int* min_max_depth, int* max_max_depth,
	int* tot_tot_poly, int* min_tot_poly, int* max_tot_poly);

  /**
   * Help function for BoxCanSeeOccludee.
   */
  void BoxOccludeeShadowPolygons (const csBox3& box,
  	const csBox3& occludee,
	csPolygonInt** polygons, int num_polygons,
	csCBuffer* cbuffer,
  	const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos);

  /**
   * Help function for BoxCanSeeOccludee.
   */
  void BoxOccludeeAddShadows (csOctreeNode* occluder, csCBuffer* cbuffer,
  	const csVector2& scale, const csVector2& shift,
	int plane_nr, float plane_pos,
  	const csBox3& box, const csBox3& occludee,
	csVector3& box_center, csVector3& occludee_center);

  /**
   * Test if 'box' can see 'occludee' through all the polygons
   * in this octree.
   */
  bool BoxCanSeeOccludee (const csBox3& box, const csBox3& occludee);

  /**
   * Build PVS for this leaf.
   */
  void BuildPVSForLeaf (csOctreeNode* occludee, csThing* thing,
  	csOctreeNode* leaf);

  /**
   * Build PVS for this node.
   */
  void BuildPVS (csThing* thing, csOctreeNode* node);

  /// Cache this node and children.
  void Cache (csOctreeNode* node, iFile* cf);

  /// Read this node from cache and also children.
  bool ReadFromCache (iFile* cf, csOctreeNode* node,
  	const csVector3& bmin, const csVector3& bmax,
  	csPolygonInt** polygons, int num);

  /// Read the PVS for this node and children from VFS.
  bool ReadFromCachePVS (iFile* cf, csOctreeNode* node);
  /// Cache the PVS for this node and children to VFS.
  void CachePVS (csOctreeNode* node, iFile* cf);

  /**
   * Get the path to a node in the tree.
   * The length should be set to 0 before calling this function.
   * The returned length will be the length of the path in 'path'.
   */
  void GetNodePath (csOctreeNode* node, csOctreeNode* child,
	unsigned char* path, int& path_len);

public:
  /**
   * Create an empty tree for the given parent, a bounding box defining the
   * outer limits of the octree, and the number of polygons at which we
   * revert to a BSP tree.
   */
  csOctree (csSector* sect, const csVector3& min_bbox,
  	const csVector3& max_bbox, int bsp_num, int mode = BSP_MINIMIZE_SPLITS);

  /**
   * Destroy the whole octree (but not the actual polygons and parent
   * objects).
   */
  virtual ~csOctree ();

  /**
   * Get the root.
   */
  csOctreeNode* GetRoot () { return (csOctreeNode*)root; }

  /**
   * Create the tree for the default parent set.
   */
  void Build ();

  /**
   * Create the tree with a given set of polygons.
   */
  void Build (csPolygonInt** polygons, int num);

  /**
   * Create the tree with a given set of polygons.
   */
  void Build (const csPolygonArray& polygons);

  /// Traverse the tree from back to front starting at the root and 'pos'.
  void* Back2Front (const csVector3& pos, csTreeVisitFunc* func, void* data,
  	csTreeCullFunc* cullfunc = NULL, void* culldata = NULL);
  /// Traverse the tree from front to back starting at the root and 'pos'.
  void* Front2Back (const csVector3& pos, csTreeVisitFunc* func, void* data,
  	csTreeCullFunc* cullfunc = NULL, void* culldata = NULL);

  /**
   * Get a convex outline (not a polygon unless projected to 2D)
   * for for this octree node as seen from the given position.
   * The coordinates returned are world space coordinates.
   * Note that you need place for at least six vectors in the array.
   */
  void GetConvexOutline (csOctreeNode* node, const csVector3& pos,
  	csVector3* array, int& num_array)
  {
    node->bbox.GetConvexOutline (pos, array, num_array);
  }

  /**
   * Update with PVS. This routine will take the given position
   * and locate the octree leaf node for that position. Then it
   * will get the PVS for that node and mark all visible octree
   * nodes and polygons.
   */
  void MarkVisibleFromPVS (const csVector3& pos);

  /**
   * Build the PVS for this octree and the csThing
   * to which this octree belongs.
   */
  void BuildPVS (csThing* thing);

  /**
   * Build vertex tables for minibsp leaves. These tables are
   * used to optimize the world to camera transformation so that only
   * the needed vertices are transformed.
   */
  void BuildVertexTables () { if (root) ((csOctreeNode*)root)->BuildVertexTables (); }

  /// Print statistics about this octree.
  void Statistics ();

  /**
   * Cache this entire octree to disk (VFS).
   */
  void Cache (iVFS* vfs, const char* name);

  /**
   * Read this entire octree from disk (VFS).
   * Returns false if not cached, or cache not valid.
   */
  bool ReadFromCache (iVFS* vfs, const char* name,
  	csPolygonInt** polygons, int num);

  /**
   * Cache the PVS to VFS.
   */
  void CachePVS (iVFS* vfs, const char* name);

  /**
   * Read the PVS from VFS.
   * Return false if not cached, or cache not valid.
   */
  bool ReadFromCachePVS (iVFS* vfs, const char* name);
};

#endif /*OCTREE_H*/

