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

#include "pvscalc.h"

CS_IMPLEMENT_APPLICATION

//#define DB(x) printf##x
#define DB(x)

#define DBA(x) printf##x

//-----------------------------------------------------------------------------

PVSCalcSector::PVSCalcSector (PVSCalc* parent, iSector* sector, iPVSCuller* pvs)
{
  PVSCalcSector::parent = parent;
  PVSCalcSector::sector = sector;
  PVSCalcSector::pvs = pvs;
  pvstree = pvs->GetPVSTree ();

  // @@@ Make dimension configurable?
  plane.covbuf = new csTiledCoverageBuffer (DIM_COVBUFFER, DIM_COVBUFFER);
  plane.covbuf_clipper = 0;
}

PVSCalcSector::~PVSCalcSector ()
{
  delete plane.covbuf;
  delete plane.covbuf_clipper;
}

void PVSCalcSector::DistributeBoxes (int axis, float where,
	const csArray<csBox3>& boxlist,
	csArray<csBox3>& boxlist_left,
	csArray<csBox3>& boxlist_right)
{
  size_t i;
  for (i = 0 ; i < boxlist.Length () ; i++)
  {
    int split = boxlist[i].TestSplit (axis, where);
    if (split <= 0)
      boxlist_left.Push (boxlist[i]);
    if (split >= 0)
      boxlist_right.Push (boxlist[i]);
  }
}

float PVSCalcSector::FindBestSplitLocation (int axis, float& where,
	const csBox3& bbox1, const csBox3& bbox2)
{
  float max1 = bbox1.Max (axis);
  float min2 = bbox2.Min (axis);
  if (max1 < min2-.01)
  {
    where = max1 + (min2-max1) * 0.5;
    return min2-max1;
  }
  float min1 = bbox1.Min (axis);
  float max2 = bbox2.Max (axis);
  if (max2 < min1-.01)
  {
    where = max2 + (min1-max2) * 0.5;
    return min1-max2;
  }
  return -1.0;
}

float PVSCalcSector::FindBestSplitLocation (int axis, float& where,
	const csBox3& node_bbox, const csArray<csBox3>& boxlist)
{
  if (boxlist.Length () == 2)
  {
    return FindBestSplitLocation (axis, where, boxlist[0], boxlist[1]);
  }

  size_t i, j;

  // Calculate minimum and maximum value along the axis.
  float mina = boxlist[0].Min (axis);
  float maxa = boxlist[0].Max (axis);
  for (i = 1 ; i < boxlist.Length () ; i++)
  {
    const csBox3& bbox = boxlist[i];
    if (bbox.Min (axis) < mina) mina = bbox.Min (axis);
    if (bbox.Max (axis) > maxa) maxa = bbox.Max (axis);
  }
  // Make sure we don't go outside node_box.
  if (mina < node_bbox.Min (axis)) mina = node_bbox.Min (axis);
  if (maxa > node_bbox.Max (axis)) maxa = node_bbox.Max (axis);

  // Do 10 tests to find best split location. This should
  // probably be a configurable parameter.

  // @@@ Is the routine below very efficient?
# define FBSL_ATTEMPTS 50
  float a;
  float best_qual = -2.0;
  float inv_num_objects = 1.0 / float (boxlist.Length ());
  for (i = 0 ; i < FBSL_ATTEMPTS ; i++)
  {
    // Calculate a possible split location.
    a = mina + float (i+1)*(maxa-mina)/float (FBSL_ATTEMPTS+1.0);
    // Now count the number of objects that are completely
    // on the left and the number of objects completely on the right
    // side. The remaining objects are cut by this position.
    int left = 0;
    int right = 0;
    for (j = 0 ; j < boxlist.Length () ; j++)
    {
      const csBox3& bbox = boxlist[j];
      // The .0001 is for safety.
      if (bbox.Max (axis) < a-.0001) left++;
      else if (bbox.Min (axis) > a+.0001) right++;
    }
    int cut = boxlist.Length ()-left-right;
    // If we have no object on the left or right then this is a bad
    // split which we should never take.
    float qual;
    if (left == 0 || right == 0)
    {
      qual = -1.0;
    }
    else
    {
      float qual_cut = 1.0 - (float (cut) * inv_num_objects);
      float qual_balance = 1.0 - (float (ABS (left-right)) * inv_num_objects);
      qual = 6.0 * qual_cut + qual_balance;
    }
    if (qual > best_qual)
    {
      best_qual = qual;
      where = a;
    }
  }
# undef FBSL_ATTEMPTS
  return best_qual;
}

