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
#include "iutil/objreg.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "ivaria/dynamics.h"
#include "physldr.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_SYSTEM,
  XMLTOKEN_GRAVITY,
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
  XMLTOKEN_COLLIDERMESH,
  XMLTOKEN_COLLIDERSPHERE,
  XMLTOKEN_RADIUS,
  XMLTOKEN_COLLIDERCYLINDER,
  XMLTOKEN_LENGTH,
  XMLTOKEN_COLLIDERBOX,
  XMLTOKEN_MOVE,
  XMLTOKEN_ROTATE,
  XMLTOKEN_JOINT,
  XMLTOKEN_CONSTRAINTS,
  XMLTOKEN_DISTANCE,
  XMLTOKEN_ANGLE,
  XMLTOKEN_CONSTRAINED,
  XMLTOKEN_FREE,
  XMLTOKEN_MIN,
  XMLTOKEN_MAX
};

SCF_IMPLEMENT_IBASE (csPhysicsLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPhysicsLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csPhysicsLoader)

SCF_EXPORT_CLASS_TABLE (physldr)
  SCF_EXPORT_CLASS (csPhysicsLoader, "crystalspace.dynamics.loader",
    "Crystal Space Dynamics Loader")
SCF_EXPORT_CLASS_TABLE_END

csPhysicsLoader::csPhysicsLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent)
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csPhysicsLoader::~csPhysicsLoader ()
{
}

bool csPhysicsLoader::Initialize (iObjectRegistry* object_reg)
{
  csPhysicsLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("system", XMLTOKEN_SYSTEM);
  xmltokens.Register ("gravity", XMLTOKEN_GRAVITY);
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
  xmltokens.Register ("collidermesh", XMLTOKEN_COLLIDERMESH);
  xmltokens.Register ("collidersphere", XMLTOKEN_COLLIDERSPHERE);
  xmltokens.Register ("collidersphere", XMLTOKEN_COLLIDERCYLINDER);
  xmltokens.Register ("colliderbox", XMLTOKEN_COLLIDERBOX);
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
  return true;
}

csPtr<iBase> csPhysicsLoader::Parse (iDocumentNode* node,
		iLoaderContext* /*ldr_context*/,
		iBase* /*context*/)
{
  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  CS_ASSERT (engine != NULL);
  csRef<iDynamics> dynamics = CS_QUERY_REGISTRY (object_reg, iDynamics);
  if (dynamics == NULL) {
    return NULL;
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    if (id == XMLTOKEN_SYSTEM) {
      csRef<iDynamicSystem> system = dynamics->CreateSystem ();
      if (!ParseSystem (child, system)) {
	    return NULL;
	  }
    } else {
      synldr->ReportBadToken (child);
      return NULL;
    }
  }
  return csPtr<iBase>(dynamics);
}

bool csPhysicsLoader::ParseSystem (iDocumentNode* node, iDynamicSystem* system)
{
  const char *name = node->GetAttributeValue ("name");
  system->QueryObject()->SetName (name);
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
        if (!synldr->ParseVector (child, v)) {
          synldr->ReportError ("crystalspace.dynamics.loader",
                child, "Error processing gravity token");
          return false;
        }
        system->SetGravity (v);
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
          if (id == XMLTOKEN_BODY) {
            csRef<iRigidBody> body = system->CreateBody ();
            group->AddBody (body);
            if (!ParseBody (child, body)) {
			  return false;
			}
          } else {
            synldr->ReportBadToken (child);
          }
        }
        break;
      }
      case XMLTOKEN_BODY:
      {
        csRef<iRigidBody> body = system->CreateBody ();
        if (!ParseBody (child, body)) {
		  return false;
		}
        break;
      }
      case XMLTOKEN_JOINT:
      {
        csRef<iJoint> joint = system->CreateJoint ();
        if (!ParseJoint (child, joint, system)) {
		  return false;
		}
        break;
      }
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }
  return true;
}

bool csPhysicsLoader::ParseBody (iDocumentNode* node, iRigidBody* body)
{
  const char *name = node->GetAttributeValue ("name");
  body->QueryObject()->SetName (name);
  float mass = node->GetAttributeValueAsFloat ("mass");

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id) {
      case XMLTOKEN_STATIC:
        body->MakeStatic ();
        break;
      case XMLTOKEN_MESH:
        body->MakeDynamic ();
        if (child->GetContentsValue ()) {
          iMeshWrapper *m = engine->FindMeshObject (child->GetContentsValue ());
          if (m) {
            body->AttachMesh (m);
          } else {
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
        if (!ParseCollider (child, body)) {
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

bool csPhysicsLoader::ParseCollider (iDocumentNode* node, iRigidBody* body)
{
  float f = node->GetAttributeValueAsFloat ("friction");
  float d = node->GetAttributeValueAsFloat ("density");
  float e = node->GetAttributeValueAsFloat ("elasticity");
   
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
        if (child->GetContentsValue ()) {
          iMeshWrapper *m = engine->FindMeshObject (child->GetContentsValue ());
          if (m) {
            body->AttachColliderMesh (m, m->GetMovable()->GetTransform (), f, d, e);
          } else {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Unable to find collider mesh in engine");
            return false;
          }
        }
        break;
      case XMLTOKEN_COLLIDERSPHERE:
      {
        float r = child->GetAttributeValueAsFloat ("radius");
        csOrthoTransform t;
        ParseTransform (child, t);
        body->AttachColliderSphere (r, t.GetOrigin(), f, d, e);
        break;
      }
      case XMLTOKEN_COLLIDERCYLINDER:
      {
        float l = child->GetAttributeValueAsFloat ("length");
        float r = child->GetAttributeValueAsFloat ("radius");
        csOrthoTransform t;
        ParseTransform (child, t);
        body->AttachColliderCylinder (l, r, t, f, d, e);
        break;
      }
      case XMLTOKEN_COLLIDERBOX:
      {
        csVector3 v;
        if (!synldr->ParseVector (child, v)) {
          synldr->ReportError ("crystalspace.dynamics.loader",
                child, "Error processing box parameters");
          return false;
        }
        csOrthoTransform t;
        ParseTransform (child, t);
        body->AttachColliderBox (v, t, f, d, e);
        break;
      }
      default:
        synldr->ReportBadToken (child);
        return false;
    }
  }
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

bool csPhysicsLoader::ParseJoint (iDocumentNode* node, iJoint* joint, iDynamicSystem* system)
{
  joint->SetTransConstraints (false, false, false); 
  joint->SetRotConstraints (false, false, false);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  csOrthoTransform t;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char *value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    csRef<iRigidBody> body1 = NULL;
    csRef<iRigidBody> body2 = NULL;
    switch (id)
    {
      case XMLTOKEN_BODY:
        if (body1 == NULL) {
          if (child->GetContentsValue ()) {
            body1 = system->FindBody (child->GetContentsValue());
          } else {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Body should have a name");
            return false;
          }
        } else if (body2 == NULL) {
          if (child->GetContentsValue ()) {
            body2 = system->FindBody (child->GetContentsValue());
          } else {
            synldr->ReportError ("crystalspace.dynamics.loader",
              child, "Body should have a name");
            return false;
          }
          joint->Attach (body1, body2);
        } else {
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
  return true;
}

bool csPhysicsLoader::ParseConstraint (iDocumentNode *node, bool &x, bool &y, bool &z, csVector3 &min, csVector3 &max)
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
