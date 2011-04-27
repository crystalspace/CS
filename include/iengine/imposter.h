/*
    Copyright (C) 2009 by Keith Fulton and Mike Gist

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

#ifndef __CS_IENGINE_IMPOSTER_H__
#define __CS_IENGINE_IMPOSTER_H__


/**\file
 * Imposter interface
 */
/**
 * \addtogroup engine3d
 * @{ */

#include "csutil/scf.h"

struct iSharedVariable;

/**
 * iImposterFactory defines the interface a mesh factory must
 * implement for its meshes to be used as imposters by
 * the engine.
 */
struct iImposterFactory : public virtual iBase
{
  SCF_INTERFACE(iImposterFactory, 1, 4, 0);

 /**
  * Given a mesh, activate its imposter.
  */
  virtual void AddImposter(iMeshWrapper* mesh, iRenderView* rview) = 0;

 /**
  * Given a mesh, deactivate and remove its imposter.
  */
  virtual void RemoveImposter(iMeshWrapper* mesh) = 0;

 /**
  * Whether we are currently rendering the imposter
  */
  virtual bool RenderingImposter(iMeshWrapper* mesh) = 0;

  /**
   * Sets the minimum imposter distance.
   * This is the distance from camera beyond which an imposter is used.
   */
  virtual void SetMinDistance(float dist) = 0;

  /**
   * Gets the minimum imposter distance.
   */
  virtual float GetMinDistance() = 0;

  /**
   * Sets the rotation tolerance.
   * This is the maximum allowable angle difference between when the
   * imposter was created and the current position of the camera.
   * Angles greater than this trigger a re-render of the imposter.
   */
  virtual void SetRotationTolerance(float angle) = 0;

  /**
   * Gets the rotation tolerance.
   */
  virtual float GetRotationTolerance() = 0;

  /**
   * Sets the camera rotation tolerance.
   * This is the tolerance angle between the z->1 vector and the object
   * on screen. Exceeding this value triggers the updating of the imposter
   * whenever the object slides too much away from the center of screen.
   */
  virtual void SetCameraRotationTolerance(float angle) = 0;

  /**
   * Gets the camera rotation tolerance.
   */
  virtual float GetCameraRotationTolerance() = 0;

  /**
   * Sets the shader to be used by the imposters.
   */
  virtual void SetShader(const char* type, const char* shader) = 0;

  /**
  * Sets whether to render the real mesh while waiting for the imposter to init.
  */
  virtual void SetRenderReal(bool renderReal) = 0;
};

struct iImposterMesh : public virtual iBase
{
  SCF_INTERFACE(iImposterMesh, 1, 1, 0);

  /**
   * Destroy this imposter.
   */
  virtual void Destroy() = 0;

  /**
   * Query whether the r2t has been performed for this imposter.
   */
  virtual bool Rendered() const = 0;
};

struct ImposterShader
{
  csString type;
  csString name;

  ImposterShader (const char* type, const char* name)
    : type (type), name (name)
  {
  }
};

/** @} */

#endif // __CS_IENGINE_IMPOSTER_H__
