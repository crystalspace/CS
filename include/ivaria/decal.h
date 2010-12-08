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

#include "csutil/scf.h"
#include "csutil/array.h"
#include "ivideo/graph3d.h"
#include "ivideo/rendermesh.h"

struct iSector;
struct iMaterialWrapper;
struct iMeshWrapper;
class csVector3;
class csVector2;
class csPoly3D;
class csColor4;
class csRenderBuffer;

/**\file
 * Decal and Decal manager interfaces
 */

/**
 * \addtogroup geom_utils
 * @{ */

/**\name Decals
 * @{ */
/**
 * A decal created by the iDecalManager. Decals somehow add a piece of geometry projected
 * on a mesh or a group of mesh. It can be used to achieve effects such as bullet holes or
 * shadows under the characters.
 */
struct iDecal
{
};

/**
 * Interface for a decal template which dictates the 
 * appearance of a newly created iDecal.
 */
struct iDecalTemplate : public virtual iBase
{
  SCF_INTERFACE(iDecalTemplate, 2, 0, 0);

  /**
   * Retrieves the time the decal will have to live in seconds before it is 
   * killed.
   *  \return The time to live in seconds.
   */
  virtual float GetTimeToLive () const = 0;

  /**
   * Retrieves the material wrapper to use for this decal.
   *  \return the material wrapper.
   */
  virtual iMaterialWrapper* GetMaterialWrapper () = 0;

  /**
   * Retrieves the rendering priority for this decal
   *  \return the rendering priority.
   */
  virtual CS::Graphics::RenderPriority GetRenderPriority () const = 0;

  /**
   * Retrieves the z-buffer mode for this decal.
   *  \return the z-buffer mode.
   */
  virtual csZBufMode GetZBufMode () const = 0;

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
  virtual float GetDecalOffset () const = 0;

  /**
   * Determines whether or not this type of decal will have its geometry
   * clipped against a plane above the decal.
   *  \return True if top clipping is enabled.
   */
  virtual bool HasTopClipping () const = 0;

  /**
   * Gets the distance between the decal position and the top clipping plane
   * as a multiple of decal size.
   *  \return The top clipping plane scale.
   */
  virtual float GetTopClippingScale () const = 0;

  /**
   * Determines whether or not this type of decal will have its geometry
   * clipped against a plane below the decal.
   *  \return True if bottom clipping is enabled.
   */
  virtual bool HasBottomClipping () const = 0;

  /**
   * Gets the distance between the decal position and the bottom clipping
   * plane as a multiple of decal size.
   *  \return The bottom clipping plane scale.
   */
  virtual float GetBottomClippingScale () const = 0;
  
  /**
   * The min tex coord is the uv coordinate of the top-left corner of the
   * decal.
   *  \return The min tex coordinate.
   */
  virtual const csVector2& GetMinTexCoord () const = 0;

  /**
   * The main color of the decal.
   *  \return The main color of the decal.
   */
  virtual const csColor4& GetMainColor () const = 0;

  /**
   * The color to give vertices close to the top of the decal.  The color of
   * a vertex between the decal position and the top plane will be interpolated
   * between this color and the main color based on distance from the top plane.
   *  \return The top color of the decal.
   */
  virtual const csColor4& GetTopColor () const = 0;

  /**
   * The color to give vertices close to the bottom of the decal.  The color of
   * a vertex between the decal position and the bottom plane will be interpolated
   * between this color and the main color based on distance from the bottom plane.
   *  \return The top color of the decal.
   */
  virtual const csColor4& GetBottomColor () const = 0;

  /**
   * The max tex coord is the uv coordinate of the bottom-right corner of the
   * decal.
   *  \return The max tex coordinate.
   */
  virtual const csVector2& GetMaxTexCoord () const = 0;

  /**
   * The mixmode of the decal.
   *  \return The mixmode.
   */
  virtual const uint GetMixMode () const = 0;
  
  /**
   * The perpendicular face threshold specifies which faces are considered
   * to be perpendicular in the decal.  Perpendicular faces will be tapered
   * outwards by an amount relative to the perpendicular face offset.
   *  \return The perpendicular face threshold.
   */
  virtual float GetPerpendicularFaceThreshold () const = 0;

  /**
   * The perpendicular face offset specifies how much perpendicular faces in
   * the decal will taper out.  The bottom of the perpendicular face will
   * taper out by the amount given, but the top of the perpendicular face
   * will not taper at all.
   *  \return The perpendicular face offset.
   */
  virtual float GetPerpendicularFaceOffset () const = 0;
      
