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
  csPoly3DBox (const csPoly3D& other) : csPoly3D (other)
  {
  }
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
 * An axis aligned polygon used during kdtree building.
 */
class csPoly3DAxis
{
private:
  csPoly3DBox* poly;
  // Axis is implicit because the polygons are kept in seperate arrays.
  float where;

public:
  csPoly3DAxis (csPoly3DBox* poly, float where)
  {
    csPoly3DAxis::poly = poly;
    csPoly3DAxis::where = where;
  }
  csPoly3DBox* GetPoly () const { return poly; }
  float GetWhere () const { return where; }
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

  // Array of axis aligned polygons which are coplanar with the split axis.
  csArray<csPoly3DAxis> axis_polygons;

  // Axis plane.
  int axis;
  float where;

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

  /**
   * Find all axis aligned polygons that are coplanar
   * with the given plane (which must be one of the sides
   * of this box). This function assumes there is a parent
   * for this node!
   */
  const csArray<csPoly3DAxis>& GetAxisPolygons (int axis, float where);
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
  int total_invisnodes;
  int total_visnodes;
  csTicks starttime;

  // For parsing meta data.
  csStringHash xmltokens;
#define CS_TOKEN_ITEM_FILE "apps/tools/pvscalc/pvscalc.tok"
#include "cstool/tokenlist.h"

  // The shadow KD tree used during calculations.
  PVSCalcNode* shadow_tree;

  // Minimum polygon area before it is being considered for PVS calculation.
  float min_polygon_area;

  // Minimal size for nodes in the kdtree.
  csVector3 minsize;
  bool minsize_specified;

  // Help variables used during calculation. They are here so that the internal
  // memory that they allocate on construction is reused.
  csPoly2D poly_intersect;
  csPoly2D poly;

  // Polygons add by meta data. Will be merged with the static polygon array
  // later.
  csArray<csPoly3D> meta_polygons;

  // All static polygons. Will be sorted on size.
  csArray<csPoly3DBox> polygons;

  // All world boxes for all objects. Will be used for calculating kdtree.
  csArray<csBox3> boxes;

  /**
   * All axis aligned polygons. This will be used during KDtree building
   * for better kdtree quality.
   */
  csArray<csPoly3DAxis> axis_polygons[3];

  // Projection plane information.
  PVSCalcProjectionPlane plane;

  /// Distribute a set of boxes to left/right.
  static void DistributeBoxes (int axis, float where,
	const csArray<csBox3>& boxlist,
	csArray<csBox3>& boxlist_left,
	csArray<csBox3>& boxlist_right);
  /// Distribute a set of polygons to left/right.
  static void DistributePolygons (int axis, float where,
	const csArray<csPoly3DBox*>& polylist,
	csArray<csPoly3DBox*>& polylist_left,
	csArray<csPoly3DBox*>& polylist_right);
  /// Distribute a set of axis aligned polygons to left/right.
  static void DistributePolygons (int axis, float where,
	const csArray<csPoly3DAxis>& polylist,
	csArray<csPoly3DAxis>& polylist_same,
	csArray<csPoly3DAxis>& polylist_left,
	csArray<csPoly3DAxis>& polylist_right);