void PVSCalcSector::BuildKDTree (void* node, const csArray<csBox3>& boxlist,
	const csBox3& bbox, const csVector3& minsize,
	bool minsize_only, int depth)
{
  if (depth > maxdepth) maxdepth = depth;

  int axis;
  float where;
  csVector3 bbox_size = bbox.Max () - bbox.Min ();
  if (minsize_only || boxlist.Length () <= 1)
  {
    // If we have 1 or less objects left then we continue splitting the
    // node so that leafs are smaller then 'minsize'.
    if (bbox_size.x > minsize.x)
    {
      axis = 0;
      where = bbox.MinX () + bbox_size.x / 2;
    }
    else if (bbox_size.y > minsize.y)
    {
      axis = 1;
      where = bbox.MinY () + bbox_size.y / 2;
    }
    else if (bbox_size.z > minsize.z)
    {
      axis = 2;
      where = bbox.MinZ () + bbox_size.z / 2;
    }
    else
    {
      return;
    }
  }
  else
  {
    float where0, where1, where2;
    float q0 = FindBestSplitLocation (0, where0, bbox, boxlist);
    float q1 = FindBestSplitLocation (1, where1, bbox, boxlist);
    float q2 = FindBestSplitLocation (2, where2, bbox, boxlist);
    if (q0 > q1 && q0 > q2)
    {
      axis = 0;
      where = where0;
    }
    else if (q1 > q0 && q1 > q2)
    {
      axis = 1;
      where = where1;
    }
    else if (q2 >= 0)
    {
      axis = 2;
      where = where2;
    }
    else
    {
      // All options are bad. Here we mark the traversal so that
      // we only continue to split for minsize.
      BuildKDTree (node, boxlist, bbox, minsize, true, depth);
      return;	// No good split location.
    }
  }

  void* child1;
  void* child2;
  csBox3 box1, box2;
  bbox.Split (axis, where, box1, box2);
  csArray<csBox3> boxlist_left;
  csArray<csBox3> boxlist_right;
  DistributeBoxes (axis, where, boxlist, boxlist_left, boxlist_right);
  pvstree->SplitNode (node, axis, where, child1, child2);
  countnodes += 2;
  BuildKDTree (child1, boxlist_left, box1, minsize, minsize_only, depth+1);
  BuildKDTree (child2, boxlist_right, box2, minsize, minsize_only, depth+1);
}

void PVSCalcSector::BuildKDTree ()
{
  iStaticPVSTree* pvstree = pvs->GetPVSTree ();
  pvstree->Clear ();
  void* root = pvstree->CreateRootNode ();
  const csBox3& bbox = pvstree->GetBoundingBox ();
  const csVector3& minsize = pvstree->GetMinimalNodeBox ();
  countnodes = 1;
  maxdepth = 0;
  BuildKDTree (root, boxes, bbox, minsize, false, 0);
}

