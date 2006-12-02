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

#ifndef __LIGHTMAPUV_SIMPLE_H__
#define __LIGHTMAPUV_SIMPLE_H__

#include "primitive.h"
#include "lightmapuv.h"

namespace lighter
{

  class SimpleUVLayoutFactory;

  class SimpleUVLayouter : public LightmapUVLayouter
  {
  public:
    SimpleUVLayouter (LightmapPtrDelArray& lightmaps) : 
      globalLightmaps (lightmaps)
    {}

    virtual csPtr<LightmapUVLayoutFactory> LayoutFactory (
      const PrimitiveArray& inPrims, ObjectVertexData& vertexData,
      csArray<PrimitiveArray>& outPrims);
  protected:
    friend class SimpleUVLayoutFactory;

    LightmapPtrDelArray& globalLightmaps;

    void DetermineNeighbouringPrims (
      const PrimitiveArray& inPrims, ObjectVertexData& vertexData,
      csArray<PrimitiveArray>& outPrims);

    bool AllocLightmap (LightmapPtrDelArray& lightmaps, int u, int v, 
      csRect &lightmapArea, int &lightmapID);

    bool ProjectPrimitives (PrimitiveArray& prims, 
      BoolDArray &usedVerts, float uscale, float vscale);
  };

  class SimpleUVLayoutFactory : public LightmapUVLayoutFactory
  {
  public:
    SimpleUVLayoutFactory (SimpleUVLayouter* parent) : parent (parent)
    {}

    virtual bool LayoutUVOnPrimitives (PrimitiveArray &prims, 
      size_t groupNum, ObjectVertexData& vertexData, uint& lmID);
  protected:
    friend class SimpleUVLayouter;
    SimpleUVLayouter* parent;
    csArray<csArray<csArray<size_t> > > coplanarGroups;

    void MapComplete (const csArray<csVector2>& sizes, 
      const csArray<csVector2>& minuvs, 
      csArray<csVector2>& remaps, uint& lmID);
  };

} // namespace lighter

#endif // __LIGHTMAPUV_SIMPLE_H__
