/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007 by Marten Svanfeldt

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

#ifndef __CS_IMESH_TERRAIN2_H__
#define __CS_IMESH_TERRAIN2_H__

/**\file
 * Terrain2 interfaces
 */

#include "csutil/scf_interface.h"
#include "iutil/array.h"
#include "ivideo/shader/shader.h"
#include "imesh/object.h"

class csVector3;
struct csCollisionPair;
struct iCollider;
struct iTerrainCell;
struct iRenderView;
struct iTerrainSystem;
struct iMovable;

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
  /// height data array
  float* data;

  /// array pitch
  size_t pitch;	    
};

/**
 * Locked material data. This class holds an information needed to
 * fill/interpret material map data. The elements are indices of materials
 * in material palette.
 *
 * Two-dimensional material index array is linearized, so additional math is
 * needed to get access to desired values:
 * Instead of data[y][x], use data[y * pitch + x]
 */
struct csLockedMaterialMap
{
  /// material index data array
  unsigned char* data;	

  /// array pitch
  size_t pitch;	
};

/**
 * Locked normal data. This class holds an information needed to
 * fill/interpret normal data. The elements are object space normals.
 *
 * Two-dimensional normal array is linearized, so additional math is
 * needed to get access to desired values:
 * Instead of data[y][x], use data[y * pitch + x]
 */
struct csLockedNormalData
{
  /// normal data array
  csVector3* data;

  /// array pitch
  size_t pitch;	
};

/// Class for an array of csVector3. Used for communication with plugins.
struct iTerrainVector3Array : public iArrayChangeAll<csVector3>
{
  SCF_IARRAYCHANGEALL_INTERFACE(iTerrainVector3Array);
};

/// Class for an array of csCollisionPair. Used for communication with plugins.
struct iTerrainCollisionPairArray : public iArrayChangeAll<csCollisionPair>
{
  SCF_IARRAYCHANGEALL_INTERFACE(iTerrainCollisionPairArray);
};

/// Type of the material palette
typedef csRefArray<iMaterialWrapper> csTerrainMaterialPalette;


/**
 * This is a base class for per-cell collider-specific properties.
 * The classes which hold the collision-related data that is specific to a given
 * cell and collider.
 */
struct iTerrainCellCollisionProperties : public virtual iBase
{
  SCF_INTERFACE (iTerrainCellCollisionProperties, 2, 0, 1);

  /**
   * Get collidable flag (if it is not set, the cell does not collide with
   * anything).
   * 
   * \return collidable flag
   */
  virtual bool GetCollidable () const = 0;
  
  /**
   * Set collidable flag
   * 
   * \param value new flag value
   */
  virtual void SetCollidable (bool value) = 0;

  /**
   * Set named parameter
   *
   * \param param parameter name
   * \param value parameter value
   */
  virtual void SetParameter(const char* param, const char* value) = 0;

  /**
   * Get a copy of the properties object
   */
  virtual csPtr<iTerrainCellCollisionProperties> Clone () = 0;

  /// Get number of parameters this object has set
  virtual size_t GetParameterCount() = 0;

  /// Get name of a parameter
  virtual const char* GetParameterName (size_t index) = 0;

  //@{
  /// Get value of a parameter
  virtual const char* GetParameterValue (size_t index) = 0;
  virtual const char* GetParameterValue (const char* name) = 0;
  //@}
};

/**
 * This is a base class for per-cell renderer-specific properties.
 * The classes which hold the render-related data that is specific to a given
 * cell and renderer.
 * Also provides a shader variable context for the cell.
 */
struct iTerrainCellRenderProperties : public virtual iShaderVariableContext
{
  SCF_INTERFACE (iTerrainCellRenderProperties, 2, 0, 1);

  /**
   * Get visibility flag (if it is not set, the cell does not get rendered)
   * 
   * \return visibility flag
   */
  virtual bool GetVisible () const = 0;
  
  /**
   * Set visibility flag
   * 
   * \param value new flag value
   */
  virtual void SetVisible (bool value) = 0;
  
  /**
   * Set named parameter
   *
   * \param param parameter name
   * \param value parameter value
   */
  virtual void SetParameter (const char* param, const char* value) = 0;

  /**
   * Get a copy of the properties object
   */
  virtual csPtr<iTerrainCellRenderProperties> Clone () = 0;

  /// Get number of parameters this object has set
  virtual size_t GetParameterCount() = 0;

  /// Get name of a parameter
  virtual const char* GetParameterName (size_t index) = 0;

  //@{
  /// Get value of a parameter
  virtual const char* GetParameterValue (size_t index) = 0;
  virtual const char* GetParameterValue (const char* name) = 0;
  //@}
};

/**
 * This is a base class for per-cell feeder-specific properties.
 */
struct iTerrainCellFeederProperties : public virtual iBase
{
  SCF_INTERFACE (iTerrainCellFeederProperties, 3, 2, 0);
  
  /**
   * Set heightmap source.
   */
  virtual void SetHeightmapSource (const char* source, const char* format) = 0;

  /**
   * Set normalmap source.
   */
  virtual void SetNormalMapSource (const char* source) = 0;

