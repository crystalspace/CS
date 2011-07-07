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

#include "terrainblock.h"

#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "csgfx/renderbuffer.h"
#include "cstool/rbuflock.h"
#include "cstool/rviewclipper.h"

#include "cellrdata.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

class TerrainBlock::BufferAccessor
  : public scfImplementation1<TerrainBlock::BufferAccessor,
			      iRenderBufferAccessor>
{
  TerrainBlock* block;
public:
  BufferAccessor (TerrainBlock* block)
   : scfImplementationType (this), block (block) {}
  
  void PreGetBuffer (csRenderBufferHolder* holder, 
		     csRenderBufferName buffer)
  {
    block->SetupTangentsBitangents();
    holder->SetAccessor (nullptr, 0);
  }
};

//---------------------------------------------------------------------

TerrainBlock::TerrainBlock ()
: stepSize (0), childIndex (0), parent (0), renderData (0), dataValid (false),
  tangentsBitangentsValid (false)
{
  for (size_t i = 0; i < 4; ++i)
  {
    children[i] = 0;
    neighbours[i] = 0;
  }
}

TerrainBlock::~TerrainBlock ()
{
  if (bufferHolder) bufferHolder->SetAccessor (nullptr, 0);
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
  csRef<iRenderBufferAccessor> accessor;
  accessor.AttachNew (new BufferAccessor (this));
  bufferHolder->SetAccessor (accessor, CS_BUFFER_TANGENT_MASK
				       | CS_BUFFER_BINORMAL_MASK);

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

void TerrainBlock::SetupTangentsBitangents ()
{
  if (tangentsBitangentsValid)
    return;

  size_t numVerts = (renderData->blockResolution) + 1;

  // Allocate the standard renderbuffers
  meshTangents = csRenderBuffer::CreateRenderBuffer (numVerts*numVerts, 
    CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  meshBitangents = csRenderBuffer::CreateRenderBuffer (numVerts*numVerts,
    CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);

  bufferHolder->SetRenderBuffer (CS_BUFFER_TANGENT, meshTangents);
  bufferHolder->SetRenderBuffer (CS_BUFFER_BINORMAL, meshBitangents);

  {
    // Lock and write the buffers
    csRenderBufferLock<csVector3> tangentData (meshTangents);
    csRenderBufferLock<csVector3> bitangentData (meshBitangents);

    // Get the data
    csLockedNormalData cellTData = renderData->cell->GetTangentData ();
    csLockedNormalData cellBData = renderData->cell->GetBitangentData ();

    for (size_t y = 0, gridY = gridTop; y < numVerts; ++y, gridY += stepSize)
    {
      csVector3* tRow = cellTData.data + cellTData.pitch * gridY + gridLeft;
      csVector3* bRow = cellBData.data + cellBData.pitch * gridY + gridLeft;

      for (size_t x = 0; x < numVerts; ++x, tRow += stepSize, bRow += stepSize)
      {
        csVector3 tangent = *tRow;
        csVector3 bitangent = *bRow;

        *tangentData++ = tangent;
        *bitangentData++ = bitangent;
      }
    }
  }


  tangentsBitangentsValid = true;
}

void TerrainBlock::InvalidateGeometry (bool recursive)
{
  dataValid = false;
  meshVertices = meshNormals = meshTexCoords = 0;
  boundingBox = csBox3 ();
  tangentsBitangentsValid = false;
  meshTangents.Invalidate();
  meshBitangents.Invalidate();

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

  // Get the optional alpha-splat material.
  iMaterialWrapper* alphaSplatMaterial = renderData->cell->GetAlphaSplatMaterial ();
  iMaterialWrapper* splatBaseMaterial = renderData->cell->GetSplatBaseMaterial ();

  // Calculate the WS distance from the camera to this cell.
  float fCellDistance = sqrt (boundingBox.SquaredPosDist (rview->GetCamera ()->GetTransform ().GetOrigin ()));

  // Calculate whether to render the base material or the splatting.
  bool renderSplatting = (fCellDistance < renderData->properties->GetSplatDistance ());
  renderSplatting &= (!palette.IsEmpty () || alphaSplatMaterial);

  // Get the render priority of terrain splatting.
  CS::Graphics::RenderPriority splatPrio = renderData->properties->GetSplatRenderPriorityValue();

  const int renderBaseMaterial = -2;
  const int renderAlphaSplatMaterial = -1;
  for (int j = -2; j < (int)palette.GetSize (); ++j)
  {
    iMaterialWrapper* mat = 0;
    iShaderVariableContext* svContext = 0;

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
      if (j == renderBaseMaterial)
      {
        if (!splatBaseMaterial)
          continue;

        mat = splatBaseMaterial;
        svContext = renderData->commonSVContext;
      }
      else if (j == renderAlphaSplatMaterial)
      {
        // Check that we have this material.
        if (!alphaSplatMaterial)
          continue;

        mat = alphaSplatMaterial;
        svContext = renderData->commonSVContext;
      }
      else
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
    if (j >= 0)
      mesh->renderPrio = splatPrio;
    else
      mesh->renderPrio = CS::Graphics::RenderPriority ();

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

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
