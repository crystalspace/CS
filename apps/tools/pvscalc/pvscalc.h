/*
    Copyright (C) 2004-2005 by Jorrit Tyberghein

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

#ifndef __PVSCALC_H__
#define __PVSCALC_H__

#include <crystalspace.h>

class PVSCalc;

/// Dimension of the coverage buffer.
#define DIM_COVBUFFER 512

/**
 * All information related to the projection plane where all area
 * shadows are projected on.
 */
struct PVSCalcProjectionPlane
{
  // The tiled coverage buffer.
  csTiledCoverageBuffer* covbuf;

  // Axis and location of plane.
  int axis;
  float where;

  // After projection of a shadow on the plane it will be a 2D polygon.
  // This 2D polygon needs to be scaled on the coverage buffer using
  // the following scale and offset.
  // From projected 2D point (p) to coverage buffer (c):
  //     c = (p-offset) * scale;
  csVector2 scale, offset;

  // Here we setup a box clipper that represents the boundaries of the
  // coverage buffer. This will be used to quickly intersect
  // polygons that are projected on this plane.
  csBoxClipper* covbuf_clipper;
};

/**
 * A polygon with a bounding box.
 */
class csPoly3DBox : public csPoly3D
{
private:
  csBox3 bbox;
  csPlane3 plane;
  float area;

public:
  csPoly3DBox () : csPoly3D () { }
  csPoly3DBox (const csPoly3DBox& other) : csPoly3D (other)
  {
    bbox = other.GetBBox ();
    plane = other.GetPlane ();
    area = other.GetPrecalcArea ();
  }

  /// Calculate the bbox, plane and area.
  void Calculate ();
  /// Get the bbox.
  const csBox3& GetBBox () const { return bbox; }
  /// Get the plane.
  const csPlane3& GetPlane () const { return plane; }
  /// Get the area.
  float GetPrecalcArea () const { return area; }
};

/**
 * Every PVSCalcNode leaf (see below) will keep a kdtree of polygons
 * in that leaf.
 */
struct PVSPolygonNode
{
  // Children.
  PVSPolygonNode* child1;
  PVSPolygonNode* child2;

  // Box for this node.
  csBox3 node_bbox;

  // Split axis and position.
  int axis;
  float where;

  // If this is a leaf then we keep a list of polygons here.
  csArray<csPoly3DBox*> polygons;

  PVSPolygonNode ()
  {
    child1 = child2 = 0;
  }

  ~PVSPolygonNode ()
  {
    delete child1;
    delete child2;
  }

  // Test if a beam hits a polygon in this node.
  bool HitBeam (const csSegment3& seg);
};

/**
 * The PVS culler maintains the real kdtree node for us but we need to have
 * additional information so that's why we keep a mirror tree.
 */
struct PVSCalcNode
{
  // Pointer to real node in the kdtree managed by PVSvis.
  void* node;

  // Box for this node.
  csBox3 node_bbox;

  // Parents and children.
  PVSCalcNode* parent;
  PVSCalcNode* child1;
  PVSCalcNode* child2;

  // If this is a leaf then we keep an additional KDtree for
  // the polygons here.
  PVSPolygonNode* polygon_tree;

  // Invisible nodes for this node.
  csSet<PVSCalcNode*> invisible_nodes;
  // If true we have calculated visibility information for this node.
  bool calculated_pvs;
  // How many nodes are represented by this node. For a leaf this will
  // be one.
  int represented_nodes;

  PVSCalcNode (void* node)
  {
    PVSCalcNode::node = node;
    parent = 0;
    child1 = child2 = 0;
    polygon_tree = 0;
    calculated_pvs = false;
  }
  ~PVSCalcNode ()
  {
    delete child1;
    delete child2;
    delete polygon_tree;
  }
  bool IsInvisible (PVSCalcNode* dest)
  {
    if (invisible_nodes.In (dest)) return true;
    if (parent)
      return parent->IsInvisible (dest);
    else
      return false;
  }

