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

#ifndef THING_H
#define THING_H

#include "csgeom/transfrm.h"
#include "csengine/polyset.h"
#include "csengine/bspbbox.h"
#include "csengine/rview.h"
#include "csutil/flags.h"
#include "ithing.h"

class csSector;
class csWorld;
class csStatLight;
class csMaterialWrapper;
class csMaterialList;
class csPolygon3D;
class CLights;
class csThingTemplate;
struct iGraphics3D;

/**
 * If CS_ENTITY_CONVEX is set then this entity is convex (what did
 * you expect :-)
 * This means the 3D engine can do various optimizations.
 * If you set 'convex' to true the center vertex will also be calculated.
 * It is unset by default (@@@ should be calculated).
 */
#define CS_ENTITY_CONVEX 1

/**
 * If CS_ENTITY_MOVEABLE is set then this entity can move.
 * This is used by several optimizations in the 3D Engine.
 * If the Engine knows that a Thing cannot move it can put
 * it's polygons in a BSP tree for example.
 * This flag is false by default (it is assumed that most Things
 * will not move).
 */
#define CS_ENTITY_MOVEABLE 2

/**
 * If CS_ENTITY_DETAIL is set then this entity is a detail
 * object. A detail object is treated as a single object by
 * the engine. The engine can do several optimizations on this.
 * In general you should use this flag for small and detailed
 * objects.
 */
#define CS_ENTITY_DETAIL 4

/**
 * If CS_ENTITY_CAMERA is set then this entity will be always
 * be centerer around the same spot relative to the camera. This
 * is useful for skyboxes or skydomes.
 */
#define CS_ENTITY_CAMERA 8

/**
 * A Thing is a polygonset just like a csSector.
 * It can be used to augment the sectors with features that
 * are difficult to describe using portals and sectors.<P>
 *
 * Things can also be used for volumetric fog. In that case
 * the Thing must be convex.
 */
class csThing : public csPolygonSet
{
private:
  /// World to object transformation.
  csReversibleTransform obj;

  /// If convex, this holds the index to the center vertex.
  int center_idx;

  /// Pointer to the Thing Template which it derived from
  csThingTemplate* ParentTemplate;

  /**
   * Utility function to be called whenever obj changes which updates
   * object to world transform in all of the curves
   */
  void UpdateCurveTransform ();

  /// Internal draw function.
  void DrawInt (csRenderView& rview, bool use_z_buf);

  /// Bounding box for polygon trees.
  csPolyTreeBBox tree_bbox;

  /// If true this thing is visible.
  bool is_visible;

  /**
   * Update this thing in the polygon trees.
   */
  void UpdateInPolygonTrees ();

public:
  /// Set of flags
  csFlags flags;

public:
  /**
   * Create an empty thing.
   */
  csThing (csWorld* world);

  /// Destructor.
  virtual ~csThing ();

  /// Set the sector for this thing.
  virtual void SetSector (csSector* sector)
  {
    csPolygonSet::SetSector (sector);
    UpdateInPolygonTrees ();
  }

  /**
   * Set convexity flag of this thing. You should call this instead
   * of SetFlags (CS_ENTITY_CONVEX, CS_ENTITY_CONVEX) because this function
   * does some extra calculations.
   */
  void SetConvex (bool c);

  /**
   * If this thing is convex you can use getCenter to get the index
   * of the vertex holding the center of this thing. This center is
   * calculated by 'setConvex(true)'.
   */
  int GetCenter () { return center_idx; }

  /**
   * Do some initialization needed for visibility testing.
   * i.e. clear camera transformation.
   */
  void VisTestReset ()
  {
    tree_bbox.ClearTransform ();
  }

  /// Mark this thing as visible.
  void MarkVisible () { is_visible = true; }

  /// Mark this thing as invisible.
  void MarkInvisible () { is_visible = false; }

  /// Return if this thing is visible.
  bool IsVisible () { return is_visible; }

  /// Get the pointer to the object to place in the polygon tree.
  csPolyTreeObject* GetPolyTreeObject ()
  {
    return &tree_bbox;
  }

