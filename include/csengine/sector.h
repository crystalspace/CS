/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_SECTOR_H__
#define __CS_SECTOR_H__

#include "csobject/pobject.h"
#include "csobject/nobjvec.h"
#include "csengine/rview.h"
#include "csengine/bsp.h"
#include "csgeom/math3d.h"
#include "csutil/cscolor.h"
#include "isector.h"

struct LightInfo;
class csThing;
class csEngine;
class csStatLight;
class csMeshWrapper;
class csPolygon3D;
class csCollection;
class csCamera;
class csTerrain;
class csDynLight;
class Dumper;
class csPolygon2DQueue;
class csProgressPulse;
struct iGraphics3D;
struct iStatLight;
struct iVisibilityCuller;

/**
 * A sector is a container for objects. It is one of
 * the base classes for the portal engine.
 */
class csSector : public csPObject
{
  friend class Dumper;

private:
  /**
   * List of meshes in this sector. Note that meshes also
   * need to be in the engine list. This vector contains objects
   * of type csMeshWrapper*.
   */
  csVector meshes;

  /**
   * List of things in this sector.
   */
  csVector things;

  /**
   * List of collections in this sector.
   */
  csVector collections;

  /**
   * List of sky objects in this sector.
   */
  csVector skies;

  /**
   * All static and pseudo-dynamic lights in this sector.
   * This vector contains objects of type csStatLight*.
   */
  csNamedObjVector lights;

  /**
   * List of terrain objects in this sector. This vector
   * contains objects of type csTerrain*.
   */
  csNamedObjVector terrains;

  /// Engine handle.
  csEngine* engine;

  /// Fog information.
  csFog fog;

  /// Ambient light level for red in this sector.
  int level_r;
  /// Ambient light level for green in this sector.
  int level_g;
  /// Ambient light level for blue in this sector.
  int level_b;

  ///
  int visited;

  /**
   * If static_tree is not NULL then this is a pointer to the csThing
   * which holds all polygons of the non-moving csThings.
   */
  csThing* static_thing;

  /**
   * The visibility culler for this sector or NULL if none.
   * In future we should support more than one visibility culler probably.
   */
  iVisibilityCuller* culler;

  /**
   * This function is called by the BSP tree traversal routine
   * to update the lighting for a number of polygons.
   */
  static void* CheckFrustumPolygons (csSector*, csPolygonInt** polygon,
  	int num, void* data);

public:
  /**
   * How many times are we busy drawing this sector (recursive).
   * This is an important variable as it indicates to
   * 'new_transformation' which set of camera vertices it should
   * use.
   */
  int draw_busy;

  /**
   * Option variable: render portals?
   * If this variable is false portals are rendered as a solid polygon.
   */
  static bool do_portals;

  /**
   * Option variable: render things?
   * If this variable is false meshes and things are not rendered.
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
  csSector (csEngine*);

  /**
   * Destroy this sector. All things in this sector are also destroyed.
   * Meshes are unlinked from the sector but not removed because they
   * could be in other sectors.
   */
  virtual ~csSector ();

  /**
   * Prepare all polygons for use. This function MUST be called
   * AFTER the texture manager has been prepared. This function
   * is normally called by csEngine::Prepare() so you only need
   * to worry about this function when you add sectors later.
   */
  virtual void Prepare (csSector* sector);

  /// Get the engine for this sector.
  csEngine* GetEngine () { return engine; }

  //----------------------------------------------------------------------
  /**
   * Add a mesh to this sector and register it to the culler.
   */
  void AddMesh (csMeshWrapper* mesh);

  /**
   * Unlink a mesh from this sector and remove it from the culler.
   */
  void UnlinkMesh (csMeshWrapper* mesh);

  /**
   * Get the number of meshes in this sector.
   */
  int GetNumberMeshes ()
  {
    return meshes.Length ();
  }

  /**
   * Get the specified mesh.
   */
  csMeshWrapper* GetMesh (int idx)
  {
    return (csMeshWrapper*)meshes[idx];
  }

  /**
   * Find the given mesh by name.
   */
  csMeshWrapper* GetMesh (const char* name);
  //----------------------------------------------------------------------
  /**
   * Add a thing to this sector and register it to the culler.
   */
  void AddThing (csThing* thing);

  /**
   * Unlink a thing from this sector and remove it from the culler.
   */
  void UnlinkThing (csThing* thing);

  /**
   * Get the number of things in this sector.
   */
  int GetNumThings ()
  {
    return things.Length ();
  }