  // Test if a beam hits a polygon in this node.
  bool HitBeam (const csSegment3& seg);
};

/**
 * The PVS calculator for one sector.
 */
class PVSCalcSector
{
private:
  PVSCalc* parent;
  iSector* sector;
  iPVSCuller* pvs;
  iStaticPVSTree* pvstree;

  // Statistics information.
  int maxdepth;
  int totalnodes;
  int countnodes;
  csTicks starttime;

  // The shadow KD tree used during calculations.
  PVSCalcNode* shadow_tree;

  // Help variables used during calculation. They are here so that the internal
  // memory that they allocate on construction is reused.
  csPoly2D poly_intersect;
  csPoly2D poly;

  // All static polygons. Will be sorted on size.
  csArray<csPoly3DBox> polygons;
  // All world boxes for all objects. Will be used for calculating kdtree.
  csArray<csBox3> boxes;

  // Projection plane information.
  PVSCalcProjectionPlane plane;

  /// Distribute a set of boxes to left/right.
  static void DistributeBoxes (int axis, float where,
	const csArray<csBox3>& boxlist,
	csArray<csBox3>& boxlist_left,
	csArray<csBox3>& boxlist_right);
  /// Distribute a set of polygons to left right.
  static void DistributePolygons (int axis, float where,
	const csArray<csPoly3DBox*>& polylist,
	csArray<csPoly3DBox*>& polylist_left,
	csArray<csPoly3DBox*>& polylist_right);

  /// Build the kdtree.
  void BuildKDTree ();
  void BuildKDTree (void* node, const csArray<csBox3>& boxlist,
	const csBox3& bbox, const csVector3& minsize,
	bool minsize_only, int depth);

  /**
   * Build the shadow KDTree from the KDTree in the culler.
   */
  PVSCalcNode* BuildShadowTree (void* node);

  /**
   * Build the polygon tree that is in the leaf of the shadow tree.
   */
  void BuildShadowTreePolygons (PVSPolygonNode* node, const csBox3& bbox_node,
	const csArray<csPoly3DBox*>& polygons);

  /**
   * Distribute all polygons over the shadow tree and if needed
   * build additional polygon trees in the leaves of the shadow tree.
   */
  void BuildShadowTreePolygons (PVSCalcNode* node,
	const csArray<csPoly3DBox*>& polygons);

  /**
   * This is a quick test to see if two nodes can surely see
   * each other. It works by doing a few HitBeam() calls. If there
   * is a beam that hits no polygons then the two nodes can surely
   * see each other.
   */
  bool NodesSurelyVisible (const csBox3& source, const csBox3& dest);

  /**
   * Try to find a split between two boxes along an axis and return the
   * quality of that split. This is used for the building of the kdtree.
   */
  static float FindBestSplitLocation (int axis, float& where,
	const csBox3& bbox1, const csBox3& bbox2);

  /**
   * Try to find the best split location for a number of boxes.
   * This is used for the building of the kdtree.
   */
  static float FindBestSplitLocation (int axis, float& where,
	const csBox3& node_bbox, const csArray<csBox3>& boxlist);

  /**
   * Try to find the best split location for a number of boxes.
   * This is used for the building of the kdtree.
   * This version uses csPoly3DBox.
   */
  static float FindBestSplitLocation (int axis, float& where,
	const csBox3& node_bbox, const csArray<csPoly3DBox*>& polylist);

  /// Sort all polygons on size.
  void SortPolygonsOnSize ();

  /**
   * Given two boxes, calculate the best plane to use for projecting
   * the area shadows on. If the boxes are adjacent or one box is enclosed
   * in the other this function will return false. In that case the destination
   * box is surely visible from the source box.
   */
  bool FindShadowPlane (const csBox3& source, const csBox3& dest,
  	int& axis, float& where);

  /**
   * Setup the projection plane and coverage buffer for two boxes. This
   * will call FindShadowPlane(). Returns false if the destination box
   * is surely visible from the source box.
   */
  bool SetupProjectionPlane (const csBox3& source, const csBox3& dest);

