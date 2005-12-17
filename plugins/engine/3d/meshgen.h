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

#ifndef __CS_MESHGEN_H__
#define __CS_MESHGEN_H__

#include "csutil/csobject.h"
#include "csutil/array.h"
#include "csutil/parray.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csgeom/box.h"
#include "iengine/mesh.h"
#include "iengine/meshgen.h"

/**
 * Geometry implementation for the mesh generator.
 */
class csMeshGeneratorGeometry : public scfImplementation1<
	csMeshGeneratorGeometry, iMeshGeneratorGeometry>
{
private:
  csRefArray<iMeshFactoryWrapper> factories;
  csArray<float> maxdistances;
  float radius;
  float density;

public:
  csMeshGeneratorGeometry ();
  virtual ~csMeshGeneratorGeometry () { }

  virtual void AddFactory (iMeshFactoryWrapper* factory, float maxdist);
  virtual size_t GetFactoryCount () const { return factories.Length (); }
  virtual void RemoveFactory (size_t idx);
  virtual iMeshFactoryWrapper* GetFactory (size_t idx)
  {
    return factories[idx];
  }
  virtual float GetMaximumDistance (size_t idx)
  {
    return maxdistances[idx];
  }
  virtual void SetRadius (float radius);
  virtual float GetRadius () const { return radius; }
  virtual void SetDensity (float density);
  virtual float GetDensity () const { return density; }
};

/**
 * A single position.
 */
struct csMGPosition
{
  csVector3 position;
  /// The type of geometry at this spot.
  int geom_type;
  /// An optional mesh for this position. Can be 0.
  iMeshWrapper* mesh;

  csMGPosition () : mesh (0) { }
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
  csArray<csMGPosition> positions;

  /// A pointer back to the cell that holds this block (or 0).
  csMGCell* parent_cell;

  /// A last-used tick (compares with 'block_tick').
  size_t last_used;

  csMGPositionBlock () : parent_cell (0), last_used (0) { }
};

/**
 * The 2D x/z plane of the box used by the mesh generator is divided
 * in cells.
 */
struct csMGCell
{
  /**
   * Block of positions. Can be 0 in case none was generated yet.
   */
  csMGPositionBlock* block;

  /**
   * An array of meshes that are relevant in this cell (for calculating the
   * beam downwards).
   */
  csArray<iMeshWrapper> meshes;

  csMGCell () : block (0) { }
};

/**
 * The mesh generator.
 */
class csMeshGenerator : public scfImplementation1<
	csMeshGenerator, iMeshGenerator>
{
private:
  csRefArray<csMeshGeneratorGeometry> geometries;
  csRefArray<iMeshWrapper> meshes;

  /// Sample box where we will place geometry.
  csBox3 samplebox;
  
  /// 2-dimensional array of cells with cell_dim*cell_dim entries.
  csMGCell* cells;
  int cell_dim;

  /// Cache of unused position blocks.
  csPDelArray<csMGPositionBlock> cache_blocks;
  /// Position blocks that are used. These blocks are linked to a cell.
  csPDelArray<csMGPositionBlock> inuse_blocks;
  /// Maximum number of blocks (both in 'cache_blocks' as in 'inuse_blocks').
  int max_blocks;

  /// A timer to keep track of when blocks were last used.
  size_t block_tick;

  /**
   * Allocate blocks. This function will allocate all blocks needed
   * around a certain position and associate those blocks with the
   * appropriate cells. If a new block is needed it will first
   * take one from 'cache_blocks'. If 'cache_blocks' is empty
   * then we take the block we needed last from 'inuse_blocks'.
   */
  void AllocateBlocks (const csVector3& pos);

public:
  csMeshGenerator ();
  virtual ~csMeshGenerator ();

  virtual iObject *QueryObject () { return (iObject*)this; }

  virtual void SetSampleBox (const csBox3& box) { samplebox = box; }
  virtual const csBox3& GetSampleBox () const { return samplebox; }

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
};

#endif // __CS_MESHGEN_H__

