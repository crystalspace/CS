/*
    Copyright (C) 2005 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_MESHGEN_H__
#define __CS_IENGINE_MESHGEN_H__


/**\file
 * Mesh Generator interface
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "csutil/scf.h"

struct iMeshFactoryWrapper;
struct iMeshWrapper;
class csBox3;
struct iTerraFormer;

/**
 * This interface defines one piece of geometry for the mesh
 * generator.
 *
 * Main creators of instances implementing this interface:
 * - iMeshGenerator::CreateGeometry()
 * 
 * Main ways to get pointers to this interface:
 * - iMeshGenerator::GetGeometry()
 * 
 * Main users of this interface:
 * - iMeshGenerator
 */
struct iMeshGeneratorGeometry : public virtual iBase
{
  SCF_INTERFACE(iMeshGeneratorGeometry, 1, 1, 3);

  /**
   * Add a factory and the maximum distance after which this factory
   * will no longer be used. The minimum distance will be calculated
   * from the maximum distance used for other factories in this geometry.
   * \sa SetMinimumDrawDistance to set a minimum drawing distance for all factories.
   */
  virtual void AddFactory (iMeshFactoryWrapper* factory, float maxdist) = 0;

  /**
   * Get the number of factories for this geometry.
   */
  virtual size_t GetFactoryCount () const = 0;

  /**
   * Remove a factory.
   */
  virtual void RemoveFactory (size_t idx) = 0;

  /**
   * Get a specified factory.
   */
  virtual iMeshFactoryWrapper* GetFactory (size_t idx) = 0;

  /**
   * Get a specified maximum distance.
   */
  virtual float GetMaximumDistance (size_t idx) = 0;

  /**
   * Set the radius for this object. No other objects will be generated
   * within this radius. If this radius is 0 then there is no limitation
   * on object generation (i.e. objects can be put on top of each other
   * if the random generator decides to do that).
   * Default is 0.
   */
  virtual void SetRadius (float radius) = 0;

  /**
   * Get the radius for this object.
   */
  virtual float GetRadius () const = 0;

  /**
   * Set the density. The density is defined as the number of objects
   * in every 1x1 square. Default density is 1.
   * \todo add density map support.
   */
  virtual void SetDensity (float density) = 0;

  /**
   * Add a density factor based on a material.
   * The base density will be used to try a number of positions in
   * every cell. Then for that position it will determine the material
   * that is hit. If that material is listed in the density factor
   * table then that factor will be used to determine if the position
   * should be used or not. Setting a factor of 0 here will disable
   * the material. Setting a factor of 1 will give full density.
   */
  virtual void AddDensityMaterialFactor (iMaterialWrapper* material,
  	float factor) = 0;

  /**
   * Set a density map in grayscale with factor.
   * The base density will be used to try a number of positions in
   * every cell. Density map will be used to affect base density at
   * given point. So for given point density will be based on 
   * base density * value from density map * map factor.
   */
  virtual void SetDensityMap (iTerraFormer* map, float factor, 
    const csStringID & type) = 0;

  /**
   * Set the default factor to use in case the material found on
   * the meshes is not any of the ones defined in the material factory
   * table. By default the default factor is 0. This means that as soon
   * as you use SetDensityMaterialFactor() above then on every material
   * that is not listed there will be no foliage generated. The default
   * factor is NOT used in case the material factor table
   * is empty.
   */
  virtual void SetDefaultDensityMaterialFactor (float factor) = 0;

  /**
   * Get the density.
   */
  virtual float GetDensity () const = 0;

  /**
   * Add position for placing mesh (only material factor will influence it).
   */
  virtual void AddPosition (const csVector2 &pos) = 0;

  virtual void AddPositionsFromMap (iTerraFormer* map, const csBox2 &region,
    uint resx, uint resy, float value, const csStringID & type) = 0;

  /**
   * Set the direction that simulated wind will blow towards.
   * \param x is the x coordinate direction.
   * \param z is the z coordinate direction.
   */
  virtual void SetWindDirection (float x, float z) = 0;