void PVSCalcSector::CollectGeometry (iMeshWrapper* mesh,
  	csBox3& allbox, csBox3& staticbox,
	int& allcount, int& staticcount,
	int& allpcount, int& staticpcount)
{
  csBox3 mbox;
  mesh->GetWorldBoundingBox (mbox);
  iMeshObject* meshobj = mesh->GetMeshObject ();
  iObjectModel* objmodel = meshobj->GetObjectModel ();
  iPolygonMesh* polybase = objmodel->GetPolygonMeshViscull ();

  // Increase stats for all objects.
  allcount++;
  allbox += mbox;
  if (polybase) allpcount += polybase->GetPolygonCount ();

  // Register the box.
  boxes.Push (mbox);

  iMeshFactoryWrapper* fact = mesh->GetFactory ();
  bool staticshape_fact = fact
    	? fact->GetMeshObjectFactory ()
		->GetFlags ().Check (CS_FACTORY_STATICSHAPE)
	: false;
  const csFlags& mesh_flags = meshobj->GetFlags ();
  bool staticshape_mesh = mesh_flags.Check (CS_MESH_STATICSHAPE);
  if (polybase && polybase->GetPolygonCount () &&
	mesh_flags.Check (CS_MESH_STATICPOS) &&
    	(staticshape_mesh || staticshape_fact))
  {
    // Increase stats for static objects.
    staticcount++;
    staticbox += mbox;
    staticpcount += polybase->GetPolygonCount ();
    csReversibleTransform trans = mesh->GetMovable ()->GetFullTransform ();
    csVector3* vertices = polybase->GetVertices ();
    csMeshedPolygon* mp = polybase->GetPolygons ();
    int p, vt;
    for (p = 0 ; p < polybase->GetPolygonCount () ; p++)
    {
      const csMeshedPolygon& poly = mp[p];
      csPoly3D poly3d;
      for (vt = 0 ; vt < poly.num_vertices ; vt++)
      {
        csVector3 vwor = trans.This2Other (vertices[poly.vertices[vt]]);
        poly3d.AddVertex (vwor);
      }
      polygons.Push (poly3d);
    }
  }

  int i;
  iMeshList* ml = mesh->GetChildren ();
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* child = ml->Get (i);
    CollectGeometry (child, allbox, staticbox, allcount, staticcount,
    	allpcount, staticpcount);
  }
}

struct poly_with_area
{
  size_t idx;
  float area;
};

static int compare_polygons_on_size (const poly_with_area& p1,
	const poly_with_area& p2)
{
  if (p1.area > p2.area) return -1;
  else if (p1.area < p2.area) return 1;
  else return 0;
}

void PVSCalcSector::SortPolygonsOnSize ()
{
  csArray<poly_with_area> polygons_with_area;
  double total_area = 0.0;
  float min_area = 1000000000.0;
  float max_area = -1.0f;
  size_t i;
  for (i = 0 ; i < polygons.Length () ; i++)
  {
    poly_with_area pwa;
    pwa.idx = i;
    pwa.area = polygons[i].GetArea ();
    polygons_with_area.Push (pwa);
    if (pwa.area < min_area) min_area = pwa.area;
    if (pwa.area > max_area) max_area = pwa.area;
    total_area += double (pwa.area);
  }
  polygons_with_area.Sort (compare_polygons_on_size);
  csArray<csPoly3D> sorted_polygons;
  for (i = 0 ; i < polygons_with_area.Length () ; i++)
    sorted_polygons.Push (polygons[polygons_with_area[i].idx]);
  polygons = sorted_polygons;

  parent->ReportInfo ("Average polygon area %g, min %g, max %g\n",
  	float (total_area / polygons.Length ()),
	min_area, max_area);
}

bool PVSCalcSector::FindShadowPlane (const csBox3& source, const csBox3& dest,
  	int& axis, float& where)
{
  float sdx = dest.MinX () - source.MaxX ();
  float sdy = dest.MinY () - source.MaxY ();
  float sdz = dest.MinZ () - source.MaxZ ();
  float dsx = source.MinX () - dest.MaxX ();
  float dsy = source.MinY () - dest.MaxY ();
  float dsz = source.MinZ () - dest.MaxZ ();
  if (sdx > SMALL_EPSILON &&
  	sdx > sdy &&
	sdx > sdz &&
	sdx > dsx &&
	sdx > dsy &&
	sdx > dsz)
  {
    axis = 0;
    where = dest.MinX ();
    return true;
  }
  if (dsx > SMALL_EPSILON &&
  	dsx > sdx &&
	dsx > sdy &&
	dsx > sdz &&
	dsx > dsy &&
	dsx > dsz)
  {
    axis = 0;
    where = dest.MaxX ();
    return true;
  }
  if (sdy > SMALL_EPSILON &&
  	sdy > sdx &&
	sdy > sdz &&
	sdy > dsx &&
	sdy > dsy &&
	sdy > dsz)
  {
    axis = 1;
    where = dest.MinY ();
    return true;
  }
  if (dsy > SMALL_EPSILON &&
  	dsy > sdx &&
	dsy > sdy &&
	dsy > sdz &&
	dsy > dsx &&
	dsy > dsz)
  {
    axis = 1;
    where = dest.MaxY ();
    return true;
  }
  if (sdz > SMALL_EPSILON &&
  	sdz > sdx &&
	sdz > sdy &&
	sdz > dsx &&
	sdz > dsy &&
	sdz > dsz)
  {
    axis = 2;
    where = dest.MinZ ();
    return true;
  }
  if (dsz > SMALL_EPSILON &&
  	dsz > sdx &&
	dsz > sdy &&
	dsz > sdz &&
	dsz > dsx &&
	dsz > dsy)
  {
    axis = 2;
    where = dest.MaxZ ();
    return true;
  }
  return false;
}

