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
#include "csengine/rview.h"

class csSector;
class csStatLight;
class csTextureHandle;
class csPolygon3D;
class CLights;
class csThingTemplate;
interface IGraphics3D;

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
  /// Where is this thing located?
  csSector* home;
  /// OBSOLETE.
  csSector* other;
  /// World to object transformation.
  csReversibleTransform obj;

  /// Set of flags
  ULong flags;

  /// If convex, this holds the index to the center vertex.
  int center_idx;

  /**
   * If not NULL this Thing has been merged into the given Thing.
   * In this case this Thing is not responsible for cleaning it's own polygons.
   */
  csThing* merged;

public:
  /**
   * Create an empty thing.
   */
  csThing ();

  /// Destructor.
  virtual ~csThing ();

  /// Set all flags with the given mask.
  void SetFlags (ULong mask, ULong value) { flags = (flags & ~mask) | value; }

  /// Get flags.
  ULong GetFlags () { return flags; }

  /// Check if all the given flags are set.
  bool CheckFlags (ULong to_check) { return (flags & to_check) != 0; }

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
   * Return true if this Thing has been merged into another Thing.
   * This happens when there is a static BSP associated with the
   * parent sector which holds all non-moving things.
   */
  bool IsMerged () { return !!merged; }

  /**
   * Merge the given Thing into this one. The other Thing is marked
   * as being merged. Note that the polygons of the other Thing will
   * not be copied but only a reference is copied. This Thing will
   * become the new owner of the polygons and the other Thing may
   * not remove them.<br>
   * Warning! All polygons from the other Thing will be modified so that
   * they refer to this new Thing. This means that the vertex array
   * of the original Thing will not be used anymore (@@@ Maybe clean this
   * up?).<br>
   * Warning! All Things are merged in world space coordinates and not
   * in object space as one could expect!
   */
  void Merge (csThing* other);

  /**
   * Set the transformation vector and sector to move thing to
   * some position.
   */
  void SetMove (csSector* home, csVector3& v) { SetMove (home, v.x, v.y, v.z); }

  /**
   * Set the transformation vector and sector to move thing to
   * some position.
   */
  void SetMove (csSector* home, float x, float y, float z);

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
   * Relative move.
   */
  void Move (float dx, float dy, float dz);

  /**
   * Relative move.
   */
  void Move (csVector3& v) { Move (v.x, v.y, v.z); }

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
   * Create all mipmaps for all textures for all polygons of
   * this thing.
   */
  void CreateLightmaps (IGraphics3D* g3d);

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
  void InitLightmaps (bool do_cache = true);

  /**
   * Update all lighting on this thing for the given light.
   */
  void CalculateLighting (csLightView& lview);

  /**
   * Cache the lightmaps for all polygons in this thing.
   */
  void CacheLightmaps ();

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
  void MergeTemplate (csThingTemplate* tpl, csTextureHandle* default_texture = NULL,
  	float default_texlen = 1, CLights* default_lightx = NULL,
	csVector3* shift = NULL, csMatrix3* transform = NULL);


  CSOBJTYPE;
};

#endif /*THING_H*/
