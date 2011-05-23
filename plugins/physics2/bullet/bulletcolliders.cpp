#include "cssysdef.h"

#include "csgeom/sphere.h"
#include "csgeom/tri.h"
#include "csutil/stringquote.h"
#include "igeom/trimesh.h"
#include "imesh/objmodel.h"
#include "iengine/movable.h"

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btConeShape.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btCylinderShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h"

#include "bulletcolliders.h"
#include "bulletcollision.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

static csRef<iTriangleMesh> FindColdetTriangleMesh (iMeshWrapper* mesh,
                                                 csStringID baseID, csStringID colldetID)
{
  iObjectModel* objModel = mesh->GetMeshObject ()->GetObjectModel ();
  csRef<iTriangleMesh> triMesh;
  bool use_trimesh = objModel->IsTriangleDataSet (baseID);
  if (use_trimesh)
  {
    if  (objModel->GetTriangleData (colldetID))
       triMesh = objModel->GetTriangleData (colldetID);
    else
      triMesh = objModel->GetTriangleData (baseID);
  }

  if (!triMesh || triMesh->GetVertexCount () == 0
      || triMesh->GetTriangleCount () == 0)
  {
    csFPrintf (stderr, "iCollider: No collision polygons, triangles or vertices on mesh factory %s\n",
      CS::Quote::Single (mesh->QueryObject ()->GetName ()));

    return NULL;
  }
  return triMesh;
}

#include "csutil/custom_new_disable.h"

static btTriangleMesh* GenerateTriMeshData (iMeshWrapper* mesh, csStringID baseID, 
                                         csStringID colldetID, float internalScale)
{
  csRef<iTriangleMesh> triMesh = FindColdetTriangleMesh (mesh, baseID, colldetID);
  if (!triMesh)
    return NULL;
  btTriangleMesh* btMesh = new btTriangleMesh ();
  
  size_t triangleCount = triMesh->GetTriangleCount ();
  size_t vertexCount = triMesh->GetVertexCount ();

  size_t i;
  csTriangle *c_triangle = triMesh->GetTriangles ();
  csVector3 *c_vertex = triMesh->GetVertices ();
  for (i =0;i<triangleCount;i++)
  {
    int index0 = c_triangle[i].a;
    int index1 = c_triangle[i].b;
    int index2 = c_triangle[i].c;

    btVector3 vertex0 (c_vertex[index0].x, c_vertex[index0].y, c_vertex[index0].z);
    btVector3 vertex1 (c_vertex[index1].x, c_vertex[index1].y, c_vertex[index1].z);
    btVector3 vertex2 (c_vertex[index2].x, c_vertex[index2].y, c_vertex[index2].z);

    vertex0 *= internalScale;
    vertex1 *= internalScale;
    vertex2 *= internalScale;

    btMesh->addTriangle (vertex0,vertex1,vertex2);
  }
  return btMesh;
}

#include "csutil/custom_new_enable.h"

csBulletCollider::csBulletCollider ()
  : scfImplementationType (this), collSector (NULL), 
    scale (1,1,1), shape (NULL), margin(0.0)
{
  //Lulu: Create a btEmptyShape for shape is safe but not efficient...
}

void csBulletCollider::SetLocalScale (const csVector3& scale)
{
  this->scale = scale;
  if(shape)
    shape->setLocalScaling (btVector3(scale.x * collSector->internalScale,
    scale.y * collSector->internalScale,
    scale.z * collSector->internalScale));
}

void csBulletCollider::SetMargin (float margin)
{
  if (margin > 0.0f)
  {
    this->margin = margin;
    if (shape)
      shape->setMargin (margin * collSector->internalScale);
  }
}

float csBulletCollider::GetMargin () const
{
  return margin;
}

csBulletColliderBox::csBulletColliderBox (csBulletCollisionSector* sector,
                                          const csVector3& boxSize)
  : scfImplementationType (this), boxSize (boxSize)
{
}

csBulletColliderBox::~csBulletColliderBox ()
{
  if (shape)
    delete shape;
}

