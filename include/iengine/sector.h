/*
    Crystal Space 3D engine
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

#ifndef __IENGINE_SECTOR_H__
#define __IENGINE_SECTOR_H__

#include "cstypes.h"
#include "csutil/scf.h"

class csSector;
struct iTerrainWrapper;
struct iMeshWrapper;
struct iThing;
struct iStatLight;
struct iVisibilityCuller;
struct iObject;
struct csFog;
struct iGraphics3D;

SCF_VERSION (iSector, 0, 1, 9);

/**
 * The iSector interface is used to work with "sectors". A "sector"
 * is a convex polyhedron, that possibly contains portals, things,
 * sprites, lights and so on. Simply speaking, a "sector" is analogous
 * to an real-world room, rooms are interconnected with doors and windows
 * (e.g. portals), and rooms contain miscelaneous things, sprites
 * and lights.
 */
struct iSector : public iBase
{
  /// Used by the engine to retrieve internal sector object (ugly)
  virtual csSector *GetPrivateObject () = 0;
  /// Get the iObject for this sector.
  virtual iObject *QueryObject() = 0;

  /// Get the ID of this sector.
  virtual CS_ID GetID () = 0;

  /// Has this sector fog?
  virtual bool HasFog () = 0;
  /// Return the fog structure (even if fog is disabled)
  virtual csFog *GetFog () = 0;
  /// Fill the fog structure with the given values
  virtual void SetFog (float density, const csColor& color) = 0;
  /// Disable fog in this sector
  virtual void DisableFog () = 0;

  /// Return the number of mesh objects in this sector
  virtual int GetMeshCount () = 0;
  /// Return a mesh wrapper by index
  virtual iMeshWrapper *GetMesh (int n) = 0;
  /// Add a mesh object to this sector.
  virtual void AddMesh (iMeshWrapper *pMesh) = 0;
  /// Find a mesh object by name
  virtual iMeshWrapper *FindMesh (const char *name) = 0;
  /// Remove a mesh object
  virtual void RemoveMesh (iMeshWrapper *pMesh) = 0;

  /// Return the number of terrain objects in this sector
  virtual int GetTerrainCount () = 0;
  /// Return a terrain wrapper by index
  virtual iTerrainWrapper *GetTerrain (int n) = 0;
  /// Add a terrain object to this sector.
  virtual void AddTerrain (iTerrainWrapper *pTerrain) = 0;
  /// Find a terrain object by name
  virtual iTerrainWrapper *FindTerrain (const char *name) = 0;
  /// Remove a terrain object
  virtual void RemoveTerrain (iTerrainWrapper *pTerrain) = 0;

  /// Add a static or pseudo-dynamic light to this sector.
  virtual void AddLight (iStatLight *light) = 0;
  /// Find a light with the given position and radius.
  virtual iStatLight *FindLight (float x, float y, float z, float dist) = 0;

  /**
   * Init the lightmaps for all polygons in this sector. If this
   * routine can find them in the cache it will load them, otherwise
   * it will prepare the lightmap for the lighting routines.
   * If do_cache == false this function will not try to read from
   * the cache.
   */
  virtual void InitLightMaps (bool do_cache = true) = 0;
  /// Calculate lighting for all objects in this sector
  virtual void ShineLights () = 0;
  /// Version of ShineLights() which only affects one mesh object.
  virtual void ShineLights (iMeshWrapper*) = 0;
  /**
   * Prepare the lightmaps so that they are suitable for the
   * 3D rasterizer.
   */
  virtual void CreateLightMaps (iGraphics3D* g3d) = 0;
  /**
   * Cache the lightmaps for all polygons in this sector.
   * The lightmaps will be cached to the current level file
   * (if it is an archive) or else to 'precalc.zip'.
   */
  virtual void CacheLightMaps () = 0;

  /**
   * Calculate the bounding box of all objects in this sector.
   * This function is not very efficient as it will traverse all objects
   * in the sector one by one and compute a bounding box from that.
   */
  virtual void CalculateSectorBBox (csBox3& bbox,
  	bool do_meshes, bool do_terrain) = 0;

  /**
   * Use the specified mesh object as the visibility culler for
   * this sector.
   */
  virtual void SetVisibilityCuller (const char *Name) = 0;
  /**
   * Get the visibility culler that is used for this sector.
   * NULL if none.
   */
  virtual iVisibilityCuller* GetVisibilityCuller () = 0;

  /**
   * Get the current draw recursion level.
   */
  virtual int GetRecLevel () = 0;
};

#endif // __IENGINE_SECTOR_H__
