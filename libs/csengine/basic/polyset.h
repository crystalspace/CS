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

#ifndef POLYSET_H
#define POLYSET_H

#include "cscom/com.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h" // texel coords
#include "csgeom/polyint.h"
#include "csobject/csobj.h"
#include "csengine/basic/fog.h"
#include "ipolygon.h"

class csPolygonInt;
class csSector;
class csWorld;
class csCamera;
class csTextureHandle;
class CLights;
class csPolygon3D;
class csCollider;
class csBspTree;
class Dumper;
class csRenderView;
class csCurve;
interface IPolygonSet;

/**
 * A PolygonSet class is a set of polygons (amazing, isn't it :-)
 * A PolygonSet describes a set of polygons that form a convex and
 * (probably) closed hull. All polygons in a set share vertices
 * from the same pool.<p>
 *
 * A recent extension also allows PolygonSets that are not convex
 * by adding an optional BSP tree. This BSP tree is only useful
 * if the PolygonSet is concave. Otherwise the tree is wasted.<p>
 *
 * Every polygon in the set has a visible and an invisible face;
 * if the vertices of the polygon are ordered clockwise then the
 * polygon is visible. Using this feature it is possible to define
 * two kinds of PolygonSets: in one kind the polygons are oriented
 * such that they are visible from within the hull. In other words,
 * the polygons form a sort of container or room where the camera
 * can be located. We call this kind of PolygonSet a csSector (a
 * subclass of PolygonSet). In another kind the polygons are
 * oriented such that they are visible from the outside. We call
 * this kind of PolygonSet a Thing (another subclass of PolygonSet).<p>
 *
 * Things and csSectors have many similarities. That's why the
 * PolygonSet class was created: to exploit these similarities.
 * However, there are some important differences between Things and
 * csSectors:<p>
 * <ul>
 * <li> Currently, only things can move. This means that the object
 *      space coordinates of a csSector are ALWAYS equal to the world
 *      space coordinates. It would be possible to allow moveable
 *      csSectors but I don't how this should be integrated into an
 *      easy model of the world.
 * <li> Things do not require portals but can use them. 
 * </ul>
 */
class csPolygonSet : public csObject, public csPolygonParentInt
{
  friend class Dumper;
  friend class csCollider;
  friend class csSector;

protected:
  /**
   * PolygonSets are linked either in a csWorld object or in another
   * PolygonSet (Thing in csSector for example).
   */
  csPolygonSet* next;

  int num_vertices;
  int max_vertices;
  
  /// Vertices in world space.
  csVector3* wor_verts;
  /// Vertices in object space.
  csVector3* obj_verts;
  /// Vertices in camera space.
  csVector3* cam_verts;

  /// Table of ptr to polygons forming the outside of the set
  csPolygonInt** polygons;
  /// Number of polygons used.
  int num_polygon;
  /// Maximum number of polygons in 'polygon' array.
  int max_polygon;

  /// Table of ptr to curves forming the outside of the set
  csCurve** curves;
  /// Number of curves used.
  int num_curves;
  /// Maximum number of curves in 'curves' array.
  int max_curves;

  /// csSector where this polyset belongs (pointer to 'this' if it is a sector).
  csSector* sector;

  /// Optional bsp tree.
  csBspTree* bsp;

  /**
   * Light frame number. Using this number one can see if gouroud shaded
   * vertices have been initialized already.
   */
  long light_frame_number;

  /**
   * Get a new transformation set of vertices (camera space) and return
   * the old set. This function is required when transforming to camera
   * space because this polyset may already have been transformed to
   * camera space in some other way. It is important to restore the
   * transformation set if you are done with it.
   * The new transformation set is automatically put in the standard
   * 'cam_verts' array and accessible using 'vcam'.
   * Do not use the result from this function except for giving it
   * to restore_transformation. The result is not meaningfull.
   * This function also depends on the correct value of 'draw_busy'.
   * 'draw_busy' should be equal to 1 for the first time you call
   * this 'new_transformation' and should also be equal to 1 when
   * you restore the set for the last time.
   */
  void NewTransformation (csVector3*& old_tr3);

