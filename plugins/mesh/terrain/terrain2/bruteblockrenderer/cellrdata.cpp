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

#include "cellrdata.h"
#include "overlaidsvc.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

TerrainCellRData::TerrainCellRData (iTerrainCell* cell, 
                                    csTerrainBruteBlockRenderer* renderer)
  : rootBlock (0), cell (cell), renderer (renderer)
{
  properties = (TerrainBBCellRenderProperties*)cell->GetRenderProperties (); 

  blockResolution = properties->GetBlockResolution ();

  commonSVContext.AttachNew (
    new OverlaidShaderVariableContext (cell->GetRenderProperties ()));
  svContextArrayMM.SetSize (renderer->GetMaterialPalette ().GetSize ());
  alphaMapArrayMM.SetSize (renderer->GetMaterialPalette ().GetSize ());
  alphaMapMMUse.SetSize (renderer->GetMaterialPalette ().GetSize ());

  // Setup the base context

  svAccessor.AttachNew (new TerrainBBSVAccessor (properties));

  csRef<csShaderVariable> lodVar; lodVar.AttachNew (
    new csShaderVariable (csTerrainBruteBlockRenderer::textureLodDistanceID));
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

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
