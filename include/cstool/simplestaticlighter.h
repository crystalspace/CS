/*
    Copyright (C) 2008 by Jorrit Tyberghein

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

#ifndef __CS_SIMPLESTATICLIGHTER_H__
#define __CS_SIMPLESTATICLIGHTER_H__

/**\file
 * Simple static vertex lighting for meshes.
 */

#include "csextern.h"

struct iMeshWrapper;
struct iLight;
struct iSector;
struct iEngine;
struct iGeneralFactoryState;
class csColor4;

namespace CS
{
namespace Lighting
{

/**
 * The functions in this class all provide a simple way to initialize
 * the "static color" of a mesh (usually genmesh) to provide a way
 * to do simple static 'lighting'. Calling this function has the same
 * effect as having a mesh which is staticly lit using vertex lighting
 * through the lighter2 utility.
 */
class CS_CRYSTALSPACE_EXPORT SimpleStaticLighter
{
public:
  /**
   * Specify what type of shadows we want here.
   */
  enum ShadowType
  {
    /// No shadows.
    CS_SHADOW_NONE = 0,
    /// Only shadow based on center of object.
    CS_SHADOW_CENTER,
    /// Shadow based on the vertices of the bounding box of the object.
    CS_SHADOW_BOUNDINGBOX,
    /// Shadow every individual vertex.
    CS_SHADOW_FULL
  };

private:
  static void CalculateLighting (iMeshWrapper* mesh,
    iGeneralFactoryState* fact_state, iLight* light,
    ShadowType shadow_type, csColor4* colors, bool init);

public:
  /**
   * Fill the static color of the mesh with the given color.
   */
  static void ConstantColor (iMeshWrapper* mesh, const csColor4& color);

  /**
   * Calculate lighting for this mesh as seen from the given light.
   */
  static void ShineLight (iMeshWrapper* mesh, iLight* light,
      ShadowType shadow_type = CS_SHADOW_NONE);

  /**
   * Calculate lighting for this mesh as seen from a specified
   * number of lights that affect this mesh.
   */
  static void ShineLights (iMeshWrapper* mesh, iEngine* engine, int maxlights,
      ShadowType shadow_type = CS_SHADOW_NONE);

  /**
   * Calculate lighting for all meshes in this sector as seen
   * from a specified number of lights that affect those meshes.
   */
  static void ShineLights (iSector* sector, iEngine* engine, int maxlights,
      ShadowType shadow_type = CS_SHADOW_NONE);
};

} // namespace Lighting
} // namespace CS

/** @} */

#endif // __CS_SIMPLESTATICLIGHTER_H__