  /**
   * After calling 'new_transformation' you MUST call this
   * function with the result from 'new_transformation'!
   */
  void RestoreTransformation (csVector3* old_tr3);

  /**
   * Draw the given array of polygons from this csPolygonSet. This
   * function is called by subclasses of csPolygonSet (csSector and
   * Thing currently).
   */
  void DrawPolygonArray (csPolygonInt** polygon, int num, csRenderView* rview, bool use_z_buf);

private:
  /**
   * Fog information.
   */
  csFog fog;

public:
  /**
   * Current light frame number. This is used for seeing
   * if gouroud shading should be recalculated for this polygonset.
   * If there is a mismatch between the frame number of this set
   * and the global number then the gouroud shading is not up-to-date.
   */
  static long current_light_frame_number;

  /**
   * How many times are we busy drawing this polyset (recursive).
   * This is an important variable as it indicates to
   * 'new_transformation' which set of camera vertices it should
   * use.
   */
  int draw_busy;


  /**
   * Tesselation parameter:
   * Center of thing to determine distance from
   */
  csVector3 curves_center;
  /// scale param (the larger this param it, the more the curves are tesselated).
  float curves_scale;  

  /**
   * Curve vertices. 
   */
  csVector3* curve_vertices;
  /// Texture coords of curve vertices
  csVector2* curve_texels;

  /// Number of vertices.
  int num_curve_vertices;
  /// Maximum number of vertices.
  int max_curve_vertices;


  /**
   * Construct a csPolygonSet. 'type' is the csObject type and should
   * be one of CS_THING or CS_SECTOR.
   */
  csPolygonSet ();

  /**
   * Delete all contents of this polygonset (vertices,
   * polygons, curves, and BSP tree).
   */
  virtual ~csPolygonSet ();

  /**
   * Prepare all polygons for use. This function MUST be called
   * AFTER the texture manager has been prepared. This function
   * is normally called by csWorld::Prepare() so you only need
   * to worry about this function when you add sectors or things
   * later.
   */
  virtual void Prepare ();

  /**
   * Just add a new vertex to the polygonset.
   */
  int AddVertex (csVector3& v) { return AddVertex (v.x, v.y, v.z); }

  /**
   * Just add a new vertex to the polygonset.
   */
  int AddVertex (float x, float y, float z);

  /**
   * Add a vertex but first check if there is already
   * a vertex close to the wanted position. In that case
   * don't add a new vertex but return the index of the old one.
   */
  int AddVertexSmart (csVector3& v) { return AddVertexSmart (v.x, v.y, v.z); }

  /**
   * Add a vertex but first check if there is already
   * a vertex close to the wanted position. In that case
   * don't add a new vertex but return the index of the old one.
   */
  int AddVertexSmart (float x, float y, float z);

  /**
   * Return the world space vector for the vertex.
   */
  csVector3& Vwor (int idx) { return wor_verts[idx]; }

  /**
   * Return the object space vector for the vertex.
   */
  csVector3& Vobj (int idx) { return obj_verts[idx]; }

  /**
   * Return the camera space vector for the vertex.
   */
  csVector3& Vcam (int idx) { return cam_verts[idx]; }

  /**
   * Return the number of vertices.
   */
  int GetNumVertices () { return num_vertices; }

  /**
   * Add a polygon to this polygonset.
   */
  void AddPolygon (csPolygonInt* spoly);

  /**
   * Create a new polygon in this polygonset and add it.
   */
  csPolygon3D* NewPolygon (csTextureHandle* texture);

  /**
   * Get the number of polygons in this polygonset.
   */
  int GetNumPolygons () { return num_polygon; }

  /**
   * Get the specified polygon from this set.
   */
  csPolygonInt* GetPolygon (int idx) { return polygons[idx]; }

  /**
   * Get the named polygon from this set.
   */
  csPolygon3D* GetPolygon (char* name);

  /**
   * Add a curve to this polygonset.
   */
  void AddCurve (csCurve* curve);

  /**
   * Get the number of curves in this polygonset.
   */
  int GetNumCurves () { return num_curves; }