  /**
   * Set materialmap source
   */
  virtual void SetMaterialMapSource (const char* source) = 0;

  /**
   * Set height offset
   */
  virtual void SetHeightOffset (float offset) = 0;

  /**
   * Add an explicit material/alpha-map pair
   */
  virtual void AddAlphaMap (const char* material, const char* alphaMapSource) = 0;

  /**
   * Set named parameter
   *
   * \param param parameter name
   * \param value parameter value
   */
  virtual void SetParameter (const char* param, const char* value) = 0;

  /**
   * Get a copy of the properties object
   */
  virtual csPtr<iTerrainCellFeederProperties> Clone () = 0;

  /// Get number of parameters this object has set
  virtual size_t GetParameterCount() = 0;

  /// Get name of a parameter
  virtual const char* GetParameterName (size_t index) = 0;

  //@{
  /// Get value of a parameter
  virtual const char* GetParameterValue (size_t index) = 0;
  virtual const char* GetParameterValue (const char* name) = 0;
  //@}


  /// Get number of alpha maps this object has set
  virtual size_t GetAlphaMapCount() = 0;

  /// Get material of an alpha map
  virtual const char* GetAlphaMapMaterial (size_t index) = 0;

  //@{
  /// Get source of an alpha map
  virtual const char* GetAlphaMapSource (size_t index) = 0;
  virtual const char* GetAlphaMapSource (const char* material) = 0;
  //@}

  //@{
  virtual void SetHeightmapSmooth (bool doSmooth) = 0;
  virtual bool GetHeightmapSmooth () const = 0;
  //@}
};

/**
 * Provides an interface for reading cell data
 */
struct iTerrainDataFeeder : public virtual iBase
{
  SCF_INTERFACE (iTerrainDataFeeder, 2, 0, 0);

  /**
   * Create an object that implements iTerrainCellFeederProperties
   * This object will be stored in the cell. This function gets invoked
   * at cells creation.
   *
   * \return properties object
   */
  virtual csPtr<iTerrainCellFeederProperties> CreateProperties () = 0;

  /**
   * Start cell data preloading (in case of threaded/async loading). This is
   * triggered by TerrainSystem::PreLoadCells, which is either called by user
   * or called automatically while rendering terrain.
   *
   * \param cell cell to start preloading for
   *
   * \return preloading success flag
   */
  virtual bool PreLoad (iTerrainCell* cell) = 0;
  
  /**
   * Load cell data. After the completion of this call the cell should have
   * all necessary information.
   *
   * \param cell cell to load
   *
   * \return loading success flag
   */
  virtual bool Load (iTerrainCell* cell) = 0;

  /**
   * Set feeder-dependent parameter
   *
   * \param param parameter name
   * \param value parameter value
   */
  virtual void SetParameter (const char* param, const char* value) = 0;
};

/**
 * Return structure for the
 * iTerrainCollider::CollideSegment(iTerrainCell*, const csVector3&, const csVector3&)
 * routine.
 */
struct csTerrainColliderCollideSegmentResult
{
  /// True if we hit.
  bool hit;

  /// Intersection point in world space.
  csVector3 isect;

  //@{
  /// Triangle we hit.
  csVector3 a, b, c;	
  //@}
  
  csTerrainColliderCollideSegmentResult()
   : hit (false), isect (0), a (0), b (0), c (0) {}
};

/// Provides an interface for custom collision
struct iTerrainCollider : public virtual iBase
{
  SCF_INTERFACE (iTerrainCollider, 3, 0, 0);

  /**
   * Create an object that implements iTerrainCellCollisionProperties
   * This object will be stored in the cell. This function gets invoked
   * at cells creation.
   *
   * \return properties object
   */
  virtual csPtr<iTerrainCellCollisionProperties> CreateProperties () = 0;
  
  /**
   * Collide segment with cell
   *
   * \param cell cell
   * \param start segment start (specified in object space)
   * \param end segment end (specified in object space)
   * \param oneHit if this is true, than stop on finding the first
   * intersection point (the closest to the segment start); otherwise, detect
   * all intersections
   * \param points destination point array
   * 
   * \return true if there were any intersections, false if there were none
   */
  virtual bool CollideSegment (iTerrainCell* cell, const csVector3& start,
                               const csVector3& end, bool oneHit, 
                               iTerrainVector3Array* points) = 0;

  /**
   * Collide segment with cell.
   * Stops on finding the first intersection point (the closest to the segment
   * start).
   * \param cell cell
   * \param start segment start (specified in object space)
   * \param end segment end (specified in object space)
   * \param hitPoint receives the intersection point
   * 
   * \return true if there was an intersections, false if there was none
   */
  virtual bool CollideSegment (iTerrainCell* cell, const csVector3& start,
                               const csVector3& end,
			       csVector3& hitPoint) = 0;
			       
  /**
   * Collide segment with cell
   *
   * \param cell cell
   * \param start segment start (specified in object space)
   * \param end segment end (specified in object space)
   * 
   * \return a csTerrainColliderCollideSegmentResult instance
   * indicating what we hit.
   */
  virtual csTerrainColliderCollideSegmentResult CollideSegment (
      iTerrainCell* cell, const csVector3& start, const csVector3& end) = 0;

