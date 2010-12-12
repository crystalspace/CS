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

#include "cssysdef.h"

#include "csgfx/imagememory.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/shadervar.h"
#include "csgfx/shadervarcontext.h"
#include "cstool/rbuflock.h"
#include "cstool/rviewclipper.h" 
#include "csutil/objreg.h"
#include "csutil/refarr.h"
#include "csutil/blockallocator.h"
#include "csutil/stringconv.h"
#include "iengine.h"
#include "imesh/terrain2.h"
#include "ivideo/rendermesh.h"
#include "ivideo/txtmgr.h"

#include "bruteblockrenderer.h"
#include "terrainsystem.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

SCF_IMPLEMENT_FACTORY (csTerrainBruteBlockRenderer)

// File-static data
static CS::ShaderVarStringID textureLodDistanceID = CS::InvalidShaderVarStringID;

//-- Per cell properties class
class TerrainBBCellRenderProperties :
  public scfImplementation2<TerrainBBCellRenderProperties,
                            iTerrainCellRenderProperties,
                            scfFakeInterface<iShaderVariableContext> >,
  public CS::Graphics::ShaderVariableContextImpl
{
public:
  TerrainBBCellRenderProperties (iEngine* engine)
    : scfImplementationType (this), visible (true), blockResolution (32), 
    minSteps (1), splitDistanceCoeff (128), splatDistance (100),
    engine (engine)
  {
  }

  TerrainBBCellRenderProperties (const TerrainBBCellRenderProperties& other)
    : scfImplementationType (this), 
    CS::Graphics::ShaderVariableContextImpl (other), visible (other.visible), 
    blockResolution (other.blockResolution), minSteps (other.minSteps), 
    splitDistanceCoeff (other.splitDistanceCoeff), splatDistance (other.splatDistance),
    splatPrio (other.splatPrio), engine (other.engine)
  {

  }

  virtual bool GetVisible () const
  {
    return visible;
  }

  virtual void SetVisible (bool value)
  {
    visible = value;
  }

  size_t GetBlockResolution () const 
  {
    return blockResolution;
  }
  void SetBlockResolution (int value)
  {
    blockResolution = ptrdiff_t(1) << csLog2 (value);
  }

  size_t GetMinSteps () const
  {
    return minSteps;
  }
  void SetMinSteps (int value)
  {
    minSteps = value > 0 ? value : 1;
  }

  float GetLODSplitCoeff () const 
  {
    return splitDistanceCoeff;
  }
  void SetLODSplitCoeff (float value) 
  {
    splitDistanceCoeff = value;
  }

  float GetSplatDistance () const 
  {
    return splatDistance;
  }
  void SetSplatDistance (float value) 
  {
    splatDistance = value;
  }

  void SetSplatRenderPriority (const char* prio)
  {
    splatPrio = engine->GetRenderPriority (prio);
  }
  CS::Graphics::RenderPriority GetSplatRenderPriorityValue() const
  {
    return splatPrio;
  }


  virtual void SetParameter (const char* name, const char* value)
  {
    if (strcmp (name, "visible") == 0)
      SetVisible (strcmp(value, "true") == 0);
    else if (strcmp (name, "block resolution") == 0)
      SetBlockResolution (atoi (value));
    else if (strcmp (name, "min steps") == 0)
      SetMinSteps (atoi (value));
    else if (strcmp (name, "lod splitcoeff") == 0)
      SetLODSplitCoeff (CS::Utility::strtof (value));
    else if (strcmp (name, "splat distance") == 0)
      SetSplatDistance (CS::Utility::strtof (value));
    else if (strcmp (name, "splat render priority") == 0)
      SetSplatRenderPriority (value);

  }

  virtual size_t GetParameterCount() { return 5; }

  virtual const char* GetParameterName (size_t index)
  {
    switch (index)
    {
      case 0: return "visible";
      case 1: return "block resolution";
      case 2: return "min steps";
      case 3: return "lod splitcoeff";
      case 4: return "splat distance";
      case 5: return "splat render priority";
      default: return 0;
    }
  }

  virtual const char* GetParameterValue (size_t index)
  {
    return GetParameterValue (GetParameterName (index));
  }
  virtual const char* GetParameterValue (const char* name)
  {
    // @@@ Not nice
    static char scratch[32];
    if (strcmp (name, "visible") == 0)
      return visible ? "true" : "false";
    else if (strcmp (name, "block resolution") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%u", (uint)blockResolution);
      return scratch;
    }
    else if (strcmp (name, "min steps") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%u", (uint)minSteps);
      return scratch;
    }
    else if (strcmp (name, "lod splitcoeff") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%f", splitDistanceCoeff);
      return scratch;
    }
    else if (strcmp (name, "splat distance") == 0)
    {
      snprintf (scratch, sizeof (scratch), "%f", splatDistance);
      return scratch;
    }
    else if (strcmp (name, "splat render priority") == 0)
    {
      if (!splatPrio.IsValid())
	return 0;
      else
	return engine->GetRenderPriorityName (splatPrio);
    }
    else
      return 0;
  }

  virtual csPtr<iTerrainCellRenderProperties> Clone ()
  {
    return csPtr<iTerrainCellRenderProperties> (
      new TerrainBBCellRenderProperties (*this));
  }

private:
  // Per cell properties
  bool visible;

  // Block resolution in "gaps".. should be 2^n
  size_t blockResolution;

  // Grid steps for lowest tessellation setting
  size_t minSteps;

  // Lod splitting coefficient
  float splitDistanceCoeff;

  // Splatting end distance
  float splatDistance;
  
  // Splat render priority
  CS::Graphics::RenderPriority splatPrio;
  
  // Engine (needed for render prio...)
  iEngine* engine;
};

class TerrainBBSVAccessor : public scfImplementation1<TerrainBBSVAccessor,
                                                      iShaderVariableAccessor>
{
public:
  TerrainBBSVAccessor (TerrainBBCellRenderProperties* prop)
    : scfImplementationType (this), properties (prop)
  {
  }

  /// The accessor method itself, the important thing
  virtual void PreGetValue (csShaderVariable *variable)
  {
    if (variable->GetName () == textureLodDistanceID)
    {
      float distance = properties->GetSplatDistance ();
      variable->SetValue (csVector3 (distance, distance, distance));
    }
  }


private:
  // Note: properties (indirectly) holds refs to all accessors
  TerrainBBCellRenderProperties* properties;
};



