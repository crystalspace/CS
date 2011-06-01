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

#include "colliders.h"
#include "bullet.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet2)
{

csRef<iTriangleMesh> FindColdetTriangleMesh (iMeshWrapper* mesh,
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

btTriangleMesh* GenerateTriMeshData (iMeshWrapper* mesh, csStringID baseID, 
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

csBulletCollider::csBulletCollider (csBulletSystem* sys)
  : scfImplementationType (this), collSystem (sys), 
    scale (1,1,1), shape (NULL), margin(0.0)
{
  //Lulu: Create a btEmptyShape for shape is safe but not efficient...
}

void csBulletCollider::SetLocalScale (const csVector3& scale)
{
  this->scale = scale;
  shape->setLocalScaling (btVector3(scale.x * collSystem->internalScale,
  scale.y * collSystem->internalScale,
  scale.z * collSystem->internalScale));
}

void csBulletCollider::SetMargin (float margin)
{
  if (margin > 0.0f)
  {
    this->margin = margin;
    shape->setMargin (margin * collSystem->internalScale);
  }
}

float csBulletCollider::GetMargin () const
{
  return margin;
}

csBulletColliderBox::csBulletColliderBox (const csVector3& boxSize)
  : scfImplementationType (this), boxSize (boxSize)
{
  shape = new btBoxShape (CSToBullet (boxSize*0.5f, collSystem->internalScale));
}

csBulletColliderBox::~csBulletColliderBox ()
{
  delete shape;
}

csBulletColliderSphere::csBulletColliderSphere (float radius)
  : scfImplementationType (this), radius (radius)
{
  shape = new btSphereShape (radius * collSystem->internalScale);
}

csBulletColliderSphere::~csBulletColliderSphere ()
{
  delete shape;
}

void csBulletColliderSphere::SetLocalScale (const csVector3& scale)
{
  if ((scale.x - scale.y) < EPSILON && (scale.y - scale.z) < EPSILON) //x == y == z
  {
    this->scale = scale;
    dynamic_cast<btSphereShape*>(shape)->setUnscaledRadius(scale.x * collSystem->internalScale);
  }
}

csBulletColliderCylinder::csBulletColliderCylinder (float length, float radius)
  : scfImplementationType (this), length (length), radius (radius)
{
  shape = new btCylinderShapeZ (btVector3 (radius * collSystem->internalScale,
    radius * collSystem->internalScale,
    length * collSystem->internalScale * 0.5f));
}

csBulletColliderCylinder::~csBulletColliderCylinder ()
{
  delete shape;
}

void csBulletColliderCylinder::GetCylinderGeometry (float& length, float& radius)
{
  length = this->length;
  radius = this->radius;
}

csBulletColliderCapsule::csBulletColliderCapsule (float length, float radius)
  : scfImplementationType (this), length (length), radius (radius)
{
  shape = new btCapsuleShapeZ (radius * collSystem->internalScale,
    length * collSystem->internalScale);
}

csBulletColliderCapsule::~csBulletColliderCapsule ()
{
  delete shape;
}

void csBulletColliderCapsule::GetCapsuleGeometry (float& length, float& radius)
{
  length = this->length;
  radius = this->radius;
}

csBulletColliderCone::csBulletColliderCone (float length, float radius)
  : scfImplementationType (this), length (length), radius (radius)
{
  shape = new btConeShapeZ (radius * collSystem->internalScale,
    length * collSystem->internalScale);
}

csBulletColliderCone::~csBulletColliderCone ()
{
  delete shape;
}

void csBulletColliderCone::GetConeGeometry (float& length, float& radius)
{
  length = this->length;
  radius = this->radius;
}

csBulletColliderPlane::csBulletColliderPlane (csBulletSector* sector,
                                           const csPlane3& plane)
  : scfImplementationType (this), plane (plane)
{
  csVector3 normal = plane.GetNormal ();
  shape = new btStaticPlaneShape (btVector3 (normal.x,normal.y,normal.z),
    plane.D () * collSystem->internalScale);
}

csBulletColliderPlane::~csBulletColliderPlane ()
{
  delete shape;
}

csBulletColliderConvexMesh::csBulletColliderConvexMesh (iMeshWrapper* mesh)
  : scfImplementationType (this), mesh (mesh)
{
  btTriangleMesh* triMesh = GenerateTriMeshData (mesh, collSystem->baseID, collSystem->colldetID, collSystem->internalScale);
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
}

csBulletColliderConvexMesh::~csBulletColliderConvexMesh ()
{
  delete shape;
}

float csBulletColliderConvexMesh::GetVolume ()
{
  csRef<iTriangleMesh> triMesh = FindColdetTriangleMesh (mesh, 
    collSystem->baseID, collSystem->colldetID);

  if (!triMesh)
    return 0.0f;

  size_t triangleCount = triMesh->GetTriangleCount ();
  size_t vertexCount = triMesh->GetVertexCount ();

  if (vertexCount == 0)
    return 0.0f;

  csTriangle *c_triangle = triMesh->GetTriangles ();
  csVector3 *c_vertex = triMesh->GetVertices ();

  float volume = 0.0f;
  btVector3 origin = c_vertex[c_triangle[0]];
  for (int i = 1; i < triangleCount; i++)
  {
    int index = i * 3;
    volume += fabsl (btDot
      (c_vertex[c_triangle[index]] - origin,
      btCross (c_vertex[c_triangle[index + 1]] - origin,
      c_vertex[c_triangle[index + 2]] - origin)));
  }

  return volume / 6.0f;
}

csBulletColliderConcaveMesh::csBulletColliderConcaveMesh (iMeshWrapper* mesh)
  : scfImplementationType (this), mesh (mesh)
{
  btTriangleMesh* triMesh = GenerateTriMeshData (mesh, collSystem->baseID, collSystem->colldetID, collSystem->internalScale);
  shape = new btBvhTriangleMeshShape (triMesh,true);
}

csBulletColliderConcaveMesh::~csBulletColliderConcaveMesh ()
{
  delete shape;
}

float csBulletColliderConcaveMesh::GetVolume ()
{
  // TODO: this is a really rough estimation
  btVector3 center;
  btScalar radius;
  shape->getBoundingSphere (center, radius);
  radius *= collSystem->inverseInternalScale;
  return 1.333333f * PI * radius * radius * radius;
}

csBulletColliderConcaveMeshScaled::csBulletColliderConcaveMeshScaled (iColliderConcaveMesh* collider, csVector3 scale)
  : scfImplementationType (this)
{
  this->scale = scale;
  originalCollider = dynamic_cast<csBulletColliderConcaveMesh*> (collider);
  if (originalCollider->shape->getShapeType () == TRIANGLE_MESH_SHAPE_PROXYTYPE)
  {
    btBvhTriangleMeshShape* meshShape = dynamic_cast<btBvhTriangleMeshShape*> (originalCollider->shape);
    shape = new btScaledBvhTriangleMeshShape (meshShape, 
      btVector3 (scale[0] * collSystem->internalScale,
      scale[1] * collSystem->internalScale,
      scale[2] * collSystem->internalScale));
  }
  else
    csFPrintf  (stderr, "csBulletColliderConcaveMeshScaled: Original collider is not concave mesh.\n");
}

csBulletColliderConcaveMeshScaled::~csBulletColliderConcaveMeshScaled ()
{
  delete shape;
}

HeightMapCollider::HeightMapCollider ( float* gridData, 
                                   int gridWidth, int gridHeight, 
                                   csVector3 gridSize, 
                                   float minHeight, float maxHeight,
                                   float internalScale)
                                   : btHeightfieldTerrainShape (gridWidth, gridHeight,
                                   gridData, 1.0f, minHeight, maxHeight,
                                   1, PHY_FLOAT, false), cell (cell), 
                                   heightData (gridData)
{
  localScale.setValue (gridSize[0] * internalScale / (gridWidth - 1),
    internalScale,
    gridSize[2] * internalScale/ (gridHeight - 1));
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

csBulletColliderTerrain::csBulletColliderTerrain (iTerrainSystem* terrain,
                                               float minimumHeight,
                                               float maximumHeight)
  : scfImplementationType (this), minimumHeight (minimumHeight), 
  maximumHeight (maximumHeight), terrainSystem(terrain)
{
  unload = true;
  terrain->AddCellLoadListener (this);
  // Find the transform of the terrain
  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (terrain);
  terrainTransform = mesh->GetMeshWrapper ()->GetMovable ()->GetFullTransform ();

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
  }
}

csBulletColliderTerrain::~csBulletColliderTerrain ()
{
  for (size_t i = 0; i < colliders.GetSize (); i++)
    delete colliders[i];
  for (size_t i = 0; i < bodies.GetSize (); i++)
    delete bodies[i];
}

void csBulletColliderTerrain::SetLocalScale (const csVector3& scale)
{
  this->scale = scale;
  for (size_t i = 0; i < colliders.GetSize (); i++)
    colliders[i]->SetLocalScale (scale * collSystem->internalScale);
}

void csBulletColliderTerrain::SetMargin (float margin)
{
  this->margin = margin;
  for (size_t i = 0; i < colliders.GetSize (); i++)
    colliders[i]->setMargin (margin * collSystem->internalScale);
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
  
  csOrthoTransform cellTransform (terrainTransform);
  csVector3 cellPosition  (cell->GetPosition ()[0], 0.0f, cell->GetPosition ()[1]);
  cellTransform.SetOrigin (terrainTransform.GetOrigin ()
    + terrainTransform.This2OtherRelative (cellPosition));

  HeightMapCollider* colliderData = new HeightMapCollider (
    heightData, gridWidth, gridHeight,
    cell->GetSize (), minHeight, maxHeight,
    collSystem->internalScale);

  colliders.Push (colliderData);

  csVector3 offset (cell->GetSize ()[0] * 0.5f,
    (maxHeight - minHeight) * 0.5f + minHeight,
    cell->GetSize ()[2] * 0.5f);

  cellTransform.SetOrigin (cellTransform.GetOrigin () + cellTransform.This2OtherRelative (offset));
  btTransform tr = CSToBullet (cellTransform, collSystem->internalScale);

  btRigidBody* body = new btRigidBody (0, 0, colliderData, btVector3 (0, 0, 0));
  body->setWorldTransform (tr);

  //collSystem->bulletWorld->addRigidBody (body);
  //wait until add object to sector...
  bodies.Push (body);
}

const csVector3& csBulletColliderTerrain::GetCellPosition (size_t index)
{
  return colliders[index]->cellPosition;
}

void csBulletColliderTerrain::RemoveRigidBodies ()
{
  for (size_t i = 0; i < bodies.GetSize (); i++)
  {
    collSector->bulletWorld->removeRigidBody (bodies[i]);
    /*delete bodies[i];
    bodies.Empty ();*/
  }
}

void csBulletColliderTerrain::AddRigidBodies (csBulletSector* sector)
{
  collSector = sector;
  for (size_t i = 0; i < bodies.GetSize (); i++)
    sector->bulletWorld->addRigidBody (bodies[i]);
}
}
CS_PLUGIN_NAMESPACE_END (Bullet2)
