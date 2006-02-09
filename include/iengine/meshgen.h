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
 */
struct iMeshGeneratorGeometry : public virtual iBase
{
  SCF_INTERFACE(iMeshGeneratorGeometry, 1, 0, 0);

  /**
   * Add a factory and the maximum distance after which this factory
   * will no longer be used. The minimum distance will be calculated
   * from the maximum distance used for other factories in this geometry.
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
   * @@@ TODO: add density map support.
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
};

/**
 * iMeshGenerator defines the interface for a mesh generator.
 */
struct iMeshGenerator : public virtual iBase
{
  SCF_INTERFACE(iMeshGenerator, 1, 0, 0);

  /**
   * Get the iObject for this mesh generator.
   */
  virtual iObject *QueryObject () = 0;

  /**
   * Set the density scale. If this is set then objects in the distance
   * can have a lower density.
   * \param mindist is the minimum distance at which density scale starts. At
   * this distance the density factor will be 1 (meaning original density
   * as given by the geometry itself).
   * \param maxdist is the maximum distance at which density scale ends. At
   * this distance the density factor will be equal to 'maxdensityfactor'.
   * Note that this is a linear function so distances beyond 'maxdist' will get
   * even lower density.
   * \param maxdensityfactor is the density factor to use at 'maxdist'. 1 means
   * full density and 0 means nothing left.
   */
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
   * y value of the box. (@@@ TODO: in future support other directions
   * for the mapping beam).
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
};

/** @} */

#endif // __CS_IENGINE_MESHGEN_H__