enum TerrainCellBorderMatch
{
  CELL_MATCH_NONE = -1,
  CELL_MATCH_TOP = 0,
  CELL_MATCH_RIGHT = 1,
  CELL_MATCH_LEFT = 2,
  CELL_MATCH_BOTTOM = 3
};

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

  // Setup geometry
  void SetupGeometry ();

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
  csRef<csRenderBufferHolder> bufferHolder;

  // Bounding box (in mesh-space)
  csBox3 boundingBox;

  // Is data built and valid?
  bool dataValid;
};

struct OverlaidShaderVariableContext : 
  public scfImplementation1<OverlaidShaderVariableContext, 
			    scfFakeInterface<iShaderVariableContext> >,
  public CS::Graphics::OverlayShaderVariableContextImpl
{
  OverlaidShaderVariableContext () : scfImplementationType (this) {}
};

struct TerrainCellRData : public csRefCount
{
  //-- Members
  TerrainCellRData (iTerrainCell* cell, csTerrainBruteBlockRenderer* renderer);
  ~TerrainCellRData ();

  // Setup the root block
  void SetupRoot ();

  // Connect cell to another one in given direction
  void ConnectCell (TerrainCellRData* otherCell, TerrainCellBorderMatch side);

  // Disconnect cell from any neighbour cells
  void DisconnectCell ();

  //-- Data
  TerrainCellRData* neighbours[4];

  TerrainBlock* rootBlock;
  csBlockAllocator<TerrainBlock> terrainBlockAllocator;

  // Per cell base material sv context
  csRef<iShaderVariableContext> commonSVContext;

  // Per cell, per layer sv contexts
  // For materialmap
  csBitArray alphaMapMMUse;
  csRefArray<OverlaidShaderVariableContext> svContextArrayMM;
  csRefArray<iTextureHandle> alphaMapArrayMM;  
  
  // For separate alpha-maps
  csRefArray<OverlaidShaderVariableContext> svContextArrayAlpha;
  csRefArray<iTextureHandle> alphaMapArrayAlpha;
  csRefArray<iMaterialWrapper> materialArrayAlpha;

  // Settings
  size_t blockResolution;
  
  // Related objects
  csRef<TerrainBBCellRenderProperties> properties;
  csRef<TerrainBBSVAccessor> svAccessor;
  iTerrainCell* cell;  
  csTerrainBruteBlockRenderer* renderer;
};


// Match cell2 to cell1 (gives where cell2 attach to cell1)
TerrainCellBorderMatch MatchCell (iTerrainCell* cell1, iTerrainCell* cell2)
{
  const csVector2& position1 = cell1->GetPosition ();
  const csVector2& position2 = cell2->GetPosition ();

  const csVector3& size1 = cell1->GetSize ();
  const csVector3& size2 = cell2->GetSize ();

  const csVector3 sizeSum2 = (size1 + size2) / 2.0f;

  if (position1.x == position2.x &&
    size1.z == size2.z)
  {
    // Center line up in x.. either top or bottom match
    if (position1.y == (position2.y - sizeSum2.z))
    {
      // 1 is on top of 2
      return CELL_MATCH_TOP;
    }
    else if (position1.y == (position2.y + sizeSum2.z))
    {
      // 1 under 2
      return CELL_MATCH_BOTTOM;
    }
  }
  else if (position1.y == position2.y &&
    size1.x == size2.x)
  {
    // Center line up in y
    if (position1.x == (position2.x + sizeSum2.x))
    {
      return CELL_MATCH_LEFT;
    }
    else if (position1.x == (position2.x - sizeSum2.x))
    {
      return CELL_MATCH_RIGHT;
    }
  }


  return CELL_MATCH_NONE;
}




TerrainBlock::TerrainBlock ()
: stepSize (0), childIndex (0), parent (0), renderData (0), dataValid (false)
{
  for (size_t i = 0; i < 4; ++i)
  {
    children[i] = 0;
    neighbours[i] = 0;
  }
}

