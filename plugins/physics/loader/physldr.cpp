/*
    Copyright (C) 2002 by Jorrit Tyberghein, Daniel Duhprey

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
#include "ivaria/dynamics.h"
#include "ivaria/reporter.h"
#include "ivaria/ode.h"
#include "physldr.h"



enum
{
  XMLTOKEN_SYSTEM,
  XMLTOKEN_GRAVITY,
  XMLTOKEN_DAMPENER,
  XMLTOKEN_GROUP,
  XMLTOKEN_BODY,
  XMLTOKEN_NAME,
  XMLTOKEN_MASS,
  XMLTOKEN_STATIC,
  XMLTOKEN_MESH,
  XMLTOKEN_BONE,
  XMLTOKEN_COLLIDER,
  XMLTOKEN_DENSITY,
  XMLTOKEN_FRICTION,
  XMLTOKEN_ELASTICITY,
  XMLTOKEN_SOFTNESS,
  XMLTOKEN_COLLIDERCONVEXMESH,
  XMLTOKEN_COLLIDERMESH,
  XMLTOKEN_COLLIDERSPHERE,
  XMLTOKEN_RADIUS,
  XMLTOKEN_COLLIDERCYLINDER,
  XMLTOKEN_COLLIDERCAPSULE,
  XMLTOKEN_LENGTH,
  XMLTOKEN_COLLIDERBOX,
  XMLTOKEN_COLLIDERPLANE,
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
  XMLTOKEN_SIMULATIONMODE,
  XMLTOKEN_AUTODISABLE,
  XMLTOKEN_WORLDSTEP,
  XMLTOKEN_STEPFAST,
  XMLTOKEN_QUICKSTEP
};

SCF_IMPLEMENT_FACTORY (csPhysicsLoader)


csPhysicsLoader::csPhysicsLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csPhysicsLoader::~csPhysicsLoader ()
{
}

bool csPhysicsLoader::Initialize (iObjectRegistry* object_reg)
{
  csPhysicsLoader::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  xmltokens.Register ("system", XMLTOKEN_SYSTEM);
  xmltokens.Register ("gravity", XMLTOKEN_GRAVITY);
  xmltokens.Register ("dampener", XMLTOKEN_DAMPENER);
  xmltokens.Register ("group", XMLTOKEN_GROUP);
  xmltokens.Register ("body", XMLTOKEN_BODY);
  xmltokens.Register ("name", XMLTOKEN_NAME);
  xmltokens.Register ("mass", XMLTOKEN_MASS);
  xmltokens.Register ("static", XMLTOKEN_STATIC);
  xmltokens.Register ("mesh", XMLTOKEN_MESH);
  xmltokens.Register ("bone", XMLTOKEN_BONE);
  xmltokens.Register ("collider", XMLTOKEN_COLLIDER);
  xmltokens.Register ("density", XMLTOKEN_DENSITY);
  xmltokens.Register ("friction", XMLTOKEN_FRICTION);
  xmltokens.Register ("elasticity", XMLTOKEN_ELASTICITY);
  xmltokens.Register ("softness", XMLTOKEN_SOFTNESS);
  xmltokens.Register ("collidermesh", XMLTOKEN_COLLIDERMESH);
  xmltokens.Register ("colliderconvexmesh", XMLTOKEN_COLLIDERCONVEXMESH);
  xmltokens.Register ("collidersphere", XMLTOKEN_COLLIDERSPHERE);
  xmltokens.Register ("collidercylinder", XMLTOKEN_COLLIDERCYLINDER);
  xmltokens.Register ("collidercapsule", XMLTOKEN_COLLIDERCAPSULE);
  xmltokens.Register ("colliderbox", XMLTOKEN_COLLIDERBOX);
  xmltokens.Register ("colliderplane", XMLTOKEN_COLLIDERPLANE);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("length", XMLTOKEN_LENGTH);
  xmltokens.Register ("move", XMLTOKEN_MOVE);
  xmltokens.Register ("rotate", XMLTOKEN_ROTATE);
  xmltokens.Register ("joint", XMLTOKEN_JOINT);
  xmltokens.Register ("constraints", XMLTOKEN_CONSTRAINTS);
  xmltokens.Register ("distance", XMLTOKEN_DISTANCE);
  xmltokens.Register ("angle", XMLTOKEN_ANGLE);
  xmltokens.Register ("min", XMLTOKEN_MIN);
  xmltokens.Register ("max", XMLTOKEN_MAX);
  // csODE* related tokens
  xmltokens.Register ("simulationmode", XMLTOKEN_SIMULATIONMODE);
  xmltokens.Register ("autodisable", XMLTOKEN_AUTODISABLE);
  xmltokens.Register ("worldstep", XMLTOKEN_WORLDSTEP);
  xmltokens.Register ("stepfast", XMLTOKEN_STEPFAST);
  xmltokens.Register ("quickstep", XMLTOKEN_QUICKSTEP);
  return true;
}

csPtr<iBase> csPhysicsLoader::Parse (iDocumentNode* node,
		iStreamSource*, iLoaderContext* ldr_context,
		iBase* /*context*/)
{
  engine = csQueryRegistry<iEngine> (object_reg);
  CS_ASSERT (engine != 0);
  csRef<iDynamics> dynamics = csQueryRegistry<iDynamics> (object_reg);
  if (dynamics == 0)
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
                node, "No dynamics in object registry!");
    return 0;
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    if (id == XMLTOKEN_SYSTEM)
    {
      csRef<iDocumentAttribute> attr = child->GetAttribute ("name");
      csRef<iDynamicSystem> system;
      if (attr)
	system = dynamics->FindSystem (attr->GetValue ());
      if (!system)
      {
        system = dynamics->CreateSystem ();
	if (attr)
	  system->QueryObject ()->SetName (attr->GetValue ());
      }
      if (!ParseSystem (child, system, ldr_context))
      {
	return 0;
      }
    } else
    {
      synldr->ReportBadToken (child);
      return 0;
    }
  }
  return csPtr<iBase>(dynamics);
}

