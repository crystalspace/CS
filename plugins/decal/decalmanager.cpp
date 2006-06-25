
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

csDecal::csDecal(iObjectRegistry * objectReg, iDecalManager * manager, 
                 size_t id)
       : objectReg(objectReg), manager(manager), id(id)
{
  engine = csQueryRegistry<iEngine>(objectReg);
}

csDecal::~csDecal()
{
}

bool csDecal::Create(iMaterialWrapper * material, iSectorList * sectors, 
    const csReversibleTransform * trans, const csArray<csPoly3D> *  polys,
    const csVector3 * decalRelPos, const csVector3 * normal, 
    const csVector3 * up, const csVector3 * right, float width, float height)
{
  this->material = material;

  // build unique names
  char facName[32];
  char meshName[32];
  sprintf(facName, "decal_fact_%zd", id);
  sprintf(facName, "decal_mesh_%zd", id);
  
  // create a genmesh factory
  meshFact = engine->CreateMeshFactory("crystalspace.mesh.object.genmesh", 
                                       facName);
  iMeshObjectFactory * fact = meshFact->GetMeshObjectFactory();
  csRef<iGeneralFactoryState> factState = 
    scfQueryInterface<iGeneralFactoryState>(fact);

  if (!factState)
  {
    printf("Couldn't create genmesh factory");
    return false;
  }

  float normalThreshold = manager->GetPolygonNormalThreshold();

  // count the number of vertices and triangles
  size_t vertCount = 0;
  size_t triCount = 0;
  size_t p, v;
  for (p=0; p<polys->Length(); ++p)
  {
    const csPoly3D * poly = &polys->Get(p);
    size_t c = poly->GetVertexCount();
    if (c < 3)
      continue;

    if (-poly->ComputeNormal() * *normal < normalThreshold)
      continue;
    
    vertCount += c;
    triCount += c - 2;
  }
  
  factState->SetVertexCount(vertCount);
  factState->SetTriangleCount(triCount);

  // we'll project the vertex onto these vectors to compute the u,v coords
  csVector3 uvRight = *right / width;
  csVector3 uvUp = *up / height;

  // offset to avoid z fighting
  csVector3 offset = *normal * manager->GetDecalOffset();

  size_t vertIdx = 0;
  size_t triIdx = 0;
  for (p=0; p<polys->Length(); ++p)
  {
    const csPoly3D * poly = &polys->Get(p);

    // we only support triangles and up
    if (poly->GetVertexCount() < 3)
      continue;

    // ignore perpendicular or backfacing polygons (depending on threshold)
    csVector3 polyNormal = poly->ComputeNormal();
    if (-polyNormal * *normal < normalThreshold)
      continue;


    size_t firstVertIdx = vertIdx;
    
    for (v=0; v<poly->GetVertexCount(); ++v)
    {
      csVector3 vert = *poly->GetVertex(v) + offset;

      // vector from the decal's center to the vertex; used for u,v calcs
      csVector3 relVert = vert - *decalRelPos;

      factState->GetVertices()[vertIdx] = vert;
      factState->GetNormals()[vertIdx] = *normal;
      factState->GetColors()[vertIdx].Set(1,1,1,1);
      factState->GetTexels()[vertIdx].Set(uvRight * relVert + 0.5f, 
                                          -uvUp * relVert + 0.5f);
      
      if (v >= 2)
      {
        factState->GetTriangles()[triIdx].Set(firstVertIdx, 
                                              vertIdx-1, 
                                              vertIdx);
        ++triIdx;
      }
      ++vertIdx;
    }
    
  }
  
  mesh = engine->CreateMeshWrapper(meshFact, meshName,
      sectors->Get(0), trans->GetOrigin());
  if (!mesh)
  {
    printf("Couldn't create genmesh");
    return false;
  }
  for (int sectorIdx = 1; sectorIdx < sectors->GetCount(); ++sectorIdx)
    mesh->GetMovable()->GetSectors()->Add(sectors->Get(sectorIdx));

  mesh->GetMeshObject()->SetMaterialWrapper(material);
  mesh->GetMovable()->SetTransform(*trans);

  mesh->GetFlags().Set(CS_ENTITY_NOHITBEAM);

  return true;
}

void Delete()
{
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
  csDecal * decal;
  csReversibleTransform trans;
  csRef<iMeshWrapperIterator> it = engine->GetNearbyMeshes(sector, *pos, 
                                                           radius, true);
  csVector3 decalRelPos;
  while (it->HasNext())
  {
    mesh = it->Next();
    decalRelPos = trans.Other2This(*pos);

    polygonBuffer.Empty();
    size_t polyCount = mesh->GetMeshObject()->TestPolygons(&decalRelPos,
        radius, (iPolygonCallback *)this);

    if (polyCount > 0)
    {
#ifdef CS_DECAL_CLIP_DECAL
      // four clip planes at the bottom, top, left, right decal borders
      // two more for near and far clipping
      csPlane3 cBottom = csPlane3(-correctUp, -height*0.5f 
                                  + correctUp * decalRelPos);
      csPlane3 cTop =    csPlane3( correctUp, -height*0.5f 
                                  - correctUp * decalRelPos);
      csPlane3 cLeft =   csPlane3(-right, -width*0.5f + right * decalRelPos);
      csPlane3 cRight =  csPlane3( right, -width*0.5f - right * decalRelPos);
      csPlane3 cNear, cFar;
      if (clipNearFar)
      {
        cNear =   csPlane3(-n, -radius*nearFarScale + n * decalRelPos);
        cFar =    csPlane3( n, -radius*nearFarScale - n * decalRelPos);
      }

      for (size_t p = 0; p<polygonBuffer.Length(); ++p)
      {
        polygonBuffer[p].CutToPlane(cBottom);
        polygonBuffer[p].CutToPlane(cTop);
        polygonBuffer[p].CutToPlane(cLeft);
        polygonBuffer[p].CutToPlane(cRight);
        if (clipNearFar)
        {
          polygonBuffer[p].CutToPlane(cNear);
          polygonBuffer[p].CutToPlane(cFar);
        }
      }
#endif

      decal = new csDecal(objectReg, this, decals.Length());
      trans = mesh->GetMovable()->GetFullTransform();
      success |= decal->Create(material, 
          mesh->GetMovable()->GetSectors(), &trans, &polygonBuffer,
          &decalRelPos, &n, &correctUp, &right, width, height);
      decals.Push(decal);
    }
  }
  return success;
}

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

