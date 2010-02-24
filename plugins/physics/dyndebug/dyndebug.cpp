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
    : scfImplementationType (this), manager (manager)
  {
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
    // Check dynsys available
    if (!system)
    {
      manager->Report (CS_REPORTER_SEVERITY_WARNING,
		       "No dynamic system defined");
      return;
    }

    // Check debug material available
    if (!material)
      material = CS::Material::MaterialBuilder::CreateColorMaterial
	(manager->object_reg, "dynsysdebug", csColor (1, 0, 0));

    if (!material)
    {
      manager->Report (CS_REPORTER_SEVERITY_WARNING,
		       "No debug material defined");
      return;
    }

    // Check display mode has changed
    if (debugMode == this->debugMode)
      return;
    this->debugMode = debugMode;

    // Find the pointer to the engine plugin
    csRef<iEngine> engine = csQueryRegistry<iEngine> (manager->object_reg);
    if (engine == 0)
    {
      manager->Report (CS_REPORTER_SEVERITY_ERROR,
		       "No iEngine plugin!");
      return;
    }

    // TODO: remove all previous debug meshes

    // Iterate through each rigid body
    for (unsigned int bodyIndex = 0;
	 bodyIndex < (unsigned int) system->GetBodysCount ();
	 bodyIndex++)
    {
      iRigidBody* body = system->GetBody (bodyIndex);

      MeshData meshData;

      // If in debug mode, then store the original attached mesh
      if (debugMode)
      {
	// TODO: store the sector of the mesh
	csRef<iMeshWrapper> mesh = body->GetAttachedMesh ();
	if (mesh)
	{
	  engine->RemoveObject (mesh);
	  meshData.mesh = mesh;
	}

	storedMeshes.PutUnique (body, meshData);
      }

      // Else, go back to normal mode
      else
      {
	if (storedMeshes.Contains (body))
	{
	  // Remove the debug mesh
	  engine->RemoveObject (body->GetAttachedMesh ());

	  // Put back the original attached mesh
	  MeshData nullData;
	  meshData = storedMeshes.Get (body, nullData);

	  if (meshData.mesh)
	  {
	    meshData.mesh->GetMovable ()->SetSector (sector);
	    body->AttachMesh (meshData.mesh);
	  }

	  else
	    body->AttachMesh (0);

	  // Erase body reference
	  storedMeshes.DeleteAll (body);
	}

	continue;
      }

      // TODO: display the joints too
      // TODO: use iDynamicsSystemCollider::FillWithColliderGeometry instead?
      // TODO: use specific colors for objects static/dynamic/active/inactive
      // TODO: display collisions

      // Iterate through each collider
      for (unsigned int colliderIndex = 0;
	   colliderIndex < (unsigned int) body->GetColliderCount ();
	   colliderIndex++)
      {
	// TODO: it won't work if there is more than one collider for a body
	csRef<iDynamicsSystemCollider> collider =
	  body->GetCollider (colliderIndex);

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
	      csRef<iMeshWrapper> mesh =
		CreateBoxMesh (box, material, collider->GetLocalTransform (),
			       sector);
	      body->AttachMesh (mesh);
	    }
	    break;

	  case SPHERE_COLLIDER_GEOMETRY:
	    {
	      // Get sphere geometry
	      csSphere sphere;
	      collider->GetSphereGeometry (sphere);

	      // Create sphere
	      csRef<iMeshWrapper> mesh =
		CreateSphereMesh (sphere, material, sector);
	      body->AttachMesh (mesh);
	    }
	    break;

	  case CYLINDER_COLLIDER_GEOMETRY:
	    {
	      // Get cylinder geometry
	      float length, radius;
	      collider->GetCylinderGeometry (length, radius);

	      // Create cylinder
	      csRef<iMeshWrapper> mesh =
		CreateCylinderMesh (length, radius, material,
				    collider->GetLocalTransform (), sector);
	      body->AttachMesh (mesh);
	    }
	    break;

	  case CAPSULE_COLLIDER_GEOMETRY:
	    {
	      // Get capsule geometry
	      float length, radius;
	      collider->GetCapsuleGeometry (length, radius);

	      // Create capsule
	      csRef<iMeshWrapper> mesh =
		CreateCapsuleMesh (length, radius, material,
				   collider->GetLocalTransform (), sector);
	      body->AttachMesh (mesh);
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
	      csRef<iMeshWrapper> mesh =
		CreateCustomMesh (vertices, vertexCount, indices, triangleCount, material,
				collider->GetLocalTransform (), sector);
	      body->AttachMesh (mesh);
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
	      csRef<iMeshWrapper> mesh =
		CreateCustomMesh (vertices, vertexCount, indices, triangleCount, material,
				collider->GetLocalTransform (), sector);
	      body->AttachMesh (mesh);
	    }
	    break;

	  default:
	    // TODO: plan meshes
	    break;
	  }
      }
    }

    // TODO: do the same with the static colliders of the dynamic system
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

}
CS_PLUGIN_NAMESPACE_END(DebugDynamics)