  /**
   * Collide set of triangles with cell
   *
   * \param cell cell
   * \param vertices vertex array
   * \param tri_count triangle count
   * \param indices vertex indices, 3 indices for each triangle
   * \param radius radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   */
  virtual bool CollideTriangles (iTerrainCell* cell, const csVector3* vertices,
                                 size_t tri_count,
                                 const unsigned int* indices, float radius,
                                 const csReversibleTransform& trans,
                                 bool oneHit, iTerrainCollisionPairArray* pairs) = 0;

  /**
   * Collide collider with cell
   *
   * \param cell cell
   * \param collider collider
   * \param radius radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   */
  virtual bool Collide (iTerrainCell* cell, iCollider* collider,
                       float radius, const csReversibleTransform& trans,
                       bool oneHit, iTerrainCollisionPairArray* pairs) = 0;
};


/// Provides an interface for custom rendering
struct iTerrainRenderer : public virtual iBase
{
  SCF_INTERFACE (iTerrainRenderer, 3, 0, 0);

  /**
   * Create an object that implements iTerrainCellRenderProperties
   * This object will be stored in the cell. This function gets invoked
   * at cells creation.
   *
   * \return properties object
   */
  virtual csPtr<iTerrainCellRenderProperties> CreateProperties () = 0;

  /**
   * Connect to a given terrain system. 
   * Setup any per-terrain render data
   */
  virtual void ConnectTerrain (iTerrainSystem* system) = 0;

  /**
   * Disconnect from a given terrain system.
   */
  virtual void DisconnectTerrain (iTerrainSystem* system) = 0;


  /**
   * Render the visible cells
   *
   * \param n output value, that will contain the size of the resulting
   * mesh array
   * \param rview view that was used for rendering
   * \param movable the terrain object
   * \param frustum_mask frustum mask
   * \param cells array with visible cells
   * \param cell_count number of visible cells
   *
   * \return array of render meshes
   */
  virtual CS::Graphics::RenderMesh** GetRenderMeshes (int& n, iRenderView* rview,
                                   iMovable* movable, uint32 frustum_mask,
                                   const csArray<iTerrainCell*>& cells) = 0;

  
  /**
   * Indicates that the material palette has been changed, and that the
   * renderer should update its internal structures to reflect the changes.
   *
   * \param material_palette new material palette
   */
  virtual void OnMaterialPaletteUpdate (const csTerrainMaterialPalette&
                                        material_palette) = 0;

  /**
   * Indicates that the cells material mask has been changed (while
   * unlocking the cell material map data either by a feeder or by a user-
   * provided functions or while setting the new mask with the respective
   * functions), and that the renderer should update its internal structures
   * to reflect the changes.
   *
   * \param cell cell with the changed data   
   * \param rectangle rectangle that was updated
   * \param materialMap the updated material map
   * \param pitch data pitch
   */
  virtual void OnMaterialMaskUpdate (iTerrainCell* cell, 
    const csRect& rectangle, const unsigned char* materialMap, size_t pitch) = 0; 

  /**
   * Indicates that the cells material mask has been changed (while
   * unlocking the cell material map data either by a feeder or by a user-
   * provided functions or while setting the new mask with the respective
   * functions), and that the renderer should update its internal structures
   * to reflect the changes.
   *
   * \param cell cell with the changed data   
   * \param materialIdx specific material index
   * \param rectangle rectangle that was updated
   * \param materialMap the updated material map
   * \param pitch data pitch
   */
  virtual void OnMaterialMaskUpdate (iTerrainCell* cell, size_t materialIdx, 
    const csRect& rectangle, const unsigned char* materialMap, size_t pitch) = 0; 

  /**
   * Indicate that the cells alpha-map matching a given material have changed
   * and that the renderer should update its internal structures.
   *
   * \param cell cell with changed data
   * \param material material
   * \param alphaMap the alpha map
   */
  virtual void OnAlphaMapUpdate (iTerrainCell* cell,
    iMaterialWrapper* material, iImage* alphaMap) = 0;
};

/// Callbacks for cell height data modifications
struct iTerrainCellHeightDataCallback : public virtual iBase
{
  SCF_INTERFACE (iTerrainCellHeightDataCallback, 1, 0, 0);

  /**
   * Callback called when the height data of a terrain cell is modified.
   *
   * \param cell updated cell
   * \param rectangle rectangle of coordinates that were updated
   */
  virtual void OnHeightUpdate (iTerrainCell* cell, const csRect& rectangle) = 0;
};

/// Callbacks for cell load/unload events
struct iTerrainCellLoadCallback : public virtual iBase
{
  SCF_INTERFACE (iTerrainCellLoadCallback, 1, 0, 0);

  /**
   * Callback on loading of a terrain cell.
   *
   * \param cell loaded cell
   */
  virtual void OnCellLoad (iTerrainCell* cell) = 0;

  /**
   * Callback on pre-loading of a terrain cell.
   *
   * \param cell pre-loaded cell
   */
  virtual void OnCellPreLoad (iTerrainCell* cell) = 0;

