/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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

#include "cssysdef.h"

#include "csgeom/sphere.h"
#include "igeom/trimesh.h"
#include "imesh/objmodel.h"
#include "imesh/terrain2.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "colliders.h"
#include "rigidbodies.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

//----------------------- Triangle mesh tools ----------------------------

static csRef<iTriangleMesh> FindColdetTriangleMesh(iMeshWrapper* mesh,
				csStringID base_id, csStringID colldet_id)
{
  iObjectModel* objmodel = mesh->GetMeshObject ()->GetObjectModel ();
  csRef<iTriangleMesh> trimesh;
  bool use_trimesh = objmodel->IsTriangleDataSet (base_id);
  if (use_trimesh)
  {
    if (objmodel->GetTriangleData(colldet_id))
      trimesh = objmodel->GetTriangleData (colldet_id);
    else
      trimesh = objmodel->GetTriangleData (base_id);
  }

  if (!trimesh || trimesh->GetVertexCount () == 0
      || trimesh->GetTriangleCount () == 0)
  {
    csFPrintf (stderr, "csBulletRigidBody: No collision polygons, triangles or vertices on mesh factory '%s'\n",
      mesh->QueryObject()->GetName());

    return 0;
  }
  return trimesh;
}

#include "csutil/custom_new_disable.h"

static btTriangleIndexVertexArray* GenerateTriMeshData
  (iMeshWrapper* mesh, int*& indices, size_t& triangleCount, btVector3*& vertices,
   size_t& vertexCount, csStringID base_id, csStringID colldet_id,
   float internalScale)
{
  csRef<iTriangleMesh> trimesh = FindColdetTriangleMesh(mesh, base_id, colldet_id);
  if (!trimesh)
    return 0;

  // TODO: remove double vertices
  csTriangle *c_triangle = trimesh->GetTriangles();
  triangleCount = trimesh->GetTriangleCount();
  vertexCount = trimesh->GetVertexCount ();

  delete[] indices;
  indices = new int[triangleCount * 3];
  int indexStride = 3 * sizeof (int);

  size_t i;
  int* id = indices;
  for (i = 0 ; i < triangleCount ; i++)
  {
    *id++ = c_triangle[i].a;
    *id++ = c_triangle[i].b;
    *id++ = c_triangle[i].c;
  }

  delete[] vertices;
  vertices = new btVector3[vertexCount];
  csVector3 *c_vertex = trimesh->GetVertices();
  int vertexStride = sizeof (btVector3);

  for (i = 0 ; i < vertexCount ; i++)
    vertices[i].setValue (c_vertex[i].x * internalScale,
			  c_vertex[i].y * internalScale,
			  c_vertex[i].z * internalScale);

  btTriangleIndexVertexArray* indexVertexArrays =
    new btTriangleIndexVertexArray ((int)triangleCount, indices, (int)indexStride,
	(int)vertexCount, (btScalar*) &vertices[0].x (), vertexStride);
  return indexVertexArrays;
}

#include "csutil/custom_new_enable.h"

//--------------------- csBulletCollider -----------------------------------

csBulletCollider::csBulletCollider (csBulletDynamicsSystem* dynSys,
				    csBulletRigidBody* body, bool isStaticBody)
  :  scfImplementationType (this), dynSys (dynSys), body (body),
     isStaticBody (isStaticBody), geomType (NO_GEOMETRY), shape (0),
     density (0.1f), friction (0.5f), softness (0.0f), elasticity (0.2f),
     vertices (0), indices (0)
{
}

csBulletCollider::~csBulletCollider ()
{
  delete shape;
  delete[] vertices;
  delete[] indices;
}

