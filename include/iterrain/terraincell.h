/*
    Copyright (C) 2006 by Kapoulkine Arseny

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

#ifndef __CS_ITERRAIN_TERRAINCELL_H__
#define __CS_ITERRAIN_TERRAINCELL_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "csutil/refarr.h"

#include "terrainarray.h"

struct iRenderView;
struct iTerrainCellRenderProperties;
struct iTerrainCellCollisionProperties;
struct iImage;

struct iCollider;

class csVector2;
class csVector3;
class csRect;
class csRefCount;
class csReversibleTransform;

/**
 * Locked height data. This class holds an information needed to
 * fill/interpret height data. The elements are object-space heights
 * (that is, they are _not_ affected by cell.GetSize ().z).
 *
 * Two-dimensional height array is linearized, so additional math is needed
 * to get access to desired height values:
 * Instead of data[y][x], use data[y * pitch + x]
 */
struct csLockedHeightData
{
  float* data;			///< height data array
  unsigned int pitch;	///< array pitch
};

/**
 * Locked height data. This class holds an information needed to
 * fill/interpret material map data. The elements are indices of materials
 * in material palette.
 *
 * Two-dimensional material index array is linearized, so additional math is
 * needed to get access to desired values:
 * Instead of data[y][x], use data[y * pitch + x]
 */
struct csLockedMaterialMap
{
  unsigned char* data;	///< material index data array
  unsigned int pitch;	///< array pitch
};


/**
 * Terrain cell class. Terrain consists of cells, each cell has its own
 * coordinate system (2-axis position and 3-axis scaling). All operations
 * (loading, preloading, destroying, construction of inner structures for
 * rendering, etc.) are done at cell level.
 *
 * A cell can be created via iTerrainFactory interface.
 */
struct iTerrainCell : public virtual iBase
{
  SCF_INTERFACE (iTerrainCell, 1, 0, 0);

  /**
   * Get cell name. It is specified at creation time and may be 0.
   * The name is used only for cell identification purposes (i.e. to get
   * the needed cell from a terrain, see iTerrainSystem::GetCell)
   *
   * \return cell name
   */
  virtual const char* GetName () const = 0;

  /**
   * Get cell rendering properties. Returns pointer to a renderer-specific
   * class, though it is possible to check/change some general properties.
   *
   * \return cell rendering properties
   */
  virtual iTerrainCellRenderProperties* GetRenderProperties () const = 0;
  
  /**
   * Get cell collision properties. Returns pointer to a collider-specific
   * class, though it is possible to check/change some general properties.
   *
   * \return cell collision properties
   */
  virtual iTerrainCellCollisionProperties* GetCollisionProperties () const = 0;

  /// Get render-specific data. Do not use it!
  virtual csRefCount* GetRenderData () const = 0;
  
  /// Set render-specific data. Do not use it!
  virtual void SetRenderData (csRefCount* data) = 0;

  /// Get collider-specific data. Do not use it!
  virtual csRefCount* GetCollisionData () const = 0;
  
  /// Set collider-specific data. Do not use it!
  virtual void SetCollisionData (csRefCount* data) = 0;

  /**
   * Get grid width. It is the width of an array of height data.
   * You can expect it to be 2^n + 1.
   *
   * \return grid width
   */
  virtual int GetGridWidth () const = 0;
  
  /**
   * Get grid height. It is the height of an array of height data.
   * You can expect it to be 2^n + 1 (note: it is equal to grid width)
   *
   * \return grid height
   */
  virtual int GetGridHeight () const = 0;

  /**
   * Get height data (for reading purposes: do not modify it!)
   * This can be used to perform very fast height lookups.
   *
   * \return cell height data
   */
  virtual csLockedHeightData GetHeightData () = 0;
  
  /**
   * Lock an area of height data (for reading/writing purposes)
   * This can be used for terrain deforming.
   * If you want to lock the whole cell, use the rectangle
   * csRect(0, 0, grid width, grid height).
   *
   * Only one area may be locked at a time, locking more than once results in
   * undefined behaviour.
   *
   * \param rectangle - the rectangle which you want to lock.
   *
   * \return cell height data
   */
  virtual csLockedHeightData LockHeightData (const csRect& rectangle) = 0;
  
  /**
   * Commit changes to height data. Use it after changing the desired height values.
   *
   * Unlocking the cell that was not locked results in undefined behaviour
   */
  virtual void UnlockHeightData () = 0;

  /**
   * Get cell position (in object space). X and Y components specify the
   * offsets along X and Z axes, respectively.
   *
   * \return cell position
   */
  virtual const csVector2& GetPosition () const = 0;
  
  /**
   * Get cell size (in object space). X and Y components specify the
   * sizes along X and Z axes, respectively. Z component specifies height
   * scale (warning: it is used only at loading stage, after that all scales
   * are in object space).
   *
   * \return cell size
   */
  virtual const csVector3& GetSize () const = 0;

  /**
   * Get material map width (essentially a width of both material array and
   * material masks, if any).
   *
   * \return material map width
   */
  virtual int GetMaterialMapWidth () const = 0;
  