  /**
   * Callback when unloading a terrain cell.
   *
   * \param cell unloaded cell
   */
  virtual void OnCellUnload (iTerrainCell* cell) = 0;
};

struct iTerrainFactoryCell;

/**
 * This class represents the terrain object as a set of cells. The object
 * can be rendered and collided with. To gain access to some operations that
 * are done at cell level you might want to use cell quering functions
 * (GetCell)
 */
struct iTerrainSystem : public virtual iBase
{
  SCF_INTERFACE (iTerrainSystem, 3, 0, 0);

  /**
   * Query a cell by name
   *
   * \param name name of cell
   * \param load set if cell should be loaded if it isn't. Default is not
   * to load cell data.
   * \return pointer to the cell with the given name, or NULL, if none found
   */
  virtual iTerrainCell* GetCell (const char* name, bool loadData = false) = 0;

  /**
   * Query a cell by position
   *
   * \param load set if cell should be loaded if it isn't. Default is not
   * to load cell data.
   * \return pointer to the first cell which intersects with the vertical ray
   * of given position, or NULL if none found
   *
   * \rem this will perform cell loading if the resulted cell was not
   * completely loaded
   */
  virtual iTerrainCell* GetCell (const csVector2& pos, bool loadData = false) = 0;

  /**
   * Query a cell by index (0 to GetCellCount ()).
   *
   * \param load set if cell should be loaded if it isn't. Default is not to
   * load cell data.
   */
  virtual iTerrainCell* GetCell (size_t index, bool loadData = false) = 0;

  /**
   * Get total number of cells in terrain (loaded or not)
   */
  virtual size_t GetCellCount () const = 0;

  /**
   * Get material palette. The material map indices index this array.
   *
   * \return material palette
   */
  virtual const csTerrainMaterialPalette& GetMaterialPalette () const = 0;
  
  /**
   * Set a new material palette.
   *
   * \param array new  material palette
   */
  virtual void SetMaterialPalette (const csTerrainMaterialPalette& array) = 0;