  /**
   * Sets the time the decal will have to live in seconds before it is 
   * killed.
   *  \param timeToLive	The time to live in seconds.
   */
  virtual void SetTimeToLive (float timeToLive) = 0;
  
  /**
   * Sets the material wrapper to use for this decal.
   *  \param material	The material wrapper of the decal.
   */
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;

  /**
   * Sets the rendering priority for this decal
   *  \param renderPriority	The render priority of the decal.
   */
  virtual void SetRenderPriority (CS::Graphics::RenderPriority renderPriority) = 0;

  /**
   * Sets the z-buffer mode for this decal.
   *  \param mode	The z-buffer mode for the decal.
   */
  virtual void SetZBufMode (csZBufMode mode) = 0;

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
  virtual void SetPolygonNormalThreshold (float polygonNormalThreshold) = 0;
  
  /**
   * A decal will be offset a bit from the geometry it wraps around in order
   * to avoid z-buffer fighting issues.
   *
   * The greater this offset is, the less chance there is of z fighting, but
   * if this is too high then the decal will appear to be floating.
   *
   *  \param decalOffset	The distance between decal and the geometry.
   */
  virtual void SetDecalOffset (float decalOffset) = 0;

  /**
   * Enables or disables clipping geometry above the decal.
   *   \param enabled		True if top clipping should be enabled.
   *   \param topPlaneScale	The distance from the decal position to the
   *                            top plane as a multiple of decal size.
   */
  virtual void SetTopClipping (bool enabled, float topPlaneScale = 0.0f) = 0;

  /**
   * Enables or disables clipping geometry below the decal.
   *   \param enabled		True if bottom clipping should be enabled.
   *   \param bottomPlaneScale	The distance from the decal position to the
   *                            bottom plane as a multiple of decal size.
   */
  virtual void SetBottomClipping (bool enabled, float bottomPlaneScale) = 0;

  /**
   * The tex coords are the uv coordinate of the top-left and bottom-right 
   * corner of the decal.
   *  \param min	The top-left corner of the decal.
   *  \param max	The bottom-right corner of the decal.
   */
  virtual void SetTexCoords (const csVector2& min, const csVector2& max) = 0;

  /**
   * The mixmode of the decal.
   *  \param mixMode	 The mixmode of the decal.
   */
  virtual void SetMixMode (uint mixMode) = 0;
  
  /**
   * The perpendicular face threshold specifies which faces are considered
   * to be perpendicular in the decal.  Perpendicular faces will be tapered
   * outwards by an amount relative to the perpendicular face offset.
   *  \param threshold The new perpendicular face threshold.
   */
  virtual void SetPerpendicularFaceThreshold (float threshold) = 0;

  /**
   * The perpendicular face offset specifies how much perpendicular faces in
   * the decal will taper out.  The bottom of the perpendicular face will
   * taper out by the amount given, but the top of the perpendicular face
   * will not taper at all.
   *  \param offset The new perpendicular face offset.
   */
  virtual void SetPerpendicularFaceOffset (float offset) = 0;

  /**
  * The main color of the decal.
  *  \param color The main color of the decal.
  */
  virtual void SetMainColor (const csColor4& color) = 0;

  /**
  * The color to give vertices close to the top of the decal.  The color of
  * a vertex between the decal position and the top plane will be interpolated
  * between this color and the main color based on distance from the top plane.
  *  \param color The top color of the decal.
  */
  virtual void SetTopColor (const csColor4& color) = 0;

  /**
  * The color to give vertices close to the bottom of the decal.  The color of
  * a vertex between the decal position and the bottom plane will be interpolated
  * between this color and the main color based on distance from the bottom plane.
  *  \param color The top color of the decal.
  */
  virtual void SetBottomColor (const csColor4& color) = 0;

  /**
   * Set whether or not the decal will do any clipping. This has to be enabled for
   * SetTopClipping() and SetBottomClipping() being used. Default value is true.
   */
  virtual void SetClipping (bool enabled) = 0;

  /**
   * Get whether or not the decal will do any clipping.
   */
  virtual bool HasClipping () const = 0;
};

/**
 * A decal animation control, to be used by the iMeshObject when the vertices
 * of the decal have to be animated.
 */
struct iDecalAnimationControl
{