  /**
   * Get the specified polygon from this set.
   */
  csCurve* GetCurve (int idx) { return curves[idx]; }

  /**
   * Get the named polygon from this set.
   */
  csCurve* GetCurve (char* name);

  /**
   * Get the number of curve vertices.
   */
  int GetNumCurveVertices () { return num_curve_vertices; }

  /**
   * Get the specified curve vertex.
   */
  csVector3& CurveVertex (int i) { return curve_vertices[i]; }

  /**
   * Get the specified curve texture coordinate (texel).
   */
  csVector2& CurveTexel (int i) { return curve_texels[i]; }

  /**
   * Add a curve vertex.
   */
  void AddCurveVertex (csVector3& v, csVector2& t);

  /**
   * Enable the optional BSP tree. This should be done AFTER
   * all polygons and vertices have been added and all polygons
   * are correctly initialized (textures and planes).
   */
  void UseBSP ();

  /**
   * Return true if there is a BSP tree used in this sector.
   */
  bool IsBSP () { return bsp != NULL; }

  /**
   * Intersect world-space segment with polygons of this set. Return
   * polygon it intersects with (or NULL) and the intersection point
   * in world coordinates.<p>
   *
   * If 'pr' != NULL it will also return a value between 0 and 1
   * indicating where on the 'start'-'end' vector the intersection
   * happened.
   */
  virtual csPolygon3D* IntersectSegment (const csVector3& start, 
                                       const csVector3& end, csVector3& isect,
				       float* pr = NULL);

  /**
   * Transform all polygon vertices in this polygonset from world
   * to camera space (don't transform curve vertices).
   * Return false if none of the vertices of this sector is in front
   * of the camera.
   */
  bool TransformWorld2Cam (csCamera& c);

  /**
   * Translate the vertices of this polygonset so that the given
   * vector is at the origin. The resulting transformation is
   * put in the 'camera' section. This function is similar to
   * TransformWorld2Cam() except that there is no rotation of the
   * camera, only a translation.
   */
  void TranslateVector (csVector3& trans);

  /**
   * Get the next polygonset in its linked list.
   */
  csPolygonSet* GetNext () { return next; }

  /**
   * Set the next polygonset in its linked list.
   */
  void SetNext (csPolygonSet* next) { csPolygonSet::next = next; }

  /**
   * Set the sector that this polygonset belongs to.
   * This is either the polygonset itself if it is a sector
   * or else the sector that this thing is in.
   */
  void SetSector (csSector* sector) { csPolygonSet::sector = sector; }

  /**
   * Return the sector that this polygonset belongs to.
   */
  csSector* GetSector () { return sector; }

  /**
   * Return true if this has fog.
   */
  bool HasFog () { return fog.enabled; }

  /**
   * Return fog structure.
   */
  csFog& GetFog () { return fog; }

  /**
   * Find the minimum and maximum Z values of all vertices in
   * this polygon set (in camera space). This is used for planed fog.
   */
  void GetCameraMinMaxZ (float& minz, float& mazx);

  /**
   * Intersect this polygon set in camera space with a polygon which
   * coincides with plane Zc = <value> (a plane parallel to the view
   * plane) and return a new polygon which is the intersection (as a 2D
   * polygon with z not given). This function assumes that the
   * polygon set is convex (so it can in general not be used for polygon
   * sets which use a BSP tree) and gives unexpected results otherwise.
   * Delete the returned polygon with 'delete []' when ready.
   */
  csVector2* IntersectCameraZPlane (float z, csVector2* clipper, int num_clip, int& num_pts);

  CSOBJTYPE;

  DECLARE_INTERFACE_TABLE( PolygonSet )
  DECLARE_IUNKNOWN()

  DECLARE_COMPOSITE_INTERFACE( PolygonSet )
};

#define GetIPolygonSetFromcsPolygonSet(a)  &a->m_xPolygonSet;
#define GetcsPolygonSetFromIPolygonSet(a)  ((csPolygonSet*)((size_t)a - offsetof(csPolygonSet, m_xPolygonSet)))

#endif /*POLYSET_H*/