void csBulletColliderBox::GenerateShape ()
{
  if (!shape)
  {
    shape = new btBoxShape (CSToBullet (boxSize*0.5f,collSector->internalScale));
    // For user who want to set margin localScale before really generate a shape.
    // Seems weired...Can I forbid user set these parameters before rebuild?
    SetMargin(margin);
    SetLocalScale(scale);
  }
}

csBulletColliderSphere::csBulletColliderSphere (csBulletCollisionSector* sector,
                                             float radius)
  : scfImplementationType (this), radius (radius)
{
}

csBulletColliderSphere::~csBulletColliderSphere ()
{
  if (shape)
    delete shape;
}

void csBulletColliderSphere::GenerateShape ()
{
  if (!shape)
  {
    shape = new btSphereShape (radius * collSector->internalScale);
    SetMargin(margin);
    SetLocalScale(scale);
  }
}

void csBulletColliderSphere::SetLocalScale (const csVector3& scale)
{
  if ((scale.x - scale.y) < EPSILON && (scale.y - scale.z) < EPSILON) //x == y == z
  {
    this->scale = scale;
    if(shape)
      dynamic_cast<btSphereShape*>(shape)->setUnscaledRadius(scale.x * collSector->internalScale);
  }
}

csBulletColliderCylinder::csBulletColliderCylinder (csBulletCollisionSector* sector, float length, float radius)
  : scfImplementationType (this), length (length), radius (radius)
{
}

csBulletColliderCylinder::~csBulletColliderCylinder ()
{
  if(shape)
    delete shape;
}

void csBulletColliderCylinder::GenerateShape ()
{
  if (!shape)
  {
    shape = new btCylinderShapeZ (btVector3 (radius * collSector->internalScale,
                                  radius * collSector->internalScale,
                                  length * collSector->internalScale * 0.5f));
    SetMargin(margin);
    SetLocalScale(scale);
  }
}

void csBulletColliderCylinder::GetCylinderGeometry (float& length, float& radius)
{
  length = this->length;
  radius = this->radius;
}

csBulletColliderCapsule::csBulletColliderCapsule (csBulletCollisionSector* sector, 
                                               float length, float radius)
  : scfImplementationType (this), length (length), radius (radius)
{
}

csBulletColliderCapsule::~csBulletColliderCapsule ()
{
  if (shape)
    delete shape;
}

void csBulletColliderCapsule::GenerateShape ()
{
  if (!shape)
  {  
    shape = new btCapsuleShapeZ (radius * collSector->internalScale,
                                 length * collSector->internalScale);
    SetMargin(margin);
    SetLocalScale(scale);
  }
}

void csBulletColliderCapsule::GetCapsuleGeometry (float& length, float& radius)
{
  length = this->length;
  radius = this->radius;
}

csBulletColliderCone::csBulletColliderCone (csBulletCollisionSector* sector, float length, float radius)
  : scfImplementationType (this), length (length), radius (radius)
{
}

csBulletColliderCone::~csBulletColliderCone ()
{
  if (shape)
    delete shape;
}

void csBulletColliderCone::GenerateShape ()
{
  if (!shape)
  {
    shape = new btConeShapeZ (radius * collSector->internalScale,
                              length * collSector->internalScale);
    SetMargin(margin);
    SetLocalScale(scale);
  }
}

void csBulletColliderCone::GetConeGeometry (float& length, float& radius)
{
  length = this->length;
  radius = this->radius;
}

csBulletColliderPlane::csBulletColliderPlane (csBulletCollisionSector* sector,
                                           const csPlane3& plane)
  : scfImplementationType (this), plane (plane)
{
}

csBulletColliderPlane::~csBulletColliderPlane ()
{
  if (shape)
    delete shape;
}

void csBulletColliderPlane::GenerateShape ()
{
  if (!shape)
  {
    csVector3 normal = plane.GetNormal ();
    shape = new btStaticPlaneShape (btVector3 (normal.x,normal.y,normal.z),
                                    plane.D () * collSector->internalScale);
    SetMargin(margin);
    SetLocalScale(scale);
  }
}

