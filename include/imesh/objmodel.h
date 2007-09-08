/*
    Crystal Space 3D engine
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_IMESH_OBJMODEL_H__
#define __CS_IMESH_OBJMODEL_H__

/**\file
 * Mesh object models
 */

/**
 * \addtogroup geom_utils
 * @{ */
 
#include "csutil/scf_interface.h"
#include "csutil/ref.h"
#include "iutil/strset.h"

struct iTriangleMesh;
struct iTerraFormer;
struct iTerrainSystem;
struct iObjectModel;

class csBox3;
class csVector3;

/**
 * Implement this class if you're interested in hearing about
 * when the object model changes.
 */
struct iObjectModelListener : public virtual iBase
{
  SCF_INTERFACE(iObjectModelListener, 2, 0, 0);
  /// The object model has changed.
  virtual void ObjectModelChanged (iObjectModel* model) = 0;
};

/**
 * Iterator to iterate over all data mesh ID's in an object model.
 * This is returned by iObjectModel::GetTriangleDataIterator().
 * This iterator will return all data meshes that are set. That includes
 * data meshes that are set but still 0.
 */
struct iTriangleMeshIterator : public virtual iBase
{
  SCF_INTERFACE(iTriangleMeshIterator, 0, 0, 1);

  /// Return true if the iterator has more elements.
  virtual bool HasNext () = 0;

  /**
   * Return next element. The id of the triangle mesh will be returned
   * in 'id'. Note that this function can return 0. This doesn't mean
   * that the iterator has ended. It just means that for the given 'id'
   * the mesh was set to 0.
   */
  virtual iTriangleMesh* Next (csStringID& id) = 0;
};

/**
 * This interface represents data related to some geometry in object
 * space. It is a generic way to describe meshes in the engine. By using
 * this interface you can make sure your code works on all engine geometry.
 * The data returned by this class is in local object space.
 * 
 * Main creators of instances implementing this interface:
 * - All mesh objects implement this interface.
 * 
 * Main ways to get pointers to this interface:
 * - iMeshObject::GetObjectModel()
 * - iMeshObjectFactory::GetObjectModel()
 */
struct iObjectModel : public virtual iBase
{
  SCF_INTERFACE(iObjectModel, 2, 1, 2);
  /**
   * Returns a number that will change whenever the shape of this object
   * changes. If that happens then the data in all the returned polygon
   * meshes and bounding volumes will be invalid.
   */
  virtual long GetShapeNumber () const = 0;

  /**
   * Get a triangle mesh representing the geometry of the object.
   * The ID indicates the type of mesh that is desired. Use the
   * string registry (iStringSet from object registry with tag
   * 'crystalspace.shared.stringset') to convert the ID string to a
   * csStringID identification. Some common possibilities are:
   * - 'base'
   * - 'colldet'
   * - 'viscull'
   * - 'shadows'
   * \return the triangle mesh for that id. If this is 0 then there
   * are two possibilities: either the mesh was never set and in this
   * case the subsystem can pick the base mesh as a fallback. Another
   * possibility is that the triangle data was explicitelly cleared
   * with SetTriangleData(id,0). In that case the mesh is assumed to
   * be empty and usually that means that the specific subsystem will
   * ignore it. To distinguish between these two cases use
   * IsTriangleDataSet(id).
   */
  virtual iTriangleMesh* GetTriangleData (csStringID id) = 0;

  /**
   * Get an iterator to iterate over all triangle meshes in this
   * object model. This includes triangle meshes that are 0.
   */
  virtual csPtr<iTriangleMeshIterator> GetTriangleDataIterator () = 0;

  /**
   * Set a triangle mesh representing the geometry of the object.
   * The ID indicates the type of mesh that you want to change. Note that
   * the base mesh (ID equal to 'base')
   * cannot be modified.
   * \param id is a numer id you can fetch from the string registry
   *        (iStringSet from object registry with tag\
   *        'crystalspace.shared.stringset').
   * \param trimesh is the new mesh. The reference count will be increased.
   *        If you pass in 0 here then this means that the mesh for this
   *        specific ID is disabled. If you want to reset this then
   *        call ResetTriangleData(id). When no mesh is set or
   *        ResetTriangleData() is called many subsystems will use
   *        the base mesh instead. However if you set 0 here then this
   *        means that this mesh is disabled.
   */
  virtual void SetTriangleData (csStringID id, iTriangleMesh* trimesh) = 0;

  /**
   * Return true if the triangle data was set for this id. This can
   * be used to distinguish between an empty mesh as set with
   * SetTriangleData() or SetTriangleData() not being called at all.
   * Calling ResetTriangleData() will clear this.
   */
  virtual bool IsTriangleDataSet (csStringID id) = 0;

  /**
   * Reset triangle data. After calling this it is as if the triangle
   * data was never set.
   */
  virtual void ResetTriangleData (csStringID id) = 0;

  /**
   * Get a terra former representing the geometry of the object.
   * This class is useful for collision detection.
   * Can return 0 if this object model doesn't support that.
   */
  virtual iTerraFormer* GetTerraFormerColldet () = 0;

  /**
   * Get a terrain representing the geometry of the object.
   * This class is useful for collision detection.
   * Can return 0 if this object model doesn't support that.
   */
  virtual iTerrainSystem* GetTerrainColldet () = 0;

  /**
   * Get the bounding box in object space for this mesh object.
   */
  virtual const csBox3& GetObjectBoundingBox () = 0;

  /**
   * Override the bounding box of this mesh object in object space.
   * Note that some mesh objects don't have a bounding box on their own
   * and may delegate this call to their factory (like genmesh).
   */
  virtual void SetObjectBoundingBox (const csBox3& bbox) = 0;

  /**
   * Get the radius and center of this object in object space.
   */
  virtual void GetRadius (float& radius, csVector3& center) = 0;

  /**
   * Add a listener to this object model. This listener will be called whenever
   * the object model changes or right before it is destroyed.
   */
  virtual void AddListener (iObjectModelListener* listener) = 0;

  /**
   * Remove a listener from this object model.
   */
  virtual void RemoveListener (iObjectModelListener* listener) = 0;
};

/** @} */

#endif // __CS_IMESH_OBJMODEL_H__

