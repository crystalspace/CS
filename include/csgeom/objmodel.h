/*
    Crystal Space 3D engine
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_CSGEOM_OBJMODEL_H__
#define __CS_CSGEOM_OBJMODEL_H__

/**\file
 * Implementation of iObjectModel.
 */

/**
 * \addtogroup geom_utils
 * @{ */
 
#include "csextern.h"

#include "csutil/refarr.h"

#include "igeom/objmodel.h"
#include "igeom/polymesh.h"



/**
 * Helper class to make it easier to implement iObjectModel in mesh
 * objects. This class does not implement the bounding box and radius
 * functions. Note: this class is meant to be used as an embedded
 * class of a mesh object. That's why it doesn't do any of the SCF stuff.
 */
class csObjectModel : public iObjectModel
{
private:
  long shapenr;
  iPolygonMesh* polymesh_base;
  csRef<iPolygonMesh> polymesh_colldet;
  csRef<iPolygonMesh> polymesh_viscull;
  csRef<iPolygonMesh> polymesh_shadows;
  csRefArray<iObjectModelListener> listeners;

public:
  /**
   * Construct a new csObjectModel. Don't forget to call
   * SetPolygonMesh<xxx>()!
   */
  csObjectModel ()
  {
    shapenr = -1;
    polymesh_base = 0;
  }

  virtual ~csObjectModel () { }

  /**
   * Set the pointer to the base polygon mesh.
   */
  void SetPolygonMeshBase (iPolygonMesh* base)
  {
    polymesh_base = base;
  }

  /**
   * Increase the shape number and also fire all listeners.
   */
  void ShapeChanged ()
  {
    shapenr++;
    FireListeners ();
  }

  /**
   * Set the shape number manually (should not be needed in most cases).
   */
  void SetShapeNumber (long n)
  {
    shapenr = n;
  }

  /**
   * Fire all listeners.
   */
  void FireListeners ()
  {
    size_t i;
    for (i = 0 ; i < listeners.Length () ; i++)
      listeners[i]->ObjectModelChanged (this);
  }

  virtual long GetShapeNumber () const { return shapenr; }
  virtual iPolygonMesh* GetPolygonMeshBase () { return polymesh_base; }
  virtual iPolygonMesh* GetPolygonMeshColldet () { return polymesh_colldet; }
  virtual void SetPolygonMeshColldet (iPolygonMesh* polymesh)
  {
    polymesh_colldet = polymesh;
  }
  virtual iPolygonMesh* GetPolygonMeshViscull () { return polymesh_viscull; }
  virtual void SetPolygonMeshViscull (iPolygonMesh* polymesh)
  {
    polymesh_viscull = polymesh;
  }
  virtual iPolygonMesh* GetPolygonMeshShadows () { return polymesh_shadows; }
  virtual void SetPolygonMeshShadows (iPolygonMesh* polymesh)
  {
    polymesh_shadows = polymesh;
  }
  virtual csPtr<iPolygonMesh> CreateLowerDetailPolygonMesh (float)
  {
    return 0;
  }
  virtual void AddListener (iObjectModelListener* listener)
  {
    RemoveListener (listener);
    listeners.Push (listener);
  }
  virtual void RemoveListener (iObjectModelListener* listener)
  {
    listeners.Delete (listener);
  }
};

/** @} */

#endif // __CS_CSGEOM_OBJMODEL_H__

