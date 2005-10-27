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

#ifndef __CS_IENGINE_SCENENODE_H__
#define __CS_IENGINE_SCENENODE_H__

/**\file
 * SceneNode - a node in the scene (linked with movables).
 */
/**
 * \addtogroup engine3d
 * @{ */
 
#include "csutil/scf.h"
#include "csutil/refarr.h"

struct iMovable;
struct iLight;
struct iMeshWrapper;
struct iCamera;

/**
 * This interface represents a node in the scene graph. It basically
 * represents an object, light, or camera. The scene graph is build out
 * of scene nodes and movables.
 * 
 * Main ways to get pointers to this interface:
 * - iMeshWrapper::QuerySceneNode()
 * - iLight::QuerySceneNode()
 * - iCamera::QuerySceneNode()
 * - iMovable::GetSceneNode()
 * 
 * Main users of this interface:
 * - iEngine
 */
struct iSceneNode : public virtual iBase
{
  SCF_INTERFACE(iSceneNode, 2,0,0);

  /**
   * Get the movable for this scene node.
   */
  virtual iMovable* GetMovable () const = 0;

  /**
   * Get the corresponding mesh. Returns 0 if this is not a node for a mesh.
   */
  virtual iMeshWrapper* QueryMesh () = 0;

  /**
   * Get the corresponding light. Returns 0 if this is not a node for a light.
   */
  virtual iLight* QueryLight () = 0;

  /**
   * Get the corresponding camera. Returns 0 if this is not a node for a camera.
   */
  virtual iCamera* QueryCamera () = 0;

  /**
   * Set the parent scene node.
   */
  virtual void SetParent (iSceneNode* parent) = 0;

  /**
   * Get the parent scene node.
   */
  virtual iSceneNode* GetParent () const = 0;

  /**
   * The children of this scene node.
   */
  virtual const csRefArray<iSceneNode>& GetChildren () const = 0;
};

/** @} */

#endif // __CS_IENGINE_SCENENODE_H__

