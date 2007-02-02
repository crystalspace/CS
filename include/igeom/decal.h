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
#include <ivideo/graph3d.h>

struct iSector;
struct iMaterialWrapper;
class csVector3;
class csVector2;
class csPoly3D;
class csColor4;

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
 * Interface for a decal template which dictates the 
 * appearance of a newly created decal.
 */
struct iDecalTemplate : public virtual iBase
{
  SCF_INTERFACE(iDecalTemplate, 1, 0, 0);

  /**
   * Retrieves the time the decal will have to live in seconds before it is 
   * killed.
   *  \return The time to live in seconds.
   */
  virtual float GetTimeToLive() const = 0;

  /**
   * Retrieves the material wrapper to use for this decal.
   *  \return the material wrapper.
   */
  virtual iMaterialWrapper* GetMaterialWrapper() = 0;

  /**
   * Retrieves the rendering priority for this decal
   *  \return the rendering priority.
   */
  virtual long GetRenderPriority() const = 0;

  /** 
   * Retrieves the z-buffer mode for this decal.
   *  \return the z-buffer mode.
   */
  virtual csZBufMode GetZBufMode() const = 0;

  /** 
   * Retrieves the polygon normal threshold for this decal.  
   *
   * Values close to 1 will exclude polygons that don't match the decal's 
   * normal closely, and values closer to 0 will be more accepting and allow
   * polygons with a very different normal from the decal's normal.
   *
   * Values between -1 and 0 are acceptable, but will allow polygons that
   * are facing in the opposite direction from the decal to be included.
   *
   *  \return the polygon threshold.
   */
  virtual float GetPolygonNormalThreshold() const = 0;

  /**
   * A decal will be offset a bit from the geometry it wraps around in order
   * to avoid z-buffer fighting issues.
   *
   * The greater this offset is, the less chance there is of z fighting, but
   * if this is too high then the decal will appear to be floating.
   *
   *  \return the decal offset.
   */
  virtual float GetDecalOffset() const = 0;
  
  /**
   * Determines whether the decal will be clipped against a near and far
   * plane.
   * \return True if near-far clipping is enabled.
   */
  virtual bool HasNearFarClipping() const = 0;

  /**
   * If near-far clipping is enabled, this determines the distance between
   * the near and far plane.
   * \return The distance between the near and far plane.
   */
  virtual float GetNearFarClippingDist() const = 0;

  /**
   * The min tex coord is the uv coordinate of the top-left corner of the
   * decal.
   *  \return The min tex coordinate.
   */
  virtual const csVector2 & GetMinTexCoord() const = 0;

  /**
   * The max tex coord is the uv coordinate of the bottom-right corner of the
   * decal.
   *  \return The max tex coordinate.
   */
  virtual const csVector2 & GetMaxTexCoord() const = 0;

  /**
   * The mixmode of the decal.
   *  \return The mixmode.
   */
  virtual const uint GetMixMode() const = 0;
  
  /**
   * Sets the time the decal will have to live in seconds before it is 
   * killed.
   *  \param timeToLive	The time to live in seconds.
   */
  virtual void SetTimeToLive(float timeToLive) = 0;
  
  /**
   * Sets the material wrapper to use for this decal.
   *  \param material	The material wrapper of the decal.
   */
  virtual void SetMaterialWrapper(iMaterialWrapper* material) = 0;

  /**
   * Sets the rendering priority for this decal
   *  \param renderPriority	The render priority of the decal.
   */
  virtual void SetRenderPriority(long renderPriority) = 0;

  /** 
   * Sets the z-buffer mode for this decal.
   *  \param mode	The z-buffer mode for the decal.
   */
  virtual void SetZBufMode(csZBufMode mode) = 0;

