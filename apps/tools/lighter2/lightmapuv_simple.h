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

  class SimpleUVObjectLayouter;

  class SimpleUVFactoryLayouter : public LightmapUVFactoryLayouter
  {
  public:
    SimpleUVFactoryLayouter (LightmapPtrDelArray& lightmaps) : 
      globalLightmaps (lightmaps)
    {}

    virtual csPtr<LightmapUVObjectLayouter> LayoutFactory (
      const FactoryPrimitiveArray& inPrims, ObjectFactoryVertexData& vertexData,
      const ObjectFactory* factory,
      csArray<FactoryPrimitiveArray>& outPrims,
      csBitArray& usedVerts);
  protected:
    friend class SimpleUVObjectLayouter;

    LightmapPtrDelArray& globalLightmaps;

    /**
     * Vertex equality criterium, for determination of nieghbouring primitives.
     */
    enum VertexEquality
    {
      /// Two vertices are considered equal if they're the same vertex.
      Pedantic,
      /**
       * Two vertices are considered equal if they have the same position and
       * normal.
       */
      PositionAndNormal,
      /// Two vertices are considered equal if they have the same position
      Position
    };

    struct Edge
    {
      const FactoryPrimitive& prim;
      size_t a, b;

      Edge (const FactoryPrimitive& prim, size_t a, size_t b) : 
        prim (prim), a (a), b (b) {}

      bool equals (VertexEquality veq, const Edge& other);
    };
    struct UberPrimitive
    {
      FactoryPrimitiveArray prims;
      csArray<Edge> outsideEdges;
      VertexEquality equality;

      UberPrimitive (VertexEquality equality, 
        const FactoryPrimitive& startPrim);

      bool UsesEdge (const Edge& edge);
      void AddPrimitive (const FactoryPrimitive& prim);
    };

    void DetermineNeighbouringPrims (
      const FactoryPrimitiveArray& inPrims, ObjectFactoryVertexData& vertexData,
      csArray<FactoryPrimitiveArray>& outPrims);

    bool AllocLightmap (LightmapPtrDelArray& lightmaps, int u, int v, 
      csRect &lightmapArea, int &lightmapID);

    bool ProjectPrimitives (FactoryPrimitiveArray& prims, 
      csBitArray &usedVerts, float uscale, float vscale,
      Vector2Array& lightmapUVs);
  };

  class SimpleUVObjectLayouter : public LightmapUVObjectLayouter
  {
  public:
    SimpleUVObjectLayouter (SimpleUVFactoryLayouter* parent) : parent (parent)
    {}

    virtual bool LayoutUVOnPrimitives (PrimitiveArray &prims, 
      size_t groupNum, ObjectVertexData& vertexData, uint& lmID);
  protected:
    friend class SimpleUVFactoryLayouter;
    SimpleUVFactoryLayouter* parent;
    Vector2Array lightmapUVs;
    csArray<csArray<csArray<size_t> > > coplanarGroups;

    void MapComplete (const csArray<csVector2>& sizes, 
      const csArray<csVector2>& minuvs, 
      csArray<csVector2>& remaps, uint& lmID);
  };

} // namespace lighter

#endif // __LIGHTMAPUV_SIMPLE_H__