csBulletColliderConvexMesh::csBulletColliderConvexMesh (csBulletCollisionSector* sector,
                                                     iMeshWrapper* mesh)
  : scfImplementationType (this), mesh (mesh)
{
}

csBulletColliderConvexMesh::~csBulletColliderConvexMesh ()
{
  if (shape)
    delete shape;
}

void csBulletColliderConvexMesh::GenerateShape ()
{
  if (!shape)
  {
    btTriangleMesh* triMesh = GenerateTriMeshData (mesh, collSector->baseID, collSector->colldetID, collSector->internalScale);
    if (! triMesh)
      return;
    btConvexShape* tmpConvexShape = new btConvexTriangleMeshShape (triMesh);
    btShapeHull* hull = new btShapeHull (tmpConvexShape);
    btScalar margin = tmpConvexShape->getMargin ();
    hull->buildHull (margin);
    tmpConvexShape->setUserPointer (hull);

    btConvexHullShape* convexShape = new btConvexHullShape ();
    for  (int i=0 ; i < hull->numVertices ();i++)
    {
      convexShape->addPoint (hull->getVertexPointer ()[i]);	
    }
    shape = convexShape;

    delete tmpConvexShape;
    delete hull;
    delete triMesh;
    SetMargin(margin);
    SetLocalScale(scale);
  }
}

csBulletColliderConcaveMesh::csBulletColliderConcaveMesh (csBulletCollisionSector* sector,
                                                       iMeshWrapper* mesh)
  : scfImplementationType (this), mesh (mesh)
{
}

csBulletColliderConcaveMesh::~csBulletColliderConcaveMesh ()
{
  if (shape)
    delete shape;
}

void csBulletColliderConcaveMesh::GenerateShape ()
{
  if (!shape)
  {
    btTriangleMesh* triMesh = GenerateTriMeshData (mesh, collSector->baseID, collSector->colldetID, collSector->internalScale);
    shape = new btBvhTriangleMeshShape (triMesh,true);
    SetMargin(margin);
    SetLocalScale(scale);
  }
}

csBulletColliderConcaveMeshScaled::csBulletColliderConcaveMeshScaled (csBulletCollisionSector* sector,
                                                                   iColliderConcaveMesh* collider, csVector3 scale)
  : scfImplementationType (this)
{
  this->scale = scale;
  originalCollider = dynamic_cast<csBulletColliderConcaveMesh*> (collider);
}

csBulletColliderConcaveMeshScaled::~csBulletColliderConcaveMeshScaled ()
{
  if(shape)
    delete shape;
}

void csBulletColliderConcaveMeshScaled::GenerateShape ()
{
  if(!shape)
  {
    if (originalCollider->shape->getShapeType () == TRIANGLE_MESH_SHAPE_PROXYTYPE)
    {
      btBvhTriangleMeshShape* meshShape = dynamic_cast<btBvhTriangleMeshShape*> (originalCollider->shape);
      shape = new btScaledBvhTriangleMeshShape (meshShape, 
        btVector3 (scale[0] * collSector->internalScale,
        scale[1] * collSector->internalScale,
        scale[2] * collSector->internalScale));
    }
    else
      csFPrintf  (stderr, "csBulletColliderConcaveMeshScaled: Original collider is not concave mesh.\n");
    SetMargin(margin);
  }
}

HeightMapCollider::HeightMapCollider ( float* gridData, 
                                   int gridWidth, int gridHeight, 
                                   csVector3 gridSize, 
                                   csVector3 position, 
                                   float minHeight, float maxHeight)
                                   : btHeightfieldTerrainShape (gridWidth, gridHeight,
                                   gridData, 1.0f, minHeight, maxHeight,
                                   1, PHY_FLOAT, false), cell (cell), 
                                   cellPosition (position), heightData (gridData),
{
  localScale.setValue (gridSize[0] / (gridWidth - 1),
    1.0,
    gridSize[2] / (gridHeight - 1));
}