  /**
   * Merge the given Thing into this one. The other polygons and
   * curves are removed from the other thing so that it is ready to
   * be removed. Warning! All Things are merged in world space
   * coordinates and not in object space as one could expect!
   */
  void Merge (csThing* other);

  /**
   * Set the transformation vector and sector to move thing to
   * some position.
   */
  void SetPosition (csSector* home, const csVector3& v);

  /**
   * Set the transformation matrix to rotate the thing in some
   * orientation.
   */
  void SetTransform (const csMatrix3& matrix);

  /**
   * Set the world to object tranformation.
   */
  void SetTransform (const csReversibleTransform& t) { obj = t; }

  /**
   * Get the world to object tranformation.
   */
  csReversibleTransform& GetTransform () { return obj; }

  /**
   * Relative move.
   */
  void MovePosition (const csVector3& v);

  /**
   * Relative transform.
   */
  void Transform (csMatrix3& matrix);

  /**
   * Really do the transformation. This should be called
   * after calling any of the set_move, set_transform, move or
   * transform functions. Ideally it should only be called
   * when the thing is visible.
   */
  void Transform ();

  /**
   * Prepare the lightmaps for all polys so that they are suitable
   * for the 3D rasterizer.
   */
  void CreateLightMaps (iGraphics3D* g3d);

  /**
   * Draw this thing given a view and transformation.
   */
  void Draw (csRenderView& rview, bool use_z_buf = true);

  /**
   * Draw all curves in this thing given a view and transformation.
   */
  void DrawCurves (csRenderView& rview, bool use_z_buf = true);

  /**
   * Draw this thing as a fog volume (only when fog is enabled for
   * this thing).
   */
  void DrawFoggy (csRenderView& rview);

  /**
   * Init the lightmaps for all polygons in this thing.
   */
  void InitLightMaps (bool do_cache = true);

  /**
   * Check frustum visibility on this thing.
   */
  void RealCheckFrustum (csFrustumView& lview);

  /**
   * Check frustum visibility on this thing.
   * First initialize the 2D culler cube.
   */
  void CheckFrustum (csFrustumView& lview);

  /**
   * Cache the lightmaps for all polygons in this thing.
   */
  void CacheLightMaps ();

  /**
   * Intersects world-space sphere with polygons of this set. Return
   * polygon it hits with (or NULL) and the intersection point
   * in world coordinates. It will also return the polygon with the
   * closest hit (the most nearby polygon).
   * If 'pr' != NULL it will also return the distance where the
   * intersection happened.
   */
  csPolygon3D* IntersectSphere (csVector3& center, float radius, float* pr = NULL);

  /**
   * Add polygons and vertices from the specified template.
   */
  void MergeTemplate (csThingTemplate* tpl, csMaterialWrapper* default_material = NULL,
  	float default_texlen = 1, CLights* default_lightx = NULL,
	csVector3* shift = NULL, csMatrix3* transform = NULL);

  /**
   * Add polygons and vertices from the specified template. Replace the materials if they 
   * match one in the matList.
   */
  void MergeTemplate (csThingTemplate* tpl, csMaterialList* matList, const char* prefix, 
	csMaterialWrapper* default_material = NULL,
  	float default_texlen = 1, CLights* default_lightx = NULL,
	csVector3* shift = NULL, csMatrix3* transform = NULL);


  /// Set parent template
  void SetTemplate (csThingTemplate *t)
  { ParentTemplate = t; }

  /// Query parent template
  csThingTemplate *GetTemplate () const
  { return ParentTemplate; }

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csPolygonSet);

  //------------------------- iSector interface --------------------------------
  struct eiThing : public iThing
  {
    DECLARE_EMBEDDED_IBASE (csThing);

    /// Used by the engine to retrieve internal sector object (ugly)
    virtual csThing *GetPrivateObject ()
    { return scfParent; }

    /// Set the position of the thing
    virtual void SetPosition (const csVector3 &iPos);
    /// Set the sector of the thing
    virtual void SetPosition (iSector *iSec);
    /// Set the transformation matrix to rotate the thing in some orientation.
    virtual void SetTransform (const csMatrix3 &iMatrix)
    { scfParent->SetTransform (iMatrix); }
  } scfiThing;
  friend struct eiThing;
};

#endif /*THING_H*/
