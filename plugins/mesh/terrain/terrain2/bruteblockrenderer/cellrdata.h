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

#ifndef __CELLRDATA_H__
#define __CELLRDATA_H__

#include "cellrenderproperties.h"
#include "overlaidsvc.h"
#include "svaccessor.h"
#include "terrainblock.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

enum TerrainCellBorderMatch
{
  CELL_MATCH_NONE = -1,
  CELL_MATCH_TOP = 0,
  CELL_MATCH_RIGHT = 1,
  CELL_MATCH_LEFT = 2,
  CELL_MATCH_BOTTOM = 3
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

}
CS_PLUGIN_NAMESPACE_END(Terrain2)

#endif // __CELLRDATA_H__