  virtual ~iDecalAnimationControl () {}
  /**
   * Update the vertices and normals of the decal.
   * \param decalTemplate The template of the decal
   * \param baseIndex The starting index of the vertices and normals that have
   * to be updated in the render buffers provided.
   * \param indices The indices of the iMeshObject corresponding to the indices
   * of the decal. These are the list of the indices provided in
   * iDecalBuilder::AddStaticPoly().
   * \param vertices The vertices of the decal that need to be updated
   * \param normals The normals of the decal that need to be updated
   */
  virtual void UpdateDecal (iDecalTemplate* decalTemplate,
			    size_t baseIndex,
			    csArray<size_t>& indices,
			    csRenderBuffer& vertices,
			    csRenderBuffer& normals) = 0;
};

/**
 * Interface for mesh objects to use to build decals for
 * their mesh object.
 */
struct iDecalBuilder
{
  virtual ~iDecalBuilder () {}

  /**
   * Adds a static polygon to the decal.  The decal builder
   * will build geometry for this polygon and append it to
   * the mesh's extra rendermesh list.
   * \param polygon The polygon to add to the decal.
   * \param indices The indices of the vertices of the iMeshObject corresponding to
   * the vertices of the given polygon. This has to be provided only if you use an
   * iDecalAnimationControl.
   */
  virtual void AddStaticPoly (const csPoly3D& polygon, csArray<size_t>* indices = 0) = 0;

  /**
   * Set the animation controller for this decal.
   */
  virtual void SetDecalAnimationControl (iDecalAnimationControl* animationControl) = 0;
    
};

/**
 * Creates and manages decals.
 *
 * To create a iDecal, just get access to this plugin using
 * csLoadPluginCheck<iDecalManager>
 *
 * Then, just use one of the decal creation functions:
 * - iDecalManager::CreateDecal()
 */
struct iDecalManager : public virtual iBase
{
  SCF_INTERFACE (iDecalManager, 2, 0, 0);

  /**
   * Creates a decal that can be shared among several meshes.
   * \param decalTemplate The template used to create the decal.
   * \param sector The sector to begin searching for nearby meshes.
   * \param pos The position of the decal in world coordinates.
   * \param up The up direction of the decal in world coordinates.
   * \param normal The overall normal of the decal in world coordinates.
   * \param width The width of the decal.
   * \param height The height of the decal.
   * \param oldDecal An existing decal that can be reused for efficiency
   * (it will therefore disappear from its previous position).
   * \return True if the decal is created.
   */
  virtual iDecal* CreateDecal (iDecalTemplate* decalTemplate, 
    iSector* sector, const csVector3& pos, const csVector3& up, 
    const csVector3& normal, float width = 1.0f, float height = 1.0f,
    iDecal* oldDecal = 0) = 0;

  /**
   * Creates a decal template and fills it with default values.
   *  \param material The material wrapper for this decal template.
   *  \return The newly created decal template.
   */
  virtual csRef<iDecalTemplate> CreateDecalTemplate (
    iMaterialWrapper* material) = 0;

  /**
   * Deletes the given decal.
   *  \param decal The decal to be deleted.
   */
  virtual void DeleteDecal (const iDecal* decal) = 0;
  
  /**
   * Gets the number of decals.
   * \return The number of decals.
   */
  virtual size_t GetDecalCount () const = 0;

  /**
   * Gets the specified decal.
   *  \param idx The index of the decal to get, must be between 0 and
   *    GetDecalCount()-1
   */
  virtual iDecal* GetDecal (size_t idx) const = 0;

  /**
   * Creates a decal on a specific mesh.
   * \param decalTemplate The template used to create the decal.
   * \param mesh The mesh to put the decal on.
   * \param pos The position of the decal in world coordinates.
   * \param up The up direction of the decal in world coordinates.
   * \param normal The overall normal of the decal in world coordinates.
   * \param width The width of the decal.
   * \param height The height of the decal.
   * \param oldDecal An existing decal that can be reused for efficiency
   * (it will therefore disappear from its previous position).
   * \return True if the decal is created.
   */
  virtual iDecal* CreateDecal (iDecalTemplate* decalTemplate, 
    iMeshWrapper* mesh, const csVector3& pos, const csVector3& up, 
    const csVector3& normal, float width = 1.0f, float height = 1.0f,
    iDecal* oldDecal = 0) = 0;
};
/** @} */

/** @} */

#endif // __CS_IGEOM_DECAL_H__