  /**
   * Calculate the area shadow on the shadow plane for a given polygon as
   * seen from the source box.
   * Also update this on the coverage buffer. This function returns true
   * if the coverage buffer was actually modified.
   */
  bool CastAreaShadow (const csBox3& source, const csPoly3D& polygon);

  /**
   * Cast shadows on the previously set up projection plane until the
   * coverage buffer is full or we ran out of relevant polygons.
   * This function will return 1 if the coverage buffer was full. That means
   * that the destination cannot be seen from the source. This function will
   * return -1 if the coverage buffer was competely empty. That means that
   * the destination is fully visible and also children of the destination
   * will be visible too (no need to traverse further).
   * If the function returns 0 we have no information except that the
   * destination is visible. So we have to test children.
   */
  int CastShadowsUntilFull (const csBox3& source);

  /**
   * Find all invisible nodes for the given source node by recursively
   * traversing the destination node. This will update the set of
   * invisible nodes.
   */
  void RecurseDestNodes (PVSCalcNode* sourcenode, PVSCalcNode* destnode,
	csSet<PVSCalcNode*>& invisible_nodes);


  /**
   * Traverse the kdtree for source nodes and calculate the visibility set
   * for each of them. The set of invisible nodes are the nodes that are
   * invisible for the parent of the source node. This set is given to this
   * function as a copy so that it is ok to modify it. nodecounter is used
   * to be able to print out some progress.
   */
  void RecurseSourceNodes (PVSCalcNode* sourcenode,
  	csSet<PVSCalcNode*> invisible_nodes, int& nodecounter);

public:
  PVSCalcSector (PVSCalc* parent, iSector* sector, iPVSCuller* pvs);
  ~PVSCalcSector ();

  /**
   * Collect all geometry from this mesh if static.
   * If not-static the mesh is still used for kdtree generation.
   */
  void CollectGeometry (iMeshWrapper* mesh,
  	csBox3& allbox, csBox3& staticbox,
	int& allcount, int& staticcount,
	int& allpcount, int& staticpcount);

  /**
   * Calculate the PVS for this sector.
   */
  void Calculate ();
};

/**
 * PVS calculator application. This is for the PVS visibility culler.
 */
class PVSCalc : public csApplicationFramework, public csBaseEventHandler
{
private:
  /// A pointer to the 3D engine.
  csRef<iEngine> engine;

  /// A pointer to the map loader plugin.
  csRef<iLoader> loader;

  /// A pointer to the 3D renderer plugin.
  csRef<iGraphics3D> g3d;

  /// A pointer to the keyboard driver.
  csRef<iKeyboardDriver> kbd;

  /// A pointer to the virtual clock.
  csRef<iVirtualClock> vc;

  /// The sector we are scanning. Or empty if we scan all.
  csString sectorname;

  /// Here we will load our world from a map file.
  bool LoadMap ();

  /// Set the current dir to the requested mapfile.
  bool SetMapDir (const char* map_dir);

  /// Calculate PVS for the given sector and culler.
  void CalculatePVS (iSector* sector, iPVSCuller* pvs);

  /// Calculate PVS for all sectors as given in 'sectorname'.
  void CalculatePVS ();

public:

  /// Construct our game. This will just set the application ID for now.
  PVSCalc ();

  /// Destructor.
  ~PVSCalc ();

  /// Final cleanup.
  void OnExit ();

  /**
   * Main initialization routine. This routine will set up some basic stuff
   * (like load all needed plugins, setup the event handler, ...).
   * In case of failure this routine will return false. You can assume
   * that the error message has been reported to the user.
   */
  bool OnInitialize (int argc, char* argv[]);

  /**
   * Run the application.
   * First, there are some more initialization (everything that is needed 
   * by PVSCalc1 to use Crystal Space), then this routine fires up the main
   * event loop. This is where everything starts. This loop will  basically
   * start firing events which actually causes Crystal Space to function.
   * Only when the program exits this function will return.
   */
  bool Application ();

};

#endif // __PVSCALC_H__
