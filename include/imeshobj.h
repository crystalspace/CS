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

#ifndef __IMESHOBJ_H__
#define __IMESHOBJ_H__

#include "csutil/scf.h"
#include "csgeom/box.h"
#include "iplugin.h"

struct iRenderView;
struct iMovable;
struct iLight;

SCF_VERSION (iMeshObject, 0, 0, 4);

/**
 * This is a general mesh object that the engine can interact with.
 */
struct iMeshObject : public iBase
{
  /**
   * First part of Draw. The engine will call this DrawTest() before
   * calling Draw() so DrawTest() can (if needed) remember computationally
   * expensive data. If DrawTest() returns false the engine will not
   * call Draw(). Possibly UpdateLighting() will be called in between
   * DrawTest() and Draw().
   */
  virtual bool DrawTest (iRenderView* rview, iMovable* movable) = 0;

  /**
   * Update lighting for the object on the given position.
   */
  virtual void UpdateLighting (iLight** lights, int num_lights,
      	iMovable* movable) = 0;

  /**
   * Draw this mesh object. Returns false if not visible.
   * If this function returns true it does not mean that the object
   * is invisible. It just means that this MeshObject thinks that the
   * object was probably visible. DrawTest() will be called before
   * this function (possibly with an UpdateLighting() in between.
   */
  virtual bool Draw (iRenderView* rview, iMovable* movable) = 0;

  /**
   * Get the bounding box in object space for this mesh object.
   * If 'accurate' is true an effort has to be done to make the
   * bounding box as accurate as possible. Otherwise it just has
   * to be a bounding box.
   */
  virtual void GetObjectBoundingBox (csBox3& bbox, bool accurate = false) = 0;
};

SCF_VERSION (iMeshObjectFactory, 0, 0, 3);

/**
 * This object is a factory which can generate
 * mesh objects of a certain type. For example, if you want to have
 * multiple sets of sprites from the same sprite template then
 * you should have an instance of iMeshObjectFactory for evey sprite
 * template and an instance of iMeshObject for every sprite.
 */
struct iMeshObjectFactory : public iBase
{
  /// Create an instance of iMeshObject.
  virtual iMeshObject* NewInstance () = 0;
};

SCF_VERSION (iMeshObjectType, 0, 0, 1);

/**
 * This plugin describes a specific type of mesh objects. Through
 * this plugin the user can create instances of mesh object factories
 * which can then be used to create instances of mesh objects.
 */
struct iMeshObjectType : public iPlugIn
{
  /// Create an instance of iMeshObjectFactory.
  virtual iMeshObjectFactory* NewFactory () = 0;
};

SCF_VERSION (iMeshFactoryWrapper, 0, 0, 1);

/**
 * This interface corresponds to the object in the engine
 * that holds reference to the real iMeshObjectFactory.
 */
struct iMeshFactoryWrapper : public iBase
{
  /// Get the iMeshObjectFactory.
  virtual iMeshObjectFactory* GetMeshObjectFactory () = 0;
};

#endif

