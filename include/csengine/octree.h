/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein
  
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

#ifndef __CS_OCTREE_H__
#define __CS_OCTREE_H__

#include "csgeom/math3d.h"
#include "csgeom/box.h"
#include "csengine/polytree.h"
#include "csengine/bsp.h"
#include "csengine/pvs.h"

class csPolygonInt;
class csOctree;
class csOctreeNode;
class csBspTree;
class csThing;
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
   * Six masks representing solid space on the boundaries
   * of this node. Use the BOX_SIDE_xxx flags to fetch them.
   */
  UShort solid_masks[6];

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
   * Get one of the masks representing solid space on the boundaries
   * of this node. Use the BOX_SIDE_xxx flags to fetch them.
   */
  UShort GetSolidMask (int idx) { return solid_masks[idx]; }

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

  /// Count the number of children (octree nodes) for this node.
  int CountChildren ();

  /**
   * Count all the polygons in this node and children.
   * This function only calls leaf polygons (i.e. polygons that will
   * actually be returned by Front2Back/Back2Front).
   */
  int CountPolygons ();

  /// Get the PVS.
  csPVS& GetPVS () { return pvs; }

  /**
   * Test if this node can see the leaf containing the given position
   * with the PVS. This function will return true (i.e. visible) if the PVS 
   * for this node has not been computed yet.
   */
  bool PVSCanSee (const csVector3& v);

  /**
   * Create an iterator to iterate over all solid polygons that
   * are on the specified side of this octree node ('side' is one
   * of BOX_SIDE_xxx flags).
   */
  void* InitSolidPolygonIterator (int side);

  /**
   * Get the next solid polygon from the iterator. Returns false
   * if there are no more polygons.
   */
  bool NextSolidPolygon (void* vspit, csPoly3D& poly);

  /**
   * If done with the iterator clean it up.
   */
  void CleanupSolidPolygonIterator (void* vspit);
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
	int* tot_tot_poly, int* min_tot_poly, int* max_tot_poly,
	int* num_pvs_leaves,
	int* tot_pvs_vis_nodes, int* min_pvs_vis_nodes, int* max_pvs_vis_nodes,
	int* tot_pvs_vis_poly, int* min_pvs_vis_poly, int* max_pvs_vis_poly);

  /**
   * Test if 'box' can see 'occludee' through all the polygons
   * in this octree.
   */
  bool BoxCanSeeOccludee (const csBox3& box, const csBox3& occludee);

  /**
   * Test if 'box' can see 'occludee' through all the polygons
   * in this octree. This function is VERY VERY slow and when the
   * two nodes cannot see each other it will actually never return.
   * This function is only useful for debugging.
   */
  bool BoxCanSeeOccludeeSuperSlow (const csBox3& box, const csBox3& occludee);

  /**
   * Build PVS for this leaf.
   * Returns true if the occludee was visible, false otherwise.
   */
  bool BuildPVSForLeaf (csOctreeNode* occludee, csThing* thing,
  	csOctreeNode* leaf);

  /**
   * Build PVS for this node.
   * Call SetupDummyPVS() and optionally BuildQADPVS()
   * before calling BuildPVS().
   */
  void BuildPVS (csThing* thing, csOctreeNode* node);

  /**
   * Build quick PVS for this node.
   * The quick PVS will not correctly find all visible nodes but all
   * nodes that it marks as visible are SURE to be correct. Note that
   * SetupDummyPVS() must be called before this. The real BuildPVS()
   * will use this PVS to optimize PVS building (i.e. don't test PVS
   * for nodes that are sure to be mutually visible).
   */
  void BuildQADPVS (csOctreeNode* node);

  /**
   * Delete the given occludee and all children of that occludee
   * from the given pvs.
   */
  void DeleteNodeAndChildrenFromPVS (csPVS& pvs, csOctreeNode* occludee);

  /**
   * Calculate masks for the sides of all nodes.
   */
  void CalculateSolidMasks (csOctreeNode* node);

  /**
   * Add all nodes of a tree to the PVS of 'leaf'.
   */
  void AddDummyPVSNodes (csOctreeNode* leaf, csOctreeNode* node);

  /**
   * Set up a dummy PVS for this node.
   */
  void SetupDummyPVS (csOctreeNode* node);

  /**
   * Classify a polygon with respect to this tree.
   */
  int ClassifyPolygon (csOctreeNode* node, const csPoly3D& poly);

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
   * Get the node for this path.
   */
  csOctreeNode* GetNodeFromPath (csOctreeNode* node,
	unsigned char* path, int path_len);

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
  csOctree (csThing* thing, const csVector3& min_bbox,
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
   * If bVisible is set true, the method returns all visible corners.
   * This could be up to 7 vertices.  
   */
  void GetConvexOutline (csOctreeNode* node, const csVector3& pos,
  	csVector3* array, int& num_array, bool bVisible = false)
  {
    node->bbox.GetConvexOutline (pos, array, num_array, bVisible);
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
   * Set up a dummy PVS (i.e. mark everything as visible).
   */
  void SetupDummyPVS ()
  {
    SetupDummyPVS ((csOctreeNode*)root);
  }

  /**
   * Test if 'box1' can see 'box2' through all the polygons
   * in this octree.
   */
  bool BoxCanSeeBox (const csBox3& box1, const csBox3& box2);

  /**
   * Given a position return the leaf that this position is in.
   */
  csOctreeNode* GetLeaf (const csVector3& pos);

  /**
   * Build vertex tables for minibsp leaves. These tables are
   * used to optimize the world to camera transformation so that only
   * the needed vertices are transformed.
   */
  void BuildVertexTables () { if (root) ((csOctreeNode*)root)->BuildVertexTables (); }

  /// Print statistics about this octree.
  void Statistics ();

  /**
   * Take a 3D polygon and classify it with respect to this tree.
   * The value returned is 1 if the polygon is entirely in solid space,
   * 0 if the polygon is entirely in open space and -1 otherwise (i.e.
   * if the polygon would have to be split). The polygon is not actually
   * inserted. Note that this algorithm cannot be 100% perfect. If a world
   * is not properly formed (i.e. there are floating single polygons)
   * then it is possible (but unlikely) that a polygon is misclassified
   * as being in solid space while it actually isn't.
   */
  int ClassifyPolygon (const csPoly3D& poly)
  {
    return ClassifyPolygon ((csOctreeNode*)root, poly);
  }

  /**
   * Take an axis-aligned rectangle in 3D space represented by the plane
   * and a 2D box on that plane.
   * Return a 4x4 bitmap with information about the solid/open state
   * of every 1/16'th of that rectangle.
   */
  UShort ClassifyRectangle (int plane_nr, float plane_pos,
  	const csBox2& box);

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

#endif // __CS_OCTREE_H__