  /// Build the kdtree.
  void BuildKDTree ();
  void BuildKDTree (void* node, const csArray<csBox3>& boxlist,
	const csArray<csPoly3DAxis>* axis_polylist,
	const csBox3& bbox, bool minsize_only, int depth);

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
	const csArray<csPoly3DBox*>& polygons,
	const csArray<csPoly3DAxis>* axis_polygons);

  /**
   * This is a quick test to see if two nodes can surely see
   * each other. It works by doing a few HitBeam() calls. If there
   * is a beam that hits no polygons then the two nodes can surely
   * see each other.
   * If 'center_only' is true we only test center. This is useful
   * in case the two boxes are adjacent.
   */
  bool NodesSurelyVisible (const csBox3& source, const csBox3& dest,
  	bool center_only);

  /**
   * Find a good split between 'from' and 'to' for the given
   * axis aligned polygons. The best split is the one that is closest
   * to the center between from and to. If there are no polygons
   * in the list the center location is returned.
   */
  static float FindBestSplitLocation (float from, float to, float& where,
	const csArray<csPoly3DAxis>& axis_polylist);

  /**
   * Try to find a split between two boxes along an axis and return the
   * quality of that split. This is used for the building of the kdtree.
   * 'axis_polylist' is a list of polygons that are axis aligned with
   * the current axis.
   */
  static float FindBestSplitLocation (int axis, float& where,
	const csArray<csPoly3DAxis>& axis_polylist,
	const csBox3& bbox1, const csBox3& bbox2);

  /**
   * Try to find the best split location for a number of boxes.
   * This is used for the building of the kdtree.
   * 'axis_polylist' is a list of polygons that are axis aligned with
   * the current axis.
   */
  static float FindBestSplitLocation (int axis, float& where,
	const csBox3& node_bbox,
	const csArray<csPoly3DAxis>& axis_polylist,
	const csArray<csBox3>& boxlist);

  /**
   * Try to find the best split location for a number of boxes.
   * This is used for the building of the kdtree.
   * This version uses csPoly3DBox.
   */
  static float FindBestSplitLocation (int axis, float& where,
	const csBox3& node_bbox, const csArray<csPoly3DBox*>& polylist);

  /// Sort all polygons on size.
  void SortPolygonsOnSize ();

  /// Extract all axis aligned polygons.
  void ExtractAxisAlignedPolygons ();

  /**
   * Given two boxes, calculate the best plane to use for projecting
   * the area shadows on. If the boxes are adjacent or one box is enclosed
   * in the other this function will return false. In that case the destination
   * box is surely visible from the source box.
   * In 'where_other' the other plane is returned. Any shadow plane
   * between 'where' and 'where_other' can be used as a valid shadow plane.
   */
  bool FindShadowPlane (const csBox3& source, const csBox3& dest,
  	int& axis, float& where, float& where_other);

  /**
   * Setup the projection plane and coverage buffer for two boxes. This
   * will call FindShadowPlane(). Returns false if the destination box
   * is surely visible from the source box.
   */
  bool SetupProjectionPlane (const csBox3& source, const csBox3& dest);

  /**
   * This is a special case of SetupProjectionPlane() which works for
   * two adjacent boxes. The 'adjacency' parameter is one of CS_BOX_SIDE_...
   * as seen from the source box.
   * Returns false if the destination box is surely visible from the
   * source box.
   */
  bool SetupProjectionPlaneAdjacent (const csBox3& source, const csBox3& dest,
  	int adjacency);

  /**
   * Calculate the area shadow on the shadow plane for a given polygon as
   * seen from the source box.
   * Also update this on the coverage buffer. This function returns true
   * if the coverage buffer was actually modified.
   */
  bool CastAreaShadow (const csBox3& source, const csPoly3D& polygon);

  /**
   * Version of CastAreaShadow() that casts a simple shadow of a polygon.
   * This is useful when the polygon is axis aligned with the coverage
   * buffer.
   */
  bool CastShadow (const csPoly3D& polygon);

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
   * Version of CastShadowsUntilFull() that works when both boxes
   * are adjacent.
   */
  int CastShadowsUntilFullAdjacent (
	const csArray<csPoly3DAxis>& axis_polygons);

  /**
   * Find all invisible nodes for the given source node by recursively
   * traversing the destination node. This will update the set of
   * invisible nodes. This function returns true if the dest node was
   * found invisible. Otherwise false.
   */
  bool RecurseDestNodes (PVSCalcNode* sourcenode, PVSCalcNode* destnode,
	csSet<PVSCalcNode*>& invisible_nodes);

  /**
   * Help function to mark destnode to be invisible from sourcenode
   * and also look at symmetry. This function will also update
   * the invisible_nodes set.
   */
  void MarkInvisible (PVSCalcNode* sourcenode, PVSCalcNode* destnode,
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

  /**
   * Collect all geometry from this mesh if static.
   * If not-static the mesh is still used for kdtree generation.
   */
  void CollectGeometry (iMeshWrapper* mesh,
  	csBox3& allbox, csBox3& staticbox,
	int& allcount, int& staticcount,
	int& allpcount, int& staticpcount);

public:
  PVSCalcSector (PVSCalc* parent, iSector* sector, iPVSCuller* pvs);
  ~PVSCalcSector ();

  /**
   * Feed meta information into this sector PVS calculator.
   * Returns false on failure. Error already reported.
   */
  bool FeedMetaInformation (iDocumentNode* node);

  /**
   * Calculate the PVS for this sector.
   */
  void Calculate (bool do_quick);
};

/**
 * A loader addon which will parse the meta data that is useful for the
 * PVS culler.
 */
class PVSMetaLoader : public iLoaderPlugin
{
private:
  PVSCalc* parent;
  csRef<iSyntaxService> synserv;

public:
  PVSMetaLoader (PVSCalc* parent);
  virtual ~PVSMetaLoader ();

  SCF_DECLARE_IBASE;

  virtual csPtr<iBase> Parse (iDocumentNode* node, iLoaderContext* ldr_context,
  	iBase* context);
};

/**
 * Meta information for a sector.
 */
struct PVSMetaInfo
{
  iSector* sector;
  csRef<iDocumentNode> meta_node;
  PVSMetaInfo (iSector* sector, iDocumentNode* meta_node)
  {
    PVSMetaInfo::sector = sector;
    PVSMetaInfo::meta_node = meta_node;
  }
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

  /// The meta loader.
  csRef<PVSMetaLoader> meta_loader;

  /// Meta information per sector.
  csArray<PVSMetaInfo> meta_info;

  /// Here we will load our world from a map file.
  bool LoadMap ();

  /// Set the current dir to the requested mapfile.
  bool SetMapDir (const char* map_dir);

  // Quick PVS calculation.
  bool do_quick;

  /**
   * Calculate PVS for the given sector and culler.
   * Returns false on failure. Error already reported.
   */
  bool CalculatePVS (iSector* sector, iPVSCuller* pvs);

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

  /**
   * Register meta information with a sector. This is called from within
   * the meta information parser (PVSMetaLoader).
   */
  void RegisterMetaInformation (iSector* sector, iDocumentNode* meta_node);
};

#endif // __PVSCALC_H__
