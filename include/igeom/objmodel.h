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

#ifndef __CS_IGEOM_OBJMODEL_H__
#define __CS_IGEOM_OBJMODEL_H__

/**\file
 * Mesh object models
 */

/**
 * \addtogroup geom_utils
 * @{ */
 
#include "csutil/scf_interface.h"
#include "csutil/ref.h"

struct iPolygonMesh;
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
  SCF_INTERFACE(iObjectModel, 2, 0, 0);
  /**
   * Returns a number that will change whenever the shape of this object
   * changes. If that happens then the data in all the returned polygon
   * meshes and bounding volumes will be invalid.
   */
  virtual long GetShapeNumber () const = 0;

  /**
   * Get a polygon mesh representing the basic geometry of the object.
   * Can return 0 if this object model doesn't support that.
   */
  virtual iPolygonMesh* GetPolygonMeshBase () = 0;

  /**
   * Get a polygon mesh representing the geometry of the object.
   * This mesh is useful for collision detection.
   * Can return 0 if this object model doesn't support that.
   */
  virtual iPolygonMesh* GetPolygonMeshColldet () = 0;

  /**
   * Set a polygon mesh representing the geometry of the object.
   * This mesh is useful for collision detection.
   * This can be used to replace the default polygon mesh returned
   * by GetPolygonMeshColldet() with one that has less detail or
   * even to support polygon mesh for mesh objects that otherwise don't
   * support it. The object model will keep a reference to the
   * given polymesh.
   */
  virtual void SetPolygonMeshColldet (iPolygonMesh* polymesh) = 0;

  /**
   * Get a polygon mesh specifically for visibility culling (to be used
   * as an occluder). This polygon mesh is guaranteed to be smaller or equal
   * to the real object. In other words: if you would render the original
   * mesh in red and this one in blue you should not see any blue anywhere.
   * This kind of lower detail version can be used for occlusion writing
   * in a visibility culling system.
   * Can return 0 if this object model doesn't support that. In that
   * case the object will not be used for visibility culling.
   */
  virtual iPolygonMesh* GetPolygonMeshViscull () = 0;

  /**
   * Set a polygon mesh representing the geometry of the object.
   * This mesh is useful for visibility culling.
   * This can be used to replace the default polygon mesh returned
   * by GetPolygonMeshViscull() with one that has less detail or
   * even to support polygon mesh for mesh objects that otherwise don't
   * support it. The object model will keep a reference to the
   * given polymesh.
   */
  virtual void SetPolygonMeshViscull (iPolygonMesh* polymesh) = 0;

  /**
   * Get a polygon mesh specifically for shadow casting (to be used by the
   * shadow manager). This polygon mesh is guaranteed to be smaller or equal
   * to the real object. In other words: if you would render the original
   * mesh in red and this one in blue you should not see any blue anywhere.
   * Can return 0 if this object model doesn't support that. In that
   * case the object will not be used for shadow casting.
   */
  virtual iPolygonMesh* GetPolygonMeshShadows () = 0;

  /**
   * Set a polygon mesh representing the geometry of the object.
   * This mesh is useful for shadow casting.
   * This can be used to replace the default polygon mesh returned
   * by GetPolygonMeshShadows() with one that has less detail or
   * even to support polygon mesh for mesh objects that otherwise don't
   * support it. The object model will keep a reference to the
   * given polymesh.
   */
  virtual void SetPolygonMeshShadows (iPolygonMesh* polymesh) = 0;

  /**
   * Create a polygon mesh representing a lower detail version of the
   * object but without the restrictions of GetPolygonMeshViscull().
   * The floating point input number is 0 for minimum detail and
   * 1 for highest detail. This function may return the same polygon
   * mesh as GetPolygonMeshColldet() (but with ref count incremented by one).
   * Can return 0 if this object model doesn't support that.
   */
  virtual csPtr<iPolygonMesh> CreateLowerDetailPolygonMesh (float detail) = 0;

  /**
   * Get the bounding box in object space for this mesh object.
   */
  virtual void GetObjectBoundingBox (csBox3& bbox) = 0;

  /**
   * Override the bounding box of this mesh object in object space.
   * Note that some mesh objects don't have a bounding box on their own
   * and may delegate this call to their factory (like genmesh).
   */
  virtual void SetObjectBoundingBox (const csBox3& bbox) = 0;

  /**
   * Get the radius and center of this object in object space.
   */
  virtual void GetRadius (csVector3& radius, csVector3& center) = 0;

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

#endif // __CS_IGEOM_OBJMODEL_H__

