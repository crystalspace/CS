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

#ifndef SECTOR_H
#define SECTOR_H

#include "csengine/basic/csobjvec.h"
#include "csengine/basic/polyset.h"
#include "csgeom/math3d.h"
#include "csengine/rview.h"

struct LightInfo;
class csThing;
class csStatLight;
class csPolygon3D;
class Polygon2D;
class csCamera;
class csDynLight;
class csSprite3D;
class Dumper;
class csBspTree;
interface IGraphics3D;

/**
 * A sector is a convex hull of polygons. It is
 * one of the base classes for the portal engine.
 */
class csSector : public csPolygonSet
{
  friend class Dumper;

public:
  /**
   * List of sprites in this sector. Note that sprites also
   * need to be in the world list. This vector contains objects
   * of type csSprite3D*.
   */
  csObjVector sprites;

  /**
   * All static and pseudo-dynamic lights in this sector.
   * This vector contains objects of type csStatLight*.
   */
  csObjVector lights;

private:
  /// First thing in this sector.
  csThing* first_thing;

  /// Ambient light level for red in this sector.
  int level_r;
  /// Ambient light level for green in this sector.
  int level_g;
  /// Ambient light level for blue in this sector.
  int level_b;

  ///
  int visited;

  /**
   * If this variable is not NULL then it is a BSP tree in this
   * sector which includes all static (non-moving) csThings.
   */
  csBspTree* static_bsp;

  /**
   * If static_bsp is not NULL then this is a pointer to the csThing
   * which holds all polygons of the non-moving csThings.
   */
  csThing* static_thing;

  /**
   * This function is called by the BSP tree traversal routine
   * to draw a number of polygons.
   */
  static void* DrawPolygons (csPolygonParentInt*, csPolygonInt** polygon,
  	int num, void* data);

  /**
  @@@ OBSOLETE
   * This function is called by the BSP tree traversal routine
   * to update the lightmaps for a number of polygons.
   */
  static void* ShinePolygons (csPolygonParentInt*, csPolygonInt** polygon,
  	int num, void* data);

  /**
   * This function is called by the BSP tree traversal routine
   * to update the lightmaps for a number of polygons.
   */
  static void* CalculateLightmapsPolygons (csPolygonParentInt*, csPolygonInt** polygon,
  	int num, void* data);

  /**
   * This function is called by the BSP tree traversal routine
   * to dump the frustrum (debugging).
   */
  static void* DumpFrustrumPolygons (csPolygonParentInt*, csPolygonInt** polygon,
  	int num, void* data);

public:
  /**
   * Option variable: render portals?
   * If this variable is false portals are rendered as a solid polygon.
   */
  static bool do_portals;

  /**
   * Option variable: render things?
   * If this variable is false sprites and things are not rendered.
   */
  static bool do_things;

  /**
   * Configuration variable: number of allowed reflections for static lighting.
   * This option controls how many time a given sector may be visited by the
   * same beam of light. When this value is 1 it means that light is not
   * reflected.
   */
  static int cfg_reflections;

  /**
   * Option variable: do pseudo-radiosity?
   * When pseudo-radiosity is enabled every polygon behaves as if
   * it is a mirroring portal when lighting calculations are concerned.
   * This simulates radiosity because light reflects from every surface.
   * The number of reflections allowed is controlled by cfg_reflections.
   */
  static bool do_radiosity;

  /// How many times are we shining a specific light through this sector.
  int beam_busy;

  /**
   * Construct a sector. This sector will be completely empty.
   */
  csSector ();

  /**
   * Destroy this sector. All things in this sector are also destroyed.
   * Sprites are unlinked from the sector but not removed because they
   * could be in other sectors.
   */
  virtual ~csSector ();

  /**
   * Prepare all polygons for use. This function MUST be called
   * AFTER the texture manager has been prepared. This function
   * is normally called by csWorld::Prepare() so you only need
   * to worry about this function when you add sectors later.
   */
  virtual void Prepare ();

  /**
   * Add a thing to this sector.
   */
  void AddThing (csThing* thing);

  /**
   * Add a static or pseudo-dynamic light to this sector.
   */
  void AddLight (csStatLight* light);

  /**
   * Find a light with the given position and radius.
   */
  csStatLight* FindLight (float x, float y, float z, float dist);

  /**
   * Find a thing with the given name.
   */
  csThing* GetThing (const char* name);

  /**
   * Get the first thing in this sector.
   */
  csThing* GetFirstThing () { return first_thing; }

  /**
   * Call this function to generate a BSP tree for all csThings
   * in this sector. This might make drawing more efficient because
   * those things can then be drawn using Z-fill instead of Z-buffer.
   * This function will only generate BSP trees for the csThings
   * which cannot move. Note that you can no longer remove a csThing
   * from the sector if it has been added to the static BSP tree.
   */
  void UseStaticBSP ();

  /**
   * Get ambient color valid in this sector.
   */
  void GetAmbientColor (int& r, int& g, int& b) { r = level_r; g = level_g; b = level_b; }

  /**
   * Set the ambient color for this sector. This is only useful
   * before lighting is calculated.
   */
  void SetAmbientColor (int r, int g, int b) { level_r = r; level_g = g; level_b = b; }

  /**
   * Follow a beam from start to end and return the first polygon that
   * is hit. This function correctly traverse portals but not space
   * warping portals.
   */
  csPolygon3D* HitBeam (csVector3& start, csVector3& end);

  /**
   * Given the first lightmap as calculated by the lighting
   * routines create the lightmaps for the other mipmap levels and
   * also convert the lightmaps so that they are suitable for the
   * 3D rasterizer.
   */
  void CreateLightmaps (IGraphics3D* g3d);

