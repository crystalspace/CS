/*
    Copyright (C) 2005-2006 by Jorrit Tyberghein

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

#ifndef __CS_MESHGEN_H__
#define __CS_MESHGEN_H__

#include "csutil/csobject.h"
#include "csutil/array.h"
#include "csutil/weakref.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/hash.h"
#include "csutil/set.h"
#include "csutil/scf_implementation.h"
#include "csutil/floatrand.h"
#include "iutil/selfdestruct.h"
#include "csgeom/box.h"
#include "iengine/mesh.h"
#include "iengine/meshgen.h"
#include "imesh/instmesh.h"
#include "ivaria/terraform.h"

struct iSector;
struct iInstancingMeshState;
class csMeshGenerator;
struct csMGCell;

#define CS_GEOM_MAX_ROTATIONS 16

/**
 * In case a csMGGeom uses an instmesh then this holds the
 * meshes used for a single cell.
 */
struct csMGGeomInstMesh
{
  // @@@ FIXME: only one instmesh per cell for now?
  csRef<iMeshWrapper> instmesh;
  csRef<iInstancingMeshState> instmesh_state;
  csArray<size_t> inst_setaside;
};

/**
 * A single geometry (for a single lod level).
 */
struct csMGGeom
{
  csRef<iMeshFactoryWrapper> factory;
  float maxdistance;
  float sqmaxdistance;

  /**
   * 2-dimensional array of cells with cell_dim*cell_dim entries.
   * This is only used in case this factory represents
   * an instmesh factory.
   */
  csArray<csMGGeomInstMesh> instmeshes;

  /// For every lod level we have a cache of meshes.
  csRefArray<iMeshWrapper> mesh_cache;
  /// For every lod level we have a cache of meshes that are set aside.
  csRefArray<iMeshWrapper> mesh_setaside;

  /**
   * A set of geom instmeshes where we have put aside instances.
   * This is an optimization to prevent having to traverse all cells.
   */
  csSet<csPtrKey<csMGGeomInstMesh> > instmesh_setaside;
};

struct csMGDensityMaterialFactor
{
  iMaterialWrapper* material;
  float factor;
};

/**
 * Geometry implementation for the mesh generator.
 */
class csMeshGeneratorGeometry : public scfImplementation1<
	csMeshGeneratorGeometry, iMeshGeneratorGeometry>
{
private:
  // Array of factories. Every index corresponds with a 'lod' level.
  // This array is sorted automatically.
  csArray<csMGGeom> factories;
  float radius;
  float density;
  float total_max_dist;
  csStringID var_name;

  //density map
  csRef<iTerraFormer> density_map;
  float density_map_factor;
  csStringID density_map_type;

  // Table with density material factors.
  csArray<csMGDensityMaterialFactor> material_factors;
  // Default material factor. Only used in case the table above is not empty.
  float default_material_factor;

  csMeshGenerator* generator;

  csArray<csVector2> *positions;
  int celldim;

public:
  csMeshGeneratorGeometry (csMeshGenerator* generator);
  virtual ~csMeshGeneratorGeometry ();

  void GetDensityMapFactor (float x, float z, float &data);

  float GetTotalMaxDist () const { return total_max_dist; }
  const csArray<csMGDensityMaterialFactor>& GetDensityMaterialFactors () const
  {
    return material_factors;
  }
  float GetDefaultDensityMaterialFactor () const
  {
    return default_material_factor;
  }

  void ResetManualPositions (int new_celldim);

  virtual void AddFactory (iMeshFactoryWrapper* factory, float maxdist);
  virtual size_t GetFactoryCount () const { return factories.Length (); }
  virtual void RemoveFactory (size_t idx);
  virtual iMeshFactoryWrapper* GetFactory (size_t idx)
  {
    return factories[idx].factory;
  }
  virtual float GetMaximumDistance (size_t idx)
  {
    return factories[idx].maxdistance;
  }
  virtual void SetRadius (float radius);
  virtual float GetRadius () const { return radius; }
  virtual void SetDensity (float density);
  virtual float GetDensity () const { return density; }
  virtual void AddDensityMaterialFactor (iMaterialWrapper* material,
  	float factor);
  virtual void SetDensityMap (iTerraFormer* map, float factor, 
    const csStringID & type);
  virtual void AddPositionsFromMap (iTerraFormer* map, const csBox2 &region,
    uint resx, uint resy, float value, const csStringID & type);
  virtual void SetDefaultDensityMaterialFactor (float factor)
  {
    default_material_factor = factor;
  }

  void AddPosition (const csVector2 &pos);

  size_t GetManualPositionCount (size_t cidx) {return positions[cidx].GetSize ();}
  const csVector2 &GetManualPosition (size_t cidx, size_t i){return positions[cidx][i];}

  /**
   * Allocate a new mesh for the given distance. Possibly from the
   * cache if possible. If the distance is too large it will return 0.
   * It will also return an instance_id if the mesh represents an
   * instance from an instmesh.
   */
  csPtr<iMeshWrapper> AllocMesh (int cidx, const csMGCell& cell,
      float sqdist, size_t& lod, size_t& instance_id);

  /**
   * Set aside the mesh temporarily. This is called if we have a mesh that
   * we want to free but we don't free it yet because we might want to allocate
   * it again. Meshes that are put aside are not removed from the sector (that
   * is a rather expensive operation) but they are only put in a queue.
   * AllocMesh() will first check that queue. At the end of the frame you have
   * to call FreeSetAsideMeshes() to really free the remaining meshes that
   * haven't been reused.
   */
  void SetAsideMesh (int cidx, iMeshWrapper* mesh,
      size_t lod, size_t instance_id);

  /**
   * Free all meshes that were put aside and that were not reused by
   * AllocMesh().
   */
  void FreeSetAsideMeshes ();

  /**
   * Move the mesh to some position.
   */
  void MoveMesh (int cidx, iMeshWrapper* mesh, size_t lod, size_t instance_id,
      const csVector3& position, const csMatrix3& matrix);

  /**
   * Get the right lod level for the given squared distance.
   * Returns csArrayItemNotFound if no lod level is defined for the distance.
   */
  size_t GetLODLevel (float sqdist);

  /**
   * Check if this is the right mesh for the given LOD level.
   */
  bool IsRightLOD (float sqdist, size_t current_lod);
};

