
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
         m_nIndexCount(0), m_nVertexCount(0), m_width(0), m_height(0),
         m_pCurrMesh(0)
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

void csDecal::Initialize(iMaterialWrapper* pMaterial, const csVector3& normal,
          const csVector3& pos, const csVector3& up, const csVector3& right,
          float width, float height)
{
    m_pMaterial = pMaterial;
    
    m_normal = normal;
    m_pos = pos;

    m_width = width;
    m_height = height;

    m_up = up;
    m_right = right;
}

void csDecal::BeginMesh(iMeshWrapper* pMesh)
{
  m_pCurrMesh = 0;

  // check if InitializePosition has been called with decent parameters
  if (m_width <= 0.01f || m_height <= 0.01f)
      return;

  // check if we hit our maximum allowed triangles
  if (m_nIndexCount >= CS_DECAL_MAX_TRIS_PER_DECAL*3)
      return;

  m_firstIndex = m_nIndexCount;
  const csReversibleTransform& trans = 
      pMesh->GetMovable()->GetFullTransform();

  m_localNormal = trans.Other2ThisRelative(m_normal);
  m_localUp = trans.Other2ThisRelative(m_up);
  m_localRight = trans.Other2ThisRelative(m_right);
  m_vertOffset = m_localNormal * m_pManager->GetDecalOffset();
  m_relPos = trans.Other2This(m_pos);

#ifdef CS_DECAL_CLIP_DECAL
  // bottom
  m_clipPlanes[0] = 
      csPlane3(-m_localUp, -m_height*0.5f + m_localUp * m_relPos);
  
  // top
  m_clipPlanes[1] = 
      csPlane3( m_localUp, -m_height*0.5f - m_localUp * m_relPos);

  // left
  m_clipPlanes[2] = 
      csPlane3(-m_localRight, -m_width*0.5f + m_localRight * m_relPos);

  // right
  m_clipPlanes[3] =
      csPlane3( m_localRight, -m_width*0.5f - m_localRight * m_relPos);

  if (m_pManager->GetNearFarClipping())
  {
    float nearFarScale = m_pManager->GetNearFarScale();
    m_clipPlanes[4] = csPlane3(-m_localNormal, 
            -m_radius*nearFarScale + m_localNormal * m_relPos);
    m_clipPlanes[5] = csPlane3( m_localNormal, 
            -m_radius*nearFarScale - m_localNormal * m_relPos);
  }
#endif // CS_DECAL_CLIP_DECAL

  // we didn't encounter any errors, so validate the current mesh
  m_pCurrMesh = pMesh;
}

void csDecal::AddStaticPoly(const csPoly3D& p)
{
  size_t a;
  unsigned int tri[3];

  if (!m_pCurrMesh)
      return;

  csPoly3D poly = p;

#ifdef CS_DECAL_CLIP_DECAL
  size_t numClipPlanes = m_pManager->GetNearFarClipping() ? 6 : 4;
  for (a=0; a<numClipPlanes; ++a)
      poly.CutToPlane(m_clipPlanes[a]);
#endif // CS_DECAL_CLIP_DECAL
  
  size_t vertCount = poly.GetVertexCount();

  // only support triangles and up
  if (vertCount < 3)
    return;
    
  // ensure the polygon isn't facing away from the decal's normal too much
  if (-poly.ComputeNormal() * m_localNormal < 
          m_pManager->GetPolygonNormalThreshold())
    return;

  // check if we hit our maximum allowed vertices
  if (m_nVertexCount + vertCount > CS_DECAL_MAX_VERTS_PER_DECAL)
    vertCount = CS_DECAL_MAX_VERTS_PER_DECAL - m_nVertexCount;

  if (vertCount < 3)
    return;

  tri[0] = m_nVertexCount;
  for (a=0; a<vertCount; ++a)
  {
    csVector3 vertPos = *poly.GetVertex(a) + m_vertOffset;
    csVector3 relVert = vertPos - m_relPos;
    size_t vertIdx = m_nVertexCount+a;

    // copy over vertex data
    m_pVertexBuffer->CopyInto(&vertPos, 1, vertIdx);
        
    // create the index buffer for each triangle in the poly
    if (a >= 2)
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
    csVector2 texCoord((m_localRight * m_width) * relVert + 0.5f,
                        -(m_localUp * m_height) * relVert + 0.5f);
    m_pTexCoordBuffer->CopyInto(&texCoord, 1, vertIdx);
     
    // copy over normal
    m_pNormalBuffer->CopyInto(&m_localNormal, 1, vertIdx);
  }

  m_nVertexCount += vertCount;
}

void csDecal::EndMesh()
{
  if (!m_pCurrMesh)
      return;

  // make sure we actually added some geometry before we create a rendermesh
  if (m_nIndexCount == m_firstIndex)
      return;

  printf("Creating decal rendermesh with %d indecies\n", 
          m_nIndexCount-m_firstIndex);

  // create a rendermesh for this mesh
  csRenderMesh* pRenderMesh = new csRenderMesh();
  m_renderMeshes.Push(pRenderMesh);
  pRenderMesh->mixmode = 0;
  pRenderMesh->meshtype = CS_MESHTYPE_TRIANGLES;
  pRenderMesh->indexstart = m_firstIndex;
  pRenderMesh->indexend = m_nIndexCount;
  pRenderMesh->material = m_pMaterial;
  pRenderMesh->buffers = m_pBufferHolder;
  pRenderMesh->geometryInstance = (void *)m_pBufferHolder;
  //m_variableContext.AttachNew(new csShaderVariableContext);
  //pRenderMesh->variablecontext = m_variableContext;
  m_pCurrMesh->AddExtraRenderMesh(pRenderMesh);
}

CS_IMPLEMENT_PLUGIN
SCF_IMPLEMENT_FACTORY(csDecalManager)

csDecalManager::csDecalManager(iBase * parent)
              : scfImplementationType(this, parent), 
                objectReg(0), m_pCurrDecal(0)
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
  csDecal * pDecal = 0;
  csVector3 relPos;
  csRef<iMeshWrapperIterator> it = engine->GetNearbyMeshes(sector, *pos, 
                                                           radius, true);
  if (!it->HasNext())
      return false;

  pDecal = new csDecal(objectReg, this, decals.Length());
  pDecal->Initialize(material, n, *pos, correctUp, right, width, height);
  decals.Push(pDecal);
  m_pCurrDecal = pDecal;
  while (it->HasNext())
  {
    iMeshWrapper* pMesh = it->Next();
    csVector3 relPos = 
        pMesh->GetMovable()->GetFullTransform().Other2This(*pos);

    pDecal->BeginMesh(pMesh);
    //pMesh->GetMeshObject()->TestPolygons(&relPos, radius, 
    //        (iPolygonCallback*)this);
    pMesh->GetMeshObject()->BuildDecal(&relPos, radius, 
            (iDecalBuilder*)pDecal);
    pDecal->EndMesh();
  }
  m_pCurrDecal = 0;
  return true;
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
  if (!m_pCurrDecal)
      return;

  m_pCurrDecal->AddStaticPoly(poly);
  //polygonBuffer.Push(poly);
}

