/*
  Copyright (C) 2009 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

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
#include "dynsysdebug.h"
#include "ivaria/dynamics.h"
#include "ivaria/reporter.h"
#include "csgeom/vector3.h"
#include "csgeom/sphere.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "cstool/materialbuilder.h"

//------------------------ csDynamicSystemDebugger ----------------------

csDynamicSystemDebugger::csDynamicSystemDebugger ()
  : debugMode (false)
{
}

void csDynamicSystemDebugger::SetObjectRegistry (iObjectRegistry* object_reg)
{
  this->object_reg = object_reg;
}

void csDynamicSystemDebugger::SetDynamicSystem (iDynamicSystem* system)
{
  this->system = system;
}

void csDynamicSystemDebugger::SetDebugSector (iSector* sector)
{
  this->sector = sector;
}

void csDynamicSystemDebugger::SetDebugDisplayMode (bool debugMode)
{
  // check dynsys available
  if (!system)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	      "crystalspace.dynsysdebugger",
	      "No dynamic system defined");
    return;
  }

  // check debug material available
  if (!material)
    material = CS::Material::MaterialBuilder::CreateColorMaterial
      (object_reg, "dynsysdebug", csColor (1, 0, 0));

  if (!material)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	      "crystalspace.dynsysdebugger",
	      "No debug material defined");
    return;
  }

  // check display mode has changed
  if (debugMode == this->debugMode)
    return;
  this->debugMode = debugMode;

  // find the pointer to the engine plugin
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  if (engine == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.dynsysdebugger",
      "No iEngine plugin!");
    return;
  }

  // iterate through each rigid body
  unsigned int bodyIndex = 0;
  for ( ; bodyIndex < (unsigned int) system->GetBodysCount (); bodyIndex++)
  {
    iRigidBody* body = system->GetBody (bodyIndex);

    MeshData meshData;

    // if in debug mode, then store the original attached mesh
    if (debugMode)
    {
      // TODO: store the sector of the mesh
      csRef<iMeshWrapper> mesh = body->GetAttachedMesh ();
      if (mesh)
      {
	engine->RemoveObject (mesh);
	meshData.mesh = mesh;
	storedMeshes.PutUnique (body, meshData);
      }
    }

    // else, put back the original attached mesh
    else
    {
      if (storedMeshes.Contains (body))
      {
	// TODO: the debug mesh is not removed when this is an animesh 
	MeshData nullData;
	meshData = storedMeshes.Get (body, nullData);
	engine->RemoveObject (body->GetAttachedMesh ());
	meshData.mesh->GetMovable ()->SetSector (sector);
	body->AttachMesh (meshData.mesh);
      }
      continue;
    }

    // TODO: display the joints too
    // TODO: use iDynamicsSystemCollider::FillWithColliderGeometry instead
    // TODO: use specific colors for objects static/dynamic/active/inactive
    // TODO: display collisions

    // iterate through each collider
    unsigned int colliderIndex = 0;
    for ( ; colliderIndex < (unsigned int) body->GetColliderCount (); colliderIndex++)
    {
      // TODO: it won't work if there is more than one collider for a body
      csRef<iDynamicsSystemCollider> collider = body->GetCollider (colliderIndex);

      switch (collider->GetGeometryType ())
      {
      case BOX_COLLIDER_GEOMETRY:
	{
	  // get box geometry
	  csVector3 boxSize;
	  collider->GetBoxGeometry (boxSize);
	  boxSize /= 2.0;
	  const csBox3 box(-boxSize, boxSize);

	  // create box
	  csRef<iMeshWrapper> mesh = DebugShape::CreateBoxMesh (box, material,
				collider->GetLocalTransform (), sector, object_reg);
	  body->AttachMesh (mesh);
	}
	break;

      case SPHERE_COLLIDER_GEOMETRY:
	{
	  // get sphere geometry
	  csSphere sphere;
	  collider->GetSphereGeometry (sphere);

	  // create sphere
	  csRef<iMeshWrapper> mesh = DebugShape::CreateSphereMesh (sphere,
						      material, sector, object_reg);
	  body->AttachMesh (mesh);
	}
	break;

      case CYLINDER_COLLIDER_GEOMETRY:
	{
	  // get cylinder geometry
	  float length, radius;
	  collider->GetCylinderGeometry (length, radius);

	  // create cylinder
	  csRef<iMeshWrapper> mesh = DebugShape::CreateCylinderMesh (length, radius,
			  material, collider->GetLocalTransform (), sector, object_reg);
	  body->AttachMesh (mesh);
	}
	break;

      case CAPSULE_COLLIDER_GEOMETRY:
	{
	  // get capsule geometry
	  float length, radius;
	  collider->GetCapsuleGeometry (length, radius);

	  // create capsule
	  csRef<iMeshWrapper> mesh = DebugShape::CreateCapsuleMesh (length, radius,
			   material, collider->GetLocalTransform (), sector, object_reg);
	  body->AttachMesh (mesh);
	}
	break;

      default:
	// TODO: convex/concave meshes
	break;
      }
    }
  }

  // TODO: do the same with the static colliders of the dynamic system
}

//------------------------ DebugShape ----------------------

csRef<iMeshWrapper> DebugShape::CreateBoxMesh (csBox3 box,
			     iMaterialWrapper* material, csOrthoTransform transform,
			     iSector* sector, iObjectRegistry* object_reg)
{
  // find the pointer to the engine plugin
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.debugshape",
      "No iEngine plugin!");
    return 0;
  }

  // Create the box mesh factory.
  csRef<iMeshFactoryWrapper> boxFact = engine->CreateMeshFactory(
		"crystalspace.mesh.object.genmesh", "boxFact");
  if (!boxFact)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
	      "crystalspace.debugshape",
	      "Error creating box mesh factory");
    return 0;
  }

  // generate the box topology
  csRef<iGeneralFactoryState> gmstate = scfQueryInterface<iGeneralFactoryState>
    (boxFact->GetMeshObjectFactory ());
  gmstate->GenerateBox (box);

  boxFact->HardTransform (transform);

  // Create the mesh.
  csRef<iMeshWrapper> mesh (engine->CreateMeshWrapper (boxFact, "box", sector));
  mesh->GetMeshObject ()->SetMaterialWrapper (material);

  return mesh;
}

csRef<iMeshWrapper> DebugShape::CreateSphereMesh (csSphere sphere,
	iMaterialWrapper* material, iSector* sector, iObjectRegistry* object_reg)
{
  // find the pointer to the engine plugin
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.debugshape",
      "No iEngine plugin!");
    return 0;
  }

   // Create the sphere mesh factory.
  csRef<iMeshFactoryWrapper> sphereFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "sphereFact");
  if (sphereFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.debugshape",
      "Error creating sphere mesh factory!");
    return 0;
  }
 
  // generate the sphere topology
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

csRef<iMeshWrapper> DebugShape::CreateCylinderMesh (float length, float radius,
			iMaterialWrapper* material, csOrthoTransform transform,
			iSector* sector, iObjectRegistry* object_reg)
{
  // find the pointer to the engine plugin
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.debugshape",
      "No iEngine plugin!");
    return 0;
  }

   // Create the cylinder mesh factory.
  csRef<iMeshFactoryWrapper> cylinderFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "cylinderFact");
  if (cylinderFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.debugshape",
      "Error creating cylinder mesh factory!");
    return 0;
  }
 
  // generate the cylinder topology
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

csRef<iMeshWrapper> DebugShape::CreateCapsuleMesh (float length, float radius,
			iMaterialWrapper* material, csOrthoTransform transform,
			iSector* sector, iObjectRegistry* object_reg)
{
  // find the pointer to the engine plugin
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.debugshape",
      "No iEngine plugin!");
    return 0;
  }

   // Create the capsule mesh factory.
  csRef<iMeshFactoryWrapper> capsuleFact = engine->CreateMeshFactory(
  	"crystalspace.mesh.object.genmesh", "capsuleFact");
  if (capsuleFact == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.debugshape",
      "Error creating capsule mesh factory!");
    return 0;
  }
 
  // generate the capsule topology
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