void TerrainBlock::SetupGeometry ()
{
  if (dataValid)
    return;

  size_t numVerts = (renderData->blockResolution) + 1;

  // Allocate the standard renderbuffers
  meshVertices = csRenderBuffer::CreateRenderBuffer (numVerts*numVerts, 
    CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  meshNormals = csRenderBuffer::CreateRenderBuffer (numVerts*numVerts,
    CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  meshTexCoords = csRenderBuffer::CreateRenderBuffer (numVerts*numVerts,
    CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);

  bufferHolder.AttachNew (new csRenderBufferHolder);

  bufferHolder->SetRenderBuffer (CS_BUFFER_POSITION, meshVertices);
  bufferHolder->SetRenderBuffer (CS_BUFFER_NORMAL, meshNormals);
  bufferHolder->SetRenderBuffer (CS_BUFFER_TEXCOORD0, meshTexCoords);

  const csVector2& cellPosition = renderData->cell->GetPosition ();
  const csVector3& cellSize = renderData->cell->GetSize ();

  boundingBox.Empty ();

  {
    // Lock and write the buffers
    csRenderBufferLock<csVector3> vertexData (meshVertices);
    csRenderBufferLock<csVector3> normalData (meshNormals);

    // Get the data
    csLockedHeightData cellData = renderData->cell->GetHeightData ();
    csLockedNormalData cellNData = renderData->cell->GetNormalData ();

    // Temporary data holder
    float minX = centerPos.x - size.x/2.0f;
    float maxX = centerPos.x + size.x/2.0f;

    float minZ = centerPos.y - size.y/2.0f;
    float maxZ = centerPos.y + size.y/2.0f;

    float xStep = size.x / (float)(numVerts - 1);
    float zStep = size.y / (float)(numVerts - 1);

    float currZ = maxZ;

    float minHeight = FLT_MAX;
    float maxHeight = -FLT_MAX;

    for (size_t y = 0, gridY = gridTop; y < numVerts; ++y, gridY += stepSize, currZ -= zStep)
    {
      float currX = minX;
      float* hRow = cellData.data + cellData.pitch * gridY + gridLeft;
      csVector3* nRow = cellNData.data + cellNData.pitch * gridY + gridLeft;

      for (size_t x = 0; x < numVerts; ++x, hRow += stepSize, nRow += stepSize, currX += xStep)
      {
        float height = *hRow;
        csVector3 normal = *nRow;

        *vertexData++ = csVector3 (currX, height, currZ);
        *normalData++ = normal;

        if (height < minHeight)
          minHeight = height;
        if (height > maxHeight)
          maxHeight = height; 
      }
    }

    boundingBox.Set (minX, minHeight, minZ, maxX, maxHeight, maxZ);
  }


  {
    csRenderBufferLock<csVector2> texcoordData (meshTexCoords);
   
    const csVector2 offs2 = 2*(centerPos - cellPosition);

    float minU = (offs2.x - size.x) / (2*cellSize.x);
    float maxU = (offs2.x + size.x) / (2*cellSize.x);

    float minV = (2*cellSize.z - offs2.y - size.y) / (2*cellSize.z);
    float maxV = (2*cellSize.z - offs2.y + size.y) / (2*cellSize.z);

    float uStep = (maxU - minU) / (float)(numVerts - 1);
    float vStep = (maxV - minV) / (float)(numVerts - 1);

    float currV = minV;
    for (size_t y = 0; y < numVerts; ++y, currV += vStep)
    {
      float currU = minU;
      for (size_t x = 0; x < numVerts; ++x, currU += uStep)
      {
	/* Apparently useless code: probably a bug. Commenting out...
        if (currV < 0.0f || currV > 1.0f || currU < 0.0f || currU > 1.0f)
          int a = 0;
	*/
        (*texcoordData).x = currU;
        (*texcoordData).y = currV;
        ++texcoordData;
      }
    }
  }

  dataValid = true;
}

void TerrainBlock::InvalidateGeometry (bool recursive)
{
  dataValid = false;
  meshVertices = meshNormals = meshTexCoords = 0;
  boundingBox = csBox3 ();

  if (recursive && !IsLeaf ())
  {
    for (size_t i = 0; i < 4; ++i)
    {
      children[i]->InvalidateGeometry (recursive);
    }
  }
}

void TerrainBlock::Split ()
{
  // Split any neighbours that needs splitting to keep the invariant
  for (size_t i = 0; i < 4; ++i)
  {
    if (neighbours[i] &&
      neighbours[i]->stepSize > stepSize &&
      neighbours[i]->IsLeaf ())
    {
      neighbours[i]->Split ();
    }
  }

  // Setup the four child-blocks
  size_t halfRight = gridLeft + (gridRight - gridLeft) / 2;
  size_t halfBottom = gridTop + (gridBottom - gridTop) / 2;
  csVector2 size4 = size / 4.0f;
  csVector2 size2 = size / 2.0f;

  children[0] = renderData->terrainBlockAllocator.Alloc ();
  children[0]->centerPos = centerPos + csVector2 (-size4.x, size4.y);
  children[0]->size = size2;
  children[0]->gridLeft = gridLeft;
  children[0]->gridRight = halfRight;
  children[0]->gridTop = gridTop;
  children[0]->gridBottom = halfBottom;
  children[0]->stepSize = stepSize / 2;
  children[0]->childIndex = 0;   
  children[0]->parent = this;
  children[0]->renderData = renderData;

  children[1] = renderData->terrainBlockAllocator.Alloc ();
  children[1]->centerPos = centerPos + csVector2 (size4.x, size4.y);
  children[1]->size = size2;
  children[1]->gridLeft = halfRight;
  children[1]->gridRight = gridRight;
  children[1]->gridTop = gridTop;
  children[1]->gridBottom = halfBottom;
  children[1]->stepSize = stepSize / 2;
  children[1]->childIndex = 1;   
  children[1]->parent = this;
  children[1]->renderData = renderData;

  children[2] = renderData->terrainBlockAllocator.Alloc ();
  children[2]->centerPos = centerPos + csVector2 (-size4.x, -size4.y);
  children[2]->size = size2;
  children[2]->gridLeft = gridLeft;
  children[2]->gridRight = halfRight;
  children[2]->gridTop = halfBottom;
  children[2]->gridBottom = gridBottom;
  children[2]->stepSize = stepSize / 2;
  children[2]->childIndex = 2;   
  children[2]->parent = this;
  children[2]->renderData = renderData;

  children[3] = renderData->terrainBlockAllocator.Alloc ();
  children[3]->centerPos = centerPos + csVector2 (size4.x, -size4.y);
  children[3]->size = size2;
  children[3]->gridLeft = halfRight;
  children[3]->gridRight = gridRight;
  children[3]->gridTop = halfBottom;
  children[3]->gridBottom = gridBottom;
  children[3]->stepSize = stepSize / 2;
  children[3]->childIndex = 3;   
  children[3]->parent = this;
  children[3]->renderData = renderData;

  // Connect inter-children neighbours
  children[0]->neighbours[1] = children[1];
  children[0]->neighbours[3] = children[2];
  
  children[1]->neighbours[2] = children[0];
  children[1]->neighbours[3] = children[3];

  children[2]->neighbours[0] = children[0];
  children[2]->neighbours[1] = children[3];

  children[3]->neighbours[0] = children[1];
  children[3]->neighbours[2] = children[2];

  // Setup neighbour-neighbour/child neighbours
  if (neighbours[0])
  {
    if (neighbours[0]->IsLeaf ())
    {
      // It is at its most tesselation
      neighbours[0]->neighbours[3] = this;
      children[0]->neighbours[0] = neighbours[0];
      children[1]->neighbours[0] = neighbours[0];
    }
    else
    {
      children[0]->neighbours[0] = neighbours[0]->children[2];
      children[1]->neighbours[0] = neighbours[0]->children[3];
      neighbours[0]->children[2]->neighbours[3] = children[0];
      neighbours[0]->children[3]->neighbours[3] = children[1];
    }
  }

  if (neighbours[1])
  {
    if (neighbours[1]->IsLeaf ())
    {
      // It is at its most tesselation
      neighbours[1]->neighbours[2] = this;
      children[1]->neighbours[1] = neighbours[1];
      children[3]->neighbours[1] = neighbours[1];
    }
    else
    {
      children[1]->neighbours[1] = neighbours[1]->children[0];
      children[3]->neighbours[1] = neighbours[1]->children[2];
      neighbours[1]->children[0]->neighbours[2] = children[1];
      neighbours[1]->children[2]->neighbours[2] = children[3];
    }
  }

  if (neighbours[2])
  {
    if (neighbours[2]->IsLeaf ())
    {
      // It is at its most tesselation
      neighbours[2]->neighbours[1] = this;
      children[0]->neighbours[2] = neighbours[2];
      children[2]->neighbours[2] = neighbours[2];
    }
    else
    {
      children[0]->neighbours[2] = neighbours[2]->children[1];
      children[2]->neighbours[2] = neighbours[2]->children[3];
      neighbours[2]->children[1]->neighbours[1] = children[0];
      neighbours[2]->children[3]->neighbours[1] = children[2];
    }
  }

  if (neighbours[3])
  {
    if (neighbours[3]->IsLeaf ())
    {
      // It is at its most tesselation
      neighbours[3]->neighbours[0] = this;
      children[2]->neighbours[3] = neighbours[3];
      children[3]->neighbours[3] = neighbours[3];
    }
    else
    {
      children[2]->neighbours[3] = neighbours[3]->children[0];
      children[3]->neighbours[3] = neighbours[3]->children[1];
      neighbours[3]->children[0]->neighbours[0] = children[2];
      neighbours[3]->children[1]->neighbours[0] = children[3];
    }
  }
}

bool TerrainBlock::TrySplit ()
{
  // Test the preconditions under which a split succeeds
  // 1. Is a leaf (no reason to split inner node again)
  if (!IsLeaf ())
    return false;

  // 2. Neighbour is after split at same level or max one level from us 
  // (if neighbour is inner node we can always connect to its children)
  for (size_t i = 0; i < 4; ++i)
  {
    if (neighbours[i] && 
        (!neighbours[i]->IsLeaf () ||
        neighbours[i]->stepSize > stepSize))
    return false;
  }

  // We can split, so do it
  Split ();

  return true;
}

void TerrainBlock::Merge ()
{
  if (IsLeaf ())
  {
    return; // Nothing to merge
  }

  // Merge all children
  for (size_t i = 0; i < 4; ++i)
  {
    children[i]->Merge ();
  }
  
  // Merge any neighbours that needs to be merged
  static const size_t childMapTable[4][2][2] = 
  {
    {
      {0,3},{2,1}
    },
    {
      {0,3},{1,2}
    },
    {
      {2,1},{3,0}
    },
    {
      {3,0},{1,2}
    },
  };

  for (size_t child = 0; child < 4; ++child)
  {
    TerrainBlock* childPtr = children[child];

    for (size_t neighbour = 0; neighbour < 2; ++neighbour)
    {
      TerrainBlock* cn = childPtr->neighbours[childMapTable[child][neighbour][0]];
      
      if (cn)
      {
        if (!cn->IsLeaf ())
        {
          cn->Merge ();
        }
        cn->neighbours[childMapTable[child][neighbour][1]] = this;
      }
    }
  }

  // Now, remove our children
  for (size_t i = 0; i < 4; ++i)
  {
    renderData->terrainBlockAllocator.Free (children[i]);
    children[i] = 0;
  }
}

void TerrainBlock::Disconnect ()
{
  if (!IsLeaf ())
  {
    for (size_t i = 0; i < 4; ++i)
    {
      children[i]->Disconnect ();
    }
  }

  // Disconnect any outer neighbour
  for (size_t i = 0; i < 4; ++i)
  {    
    size_t neighbourBackPtr = 3-i;

    if (neighbours[i])
    {
      // Check if we are neighbour of neighbour
      if (neighbours[i]->neighbours[neighbourBackPtr] == this)
      {
        neighbours[i]->neighbours[neighbourBackPtr] = 0;
      }

      // If neighbour is inner, check one level child
      if (!neighbours[i]->IsLeaf())
      {
        for (size_t c = 0; c < 4; ++c)
        {
          if (neighbours[i]->children[c]->neighbours[neighbourBackPtr] == this)
          {
            neighbours[i]->children[c]->neighbours[neighbourBackPtr] = 0;
          }
        }
      }
    }
  }
}

void TerrainBlock::ComputeLOD (const csReversibleTransform& transformO2C, const size_t order[4])
{
  const size_t blockRes = renderData->blockResolution;

  if (boundingBox.Empty ())
    return;

  const csVector3& camPosInObj = transformO2C.GetOrigin ();
  csBox3 camBox (boundingBox.Min () - camPosInObj, boundingBox.Max () - camPosInObj);

/*  const csVector3 toBBVec = camBox.GetCenter ().Unit ();
  const csVector3 bbSize = camBox.GetSize ();

  // Compute projected area of the seen (3) faces of the bb
  const float projectedArea =
    (bbSize.x*bbSize.y) * fabsf (toBBVec.z) +
    (bbSize.x*bbSize.z) * fabsf (toBBVec.y) +
    (bbSize.y*bbSize.z) * fabsf (toBBVec.x);

  const float resFactor = renderData->properties->GetLODSplitCoeff () / (float)blockRes;
  const float targetDistSq = projectedArea*resFactor*resFactor;
*/

  const float splitDist = size.x * renderData->properties->GetLODSplitCoeff () / (float)blockRes;
  const float targetDistSq = splitDist*splitDist;

  if (camBox.SquaredOriginDist  () < targetDistSq &&
    stepSize > renderData->properties->GetMinSteps ())
  {
    if (IsLeaf ())
    {
      Split ();
    }
  }
  else
  {
    if (!IsLeaf ())
    {
      Merge ();
    }
  }

  if (!IsLeaf ())
  {
    for (size_t i = 0; i < 4; ++i)
    {
      children[order[i]]->ComputeLOD (transformO2C, order);
    }
  }
}

void TerrainBlock::CullRenderMeshes (iRenderView* rview, const csPlane3* cullPlanes, 
  uint32 frustumMask, const csReversibleTransform& obj2cam, iMovable* movable,
  csDirtyAccessArray<csRenderMesh*>& meshCache)
{
  int clipPortal, clipPlane, clipZPlane;
  if (!CS::RenderViewClipper::CullBBox (rview->GetRenderContext (), cullPlanes, 
    frustumMask, boundingBox, clipPortal, clipPlane, clipZPlane) &&
    !boundingBox.Empty ())
    return; // If we're not visible, our children won't be either

  if (!IsLeaf ())
  {
    // Have children, dispatch to them
    for (size_t i = 0; i < 4; ++i)
    {
      children[i]->CullRenderMeshes (rview, cullPlanes, frustumMask, obj2cam,
        movable, meshCache);
    }
    return;
  }

  SetupGeometry ();

  const csVector3 worldOrigin = movable->GetFullTransform ().GetOrigin () + 
    csVector3 (centerPos.x, 0, centerPos.y);  


  const csTerrainMaterialPalette& palette = renderData->renderer->GetMaterialPalette ();

  // Get the index buffer
  size_t indexType = 
    ((neighbours[0] && neighbours[0]->stepSize > stepSize) ? 1<<0 : 0) |
    ((neighbours[1] && neighbours[1]->stepSize > stepSize) ? 1<<1 : 0) |
    ((neighbours[2] && neighbours[2]->stepSize > stepSize) ? 1<<2 : 0) |
    ((neighbours[3] && neighbours[3]->stepSize > stepSize) ? 1<<3 : 0);

  uint numIndices;
  bufferHolder->SetRenderBuffer (CS_BUFFER_INDEX,
    renderData->renderer->GetIndexBuffer (renderData->blockResolution,
    indexType, numIndices));

  // Calculate the WS distance from the camera to this cell.
  float fCellDistance = sqrt (boundingBox.SquaredPosDist (rview->GetCamera ()->GetTransform ().GetOrigin ()));

  // Calculate whether to render the base material or the splatting.
  bool renderSplatting = (fCellDistance < renderData->properties->GetSplatDistance ());

  // Get the render priority of terrain splatting.
  CS::Graphics::RenderPriority splatPrio = renderData->properties->GetSplatRenderPriorityValue();

  // Get the optional alpha-splat material.
  csRef<iMaterialWrapper> alphaSplatMaterial = renderData->cell->GetAlphaSplatMaterial ();

  const int renderBaseMaterial = -2;
  const int renderAlphaSplatMaterial = -1;
  for (int j = -2; j < (int)palette.GetSize (); ++j)
  {
    iMaterialWrapper* mat = 0;
    iShaderVariableContext* svContext;

    if (!renderSplatting)
    {
      if (j == renderBaseMaterial)
      {
        mat = renderData->cell->GetBaseMaterial ();
        svContext = renderData->commonSVContext;
      }
    }
    else
    {
      if (j == renderAlphaSplatMaterial)
      {
        // Check that we have this material.
        if (!alphaSplatMaterial.IsValid ())
          continue;

        mat = alphaSplatMaterial;
        svContext = renderData->commonSVContext;
      }
      else if (j != renderBaseMaterial)
      {
        // Check if the map is used.
        if (!renderData->alphaMapMMUse.IsBitSet (j))
        {
          continue;
        }

        mat = palette.Get (j);
        svContext = renderData->svContextArrayMM[j];
      }
    }

    if (!mat || !svContext)
      continue;

    bool created;
    csRenderMesh*& mesh = renderData->renderer->GetMeshHolder ().GetUnusedMesh (created,
      rview->GetCurrentFrameNumber ());

    mesh->meshtype = CS_MESHTYPE_TRIANGLESTRIP;
    mesh->clip_portal = clipPortal;
    mesh->clip_plane = clipPlane;
    mesh->clip_z_plane = clipZPlane;
    mesh->indexstart = 0;
    mesh->indexend = numIndices;
    mesh->material = mat;
    mesh->variablecontext = svContext;
    mesh->buffers = bufferHolder;
    if (j >= 0) mesh->renderPrio = splatPrio;

    mesh->worldspace_origin = worldOrigin;
    mesh->bbox = boundingBox;

    meshCache.Push (mesh);
  }
  
  for (size_t j = 0; j < renderData->alphaMapArrayAlpha.GetSize (); ++j)
  {
    iMaterialWrapper* mat = renderData->materialArrayAlpha[j];
    iShaderVariableContext* svContext = renderData->svContextArrayAlpha[j];

    if (!mat || !svContext)
      continue;

    bool created;
    csRenderMesh*& mesh = renderData->renderer->GetMeshHolder ().GetUnusedMesh (created,
      rview->GetCurrentFrameNumber ());

    mesh->meshtype = CS_MESHTYPE_TRIANGLESTRIP;
    mesh->clip_portal = clipPortal;
    mesh->clip_plane = clipPlane;
    mesh->clip_z_plane = clipZPlane;
    mesh->indexstart = 0;
    mesh->indexend = numIndices;
    mesh->material = mat;
    mesh->variablecontext = svContext;
    mesh->buffers = bufferHolder;
    mesh->renderPrio = splatPrio;

    mesh->worldspace_origin = worldOrigin;
    mesh->bbox = boundingBox;

    meshCache.Push (mesh);
  }
}


TerrainCellRData::TerrainCellRData (iTerrainCell* cell, 
                                    csTerrainBruteBlockRenderer* renderer)
  : rootBlock (0), cell (cell), renderer (renderer)
{
  properties = (TerrainBBCellRenderProperties*)cell->GetRenderProperties (); 

  blockResolution = properties->GetBlockResolution ();

  commonSVContext = cell->GetRenderProperties ();
  svContextArrayMM.SetSize (renderer->GetMaterialPalette ().GetSize ());
  alphaMapArrayMM.SetSize (renderer->GetMaterialPalette ().GetSize ());
  alphaMapMMUse.SetSize (renderer->GetMaterialPalette ().GetSize ());

  // Setup the base context

  svAccessor.AttachNew (new TerrainBBSVAccessor (properties));

  if (textureLodDistanceID == CS::InvalidShaderVarStringID)
  {
    textureLodDistanceID = renderer->GetStringSet ()->Request ("texture lod distance");
  }

  csRef<csShaderVariable> lodVar; lodVar.AttachNew (
    new csShaderVariable (textureLodDistanceID));
  lodVar->SetAccessor (svAccessor);
  
  commonSVContext->AddVariable (lodVar);

  for (size_t i = 0; i < 4; ++i)
  {
    neighbours[i] = 0;
  }
}

TerrainCellRData::~TerrainCellRData ()
{  
}

void TerrainCellRData::SetupRoot ()
{
  if (rootBlock)
    return;

  rootBlock = terrainBlockAllocator.Alloc ();
  
  const csVector3& cellSize = cell->GetSize ();
  rootBlock->centerPos = cell->GetPosition () + 
    csVector2 (cellSize.x / 2.0f, cellSize.z / 2.0f);
  rootBlock->size = csVector2 (cellSize.x, cellSize.z);
  
  rootBlock->gridLeft = rootBlock->gridTop = 0;
  rootBlock->gridRight = cell->GetGridWidth () - 1;
  rootBlock->gridBottom = cell->GetGridHeight () - 1;

  rootBlock->stepSize = rootBlock->gridRight / blockResolution;
  rootBlock->renderData = this;
}

void TerrainCellRData::ConnectCell (TerrainCellRData* otherCell, TerrainCellBorderMatch side)
{
  size_t sideIdx = (size_t)side;
  size_t sideBackIdx = 3-sideIdx;

  // Reduce other cell to be small enough for making connection
  if (rootBlock)
    rootBlock->Merge ();

  if (otherCell->rootBlock)
    otherCell->rootBlock->Merge ();

  neighbours[sideIdx] = otherCell;
  otherCell->neighbours[sideBackIdx] = this;

  // Make sure both actually have a root
  SetupRoot ();
  otherCell->SetupRoot ();

  rootBlock->neighbours[sideIdx] = otherCell->rootBlock;
  otherCell->rootBlock->neighbours[sideBackIdx] = rootBlock;
}

void TerrainCellRData::DisconnectCell ()
{
  if (rootBlock)
  {
    rootBlock->Disconnect ();
  }

  for (size_t i = 0; i < 4; ++i)
  {
    size_t bi = 3-i;

    if (neighbours[i] && 
      neighbours[i]->neighbours[bi] == this)
    {
      neighbours[i]->neighbours[bi] = 0;
    }
  }

  rootBlock = 0;
  terrainBlockAllocator.Empty ();
}

//-- THE TERRAIN RENDERER ITSELF
csTerrainBruteBlockRenderer::csTerrainBruteBlockRenderer (iBase* parent)
  : scfImplementationType (this, parent), engine (nullptr), materialPalette (0)
{  
}

csTerrainBruteBlockRenderer::~csTerrainBruteBlockRenderer ()
{

}

csPtr<iTerrainCellRenderProperties> csTerrainBruteBlockRenderer::CreateProperties ()
{
  return csPtr<iTerrainCellRenderProperties> (new TerrainBBCellRenderProperties (engine));
}

void csTerrainBruteBlockRenderer::ConnectTerrain (iTerrainSystem* system)
{
  system->AddCellLoadListener (this);
  system->AddCellHeightUpdateListener (this);
}

void csTerrainBruteBlockRenderer::DisconnectTerrain (iTerrainSystem* system)
{
  system->RemoveCellHeightUpdateListener (this);
  system->RemoveCellLoadListener (this);
}

csRenderMesh** csTerrainBruteBlockRenderer::GetRenderMeshes (int& n, 
  iRenderView* rview, iMovable* movable, uint32 frustum_mask,
  const csArray<iTerrainCell*>& cells)
{
  renderMeshCache.Empty ();

  // Setup camera properties and clip planes
  const csReversibleTransform& trO2W = movable->GetFullTransform ();
  
  iCamera* camera = rview->GetCamera ();
  csReversibleTransform trO2C = camera->GetTransform ();
  if (!trO2W.IsIdentity ())
    trO2C /= trO2W;

  // At very most we can have 10 planes
  csPlane3 planes[10];
  CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext (), 
    trO2C, planes, frustum_mask); 

  // Compute an approximate back to front direction
  const csVector3& direction = trO2C.GetT2O ().Col3 ();
  const size_t orderIdx = (direction.x > 0 ? 1 : 0) |
                          (direction.z > 0 ? 2 : 0);

  static const size_t blockOrderTable[4][4] = 
  {
    {2,3,0,1},
    {3,2,1,0},
    {0,1,2,3},
    {1,0,3,2}
  };

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    TerrainCellRData* renderData = (TerrainCellRData*)cells[i]->GetRenderData ();

    if (!renderData)
      continue; // No data, nothing to render

    if (!renderData->rootBlock)
      renderData->SetupRoot ();

    renderData->rootBlock->ComputeLOD (trO2C, blockOrderTable[orderIdx]);
  }

  for (size_t i = 0; i < cells.GetSize (); ++i)
  {
    TerrainCellRData* renderData = (TerrainCellRData*)cells[i]->GetRenderData ();

    if (!renderData)
      continue; // No data, nothing to render

    renderData->rootBlock->CullRenderMeshes (rview, planes, frustum_mask, trO2C, 
      movable, renderMeshCache);
  }

  n = (int)renderMeshCache.GetSize ();
  return renderMeshCache.GetArray ();
}

