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

#ifndef __CS_ITERRAIN_TERRAINSYSTEM_H__
#define __CS_ITERRAIN_TERRAINSYSTEM_H__

#include "csutil/scf.h"
#include "csutil/array.h"
#include "csutil/refarr.h"

#include "terrainarray.h"

struct iRenderView;
struct iTerrainCell;
struct iMaterialWrapper;
struct iMovable;
struct iCollider;

class csVector2;
class csVector3;

class csReversibleTransform;

/**
 * This class represents the terrain object as a set of cells. The object
 * can be rendered and collided with. To gain access to some operations that
 * are done at cell level you might want to use cell quering functions
 * (GetCell)
 */
struct iTerrainSystem : public virtual iBase
{
  SCF_INTERFACE (iTerrainSystem, 1, 0, 0);

  /**
   * Query a cell by name
   *
   * \return pointer to the cell with the given name, or NULL, if none found
   *
   * \rem this will perform cell loading if the resulted cell was not
   * completely loaded
   */
  virtual iTerrainCell* GetCell (const char* name) = 0;

  /**
   * Query a cell by position
   *
   * \return pointer to the first cell which intersects with the vertical ray
   * of given position, or NULL if none found
   *
   * \rem this will perform cell loading if the resulted cell was not
   * completely loaded
   */
  virtual iTerrainCell* GetCell (const csVector2& pos) = 0;

  /**
   * Get material palette. The material map indices index this array.
   *
   * \return material palette
   */
  virtual const csRefArray<iMaterialWrapper>& GetMaterialPalette () const = 0;
  
  /**
   * Set a new material palette.
   *
   * \param array - new  material palette
   */
  virtual void SetMaterialPalette (const csRefArray<iMaterialWrapper>& array)
                                                                          = 0;

  /**
   * Collide segment with the terrain
   *
   * \param start - segment start (specified in object space)
   * \param end - segment end (specified in object space)
   * \param oneHit - if this is true, than stop on finding the first
   * intersection point (the closest to the segment start); otherwise, detect
   * all intersections
   * \param points - destination point array
   * 
   * \return true if there were any intersections, false if there were none
   *
   * \rem this will perform cell loading for the cells that potentially
   * collide with the segment
   */
  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
                           bool oneHit, iTerrainVector3Array& points) = 0;

  /**
   * Collide set of triangles with the terrain
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
   *
   * \rem this will perform cell loading for the cells that potentially
   * collide with the triangle set
   */
  virtual bool CollideTriangles (const csVector3* vertices,
                       unsigned int tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform* trans,
                       bool oneHit, iTerrainCollisionPairArray& pairs) = 0;

  /**
   * Collide collider with the terrain
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
   *
   * \rem this will perform cell loading for the cells that potentially
   * collide with the collider
   */
  virtual bool Collide (iCollider* collider, float radius,
                       const csReversibleTransform* trans, bool oneHit,
                       iTerrainCollisionPairArray& pairs) = 0;

  /**
   * Get virtual view distance, that is, the distance from camera, at which
   * the cells are preloaded
   *
   * \return virtual view distance
   */
  virtual float GetVirtualViewDistance () const = 0;
  
  /**
   * Set virtual view distance, that is, the distance from camera, at which
   * the cells are preloaded
   *
   * \param distance - new virtual view distance
   */
  virtual void SetVirtualViewDistance (float distance) = 0;

  /**
   * Get automatic preload flag. If it is set, then PreLoadCells is called
   * when rendering an object. Otherwise, you have to call it yourself if
   * you want cell streaming. The default value is true.
   *
   * \return automatic preload flag
   */
  virtual bool GetAutoPreLoad () const = 0;
  
  /**
   * Set automatic preload flag.
   *
   * \param mode - new automatic preload flag
   */
  virtual void SetAutoPreLoad (bool mode) = 0;

  /**
   * Preload all cells that are in the 'virtual view' (that is, the given
   * view, extended to virtual view distance). Preloading is feeder-
   * dependent (that is, cell feeders are free to either implement or not
   * implement it).
   *
   * \param rview - real view
   * \param movable - terrain object
   */
  virtual void PreLoadCells (iRenderView* rview, iMovable* movable) = 0;
  
  /**
   * Query height doing bilinear interpolation. This is equivalent to doing
   * an intersection with vertical ray, except that it is faster.
   *
   * \param pos - object-space position.
   *
   * \return height value
   *
   * \rem this will perform cell loading for the cell that is used to sample
   * height value
   */
  virtual float GetHeight (const csVector2& pos) = 0;
  
  /**
   * Get tangent with bilinear interpolation.
   *
   * \param pos - object-space position.
   *
   * \return tangent value
   *
   * \rem this will perform cell loading for the cell that is used to sample
   * tangent value
   */
  virtual csVector3 GetTangent (const csVector2& pos) = 0;
  
  /**
   * Get binormal with bilinear interpolation.
   *
   * \param pos - object-space position.
   *
   * \return binormal value
   *
   * \rem this will perform cell loading for the cell that is used to sample
   * binormal value
   */
  virtual csVector3 GetBinormal (const csVector2& pos) = 0;

  /**
   * Get normal with bilinear interpolation.
   *
   * \param pos - object-space position.
   *
   * \return normal value
   *
   * \rem this will perform cell loading for the cell that is used to sample
   * normal value
   */
  virtual csVector3 GetNormal (const csVector2& pos) = 0;
};

#endif // __CS_ITERRAIN_TERRAINSYSTEM_H__