  /**
   * Get material map height (essentially a height of both material array and
   * material masks, if any).
   *
   * \return material map height
   */
  virtual int GetMaterialMapHeight () const = 0;

  /**
   * Lock an area of material map (practically write-only, reading the
   * values will not produce sensible values if you did not just write
   * them - that is, the returned block memory is a read-write one, but
   * it is a temporary block of memory filled with garbage).
   *
   * If you want to lock the whole cell, use the rectangle
   * csRect(0, 0, material map width, material map height).
   *
   * Only one area may be locked at a time, locking more than once results in
   * undefined behaviour.
   *
   * \param rectangle - the rectangle which you want to lock.
   *
   * \return cell material data
   */
  virtual csLockedMaterialMap LockMaterialMap (const csRect& rectangle) = 0;

  /**
   * Commit changes to material data. Use it after setting the desired
   * material map values.
   *
   * Unlocking the cell that was not locked results in undefined behaviour
   *
   * This updates the material masks with appropriate values.
   */
  virtual void UnlockMaterialMap() = 0;

  /**
   * Set new material mask for the specified material.
   *
   * This function will do image rescaling if needed (i.e. if material map
   * dimensions and image dimensions do not match).
   *
   * \param material - material index
   * \param image - an image of format CS_IMGFMT_PALETTED8
   */
  virtual void SetMaterialMask (unsigned int material, iImage* image) = 0;
  
  /**
   * Set new material mask for the specified material.
   *
   * This function will do image rescaling if needed (i.e. if material map
   * dimensions and image dimensions do not match).
   *
   * \param material - material index
   * \param data - linearized array with material indices
   * \param width - image width
   * \param height - image height
   */
  virtual void SetMaterialMask (unsigned int material, const unsigned char*
                          data, unsigned int width, unsigned int height) = 0;
  
  /**
   * Collide segment with cell (using the collider)
   *
   * \param start - segment start (specified in object space)
   * \param end - segment end (specified in object space)
   * \param oneHit - if this is true, than stop on finding the first
   * intersection point (the closest to the segment start); otherwise, detect
   * all intersections
   * \param points - destination point array
   * 
   * \return true if there were any intersections, false if there were none
   */
  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
                           bool oneHit, iTerrainVector3Array& points) = 0;

  /**
   * Collide set of triangles with cell (using the collider)
   *
   * \param vertices - vertex array
   * \param tri_count - triangle count
   * \param indices - vertex indices, 3 indices for each triangle
   * \param radius - radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans - triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit - if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points - destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   */
  virtual bool CollideTriangles (const csVector3* vertices,
                       unsigned int tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs) = 0;

  /**
   * Collide collider with cell (using the collider)
   *
   * \param collider - collider
   * \param radius - radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans - triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit - if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points - destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   */
  virtual bool Collide (iCollider* collider, float radius,
                       const csReversibleTransform* trans, bool oneHit,
                       iTerrainCollisionPairArray& pairs) = 0;

  /**
   * Query height, that is, do a lookup on height table. For a set of
   * lookups, use GetHeightData for efficiency reasons.
   *
   * \param x - x coordinate (from 0 to grid width - 1 all inclusive)
   * \param y - y coordinate (from 0 to grid height - 1 all inclusive)
   *
   * \return height value
   */
  virtual float GetHeight (int x, int y) const = 0;

  /**
   * Query height doing bilinear interpolation. This is equivalent to doing
   * an intersection with vertical ray, except that it is faster.
   *
   * \param pos - object-space position.
   *
   * \return height value
   */
  virtual float GetHeight (const csVector2& pos) const = 0;
  
  /**
   * Get tangent value.
   *
   * \param x - x coordinate (from 0 to grid width - 1 all inclusive)
   * \param y - y coordinate (from 0 to grid height - 1 all inclusive)
   *
   * \return tangent value
   */
  virtual csVector3 GetTangent (int x, int y) const = 0;
  
  /**
   * Get tangent with bilinear interpolation.
   *
   * \param pos - object-space position.
   *
   * \return tangent value
   */
  virtual csVector3 GetTangent (const csVector2& pos) const = 0;

  /**
   * Get binormal value.
   *
   * \param x - x coordinate (from 0 to grid width - 1 all inclusive)
   * \param y - y coordinate (from 0 to grid height - 1 all inclusive)
   *
   * \return binormal value
   */
  virtual csVector3 GetBinormal (int x, int y) const = 0;
  
  /**
   * Get binormal with bilinear interpolation.
   *
   * \param pos - object-space position.
   *
   * \return binormal value
   */
  virtual csVector3 GetBinormal (const csVector2& pos) const = 0;

  /**
   * Get normal value.
   *
   * \param x - x coordinate (from 0 to grid width - 1 all inclusive)
   * \param y - y coordinate (from 0 to grid height - 1 all inclusive)
   *
   * \return normal value
   */
  virtual csVector3 GetNormal (int x, int y) const = 0;
  
  /**
   * Get normal with bilinear interpolation.
   *
   * \param pos - object-space position.
   *
   * \return normal value
   */
  virtual csVector3 GetNormal (const csVector2& pos) const = 0;
};

#endif // __CS_ITERRAIN_TERRAINCELL_H__