void csTerrainBruteBlockRenderer::OnMaterialPaletteUpdate (
  const csRefArray<iMaterialWrapper>& material_palette)
{
  materialPalette = &material_palette;  
}

void csTerrainBruteBlockRenderer::OnMaterialMaskUpdate (iTerrainCell* cell, 
  const csRect& rectangle, const unsigned char* materialMap, size_t srcPitch)
{
  SetupCellMMArrays (cell);

  csRef<TerrainCellRData> data = (TerrainCellRData*)cell->GetRenderData ();

  if (data && materialPalette)
  {    
    // Iterate and build all the alpha-masks    
    for (size_t i = 0; i < materialPalette->GetSize (); ++i)
    {
      size_t dstPitch;
      uint8* buffer = data->alphaMapArrayMM[i]->
        QueryBlitBuffer (rectangle.xmin, rectangle.ymin, rectangle.Width (),
        rectangle.Height (), dstPitch, iTextureHandle::RGBA8888);

      csRGBpixel* dstBuffer = (csRGBpixel*)buffer;
      dstPitch /= sizeof (csRGBpixel);

      bool isUsed = false;

      for (int y = rectangle.ymin; y < rectangle.ymax; ++y)
      {
        const unsigned char* src_data = materialMap + y * srcPitch;

        for (int x = rectangle.xmin, rx = 0; x < rectangle.xmax; ++x, ++src_data, ++rx)
        {
          unsigned char result = 0;

          if (*src_data == i)
          {
            result = 255;
            isUsed = true;
          }

          dstBuffer[rx].Set (result, result, result, result);
        }

        dstBuffer += dstPitch;
      }


      data->alphaMapArrayMM[i]->ApplyBlitBuffer (buffer);

      if (isUsed)
      {
        data->alphaMapMMUse.SetBit (i);
      }
      else
      {
        data->alphaMapMMUse.ClearBit (i);
      }
    }
  }
}

