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
   * @@@TODO: add density map support.
   * @@@TODO: add density per material.
   */
  virtual void SetDensity (float density) = 0;

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