  /**
   * Draw the sector in the given view and with the given transformation.
   */
  void Draw (csRenderView& rview);

  /**
   * Init the lightmaps for all polygons in this sector. If this
   * routine can find them in the cache it will load them, otherwise
   * it will prepare the lightmap for the lighting routines.
   * If do_cache == false this function will not try to read from
   * the cache.
   */
  void InitLightmaps (bool do_cache = true);

  /**
   * Update the lightmaps for all things and polygons in this sector
   * and possibly traverse through portals to other sectors.
   */
  void CalculateLightmaps (csLightView& lview);

  /**
   * Update the lightmaps for all polygons in this sector and possibly
   * traverse through portals to other sectors.
   * If lview.frustrum == NULL it is equivalent to full view (everything is
   * visible).
   */
  void ShineLightmaps (csLightView& lview);

  /**
   * This is a debugging function that will show the outlines
   * on all polygons that are hit by a light in this sector.
   * It will draw perspective correct outlines so it is meant to
   * be called with a camera transformation.
   * @@@ THIS IS A DEBUGGING FUNCTION WHICH SHOULD PERHAPS NOT
   * BE PART OF THE ENGINE!
   */
  void DumpFrustrum (csStatLight* l, csVector3* frustrum, int num_frustrum,
  	csTransform& t);

  /**
   * Cache the lightmaps for all polygons in this sector.
   * The lightmaps will be cached to the current level file
   * (if it is an archive) or else to 'precalc.zip'.
   */
  void CacheLightmaps ();

  /**
   * Intersects world-space sphere with polygons of this set. Return
   * polygon it hits with (or NULL) and the intersection point
   * in world coordinates. It will also return the polygon with the
   * closest hit (the most nearby polygon).
   * If 'pr' != NULL it will also return the distance where the
   * intersection happened.
   * Note. This function correctly accounts for portal polygons
   * and could thus return a polygon not belonging to this sector.
   */
  csPolygon3D* IntersectSphere (csVector3& center, float radius, float* pr = NULL);

  /**
   * Follow a segment starting at this sector. If the segment intersects
   * with a polygon it will stop there unless the polygon is a portal in which
   * case it will recursively go to that sector (possibly applying warping
   * transformations) and continue there.<p>
   *
   * This routine will modify all the given parameters to reflect space warping.
   * These should be used as the new camera transformation when you decide to
   * really go to the new position.<p>
   *
   * This function returns the resulting sector and new_position will be set
   * to the last position that you can go to before hitting a wall.
   */
  csSector* FollowSegment (csReversibleTransform& t, csVector3& new_position, 
                          bool& mirror);

  /**
   * Intersect world-space segment with polygons of this sector. Return
   * polygon it intersects with (or NULL) and the intersection point
   * in world coordinates.<p>
   *
   * If 'pr' != NULL it will also return a value between 0 and 1
   * indicating where on the 'start'-'end' vector the intersection
   * happened.<p>
   *
   * This function is an extension of csPolygonSet::intersect_segment in
   * that it will also test for hits against things.
   */
  virtual csPolygon3D* IntersectSegment (const csVector3& start,
                                       const csVector3& end, csVector3& isect,
				       float* pr = NULL);

  //------------------------------------------------
  // Everything for setting up the lighting system.
  //------------------------------------------------

  /**
   * The whole setup starts with csWorld::shine_lights calling
   * csSector::shine_lights for every sector in the world.
   * This function will call csStatLight::shine_lightmaps for every
   * light in the sector.
   * csStatLight::shine_light will generate a view frustrum from the
   * center of the light and use that to light all polygons that
   * are hit by the frustrum.
   */
  void ShineLights ();

  /// Version of shine_lights() which only affects one thing.
  void ShineLights (csThing* th);

  /**
   * Follow a beam of light starting in this sector at 'start' and
   * ending on the given 'poly' at 'end'. Return true if the polygon
   * is actually reached and also return the squared distance in 'sqdist'
   * where the beam of light hits with the polygon.
   */
  bool HitBeam (csVector3& start, csVector3& end, csPolygon3D* poly, float* sqdist);

  /**
   * Check for csThings along the way. This function assumes that there
   * is valid path from 'start' to 'end' through portals (and it will
   * follow that path). It will return false if there are no csThings
   * blocking the path, otherwise it will return true and put the first polygon
   * that is found to be blocking the beam in 'poly' (only Polygons of csThings
   * are considered).<br>
   * This function also returns true (but with 'poly' set to NULL) if it
   * could not go through a portal before it reached 'end' (there was
   * an error in the path somewhere).
   * If same_sector is true we know that we are looking for a hit in the
   * same sector only.
   */
  bool BlockingThings (csVector3& start, csVector3& end, csPolygon3D** poly, bool same_sector = false);

  /**
   * Follow a beam of light with a given start and end point and return
   * the first polygon that is hit. Also return the squared distance of the
   * path that was followed to get at the polygon (this correctly takes care
   * of mirroring portals and other warping portals).
   */
  csPolygon3D* FollowBeam (csVector3& start, csVector3& end, csPolygon3D* poly, float* sqdist);

private:
  /**
   * BSP function used by follow_beam which does the actual work.
   */
  static void* BeamPolygons (csPolygonParentInt*, csPolygonInt** polygon,
  	int num, void* data);

  /**
   * BSP function used by blocking_things which does the actual work.
   */
  static void* CheckForThings (csPolygonParentInt*, csPolygonInt** polygon,
  	int num, void* data);

  CSOBJTYPE;
};

#endif /*SECTOR_H*/