void csTerrainBruteBlockRenderer::OnMaterialMaskUpdate (iTerrainCell* cell, 
  size_t matIdx, const csRect& rectangle, const unsigned char* materialMap, 
  size_t srcPitch)
{
  SetupCellMMArrays (cell);

  csRef<TerrainCellRData> data = (TerrainCellRData*)cell->GetRenderData ();

  if (data)
  {
    // Update the alpha map
    size_t dstPitch;
    uint8* buffer = data->alphaMapArrayMM[matIdx]->
      QueryBlitBuffer (rectangle.xmin, rectangle.ymin, rectangle.Width (),
      rectangle.Height (), dstPitch, iTextureHandle::RGBA8888);
    
    csRGBpixel* dstBuffer = (csRGBpixel*)buffer;
    dstPitch /= sizeof (csRGBpixel);

    bool isUsed = false;

    for (int y = rectangle.ymin; y < rectangle.ymax; ++y)
    {
      const unsigned char* src_data = materialMap + y * srcPitch;

      for (int x = rectangle.xmin, rx = 0; x < rectangle.xmax; ++x, ++src_data, ++rx)
      {
        const unsigned char result = *src_data;

        if (result > 0)
        {
          isUsed = true;
        }

        dstBuffer[rx].Set (result, result, result, result);
      }

      dstBuffer += dstPitch;
    }

    data->alphaMapArrayMM[matIdx]->ApplyBlitBuffer (buffer);

    if (isUsed)
    {
      data->alphaMapMMUse.SetBit (matIdx);
    }
    else
    {
      data->alphaMapMMUse.ClearBit (matIdx);
    }
  }
}