bool PVSCalcSector::SetupProjectionPlane (const csBox3& source,
	const csBox3& dest)
{
  if (!FindShadowPlane (source, dest, plane.axis, plane.where))
    return false;

  // We project the destination box on the given shadow plane as seen
  // from every corner of the source box. ProjectOutline() will add the
  // projected vertices to 'poly' so in the end we will have a collection
  // of points on the shadow plane. These points form the boundaries
  // where all the projected shadows will be relevant for visibility.
  csDirtyAccessArray<csVector2> poly;
  dest.ProjectOutline (source.GetCorner (CS_BOX_CORNER_xyz),
  	plane.axis, plane.where, poly);
  dest.ProjectOutline (source.GetCorner (CS_BOX_CORNER_Xyz),
  	plane.axis, plane.where, poly);
  dest.ProjectOutline (source.GetCorner (CS_BOX_CORNER_xYz),
  	plane.axis, plane.where, poly);
  dest.ProjectOutline (source.GetCorner (CS_BOX_CORNER_XYz),
  	plane.axis, plane.where, poly);
  dest.ProjectOutline (source.GetCorner (CS_BOX_CORNER_xyZ),
  	plane.axis, plane.where, poly);
  dest.ProjectOutline (source.GetCorner (CS_BOX_CORNER_XyZ),
  	plane.axis, plane.where, poly);
  dest.ProjectOutline (source.GetCorner (CS_BOX_CORNER_xYZ),
  	plane.axis, plane.where, poly);
  dest.ProjectOutline (source.GetCorner (CS_BOX_CORNER_XYZ),
  	plane.axis, plane.where, poly);

  // Now we calculate the convex hull of the projection points. This
  // convex hull will refine the boundary which is calculated above as
  // only projections that intersect with this hull are relevant for
  // visibility. This convex hull can later be inserted in the coverage
  // buffer.
  csChainHull2D::SortXY (poly.GetArray (), poly.Length ());
  csVector2* hull = new csVector2[poly.Length ()];
  int hull_points = csChainHull2D::CalculatePresorted (poly.GetArray (),
  	poly.Length (), hull);

  // Now we calculate the bounding 2D box of those points. That will
  // be used to calculate how we will transform projected 2D coordinates
  // to coordinates on the coverage buffer.
  csBox2 bbox (hull[0]);
  size_t i;
  for (i = 1 ; i < (size_t)hull_points ; i++)
    bbox.AddBoundingVertexSmart (hull[i]);
  plane.offset = bbox.Min ();
  plane.scale.x = float (DIM_COVBUFFER) / (bbox.MaxX ()-bbox.MinX ());
  plane.scale.y = float (DIM_COVBUFFER) / (bbox.MaxY ()-bbox.MinY ());
  DB(("  Hull box: (%g,%g)-(%g,%g) scale=%g,%g offset=%g,%g\n",
  	bbox.MinX (), bbox.MinY (), bbox.MaxX (), bbox.MaxY (),
	plane.scale.x, plane.scale.y, plane.offset.x, plane.offset.y));

  // Clear the coverage buffer.
  plane.covbuf->Initialize ();

  // Now insert our hull outline inverted in the coverage buffer. That
  // will basically mask out all vertices outside the relevant area.
  // Before we do that we have to transform the hull to coverage buffer
  // space.
  DB(("  Hull:\n"));
  for (i = 0 ; i < (size_t)hull_points ; i++)
  {
    DB(("    N:%d (%g,%g)\n", i, hull[i].x, hull[i].y));
    hull[i].x = (hull[i].x-plane.offset.x) * plane.scale.x;
    hull[i].y = (hull[i].y-plane.offset.y) * plane.scale.y;
    DB(("    C:%d (%g,%g)\n", i, hull[i].x, hull[i].y));
  }
  plane.covbuf->InsertPolygonInvertedNoDepth (hull, hull_points);

  //csRef<iString> str = plane.covbuf->Debug_Dump ();
  //printf ("%s\n", str->GetData ());

  // We no longer need the hull points here.
  delete[] hull;

  // Here we setup the box clipper that represents the boundaries of our
  // coverage buffer.
  plane.covbuf_clipper = new csBoxClipper (bbox);

  return true;
}

