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

#include "csutil/scf.h"
#include "ivaria/reporter.h"
#include "ivaria/dynamics.h"
#include "csgeom/vector3.h"
#include "csgeom/sphere.h"
#include "csgeom/tri.h"
#include "iengine/engine.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "cstool/materialbuilder.h"
#include "dyndebug.h"

CS_PLUGIN_NAMESPACE_BEGIN(DebugDynamics)
{

  //------------------------ DebuggerManager ----------------------

  SCF_IMPLEMENT_FACTORY(DebuggerManager);

  CS_LEAKGUARD_IMPLEMENT(DebuggerManager);

  DebuggerManager::DebuggerManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  iDynamicSystemDebugger* DebuggerManager::CreateDebugger ()
  {
    csRef<iDynamicSystemDebugger> ref;
    ref.AttachNew (new DynamicsDebugger (this));
    debuggers.Push (ref);
    return ref;
  }

  bool DebuggerManager::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void DebuggerManager::Report (int severity, const char* msg, ...)
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity, "crystalspace.dynamics.debug",
		    msg, arg);
    else
      {
	csPrintfV (msg, arg);
	csPrintf ("\n");
      }
    va_end (arg);
  }

  //------------------------ DynamicsDebugger ----------------------

  CS_LEAKGUARD_IMPLEMENT(DynamicsDebugger);

  DynamicsDebugger::DynamicsDebugger (DebuggerManager* manager)
    : scfImplementationType (this), manager (manager), debugMode (false)
  {
    // Init debug materials
    materials[0] = CS::Material::MaterialBuilder::CreateColorMaterial
      (manager->object_reg, "dyndebug_static", csColor (0, 0, 1));
    materials[1] = CS::Material::MaterialBuilder::CreateColorMaterial
      (manager->object_reg, "dyndebug_dynamic", csColor (0, 1, 0));
    materials[2] = CS::Material::MaterialBuilder::CreateColorMaterial
      (manager->object_reg, "dyndebug_kinematic", csColor (0, 0, 1));
  }

  void DynamicsDebugger::SetDynamicSystem (iDynamicSystem* system)
  {
    this->system = system;
  }

  void DynamicsDebugger::SetDebugSector (iSector* sector)
  {
    this->sector = sector;
  }

  void DynamicsDebugger::SetDebugDisplayMode (bool debugMode)
  {
    // Check display mode has changed
    if (debugMode == this->debugMode)
      return;

    this->debugMode = debugMode;

    UpdateDisplay ();
  }

  void DynamicsDebugger::UpdateDisplay ()
  {
    // Check dynsys available
    if (!system)
    {
      manager->Report (CS_REPORTER_SEVERITY_WARNING,
		       "No dynamic system defined");
      return;
    }

    // Find the pointer to the engine plugin
    csRef<iEngine> engine = csQueryRegistry<iEngine> (manager->object_reg);
    if (engine == 0)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "No iEngine plugin!");
      return;
    }

    // Reset all stored meshes
    for (csArray<MeshData>::Iterator it = storedMeshes.GetIterator (); it.HasNext (); )
    {
      MeshData &meshData = it.Next ();

      // Remove the debug mesh
      if (meshData.debugMesh)
      {
	engine->RemoveObject (meshData.debugMesh);
	if (meshData.rigidBody)
	  meshData.rigidBody->AttachMesh (0);
      }

      // Put back the original mesh
      if (meshData.originalMesh)
      {
	meshData.originalMesh->GetMovable ()->SetSector (sector);
	meshData.rigidBody->AttachMesh (meshData.originalMesh);
      }

      // Put back the kinematic callback
      if (meshData.callback && meshData.rigidBody)
      {
	csRef<iBulletRigidBody> bulletBody =
	  scfQueryInterface<iBulletRigidBody> (meshData.rigidBody);
	bulletBody->SetKinematicCallback (meshData.callback->callback);
      }
    }
    storedMeshes.DeleteAll ();

    // If not in debug mode then it is over
    if (!debugMode)
      return;

    // Iterate through each rigid body
    for (unsigned int bodyIndex = 0;
	 bodyIndex < (unsigned int) system->GetBodysCount ();
	 bodyIndex++)
    {
      iRigidBody* body = system->GetBody (bodyIndex);

      // Search the bullet interface of the rigid body
      csRef<iBulletRigidBody> bulletBody =
	scfQueryInterface<iBulletRigidBody> (body);

      // Find the material to be used for this object
      csBulletState state = BULLET_STATE_DYNAMIC;
      if (bulletBody)
	state = bulletBody->GetDynamicState ();
      else if (body->IsStatic ())
	state = BULLET_STATE_STATIC;

      iMaterialWrapper* material = materials[state];
      if (!material)
	continue;

      // Create the MeshData object
      MeshData meshData;
      meshData.rigidBody = body;

      // Store the current mesh attached to the rigid body
      // TODO: not debug mesh stored twice?
      meshData.originalMesh = body->GetAttachedMesh ();
      if (meshData.originalMesh)
      {
	// TODO: store the sector of the mesh
	engine->RemoveObject (meshData.originalMesh);
      }

      // TODO: display the joints too
      // TODO: use iDynamicsSystemCollider::FillWithColliderGeometry instead?
      // TODO: use specific colors for objects active/inactive
      // TODO: display collisions

      // Iterate through each collider
      for (unsigned int colliderIndex = 0;
	   colliderIndex < (unsigned int) body->GetColliderCount ();
	   colliderIndex++)
      {
	// TODO: it won't work if there is more than one collider for a body
	csRef<iDynamicsSystemCollider> collider =
	  body->GetCollider (colliderIndex);

	csRef<iMeshWrapper> mesh = CreateColliderMesh (collider, material);

	// Register the new debug mesh
	if (mesh)
	{
	  body->AttachMesh (mesh);
	  meshData.debugMesh = mesh;
	}

	// If the body is kinematic then create a new kinematic callback
	if (mesh && state == BULLET_STATE_KINEMATIC)
	{
	  meshData.callback.AttachNew
	    (new BoneKinematicCallback (mesh, bulletBody->GetKinematicCallback ()));
	  bulletBody->SetKinematicCallback (meshData.callback);
	}
      }

      // Store the MeshData
      storedMeshes.Push (meshData);
    }

    // Iterate through each colliders of the dynamic system
    for (unsigned int colliderIndex = 0;
	 colliderIndex < (unsigned int) system->GetColliderCount ();
	 colliderIndex++)
    {
      iDynamicsSystemCollider* collider = system->GetCollider (colliderIndex);

      // Find the material to be used for this object
      csBulletState state = BULLET_STATE_DYNAMIC;
      if (collider->IsStatic ())
	state = BULLET_STATE_STATIC;

      iMaterialWrapper* material = materials[state];
      if (!material)
	continue;

      // Create the MeshData object
      MeshData meshData;
      meshData.debugMesh = CreateColliderMesh (collider, material);

      // Store the MeshData
      storedMeshes.Push (meshData);
    }
  }

  void DynamicsDebugger::SetStaticBodyMaterial (iMaterialWrapper* material)
  {
    materials[0] = material;
  }

  void DynamicsDebugger::SetDynamicBodyMaterial (iMaterialWrapper* material)
  {
    materials[1] = material;
  }

  void DynamicsDebugger::SetBodyStateMaterial (csBulletState state,
					       iMaterialWrapper* material)
  {
    materials[state] = material;
  }

  csRef<iMeshWrapper> DynamicsDebugger::CreateColliderMesh
    (iDynamicsSystemCollider* collider, iMaterialWrapper* material)
  {
    csRef<iMeshWrapper> mesh;

    switch (collider->GetGeometryType ())
      {
      case BOX_COLLIDER_GEOMETRY:
	{
	  // Get box geometry
	  csVector3 boxSize;
	  collider->GetBoxGeometry (boxSize);
	  boxSize /= 2.0;
	  const csBox3 box(-boxSize, boxSize);

	  // Create box
	  mesh = CreateBoxMesh (box, material, collider->GetLocalTransform (),
				sector);
	}
	break;

      case SPHERE_COLLIDER_GEOMETRY:
	{
	  // Get sphere geometry
	  csSphere sphere;
	  collider->GetSphereGeometry (sphere);

	  // Create sphere
	  mesh = CreateSphereMesh (sphere, material, sector);
	}
	break;

      case CYLINDER_COLLIDER_GEOMETRY:
	{
	  // Get cylinder geometry
	  float length, radius;
	  collider->GetCylinderGeometry (length, radius);

	  // Create cylinder
	  mesh = CreateCylinderMesh (length, radius, material,
				     collider->GetLocalTransform (), sector);
	}
	break;

      case CAPSULE_COLLIDER_GEOMETRY:
	{
	  // Get capsule geometry
	  float length, radius;
	  collider->GetCapsuleGeometry (length, radius);

	  // Create capsule
	  mesh = CreateCapsuleMesh (length, radius, material,
				    collider->GetLocalTransform (), sector);
	}
	break;

      case TRIMESH_COLLIDER_GEOMETRY:
	{
	  // Get mesh geometry
	  csVector3* vertices = 0;
	  int* indices = 0;
	  size_t vertexCount, triangleCount;
	  collider->GetMeshGeometry (vertices, vertexCount, indices, triangleCount);

	  // Create mesh
	  mesh = CreateCustomMesh (vertices, vertexCount, indices, triangleCount,
				   material, collider->GetLocalTransform (), sector);
	}
	break;

      case CONVEXMESH_COLLIDER_GEOMETRY:
	{
	  // Get mesh geometry
	  csVector3* vertices = 0;
	  int* indices = 0;
	  size_t vertexCount, triangleCount;
	  collider->GetConvexMeshGeometry (vertices, vertexCount, indices, triangleCount);

	  // Create mesh
	  mesh = CreateCustomMesh (vertices, vertexCount, indices, triangleCount, material,
				   collider->GetLocalTransform (), sector);
	}
	break;

      default:
	// TODO: plan meshes
	break;
      }

    return mesh;
  }

  csRef<iMeshWrapper> DynamicsDebugger::CreateBoxMesh (csBox3 box,
				iMaterialWrapper* material, csOrthoTransform transform,
				iSector* sector)
  {
    // Find the pointer to the engine plugin
    csRef<iEngine> engine = csQueryRegistry<iEngine> (manager->object_reg);
    if (!engine)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "No iEngine plugin!");
      return 0;
    }

    // Create the box mesh factory.
    csRef<iMeshFactoryWrapper> boxFact =
      engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh",
				 "boxFact");
    if (!boxFact)
    {
      manager->Report (CS_REPORTER_SEVERITY_WARNING,
		       "Error creating box mesh factory");
      return 0;
    }

    // Generate the box topology
    csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
      (boxFact->GetMeshObjectFactory ());
    gmstate->GenerateBox (box);

    boxFact->HardTransform (transform);

    // Create the mesh.
    csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (boxFact, "box", sector));
    mesh->GetMeshObject ()->SetMaterialWrapper (material);

    return mesh;
  }

  csRef<iMeshWrapper> DynamicsDebugger::CreateSphereMesh (csSphere sphere,
                                iMaterialWrapper* material, iSector* sector)
  {
    // Find the pointer to the engine plugin
    csRef<iEngine> engine = csQueryRegistry<iEngine> (manager->object_reg);
    if (!engine)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "No iEngine plugin!");
      return 0;
    }

    // Create the sphere mesh factory.
    csRef<iMeshFactoryWrapper> sphereFact =
      engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh",
				 "sphereFact");
    if (sphereFact == 0)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Error creating sphere mesh factory!");
      return 0;
    }
 
    // Generate the sphere topology
    csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
      (sphereFact->GetMeshObjectFactory ());
    csVector3 radius (sphere.GetRadius ());
    csEllipsoid ellips (sphere.GetCenter (), radius);
    gmstate->GenerateSphere (ellips, 16);

    // Create the mesh.
    csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (sphereFact, "sphere", sector));
    mesh->GetMeshObject ()->SetMaterialWrapper (material);

    return mesh;
  }

  csRef<iMeshWrapper> DynamicsDebugger::CreateCylinderMesh (float length, float radius,
				iMaterialWrapper* material, csOrthoTransform transform,
				iSector* sector)
  {
    // Find the pointer to the engine plugin
    csRef<iEngine> engine = csQueryRegistry<iEngine> (manager->object_reg);
    if (!engine)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "No iEngine plugin!");
      return 0;
    }

    // Create the cylinder mesh factory.
    csRef<iMeshFactoryWrapper> cylinderFact =
      engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh",
				 "cylinderFact");
    if (cylinderFact == 0)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Error creating cylinder mesh factory!");
      return 0;
    }
 
    // Generate the cylinder topology
    csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
      (cylinderFact->GetMeshObjectFactory ());
    gmstate->GenerateCylinder (length, radius, 10);

    csReversibleTransform centerTransform (csYRotMatrix3 (PI/2), csVector3 (0));
    cylinderFact->HardTransform (centerTransform * transform);

    // Create the mesh.
    csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (cylinderFact, "cylinder",
						       sector));
    mesh->GetMeshObject ()->SetMaterialWrapper (material);

    return mesh;
  }

  csRef<iMeshWrapper> DynamicsDebugger::CreateCapsuleMesh (float length, float radius,
				iMaterialWrapper* material, csOrthoTransform transform,
				iSector* sector)
  {
    // Find the pointer to the engine plugin
    csRef<iEngine> engine = csQueryRegistry<iEngine> (manager->object_reg);
    if (!engine)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "No iEngine plugin!");
      return 0;
    }

    // Create the capsule mesh factory.
    csRef<iMeshFactoryWrapper> capsuleFact =
      engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh",
				 "capsuleFact");
    if (capsuleFact == 0)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Error creating capsule mesh factory!");
      return 0;
    }
 
    // Generate the capsule topology
    csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
      (capsuleFact->GetMeshObjectFactory ());
    gmstate->GenerateCapsule (length, radius, 10);

    csReversibleTransform centerTransform (csYRotMatrix3 (PI/2), csVector3 (0));
    capsuleFact->HardTransform (centerTransform * transform);

    // Create the mesh.
    csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (capsuleFact, "capsule",
						       sector));
    mesh->GetMeshObject ()->SetMaterialWrapper (material);

    return mesh;
  }

  csRef<iMeshWrapper> DynamicsDebugger::CreateCustomMesh
    (csVector3*& vertices, size_t& vertexCount, int*& indices, size_t& triangleCount,
     iMaterialWrapper* material, csOrthoTransform transform, iSector* sector)
  {
    // Find the pointer to the engine plugin
    csRef<iEngine> engine = csQueryRegistry<iEngine> (manager->object_reg);
    if (!engine)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "No iEngine plugin!");
      return 0;
    }

    // Create the mesh factory.
    csRef<iMeshFactoryWrapper> meshFact =
      engine->CreateMeshFactory ("crystalspace.mesh.object.genmesh",
				 "meshFact");
    if (meshFact == 0)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "Error creating custom mesh factory!");
      return 0;
    }
 
    // Generate the mesh topology
    csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
      (meshFact->GetMeshObjectFactory ());

    gmstate->SetVertexCount (vertexCount);
    gmstate->SetTriangleCount (triangleCount);

    for (unsigned int i = 0; i < vertexCount; i++)
      gmstate->GetVertices ()[i] = vertices[i];

    for (unsigned int i = 0; i < triangleCount; i++)
    {
      gmstate->GetTriangles ()[i].a = indices[i * 3];
      gmstate->GetTriangles ()[i].b = indices[i * 3 + 1];
      gmstate->GetTriangles ()[i].c = indices[i * 3 + 2];
    }
    gmstate->CalculateNormals ();

    meshFact->HardTransform (transform);

    // Create the mesh.
    csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (meshFact, "mesh",
						       sector));
    mesh->GetMeshObject ()->SetMaterialWrapper (material);

    return mesh;
  }

  BoneKinematicCallback::BoneKinematicCallback (iMeshWrapper* mesh,
			   iBulletKinematicCallback* callback)
    : scfImplementationType (this), mesh (mesh), callback (callback)
  {
  }

  BoneKinematicCallback::~BoneKinematicCallback ()
  {
  }

  void BoneKinematicCallback::GetBodyTransform
    (iRigidBody* body, csOrthoTransform& transform) const
  {
    callback->GetBodyTransform (body, transform);
    mesh->GetMovable ()->SetTransform (transform);
    mesh->GetMovable ()->UpdateMove ();
  }

}
CS_PLUGIN_NAMESPACE_END(DebugDynamics)