void csTerrainBruteBlockRenderer::OnAlphaMapUpdate (iTerrainCell* cell,
  iMaterialWrapper* material, iImage* alphaMap)
{
  SetupCellData (cell);

  csRef<TerrainCellRData> data = (TerrainCellRData*)cell->GetRenderData ();
  if (data)
  {
    // Check if we already have the material or not
    size_t idx = data->materialArrayAlpha.Find (material);
    
    if (idx == csArrayItemNotFound)
    {
      // Setup new one
      idx = data->materialArrayAlpha.Push (material);

      // Allocate SV & texture
      {
        csRef<OverlaidShaderVariableContext> ctx;
        ctx.AttachNew (new OverlaidShaderVariableContext);
        ctx->SetParentContext (data->commonSVContext);

        data->svContextArrayAlpha.Push (ctx);
      }

      csRef<iTextureHandle> txtHandle = graph3d->GetTextureManager ()->
        RegisterTexture (alphaMap, CS_TEXTURE_3D | CS_TEXTURE_CLAMP);

      data->alphaMapArrayAlpha.Push (txtHandle);
      csRef<csShaderVariable> var;
      var = data->svContextArrayAlpha[idx]->GetVariableAdd (
        stringSet->Request ("splat alpha map"));        
      var->SetType (csShaderVariable::TEXTURE);
      var->SetValue (data->alphaMapArrayAlpha[idx]);      
    }

    // Get a buffer to blit to for the texture
    size_t pitch;
    uint8* buffer = data->alphaMapArrayAlpha[idx]->
      QueryBlitBuffer (0, 0, alphaMap->GetWidth (), alphaMap->GetHeight (),
      pitch, iTextureHandle::RGBA8888);

    csRGBpixel* dstBuffer = (csRGBpixel*)buffer;
    pitch /= sizeof(csRGBpixel);

    const int w = alphaMap->GetWidth ();
    const int h = alphaMap->GetHeight ();

    csRGBpixel* srcBuffer = (csRGBpixel*)alphaMap->GetImageData ();
 
    if (alphaMap->GetFormat () & CS_IMGFMT_ALPHA)
    {
      // With alpha
      for (int y = 0; y < h; ++y)
      {
        // Just copy a line
        memcpy (dstBuffer, srcBuffer, w*sizeof(csRGBpixel));          
        srcBuffer += w;
        dstBuffer += pitch;
      }
    }
    else
    {
      // No alpha
      for (int y = 0; y < h; ++y)
      {
        // Take a line, set alpha to intensity
        for (int x = 0; x < w; ++x)
        {
          dstBuffer[x].red = srcBuffer[x].red;
          dstBuffer[x].green = srcBuffer[x].green;
          dstBuffer[x].blue = srcBuffer[x].blue;
          dstBuffer[x].alpha = srcBuffer[x].Intensity ();
        }

        srcBuffer += w;
        dstBuffer += pitch;
      }
    }    

    data->alphaMapArrayAlpha[idx]->ApplyBlitBuffer (buffer);
  }
}