  /**
   * Collide segment with the terrain.
   * \param start segment start (specified in object space)
   * \param end segment end (specified in object space)
   * \param oneHit if this is true, than stop on finding the first
   * intersection point (the closest to the segment start); otherwise, detect
   * all intersections
   * \param points destination point array
   * 
   * \return true if there were any intersections, false if there were none
   *
   * \rem this will perform cell loading for the cells that potentially
   * collide with the segment
   *
   * \rem this will not perform collision for cells that have Collideable
   * property set to false
   */
  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
                           bool oneHit, iTerrainVector3Array* points,
                           iMaterialArray* materials) = 0;
			   
  /**
   * Collide segment with the terrain.
   * Stops on finding the first intersection point (the closest to the segment
   * start).
   * \param start segment start (specified in object space)
   * \param end segment end (specified in object space)
   * \param hitPoint receives intersection point
   * \param hitMaterial receives material at intersection point if not null
   * 
   * \return true if there was an intersections, false if there were none
   *
   * \rem this will perform cell loading for the cells that potentially
   * collide with the segment
   *
   * \rem this will not perform collision for cells that have Collideable
   * property set to false
   */
  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
			       csVector3& hitPoint,
			       iMaterialWrapper** hitMaterial) = 0;

  /**
   * Collide segment with the terrain
   *
   * \param start segment start (specified in object space)
   * \param end segment end (specified in object space)
   * \param use_ray if true then use a ray instead of a segment
   * (default false).
   * 
   * \return the intersection result.
   *
   * \rem this will perform cell loading for the cells that potentially
   * collide with the segment
   *
   * \rem this will not perform collision for cells that have Collideable
   * property set to false
   */
  virtual csTerrainColliderCollideSegmentResult CollideSegment (
      const csVector3& start, const csVector3& end,
      bool use_ray = false) = 0;

  /**
   * Collide set of triangles with the terrain
   *
   * \param vertices vertex array
   * \param tri_count triangle count
   * \param indices vertex indices, 3 indices for each triangle
   * \param radius radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   *
   * \rem this will perform cell loading for the cells that potentially
   * collide with the triangle set
   *
   * \rem this will not perform collision for cells that have Collideable
   * property set to false
   */
  virtual bool CollideTriangles (const csVector3* vertices,
                       size_t tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform& trans,
                       bool oneHit, iTerrainCollisionPairArray* pairs) = 0;

  /**
   * Collide collider with the terrain
   *
   * \param collider collider
   * \param radius radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   *
   * \rem this will perform cell loading for the cells that potentially
   * collide with the collider
   *
   * \rem this will not perform collision for cells that have Collideable
   * property set to false
   */
  virtual bool Collide (iCollider* collider, float radius,
                       const csReversibleTransform& trans, bool oneHit,
                       iTerrainCollisionPairArray* pairs) = 0;

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
   * \param distance new virtual view distance
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
   * \param mode new automatic preload flag
   */
  virtual void SetAutoPreLoad (bool mode) = 0;

  /**
   * Preload all cells that are in the 'virtual view' (that is, the given
   * view, extended to virtual view distance). Preloading is feeder-
   * dependent (that is, cell feeders are free to either implement or not
   * implement it).
   *
   * \param rview real view
   * \param movable terrain object
   *
   * \rem this will not perform preloading for cells that have Visible
   * property set to false
   */
  virtual void PreLoadCells (iRenderView* rview, iMovable* movable) = 0;
  
  /**
   * Query height doing bilinear interpolation. This is equivalent to doing
   * an intersection with vertical ray, except that it is faster.
   *
   * \param pos object-space position.
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
   * \param pos object-space position.
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
   * \param pos object-space position.
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
   * \param pos object-space position.
   *
   * \return normal value
   *
   * \rem this will perform cell loading for the cell that is used to sample
   * normal value
   */
  virtual csVector3 GetNormal (const csVector2& pos) = 0;

  /**
   * Get maximum number of loaded cells
   *
   * \return maximum number of loaded cells
   */
  virtual size_t GetMaxLoadedCells () const = 0;

  /**
   * Set maximum number of loaded cells. If the number of loaded cells becomes
   * greater than this value (in the process of cell loading), the cell with
   * least recent usage is unloaded.
   *
   * \param value maximum number of loaded cells
   */
  virtual void SetMaxLoadedCells (size_t value) = 0;

  /**
   * Unload cells to satisfy the requirement of max loaded cell count
   */
  virtual void UnloadOldCells () = 0;

  /**
   * Add a listener to the cell load/unload callback
   */
  virtual void AddCellLoadListener (iTerrainCellLoadCallback* cb) = 0;

  /**
   * Remove a listener to the cell load/unload callback
   */
  virtual void RemoveCellLoadListener (iTerrainCellLoadCallback* cb) = 0;
  
  /**
   * Add a listener to the cell height update callback
   */
  virtual void AddCellHeightUpdateListener (iTerrainCellHeightDataCallback* cb) = 0;

  /**
   * Remove a listener to the cell height update callback
   */
  virtual void RemoveCellHeightUpdateListener (iTerrainCellHeightDataCallback* cb) = 0;

  /**
   * Add a cell to the terrain instance based on the given iTerrainFactoryCell.
   *
   * \return added cell
   * \rem If you change the renderer, collider or feeder after adding cells
   * you might get into trouble.
   */
  virtual iTerrainCell* AddCell (iTerrainFactoryCell*) = 0;

  /**
   * Remove the given cell from this instance
   */
  virtual void RemoveCell (iTerrainCell*) = 0;
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
  SCF_INTERFACE (iTerrainCell, 6, 0, 0);

  /// Enumeration that specifies current cell state
  enum LoadState
  {
    NotLoaded,    /**< cell is not loaded in memory, do not do any operations
                       with it directly */
    PreLoaded,    /**< cell is in preload state, do not do any operations with 
                       it directly */
    Loaded        ///< cell is loaded, you can use it as you like
  };

  /**
   * Get cell's current loading state.
   *
   * \return cell's current loading state
   */
  virtual LoadState GetLoadState () const = 0;

  /**
   * Set cell's current loading state.
   *
   * If the cell's current state is equal to the passed state, nothing happens.
   * If the cell was not loaded (NotLoaded state), then it is put into the
   * passed state (either preloading or loading is started)
   * If the cell was loaded, then it is unloaded in case of NotLoaded state.
   * Passing PreLoaded state has no effect.
   * If the cell was being preloaded, then it is loaded in case of Loaded state.
   * Passing NotLoaded state has no effect (note, that if you want to stop
   * preloading, you'll have to finish it (SetLoadState (Loaded)) and then
   * unload the cell (SetLoadState (NotLoaded)).)
   *
   * \param state cell's new loading state
   */
  virtual void SetLoadState (LoadState state) = 0;

  /**
   * Get the terrain to which the cell belongs
   *
   * \return terrain object
   */
  virtual iTerrainSystem* GetTerrain () = 0;

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

  /**
   * Get cell feeder properties. Returns pointer to a feeder-specific class
   * though it is possible to check/change some general properties.
   *
   * \return cell feeder properties
   */
  virtual iTerrainCellFeederProperties* GetFeederProperties () const = 0;

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
   * \param rectangle the rectangle which you want to lock.
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
   * Get normal data (for reading purposes: do not modify it!)
   * This can be used to perform very fast normal lookups.
   *
   * \return cell normal data
   */
  virtual csLockedNormalData GetNormalData () = 0;
  
  /**
   * Lock an area of normal data (for reading/writing purposes)
   * If you want to lock the whole cell, use the rectangle
   * csRect(0, 0, grid width, grid height).
   *
   * Only one area may be locked at a time, locking more than once results in
   * undefined behaviour.
   *
   * \param rectangle the rectangle which you want to lock.
   *
   * \return cell normal data
   */
  virtual csLockedNormalData LockNormalData (const csRect& rectangle) = 0;
  
  /**
   * Commit changes to height data. Use it after changing the desired height values.
   *
   * Unlocking the cell that was not locked results in undefined behaviour
   */
  virtual void UnlockNormalData () = 0;

  /**
   * Recalculates the cell normals.
   */
  virtual void RecalculateNormalData () = 0;

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
   * Get material persistent flag. If it is true, material data is stored in
   * the cell (that makes updating material data faster and makes material data
   * lock read/write, but it means larger memory overhead)
   */
  virtual bool GetMaterialPersistent() const = 0;

  /**
   * Lock an area of material map (practically write-only, reading the
   * values will not produce sensible values if you did not just write
   * them that is, the returned block memory is a read-write one, but
   * it is a temporary block of memory filled with garbage).
   * Note, that if you created cell with 'material_persistent' flag, the
   * lock is read/write.
   *
   * If you want to lock the whole cell, use the rectangle
   * csRect(0, 0, material map width, material map height).
   *
   * Only one area may be locked at a time, locking more than once results in
   * undefined behaviour.
   *
   * \param rectangle the rectangle which you want to lock.
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
   * \param material material index
   * \param image an image of format CS_IMGFMT_PALETTED8
   */
  virtual void SetMaterialMask (unsigned int material, iImage* image) = 0;
  
  /**
   * Set new material mask for the specified material.
   *
   * This function will do image rescaling if needed (i.e. if material map
   * dimensions and image dimensions do not match).
   *
   * \param material material index
   * \param data linearized array with material indices
   * \param width image width
   * \param height image height
   */
  virtual void SetMaterialMask (unsigned int material, const unsigned char*
                          data, unsigned int width, unsigned int height) = 0;
  
  /**
   * Set alpha mask for a specified material.
   *
   * \param material the material
   * \param alphaMap the alpha map to use
   */
  virtual void SetAlphaMask (iMaterialWrapper* material, iImage* alphaMap) = 0;

  /**
   * Set base material for the cell.
   *
   * \param material material handle of base material
   */
  virtual void SetBaseMaterial (iMaterialWrapper* material) = 0;

  /**
   * Get base material for the cell
   */
  virtual iMaterialWrapper* GetBaseMaterial () const = 0;

  /**
   * Set the optional alpha-splat material for the cell.
   *
   * \param material material handle of the alpha-splat material.
   */
  virtual void SetAlphaSplatMaterial (iMaterialWrapper* material) = 0;

  /**
   * Get the optional alpha-splat material for the cell.
   *
   * \return nullptr if no material, else the material.
   */
  virtual iMaterialWrapper* GetAlphaSplatMaterial () const = 0;

  /**
   * Collide segment with cell (using the collider)
   *
   * \param start segment start (specified in object space)
   * \param end segment end (specified in object space)
   * \param oneHit if this is true, than stop on finding the first
   * intersection point (the closest to the segment start); otherwise, detect
   * all intersections
   * \param points destination point array
   * 
   * \return true if there were any intersections, false if there were none
   */
  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
                           bool oneHit, iTerrainVector3Array* points) = 0;

  /**
   * Collide segment with cell.
   * Stops on finding the first intersection point (the closest to the segment
   * start).
   * \param cell cell
   * \param start segment start (specified in object space)
   * \param end segment end (specified in object space)
   * \param hitPoint receives the intersection point
   * 
   * \return true if there was an intersections, false if there was none
   */
  virtual bool CollideSegment (const csVector3& start, const csVector3& end,
			       csVector3& hitPoint) = 0;

  /**
   * Collide set of triangles with cell (using the collider)
   *
   * \param vertices vertex array
   * \param tri_count triangle count
   * \param indices vertex indices, 3 indices for each triangle
   * \param radius radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   */
  virtual bool CollideTriangles (const csVector3* vertices,
                       size_t tri_count,
                       const unsigned int* indices, float radius,
                       const csReversibleTransform& trans,
                       bool oneHit, iTerrainCollisionPairArray* pairs) = 0;

  /**
   * Collide collider with cell (using the collider)
   *
   * \param collider collider
   * \param radius radius of the bounding sphere surrounding the given set
   * of triangles (used for fast rejection)
   * \param trans triangle set transformation (vertices' coordinates are
   * specified in the space defined by this transformation)
   * \param oneHit if this is true, than stop on finding the first
   * collision pair; otherwise, detect all collisions
   * \param points destination collision pair array
   * 
   * \return true if there were any collisions, false if there were none
   */
  virtual bool Collide (iCollider* collider, float radius,
                       const csReversibleTransform& trans, bool oneHit,
                       iTerrainCollisionPairArray* pairs) = 0;

  /**
   * Query height, that is, do a lookup on height table. For a set of
   * lookups, use GetHeightData for efficiency reasons.
   *
   * \param x x coordinate (from 0 to grid width - 1 all inclusive)
   * \param y y coordinate (from 0 to grid height - 1 all inclusive)
   *
   * \return height value
   */
  virtual float GetHeight (int x, int y) const = 0;

  /**
   * Query height doing bilinear interpolation. This is equivalent to doing
   * an intersection with vertical ray, except that it is faster.
   *
   * \param pos object-space position.
   *
   * \return height value
   */
  virtual float GetHeight (const csVector2& pos) const = 0;
  
  /**
   * Get tangent value.
   *
   * \param x x coordinate (from 0 to grid width - 1 all inclusive)
   * \param y y coordinate (from 0 to grid height - 1 all inclusive)
   *
   * \return tangent value
   */
  virtual csVector3 GetTangent (int x, int y) const = 0;
  
  /**
   * Get tangent with bilinear interpolation.
   *
   * \param pos object-space position.
   *
   * \return tangent value
   */
  virtual csVector3 GetTangent (const csVector2& pos) const = 0;

  /**
   * Get binormal value.
   *
   * \param x x coordinate (from 0 to grid width - 1 all inclusive)
   * \param y y coordinate (from 0 to grid height - 1 all inclusive)
   *
   * \return binormal value
   */
  virtual csVector3 GetBinormal (int x, int y) const = 0;
  
  /**
   * Get binormal with bilinear interpolation.
   *
   * \param pos object-space position.
   *
   * \return binormal value
   */
  virtual csVector3 GetBinormal (const csVector2& pos) const = 0;

  /**
   * Get normal value.
   *
   * \param x x coordinate (from 0 to grid width - 1 all inclusive)
   * \param y y coordinate (from 0 to grid height - 1 all inclusive)
   *
   * \return normal value
   */
  virtual csVector3 GetNormal (int x, int y) const = 0;
  
  /**
   * Get normal with bilinear interpolation.
   *
   * \param pos object-space position.
   *
   * \return normal value
   */
  virtual csVector3 GetNormal (const csVector2& pos) const = 0;

  /// Get render-specific data. Only to be used by renderer plugin.
  virtual csRefCount* GetRenderData () const = 0;

  /// Set render-specific data. Only to be used by renderer plugin.
  virtual void SetRenderData (csRefCount* data) = 0;

  /// Get collider-specific data. Only to be used by collision plugin.
  virtual csRefCount* GetCollisionData () const = 0;

  /// Set collider-specific data. Only to be used by collision plugin.
  virtual void SetCollisionData (csRefCount* data) = 0;

  /// Get feeder-specific data. Only to be used by feeder plugin.
  virtual csRefCount* GetFeederData () const = 0;

  /// Set feeder-specific data. Only to be used by feeder plugin.
  virtual void SetFeederData (csRefCount* data) = 0;

  /**
   * Set name of this cell.
   * \warning The cell name is used to map object cells to factory cells.
   *   Changing the name of a cell does not change any existing mappings,
   *   however, it will take effect when saving the mesh. If that is done
   *   careless changing of cell names can confuse the mapping of cells.
   */
  virtual void SetName (const char* name) = 0;

  /**
   * Set optional splat base material for the cell.
   * The splat base material is rendered before splatting or
   * alpha-splatting is rendered for the cell.
   * \param material material handle of spalt base material.
   */
  virtual void SetSplatBaseMaterial (iMaterialWrapper* material) = 0;
  
  /// Get splat base material for the cell.
  virtual iMaterialWrapper* GetSplatBaseMaterial () const = 0;

  /**
   * Get tangent data (for reading purposes: do not modify it!).
   * \return cell tangent data
   */
  virtual csLockedNormalData GetTangentData () = 0;
  
  /**
   * Get tangent data (for reading purposes: do not modify it!).
   * \return cell tangent data
   */
  virtual csLockedNormalData GetBitangentData () = 0;
};

