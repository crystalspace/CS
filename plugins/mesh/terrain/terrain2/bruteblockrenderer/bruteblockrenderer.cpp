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

#include "bruteblockrenderer.h"

#include "iengine/camera.h"
#include "iengine/movable.h"
#include "iengine/rview.h"
#include "csgfx/imagememory.h"
#include "csgfx/renderbuffer.h"
#include "cstool/rviewclipper.h" 
#include "csutil/objreg.h"
#include "ivideo/txtmgr.h"

#include "cellrdata.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

SCF_IMPLEMENT_FACTORY (csTerrainBruteBlockRenderer)

// File-static data
CS::ShaderVarStringID csTerrainBruteBlockRenderer::textureLodDistanceID = CS::InvalidShaderVarStringID;

// Match cell2 to cell1 (gives where cell2 attach to cell1)
static TerrainCellBorderMatch MatchCell (iTerrainCell* cell1, iTerrainCell* cell2)
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

//-- THE TERRAIN RENDERER ITSELF
csTerrainBruteBlockRenderer::csTerrainBruteBlockRenderer (iBase* parent)
  : scfImplementationType (this, parent), engine (nullptr), materialPalette (0)
{  
}

csTerrainBruteBlockRenderer::~csTerrainBruteBlockRenderer ()
{
  textureLodDistanceID = CS::InvalidShaderVarStringID;
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

  textureLodDistanceID = stringSet->Request ("texture lod distance");

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

	{
	  csRef<csShaderVariable> var;
	  var.AttachNew (new csShaderVariable(stringSet->Request ("splat alpha map")));
	  var->SetType (csShaderVariable::TEXTURE);
	  var->SetValue (data->alphaMapArrayMM[matIdx]);

	  data->svContextArrayMM[matIdx]->AddVariable (var);
	}
	{
	  csRef<csShaderVariable> var;
	  var.AttachNew (new csShaderVariable(stringSet->Request ("alpha map scale")));
	  float inv_matmap_w = 1.0f / cell->GetMaterialMapWidth ();
	  float inv_matmap_h = 1.0f / cell->GetMaterialMapHeight ();
	  var->SetValue (
	    csVector4 ((cell->GetMaterialMapWidth ()-1) * inv_matmap_w,
		       (cell->GetMaterialMapHeight ()-1) * inv_matmap_h,
		       0.5f * inv_matmap_w,
		       0.5f * inv_matmap_h));

	  data->svContextArrayMM[matIdx]->AddVariable (var);
	}
      }
    }
  }
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
