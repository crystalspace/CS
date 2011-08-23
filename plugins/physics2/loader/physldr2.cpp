/*
    Copyright (C) 2011 by Liu Lu

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

#include "csgeom/plane3.h"
#include "csgeom/transfrm.h"
#include "csutil/stringquote.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imap/ldrctxt.h"
#include "imap/loader.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "ivaria/collision2.h"
#include "ivaria/bullet2.h"
#include "ivaria/physical2.h"
#include "imesh/terrain2.h"
#include "physldr2.h"

enum
{
  XMLTOKEN_INTERNALSCALE,
  XMLTOKEN_COLLISIONSECTOR,
  XMLTOKEN_GRAVITY,
  XMLTOKEN_DAMPENER,
  XMLTOKEN_GROUP,
  XMLTOKEN_SECTOR,
  XMLTOKEN_STEPPARAS,
  XMLTOKEN_SIMULATIONSPEED,
  XMLTOKEN_RIGIDBODY,
  XMLTOKEN_SOFTBODY,
  XMLTOKEN_COLLISIONOBJECT,
  XMLTOKEN_MASS,
  XMLTOKEN_MESH,
  XMLTOKEN_COLLIDERCONVEXMESH,
  XMLTOKEN_COLLIDERCONCAVEMESH,
  XMLTOKEN_COLLIDERSPHERE,
  XMLTOKEN_COLLIDERCYLINDER,
  XMLTOKEN_COLLIDERCAPSULE,
  XMLTOKEN_COLLIDERCONE,
  XMLTOKEN_COLLIDERBOX,
  XMLTOKEN_COLLIDERPLANE,
  XMLTOKEN_COLLIDERTERRAIN,
  XMLTOKEN_MOVE,
  XMLTOKEN_ROTATE,
  XMLTOKEN_JOINT,
  XMLTOKEN_CONSTRAINTS,
  XMLTOKEN_DISTANCE,
  XMLTOKEN_ANGLE,
  XMLTOKEN_CONSTRAINED,
  XMLTOKEN_FREE,
  XMLTOKEN_MIN,
  XMLTOKEN_MAX,
  XMLTOKEN_MOVABLE,
  XMLTOKEN_DYNAMICSTATE,
  XMLTOKEN_DYNAMIC,
  XMLTOKEN_STATIC,
  XMLTOKEN_KINEMATIC,
  XMLTOKEN_ANCHOR,
  XMLTOKEN_WIND,
  XMLTOKEN_SPRING,
  XMLTOKEN_LINEARSTIFFNESS,
  XMLTOKEN_ANGULARSTIFFNESS,
  XMLTOKEN_LINEARDAMPING,
  XMLTOKEN_ANGULARDAMPING,
};

SCF_IMPLEMENT_FACTORY (csPhysicsLoader2)


csPhysicsLoader2::csPhysicsLoader2 (iBase* pParent) :
scfImplementationType(this, pParent)
{
}

csPhysicsLoader2::~csPhysicsLoader2 ()
{
}

bool csPhysicsLoader2::Initialize (iObjectRegistry* object_reg)
{
  csPhysicsLoader2::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  xmltokens.Register ("scale", XMLTOKEN_INTERNALSCALE);
  xmltokens.Register ("collisionsector", XMLTOKEN_COLLISIONSECTOR);
  xmltokens.Register ("gravity", XMLTOKEN_GRAVITY);
  xmltokens.Register ("dampener", XMLTOKEN_DAMPENER);
  xmltokens.Register ("group", XMLTOKEN_GROUP);
  xmltokens.Register ("sector", XMLTOKEN_SECTOR);
  xmltokens.Register ("stepparas", XMLTOKEN_STEPPARAS);
  xmltokens.Register ("simulationspeed", XMLTOKEN_SIMULATIONSPEED);
  xmltokens.Register ("rigidbody", XMLTOKEN_RIGIDBODY);
  xmltokens.Register ("softbody", XMLTOKEN_SOFTBODY);
  xmltokens.Register ("collisionobject", XMLTOKEN_COLLISIONOBJECT);
  xmltokens.Register ("mass", XMLTOKEN_MASS);
  xmltokens.Register ("mesh", XMLTOKEN_MESH);
  xmltokens.Register ("colliderconcavemesh", XMLTOKEN_COLLIDERCONCAVEMESH);
  xmltokens.Register ("colliderconvexmesh", XMLTOKEN_COLLIDERCONVEXMESH);
  xmltokens.Register ("collidersphere", XMLTOKEN_COLLIDERSPHERE);
  xmltokens.Register ("collidercylinder", XMLTOKEN_COLLIDERCYLINDER);
  xmltokens.Register ("collidercapsule", XMLTOKEN_COLLIDERCAPSULE);
  xmltokens.Register ("collidercone", XMLTOKEN_COLLIDERCONE);
  xmltokens.Register ("colliderbox", XMLTOKEN_COLLIDERBOX);
  xmltokens.Register ("colliderplane", XMLTOKEN_COLLIDERPLANE);
  xmltokens.Register ("colliderterrain", XMLTOKEN_COLLIDERTERRAIN);
  xmltokens.Register ("move", XMLTOKEN_MOVE);
  xmltokens.Register ("rotate", XMLTOKEN_ROTATE);
  xmltokens.Register ("joint", XMLTOKEN_JOINT);
  xmltokens.Register ("constraints", XMLTOKEN_CONSTRAINTS);
  xmltokens.Register ("distance", XMLTOKEN_DISTANCE);
  xmltokens.Register ("angle", XMLTOKEN_ANGLE);
  xmltokens.Register ("min", XMLTOKEN_MIN);
  xmltokens.Register ("max", XMLTOKEN_MAX);
  xmltokens.Register ("movable", XMLTOKEN_MOVABLE);
  xmltokens.Register ("dynamicstate", XMLTOKEN_DYNAMICSTATE);
  xmltokens.Register ("dynamic", XMLTOKEN_DYNAMIC);
  xmltokens.Register ("static", XMLTOKEN_STATIC);
  xmltokens.Register ("kinematic", XMLTOKEN_KINEMATIC);
  xmltokens.Register ("anchor", XMLTOKEN_ANCHOR);
  xmltokens.Register ("wind", XMLTOKEN_WIND);
  xmltokens.Register ("spring", XMLTOKEN_SPRING);
  xmltokens.Register ("linearstiffness", XMLTOKEN_LINEARSTIFFNESS);
  xmltokens.Register ("angularstiffness", XMLTOKEN_ANGULARSTIFFNESS);
  xmltokens.Register ("lineardamping", XMLTOKEN_LINEARDAMPING);
  xmltokens.Register ("angulardamping", XMLTOKEN_ANGULARDAMPING);
  return true;
}

csPtr<iBase> csPhysicsLoader2::Parse (iDocumentNode *node, 
                                      iStreamSource*, 
                                      iLoaderContext* ldr_context, 
                                      iBase* context)
{
  engine = csQueryRegistry<iEngine> (object_reg);
  CS_ASSERT (engine);
  collisionSystem = csQueryRegistry<CS::Collision2::iCollisionSystem> (object_reg);
  if (collisionSystem == NULL)
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
      node, "No physics in object registry!");
    return 0;
  }
  physicalSystem = scfQueryInterface<CS::Physics2::iPhysicalSystem> (collisionSystem);
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    if (id == XMLTOKEN_COLLISIONSECTOR)
    {
      csRef<iDocumentAttribute> attr = child->GetAttribute ("name");
      csRef<CS::Collision2::iCollisionSector> collisionSector;
      if (attr)
        collisionSector = collisionSystem->FindCollisionSector (attr->GetValue ());
      if (! collisionSector)
      {
        collisionSector = collisionSystem->CreateCollisionSector ();
        if (attr)
          collisionSector->QueryObject ()->SetName (attr->GetValue ());
      }
      if (!ParseCollisionSector (child, collisionSector, ldr_context))
        return 0;
      else
      {
        synldr->ReportBadToken (child);
        return 0;
      }
    }
    else if (id == XMLTOKEN_INTERNALSCALE)
      collisionSystem->SetInternalScale (child->GetAttributeValueAsFloat ("scale"));
  }
  return csPtr<iBase> (collisionSystem);
}

bool csPhysicsLoader2::ParseCollisionSector (iDocumentNode *node, 
                                             CS::Collision2::iCollisionSector* collSector,
                                             iLoaderContext* ldr_context)
{
  csRef<CS::Physics2::iPhysicalSector> physSector = 
    scfQueryInterface<CS::Physics2::iPhysicalSector> (collSector);
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT)
      continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_GRAVITY:
      {
        csVector3 v;
        if (!synldr->ParseVector (child, v))
        {
          synldr->ReportError ("crystalspace.dynamics.loader",
            child, "Error processing gravity token");
          return false;
        }
        collSector->SetGravity (v);
        break;
      }
    case XMLTOKEN_GROUP:
      {
        const char* name = child->GetAttributeValue ("name");
        if (collSector->FindCollisionGroup (name).name.Compare (name) == false)
          collSector->CreateCollisionGroup (name);
        break;
      }
    case XMLTOKEN_SECTOR:
      {
        const char* name = child->GetAttributeValue ("name");
        iSector* sec = engine->FindSector (name);
        if (sec)
          collSector->SetSector (sec);
        else
        {
          synldr->ReportError ("crystalspace.dynamics.loader",
            child, "Error processing sector token");
          return false;
        }
        break;
      }
    case XMLTOKEN_DAMPENER:
      {
        float angular = child->GetAttributeValueAsFloat ("angular");
        float linear = child->GetAttributeValueAsFloat ("linear");
        physSector->SetRollingDampener (angular);
        physSector->SetLinearDampener (linear);
        break;
      }
    case XMLTOKEN_SIMULATIONSPEED:
      {
        float speed = child->GetAttributeValueAsFloat ("speed");
        physSector->SetSimulationSpeed (speed);
        break;
      }
    case XMLTOKEN_STEPPARAS:
      {
        float timeStep = child->GetAttributeValueAsFloat ("timestep");
        float maxStep = child->GetAttributeValueAsFloat ("maxstep");
        float iteration = child->GetAttributeValueAsFloat ("iteration");
      }
    case XMLTOKEN_COLLISIONOBJECT:
      {
        csRef<CS::Collision2::iCollisionObject> obj= collisionSystem->CreateCollisionObject ();
        if (!ParseCollisionObject (child, obj, collSector, ldr_context))
          return false;
        break;
      }
    case XMLTOKEN_RIGIDBODY:
      {
        csRef<CS::Physics2::iRigidBody> rb = physicalSystem->CreateRigidBody ();
        if (!ParseRigidBody (child, rb, collSector, ldr_context))
          return false;
        break;
      }
    case XMLTOKEN_SOFTBODY:
      {
        if (!ParseSoftBody (child, physSector, ldr_context))
          return false;
        break;
      }
    case XMLTOKEN_JOINT:
      {
        csRef<CS::Physics2::iJoint> joint = physicalSystem->CreateJoint ();
        if (!ParseJoint (child, joint, physSector))
          return false;
        break;
      }
    default:
      synldr->ReportBadToken (child);
      return false;
    }
  }
  return true;
}

bool csPhysicsLoader2::ParseCollisionObject (iDocumentNode *node, 
                                             CS::Collision2::iCollisionObject* object, 
                                             CS::Collision2::iCollisionSector* collSector, 
                                             iLoaderContext* ldr_context)
{
  const char *name = node->GetAttributeValue ("name");
  object->QueryObject()->SetName (name);

  if (node->GetAttribute ("ghost"))
    object->SetObjectType (CS::Collision2::COLLISION_OBJECT_GHOST);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_MOVABLE:
      {
        if (child->GetContentsValue ())
        {
          csRef<iMeshWrapper> m = ldr_context->FindMeshObject (child->GetContentsValue ());
          if (m)
            object->SetAttachedMovable (m->GetMovable ());
          else
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Unable to find mesh in engine");
            return false;
          }
        }
        break;
      }
    case XMLTOKEN_COLLIDERBOX:
      {
        if (!ParseColliderBox (child, object))
          return false;
        break;
      }
    case XMLTOKEN_COLLIDERSPHERE:
      {
        if (!ParseColliderSphere (node, object))
          return false;
        break;
      }
    case XMLTOKEN_COLLIDERCAPSULE:
      {
        if (!ParseColliderCapsule (node, object))
          return false;
        break;
      }
    case XMLTOKEN_COLLIDERCYLINDER:
      {
        if (!ParseColliderCylinder (node, object))
          return false;
        break;
      }
    case XMLTOKEN_COLLIDERCONE:
      {
        if (!ParseColliderCone (node, object))
          return false;
        break;
      }
    case XMLTOKEN_COLLIDERPLANE:
      {
        if (!ParseColliderPlane (node, object))
          return false;
        break;
      }
    case XMLTOKEN_COLLIDERCONVEXMESH:
      {
        if (!ParseColliderConvexMesh (node, object, ldr_context))
          return false;
        break;
      }
    case XMLTOKEN_COLLIDERCONCAVEMESH:
      {
        if (!ParseColliderConcaveMesh (node, object, ldr_context))
          return false;
        break;
      }
    case XMLTOKEN_COLLIDERTERRAIN:
      {
        if (!ParseColliderTerrain (node, object, ldr_context))
          return false;
        break;
      }
    }
  }
  object->RebuildObject ();
  collSector->AddCollisionObject (object);

  csOrthoTransform trans;
  ParseTransform (node, trans);
  object->SetTransform (trans);
  
  return true;
}

bool csPhysicsLoader2::ParseRigidBody (iDocumentNode *node, 
                                       CS::Physics2::iRigidBody* body,
                                       CS::Collision2::iCollisionSector* collSector,
                                       iLoaderContext* ldr_context)
{
  if (node->GetAttributeValue ("mass"))
    body->SetMass ( node->GetAttributeValueAsFloat ("mass"));

  if (node->GetAttributeValue ("density"))
    body->SetDensity (node->GetAttributeValueAsFloat ("density"));

  if (node->GetAttributeValue ("friction"))
    body->SetFriction (node->GetAttributeValueAsFloat ("friction"));
  if (node->GetAttributeValue ("elasticity"))
    body->SetElasticity (node->GetAttributeValueAsFloat ("elasticity"));

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    if (id == XMLTOKEN_DAMPENER)
    {
      float angular = child->GetAttributeValueAsFloat ("angular");
      float linear = child->GetAttributeValueAsFloat ("linear");
      body->SetRollingDampener (angular);
      body->SetLinearDampener (linear);
      break;
    }
    else if (id == XMLTOKEN_DYNAMICSTATE)
    {
      const char* state = child->GetContentsValue ();
      csStringID stateID = xmltokens.Request (state);
      switch (stateID)
      {
      case XMLTOKEN_DYNAMIC:
        body->SetState (CS::Physics2::STATE_DYNAMIC);
        break;
      case XMLTOKEN_STATIC:
        body->SetState (CS::Physics2::STATE_STATIC);
        break;
      case XMLTOKEN_KINEMATIC:
        body->SetState (CS::Physics2::STATE_KINEMATIC);
        break;
      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }
  }

  return ParseCollisionObject (node, body->QueryCollisionObject (), collSector, ldr_context);
}

bool csPhysicsLoader2::ParseSoftBody (iDocumentNode *node, 
                                      CS::Physics2::iPhysicalSector* physSector,
                                      iLoaderContext* ldr_context)
{
  csOrthoTransform trans;
  ParseTransform (node, trans);

  csRef<CS::Physics2::iSoftBody> body;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_MESH:
      if (child->GetContentsValue ())
      {
        csRef<iMeshFactoryWrapper> m = ldr_context->FindMeshFactory (child->GetContentsValue ());
        if (!m)
        {
          synldr->ReportError ("crystalspace.dynamics.loader",
            child, "Unable to find mesh factory in engine");
          return false;
        }
        csRef<iGeneralFactoryState> gmstate = scfQueryInterface<
          iGeneralFactoryState> (m->GetMeshObjectFactory ());

        body = physicalSystem->CreateSoftBody (gmstate, trans);
      }
      break;
    }
  }

  const char *name = node->GetAttributeValue ("name");
  body->QueryObject()->SetName (name);

  if (node->GetAttributeValue ("mass"))
    body->SetMass ( node->GetAttributeValueAsFloat ("mass"));

  if (node->GetAttributeValue ("friction"))
    body->SetFriction (node->GetAttributeValueAsFloat ("friction"));
  if (node->GetAttributeValue ("rigidity"))
    body->SetRigidity (node->GetAttributeValueAsFloat ("rigidity"));

  body->RebuildObject ();
  physSector->AddSoftBody (body);

  it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_ANCHOR:
      {
        size_t index = child->GetAttributeValueAsInt ("index");
        const char* rname = child->GetAttributeValue ("rigidbody");
        if (rname)
        {
          CS::Physics2::iRigidBody* rb = physSector->FindRigidBody (rname);
          if (!rb)
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Can't find rigid body with name %s!", CS::Quote::Single (rname));
            return false;
          }
          body->AnchorVertex (index, rb);
        }
        else
          body->AnchorVertex (index);
        break;
      }
    case XMLTOKEN_WIND:
      {
        csVector3 v;
        synldr->ParseVector (child, v);
        body->SetWindVelocity (v);
        break;
      }
    default:
      synldr->ReportBadToken (child);
      return false;
    }
  }
  return true;
}

static float GetFloat (iDocumentNode* node, const char* name, float def)
{
  csRef<iDocumentAttribute> attr = node->GetAttribute (name);
  if (!attr) return def;
  return attr->GetValueAsFloat ();
}

bool csPhysicsLoader2::ParseColliderBox (iDocumentNode *node, CS::Collision2::iCollisionObject* object)
{
  csVector3 v;
  if (!synldr->ParseVector (node, v))
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
      node, "Error processing box parameters");
    return false;
  }

  csOrthoTransform trans;
  trans.Identity ();
  ParseTransform (node, trans);

  csRef<CS::Collision2::iColliderBox> box = collisionSystem->CreateColliderBox (v);
  object->AddCollider (box, trans);
  return true;
}

bool csPhysicsLoader2::ParseColliderSphere (iDocumentNode *node, CS::Collision2::iCollisionObject* object)
{
  float r = node->GetAttributeValueAsFloat ("radius");
  csRef<CS::Collision2::iColliderSphere> sp = collisionSystem->CreateColliderSphere (r);

  csOrthoTransform trans;
  trans.Identity ();
  ParseTransform (node, trans);

  object->AddCollider (sp, trans);
  return true;
}

bool csPhysicsLoader2::ParseColliderCylinder (iDocumentNode *node, CS::Collision2::iCollisionObject* object)
{
  float l = node->GetAttributeValueAsFloat ("length");
  float r = node->GetAttributeValueAsFloat ("radius");
  csRef<CS::Collision2::iColliderCylinder> cy = collisionSystem->CreateColliderCylinder (l, r);

  csOrthoTransform trans;
  trans.Identity ();
  ParseTransform (node, trans);

  object->AddCollider (cy, trans);
  return true;
}

bool csPhysicsLoader2::ParseColliderCapsule (iDocumentNode *node, CS::Collision2::iCollisionObject* object)
{
  float l = node->GetAttributeValueAsFloat ("length");
  float r = node->GetAttributeValueAsFloat ("radius");
  csRef<CS::Collision2::iColliderCapsule> ca = collisionSystem->CreateColliderCapsule (l, r);

  csOrthoTransform trans;
  trans.Identity ();
  ParseTransform (node, trans);

  object->AddCollider (ca, trans);
  return true;
}

bool csPhysicsLoader2::ParseColliderCone (iDocumentNode *node, CS::Collision2::iCollisionObject* object)
{
  float l = node->GetAttributeValueAsFloat ("length");
  float r = node->GetAttributeValueAsFloat ("radius");
  csRef<CS::Collision2::iColliderCone> co = collisionSystem->CreateColliderCone (l, r);

  csOrthoTransform trans;
  trans.Identity ();
  ParseTransform (node, trans);

  object->AddCollider (co, trans);
  return true;
}

bool csPhysicsLoader2::ParseColliderPlane (iDocumentNode *node, CS::Collision2::iCollisionObject* object)
{
  csPlane3 plane;
  if (synldr->ParsePlane (node, plane))
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
      node, "Error processing plane parameters");
    return false;
  }
  csRef<CS::Collision2::iColliderPlane> pl = collisionSystem->CreateColliderPlane (plane);

  csOrthoTransform trans;
  trans.Identity ();
  ParseTransform (node, trans);

  object->AddCollider (pl, trans);
  return true;
}

bool csPhysicsLoader2::ParseColliderConvexMesh (iDocumentNode *node, CS::Collision2::iCollisionObject* object, iLoaderContext* ldr_context)
{
  if (!node->GetContentsValue ()) { return false; }

  // Wait for load - assume it will exist eventually.
  csRef<iMeshWrapper> m = ldr_context->FindMeshObject
    (node->GetContentsValue ());
  
  if (m)
  {
    csRef<CS::Collision2::iColliderConvexMesh> conv = collisionSystem->CreateColliderConvexMesh (m);

    csOrthoTransform trans;
    trans.Identity ();
    ParseTransform (node, trans);

    object->AddCollider (conv, trans);
    return true;
  }
  else
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
      node, "Unable to find collider mesh in engine");
    return false;
  }
}

bool csPhysicsLoader2::ParseColliderConcaveMesh (iDocumentNode *node, CS::Collision2::iCollisionObject* object, iLoaderContext* ldr_context)
{
  if (!node->GetContentsValue ()) { return false; }

  // Wait for load - assume it will exist eventually.
  csRef<iMeshWrapper> m = ldr_context->FindMeshObject
    (node->GetContentsValue ());

  if (m)
  {
    csRef<CS::Collision2::iColliderConcaveMesh> conc = collisionSystem->CreateColliderConcaveMesh (m);

    csOrthoTransform trans;
    trans.Identity ();
    ParseTransform (node, trans);

    object->AddCollider (conc, trans);
    return true;
  }
  else
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
      node, "Unable to find collider mesh in engine");
    return false;
  }
}

bool csPhysicsLoader2::ParseColliderTerrain (iDocumentNode *node, CS::Collision2::iCollisionObject* object, iLoaderContext* ldr_context)
{
  if (!node->GetContentsValue ()) { return false; }

  // Wait for load - assume it will exist eventually.
  csRef<iMeshWrapper> m = ldr_context->FindMeshObject
    (node->GetContentsValue ());

  if (m)
  {
    csRef<iTerrainSystem> terrain = scfQueryInterface<iTerrainSystem> (m->GetMeshObject ());
    if (!terrain)
    {
      synldr->ReportError ("crystalspace.dynamics.loader",
        node, "Unable to find terrain system in engine");
      return false;
    }
    csRef<CS::Collision2::iColliderTerrain> terr = collisionSystem->CreateColliderTerrain (terrain);

    csOrthoTransform trans;
    trans.Identity ();
    ParseTransform (node, trans);

    object->AddCollider (terr, trans);
    return true;
  }
  else
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
      node, "Unable to find collider mesh in engine");
    return false;
  }
}

bool csPhysicsLoader2::ParseTransform (iDocumentNode* node, csOrthoTransform &t)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_MOVE:
      {
        csVector3 v;
        synldr->ParseVector (child, v);
        t.SetOrigin (v);
        break;
      }
    case XMLTOKEN_ROTATE:
      {
        csMatrix3 m;
        synldr->ParseMatrix (child, m);
        t.SetO2T (m);
        break;
      }
    }
  }
  return true;
}

bool csPhysicsLoader2::ParseJoint (iDocumentNode *node, CS::Physics2::iJoint* joint, CS::Physics2::iPhysicalSector* sector)
{
  joint->SetTransConstraints (false, false, false);
  joint->SetRotConstraints (false, false, false);
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  csOrthoTransform t;
  csRef<CS::Physics2::iPhysicalBody> body1 = 0;
  csRef<CS::Physics2::iPhysicalBody> body2 = 0;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_RIGIDBODY:
      if (body1 == NULL)
      {
        const char* name = child->GetContentsValue ();
        if (name)
        {
          body1 = sector->FindRigidBody (name);
          if (!body1)
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Can't find rigid body with name %s!", CS::Quote::Single (name));
            return false;
          }
        }
        else
        {
          synldr->ReportError ("crystalspace.dynamics.loader",
            child, "Body should have a name");
          return false;
        }
      }
      else if (body2 == NULL)
      {
        const char* name = child->GetContentsValue ();
        if (name)
        {
          body2 = sector->FindRigidBody (name);
          if (!body2)
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Can't find rigid body with name %s!", CS::Quote::Single (name));
            return false;
          }
        }
        else
        {
          synldr->ReportError ("crystalspace.dynamics.loader",
            child, "Body should have a name");
          return false;
        }
      }
      else
      {
        synldr->ReportError ("crystalspace.dynamics.loader",
          child, "Too many bodies attached to joint");
        return false;
      }
      break;
    case XMLTOKEN_SOFTBODY:
      if (body1 == 0)
      {
        const char* name = child->GetContentsValue ();
        if (name)
        {
          body1 = sector->FindSoftBody (name);
          if (!body1)
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Can't find soft body with name %s!", CS::Quote::Single (name));
            return false;
          }
        }
        else
        {
          synldr->ReportError ("crystalspace.dynamics.loader",
            child, "Body should have a name");
          return false;
        }
      }
      else if (body2 == 0)
      {
        const char* name = child->GetContentsValue ();
        if (name)
        {
          body2 = sector->FindSoftBody (name);
          if (!body2)
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Can't find soft body with name %s!", CS::Quote::Single (name));
            return false;
          }
        }
        else
        {
          synldr->ReportError ("crystalspace.dynamics.loader",
            child, "Body should have a name");
          return false;
        }
      }
      else
      {
        synldr->ReportError ("crystalspace.dynamics.loader",
          child, "Too many bodies attached to joint");
        return false;
      }
      break;
    case XMLTOKEN_CONSTRAINTS:
      {
        csRef<iDocumentNodeIterator> cit = child->GetNodes ();
        while (cit->HasNext ())
        {
          csRef<iDocumentNode> cchild = cit->Next ();
          if (cchild->GetType () != CS_NODE_ELEMENT) continue;
          const char *cvalue = cchild->GetValue ();
          csStringID cid = xmltokens.Request (cvalue);
          switch (cid)
          {
          case XMLTOKEN_DISTANCE:
            {
              bool x, y, z;
              csVector3 min, max;
              ParseConstraint (child, x, y, z, min, max);
              joint->SetTransConstraints (x, y, z);
              joint->SetMinimumDistance (min);
              joint->SetMaximumDistance (max);
              break;
            }
          case XMLTOKEN_ANGLE:
            {
              bool x, y, z;
              csVector3 min, max;
              ParseConstraint (child, x, y, z, min, max);
              joint->SetRotConstraints (x, y, z);
              joint->SetMinimumAngle (min);
              joint->SetMaximumAngle (max);
              break;
            }
          default:
            synldr->ReportBadToken (child);
            return false;
          }
        }
        break;
      }
    case XMLTOKEN_MOVE:
      {
        csVector3 v;
        synldr->ParseVector (child, v);
        t.SetOrigin (v);
        break;
      }
    case XMLTOKEN_ROTATE:
      {
        csMatrix3 m;
        synldr->ParseMatrix (child, m);
        t.SetO2T (m);
        break;
      }
    case XMLTOKEN_SPRING:
      {
        joint->SetSpring (true);

        csRef<iDocumentNodeIterator> chIt = child->GetNodes ();
        while (chIt->HasNext ())
        {
          csRef<iDocumentNode> vchild = chIt->Next ();
          if (vchild->GetType () != CS_NODE_ELEMENT) continue;
          const char *vvalue = vchild->GetValue ();
          csStringID id = xmltokens.Request (vvalue);
          if (id == XMLTOKEN_LINEARSTIFFNESS)
          {
            csVector3 v;
            synldr->ParseVector (vchild, v);
            joint->SetLinearStiffness (v);
          }
          else if (id == XMLTOKEN_ANGULARSTIFFNESS)
          {
            csVector3 v;
            synldr->ParseVector (vchild, v);
            joint->SetAngularStiffness (v);
          }
          else if (id == XMLTOKEN_LINEARDAMPING)
          {
            csVector3 v;
            synldr->ParseVector (vchild, v);
            joint->SetLinearDamping (v);
          }
          else if (id == XMLTOKEN_ANGULARDAMPING)
          {
            csVector3 v;
            synldr->ParseVector (vchild, v);
            joint->SetAngularDamping (v);
          }
          else
          {
            synldr->ReportBadToken (vchild);
            return false;
          }
        }
        break;
      }
    default:
      synldr->ReportBadToken (child);
      return false;
    }
  }
  joint->SetTransform (t);
  joint->Attach (body1, body2);
  return true;
}

bool csPhysicsLoader2::ParseConstraint (iDocumentNode *node, bool &x,
                                        bool &y, bool &z, csVector3 &min,
                                        csVector3 &max)
{
  x = strcmp (node->GetAttributeValue ("x"), "true") == 0;
  y = strcmp (node->GetAttributeValue ("y"), "true") == 0;
  z = strcmp (node->GetAttributeValue ("z"), "true") == 0;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_MIN:
      synldr->ParseVector (child, min);
      break;
    case XMLTOKEN_MAX:
      synldr->ParseVector (child, max);
      break;
    default:
      synldr->ReportBadToken (child);
      return false;
    }
  }
  return true;
}