bool csPhysicsLoader::ParseSystem (iDocumentNode* node, iDynamicSystem* system,
                                   iLoaderContext* ldr_context)
{
  // Get name for system
  const char *name = node->GetAttributeValue ("name");
  system->QueryObject()->SetName (name);
  // Look for ODE specific properties
  csRef<iODEDynamicSystemState> osys = 
    scfQueryInterface<iODEDynamicSystemState> (system);
  if (osys)
  {
    if (node->GetAttribute("cfm"))
    {
      float cfm = node->GetAttributeValueAsFloat ("cfm");
      osys->SetCFM(cfm);
    }
    if (node->GetAttribute("erp"))
    {
      float erp = node->GetAttributeValueAsFloat ("erp");
      osys->SetERP(erp);
    }
  }
  // Get all tokens
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
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
        system->SetGravity (v);
        break;
      }
      case XMLTOKEN_DAMPENER:
      {
        float angular = child->GetAttributeValueAsFloat ("angular");
        float linear = child->GetAttributeValueAsFloat ("linear");
        system->SetRollingDampener (angular);
        system->SetLinearDampener (linear);
        break;
      }
      case XMLTOKEN_GROUP:
      {
        csRef<iBodyGroup> group = system->CreateGroup ();
        csRef<iDocumentNodeIterator> git = child->GetNodes ();
        while (git->HasNext ())
        {
          csRef<iDocumentNode> gchild = git->Next ();
          if (gchild->GetType () != CS_NODE_ELEMENT) continue;
          const char *value = gchild->GetValue ();
          csStringID id = xmltokens.Request (value);
          if (id == XMLTOKEN_BODY)
	  {
            csRef<iRigidBody> body = system->CreateBody ();
            group->AddBody (body);
            if (!ParseBody (gchild, body, ldr_context))
	    {
	      return false;
	    }
          }
	  else
	  {
            synldr->ReportBadToken (gchild);
          }
        }
        break;
      }
      case XMLTOKEN_COLLIDERMESH:
      case XMLTOKEN_COLLIDERCONVEXMESH:
      {
	if (!ParseSystemColliderMesh (child, system,
                            id == XMLTOKEN_COLLIDERCONVEXMESH, ldr_context)) return false;
	break;
      }
      case XMLTOKEN_COLLIDERSPHERE:
      {
	if (!ParseSystemColliderSphere (child, system)) return false;
	break;
      }
      case XMLTOKEN_COLLIDERPLANE:
      {
        if (!ParseSystemColliderPlane (child, system)) return false;
    	break;
      }
      case XMLTOKEN_COLLIDERCYLINDER:
      {
	if (!ParseSystemColliderCylinder (child, system)) return false;
	break;
      }
      case XMLTOKEN_COLLIDERCAPSULE:
      {
	if (!ParseSystemColliderCapsule (child, system)) return false;
	break;
      }
      case XMLTOKEN_COLLIDERBOX:
      {
	if (!ParseSystemColliderBox (child, system)) return false;
	break;
      }
      case XMLTOKEN_BODY:
      {
        csRef<iRigidBody> body = system->CreateBody ();
        if (!ParseBody (child, body, ldr_context)) return false;
        break;
      }
      case XMLTOKEN_JOINT:
      {
        csRef<iJoint> joint = system->CreateJoint ();
        if (!ParseJoint (child, joint, system)) return false;
        break;
      }
      case XMLTOKEN_AUTODISABLE:
      {
	csRef<iODEDynamicSystemState> osys =
          scfQueryInterface<iODEDynamicSystemState> (system);
	if (osys)
	{
	  bool autodisable;
	  if (!synldr->ParseBool (child, autodisable, false))
	  {
	    synldr->ReportError ("crystalspace.dynamics.loader",
		child, "Error processing autodisable token");
	    return false;
	  }
	  osys->EnableAutoDisable(autodisable);
	}
	break;

      }
      case XMLTOKEN_SIMULATIONMODE:
      {
    	const char* sm = child->GetContentsValue ();
	csStringID sm_id = xmltokens.Request (sm);

	csRef<iODEDynamicSystemState> osys =
          scfQueryInterface<iODEDynamicSystemState> (system);
	if (osys)
	{
	  switch (sm_id)
	  {
	    case XMLTOKEN_WORLDSTEP:
		osys->EnableQuickStep(false);
		osys->EnableStepFast(false);
		break;
	    case XMLTOKEN_STEPFAST:
		osys->EnableStepFast(true);
		break;
	    case XMLTOKEN_QUICKSTEP:
		osys->EnableQuickStep(true);
		break;
	    default:
          	synldr->ReportBadToken (child);
        	return false;
	  }
	}
	// if there is no iODEDynamicSystemState simply
	// ignore the property so maps can still be loaded.
        break;
      }
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }
  return true;
}