bool PVSCalcSector::CastAreaShadow (const csBox3& source,
	const csPoly3D& polygon)
{
  DB(("  Polygon:"));
  size_t j;
  for (j = 0 ; j < polygon.GetVertexCount () ; j++)
  {
    DB((" (%g,%g,%g)", polygon[j].x, polygon[j].y, polygon[j].z));
  }
  DB(("\n"));

  // First we calculate the projection of the polygon on the shadow plane
  // as seen from the first point on the source box. That will be the start
  // for calculating the intersection of all those projections.
  csPoly2D poly_intersect;
  if (!polygon.ProjectAxisPlane (source.GetCorner (CS_BOX_CORNER_xyz),
  	plane.axis, plane.where, &poly_intersect))
    return false;

  // First we clip the projected polygon to the bounding box that
  // represents the coverage buffer. If this intersection is empty already
  // (returns false) then this is an easy way out.
  if (!poly_intersect.ClipAgainst (plane.covbuf_clipper))
    return false;

  // Now we continue calculating projections for the remaining 7 box
  // corners and we intersect those with poly_intersect.
  int i;
  for (i = 1 ; i < 8 ; i++)
  {
    csPoly2D poly;
    if (!polygon.ProjectAxisPlane (source.GetCorner (i),
  	plane.axis, plane.where, &poly))
      return false;

    // Now we construct a clipper from this projected polygon.
    csPolygonClipper clip (&poly);

    // Time to clip our 'poly_intersect'.
    if (!poly_intersect.ClipAgainst (&clip))
      return false;
  }

  DB(("    -> has valid shadow\n"));

  // Now 'poly_intersect' contains the intersection of all projected
  // polygons on the shadow plane. This is the area shadow and we will now
  // insert that in the coverage buffer. But before we do that we first
  // have to scale the 2D coordinates so that they match coverage buffer
  // coordinates.
  csVector2* pi_verts = poly_intersect.GetVertices ();
  DB(("    -> covbuf poly:"));
  for (i = 0 ; i < poly_intersect.GetVertexCount () ; i++)
  {
    pi_verts[i].x = (pi_verts[i].x-plane.offset.x) * plane.scale.x;
    pi_verts[i].y = (pi_verts[i].y-plane.offset.y) * plane.scale.y;
    DB((" (%g,%g)", pi_verts[i].x, pi_verts[i].y));
  }
  DB(("\n"));
  int nummod = plane.covbuf->InsertPolygonNoDepth (
  	pi_verts, poly_intersect.GetVertexCount ());
  DB(("    -> nummod = %d\n", nummod));

  // If nummod > 0 then we modified the coverage buffer and we return
  // true then.
  return nummod > 0;
}

bool PVSCalcSector::CheckRelevantPolygon (float minsource, float maxsource,
	const csPoly3D& polygon)
{
  // A polygon is relevant if it is completely enclosed within the
  // source and the shadow plane. A polygon that intersects the shadow
  // plane or enters the source box is not valid for shadow casting.
  size_t i;
  const csVector3* vertices = polygon.GetVertices ();
  for (i = 0 ; i < polygon.GetVertexCount () ; i++)
  {
    float value = vertices[i][plane.axis];
    if (plane.where < minsource)
    {
      if (value < plane.where || value > minsource)
	return false;
    }
    else
    {
      if (value > plane.where || value < maxsource)
	return false;
    }
  }
  return true;
}

