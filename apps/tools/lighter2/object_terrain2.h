/*
  Copyright (C) 2008 by Frank Richter

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

#ifndef __OBJECT_TERRAIN2_H__
#define __OBJECT_TERRAIN2_H__

#include "object.h"

namespace lighter
{
  class ObjectFactory_Terrain2;

  class Object_Terrain2 : public Object
  {
    // Vertex data for primitives
    ObjectVertexData primVertexData;

    struct LMDimensions
    {
      int w, h;
    };
    csArray<LMDimensions> lmDims;
  public:
    Object_Terrain2 (ObjectFactory_Terrain2* factory);

    bool Initialize (Sector* sector);

    // Parse data
    virtual void ParseMesh (iMeshWrapper *wrapper);

    void PrepareLighting ();

    void SaveMeshPostLighting (Scene* scene);

    virtual void SaveMesh (iDocumentNode *node);
  };

  class ObjectFactory_Terrain2 : public ObjectFactory
  {
    friend class Object_Terrain2;

    csRef<LightmapUVFactoryLayouter> uvlayout;
  public:
    ObjectFactory_Terrain2 (const Configuration& config);

    // Get a new object
    virtual csPtr<Object> CreateObject ();

    bool PrepareLightmapUV (LightmapUVFactoryLayouter* uvlayout);

    // Parse data
    virtual void ParseFactory (iMeshFactoryWrapper *factory);

    // Write out the data again
    //virtual void SaveFactory (iDocumentNode *node);

  };

} // namespace lighter

#endif // __OBJECT_TERRAIN2_H__
