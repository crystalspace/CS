/*
    Crystal Space 3D engine
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
#ifndef __CS_IGEOM_DECAL_H__
#define __CS_IGEOM_DECAL_H__

#include <csutil/scf.h>

struct iSector;
struct iMaterialWrapper;
class csVector3;

/**\file
 * Decal and Decal manager interfaces
 */

/**
 * \addtogroup geom_utils
 * @{ */

/**
 * A decal created by the decal manager.
 */
struct iDecal
{
};

/**
 * Creates and manages decals.
 *
 * To create a decal, just get access to this plugin using
 * csLoadPluginCheck<iDecalManager>
 *
 * Then, just use one of the decal creation functions:
 * - iDecalManager::CreateDecal()
 * - iDecalManager::ProjectDecal()
 */
struct iDecalManager : public virtual iBase
{
  SCF_INTERFACE(iDecalManager, 1, 0, 0);

  /**
   * Creates a decal.
   * \param material The material to assign to the decal.
   * \param sector The sector to begin searching for nearby meshes.
   * \param pos The position of the decal in world coordinates.
   * \param up The up direction of the decal.
   * \param normal The overall normal of the decal.
   * \param width The width of the decal.
   * \param height The height of the decal.
   * \return True if the decal is created.
   */
  virtual bool CreateDecal(iMaterialWrapper *  material, iSector * sector, 
      const csVector3 * pos, const csVector3 * up, const csVector3 * normal, 
      float width=1.0f, float height=1.0f) = 0;

  /**
   * Projects a decal using TraceBeam.
   * \param material The material to assign to the decal.
   * \param sector The sector to begin in the TraceBeam.
   * \param start The starting vertex of the line passed to TraceBeam.
   * \param end The end vertex of the line passed to TraceBeam.
   * \param up The up direction of the decal.
   * \param normal The overall normal of the decal, or 0 to compute a normal
   *               based on the first intersecting triangle of the beam.
   * \param width The width of the decal.
   * \param height The height of the decal.
   * \return True if the decal is created.
   */
  virtual bool ProjectDecal(iMaterialWrapper * material, iSector * sector, 
      const csVector3 * start, const csVector3 * end, const csVector3 * up,
      const csVector3 * normal=0.0f, float width=1.0f, float height=1.0f) = 0;

  /**
   *  Sets the threshold between polygon normal and decal normal.
   *
   *  If the dot product between the polygon's normal and the normal of the
   *  decal is less than this threshold, then the decal won't show up on that
   *  polygon.
   *
   *  \param threshold The new threshold for new decals.
   */
  virtual void SetPolygonNormalThreshold(float threshold) = 0;

  /**
   *  Gets the threshold between polygon normal and decal normal.
   *
   *  If the dot product between the polygon's normal and the normal of the
   *  decal is less than this threshold, then the decal won't show up on that
   *  polygon.
   *
   *  \return The current threshold for new decals.
   */
  virtual float GetPolygonNormalThreshold() const = 0;

  /**
   * Sets the distance to offset the decal along its normal in order to avoid
   * z fighting.
   *
   * \param offset The new polygon offset for new decals.
   */
  virtual void SetDecalOffset(float offset) = 0;

  /**
   * Gets the distance to offset the decal along its normal in order to avoid
   * z fighting.
   *
   * \return The current polygon offset for new decals.
   */
  virtual float GetDecalOffset() const = 0;

  /**
   * Turns on and off near/far plane clipping of the decals.
   * \param clip True if near/far clipping should be enabled.
   */
  virtual void SetNearFarClipping(bool clip) = 0;

  /**
   * Returns whether near/far plane clipping of decals is enabled.
   * \return True if near/far clipping is enabled.
   */
  virtual bool GetNearFarClipping() const = 0;

  /**
   * Sets the distance between the decal position and the near/far plane based
   * on a multiple of the decal's radius.
   * \param scale The new near/far scale.
   */
  virtual void SetNearFarScale(float scale) = 0;

  /**
   * Gets the distance between the decal position and the near/far plane based
   * on a multiple of the decal's radius.
   * \return The current near/far scale.
   */
  virtual float GetNearFarScale() const = 0;
};

#endif // __CS_IGEOM_DECAL_H__