  /**
   * Set the swing bias of simulated wind.
   * A higher bias equals a lower back swing on foliage.
   * Min bias of 1.0;
   * \param bias is the value of the wind bias.
   */
  virtual void SetWindBias (float bias) = 0;

  /**
   * Set the swing speed of simulated wind.
   * Min speed of 0.0;
   * \param speed is the value of the wind speed.
   */
  virtual void SetWindSpeed (float speed) = 0;

  /**
   * Use a density factor map.
   * The sum of all densities from factor maps will affect the density
   * at a given point. The value from the map will be scaled by the given
   * factor. The final density will be based on
   * base density * sum of density factor map values.
   * \param factorMapID The identifier with which the map was added to the
   *   mesh generator.
   * \param factor Factor by which the density from the image is multiplied.
   */
  virtual bool UseDensityFactorMap (const char* factorMapID,
				    float factor) = 0;

  /**
   * Set the minimum drawing distance for any mesh in this geometry.
   * A mesh is not displayed if it is closer than this distance.
   */
  virtual void SetMinimumDrawDistance (float dist) = 0;
  /// Get the minimum drawing distance for any mesh in this geometry.
  virtual float GetMinimumDrawDistance () = 0;
  
  /**
   * Set the minimum distance at which meshes appear opaque.
   * A mesh, at the minimum drawing distance, is drawn fully transparent and
   * is faded in until it reaches the minimum opacity distance (where it is
   * drawn fully opaque).
   */
  virtual void SetMinimumOpaqueDistance (float dist) = 0;
  /// Get the minimum distance at which meshes appear opaque.
  virtual float GetMinimumOpaqueDistance () = 0;

  /**
   * Set the maximum distance at which meshes appear opaque.
   * A mesh, at the maximum opacity distance, is drawn fully opaque and
   * is faded out until it reaches the maximum drawing distance (where it is
   * drawn fully transparent).
   * 
   * \remarks If the mesh generator has distances to fade in between set 
   * (with iMeshGenerator::SetAlphaScale), the closer of the per-generator
   * and per-geometry distances are used for fading.
   */
  virtual void SetMaximumOpaqueDistance (float dist) = 0;
  /// Get the maximum distance at which meshes appear opaque.
  virtual float GetMaximumOpaqueDistance () = 0;
};

/**
 * iMeshGenerator defines the interface for a mesh generator.
 *
 * Main creators of instances implementing this interface:
 * - iSector::CreateMeshGenerator()
 * 
 * Main ways to get pointers to this interface:
 * - iSector::GetMeshGenerator()
 * 
 * Main users of this interface:
 * - engine
 */
struct iMeshGenerator : public virtual iBase
{
  SCF_INTERFACE(iMeshGenerator, 1, 0, 4);

  /**
   * Get the iObject for this mesh generator.
   */
  virtual iObject *QueryObject () = 0;

  /**
   * \deprecated Feature removed in 2.1.
   *   Similar functionality can be achieved by using multiple geometries with
   *   different densities.
   */
  CS_DEPRECATED_METHOD_MSG("Feature removed in 2.1. "
    "Similar functionality can be achieved by using multiple geometries with "
   "different densities.")
  virtual void SetDensityScale (float mindist, float maxdist,
  	float maxdensityfactor) = 0;

  /**
   * Set the alpha scale. If this is set then objects in the distance
   * will use alpha mode.
   * \param mindist is the minimum distance at which alpha scale starts. At
   * this distance the alpha factor will be 1 (meaning no alpha).
   * \param maxdist is the maximum distance at which alpha scale ends. At
   * this distance the alpha factor will be equal to '0'.
   */
  virtual void SetAlphaScale (float mindist, float maxdist) = 0;

  /**
   * Get the box where where we will sample. We will sample starting
   * at the highest y value of the box and pointing down to the lowest
   * y value of the box.
   * \todo In future support other directions for the mapping beam.
   */
  virtual void SetSampleBox (const csBox3& box) = 0;

  /**
   * Get the sample box.
   */
  virtual const csBox3& GetSampleBox () const = 0;