bool csPhysicsLoader::ParseBody (iDocumentNode* node, iRigidBody* body, iLoaderContext* ldr_context)
{
  const char *name = node->GetAttributeValue ("name");
  body->QueryObject()->SetName (name);
  float mass = node->GetAttributeValueAsFloat ("mass");
  bool enabled=true;
  if (node->GetAttribute ("enabled"))
    enabled=node->GetAttributeValueAsBool("enabled");
  if (!enabled)
    body->Disable();

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_STATIC:
        body->MakeStatic ();
        break;
      case XMLTOKEN_MESH:
        if (child->GetContentsValue ())
        {
          csRef<iMeshWrapper> m = ldr_context->FindMeshObject
	    (child->GetContentsValue ());
          if (m)
          {
            body->SetTransform (m->GetMovable()->GetTransform());
            body->AttachMesh (m);
          }
          else
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Unable to find mesh in engine");
            return false;
          }
        }
        break;
      case XMLTOKEN_BONE:
        body->MakeDynamic ();
        synldr->ReportError ("crystalspace.dynamics.loader",
          child, "Currently unable to parse a bone, sorry.");
        break;
      case XMLTOKEN_COLLIDER:
        if (!ParseCollider (child, body, ldr_context))
	{
          synldr->ReportError ("crystalspace.dynamics.loader",
            child, "Currently unable to parse a bone, sorry.");
          return false;
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }
  body->AdjustTotalMass (mass);
  return true;
}