int PVSCalcSector::CastShadowsUntilFull (const csBox3& source)
{
  // Start casting shadows starting with the biggest polygons.
  size_t i;
  int status = -1;	// Coverage buffer is now empty.

  // Calculate this for CheckRelevantPolygon.
  float minsource = source.Min (plane.axis);
  float maxsource = source.Max (plane.axis);

  DB(("  Cast Shadows between %g and %g (plane %d/%g)\n",
  	plane.where < minsource ? plane.where : maxsource,
	plane.where < minsource ? minsource : plane.where,
	plane.axis, plane.where));

  for (i = 0 ; i < polygons.Length () ; i++)
  {
    if (CheckRelevantPolygon (minsource, maxsource, polygons[i]))
      if (CastAreaShadow (source, polygons[i]))
      {
        // The shadow modified the coverage buffer. Test if our buffer is
        // full now.
        status = plane.covbuf->StatusNoDepth ();
        if (status == 1)
        {
          // Yes! Coverage buffer is full. We can return early here.
	  return 1;
        }
      }
  }

  return status;
}

void PVSCalcSector::RecurseDestNodes (void* sourcenode, void* destnode,
	csSet<void*>& invisible_nodes)
{
  // If sourcenode is equal to node then visibility is obvious. In that
  // case we don't proceed since all children of node will also be visible
  // (since they are contained in sourcenode then).
  if (sourcenode == destnode) return;

  // If the destination node is already in the set of invisible nodes
  // then we don't have to proceed either. That means that the destination
  // node is invisible for one of the parents of the sourcenode.
  if (invisible_nodes.In (destnode)) return;

  // Here we have to do the actual shadow casting to see if 'sourcenode'
  // can see 'node'.
  const csBox3& source = pvstree->GetNodeBBox (sourcenode);
  const csBox3& dest = pvstree->GetNodeBBox (destnode);

  // If the source node is contained in the destination node then
  // we know the destination node is visible. However we do have to continue
  // traversing to the children so we only skip the testing part.
  if (!dest.Overlap (source))
  {
    DB(("\nTEST (%g,%g,%g)-(%g,%g,%g) -> (%g,%g,%g)-(%g,%g,%g)\n",
	source.MinX (), source.MinY (), source.MinZ (),
	source.MaxX (), source.MaxY (), source.MaxZ (),
	dest.MinX (), dest.MinY (), dest.MinZ (),
	dest.MaxX (), dest.MaxY (), dest.MaxZ ()));

    // If the projection plane failed to set up we still have to test
    // children.
    if (SetupProjectionPlane (source, dest))
    {
      int level = CastShadowsUntilFull (source);
      if (level == 1)
      {
        // The coverage buffer is full! This means the destination node
        // cannot be seen from the source node.
        invisible_nodes.Add (destnode);
        pvstree->MarkInvisible (sourcenode, destnode);
	DBA(("-"));
        DB(("Marked invisible (%g,%g,%g)-(%g,%g,%g) to (%g,%g,%g)-(%g,%g,%g)\n",
    	    source.MinX (), source.MinY (), source.MinZ (),
    	    source.MaxX (), source.MaxY (), source.MaxZ (),
    	    dest.MinX (), dest.MinY (), dest.MinZ (),
    	    dest.MaxX (), dest.MaxY (), dest.MaxZ ()));

        // If the destination node is invisible that automatically implies
        // that the children are invisible too. No need to recurse further.
        return;
      }
    }
  }

  // The destination node was visible and coverage buffer was partially
  // full. This means that we have to proceed to test visibility for the
  // children of the destination node.
  void* child1 = pvstree->GetFirstChild (destnode);
  if (child1) RecurseDestNodes (sourcenode, child1, invisible_nodes);
  void* child2 = pvstree->GetSecondChild (destnode);
  if (child2) RecurseDestNodes (sourcenode, child2, invisible_nodes);
}

void PVSCalcSector::RecurseSourceNodes (void* sourcenode,
	csSet<void*> invisible_nodes, int& nodecounter)
{
  nodecounter--;
  printf ("\nsource node %p (%d) ", sourcenode, nodecounter); fflush (stdout);
  // We recurse through all destination nodes now. This will update the
  // set of invisible_nodes.
  RecurseDestNodes (sourcenode, pvstree->GetRootNode (), invisible_nodes);

  // Traverse to the children to calculate visibility from there.
  void* child1 = pvstree->GetFirstChild (sourcenode);
  if (child1) RecurseSourceNodes (child1, invisible_nodes, nodecounter);
  void* child2 = pvstree->GetSecondChild (sourcenode);
  if (child2) RecurseSourceNodes (child2, invisible_nodes, nodecounter);
}