/**
 * A single position.
 */
struct csMGPosition
{
  csVector3 position;
  /**
   * The type of geometry at this spot.
   * This is basically the index in the 'geometries' table
   * in csMeshGenerator.
   */
  size_t geom_type;

  /**
   * A number between 0 and CS_GEOM_MAX_ROTATIONS indicating how
   * to rotate this object.
   */
  size_t rotation;

  /**
   * A random number between 0 and 1 which controls fade and density
   * scaling for this position.
   */
  float random;

  /**
   * Last used mixmode.
   */
  uint last_mixmode;

  /// An optional mesh for this position. Can be 0.
  iMeshWrapper* mesh;
  /// The LOD level for the mesh above.
  size_t lod;
  /**
   * If the mesh on this position is an instmesh then this
   * number is the id of the instance. Otherwise this id will
   * be csArrayItemNotFound.
   */
  size_t instance_id;

  csMGPosition () : last_mixmode (CS_FX_COPY), mesh (0),
		    instance_id (csArrayItemNotFound) { }
};

struct csMGCell;

/**
 * A block of positions for geometry in a single cell (csMGCell). A cell
 * can be associated with such a position block if the camera was close to
 * it. Otherwise the position block will be put back in the pool for later
 * use (potentially for another cell).
 */
struct csMGPositionBlock
{
  /// Used when block is in 'inuse_blocks'.
  csMGPositionBlock* next, * prev;

  csArray<csMGPosition> positions;

  /// An index back to the cell that holds this block (or ~0).
  size_t parent_cell;

  csMGPositionBlock () : next (0), prev (0),
			 parent_cell (csArrayItemNotFound) { }
};

/**
 * The 2D x/z plane of the box used by the mesh generator is divided
 * in cells.
 */
struct csMGCell
{
  /// Box for this cell (in 2D space).
  csBox2 box;

  /**
   * Block of positions. Can be 0 in case none was generated yet.
   */
  csMGPositionBlock* block;

  /**
   * An array of meshes that are relevant in this cell (for calculating the
   * beam downwards).
   */
  csRefArray<iMeshWrapper> meshes;

  csMGCell () : block (0) { }
};

/**
 * The mesh generator.
 */