HeightMapCollider::~HeightMapCollider ()
{
  delete heightData;
}

void HeightMapCollider::SetLocalScale (csVector3& scale)
{
  this->setLocalScaling (btVector3(localScale.x * scale,
    localScale.y * scale,
    localScale.z * scale));
}

void HeightMapCollider::UpdataMinHeight (float minHeight)
{

}

void HeightMapCollider::UpdateMaxHeight (float maxHeight)
{

}

csBulletColliderTerrain::csBulletColliderTerrain (csBulletCollisionSector* sector,
                                               iTerrainSystem* terrain,
                                               float minimumHeight,
                                               float maximumHeight)
  : scfImplementationType (this), minimumHeight (minimumHeight), 
  maximumHeight (maximumHeight), terrainSystem(terrain)
{
  unload = true;
  terrain->AddCellLoadListener (this);
}

csBulletColliderTerrain::~csBulletColliderTerrain ()
{
  for  (size_t i = 0; i < colliders.GetSize (); i++)
    delete colliders[i];
}

void csBulletColliderTerrain::GenerateShape ()
{
  if(unload)
  {
    for (size_t i =0; i<terrainSystem->GetCellCount (); i++)
    {
      iTerrainCell* cell = terrainSystem->GetCell (i);

      if (cell->GetLoadState () != iTerrainCell::Loaded)
        continue;
      LoadCellToCollider (cell);       
    }
    unload = true;
    SetMargin (margin);
    SetLocalScale (scale);
  }
}

void csBulletColliderTerrain::SetLocalScale (const csVector3& scale)
{
  this->scale = scale;
  for (size_t i = 0; i < colliders.GetSize (); i++)
    colliders[i]->SetLocalScale (scale * collSector->internalScale);
}

void csBulletColliderTerrain::SetMargin (float margin)
{
  this->margin = margin;
  for (size_t i = 0; i < colliders.GetSize (); i++)
    colliders[i]->setMargin (margin * collSector->internalScale);
}

void csBulletColliderTerrain::OnCellLoad (iTerrainCell *cell)
{
  LoadCellToCollider (cell);
}

void csBulletColliderTerrain::OnCellPreLoad (iTerrainCell *cell)
{
}

void csBulletColliderTerrain::OnCellUnload (iTerrainCell *cell)
{
  for (size_t i = 0;i<colliders.GetSize ();i++)
    if (colliders[i]->cell == cell)
    {
      colliders.DeleteIndexFast (i);
      break;
    }
}

void csBulletColliderTerrain::LoadCellToCollider (iTerrainCell *cell)
{
  float minHeight,maxHeight;
  csLockedHeightData gridData = cell->GetHeightData ();
  bool needExtremum =  (minimumHeight == 0.0f && maximumHeight == 0.0f);
  if  (needExtremum)
    minHeight = maxHeight = gridData.data[0];
  int gridWidth = cell->GetGridWidth ();
  int gridHeight = cell->GetGridHeight ();

  float* heightData = new float[gridHeight*gridWidth];
  for (int i=0;i<gridWidth;i++)
    for (int j=0;j<gridHeight;j++)
    {
      float height = heightData[ (gridWidth-i-1) * gridWidth + j]
      = gridData.data[i * gridWidth + j];

      if (needExtremum)
      {
        minHeight = MIN (minHeight, height);
        maxHeight = MAX (maxHeight, height);
      }
    }
  
  csVector3 cellPosition  (cell->GetPosition ()[0], 0.0f, cell->GetPosition ()[1]);
  HeightMapCollider* colliderData = new HeightMapCollider (
    collSector, heightData, gridWidth, gridHeight,
    cell->GetSize (), cellPosition, minHeight, maxHeight);

  colliders.Push (colliderData);
}

const csVector3& csBulletColliderTerrain::GetCellPosition (size_t index)
{
  return colliders[index]->cellPosition;
}

}
CS_PLUGIN_NAMESPACE_END (Bullet2)