bool csPhysicsLoader::ParseCollider (iDocumentNode* node, iRigidBody* body,
                                     iLoaderContext* ldr_context)
{
  float f = node->GetAttributeValueAsFloat ("friction");
  float d = node->GetAttributeValueAsFloat ("density");
  float e = node->GetAttributeValueAsFloat ("elasticity");
  float s = node->GetAttributeValueAsFloat ("softness");

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COLLIDERMESH:
      case XMLTOKEN_COLLIDERCONVEXMESH:
        {
          if (!child->GetAttributeValue ("mesh"))
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "No mesh specified for collidermesh");
            return false;
          }

          csRef<iMeshWrapper> m = ldr_context->FindMeshObject
	    (child->GetAttributeValue ("mesh"));

          csOrthoTransform t;
          ParseTransform (child, t);
          if (m)
          {
            if (id == XMLTOKEN_COLLIDERMESH)
            {
              if (s > 0)
                body->AttachColliderMesh (m, t, f, d, e, s);
              else  //no softness parameter, so use default
                body->AttachColliderMesh (m, t, f, d, e);
            }
            else
            {
              if (s > 0)
                body->AttachColliderConvexMesh (m, t, f, d, e, s);
              else  //no softness parameter, so use default
                body->AttachColliderConvexMesh (m, t, f, d, e);
            }

          }
          else
          {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Unable to find collider mesh in engine");
            return false;
          }
          break;
      }
      case XMLTOKEN_COLLIDERSPHERE:
      {
        float r = child->GetAttributeValueAsFloat ("radius");
        csOrthoTransform t;
        ParseTransform (child, t);
        body->AttachColliderSphere (r, t.GetOrigin(), f, d, e, s);
        break;
      }
      case XMLTOKEN_COLLIDERCYLINDER:
      {
        float l = child->GetAttributeValueAsFloat ("length");
        float r = child->GetAttributeValueAsFloat ("radius");
        csOrthoTransform t;
        ParseTransform (child, t);
        body->AttachColliderCylinder (l, r, t, f, d, e, s);
        break;
      }
      case XMLTOKEN_COLLIDERCAPSULE:
      {
        float l = child->GetAttributeValueAsFloat ("length");
        float r = child->GetAttributeValueAsFloat ("radius");
        csOrthoTransform t;
        ParseTransform (child, t);
        body->AttachColliderCapsule (l, r, t, f, d, e, s);
        break;
      }
      case XMLTOKEN_COLLIDERPLANE:
      {
        csPlane3 plane;
        synldr->ParsePlane (node, plane);
        body->AttachColliderPlane (plane, f, d, e, s);
      }
      case XMLTOKEN_COLLIDERBOX:
      {
        csVector3 v;
        if (!synldr->ParseVector (child, v))
	{
          synldr->ReportError ("crystalspace.dynamics.loader",
                child, "Error processing box parameters");
          return false;
        }
        csOrthoTransform t;
        ParseTransform (child, t);
        body->AttachColliderBox (v, t, f, d, e, s);
        break;
      }
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }
  return true;
}

bool csPhysicsLoader::ParseSystemColliderMesh (
  iDocumentNode* node, iDynamicSystem* system, bool convex,
  iLoaderContext* ldr_context)
{
  float f = node->GetAttributeValueAsFloat ("friction");
  float e = node->GetAttributeValueAsFloat ("elasticity");
  float s = node->GetAttributeValueAsFloat ("softness");
  if (!node->GetContentsValue ()) { return false; }

  // Wait for load - assume it will exist eventually.
  csRef<iMeshWrapper> m = ldr_context->FindMeshObject
    (node->GetContentsValue ());

  if (m)
  {
    if (convex)
    {
      if( s > 0)
        system->AttachColliderConvexMesh (m, m->GetMovable()->GetTransform (), f, e, s);
      else  //no softness parameter, so use default
        system->AttachColliderConvexMesh (m, m->GetMovable()->GetTransform (), f, e);
    }
    else
    {
      if( s > 0)
        system->AttachColliderMesh (m, m->GetMovable()->GetTransform (), f, e, s);
      else  //no softness parameter, so use default
        system->AttachColliderMesh (m, m->GetMovable()->GetTransform (), f, e);
    }
  }
  else
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
      node, "Unable to find collider mesh in engine");
    return false;
  }
  return true;
}

static float GetFloat (iDocumentNode* node, const char* name, float def)
{
  csRef<iDocumentAttribute> attr = node->GetAttribute (name);
  if (!attr) return def;
  return attr->GetValueAsFloat ();
}

