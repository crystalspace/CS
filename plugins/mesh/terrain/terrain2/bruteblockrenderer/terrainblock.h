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

#ifndef __TERRAINBLOCK_H__
#define __TERRAINBLOCK_H__

#include "ivideo/rendermesh.h"
#include "csutil/dirtyaccessarray.h"

struct iMovable;
struct iRenderView;

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

struct TerrainCellRData;

/**
 * A single terrain _block_
 *
 * A single terrain cell is made up of a hierarchy of blocks which basically 
 * forms a quadtree. The quadtree is used to speed up rendering (culling) as 
 * well as for LOD.
 *
 * To avoid cracks in the rendering there will be a set of index buffers for 
 * any given block size where each of the buffers will contain pre-tesselated
 * "connection" blocks. This requires adjacent blocks not to differ more than
 * a single level.
 *
 * Parent own child-blocks
 *
 * Numbering of children and neighbours
 *
 *         0
 *     ---------
 *     | 0 | 1 |
 *   2 |---+---| 1
 *     | 2 | 3 |
 *     ---------
 *         3
 */
struct TerrainBlock
{
  TerrainBlock ();
  ~TerrainBlock ();

  // Setup geometry
  void SetupGeometry ();
  void SetupTangentsBitangents ();

  // Invalidate the geometry and make it recalculate it
  void InvalidateGeometry (bool recursive = false);

  // Split a block, if required by invariant, split the neighbours too
  void Split ();

  // Try to split a single block if it could be done without breaking the invariant
  bool TrySplit ();

  // Merge down block, taking care to fix children and neighbours to keep the invariant
  void Merge ();

  // Disconnect a block (and all its children) from any "external" neighbours
  void Disconnect ();

  // Compute lod, split/merge blocks to get "right" tesselation
  void ComputeLOD (const csReversibleTransform& transformO2C, const size_t order[4]);

  // Basic helpers
  inline bool IsLeaf () const
  {
    return children[0] == 0;
  }

  // Recursivly cull and setup render meshes
  void CullRenderMeshes (iRenderView* rview, const csPlane3* cullPlanes, 
    uint32 frustumMask, const csReversibleTransform& obj2cam, iMovable* movable,
    csDirtyAccessArray<csRenderMesh*>& meshCache);

  //-- Memebers
  // Basic geometric properties
  csVector2 centerPos;
  csVector2 size;

  // The coordinate limits on the 2d grid 
  size_t gridLeft, gridRight, gridTop, gridBottom;
  
  // The size of each step in number of grid-points
  size_t stepSize;

  // Index of us as a child (if we are one) within parent
  size_t childIndex;

  // References to children
  TerrainBlock* children[4];

  // Neighbour pointers
  TerrainBlock* neighbours[4];

  // Parent block
  TerrainBlock* parent;

  // Owning renderer data
  TerrainCellRData* renderData;

  // Data holders for rendering
  csRef<iRenderBuffer> meshVertices, meshNormals, meshTexCoords;
  csRef<iRenderBuffer> meshTangents, meshBitangents;
  csRef<csRenderBufferHolder> bufferHolder;

  // Bounding box (in mesh-space)
  csBox3 boundingBox;

  // Is data built and valid?
  bool dataValid;
  bool tangentsBitangentsValid;
  
  class BufferAccessor;
};

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __TERRAINBLOCK_H__