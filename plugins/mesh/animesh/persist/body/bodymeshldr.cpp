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

#include "csutil/ref.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/plugin.h"
#include "imap/ldrctxt.h"
#include "iengine/mesh.h"
#include "imesh/bodymesh.h"
#include "csgeom/sphere.h"

#include "bodymeshldr.h"

CS_PLUGIN_NAMESPACE_BEGIN(BodyMeshLdr)
{
  SCF_IMPLEMENT_FACTORY(BodyMeshLoader);

  BodyMeshLoader::BodyMeshLoader (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  static const char* msgid = "crystalspace.mesh.loader.animesh.body";

  csPtr<iBase> BodyMeshLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context,
    iBase* context)
  {
    if (!bodyManager)
    {
      synldr->ReportError (msgid, node, "Couldn't get any body mesh system");
      return false;
    }

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_SKELETON:
        if (!ParseSkeleton (child, ldr_context))
          return false;
        break;

      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return csPtr<iBase> (bodyManager);
  }

  bool BodyMeshLoader::Initialize (iObjectRegistry* objReg)
  {
    object_reg = objReg;

    synldr = csQueryRegistry<iSyntaxService> (object_reg);

    bodyManager = csQueryRegistryOrLoad<CS::Animation::iBodyManager> (object_reg,
      "crystalspace.mesh.animesh.body");

    InitTokenTable (xmltokens);
    return true;
  }

  bool BodyMeshLoader::ParseSkeleton (iDocumentNode* node,
				      iLoaderContext* ldr_context)
  {
    // read name
    const char* name = node->GetAttributeValue ("name");
    if (!name)
    {
      synldr->ReportError (msgid, node, "No name set for skeleton");
      return false;
    }

    // read animesh factory
    const char* factoryName = node->GetAttributeValue ("skeletonfact");
    if (!factoryName)
    {
      synldr->ReportError (msgid, node, "No animesh factory specified");
      return false;
    }

    csRef<CS::Animation::iSkeletonManager> skeletonManager =
      csQueryRegistry<CS::Animation::iSkeletonManager> (object_reg);
    if (!skeletonManager)
    {
      synldr->ReportError (msgid, node, "No animesh skeleton manager");
      return false;
    }

    CS::Animation::iSkeletonFactory* skeletonFactory =
      skeletonManager->FindSkeletonFactory (factoryName);
    if (!skeletonFactory)
    {
      synldr->ReportError (msgid, node, "No animesh skeleton factory named %s",
			   factoryName);
      return false;
    }

    // create skeleton
    CS::Animation::iBodySkeleton* skeleton = bodyManager->CreateBodySkeleton (name, skeletonFactory);

    // parse child nodes
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_BONE:
        if (!ParseBone (child, ldr_context, skeleton))
          return false;
        break;

      case XMLTOKEN_CHAIN:
        if (!ParseChain (child, skeleton))
          return false;
        break;

      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool BodyMeshLoader::ParseBone (iDocumentNode* node, iLoaderContext* ldr_context,
				  CS::Animation::iBodySkeleton* skeleton)
  {
    // parse bone name
    const char* name = node->GetAttributeValue ("name");
    if (!name)
    {
      synldr->ReportError (msgid, node, "No name set for bone");
      return false;
    }

    CS::Animation::BoneID id = skeleton->GetSkeletonFactory ()->FindBone (name);
    if (id == CS::Animation::InvalidBoneID)
    {
      synldr->ReportError (msgid, node, "No bone with name %s in skeleton factory",
			   name);
      return false;
    }

    // create body bone
    CS::Animation::iBodyBone* bone = skeleton->CreateBodyBone (id);

    // parse child nodes
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_PROPERTIES:
        if (!ParseProperties (child, bone))
          return false;
        break;

      case XMLTOKEN_COLLIDERS:
        if (!ParseColliders (child, ldr_context, bone))
          return false;
        break;

      case XMLTOKEN_JOINT:
        if (!ParseJoint (child, bone))
          return false;
        break;

      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool BodyMeshLoader::ParseProperties (iDocumentNode* node, CS::Animation::iBodyBone* bone)
  {
    CS::Animation::iBodyBoneProperties* properties = bone->CreateBoneProperties ();

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_MASS:
	{
	  float mass = child->GetAttributeValueAsFloat ("value");
	  properties->SetMass (mass);
	  break;
	}

      case XMLTOKEN_CENTER:
	{
	  csVector3 center;
	  if (!synldr->ParseVector (child, center))
          {
            synldr->ReportError (msgid, child, "Couldn't parse vector");
            return false;
          }
	  properties->SetCenter (center);
	  break;
	}

      case XMLTOKEN_INERTIA:
	{
	  csMatrix3 inertia;
	  if (!synldr->ParseMatrix (child, inertia))
          {
            synldr->ReportError (msgid, child, "Couldn't parse matrix");
            return false;
          }

	  properties->SetInertia (inertia);
	  break;
	}

      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool BodyMeshLoader::ParseColliders (iDocumentNode* node,
				       iLoaderContext* ldr_context, CS::Animation::iBodyBone* bone)
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
	case XMLTOKEN_COLLIDERMESH:
	case XMLTOKEN_COLLIDERCONVEXMESH:
	  {
	    if (!child->GetAttributeValue ("mesh"))
	    {
	      synldr->ReportError (msgid, child,
				   "No mesh specified for collidermesh");
	      return false;
	    }

	    // try to find a mesh factory
	    csRef<iMeshWrapper> mesh;
	    csRef<iMeshFactoryWrapper> meshFactory = ldr_context->FindMeshFactory
	      (child->GetAttributeValue ("mesh"));
	    if (meshFactory)
	      mesh = meshFactory->CreateMeshWrapper ();

	    // try to find a mesh
	    else
	    {
	      mesh = ldr_context->FindMeshObject (child->GetAttributeValue ("mesh"));
	      if (!mesh)
		{
		  synldr->ReportError (msgid, child,
				       "Unable to find mesh or factory '%s' while loading collider",
				       child->GetAttributeValue ("mesh"));
		  return false;
		}
	    }

	    // create collider
	    CS::Animation::iBodyBoneCollider* collider = bone->CreateBoneCollider ();
	    if (id == XMLTOKEN_COLLIDERMESH)
	      collider->SetMeshGeometry (mesh);
	    else
	      collider->SetConvexMeshGeometry (mesh);

	    // parse params
	    ParseColliderParams (child, collider);

	    break;
 	  }

	case XMLTOKEN_COLLIDERBOX:
	  {
	    csVector3 v;
	    if (!synldr->ParseVector (child, v))
	    {
	      synldr->ReportError (msgid, child, "Error processing box parameters");
	      return false;
	    }

	    CS::Animation::iBodyBoneCollider* collider = bone->CreateBoneCollider ();
	    collider->SetBoxGeometry (v);
	    ParseColliderParams (child, collider);

	    break;
	  }

	case XMLTOKEN_COLLIDERSPHERE:
	  {
	    float r = child->GetAttributeValueAsFloat ("radius");
	    csVector3 v;
	    if (!synldr->ParseVector (child, v))
	    {
	      synldr->ReportError (msgid, child, "Error processing box parameters");
	      return false;
	    }

	    CS::Animation::iBodyBoneCollider* collider = bone->CreateBoneCollider ();
	    collider->SetSphereGeometry (csSphere (v, r));
	    ParseColliderParams (child, collider);

	    break;
	  }

	case XMLTOKEN_COLLIDERCYLINDER:
	  {
	    float l = child->GetAttributeValueAsFloat ("length");
	    float r = child->GetAttributeValueAsFloat ("radius");

	    CS::Animation::iBodyBoneCollider* collider = bone->CreateBoneCollider ();
	    collider->SetCylinderGeometry (l, r);
	    ParseColliderParams (child, collider);

	    break;
	  }

	case XMLTOKEN_COLLIDERCAPSULE:
	  {
	    float l = child->GetAttributeValueAsFloat ("length");
	    float r = child->GetAttributeValueAsFloat ("radius");

	    CS::Animation::iBodyBoneCollider* collider = bone->CreateBoneCollider ();
	    collider->SetCapsuleGeometry (l, r);
	    ParseColliderParams (child, collider);

	    break;
	  }

	case XMLTOKEN_COLLIDERPLANE:
	  {
	    csPlane3 plane;
	    synldr->ParsePlane (node, plane);

	    CS::Animation::iBodyBoneCollider* collider = bone->CreateBoneCollider ();
	    collider->SetPlaneGeometry (plane);
	    ParseColliderParams (child, collider);

	    break;
	  }

	default:
	  synldr->ReportBadToken (child);
	  return false;
	}
    }

    return true;
  }

  bool BodyMeshLoader::ParseColliderParams (iDocumentNode* node,
					    CS::Animation::iBodyBoneCollider* collider)
  {
    if (node->GetAttribute ("density"))
      collider->SetDensity (node->GetAttributeValueAsFloat ("density"));
    if (node->GetAttribute ("friction"))
      collider->SetFriction (node->GetAttributeValueAsFloat ("friction"));
    if (node->GetAttribute ("elasticity"))
      collider->SetElasticity (node->GetAttributeValueAsFloat ("elasticity"));
    if (node->GetAttribute ("softness"))
      collider->SetSoftness (node->GetAttributeValueAsFloat ("softness"));

    csOrthoTransform t;
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

	default:
	  synldr->ReportBadToken (child);
	  return false;
	}
    }

    collider->SetTransform (t);

    return true;
  }

  bool BodyMeshLoader::ParseJoint (iDocumentNode* node, CS::Animation::iBodyBone* bone)
  {
    CS::Animation::iBodyBoneJoint* joint = bone->CreateBoneJoint ();

    csOrthoTransform t;
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char *value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
	{
	case XMLTOKEN_BOUNCE:
	  {
	    csVector3 v;
	    if (!synldr->ParseVector (child, v))
	    {
	      synldr->ReportError (msgid, child, "Couldn't parse vector");
	      return false;
	    }
	    joint->SetBounce (v);
	    break;
	  }

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

    // TODO: add transform in CS::Animation::iBodyBoneJoint
    //joint->SetTransform (t);

    return true;
  }

  bool BodyMeshLoader::ParseConstraint (iDocumentNode *node, bool &x,
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

  bool BodyMeshLoader::ParseChain (iDocumentNode* node, CS::Animation::iBodySkeleton* skeleton)
  {
    // TODO
    return true;
  }

}
CS_PLUGIN_NAMESPACE_END(BodyMeshLdr)