void csTerrainBruteBlockRenderer::OnHeightUpdate (iTerrainCell* cell, 
                                                  const csRect& rectangle)
{
  // Invalidate all data, recursively
  // @@TODO: Make this smarter!
  csRef<TerrainCellRData> data = (TerrainCellRData*)cell->GetRenderData ();

  if (data && data->rootBlock)
  {
    data->rootBlock->InvalidateGeometry (true);
  }
}


void csTerrainBruteBlockRenderer::OnCellLoad (iTerrainCell* cell)
{
  SetupCellData (cell);
}

void csTerrainBruteBlockRenderer::OnCellPreLoad (iTerrainCell* cell)
{

}

void csTerrainBruteBlockRenderer::OnCellUnload (iTerrainCell* cell)
{
  csRef<TerrainCellRData> data = (TerrainCellRData*)cell->GetRenderData ();

  if (data)
  {
    activeCellList.Delete (data);
    data->DisconnectCell ();
  }

  // Clean out any renderer data in cell
  cell->SetRenderData (0);
}

void csTerrainBruteBlockRenderer::SetupCellData (iTerrainCell* cell)
{
  // We got a new cell, so setup a render-data structure for it
  if (cell->GetRenderData ())
    return;

  csRef<TerrainCellRData> newD;
  newD.AttachNew (new TerrainCellRData (cell, this));
  newD->SetupRoot ();

  cell->SetRenderData (newD);

  // Check possible matches for automatic neighbouring
  for (size_t i = 0; i < activeCellList.GetSize (); ++i)
  {
    TerrainCellBorderMatch match = MatchCell (cell, activeCellList[i]->cell);

    if (match != CELL_MATCH_NONE)
    {
      // We have a match, do connect it
      newD->ConnectCell (activeCellList[i], match);
    }
  }

  activeCellList.Push (newD);
}


bool csTerrainBruteBlockRenderer::Initialize (iObjectRegistry* objectReg)
{
  objectRegistry = objectReg;
  graph3d = csQueryRegistry<iGraphics3D> (objectReg);
  stringSet = csQueryRegistryTagInterface<iShaderVarStringSet> (objectReg,
    "crystalspace.shader.variablenameset");
  csRef<iEngine> engine = csQueryRegistry<iEngine> (objectReg);

  // Error getting globals
  if (!graph3d || !stringSet || !engine)
    return false;
  this->engine = engine;

  return true;
}