class csMeshGenerator : public scfImplementationExt2<csMeshGenerator,
						     csObject,
						     iMeshGenerator,
						     iSelfDestruct>
{
private:
  /// All geometries.
  csRefArray<csMeshGeneratorGeometry> geometries;
  /// The maximum radius for all geometries.
  float total_max_dist;
  float sq_total_max_dist;

  /// Random generator.
  csRandomFloatGen random;

  /// CS_GEOM_MAX_ROTATIONS rotation matrices.
  csMatrix3 rotation_matrices[CS_GEOM_MAX_ROTATIONS];

  /// Meshes on which we will map geometry.
  csRefArray<iMeshWrapper> meshes;

  /// Sector for this generator.
  csWeakRef<iSector> sector;

  /// Alpha and object render priority.
  int alpha_priority;
  int object_priority;

  /// Sample box where we will place geometry.
  csBox3 samplebox;
  float samplefact_x;
  float samplefact_z;
  float samplecellwidth_x;
  float samplecellheight_z;

  /// For density scaling.
  bool use_density_scaling;
  float density_mindist, sq_density_mindist, density_maxdist;
  float density_scale;
  float density_maxfactor;

  /// For alpha scaling.
  bool use_alpha_scaling;
  float alpha_mindist, sq_alpha_mindist, alpha_maxdist;
  float alpha_scale;

  csVector2 last_pos;

  /**
   * If true the cells are correctly set up. This is cleared by
   * changing the sample box.
   */
  bool setup_cells;

  /// 2-dimensional array of cells with cell_dim*cell_dim entries.
  csMGCell* cells;
  int cell_dim;

  /**
   * Here we store the block of cell coordinates that we used the
   * previous frame. We can use this to find out where to deallocate
   * meshes on cells that are no longer used in this frame.
   */
  csRect prev_cells;

  /// Cache of unused position blocks.
  csPDelArray<csMGPositionBlock> cache_blocks;
  /**
   * Position blocks that are used. These blocks are linked to a cell.
   * This list is ordered so that the most recently used blocks are in front.
   */
  csMGPositionBlock* inuse_blocks, * inuse_blocks_last;
  /// Maximum number of blocks (both in 'cache_blocks' as in 'inuse_blocks').
  int max_blocks;

  /**
   * Allocate a block for the given cell.
   */
  void AllocateBlock (int cidx, csMGCell& cell);

  /**
   * Allocate the meshes in this block that are close enough to
   * the given position.
   * This function assumes the cell has a valid block.
   */
  void AllocateMeshes (int cidx, csMGCell& cell, const csVector3& pos);

  /**
   * Generate the positions in the given block.
   */
  void GeneratePositions (int cidx, csMGCell& cell, csMGPositionBlock* block);

  /**
   * Free all meshes in a block.
   */
  void FreeMeshesInBlock (int cidx, csMGCell& cell);

  /// Get the total maximum distance for all geometries.
  float GetTotalMaxDist ();

  /// World coordinate to cell X.
  int GetCellX (float x)
  {
    return int ((x - samplebox.MinX ()) * samplefact_x);
  }
  /// World coordinate to cell Z.
  int GetCellZ (float z)
  {
    return int ((z - samplebox.MinZ ()) * samplefact_z);
  }
  /// Cell coordinate to world X.
  float GetWorldX (int x) { return samplebox.MinX () + x * samplecellwidth_x; }
  /// Cell coordinate to world Z.
  float GetWorldZ (int z) { return samplebox.MinZ () + z * samplecellheight_z; }

  /// Set the fade for a mesh.
  void SetFade (csMGPosition& p, float factor);
  void SetFade (iMeshWrapper* mesh, uint mode);

  /// Statistics.
  size_t CountPositions (int cidx, csMGCell& cell);
  size_t CountAllPositions ();

public:
  csMeshGenerator ();
  virtual ~csMeshGenerator ();

  void SetSector (iSector* sector) { csMeshGenerator::sector = sector; }
  iSector* GetSector () { return sector; }

  /**
   * Allocate blocks. This function will allocate all blocks needed
   * around a certain position and associate those blocks with the
   * appropriate cells. If a new block is needed it will first
   * take one from 'cache_blocks'. If 'cache_blocks' is empty
   * then we take the block we needed last from 'inuse_blocks'.
   */
  void AllocateBlocks (const csVector3& pos);

  /**
   * Make sure that the sample box dependend data is correctly set up.
   */
  void SetupSampleBox ();

  virtual iObject *QueryObject () { return (iObject*)this; }

  virtual void SetDensityScale (float mindist, float maxdist,
  	float maxdensityfactor);
  virtual void SetAlphaScale (float mindist, float maxdist);

  virtual void SetSampleBox (const csBox3& box);
  virtual const csBox3& GetSampleBox () const { return samplebox; }

  virtual void SetCellCount (int number);
  virtual int GetCellCount () const { return cell_dim; }
  virtual void SetBlockCount (int number);
  virtual int GetBlockCount () const { return max_blocks; }

  int GetCellId (const csVector2& pos);

  virtual iMeshGeneratorGeometry* CreateGeometry ();
  virtual size_t GetGeometryCount () const { return geometries.Length (); }
  virtual iMeshGeneratorGeometry* GetGeometry (size_t idx)
  {
    return geometries[idx];
  }
  virtual void RemoveGeometry (size_t idx);

  virtual void AddMesh (iMeshWrapper* mesh) { meshes.Push (mesh); }
  virtual size_t GetMeshCount () const { return meshes.Length (); }
  virtual iMeshWrapper* GetMesh (size_t idx) { return meshes[idx]; }
  virtual void RemoveMesh (size_t idx);

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();
};

#endif // __CS_MESHGEN_H__