  /** 
   * Sets the polygon normal threshold for this decal.  
   *
   * Values close to 1 will exclude polygons that don't match the decal's 
   * normal closely, and values closer to 0 will be more accepting and allow
   * polygons with a very different normal from the decal's normal.
   *
   * Values between -1 and 0 are acceptable, but will allow polygons that
   * are facing in the opposite direction from the decal to be included.
   *
   *  \param polygonNormalThreshold	The polygon normal threshold.
   */
  virtual void SetPolygonNormalThreshold(float polygonNormalThreshold) = 0;
  
  /**
   * A decal will be offset a bit from the geometry it wraps around in order
   * to avoid z-buffer fighting issues.
   *
   * The greater this offset is, the less chance there is of z fighting, but
   * if this is too high then the decal will appear to be floating.
   *
   *  \param decalOffset	The distance between decal and the geometry.
   */
  virtual void SetDecalOffset(float decalOffset) = 0;

  /**
   * Determines whether the decal will be clipped against a near and far
   * plane.
   *  \param enabled	True if near-far clipping is enabled.
   */
  virtual void SetNearFarClipping(bool enabled) = 0;

  /**
   * If near-far clipping is enabled, this determines the distance between
   * the near and far plane.
   *  \param dist	The distance between the near and far plane.
   */
  virtual void SetNearFarClippingDist(float dist) = 0;

  /**
   * The tex coords are the uv coordinate of the top-left and bottom-right 
   * corner of the decal.
   *  \param min	The top-left corner of the decal.
   *  \param max	The bottom-right corner of the decal.
   */
  virtual void SetTexCoords(const csVector2 & min, const csVector2 & max) = 0;

  /**
   * The mixmode of the decal.
   *  \param mixMode	 The mixmode of the decal.
   */
  virtual void SetMixMode(uint mixMode) = 0;
};

/**
 * Interface for mesh objects to use to build decals for
 * their mesh object.
 */
struct iDecalBuilder
{
  virtual ~iDecalBuilder() {}

  /**
   * Adds a static polygon to the decal.  The decal builder
   * will build geometry for this polygon and append it to
   * the mesh's extra rendermesh list.
   * \param p   The polygon to add to the decal.
   */
  virtual void AddStaticPoly(const csPoly3D & p) = 0;
    
};

/**
 * Creates and manages decals.
 *
 * To create a decal, just get access to this plugin using
 * csLoadPluginCheck<iDecalManager>
 *
 * Then, just use one of the decal creation functions:
 * - iDecalManager::CreateDecal()
 */
struct iDecalManager : public virtual iBase
{
  SCF_INTERFACE(iDecalManager, 1, 0, 0);

  /**
   * Creates a decal.
   * \param decalTemplate The template used to create the decal.
   * \param sector The sector to begin searching for nearby meshes.
   * \param pos The position of the decal in world coordinates.
   * \param up The up direction of the decal.
   * \param normal The overall normal of the decal.
   * \param width The width of the decal.
   * \param height The height of the decal.
   * \return True if the decal is created.
   */
  virtual bool CreateDecal(iDecalTemplate * decalTemplate, 
    iSector * sector, const csVector3 & pos, const csVector3 & up, 
    const csVector3 & normal, float width=1.0f, float height=1.0f) = 0;

  /**
   * Creates a decal template and fills it with default values.
   *  \param The material wrapper for this decal template.
   *  \return The newly created decal template.
   */
  virtual csRef<iDecalTemplate> CreateDecalTemplate(
          iMaterialWrapper* material) = 0;

  /**
   * Deletes the given decal.
   *  \param decal The decal to be deleted.
   */
  virtual void DeleteDecal(const iDecal * decal) = 0;
  
  /**
   * Gets the number of decals.
   * \return The number of decals.
   */
  virtual size_t GetDecalCount() const = 0;

  /**
   * Gets the specified decal.
   *  \param idx The index of the decal to get, must be between 0 and
   *    GetDecalCount()-1
   */
  virtual iDecal * GetDecal(size_t idx) const = 0;
};

#endif // __CS_IGEOM_DECAL_H__