  /**
   * Get the specified thing.
   */
  csThing* GetThing (int idx)
  {
    return (csThing*)things[idx];
  }

  /**
   * Find a thing with the given name.
   */
  csThing* GetThing (const char* name);
  //----------------------------------------------------------------------
  /**
   * Add a sky thing to this sector and register it to the culler.
   */
  void AddSky (csThing* thing);

  /**
   * Unlink a sky thing from this sector and remove it from the culler.
   */
  void UnlinkSky (csThing* thing);

  /**
   * Get the number of skies in this sector.
   */
  int GetNumberSkies ()
  {
    return skies.Length ();
  }

  /**
   * Get the specified sky.
   */
  csThing* GetSky (int idx)
  {
    return (csThing*)skies[idx];
  }

  /**
   * Find a sky thing with the given name.
   */
  csThing* GetSky (const char* name);
  //----------------------------------------------------------------------
  /**
   * Add a collection to this sector.
   */
  void AddCollection (csCollection* col);

  /**
   * Unlink a collection from this sector.
   */
  void UnlinkCollection (csCollection* col);

  /**
   * Get the number of collections in this sector.
   */
  int GetNumberCollections ()
  {
    return collections.Length ();
  }

  /**
   * Get the specified collection.
   */
  csCollection* GetCollection (int idx)
  {
    return (csCollection*)collections[idx];
  }

  /**
   * Find a collection with the given name.
   */
  csCollection* GetCollection (const char* name);
  //----------------------------------------------------------------------
  /**
   * Add a static or pseudo-dynamic light to this sector.
   */
  void AddLight (csStatLight* light);

  /**
   * Unlink a light from this sector.
   */
  void UnlinkLight (csStatLight* light);

  /**
   * Get the number of lights in this sector.
   */
  int GetNumberLights ()
  {
    return lights.Length ();
  }

  /**
   * Get the specified light.
   */
  csStatLight* GetLight (int idx)
  {
    return (csStatLight*)lights[idx];
  }

  /**
   * Find a light with the given name.
   */
  csStatLight* GetLight (const char* name)
  {
    return (csStatLight*)lights.FindByName (name);
  }

  /**
   * Find a light with the given position and radius.
   */
  csStatLight* FindLight (float x, float y, float z, float dist);

  /**
   * Find the light with the given object id.
   */
  csStatLight* FindLight (CS_ID id);
  //----------------------------------------------------------------------
  /**
   * Add a terrain to this sector.
   */
  void AddTerrain (csTerrain* terrain);

  /**
   * Unlink a terrain from this sector.
   */
  void UnlinkTerrain (csTerrain* col);

  /**
   * Get the number of terrains in this sector.
   */
  int GetNumberTerrains ()
  {
    return terrains.Length ();
  }

  /**
   * Get the specified terrain.
   */
  csTerrain* GetTerrain (int idx)
  {
    return (csTerrain*)terrains[idx];
  }

  /**
   * Find a terrain with the given name.
   */
  csTerrain* GetTerrains (const char* name)
  {
    return (csTerrain*)terrains.FindByName (name);
  }
  //----------------------------------------------------------------------

  /**
   * Get the thing representing all non-moving things (all
   * things which are moved into this thing will return IsMerged()==true).
   */
  csThing* GetStaticThing () { return static_thing; }

  /**
   * Get the visibility culler that is used for this sector.
   * NULL if none.
   */
  iVisibilityCuller* GetVisibilityCuller () { return culler; }

  /**
   * Call this function to generate a polygon tree for all csThings
   * in this sector which are marked as needing such a tree.
   * If 'octree' is true this function will create an octree with mini-bsp
   * trees instead of a BSP tree alone.
   * Note that at this moment only one thing with such a tree will be
   * properly supported.
   */
  void UseStaticTree (int mode = BSP_MINIMIZE_SPLITS, bool octree = false);

  /**
   * Get ambient color valid in this sector.
   */
  void GetAmbientColor (int& r, int& g, int& b)
  { r = level_r; g = level_g; b = level_b; }

  /**
   * Set the ambient color for this sector. This is only useful
   * before lighting is calculated.
   */
  void SetAmbientColor (int r, int g, int b)
  { level_r = r; level_g = g; level_b = b; }

  /// Return true if this has fog.
  bool HasFog () { return fog.enabled; }

  /// Return fog structure.
  csFog& GetFog () { return fog; }