/// Factory representation of a cell
struct iTerrainFactoryCell : public virtual iBase
{
  SCF_INTERFACE (iTerrainFactoryCell, 2, 0, 1);

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

  /**
   * Get cell feeder properties. Returns pointer to a feeder-specific class
   * though it is possible to check/change some general properties.
   *
   * \return cell feeder properties
   */
  virtual iTerrainCellFeederProperties* GetFeederProperties () const = 0;

  /**
   * Set base material for the cell.
   *
   * \param material material handle of base material.
   */
  virtual void SetBaseMaterial (iMaterialWrapper* material) = 0;

  /**
   * Set the optional alpha-splat material for the cell.
   *
   * \param material material handle of the alpha-splat material.
   */
  virtual void SetAlphaSplatMaterial (iMaterialWrapper* material) = 0;

  /// Get name of this cell
  virtual const char* GetName() = 0;

  /// Set name of this cell
  virtual void SetName (const char* name) = 0;

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
   * Get base material for the cell.
   */
  virtual iMaterialWrapper* GetBaseMaterial () const = 0;

  /**
   * Get the optional alpha-splat material for the cell.
   *
   * \return nullptr if no material, else the material.
   */
  virtual iMaterialWrapper* GetAlphaSplatMaterial () const = 0;

  /**
   * Get material persistent flag. If it is true, material data is stored in
   * the cell (that makes updating material data faster and makes material data
   * lock read/write, but it means larger memory overhead)
   */
  virtual bool GetMaterialPersistent() const = 0;