  /**
   * Set the number of cells to use in one direction. Total cells
   * will be 'number*number'. A cell is a logical unit that can
   * keep a number of generated positions. Using bigger (fewer) cells
   * means that more positions are generated at once (possibly causing
   * hickups when this happens). Smaller cells may mean more runtime
   * overhead. Default is 50.
   */
  virtual void SetCellCount (int number) = 0;

  /**
   * Get the cell count.
   */
  virtual int GetCellCount () const = 0;

  /**
   * Set the maximum number of blocks to keep in memory at the same time.
   * A block contains generated positions. Generating a block may be
   * expensive (depending on density and size of the cells) so it may be
   * good to have a high number here. Having a high number means more
   * memory usage though. Default is 100.
   */
  virtual void SetBlockCount (int number) = 0;

  /**
   * Get the block count.
   */
  virtual int GetBlockCount () const = 0;

  /**
   * Create a geometry specification for this mesh generator.
   */
  virtual iMeshGeneratorGeometry* CreateGeometry () = 0;

  /**
   * Get the number of geometry specifications.
   */
  virtual size_t GetGeometryCount () const = 0;

  /**
   * Get a specific geometry.
   */
  virtual iMeshGeneratorGeometry* GetGeometry (size_t idx) = 0;

  /**
   * Remove a geometry.
   */
  virtual void RemoveGeometry (size_t idx) = 0;

  /**
   * Add a mesh on which we will map our geometry.
   */
  virtual void AddMesh (iMeshWrapper* mesh) = 0;

  /**
   * Get the number of meshes.
   */
  virtual size_t GetMeshCount () const = 0;

  /**
   * Get a specific mesh.
   */
  virtual iMeshWrapper* GetMesh (size_t idx) = 0;

  /**
   * Remove a mesh.
   */
  virtual void RemoveMesh (size_t idx) = 0;

  /**
   * Clear the block at a certain position. This is useful for regenerating
   * the block after making changes to the density map (for example).
   */
  virtual void ClearPosition (const csVector3& pos) = 0;
  
  /**
   * Add a density factor map in grayscale.
   * These can be used by geometries to influence the density at some given
   * point.
   * \param factorMapID Identifier by which this map is referenced.
   *   Used to link factor maps to geometries.
   * \param mapImage Image with density data.
   * \param worldToMap Transformation to map world coordinates into
   *   normalized image coordinates. Transform is applied to coordinates
   *   for which the density is to be determined, and the X and Y
   *   components of the transformed coordinates are used as image 
   *   coordinates.
   */
  virtual void AddDensityFactorMap (const char* factorMapID,
				    iImage* mapImage,
				    const CS::Math::Matrix4& worldToMap) = 0;

  /**
   * Update a density factor map from an image.
   * This function does nothing if the factor map doesn't exist.
   */
  virtual void UpdateDensityFactorMap (const char* factorMapID, iImage* mapImage) = 0;

  /**
   * Check if a given density factor map exists.
   */
  virtual bool IsValidDensityFactorMap (const char* factorMapID) const = 0;

  /**
   * Return the world to map transform of a density factor map. This function
   * does not attempt to check if the factor map actually exists so use
   * IsValidDensityFactorMap() first!
   */
  virtual const CS::Math::Matrix4& GetWorldToMapTransform (const char* factorMapID) const = 0;

  /**
   * Get the width of the density factory map. If the map doesn't exist then this
   * will return 0.
   */
  virtual int GetDensityFactorMapWidth (const char* factorMapID) const = 0;
  /**
   * Get the height of the density factory map. If the map doesn't exist then this
   * will return 0.
   */
  virtual int GetDensityFactorMapHeight (const char* factorMapID) const = 0;

  /**
   * Set a default density factor. This factor is the final multiplier
   * for density and so it can be used to control global density to cater
   * for higher/lower-end hardware more easily. The default is 1.0.
   */
  virtual void SetDefaultDensityFactor (float factor) = 0;

  /**
   * Get the default density factor.
   */
  virtual float GetDefaultDensityFactor () const = 0;
};

/** @} */

#endif // __CS_IENGINE_MESHGEN_H__