template<typename T>
static void FillEdge (bool halfres, int res, T* indices, int &indexcount,
                      int offs, int xadd, int zadd)
{
  int x;
  // This triangulation scheme could probably be better.
  for (x=0; x<res; x+=2)
  {
    if (x>0)
    {
      indices[indexcount++] = offs+x*xadd;
      indices[indexcount++] = offs+x*xadd+zadd;
    }
    else
    {
      indices[indexcount++] = offs;
      indices[indexcount++] = offs;
      indices[indexcount++] = offs;
    }

    if (!halfres)
      indices[indexcount++] = offs+(x+1)*xadd;
    else
      indices[indexcount++] = offs+x*xadd;

    indices[indexcount++] = offs+(x+1)*xadd+zadd;

    if (x<res-2)
    {
      indices[indexcount++] = offs+(x+2)*xadd;
      indices[indexcount++] = offs+(x+2)*xadd+zadd;
    }
    else
    {
      indices[indexcount++] = offs+(x+2)*xadd;
      indices[indexcount++] = offs+(x+2)*xadd;
      indices[indexcount++] = offs+(x+2)*xadd;
    }
  }
}

template<typename T>
static void FillBlock (T* indices, int &indexcount, int block_res, 
                       int t, int r, int l, int b)
{
  indexcount = 0;
  int x, z;

  uint32 numVert = block_res + 1;

  for (z=1; z<(block_res-1); z++)
  {
    // Start row
    indices[indexcount++] = 1+z*(numVert);
    indices[indexcount++] = 1+z*(numVert);

    for (x=1; x<(block_res); x++)
    { 
      indices[indexcount++] = x+(z+1)*(numVert);
      indices[indexcount++] = x+(z+0)*(numVert);
    }

    indices[indexcount++] = x-1+(z+1)*(numVert);
    indices[indexcount++] = x-1+(z+1)*(numVert);
  }

  FillEdge (t==1,
    block_res, indices, indexcount, 
    0, 1, (block_res+1));

  FillEdge (r==1,
    block_res, indices, indexcount,
    block_res, (block_res+1), -1);

  FillEdge (l==1,
    block_res, indices, indexcount, 
    block_res*(block_res+1), -(block_res+1), 1);

  FillEdge (b==1, 
    block_res, indices, indexcount,
    block_res*(block_res+1)+block_res, -1, -(block_res+1));

}

iRenderBuffer* csTerrainBruteBlockRenderer::GetIndexBuffer (size_t blockResolution, 
  size_t indexType, uint& numIndices)
{
  csRef<IndexBufferSet> set;

  size_t blockResLog2 = csLog2 ((int)blockResolution);

  if (blockResLog2 >= indexBufferList.GetSize () ||
    indexBufferList[blockResLog2] == 0)
  {
    //Need a new one
    indexBufferList.SetSize (blockResLog2+1);
    set.AttachNew (new IndexBufferSet);
    indexBufferList.Put (blockResLog2, set);

    int* numIndices = set->numIndices;
    size_t maxIndex = (blockResolution+1)*(blockResolution+1) - 1;

    // Iterate over all possible border conditions
    for (int t = 0; t <= 1; ++t)
    {
      for (int r = 0; r <= 1; ++r)
      {
        for (int l = 0; l <= 1; ++l)
        {
          for (int b = 0; b <= 1; ++b)
          {
            int indexType = t | (r<<1) | (l<<2) | (b<<3);

            if (maxIndex > 0xFFFF)
            {
              // need 32-bit or more
              set->meshIndices[indexType] = 
                csRenderBuffer::CreateIndexRenderBuffer (
                blockResolution*blockResolution*2*3, CS_BUF_STATIC,
                CS_BUFCOMP_UNSIGNED_INT, 0, maxIndex);

              uint32* indices = (uint32*)
                set->meshIndices[indexType]->Lock(CS_BUF_LOCK_NORMAL);

              FillBlock (indices, numIndices[indexType], (int)blockResolution,
                t, r, l, b);
              set->meshIndices[indexType]->Release ();
            }
            else
            {
              // 16 bits is enough
              set->meshIndices[indexType] = 
                csRenderBuffer::CreateIndexRenderBuffer (
                blockResolution*blockResolution*2*3, CS_BUF_STATIC,
                CS_BUFCOMP_UNSIGNED_SHORT, 0, maxIndex);

              uint16* indices = (uint16*)
                set->meshIndices[indexType]->Lock(CS_BUF_LOCK_NORMAL);

              FillBlock (indices, numIndices[indexType], (int)blockResolution,
                t, r, l, b);
              set->meshIndices[indexType]->Release ();
            }
          }
        }
      }
    }
  }
  else
  {
    set = indexBufferList[blockResLog2];
  }

  numIndices = set->numIndices[indexType];
  return set->meshIndices[indexType];
}

void csTerrainBruteBlockRenderer::SetupCellMMArrays (iTerrainCell* cell)
{
  SetupCellData (cell);

  csRef<TerrainCellRData> data = (TerrainCellRData*)cell->GetRenderData ();

  if (data && materialPalette)
  {
    size_t numMats = materialPalette->GetSize ();

    // Set enough length
    if (data->svContextArrayMM.GetSize () < numMats)
    {
      data->svContextArrayMM.SetSize (numMats);
      data->alphaMapArrayMM.SetSize (numMats);
    }

    for (size_t matIdx = 0; matIdx < numMats; ++matIdx)
    {
      if (!data->svContextArrayMM[matIdx])
      {
        csRef<OverlaidShaderVariableContext> ctx;
        ctx.AttachNew (new OverlaidShaderVariableContext);
        ctx->SetParentContext (data->commonSVContext);

        data->svContextArrayMM.Put (matIdx, ctx);
      }

      if (!data->alphaMapArrayMM[matIdx])
      {
        csRef<iImage> alphaImg;
        alphaImg.AttachNew (new csImageMemory (cell->GetMaterialMapWidth (), 
          cell->GetMaterialMapHeight (), CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));

        csRef<iTextureHandle> txtHandle = graph3d->GetTextureManager ()->RegisterTexture (alphaImg, 
          CS_TEXTURE_3D | CS_TEXTURE_CLAMP);

        data->alphaMapArrayMM.Put (matIdx, txtHandle);

        csRef<csShaderVariable> var;
        var.AttachNew (new csShaderVariable(stringSet->Request ("splat alpha map")));
        var->SetType (csShaderVariable::TEXTURE);
        var->SetValue (data->alphaMapArrayMM[matIdx]);

        data->svContextArrayMM[matIdx]->AddVariable (var);
      }
    }
  }
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