bool csBulletCollider::CreateSphereGeometry (const csSphere& sphere)
{
  // TODO: the body won't be set if one create a body, then AttachCollider,
  //       then CreateGeometry on the collider

  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  shape = new btSphereShape (sphere.GetRadius () * dynSys->internalScale);
  geomType = SPHERE_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    // TODO: add local transform?
    localTransform.Identity ();
    if (!(sphere.GetCenter () < 0.0001f))
      localTransform.SetOrigin (sphere.GetCenter ());

    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreatePlaneGeometry (const csPlane3& plane)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  // Bullet doesn't support dynamic plane shapes
  if (!isStaticBody && body->dynamicState != CS_BULLET_STATE_STATIC)
    return false;
  
  csVector3 normal = plane.GetNormal ();
  shape = new btStaticPlaneShape (btVector3 (normal.x, normal.y, normal.z),
				  plane.D () * dynSys->internalScale);                                       
  geomType = PLANE_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreateConvexMeshGeometry (iMeshWrapper* mesh)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  btTriangleIndexVertexArray* indexVertexArrays =
    GenerateTriMeshData (mesh, indices, triangleCount, vertices, vertexCount,
			 dynSys->baseId, dynSys->colldetId, dynSys->internalScale);
  if (!indexVertexArrays)
    return false;

  //shape = new btConvexTriangleMeshShape (indexVertexArrays);
  btConvexHullShape* convexShape = new btConvexHullShape ();
  for (size_t i = 0; i < vertexCount; i++)
    convexShape->addPoint(*(vertices + i));
  shape = convexShape;

  geomType = CONVEXMESH_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreateMeshGeometry (iMeshWrapper* mesh)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  btTriangleIndexVertexArray* indexVertexArrays =
    GenerateTriMeshData (mesh, indices, triangleCount, vertices, vertexCount,
			 dynSys->baseId, dynSys->colldetId, dynSys->internalScale);
  if (!indexVertexArrays)
    return false;

  // this shape is optimized for static concave meshes
  if (isStaticBody || body->dynamicState == CS_BULLET_STATE_STATIC)
  {
    btBvhTriangleMeshShape* concaveShape =
      new btBvhTriangleMeshShape (indexVertexArrays, true);
    // it seems to work without that line, to be tested if problems occur
    //concaveShape->refitTree (btVector3 (-1000, -1000, -1000),
    //                         btVector3 (1000, 1000, 1000));
    shape = concaveShape;
  }

  // this one is for dynamic meshes
  else
  {
    dynSys->RegisterGimpact ();
    btGImpactMeshShape* concaveShape = new btGImpactMeshShape (indexVertexArrays);
    concaveShape->updateBound();
    shape = concaveShape;
  }
  geomType = TRIMESH_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

void csBulletCollider::RebuildMeshGeometry ()
{
  if (geomType != TRIMESH_COLLIDER_GEOMETRY
      || !triangleCount)
    return;

  btTriangleIndexVertexArray* indexVertexArrays =
    new btTriangleIndexVertexArray ((int)triangleCount, indices, 3 * sizeof (int),
	      (int)vertexCount, (btScalar*) &vertices[0].x (), sizeof (btVector3));

  // this shape is optimized for static concave meshes
  if (isStaticBody || body->dynamicState == CS_BULLET_STATE_STATIC)
  {
    btBvhTriangleMeshShape* concaveShape =
      new btBvhTriangleMeshShape (indexVertexArrays, true);
    // it seems to work without that line, to be tested if problems occur
    //concaveShape->refitTree (btVector3 (-1000, -1000, -1000),
    //                         btVector3 (1000, 1000, 1000));
    shape = concaveShape;
  }

  // this one is for dynamic meshes
  else
  {
    dynSys->RegisterGimpact ();
    btGImpactMeshShape* concaveShape = new btGImpactMeshShape (indexVertexArrays);
    concaveShape->updateBound();
    shape = concaveShape;
  }
}

bool csBulletCollider::CreateBoxGeometry (const csVector3& size)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  shape = new btBoxShape (CSToBullet (size * 0.5f, dynSys->internalScale));
  geomType = BOX_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreateCylinderGeometry (float length,
  float radius)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  shape = new btCylinderShapeZ (btVector3 (radius * dynSys->internalScale,
					   radius * dynSys->internalScale,
					   length * dynSys->internalScale * 0.5f));
  geomType = CYLINDER_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

bool csBulletCollider::CreateCapsuleGeometry (float length,
  float radius)
{
  delete shape; shape = 0;
  delete[] vertices; vertices = 0;
  delete[] indices; indices = 0;

  shape = new btCapsuleShapeZ (radius * dynSys->internalScale,
			       length * dynSys->internalScale);
  geomType = CAPSULE_COLLIDER_GEOMETRY;

  if (isStaticBody)
  {
    body->compoundChanged = true;
    body->RebuildBody ();
  }

  return true;
}

void csBulletCollider::SetCollisionCallback (
  iDynamicsColliderCollisionCallback* cb)
{
  collCb = cb;
}

void csBulletCollider::SetFriction (float friction)
{
  // @@@ TODO: check value range
  this->friction = friction;
  // @@@ TODO: update body
}

void csBulletCollider::SetSoftness (float softness)
{
  // @@@ TODO: check value range
  this->softness = softness;
  // @@@ TODO: update body
}

void csBulletCollider::SetDensity (float density)
{
  // @@@ TODO: check value range
  this->density = density * dynSys->inverseInternalScale
    * dynSys->inverseInternalScale * dynSys->inverseInternalScale;
  // @@@ TODO: update body
}

void csBulletCollider::SetElasticity (float elasticity)
{
  // @@@ TODO: check value range
  this->elasticity = elasticity;
  // @@@ TODO: update body
}

float csBulletCollider::GetFriction ()
{
  return friction;
}

float csBulletCollider::GetSoftness ()
{
  return softness;
}

float csBulletCollider::GetDensity ()
{
  return density * dynSys->internalScale * dynSys->internalScale * dynSys->internalScale;
}

float csBulletCollider::GetElasticity ()
{
  return elasticity;
}

void csBulletCollider::FillWithColliderGeometry (
    csRef<iGeneralFactoryState> genmesh_fact)
{
  // @@@ TODO
#if 0
  switch (geomType)
  {
    case BOX_COLLIDER_GEOMETRY:
    {
      SimdTransform trans;
      SimdVector3 max;
      SimdVector3 min;
      BoxShape*  b = (BoxShape*)pc->GetRigidBody ()->GetCollisionShape ();

      csBox3 box;
      for (int i = 0; i < b->GetNumVertices (); i++)
      {
        SimdVector3 vtx;
        b->GetVertex (i, vtx); 
        box.AddBoundingVertexTest (csVector3 (vtx[0], vtx[1], vtx[2]));
      }
      genmesh_fact->GenerateBox (box);
      genmesh_fact->CalculateNormals (); 
    }
    break;
    default:
    break;
  }
#endif
}

csOrthoTransform csBulletCollider::GetTransform ()
{
  csOrthoTransform trans = body->GetTransform ();
  return localTransform * trans;
}

csOrthoTransform csBulletCollider::GetLocalTransform ()
{
  if (isStaticBody || body->dynamicState == CS_BULLET_STATE_STATIC)
    return localTransform * body->GetTransform ();

  return localTransform;
}

void csBulletCollider::SetTransform (const csOrthoTransform& trans)
{
  if (isStaticBody)
    body->SetTransform (localTransform.GetInverse () * trans);

  else
  {
    localTransform = trans;
    body->compoundChanged = true;
    body->RebuildBody ();
  }
}

bool csBulletCollider::GetBoxGeometry (csVector3& size)
{
  if (geomType != BOX_COLLIDER_GEOMETRY)
    return false;

  btBoxShape* geometry = static_cast<btBoxShape*> (shape);
  btVector3 btSize = geometry->getHalfExtentsWithMargin ();
  size.Set (BulletToCS (btSize, dynSys->inverseInternalScale));
  size *= 2.0f;

  return true;
}

bool csBulletCollider::GetSphereGeometry (csSphere& sphere)
{
  if (geomType != SPHERE_COLLIDER_GEOMETRY)
    return false;

  btSphereShape* geometry = static_cast<btSphereShape*> (shape);
  sphere.SetCenter (localTransform.GetOrigin ());
  sphere.SetRadius (geometry->getRadius () * dynSys->inverseInternalScale);

  return true;
}

bool csBulletCollider::GetPlaneGeometry (csPlane3& plane)
{
  // TODO
  return false;
}

bool csBulletCollider::GetCylinderGeometry (float& length,
  float& radius)
{
  if (geomType != CYLINDER_COLLIDER_GEOMETRY)
    return false;

  btCylinderShapeZ* geometry = static_cast<btCylinderShapeZ*> (shape);
  btVector3 btSize = geometry->getHalfExtentsWithMargin ();
  radius = btSize.getX () * dynSys->inverseInternalScale;
  length = btSize.getZ () * 2.0f * dynSys->inverseInternalScale;

  return true;
}

bool csBulletCollider::GetCapsuleGeometry (float& length,
  float& radius)
{
  if (geomType != CAPSULE_COLLIDER_GEOMETRY)
    return false;

  btCapsuleShapeZ* geometry = static_cast<btCapsuleShapeZ*> (shape);
  radius = geometry->getRadius () * dynSys->inverseInternalScale;
  length = geometry->getHalfHeight () * 2.0f * dynSys->inverseInternalScale;

  return true;
}

bool csBulletCollider::GetMeshGeometry (csVector3*& vertices, size_t& vertexCount,
					int*& indices, size_t& triangleCount)
{
  if (geomType != TRIMESH_COLLIDER_GEOMETRY)
    return false;

  triangleCount = this->triangleCount;
  delete[] indices;
  indices = new int[this->triangleCount * 3];
  for (unsigned int i = 0; i < triangleCount * 3; i++)
    indices[i] = this->indices[i];

  vertexCount = this->vertexCount;
  delete[] vertices;
  vertices = new csVector3[this->vertexCount];
  for (unsigned int i = 0; i < vertexCount; i++)
    vertices[i].Set (BulletToCS (this->vertices[i], dynSys->inverseInternalScale));

  return true;
}

bool csBulletCollider::GetConvexMeshGeometry (csVector3*& vertices, size_t& vertexCount,
					      int*& indices, size_t& triangleCount)
{
  if (geomType != CONVEXMESH_COLLIDER_GEOMETRY)
    return false;

  triangleCount = this->triangleCount;
  delete[] indices;
  indices = new int[this->triangleCount * 3];
  for (unsigned int i = 0; i < triangleCount * 3; i++)
    indices[i] = this->indices[i];

  vertexCount = this->vertexCount;
  delete[] vertices;
  vertices = new csVector3[this->vertexCount];
  for (unsigned int i = 0; i < vertexCount; i++)
    vertices[i].Set (BulletToCS (this->vertices[i], dynSys->inverseInternalScale));

  return true;
}

void csBulletCollider::MakeStatic ()
{
  // nonsense?
}

void csBulletCollider::MakeDynamic ()
{
  // nonsense?
}

bool csBulletCollider::IsStatic ()
{
  return isStaticBody || body->dynamicState == CS_BULLET_STATE_STATIC;
}

float csBulletCollider::GetVolume ()
{
  switch (geomType)
  {
    case BOX_COLLIDER_GEOMETRY:
      {
	csVector3 size;
	GetBoxGeometry (size);
	return size[0] * size[1] * size[2];
      }

    case SPHERE_COLLIDER_GEOMETRY:
      {
	csSphere sphere;
	GetSphereGeometry (sphere);
	return 1.333333f * PI * sphere.GetRadius () * sphere.GetRadius ()
	  * sphere.GetRadius ();
      }

    case CYLINDER_COLLIDER_GEOMETRY:
      {
	float length;
	float radius;
	GetCylinderGeometry (length, radius);
	return PI * radius * radius * length;
      }

    case CAPSULE_COLLIDER_GEOMETRY:
      {
	float length;
	float radius;
	GetCapsuleGeometry (length, radius);
	return PI * radius * radius * length
	  + 1.333333f * PI * radius * radius * radius;
      }

    case CONVEXMESH_COLLIDER_GEOMETRY:
      {
	if (vertexCount == 0)
	  return 0.0f;

	float volume = 0.0f;
	int faceCount = (int)vertexCount / 3;
	btVector3 origin = vertices[indices[0]];
	for (int i = 1; i < faceCount; i++)
	{
	  int index = i * 3;
	  volume += fabsl (btDot
			   (vertices[indices[index]] - origin,
			    btCross (vertices[indices[index + 1]] - origin,
				     vertices[indices[index + 2]] - origin)));
	}

	return volume / 6.0f;
      }

    case TRIMESH_COLLIDER_GEOMETRY:
      {
	if (vertexCount == 0)
	  return 0.0f;

	// TODO: this is a really rough estimation
	btVector3 center;
	btScalar radius;
	shape->getBoundingSphere (center, radius);
	return 1.333333f * PI * radius * radius * radius;
      }

  default:
    return 0.0f;
  }

  return 0.0f;
}

//----------------------- HeightMapCollider ----------------------------

HeightMapCollider::HeightMapCollider (csBulletDynamicsSystem* dynSys,
				      csLockedHeightData gridData,
				      int gridWidth, int gridHeight,
				      csVector3 gridSize,
				      csOrthoTransform transform,
				      float minimumHeight, float maximumHeight)
  : dynSys (dynSys)
{
  // Check if the min/max have to be computed
  bool needExtremum = minimumHeight == 0.0f && maximumHeight == 0.0f;
  if (needExtremum)
    minimumHeight = maximumHeight = gridData.data[0];

  // Initialize the terrain height data
  heightData = new float[gridHeight * gridWidth];
  for (int i = 0; i < gridWidth; i++)
    for (int j = 0; j < gridHeight; j++)
    {
      float height = heightData[(gridWidth - i - 1) * gridWidth + j]
	= gridData.data[i * gridWidth + j];

      if (needExtremum)
	{
	  minimumHeight = MIN (minimumHeight, height);
	  maximumHeight = MAX (maximumHeight, height);
	}
    }

  // Create the terrain shape
  shape = new btHeightfieldTerrainShape (gridWidth, gridHeight,
					 heightData, 1.0f, minimumHeight, maximumHeight,
					 1, PHY_FLOAT, false);

  // Apply the local scaling on the shape
  btVector3 localScale (gridSize[0] * dynSys->internalScale / (gridWidth - 1),
			dynSys->internalScale,
			gridSize[2] * dynSys->internalScale / (gridHeight - 1));
  shape->setLocalScaling (localScale);

  // Set the origin to the middle of the heightfield 
  csVector3 offset (gridSize[0] * 0.5f,
		    (maximumHeight - minimumHeight) * 0.5f + minimumHeight,
		    gridSize[2] * 0.5f);
  transform.SetOrigin (transform.GetOrigin () + transform.This2OtherRelative (offset));
  btTransform tr = CSToBullet (transform, dynSys->internalScale);

  // Create the rigid body and add it to the world
  body = new btRigidBody (0, 0, shape, btVector3 (0, 0, 0));	
  body->setWorldTransform (tr);
  dynSys->bulletWorld->addRigidBody (body);
}

HeightMapCollider::~HeightMapCollider ()
{
  dynSys->bulletWorld->removeRigidBody (body);

  delete body;
  delete shape;
  delete heightData;
}

//----------------------- csBulletTerrainCollider ----------------------------

csBulletTerrainCollider::csBulletTerrainCollider (csBulletDynamicsSystem* dynSys,
						  csLockedHeightData& heightData,
						  int gridWidth, int gridHeight,
						  csVector3 gridSize,
						  csOrthoTransform& transform,
						  float minimumHeight, float maximumHeight)
  :  scfImplementationType (this)
{
  // Create the terrain collider
  colliders.Push (new HeightMapCollider
		  (dynSys, heightData, gridWidth, gridHeight, gridSize,
		   transform, minimumHeight, maximumHeight));
}

csBulletTerrainCollider::csBulletTerrainCollider (csBulletDynamicsSystem* dynSys,
						  iTerrainCell* cell,
						  float minimumHeight, float maximumHeight)
  :  scfImplementationType (this)
{
  // Make sure that the cell data has been loaded
  cell->SetLoadState (iTerrainCell::Loaded);

  // Find the transform of the terrain
  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (cell->GetTerrain ());
  csOrthoTransform terrainTransform = mesh->GetMeshWrapper ()->GetMovable ()->GetTransform ();

  // Compute the transform of the cell
  csOrthoTransform cellTransform (terrainTransform);
  csVector3 position (cell->GetPosition ()[0], 0.0f, cell->GetPosition ()[1]);
  cellTransform.SetOrigin (terrainTransform.GetOrigin ()
			   + terrainTransform.This2OtherRelative (position));

  // Create the terrain collider
  colliders.Push
    (new HeightMapCollider (dynSys, cell->GetHeightData (), cell->GetGridWidth (),
			    cell->GetGridHeight (), cell->GetSize (),
			    cellTransform, minimumHeight, maximumHeight));
}

csBulletTerrainCollider::csBulletTerrainCollider (csBulletDynamicsSystem* dynSys,
						  iTerrainSystem* terrain,
						  float minimumHeight, float maximumHeight)
  :  scfImplementationType (this)
{
  // Find the transform of the terrain
  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (terrain);
  csOrthoTransform terrainTransform = mesh->GetMeshWrapper ()->GetMovable ()->GetTransform ();

  // Create a terrain collider for each cell of the terrain
  for (size_t i = 0; i < terrain->GetCellCount (); i++)
  {
    iTerrainCell* cell = terrain->GetCell (i);

    // TODO: add only cells already loaded + listen to load/modify events
    // Make sure that the cell data has been loaded
    cell->SetLoadState (iTerrainCell::Loaded);

    // Compute the transform of the cell
    csOrthoTransform cellTransform (terrainTransform);
    csVector3 position (cell->GetPosition ()[0], 0.0f, cell->GetPosition ()[1]);
    cellTransform.SetOrigin (terrainTransform.GetOrigin ()
			     + terrainTransform.This2OtherRelative (position));

    // Create the terrain collider
    colliders.Push
      (new HeightMapCollider (dynSys, cell->GetHeightData (), cell->GetGridWidth (),
			      cell->GetGridHeight (), cell->GetSize (),
			      cellTransform, minimumHeight, maximumHeight));
  }
}

csBulletTerrainCollider::~csBulletTerrainCollider ()
{
  for (size_t i = 0; i < colliders.GetSize (); i++)
    delete (colliders[i]);
}

}
CS_PLUGIN_NAMESPACE_END(Bullet)