  /// Conveniance function to set fog to some setting.
  void SetFog (float density, const csColor& color)
  {
    fog.enabled = true;
    fog.density = density;
    fog.red = color.red;
    fog.green = color.green;
    fog.blue = color.blue;
  }

  /// Disable fog.
  void DisableFog () { fog.enabled = false; }

  /**
   * Follow a beam from start to end and return the first polygon that
   * is hit. This function correctly traverse portals and space warping
   * portals. Normally the sector you call this on should be the sector
   * containing the 'start' point. 'isect' will be the intersection point
   * if a polygon is returned.
   */
  csPolygon3D* HitBeam (const csVector3& start, const csVector3& end,
  	csVector3& isect);

  /**
   * Follow a beam from start to end and return the first object
   * that is hit. Objects can be meshes, things, or sectors.
   * In case it is a thing or sector the csPolygon3D field will be
   * filled with the polygon that was hit.
   * If polygonPtr is null then the polygon will not be filled in.
   */
  csObject* HitBeam (const csVector3& start, const csVector3& end,
  	csPolygon3D** polygonPtr);

  /**
   * Prepare the lightmaps so that they are suitable for the
   * 3D rasterizer.
   */
  void CreateLightMaps (iGraphics3D* g3d);

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
  void InitLightMaps (bool do_cache = true);

  /**
   * Check visibility in a frustum way for all things and polygons in
   * this sector and possibly traverse through portals to other sectors.
   * This version doesn't init the 2D culler cube so it can be used
   * for recursing.
   */
  void RealCheckFrustum (csFrustumView& lview);

  /**
   * Check visibility in a frustum way for all things and polygons in
   * this sector and possibly traverse through portals to other sectors.
   */
  void CheckFrustum (csFrustumView& lview);

  /**
   * Get a list of all things which are visible in the given frustum.
   * Return an array to pointers to visible things.
   * You must delete this array after you are ready using it.
   */
  csThing** GetVisibleThings (csFrustumView& lview, int& num_things);

  /**
   * Cache the lightmaps for all polygons in this sector.
   * The lightmaps will be cached to the current level file
   * (if it is an archive) or else to 'precalc.zip'.
   */
  void CacheLightMaps ();

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
  csPolygon3D* IntersectSphere (csVector3& center, float radius,
                               float* pr = NULL);

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

  /**
   * Calculate the bounding box of all objects in this sector.
   * This function is not very efficient as it will traverse all objects
   * in the sector one by one and compute a bounding box from that.
   */
  void CalculateSectorBBox (csBox3& bbox, bool do_things, bool do_meshes,
  	bool do_terrain);

  //------------------------------------------------
  // Everything for setting up the lighting system.
  //------------------------------------------------

  /**
   * The whole setup starts with csEngine::shine_lights calling
   * csSector::shine_lights for every sector in the engine.
   * This function will call csStatLight::shine_lightmaps for every
   * light in the sector.
   * csStatLight::shine_light will generate a view frustum from the
   * center of the light and use that to light all polygons that
   * are hit by the frustum.
   */
  void ShineLights (csProgressPulse* = 0);

  /// Version of shine_lights() which only affects one thing.
  void ShineLights (csThing*, csProgressPulse* = 0);

  CSOBJTYPE;
  DECLARE_IBASE_EXT (csPObject);

  //------------------------- iSector interface -------------------------------
  struct eiSector : public iSector
  {
    DECLARE_EMBEDDED_IBASE (csSector);

    virtual csSector *GetPrivateObject ()
    { return scfParent; }
    virtual void CreateBSP ()
    { scfParent->UseStaticTree (); }
    virtual iVisibilityCuller* GetVisibilityCuller ()
    {
      return scfParent->GetVisibilityCuller ();
    }
    virtual iThing *GetSkyThing (const char *name);
    virtual int GetNumSkyThings ()
    { return scfParent->skies.Length (); }
    virtual iThing *GetSkyThing (int iIndex);
    virtual iThing *GetThing (const char *name);
    virtual int GetNumThings ()
    { return scfParent->things.Length (); }
    virtual iThing *GetThing (int iIndex);
    virtual void AddLight (iStatLight *light);
    virtual iStatLight *FindLight (float x, float y, float z, float dist);
    virtual void CalculateSectorBBox (csBox3& bbox, bool do_things, bool do_meshes,
  	bool do_terrain)
    {
      scfParent->CalculateSectorBBox (bbox, do_things, do_meshes, do_terrain);
    }
  } scfiSector;
  friend struct eiSector;
};

#endif // __CS_SECTOR_H__