  /**
   * Set grid width. It will be changed to match the grid
   * width requirements`- it will be 2^n + 1.
   */
  virtual void SetGridWidth (int w) = 0;
  
  /**
   * Set grid height. It will be changed to match the grid
   * width requirements`- equal to grid width.
   */
  virtual void SetGridHeight (int h) = 0;

  /**
   * Set cell position (in object space). X and Y components specify the
   * offsets along X and Z axes, respectively.
   */
  virtual void SetPosition (const csVector2& pos) = 0;
  
  /**
   * Set cell size (in object space). X and Y components specify the
   * sizes along X and Z axes, respectively. Z component specifies height
   * scale (warning: it is used only at loading stage, after that all scales
   * are in object space).
   */
  virtual void SetSize (const csVector3& size) = 0;

  /**
   * Set material map width (essentially a width of both material array and
   * material masks, if any).
   */
  virtual void SetMaterialMapWidth (int w) = 0;
  
  /**
   * Set material map height (essentially a height of both material array and
   * material masks, if any).
   */
  virtual void SetMaterialMapHeight (int h) = 0;

  /**
   * Set material persistent flag. 
   * \sa GetMaterialPersistent
   */
  virtual void SetMaterialPersistent (bool flag) = 0;

  /**
   * Set optional splat base material for the cell.
   * The splat base material is rendered before splatting or
   * alpha-splatting is rendered for the cell.
   * \param material material handle of spalt base material.
   */
  virtual void SetSplatBaseMaterial (iMaterialWrapper* material) = 0;
  