bool csPhysicsLoader::ParseSystemColliderPlane (iDocumentNode *node,
		iDynamicSystem* system)
{
  float f = GetFloat (node, "friction", 1.0f);
  float e = GetFloat (node, "elasticity", 0.0f);
  float s = GetFloat (node, "softness", 0.0f);
  csPlane3 plane;
  synldr->ParsePlane (node, plane);
  system->AttachColliderPlane (plane, f, e, s);
  return true;
}

bool csPhysicsLoader::ParseSystemColliderSphere (iDocumentNode* node,
		iDynamicSystem* system)
{
  float f = GetFloat (node, "friction", 1.0f);
  float e = GetFloat (node, "elasticity", 0.0f);
  float s = GetFloat (node, "softness", 0.0f);
  float r = node->GetAttributeValueAsFloat ("radius");
  csOrthoTransform t;
  ParseTransform (node, t);
  system->AttachColliderSphere (r, t.GetOrigin(), f, e, s);
  return true;
}


bool csPhysicsLoader::ParseSystemColliderCylinder (iDocumentNode* node,
		iDynamicSystem* system)
{
  float f = GetFloat (node, "friction", 1.0f);
  float e = GetFloat (node, "elasticity", 0.0f);
  float s = GetFloat (node, "softness", 0.0f);
  float l = node->GetAttributeValueAsFloat ("length");
  float r = node->GetAttributeValueAsFloat ("radius");
  csOrthoTransform t;
  ParseTransform (node, t);
  system->AttachColliderCylinder (l, r, t, f, e, s);
  return true;
}

bool csPhysicsLoader::ParseSystemColliderCapsule (iDocumentNode* node,
		iDynamicSystem* system)
{
  float f = GetFloat (node, "friction", 1.0f);
  float e = GetFloat (node, "elasticity", 0.0f);
  float s = GetFloat (node, "softness", 0.0f);
  float l = node->GetAttributeValueAsFloat ("length");
  float r = node->GetAttributeValueAsFloat ("radius");
  csOrthoTransform t;
  ParseTransform (node, t);
  system->AttachColliderCapsule (l, r, t, f, e, s);
  return true;
}

bool csPhysicsLoader::ParseSystemColliderBox (iDocumentNode* node,
		iDynamicSystem* system)
{
  float f = GetFloat (node, "friction", 1.0f);
  float e = GetFloat (node, "elasticity", 0.0f);
  float s = GetFloat (node, "softness", 0.0f);
  csVector3 v;
  if (!synldr->ParseVector (node, v))
  {
    synldr->ReportError ("crystalspace.dynamics.loader",
      node, "Error processing box parameters");
    return false;
  }
  csOrthoTransform t;
  ParseTransform (node, t);
  system->AttachColliderBox (v, t, f, e, s);
  return true;
}

bool csPhysicsLoader::ParseTransform (iDocumentNode* node, csOrthoTransform &t)
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

bool csPhysicsLoader::ParseJoint (iDocumentNode* node, iJoint* joint,
		iDynamicSystem* system)
{
  joint->SetTransConstraints (false, false, false);
  joint->SetRotConstraints (false, false, false);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  csOrthoTransform t;
  csRef<iRigidBody> body1 = 0;
  csRef<iRigidBody> body2 = 0;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_BODY:
        if (body1 == 0)
	{
	  const char* name = child->GetContentsValue ();
          if (name)
	  {
            body1 = system->FindBody (name);
	    if (!body1)
	    {
              synldr->ReportError ("crystalspace.dynamics.loader",
                child, "Can't find body with name %s!", CS::Quote::Single (name));
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
            body2 = system->FindBody (name);
	    if (!body2)
	    {
              synldr->ReportError ("crystalspace.dynamics.loader",
                child, "Can't find body with name %s!", CS::Quote::Single (name));
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
        csRef<iDocumentNodeIterator> it = child->GetNodes ();
        while (it->HasNext ())
        {
          csRef<iDocumentNode> child = it->Next ();
          if (child->GetType () != CS_NODE_ELEMENT) continue;
          const char *value = child->GetValue ();
          csStringID id = xmltokens.Request (value);
          switch (id)
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
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }
  joint->SetTransform (t);
  joint->Attach (body1, body2);
  return true;
}

bool csPhysicsLoader::ParseConstraint (iDocumentNode *node, bool &x,
		bool &y, bool &z, csVector3 &min, csVector3 &max)
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