void PVSCalcSector::Calculate ()
{
  parent->ReportInfo ("Calculating PVS for '%s'!",
  	sector->QueryObject ()->GetName ());

  const csVector3& minsize = pvstree->GetMinimalNodeBox ();
  parent->ReportInfo ("Minimal node size %g,%g,%g",
  	minsize.x, minsize.y, minsize.z);

  const csBox3& bbox = pvstree->GetBoundingBox ();
  parent->ReportInfo ("Total box (from culler) %g,%g,%g  %g,%g,%g",
  	bbox.MinX (), bbox.MinY (), bbox.MinZ (),
  	bbox.MaxX (), bbox.MaxY (), bbox.MaxZ ());

  int i;
  csBox3 allbox, staticbox;
  allbox.StartBoundingBox ();
  staticbox.StartBoundingBox ();
  int allcount = 0, staticcount = 0;
  int allpcount = 0, staticpcount = 0;
  iMeshList* ml = sector->GetMeshes ();
  for (i = 0 ; i < ml->GetCount () ; i++)
  {
    iMeshWrapper* m = ml->Get (i);
    CollectGeometry (m, allbox, staticbox,
	allcount, staticcount,
	allpcount, staticpcount);
  }

  // We sort polygons so that the polygons with the biggest area
  // are in front of the list. That way we will first try to fill
  // big polygons during visibility calculation.
  SortPolygonsOnSize ();

  parent->ReportInfo ("Total box (all geometry) %g,%g,%g  %g,%g,%g",
  	allbox.MinX (), allbox.MinY (), allbox.MinZ (),
  	allbox.MaxX (), allbox.MaxY (), allbox.MaxZ ());
  parent->ReportInfo ("Total box (static geometry) %g,%g,%g  %g,%g,%g",
  	staticbox.MinX (), staticbox.MinY (), staticbox.MinZ (),
  	staticbox.MaxX (), staticbox.MaxY (), staticbox.MaxZ ());
  parent->ReportInfo( "%d static and %d total objects",
  	staticcount, allcount);
  parent->ReportInfo ("%d static and %d total polygons",
  	staticpcount, allpcount);

  // From all geometry (static and dynamic) we now build the KDtree
  // that is the base for visibility culling.
  BuildKDTree ();
  pvstree->UpdateBoundingBoxes ();
  parent->ReportInfo ("KDTree: max depth=%d, number of nodes=%d",
  	maxdepth, countnodes);

  // Now it is time to calculate (in)visibility between all nodes. This
  // happens in a recursive process where we first traverse source nodes
  // in the tree from the root up. And for every such source nodes we
  // again traverse the tree from the root up for the destination nodes.
  // During this traversal we maintain a set of invisible nodes. These
  // are the nodes that are certainly invisible for the parent of the source
  // node that we're currently considering. That means that those nodes
  // are automatically invisible for the children too and don't have
  // to be considered anymore.
  csSet<void*> invisible_nodes;
  int nodecounter = countnodes;
  RecurseSourceNodes (pvstree->GetRootNode (), invisible_nodes, nodecounter);

  // Write our KDTree with pvs.
  if (!pvstree->WriteOut ())
  {
    parent->ReportError ("Error writing out PVS cache!");
  }
}

//-----------------------------------------------------------------------------

PVSCalc::PVSCalc ()
{
  SetApplicationName ("CrystalSpace.PVSCalcMap");
}

PVSCalc::~PVSCalc ()
{
}

