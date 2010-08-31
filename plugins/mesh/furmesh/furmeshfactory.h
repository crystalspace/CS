/*
  Copyright (C) 2010 Alexandru - Teodor Voicu

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __FUR_MESHFACTORY_H__
#define __FUR_MESHFACTORY_H__

#include <iutil/comp.h>
#include <csgeom/vector3.h>
#include <imesh/furmesh.h>
#include <csgfx/shadervarcontext.h>
#include <imesh/genmesh.h>

#include "crystalspace.h"
#include "csutil/scf_implementation.h"

struct iObjectRegistry;

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  class FurMeshFactory : public scfImplementationExt1<FurMeshFactory, 
    csMeshFactory, iFurMeshFactory>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMeshFactory);

    FurMeshFactory (iEngine *e, iObjectRegistry* reg, iMeshObjectType* type);
    virtual ~FurMeshFactory ();

    virtual csPtr<iMeshObject> NewInstance ();
    virtual csPtr<iMeshObjectFactory> Clone () { return 0; }

    /// geometry access
    virtual void SetVertexCount (uint n);
    virtual void SetTriangleCount (uint n);

    virtual uint GetVertexCount();
    virtual uint GetIndexCount();

    virtual iRenderBuffer* GetIndices ();
    virtual bool SetIndices (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetVertices ();
    virtual bool SetVertices (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetTexCoords ();
    virtual bool SetTexCoords (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetNormals ();
    virtual bool SetNormals (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetTangents ();
    virtual bool SetTangents (iRenderBuffer* renderBuffer);
    virtual iRenderBuffer* GetBinormals ();
    virtual bool SetBinormals (iRenderBuffer* renderBuffer);

  protected:
    uint indexCount;
    uint vertexCount;
    csRef<iRenderBuffer> indexBuffer;
    csRef<iRenderBuffer> vertexBuffer;
    csRef<iRenderBuffer> texcoordBuffer;
    csRef<iRenderBuffer> normalBuffer;
    csRef<iRenderBuffer> tangentBuffer;
    csRef<iRenderBuffer> binormalBuffer;
  };

  class FurMeshType : public 
    scfImplementation2<FurMeshType,iFurMeshType,iComponent>
  {
  public:
    CS_LEAKGUARD_DECLARE(FurMeshType);

    FurMeshType (iBase* parent);
    virtual ~FurMeshType ();

    // From iComponent	
    virtual bool Initialize (iObjectRegistry*);

    // From iMeshObjectType
    virtual csPtr<iMeshObjectFactory> NewFactory ();

  private:
    iObjectRegistry* object_reg;
    /// pointer to the engine if available.
    iEngine *Engine;
  };
}
CS_PLUGIN_NAMESPACE_END(FurMesh)

#endif // __FUR_MESH_H__
