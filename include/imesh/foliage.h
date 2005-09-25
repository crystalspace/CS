/*
    Copyright (C) 2004 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_FOLIAGEMESH_H__
#define __CS_IMESH_FOLIAGEMESH_H__

#include "csutil/scf.h"

#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"

struct iMaterialWrapper;
struct iTerraFormer;
struct iLODControl;

class csBox2;
class csTriangle;

/**
 * Vertex data for the foliage mesh.
 */
struct csFoliageVertex
{
  csVector3 pos;
  csVector2 texel;
  csColor color;
  csVector3 normal;
};

SCF_VERSION (iFoliageGeometry, 0, 0, 1);

/**
 * Geometrical data for the foliage object.
 */
struct iFoliageGeometry : public iBase
{
  /**
   * Add a vertex.
   */
  virtual size_t AddVertex (const csVector3& pos, const csVector2& texel,
      	const csColor& color, const csVector3& normal) = 0;

  /**
   * Get the vertices.
   */
  virtual const csDirtyAccessArray<csFoliageVertex>& GetVertices () const = 0;

  /**
   * Add a triangle.
   */
  virtual size_t AddTriangle (const csTriangle& tri) = 0;

  /**
   * Get the triangles.
   */
  virtual const csDirtyAccessArray<csTriangle>& GetTriangles () const = 0;

  /**
   * Set the material.
   */
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;

  /**
   * Get the material.
   */
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
};

/**
 * A representation of an object in the foliage mesh.
 * An object represents geometry at different LOD levels (using
 * iFoliageGeometry).
 */
struct iFoliageObject : public virtual iBase
{
  SCF_INTERFACE (iFoliageObject, 2, 0, 0);

  /**
   * Get the name of this object.
   */
  virtual const char* GetName () const = 0;

  /**
   * Create a new geometry for this object at the specified LOD
   * slot. Note that this function can also be used to replace
   * geometry that is already in that slot.
   * \param lodslot is number (>=0) with 0 meaning low detail.
   */
  virtual csPtr<iFoliageGeometry> CreateGeometry (size_t lodslot) = 0;

  /**
   * After setting the geometry for specific lod slots you
   * can use this function to automatically create the geometry
   * for a missing LOD slots by using a triangle reduction
   * algorithm.
   * \param fromslot is the source mesh to use for the reduction.
   * \param toslot is where the new geometry must be placed.
   * \param factor is a number between 0 and 1 indicating how
   *        many triangles must remain. 0 means none, and 1 means
   *        all.
   */
  virtual csPtr<iFoliageGeometry> CreateGeometryLOD (size_t fromslot,
      size_t toslot, float factor) = 0;

  /**
   * Get the geometry at the specified lodslot.
   * \param lodslot is number (>=0) with 0 meaning low detail.
   * \return 0 in case there is no geometry at this slot.
   */
  virtual iFoliageGeometry* GetGeometry (size_t lodslot) = 0;

  /**
   * Get the maximum lodslot number that has been used by
   * this object. Returns ~0 in case there is no geometry
   * yet.
   */
  virtual size_t GetMaxLodSlot () const = 0;

  /**
   * Return the LOD control for this object.
   */
  virtual iLODControl* GetLODControl () = 0;
};


SCF_VERSION (iFoliageFactoryState, 0, 0, 1);

/**
 * The foliage mesh can be used to make foliage (plants, boulders, ...) that 
 * fits nicely with a terrain.
 * The general API for the foliage factory. Here you define the
 * actual geometry which is shared between all foliage mesh instances.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Foliage mesh object plugin (crystalspace.mesh.object.foliage)
 *   <li>iMeshObjectType::NewFactory()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshFactoryWrapper::GetMeshObjectFactory()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Foliage Factory Loader plugin
 *      (crystalspace.mesh.loader.factory.foliage)
 *   </ul>
 */
struct iFoliageFactoryState : public iBase
{
  /**
   * Create a new foliage object.
   */
  virtual csPtr<iFoliageObject> CreateObject (const char* name) = 0;

  /**
   * Find a foliage object by name.
   */
  virtual iFoliageObject* FindObject (const char* name) const = 0;

  /**
   * Return all foliage objects.
   */
  virtual const csRefArray<iFoliageObject>& GetObjects () const = 0;

  /**
   * Add a foliage object name to a palette index. Palette indices
   * are selected by the 'foliage_types' map (see SetTerraFormer()).
   * Every foliage palette can contain zero or more object names. When
   * a certain block in the landscape uses a specific palette index
   * then the density given in the density map is multiplied with
   * the normalized densities for the objects in that specific foliage
   * palette and this will give the distribution of objects at that
   * spot.
   * \param typeidx is the palette index (or type index) as it corresponds 
   *        with the values from the 'foliage_types' map.
   * \param objectname is the name of the object to use in this type.
   * \param relative_density is the relative density.
   */
  virtual void AddPaletteEntry (size_t typeidx, const char* objectname,
      float relative_density) = 0;

  /**
   * Clear a given palette type.
   */
  virtual void ClearPaletteType (size_t typeidx) = 0;

  /**
   * Get the total number of palette types.
   */
  virtual size_t GetPaletteTypeCount () const = 0;

  /**
   * For a given palette type, return the number of objects
   * in this palette.
   */
  virtual size_t GetPaletteEntryCount (size_t typeidx) const = 0;

  /**
   * For a given palette type and entry index, return the
   * object name and density.
   */
  virtual const char* GetPaletteEntry (size_t typeidx, size_t entryidx,
      float& relative_density) = 0;

  /**
   * The terraformer defines various properties for this foliage mesh.
   * The terraformer needs to support the following properties:
   * <ul>
   * <li>'heights': this basically comes directly from the heightmap
   *     and is returned as an array of floats. The normal simpleformer
   *     (as is used by the landscape engine) supports this automatically.
   * <li>'vertices': this basically comes directly from the heightmap
   *     and is returned as an array of csVector3's. The normal simpleformer
   *     (as is used by the landscape engine) supports this automatically.
   * <li>'foliage_density': this is another float map which represents the
   *     density of foliage at that point. The number represents the
   *     number of foliage objects in one 1x1 unit. So using a uniform
   *     density of 1 will place one object in every 1x1 square.
   *     This map must have same resolution as the heightmap.
   * <li>'foliage_types': this is an int map which indicates the types
   *     of foliage to use for every block in the heightmap. These numbers
   *     correspond with the foliage palette numbers. If this map contains
   *     palette indices that are not defined then no foliage will
   *     be generated on that spot.
   *     This map must have same resolution as the heightmap.
   * </ul>
   */
  virtual void SetTerraFormer (iTerraFormer* form) = 0;
  /**
   * Get the terraformer.
   */
  virtual iTerraFormer* GetTerraFormer () = 0;

  /**
   * This specifies the max region the foliage mesh will sample from
   * (from the terraformer).
   */
  virtual void SetSamplerRegion (const csBox2& region) = 0;
  /**
   * Get the sampler region.
   */
  virtual const csBox2& GetSamplerRegion () const = 0;
};

SCF_VERSION (iFoliageMeshState, 0, 0, 1);

/**
 * This interface describes the API for the foliage mesh object.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Foliage mesh object plugin (crystalspace.mesh.object.foliage)
 *   <li>iMeshObjectFactory::NewInstance()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>SCF_QUERY_INTERFACE() on iMeshWrapper::GetMeshObject()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Foliage Loader plugin (crystalspace.mesh.loader.foliage)
 *   </ul>
 */
struct iFoliageMeshState : public iBase
{
};

#endif // __CS_IMESH_FOLIAGEMESH_H__

