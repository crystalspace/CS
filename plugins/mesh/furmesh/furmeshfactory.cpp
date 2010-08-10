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

#include <cssysdef.h>

#include "furmesh.h"
#include "furmeshfactory.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /********************
  *  FurMeshFactory
  ********************/

  CS_LEAKGUARD_IMPLEMENT(FurMeshFactory);

  FurMeshFactory::FurMeshFactory (iEngine *e, iObjectRegistry* reg, iMeshObjectType* type)
    : scfImplementationType(this, e, reg, type), indexCount(0), vertexCount(0), 
    indexBuffer(0), vertexBuffer(0), texcoordBuffer(0), tangentBuffer(0), binormalBuffer(0)
  {
  }

  FurMeshFactory::~FurMeshFactory ()
  {
  }

  csPtr<iMeshObject> FurMeshFactory::NewInstance ()
  {
    csRef<iMeshObject> ref;
    ref.AttachNew (new FurMesh(Engine, object_reg, this));
    return csPtr<iMeshObject> (ref);
  }

  /// geometry access
  void FurMeshFactory::SetVertexCount (uint n)
  {
    vertexCount = n;

    vertexBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

    if (!vertexBuffer)
      csPrintfErr("Could not create vertex buffer!\n");

    texcoordBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 2);

    if (!texcoordBuffer)
      csPrintfErr("Could not create texcoord buffer!\n");

    normalBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

    if (!normalBuffer)
      csPrintfErr("Could not create normal buffer!\n");

    binormalBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

    if (!binormalBuffer)
      csPrintfErr("Could not create binormal buffer!\n");

    tangentBuffer = csRenderBuffer::CreateRenderBuffer (n, 
      CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);

    if (!tangentBuffer)
      csPrintfErr("Could not create tangent buffer!\n");
  }

  void FurMeshFactory::SetTriangleCount (uint n)
  {
    if (!vertexCount)
      return;

    indexCount = n;

    indexBuffer = csRenderBuffer::CreateIndexRenderBuffer (3 * n, 
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, vertexCount - 1);

    if (!indexBuffer)
      csPrintfErr("Could not create index buffer!\n");
  }

  uint FurMeshFactory::GetVertexCount() const
  {
    return vertexCount;
  }

  uint FurMeshFactory::GetIndexCount() const
  {
    return indexCount;
  }

  iRenderBuffer* FurMeshFactory::GetIndices () const
  {
    return indexBuffer;
  }

  bool FurMeshFactory::SetIndices (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetComponentCount () < 3)
      return false;

    indexBuffer = renderBuffer;
    indexCount = (uint)indexBuffer->GetElementCount ();
    return true;
  }

  iRenderBuffer* FurMeshFactory::GetVertices () const
  {
    return vertexBuffer;
  }

  bool FurMeshFactory::SetVertices (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetComponentCount () < 3)
      return false;

    vertexBuffer = renderBuffer;
    vertexCount = (uint)vertexBuffer->GetElementCount ();

    return true;
  }

  iRenderBuffer* FurMeshFactory::GetTexCoords () const
  {
    return texcoordBuffer;
  }

  bool FurMeshFactory::SetTexCoords (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    texcoordBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshFactory::GetNormals () const
  {
    return normalBuffer;
  }

  bool FurMeshFactory::SetNormals (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    normalBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshFactory::GetTangents () const
  {
    return tangentBuffer;
  }

  bool FurMeshFactory::SetTangents (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    tangentBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshFactory::GetBinormals () const
  {
    return binormalBuffer;
  }

  bool FurMeshFactory::SetBinormals (iRenderBuffer* renderBuffer)
  { 
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    binormalBuffer = renderBuffer;    
    return true;
  }

  /********************
  *  FurMeshType
  ********************/

  SCF_IMPLEMENT_FACTORY (FurMeshType)

    CS_LEAKGUARD_IMPLEMENT(FurMeshType);

  FurMeshType::FurMeshType (iBase* parent) :
  scfImplementationType (this, parent),
    object_reg(0), Engine(0)
  {
  }

  FurMeshType::~FurMeshType ()
  {
  }

  // From iComponent
  bool FurMeshType::Initialize (iObjectRegistry* r)
  {
    csRef<iEngine> e = csQueryRegistry<iEngine> (r);
    Engine = e;

    if (!e)
    {
      csPrintfErr("Could not find engine!\n");
      return false;
    }

    object_reg = r;
    return true;
  }

  csPtr<iMeshObjectFactory> FurMeshType::NewFactory ()
  {
    csRef<iMeshObjectFactory> ref;
    ref.AttachNew (new FurMeshFactory(Engine, object_reg, this));
    return csPtr<iMeshObjectFactory> (ref);
  }

}
CS_PLUGIN_NAMESPACE_END(FurMesh)