bool PVSCalc::OnInitialize (int argc, char* argv[])
{
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  if (!csInitializer::RequestPlugins (GetObjectRegistry (),
      CS_REQUEST_VFS,
      CS_REQUEST_NULL3D,
      CS_REQUEST_FONTSERVER,
      CS_REQUEST_ENGINE,
      CS_REQUEST_IMAGELOADER,
      CS_REQUEST_LEVELLOADER,
      CS_REQUEST_REPORTER,
      CS_REQUEST_REPORTERLISTENER,
      CS_REQUEST_END))
    return ReportError ("Failed to initialize plugins!");

  // Change the standard reporter listener so that it reports info to
  // standard output.
  csRef<iStandardReporterListener> stdrep = CS_QUERY_REGISTRY (object_reg,
  	iStandardReporterListener);
  stdrep->SetMessageDestination (CS_REPORTER_SEVERITY_NOTIFY, true, false,
  	false, false, false, false);

  // Now we need to setup an event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  if (!RegisterQueue (GetObjectRegistry ()))
    return ReportError ("Failed to set up event handler!");

  return true;
}

void PVSCalc::OnExit ()
{
}

bool PVSCalc::Application ()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError ("Error opening system!");

  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  // The virtual clock.
  g3d = CS_QUERY_REGISTRY(GetObjectRegistry(), iGraphics3D);
  if (!g3d) return ReportError ("Failed to locate 3D renderer!");

  engine = CS_QUERY_REGISTRY(GetObjectRegistry(), iEngine);
  if (!engine) return ReportError ("Failed to locate 3D engine!");

  vc = CS_QUERY_REGISTRY(GetObjectRegistry(), iVirtualClock);
  if (!vc) return ReportError ("Failed to locate Virtual Clock!");

  kbd = CS_QUERY_REGISTRY(GetObjectRegistry(), iKeyboardDriver);
  if (!kbd) return ReportError ("Failed to locate Keyboard Driver!");

  loader = CS_QUERY_REGISTRY(GetObjectRegistry(), iLoader);
  if (!loader) return ReportError ("Failed to locate Loader!");

  // Here we load our world from a map file.
  if (!LoadMap ()) return false;

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  // Calculate the PVS.
  CalculatePVS ();

  return true;
}

bool PVSCalc::SetMapDir (const char* map_dir)
{
  csStringArray paths;
  paths.Push ("/lev/");
  csRef<iVFS> VFS = CS_QUERY_REGISTRY (object_reg, iVFS);
  if (!VFS->ChDirAuto (map_dir, &paths, 0, "world"))
  {
    ReportError ("Error setting VFS directory '%s'!", map_dir);
    return false;
  }
  return true;
}

bool PVSCalc::LoadMap ()
{
  // Set VFS current directory to the level we want to load.
  csRef<iCommandLineParser> cmdline = CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser);
  const char* mapfile = cmdline->GetName (0);
  if (!mapfile)
  {
    ReportError ("Required parameters: <mapdir/zip> [ <sectorname> ]!");
    return false;
  }

  if (!SetMapDir (mapfile))
    return false;

  // Load the level file which is called 'world'.
  if (!loader->LoadMapFile ("world"))
  {
    ReportError ("Error couldn't load world file at '%s'!", mapfile);
    return false;
  }

  // Get optional sector name. If no sector is given then all sectors
  // that have PVS will be calculated.
  sectorname = cmdline->GetName (1);

  return true;
}

void PVSCalc::CalculatePVS (iSector* sector, iPVSCuller* pvs)
{
  PVSCalcSector pvscalcsector (this, sector, pvs);
  pvscalcsector.Calculate ();
}

void PVSCalc::CalculatePVS ()
{
  int i;
  iSectorList* sl = engine->GetSectors ();
  bool found = false;
  for (i = 0 ; i < sl->GetCount () ; i++)
  {
    iSector* sector = sl->Get (i);
    const char* sname = sector->QueryObject ()->GetName ();
    if (sectorname.IsEmpty () || (sname != 0 &&
    	strcmp ((const char*)sectorname, sname) == 0))
    {
      iVisibilityCuller* viscul = sector->GetVisibilityCuller ();
      if (viscul)
      {
	csRef<iPVSCuller> pvs = SCF_QUERY_INTERFACE (viscul, iPVSCuller);
        if (pvs)
	{
	  found = true;
	  CalculatePVS (sector, pvs);
	}
      }
    }
  }
  if (!found)
  {
    if (sectorname.IsEmpty ())
      ReportError ("Found no sectors that have a PVS visibility culler!");
    else
      ReportError ("Sector '%s' has no PVS visibility culler!",
      	(const char*)sectorname);
  }
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  return PVSCalc().Main(argc, argv);
}