  /// Get splat base material for the cell.
  virtual iMaterialWrapper* GetSplatBaseMaterial () const = 0;
};

/// Provides an interface for creating terrain system
struct iTerrainFactory : public virtual iBase
{
  SCF_INTERFACE (iTerrainFactory, 2, 0, 3);

  /**
   * Set desired renderer (there is a single renderer for the whole terrain)
   *
   * \param renderer new renderer
   */
  virtual void SetRenderer (iTerrainRenderer* renderer) = 0;
  
  /**
   * Set desired collider (there is a single collider for the whole terrain)
   *
   * \param collider new collider
   */
  virtual void SetCollider (iTerrainCollider* collider) = 0;

  /**
   * Set desired feeder (there is a single feeder for the whole terrain)
   *
   * \param feeder new feeder
   */
  virtual void SetFeeder (iTerrainDataFeeder* feeder) = 0;
  
  /**
   * Add cell to the terrain
   *
   * \param name optional cell name
   * \param grid_width grid width. It will be changed to match the grid
   * width requirements. See iTerrainCell::GetGridWidth
   * \param grid_height grid height. It will be changed to match the grid
   * height requirements. See iTerrainCell::GetGridHeight
   * \param material_width material map width
   * \param material_height material map height
   * \param material_persistent true if you want to store material data
   * (that makes updating material data faster and makes material data lock
   * read/write, but it means larger memory overhead)
   * \param position cell object-space position
   * \param size cell object-space size and height scale
   *
   * \return added cell
   * \rem If you change the renderer, collider or feeder after adding cells
   * you might get into trouble.
   */
  virtual iTerrainFactoryCell* AddCell (const char* name, 
    int gridWidth, int gridHeight, int materialMapWidth,
    int materialMapHeight, bool materiaMapPersistent,
    const csVector2& position, const csVector3& size) = 0;
  
  /**
   * Set maximum number of loaded cells. See iTerrainSystem::SetMaxLoadedCells
   *
   * \param number maximum number of loaded cells
   */
  virtual void SetMaxLoadedCells (size_t number) = 0;
  
  /**
   * Set virtual view distance, that is, the distance from camera, at which
   * the cells are preloaded
   *
   * \param distance new virtual view distance
   */
  virtual void SetVirtualViewDistance (float distance) = 0;
  
  /**
   * Set automatic preload flag.
   *
   * \param mode new automatic preload flag
   */
  virtual void SetAutoPreLoad (bool mode) = 0;


  /**
   * Get renderer for the whole terrain
   */
  virtual iTerrainRenderer* GetRenderer () = 0;
  
  /**
   * Get collider for the whole terrain
   */
  virtual iTerrainCollider* GetCollider () = 0;

  /**
   * Set feeder for the whole terrain
   */
  virtual iTerrainDataFeeder* GetFeeder () = 0;

  /**
   * Get maximum number of loaded cells.
   * \param number maximum number of loaded cells
   */
  virtual size_t GetMaxLoadedCells () = 0;

  /// Get number of cells in this factory
  virtual size_t GetCellCount () = 0;

  /// Get a cell in this factory
  virtual iTerrainFactoryCell* GetCell (size_t index) = 0;

  /// Get pseudo-cell with default settings for all cells
  virtual iTerrainFactoryCell* GetDefaultCell () = 0;

  /**
   * Add cell to the terrain
   *
   * \return added cell
   * \rem If you change the renderer, collider or feeder after adding cells
   * you might get into trouble.
   */
  virtual iTerrainFactoryCell* AddCell () = 0;

  /// Get a cell in this factory by name
  virtual iTerrainFactoryCell* GetCell (const char* name) = 0;

  /// Remove the given cell from this factory
  virtual void RemoveCell (iTerrainFactoryCell*) = 0;
};


#endif
