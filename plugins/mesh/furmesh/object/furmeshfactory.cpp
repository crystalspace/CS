/*
  Copyright (C) 2010 Alexandru - Teodor Voicu
      Faculty of Automatic Control and Computer Science of the "Politehnica"
      University of Bucharest
      http://csite.cs.pub.ro/index.php/en/

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
#include "hairphysicscontrol.h"
#include "animationphysicscontrol.h"
#include "furmeshfactory.h"
#include "furmeshproperties.h"

CS_PLUGIN_NAMESPACE_BEGIN(FurMesh)
{
  /********************
  *  FurMeshFactory
  ********************/

  CS_LEAKGUARD_IMPLEMENT(FurMeshFactory);

  FurMeshFactory::FurMeshFactory (iEngine *e, iObjectRegistry* reg, 
    iMeshObjectType* type) : scfImplementationType(this, e, reg, type)
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

  bool FurMeshType::Initialize (iObjectRegistry* r)
  {
    csRef<iEngine> e = csQueryRegistry<iEngine> (r);
    Engine = e;

    // This should not happen
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

  CS::Mesh::iFurMeshMaterialProperties* FurMeshType::
    CreateHairMeshMarschnerProperties (const char* name)
  {
    csRef<CS::Mesh::iFurMeshMaterialProperties> newFurMeshMaterial;
    newFurMeshMaterial.AttachNew(new HairMeshProperties (object_reg));
    return furMeshMaterialPropertiesHash.PutUnique (name, newFurMeshMaterial);
  }

  CS::Mesh::iFurMeshMaterialProperties* FurMeshType::
    CreateFurMeshBasicProperties (const char* name)
  {
    csRef<CS::Mesh::iFurMeshMaterialProperties> newFurMeshMaterial;
    newFurMeshMaterial.AttachNew(new FurMeshProperties (object_reg));
    return furMeshMaterialPropertiesHash.PutUnique (name, newFurMeshMaterial);
  }

  CS::Mesh::iFurMeshMaterialProperties* FurMeshType::
    FindFurMeshMaterialProperites (const char* name) const
  {
    return furMeshMaterialPropertiesHash.Get (name, 0);
  }

  void FurMeshType::RemoveFurMeshMaterialProperites (const char* name)
  {
    CS::Mesh::iFurMeshMaterialProperties* newFurMeshMaterial = 
      FindFurMeshMaterialProperites(name);

    if (!newFurMeshMaterial)
      furMeshMaterialPropertiesHash.Delete (name, newFurMeshMaterial);    
  }

  void FurMeshType::ClearFurMeshMaterialProperites ()
  {
    furMeshMaterialPropertiesHash.DeleteAll ();
  }

  CS::Animation::iFurAnimationControl* FurMeshType::CreateFurPhysicsControl (const char* name)
  {
    csRef<CS::Animation::iFurAnimationControl> newAnimationControl;
    newAnimationControl.AttachNew(new HairPhysicsControl (this));
    return furAnimationControlHash.PutUnique (name, newAnimationControl);
  }

  CS::Animation::iFurAnimationControl* FurMeshType::CreateFurAnimatedMeshControl (const char* name)
  {
    csRef<CS::Animation::iFurAnimationControl> newAnimationControl;
    newAnimationControl.AttachNew(new AnimationPhysicsControl (this));
    return furAnimationControlHash.PutUnique (name, newAnimationControl);
  }

  CS::Animation::iFurAnimationControl* FurMeshType::FindFurAnimationControl (const char* name) const
  {
    return furAnimationControlHash.Get (name, 0);
  }

  void FurMeshType::RemoveFurAnimationControl (const char* name)
  {
    CS::Animation::iFurAnimationControl* newAnimationControl = FindFurAnimationControl(name);
    
    if (!newAnimationControl)
      furAnimationControlHash.Delete (name, newAnimationControl);
  }

  void FurMeshType::ClearFurAnimationControls ()
  {
    furAnimationControlHash.DeleteAll ();
  }

  /********************
  *  FurMeshGeometry
  ********************/

  CS_LEAKGUARD_IMPLEMENT(FurMeshGeometry);

  FurMeshGeometry::FurMeshGeometry () : 
    indexCount(0), vertexCount(0), indexBuffer(0), vertexBuffer(0), 
    texcoordBuffer(0), tangentBuffer(0), binormalBuffer(0)
  {
  }

  FurMeshGeometry::~FurMeshGeometry ()
  {
  }


  // Also allocates the vertex render buffers
  void FurMeshGeometry::SetVertexCount (uint n)
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

  // Also allocates the index render buffer
  // The index buffer will be 3 * n, set triangle count not index count
  void FurMeshGeometry::SetTriangleCount (uint n)
  {
    if (!vertexCount)
      return;

    indexCount = n;

    indexBuffer = csRenderBuffer::CreateIndexRenderBuffer (3 * n, 
      CS_BUF_STATIC, CS_BUFCOMP_UNSIGNED_INT, 0, vertexCount - 1);

    if (!indexBuffer)
      csPrintfErr("Could not create index buffer!\n");
  }

  uint FurMeshGeometry::GetVertexCount() const
  {
    return vertexCount;
  }

  uint FurMeshGeometry::GetIndexCount() const
  {
    return indexCount;
  }

  iRenderBuffer* FurMeshGeometry::GetIndices () const
  {
    return indexBuffer;
  }

  bool FurMeshGeometry::SetIndices (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetComponentCount () < 3)
      return false;

    indexBuffer = renderBuffer;
    indexCount = (uint)indexBuffer->GetElementCount ();
    return true;
  }

  iRenderBuffer* FurMeshGeometry::GetVertices () const
  {
    return vertexBuffer;
  }

  bool FurMeshGeometry::SetVertices (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetComponentCount () < 3)
      return false;

    vertexBuffer = renderBuffer;
    vertexCount = (uint)vertexBuffer->GetElementCount ();

    return true;
  }

  iRenderBuffer* FurMeshGeometry::GetTexCoords () const
  {
    return texcoordBuffer;
  }

  bool FurMeshGeometry::SetTexCoords (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    texcoordBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshGeometry::GetNormals () const
  {
    return normalBuffer;
  }

  bool FurMeshGeometry::SetNormals (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    normalBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshGeometry::GetTangents () const
  {
    return tangentBuffer;
  }

  bool FurMeshGeometry::SetTangents (iRenderBuffer* renderBuffer)
  {
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    tangentBuffer = renderBuffer;    
    return true;
  }

  iRenderBuffer* FurMeshGeometry::GetBinormals () const
  {
    return binormalBuffer;
  }

  bool FurMeshGeometry::SetBinormals (iRenderBuffer* renderBuffer)
  { 
    if (!renderBuffer)
      return false;

    if (renderBuffer->GetElementCount () < vertexCount)
      return false;

    binormalBuffer = renderBuffer;    
    return true;
  }

}
CS_PLUGIN_NAMESPACE_END(FurMesh)
