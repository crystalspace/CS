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

#include "csutil/csobject.h"
#include "csutil/nobjvec.h"
#include "iengine/region.h"
#include "isndsys/ss_manager.h"

class csEngine;

/**
 * A region. A region is basically a collection of objects in the
 * 3D engine that can be treated as a unit.
 */
class csRegion : public csObject
{
private:
  csEngine* engine;
  csRef<iSndSysManager> snd_manager;

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
  virtual void Add (iObject *obj)
  { ObjAdd (obj); }

  /**
   * Remove an object from this region.
   */
  virtual void Remove (iObject *obj)
  { ObjRemove (obj); }

  /**
   * Check if some object is in this region.
   * The speed of this function is independent of the number of
   * objects in this region (i.e. very fast).
   */
  virtual bool IsInRegion (iObject* obj);

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

  SCF_DECLARE_IBASE_EXT (csObject);

  //--------------------- iRegion implementation ---------------------
  struct Region : public iRegion
  {
    SCF_DECLARE_EMBEDDED_IBASE (csRegion);

    /// Query the iObject.
    virtual iObject *QueryObject();

    virtual void Add (iObject *obj)
    { scfParent->Add(obj); }
    virtual void Remove (iObject *obj)
    { scfParent->Remove (obj); }

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
    virtual iTextureWrapper *FindTexture (const char *iName);
    /// Find a material in this region by name
    virtual iMaterialWrapper *FindMaterial (const char *iName);
    /// Find a camera position in this region by name
    virtual iCameraPosition *FindCameraPosition (const char *iName);
    /// Find a collection in this region by name
    virtual iCollection *FindCollection (const char *iName);

    /**
     * Check if some object is in this region.
     * The speed of this function is independent of the number of
     * objects in this region (i.e. very fast).
     */
    virtual bool IsInRegion (iObject* obj);
  } scfiRegion;
  friend struct Region;
};


/// List of 3D engine regions.
class csRegionList : public csRefArrayObject<iRegion>
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csRegionList ();
  virtual ~csRegionList ();

  class RegionList : public iRegionList
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csRegionList);

    virtual int GetCount () const;
    virtual iRegion *Get (int n) const;
    virtual int Add (iRegion *obj);
    virtual bool Remove (iRegion *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iRegion *obj) const;
    virtual iRegion *FindByName (const char *Name) const;
  } scfiRegionList;
};

#endif // __CS_REGION_H__
