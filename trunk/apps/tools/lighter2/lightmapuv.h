/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __LIGHTMAPUV_H__
#define __LIGHTMAPUV_H__

#include "primitive.h"
#include "lightmap.h"

namespace lighter
{

  struct ObjectFactoryVertexData;
  struct ObjectVertexData;
  
  class LightmapUVObjectLayouter;
  class ObjectFactory;

  class LightmapUVFactoryLayouter : public csRefCount
  {
  public:
    virtual ~LightmapUVFactoryLayouter () {}
    /**
     * Lay out lightmaps for a factory.
     * This should split vertices as necessary and assign primitives
     * to lightmaps.
     * \param inPrims Input primitives.
     * \param inPrims Input vertex data.
     * \param factory Factory object primitives are part of
     * \param outPrims Output primitives. A number of primitive arrays. All
     *   primitives of a sub-array would fit on a single lightmap.
     * \param noSplit Indicates that no splitting is to be done.
     */
    virtual csPtr<LightmapUVObjectLayouter> LayoutFactory (
      const FactoryPrimitiveArray& inPrims, ObjectFactoryVertexData& vertexData,
      const ObjectFactory* factory, csArray<FactoryPrimitiveArray>& outPrims,
      csBitArray& usedVerts, bool noSplit) = 0;

    virtual void PrepareLighting (Statistics::Progress& progress) = 0;

    /**
     * Allocate a complete lightmap. Useful when a mesh can provide a layout
     * on its own, specialized to the particular mesh type.
     */
    virtual uint AllocLightmap (uint lmW = 0, uint ulmH = 0) = 0;
  };

  class LightmapUVObjectLayouter : public csRefCount
  {
  public:
    /**
     * Lay out lightmaps for primitives of an object.
     * \param prims Input primitives.
     * \param groupNum Index of the primitive arrays as returned by 
     *   LightmapUVLayouter::LayoutFactory.
     * \param vertexData Input vertex data.
     * \param lmID Output global lightmap ID onto which all primitives were
     *   layouted.
     */
    virtual size_t LayoutUVOnPrimitives (PrimitiveArray &prims, 
      size_t groupNum, Sector* sector, const csBitArray& pdBits) = 0;

    virtual void FinalLightmapLayout (PrimitiveArray &prims, size_t layoutID,
      size_t groupNum, ObjectVertexData& vertexData, uint& lmID) = 0;
  };

} // namespace lighter

#endif // __LIGHTMAPUV_H__
