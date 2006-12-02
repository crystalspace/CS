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

  struct ObjectVertexData;
  
  class LightmapUVLayoutFactory;

  class LightmapUVLayouter 
  {
  public:
    virtual ~LightmapUVLayouter() {}
    /**
     * Lay out lightmaps for a factory.
     * This should split vertices as necessary and assign primitives
     * to lightmaps.
     * \param inPrims Input primitives.
     * \param inPrims Input vertex data.
     * \param outPrims Output primitives. A number of primitive arrays. All
     *   primitives of a sub-array would fit on a single lightmap.
     */
    virtual csPtr<LightmapUVLayoutFactory> LayoutFactory (
      const PrimitiveArray& inPrims, ObjectVertexData& vertexData,
      csArray<PrimitiveArray>& outPrims) = 0;
  };

  class LightmapUVLayoutFactory : public csRefCount
  {
  public:
    virtual ~LightmapUVLayoutFactory() {}
    /**
     * Lay out lightmaps for primitives of ab object.
     * \param prims Input primitives.
     * \param groupNum Index of the primitive arrays as returned by 
     *   LightmapUVLayouter::LayoutFactory.
     * \param inPrims Input vertex data.
     * \param lmID Output global lightmap ID onto which all primitives were
     *   layouted.
     */
    virtual bool LayoutUVOnPrimitives (PrimitiveArray &prims, 
      size_t groupNum, ObjectVertexData& vertexData, uint& lmID) = 0;
  };

} // namespace lighter

#endif // __LIGHTMAPUV_H__
