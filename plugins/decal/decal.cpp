/*
    Copyright (C) 2006 by Andrew Robberts

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

#include "cssysdef.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/movable.h"
#include "imesh/genmesh.h"
#include "iengine/material.h"
#include "imesh/object.h"
#include "ivideo/material.h"
#include "iutil/eventq.h"

#include "csgeom/tri.h"
#include "csgfx/renderbuffer.h"
#include "csgfx/shadervarcontext.h"
#include "csutil/event.h"
#include "cstool/collider.h"

#include "decal.h"
#include "decaltemplate.h"

csDecal::csDecal(iObjectRegistry * objectReg, iDecalManager * pManager)
       : objectReg(objectReg),
         indexCount(0), vertexCount(0), width(0), height(0), currMesh(0)
{
  engine = csQueryRegistry<iEngine>(objectReg);

  vertexBuffer = csRenderBuffer::CreateRenderBuffer(
          CS_DECAL_MAX_VERTS_PER_DECAL, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  texCoordBuffer = csRenderBuffer::CreateRenderBuffer(
          CS_DECAL_MAX_VERTS_PER_DECAL, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
  normalBuffer = csRenderBuffer::CreateRenderBuffer(
          CS_DECAL_MAX_VERTS_PER_DECAL, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  indexBuffer = csRenderBuffer::CreateIndexRenderBuffer(
          CS_DECAL_MAX_TRIS_PER_DECAL*3, CS_BUF_STATIC, 
          CS_BUFCOMP_UNSIGNED_INT, 0, CS_DECAL_MAX_TRIS_PER_DECAL*3-1);

  bufferHolder.AttachNew(new csRenderBufferHolder);
  bufferHolder->SetRenderBuffer(CS_BUFFER_INDEX, indexBuffer);
  bufferHolder->SetRenderBuffer(CS_BUFFER_POSITION, vertexBuffer);
  bufferHolder->SetRenderBuffer(CS_BUFFER_TEXCOORD0, texCoordBuffer);
  bufferHolder->SetRenderBuffer(CS_BUFFER_NORMAL, normalBuffer);
}

csDecal::~csDecal()
{
  for (size_t a=0; a<renderMeshInfos.GetSize(); ++a)
  {
    renderMeshInfos[a].mesh->RemoveExtraRenderMesh(
	renderMeshInfos[a].pRenderMesh);
    delete renderMeshInfos[a].pRenderMesh;
  }
}

void csDecal::Initialize(iDecalTemplate * decalTemplate, 
    const csVector3 & normal, const csVector3 & pos, const csVector3 & up, 
    const csVector3 & right, float width, float height)
{
  this->decalTemplate = decalTemplate;
    
  this->normal = normal;
  this->pos = pos;

  this->width = width;
  this->height = height;

  invWidth = 1.0f / width;
  invHeight = 1.0f / height;
           
  this->up = up;
  this->right = right;

  life = 0;
}

void csDecal::BeginMesh(iMeshWrapper * mesh)
{
  currMesh = 0;

  // check if InitializePosition has been called with decent parameters
  if (width <= 0.01f || height <= 0.01f)
    return;

  // check if we hit our maximum allowed triangles
  if (indexCount >= CS_DECAL_MAX_TRIS_PER_DECAL*3)
    return;

  firstIndex = indexCount;
  const csReversibleTransform& trans = 
      mesh->GetMovable()->GetFullTransform();

  localNormal = trans.Other2ThisRelative(normal);
  localUp = trans.Other2ThisRelative(up);
  localRight = trans.Other2ThisRelative(right);
  vertOffset = localNormal * decalTemplate->GetDecalOffset();
  relPos = trans.Other2This(pos);

#ifdef CS_DECAL_CLIP_DECAL
  // bottom
  clipPlanes[0] = csPlane3(-localUp, -height*0.5f + localUp * relPos);
  
  // top
  clipPlanes[1] = csPlane3( localUp, -height*0.5f - localUp * relPos);

  // left
  clipPlanes[2] = csPlane3(-localRight, -width*0.5f + localRight * relPos);

  // right
  clipPlanes[3] = csPlane3( localRight, -width*0.5f - localRight * relPos);

  if (decalTemplate->HasNearFarClipping())
  {
    float halfNearFarDist = decalTemplate->GetNearFarClippingDist() * 0.5f;
    clipPlanes[4] = csPlane3(-localNormal, 
            -halfNearFarDist + localNormal * relPos);
    clipPlanes[5] = csPlane3( localNormal, 
            -halfNearFarDist - localNormal * relPos);
  }
#endif // CS_DECAL_CLIP_DECAL

  // we didn't encounter any errors, so validate the current mesh
  currMesh = mesh;
}

void csDecal::AddStaticPoly(const csPoly3D & p)
{
  size_t a;
  CS::TriangleT<size_t> tri;

  if (!currMesh)
      return;

  csPoly3D poly = p;

#ifdef CS_DECAL_CLIP_DECAL
  size_t numClipPlanes = decalTemplate->HasNearFarClipping() ? 6 : 4;
  for (a=0; a<numClipPlanes; ++a)
      poly.CutToPlane(clipPlanes[a]);
#endif // CS_DECAL_CLIP_DECAL
  
  size_t vertCount = poly.GetVertexCount();

  // only support triangles and up
  if (vertCount < 3)
    return;
    
  // ensure the polygon isn't facing away from the decal's normal too much
  if (-poly.ComputeNormal() * localNormal < 
          decalTemplate->GetPolygonNormalThreshold())
    return;

  // check if we hit our maximum allowed vertices
  if (vertexCount + vertCount > CS_DECAL_MAX_VERTS_PER_DECAL)
    vertCount = CS_DECAL_MAX_VERTS_PER_DECAL - vertexCount;

  if (vertCount < 3)
    return;

  // check if we hit our maximum allowed indecies
  size_t idxCount = (vertCount - 2) * 3;
  if (indexCount + idxCount > CS_DECAL_MAX_TRIS_PER_DECAL*3)
    return;

  const csVector2 & minTexCoord = decalTemplate->GetMinTexCoord();
  csVector2 texCoordRange = decalTemplate->GetMaxTexCoord() - minTexCoord;

  tri[0] = vertexCount;
  for (a=0; a<vertCount; ++a)
  {
    csVector3 vertPos = *poly.GetVertex(a) + vertOffset;
    csVector3 relVert = vertPos - relPos;
    size_t vertIdx = vertexCount+a;

    // copy over vertex data
    vertexBuffer->CopyInto(&vertPos, 1, vertIdx);
        
    // create the index buffer for each triangle in the poly
    if (a >= 2)
    {
      tri[1] = vertIdx-1;
      tri[2] = vertIdx;
      indexBuffer->CopyInto(&tri, 3, indexCount);
      indexCount += 3;
    }

    // generate uv coordinates
    csVector2 texCoord(
      minTexCoord.x + texCoordRange.x * 0.5f + 
        texCoordRange.x * localRight * invWidth * relVert,
      minTexCoord.y + texCoordRange.y * 0.5f - 
        texCoordRange.y * localUp * invHeight * relVert);
    texCoordBuffer->CopyInto(&texCoord, 1, vertIdx);
     
    // copy over normal
    normalBuffer->CopyInto(&localNormal, 1, vertIdx);
  }

  vertexCount += vertCount;
}

void csDecal::EndMesh()
{
  if (!currMesh)
      return;

  // make sure we actually added some geometry before we create a rendermesh
  if (indexCount == firstIndex)
      return;

  // create a rendermesh for this mesh
  csRenderMesh* pRenderMesh = new csRenderMesh();
  
  csDecalRenderMeshInfo renderMeshInfo;
  renderMeshInfo.pRenderMesh = pRenderMesh;
  renderMeshInfo.mesh = currMesh;
  renderMeshInfos.Push(renderMeshInfo);

  pRenderMesh->mixmode = decalTemplate->GetMixMode();
  pRenderMesh->meshtype = CS_MESHTYPE_TRIANGLES;
  pRenderMesh->indexstart = firstIndex;
  pRenderMesh->indexend = indexCount;
  pRenderMesh->material = decalTemplate->GetMaterialWrapper();
  pRenderMesh->buffers = bufferHolder;
  pRenderMesh->geometryInstance = (void *)bufferHolder;
  //variableContext.AttachNew(new csShaderVariableContext);
  //pRenderMesh->variablecontext = variableContext;
  currMesh->AddExtraRenderMesh(pRenderMesh, 
          decalTemplate->GetRenderPriority(), decalTemplate->GetZBufMode());
}

bool csDecal::Age(csTicks ticks)
{
  const float lifespan = decalTemplate->GetTimeToLive();
  life += (float)ticks * 0.001f;

  if (lifespan <= 0.0f)
    return true;

  return life < lifespan;
}
