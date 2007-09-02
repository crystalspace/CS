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

#include "radprimitive.h"
#include "lightmap.h"

namespace lighter
{

  struct RadObjectVertexData;
  
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
    virtual LightmapUVLayoutFactory* LayoutFactory (
      const RadPrimitiveArray& inPrims, RadObjectVertexData& vertexData,
      csArray<RadPrimitiveArray>& outPrims) = 0;
  };

  class LightmapUVLayoutFactory
  {
  public:
    virtual ~LightmapUVLayoutFactory() {}
    /**
     * Lay out lightmaps for primitives of ab object.
     * \param prims Input primitives.
     * \param inPrims Input vertex data.
     * \param lmID Output global lightmap ID onto which all primitives were
     *   layouted.
     */
    virtual bool LayoutUVOnPrimitives (RadPrimitiveArray &prims, 
      RadObjectVertexData& vertexData, uint& lmID) = 0;
  };

  class SimpleUVLayoutFactory;

  class SimpleUVLayouter : public LightmapUVLayouter
  {
  public:
    SimpleUVLayouter (LightmapPtrDelArray& lightmaps) : 
      globalLightmaps (lightmaps)
    {}

    virtual LightmapUVLayoutFactory* LayoutFactory (
      const RadPrimitiveArray& inPrims, RadObjectVertexData& vertexData,
      csArray<RadPrimitiveArray>& outPrims);
  protected:
    friend class SimpleUVLayoutFactory;

    LightmapPtrDelArray& globalLightmaps;

    bool AllocLightmap (LightmapPtrDelArray& lightmaps, int u, int v, 
      csRect &lightmapArea, int &lightmapID);

    bool ProjectPrimitive (RadPrimitive &prim, BoolDArray &usedVerts,
      float uscale, float vscale);
  };

  class SimpleUVLayoutFactory : public LightmapUVLayoutFactory
  {
  public:
    SimpleUVLayoutFactory (SimpleUVLayouter* parent) : parent (parent) {}

    virtual bool LayoutUVOnPrimitives (RadPrimitiveArray &prims, 
      RadObjectVertexData& vertexData, uint& lmID);
  protected:
    SimpleUVLayouter* parent;
  };

} // namespace lighter

#endif // __LIGHTMAPUV_H__
