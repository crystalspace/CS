/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __CS_REGION_H__
#define __CS_REGION_H__

#include "csobject/csobject.h"
#include "iengine/region.h"

class csEngine;

/**
 * A region. A region is basically a collection of objects in the
 * 3D engine that can be treated as a unit.
 */
class csRegion : public csObjectNoDel
{
  friend class Dumper;

private:
  csEngine* engine;

public:
  /**
   * Initialize an empty region.
   */
  csRegion (csEngine*);

  /**
   * Delete the region without deleting the entities in it. The entities
   * in this region will simply get unconnected.
   */
  virtual ~csRegion ();

  /**
   * Add an object to this region.
   */
  void AddToRegion (csObject* obj);

  /**
   * Release an object from this region.
   */
  void ReleaseFromRegion (csObject* obj);

  /**
   * Find an object of a given name and type.
   * If 'derived' == true it will also consider objects
   * which are of derived types of the given type.
   */
  csObject* FindObject (const char* iName, const csIdType& type,
      	bool derived = false);

  /**
   * Check if some object is in this region.
   * The speed of this function is independent of the number of
   * objects in this region (i.e. very fast).
   */
  virtual bool IsInRegion (iObject* obj);

  CSOBJTYPE;
  DECLARE_IBASE;

  //--------------------- iRegion implementation ---------------------
  struct Region : public iRegion
  {
    DECLARE_EMBEDDED_IBASE (csRegion);

    /**
     * Clear this region without removing the entities in it. The entities
     * will simply get unconnected from this region.
     */
    virtual void Clear ();

    /**
     * Delete all entities in this region.
     */
    virtual void DeleteAll ();

    /**
     * Prepare all textures and materials in this region.
     */
    virtual bool PrepareTextures ();

    /**
     * Do lighting calculations (or read from cache).
     */
    virtual bool ShineLights ();

    /**
     * Prepare all objects in this region. This has to be called
     * directly after loading new objects.
     * This function is equivalent to calling PrepareTextures(),
     * followed by ShineLights().
     */
    virtual bool Prepare ();

    /// Find a sector in this region by name.
    virtual iSector *FindSector (const char *iName);
    /// Find a mesh in this region by name
    virtual iMeshWrapper *FindMeshObject (const char *iName);
    /// Find a mesh factory in this region by name
    virtual iMeshFactoryWrapper *FindMeshFactory (const char *iName);
    /// Find a texture in this region by name
    /// Find a terrain in this region by name
    virtual iTerrainWrapper *FindTerrainObject (const char *iName);
    /// Find a terrain factory in this region by name
    virtual iTerrainFactoryWrapper *FindTerrainFactory (const char *iName);
    virtual iTextureWrapper *FindTexture (const char *iName);
    /// Find a material in this region by name
    virtual iMaterialWrapper *FindMaterial (const char *iName);
    /// Find a camera position in this region by name
    virtual iCameraPosition *FindCameraPosition (const char *iName);

    /**
     * Check if some object is in this region.
     * The speed of this function is independent of the number of
     * objects in this region (i.e. very fast).
     */
    virtual bool IsInRegion (iObject* obj);
  } scfiRegion;
  friend struct Region;
};

#endif // __CS_REGION_H__
