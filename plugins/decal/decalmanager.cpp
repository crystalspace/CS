
#include <cssysdef.h>
#include "decalmanager.h"
#include <iutil/objreg.h>
#include <iutil/plugin.h>
#include <iutil/object.h>
#include <iengine/mesh.h>
#include <iengine/engine.h>
#include <csgfx/renderbuffer.h>
#include <iengine/movable.h>
#include <imesh/genmesh.h>
#include <csutil/cscolor.h>
#include <iengine/material.h>
#include <imesh/object.h>
#include <cstool/collider.h>
#include <ivideo/material.h>
#include "csgfx/shadervarcontext.h"

csDecal::csDecal(iObjectRegistry * pObjectReg, iDecalManager * pManager, 
                 size_t id)
       : m_pObjectReg(pObjectReg), m_pManager(pManager), m_nID(id), 
         m_nIndexCount(0), m_nVertexCount(0), m_width(0), m_height(0)
{
  m_pEngine = csQueryRegistry<iEngine>(m_pObjectReg);

  m_pVertexBuffer = csRenderBuffer::CreateRenderBuffer(
          CS_DECAL_MAX_VERTS_PER_DECAL, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  m_pTexCoordBuffer = csRenderBuffer::CreateRenderBuffer(
          CS_DECAL_MAX_VERTS_PER_DECAL, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 2);
  m_pNormalBuffer = csRenderBuffer::CreateRenderBuffer(
          CS_DECAL_MAX_VERTS_PER_DECAL, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 3);
  m_pColorBuffer = csRenderBuffer::CreateRenderBuffer(
          CS_DECAL_MAX_VERTS_PER_DECAL, CS_BUF_STATIC, CS_BUFCOMP_FLOAT, 4);
  m_pIndexBuffer = csRenderBuffer::CreateIndexRenderBuffer(
          CS_DECAL_MAX_TRIS_PER_DECAL*3, CS_BUF_STATIC, 
          CS_BUFCOMP_UNSIGNED_INT, 0, CS_DECAL_MAX_TRIS_PER_DECAL*3-1);

  m_pBufferHolder.AttachNew(new csRenderBufferHolder);
  m_pBufferHolder->SetRenderBuffer(CS_BUFFER_INDEX, m_pIndexBuffer);
  m_pBufferHolder->SetRenderBuffer(CS_BUFFER_POSITION, m_pVertexBuffer);
  m_pBufferHolder->SetRenderBuffer(CS_BUFFER_TEXCOORD0, m_pTexCoordBuffer);
  m_pBufferHolder->SetRenderBuffer(CS_BUFFER_NORMAL, m_pNormalBuffer);
  m_pBufferHolder->SetRenderBuffer(CS_BUFFER_COLOR, m_pColorBuffer);
}

csDecal::~csDecal()
{
  for (size_t a=0; a<m_renderMeshes.Length(); ++a)
    delete m_renderMeshes[a];
}

void csDecal::InitializePosition(const csVector3& normal, 
        const csVector3& pos, const csVector3& up, const csVector3& right,
        float width, float height)
{
    m_normal = normal;
    m_pos = pos;

    m_width = width;
    m_height = height;

    m_up = up;
    m_right = right;
}

void csDecal::AddMeshPolys(iMeshWrapper* pMesh, iMaterialWrapper* pMaterial, 
          csArray<csPoly3D>& polys)
{
  size_t a, b;
  unsigned int tri[3];
  size_t firstIndex = m_nIndexCount;
  float normalThreshold = m_pManager->GetPolygonNormalThreshold();
  size_t polyCount = polys.GetSize();

  // check if we actually have some polygons to add
  if (polyCount == 0)
      return;

  // check if InitializePosition has been called with decent parameters
  if (m_width <= 0.01f || m_height <= 0.01f)
      return;

  // check if we hit our maximum allowed triangles
  if (m_nIndexCount >= CS_DECAL_MAX_TRIS_PER_DECAL*3)
      return;

  const csReversibleTransform& trans = 
      pMesh->GetMovable()->GetFullTransform();

  csVector3 localNormal = trans.Other2ThisRelative(m_normal);
  csVector3 localUp = trans.Other2ThisRelative(m_up);
  csVector3 localRight = trans.Other2ThisRelative(m_right);
  csVector3 vertOffset = localNormal * m_pManager->GetDecalOffset();
  csVector3 relPos = trans.Other2This(m_pos);

#ifdef CS_DECAL_CLIP_DECAL
  // four clip planes at the bottom, top, left, right decal borders
  // two more for near and far clipping
  csPlane3 cBottom = csPlane3(-localUp, -m_height*0.5f + localUp * relPos);
  csPlane3 cTop =    csPlane3( localUp, -m_height*0.5f - localUp * relPos);

  csPlane3 cLeft = csPlane3(-localRight, -m_width*0.5f + localRight * relPos);
  csPlane3 cRight =csPlane3( localRight, -m_width*0.5f - localRight * relPos);

  csPlane3 cNear, cFar;
  bool clipNearFar = m_pManager->GetNearFarClipping();
  if (clipNearFar)
  {
    float nearFarScale = m_pManager->GetNearFarScale();
    cNear = csPlane3(-localNormal, 
            -m_radius*nearFarScale + localNormal * relPos);
    cFar =  csPlane3( localNormal, 
            -m_radius*nearFarScale - localNormal * relPos);
  }

  for (a=0; a<polyCount; ++a)
  {
    polys[a].CutToPlane(cBottom);
    polys[a].CutToPlane(cTop);
    polys[a].CutToPlane(cLeft);
    polys[a].CutToPlane(cRight);

    if (clipNearFar)
    {
      polys[a].CutToPlane(cNear);
      polys[a].CutToPlane(cFar);
    }
  }
#endif
      
  for (a=0; a<polyCount; ++a)
  {
    csPoly3D* pPoly = &polys[a];
    size_t vertCount = pPoly->GetVertexCount();

    if (vertCount < 3)
        continue;
    
    if (-pPoly->ComputeNormal() * localNormal < normalThreshold)
        continue;

    // check if we hit our maximum allowed vertices
    if (m_nVertexCount + vertCount > CS_DECAL_MAX_VERTS_PER_DECAL)
        vertCount = CS_DECAL_MAX_VERTS_PER_DECAL - m_nVertexCount;

    if (vertCount == 0)
        break;

    tri[0] = m_nVertexCount;
    for (b=0; b<pPoly->GetVertexCount(); ++b)
    {
        csVector3 vertPos = *pPoly->GetVertex(b) + vertOffset;
        csVector3 relVert = vertPos - relPos;
        size_t vertIdx = m_nVertexCount+b;

        // copy over vertex data
        m_pVertexBuffer->CopyInto(&vertPos, 1, vertIdx);
        
        // create the index buffer for each triangle in the poly
        if (b >= 2)
        {
            tri[1] = vertIdx-1;
            tri[2] = vertIdx;
            m_pIndexBuffer->CopyInto(&tri, 3, m_nIndexCount);
            m_nIndexCount += 3;
        }

        // copy over a color
        float c[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        m_pColorBuffer->CopyInto(c, 1, vertIdx);

        // generate uv coordinates
        csVector2 texCoord((localRight * m_width) * relVert + 0.5f,
                            -(localUp * m_height) * relVert + 0.5f);
        m_pTexCoordBuffer->CopyInto(&texCoord, 1, vertIdx);
         
        // copy over normal
        m_pNormalBuffer->CopyInto(&localNormal, 1, vertIdx);
    }

    m_nVertexCount += vertCount;
  }

  // make sure we actually added some geometry before we create a rendermesh
  if (m_nIndexCount == firstIndex)
      return;

  printf("Creating decal rendermesh with %d indecies\n", 
          m_nIndexCount-firstIndex);
        
  // create a decal rendermesh for this mesh
  csRenderMesh* pRenderMesh = new csRenderMesh();
  m_renderMeshes.Push(pRenderMesh);
  pRenderMesh->mixmode = 0;
  pRenderMesh->meshtype = CS_MESHTYPE_TRIANGLES;
  pRenderMesh->indexstart = firstIndex;
  pRenderMesh->indexend = m_nIndexCount;
  pRenderMesh->material = pMaterial;
  pRenderMesh->buffers = m_pBufferHolder;
  pRenderMesh->geometryInstance = (void *)m_pBufferHolder;
  //m_variableContext.AttachNew(new csShaderVariableContext);
  //pRenderMesh->variablecontext = m_variableContext;
  pMesh->AddExtraRenderMesh(pRenderMesh);
}

CS_IMPLEMENT_PLUGIN
SCF_IMPLEMENT_FACTORY(csDecalManager)

csDecalManager::csDecalManager(iBase * parent)
              : scfImplementationType(this, parent), 
                objectReg(0) 
{
  polygonNormalThreshold = CS_DECAL_DEFAULT_NORMAL_THRESHOLD;
  decalOffset = CS_DECAL_DEFAULT_OFFSET;
  nearFarScale = CS_DECAL_DEFAULT_NEAR_FAR_SCALE;
  clipNearFar = CS_DECAL_DEFAULT_CLIP_NEAR_FAR;
}

csDecalManager::~csDecalManager()
{
  for (size_t a=0; a<decals.Length(); ++a)
    delete decals[a];
}

bool csDecalManager::Initialize(iObjectRegistry * objectReg)
{
  this->objectReg = objectReg;
  return true;
}

bool csDecalManager::CreateDecal(iMaterialWrapper * material, 
    iSector * sector, const csVector3 * pos, const csVector3 * up, 
    const csVector3 * normal, float width, float height)
{
  // compute the maximum distance the decal can reach
  float radius = sqrt(width*width + height*height) * 2.0f;

  csVector3 n = normal->Unit();

  // our generated up vector depends on the primary axis of the normal
  csVector3 normalAxis = normal->UnitAxisClamped(); 

  csVector3 right = n % *up;
  csVector3 correctUp = right % n;

  if (!engine)
  {
    engine = csQueryRegistry<iEngine>(objectReg);
    if (!engine)
    {
      printf("Couldn't query engine");
      return false;
    }
  }

  // get all meshes that could be affected by this decal
  bool success = false;
  iMeshWrapper * mesh;
  csDecal * pDecal = 0;
  csVector3 relPos;
  csRef<iMeshWrapperIterator> it = engine->GetNearbyMeshes(sector, *pos, 
                                                           radius, true);
  if (!it->HasNext())
      return false;

  pDecal = new csDecal(objectReg, this, decals.Length());
  pDecal->InitializePosition(n, *pos, correctUp, right, width, height);
  decals.Push(pDecal);
  while (it->HasNext())
  {
    mesh = it->Next();
    relPos = mesh->GetMovable()->GetFullTransform().Other2This(*pos);
    polygonBuffer.Empty();
    size_t polyCount = mesh->GetMeshObject()->TestPolygons(&relPos,
        radius, (iPolygonCallback *)this);

    if (polyCount > 0)
      pDecal->AddMeshPolys(mesh, material, polygonBuffer);
  }
  return success;
}

/*
bool csDecalManager::ProjectDecal(iMaterialWrapper * material, 
    iSector * sector, const csVector3 * start, const csVector3 * end, 
    const csVector3 * up, const csVector3 * normal, float width, 
    float height)
{
  if (!cdsys)
  {
    cdsys = csQueryRegistry<iCollideSystem>(objectReg);
    if (!cdsys)
    {
      printf("Failed to locate iCollideSystem!\n");
      return false;
    }
  }

  // trace a beam to get an intersection
  csVector3 iSect;
  csIntersectingTriangle closestTri;
  iMeshWrapper * selMesh;
  csColliderHelper::TraceBeam(cdsys, sector, *start, *end, true, closestTri, 
                              iSect, &selMesh);

  // if they specified a normal then use it, otherwise calculate one
  csVector3 n;
  if (normal != 0)
    n = *normal;
  else 
    n = (closestTri.b - closestTri.a) % (closestTri.c - closestTri.a);
  
  if (!selMesh || !selMesh->GetMovable() 
      || selMesh->GetMovable()->GetSectors()->GetCount() == 0)
    return false;

  return CreateDecal(material, selMesh->GetMovable()->GetSectors()->Get(0), 
                     &iSect, up, &n, width, height);
}
*/

void csDecalManager::SetPolygonNormalThreshold(float threshold)
{
  polygonNormalThreshold = threshold;
}

float csDecalManager::GetPolygonNormalThreshold() const
{
  return polygonNormalThreshold;
}

void csDecalManager::SetDecalOffset(float offset)
{
  decalOffset = offset;
}

float csDecalManager::GetDecalOffset() const
{
  return decalOffset;
}

void csDecalManager::SetNearFarClipping(bool clip)
{
  clipNearFar = clip;
}

bool csDecalManager::GetNearFarClipping() const
{
  return clipNearFar;
}

void csDecalManager::SetNearFarScale(float scale)
{
  nearFarScale = scale;
}

float csDecalManager::GetNearFarScale() const
{
  return nearFarScale;
}

void csDecalManager::AddPolygon(const csPoly3D & poly)
{
  polygonBuffer.Push(poly);
}

