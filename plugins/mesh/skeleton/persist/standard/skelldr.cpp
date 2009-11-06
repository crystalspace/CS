/*
    Copyright (C) 2006 by Hristo Hristov

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

#include "csgeom/math3d.h"
#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector4.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "imesh/skeleton.h"

#include "skelldr.h"



enum
{
  XMLTOKEN_BONE = 1,
  XMLTOKEN_MOVE,
  XMLTOKEN_SKINBOX,
  XMLTOKEN_MIN,
  XMLTOKEN_MAX,
  XMLTOKEN_SCRIPT,
  XMLTOKEN_FRAME,
  XMLTOKEN_SOCKET,
  XMLTOKEN_RAGDOLL,
  XMLTOKEN_GEOM,
  XMLTOKEN_BOX,
  XMLTOKEN_SPHERE,
  XMLTOKEN_CYLINDER,
  XMLTOKEN_FRICTION,
  XMLTOKEN_ELASTICITY,
  XMLTOKEN_SOFTNESS,
  XMLTOKEN_SLIP,
  XMLTOKEN_BODY,
  XMLTOKEN_MASS,
  XMLTOKEN_GRAVMODE,
  XMLTOKEN_ROTCONSTRAINTS,
  XMLTOKEN_TRANSCONSTRAINTS,
  XMLTOKEN_JOINT,
  XMLTOKEN_ATTACHTOPARENT,
  XMLTOKEN_DISABLED,
  XMLTOKEN_LOOP,
  XMLTOKEN_RELATIVE
};

SCF_IMPLEMENT_FACTORY (csSkeletonFactoryLoader)

csSkeletonFactoryLoader::csSkeletonFactoryLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csSkeletonFactoryLoader::~csSkeletonFactoryLoader ()
{
}

bool csSkeletonFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSkeletonFactoryLoader::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  xmltokens.Register ("bone", XMLTOKEN_BONE);
  xmltokens.Register ("move", XMLTOKEN_MOVE);
  xmltokens.Register ("skinbox", XMLTOKEN_SKINBOX);
  xmltokens.Register ("min", XMLTOKEN_MIN);
  xmltokens.Register ("max", XMLTOKEN_MAX);
  xmltokens.Register ("script", XMLTOKEN_SCRIPT);
  xmltokens.Register ("frame", XMLTOKEN_FRAME);
  xmltokens.Register ("loop", XMLTOKEN_LOOP);
  xmltokens.Register ("socket", XMLTOKEN_SOCKET);
  xmltokens.Register ("ragdoll", XMLTOKEN_RAGDOLL);
  xmltokens.Register ("geom", XMLTOKEN_GEOM);
  xmltokens.Register ("box", XMLTOKEN_BOX);
  xmltokens.Register ("sphere", XMLTOKEN_SPHERE);
  xmltokens.Register ("cylinder", XMLTOKEN_CYLINDER);
  xmltokens.Register ("friction", XMLTOKEN_FRICTION);
  xmltokens.Register ("elasticity", XMLTOKEN_ELASTICITY);
  xmltokens.Register ("softness", XMLTOKEN_SOFTNESS);
  xmltokens.Register ("slip", XMLTOKEN_SLIP);
  xmltokens.Register ("body", XMLTOKEN_BODY);
  xmltokens.Register ("mass", XMLTOKEN_MASS);
  xmltokens.Register ("gravmode", XMLTOKEN_GRAVMODE);
  xmltokens.Register ("rotconstraints", XMLTOKEN_ROTCONSTRAINTS);
  xmltokens.Register ("transconstraints", XMLTOKEN_TRANSCONSTRAINTS);
  xmltokens.Register ("joint", XMLTOKEN_JOINT);
  xmltokens.Register ("attachtoparent", XMLTOKEN_ATTACHTOPARENT);
  xmltokens.Register ("disabled", XMLTOKEN_DISABLED);
  xmltokens.Register ("relative", XMLTOKEN_RELATIVE);

  return true;
}

const char *csSkeletonFactoryLoader::ParseBone (iDocumentNode* node, 
  iSkeletonFactory *skel_fact, iSkeletonBoneFactory *parent_bone)
{

  const char* bonename = node->GetAttributeValue ("name");

  if (!bonename)
    return "Name of the bone is missing!";

  iSkeletonBoneFactory *bone = skel_fact->CreateBone(bonename);
  bone->SetParent(parent_bone);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MOVE:
        {
          csRef<iSyntaxService> SyntaxService = csQueryRegistry<iSyntaxService> (object_reg);
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
            csVector3 v;
            if (!SyntaxService->ParseVector (vector_node, v))
              return false;
            bone->GetTransform ().SetOrigin (v);
          }

          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
            csMatrix3 m;
            if (!SyntaxService->ParseMatrix (matrix_node, m))
              return false;
            bone->GetTransform ().SetO2T (m);
          }
        }
        break;
      case XMLTOKEN_BONE:
        {
          const char* err = ParseBone (child, skel_fact, bone);
          if (err != 0) return err;
        }
        break;

    case XMLTOKEN_SOCKET:
    {
      const char *socket_name = child->GetAttributeValue ("name");
      iSkeletonSocketFactory *socket = skel_fact->CreateSocket(socket_name, bone);

      csRef<iDocumentNodeIterator> it = child->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        if (child->GetType () != CS_NODE_ELEMENT) continue;
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);
        switch (id)
        {
          case XMLTOKEN_MOVE:
          {
            csReversibleTransform socket_transform;
            csRef<iSyntaxService> SyntaxService = csQueryRegistry<iSyntaxService> (object_reg);
            csRef<iDocumentNode> vector_node = child->GetNode ("v");
            if (vector_node)
            {
              csVector3 v;
              if (!SyntaxService->ParseVector (vector_node, v))
              return false;
              socket_transform.SetOrigin (v);
            }
    
            csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
            if (matrix_node)
            {
              csMatrix3 m;
              if (!SyntaxService->ParseMatrix (matrix_node, m))
              return false;
              socket_transform.SetO2T (m);
            }
            socket->SetTransform(socket_transform);
          }
          break;
        }
      }
    }
    break;

    case XMLTOKEN_SKINBOX:
    {
      csBox3 box;
      csRef<iDocumentNodeIterator> it = child->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        if (child->GetType () != CS_NODE_ELEMENT) continue;
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);
        switch (id)
        {
          case XMLTOKEN_MIN:
          {
            box.SetMin(0, child->GetAttributeValueAsFloat("x"));
            box.SetMin(1, child->GetAttributeValueAsFloat("y"));
            box.SetMin(2, child->GetAttributeValueAsFloat("z"));
          }
          break;
          case XMLTOKEN_MAX:
          {
            box.SetMax(0, child->GetAttributeValueAsFloat("x"));
            box.SetMax(1, child->GetAttributeValueAsFloat("y"));
            box.SetMax(2, child->GetAttributeValueAsFloat("z"));
          }
          break;
        }
      }
      bone->SetSkinBox(box);
    }
    break;

    case XMLTOKEN_RAGDOLL:
    {
      csRef<iDocumentNodeIterator> it = child->GetNodes ();
      while (it->HasNext ())
      {
        csRef<iDocumentNode> child = it->Next ();
        if (child->GetType () != CS_NODE_ELEMENT) continue;
        const char* value = child->GetValue ();
        csStringID id = xmltokens.Request (value);
        switch (id)
        {
          case XMLTOKEN_DISABLED:
          {
            bone->GetRagdollInfo()->SetEnabled(false);
          }
          break;
          case XMLTOKEN_GEOM:
          {
            const char *name = child->GetAttributeValue("name");
            bone->GetRagdollInfo()->SetGeomName(name);
            csRef<iDocumentNodeIterator> it = child->GetNodes ();
            while (it->HasNext ())
            {
              csRef<iDocumentNode> child = it->Next ();
              if (child->GetType () != CS_NODE_ELEMENT) continue;
              const char* value = child->GetValue ();
              csStringID id = xmltokens.Request (value);
              switch (id)
              {
                case XMLTOKEN_BOX:
                {
                  bone->GetRagdollInfo()->SetGeomType(CS_BGT_BOX);
                  csRef<iDocumentNodeIterator> it = child->GetNodes ();
                  while (it->HasNext ())
                  {
                    csRef<iDocumentNode> child = it->Next ();
                    if (child->GetType () != CS_NODE_ELEMENT) continue;
                    const char* value = child->GetValue ();
                    csStringID id = xmltokens.Request (value);
                    switch (id)
                    {
                      case XMLTOKEN_SKINBOX:
                      {
                        csVector3 size = bone->GetSkinBox().GetSize();
                        bone->GetRagdollInfo()->SetGeomDimensions(size);
                      }
                      break;
                    }
                  }
                }
                break;
                case XMLTOKEN_SPHERE:
                {
                  //TODO
                }
                break;
                case XMLTOKEN_CYLINDER:
                {
                  //TODO
                }
                break;
                case XMLTOKEN_FRICTION:
                {
                  bone->GetRagdollInfo()->SetFriction(child->GetContentsValueAsFloat());
                }
                break;
                case XMLTOKEN_ELASTICITY:
                {
                  bone->GetRagdollInfo()->SetElasticity(child->GetContentsValueAsFloat());
                }
                break;
                case XMLTOKEN_SOFTNESS:
                {
                  bone->GetRagdollInfo()->SetSoftness(child->GetContentsValueAsFloat());
                }
                break;
                case XMLTOKEN_SLIP:
                {
                  bone->GetRagdollInfo()->SetSlip(child->GetContentsValueAsFloat());
                }
                break;
              }
            }
          }
          break;
          case XMLTOKEN_BODY:
          {
            const char *name = child->GetAttributeValue("name");
            bone->GetRagdollInfo()->SetBodyName(name);
            csRef<iDocumentNodeIterator> it = child->GetNodes ();
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
                  bone->GetRagdollInfo()->SetBodyMass(child->GetContentsValueAsFloat());
                }
                break;
                case XMLTOKEN_GRAVMODE:
                {
                  bone->GetRagdollInfo()->SetBodyGravmode(child->GetContentsValueAsInt());
                }
                break;
              }
            }
          }
          break;
          case XMLTOKEN_JOINT:
          {
            const char *name = child->GetAttributeValue("name");
            bone->GetRagdollInfo()->SetJointName(name);
            csRef<iDocumentNodeIterator> it = child->GetNodes ();
            while (it->HasNext ())
            {
              csRef<iDocumentNode> child = it->Next ();
              if (child->GetType () != CS_NODE_ELEMENT) continue;
              const char* value = child->GetValue ();
              csStringID id = xmltokens.Request (value);
              switch (id)
              {
                case XMLTOKEN_ROTCONSTRAINTS:
                {
                  csRef<iDocumentNodeIterator> it = child->GetNodes ();
                  while (it->HasNext ())
                  {
                    csRef<iDocumentNode> child = it->Next ();
                    if (child->GetType () != CS_NODE_ELEMENT) continue;
                    const char* value = child->GetValue ();
                    csStringID id = xmltokens.Request (value);
                    switch (id)
                    {
                      case XMLTOKEN_MIN:
                      {
                        csVector3 constraints = csVector3(child->GetAttributeValueAsFloat("x"),
                                        child->GetAttributeValueAsFloat("y"),
                                        child->GetAttributeValueAsFloat("z"));
                        bone->GetRagdollInfo()->SetJointMinRotContraints(constraints);
                      }
                      break;
                      case XMLTOKEN_MAX:
                      {
                        csVector3 constraints = csVector3(child->GetAttributeValueAsFloat("x"),
                                        child->GetAttributeValueAsFloat("y"),
                                        child->GetAttributeValueAsFloat("z"));
                        bone->GetRagdollInfo()->SetJointMaxRotContraints(constraints);
                      }
                      break;
                    }
                  }
                }
                break;
                case XMLTOKEN_TRANSCONSTRAINTS:
                {
                  csRef<iDocumentNodeIterator> it = child->GetNodes ();
                  while (it->HasNext ())
                  {
                    csRef<iDocumentNode> child = it->Next ();
                    if (child->GetType () != CS_NODE_ELEMENT) continue;
                    const char* value = child->GetValue ();
                    csStringID id = xmltokens.Request (value);
                    switch (id)
                    {
                      case XMLTOKEN_MIN:
                      {
                        csVector3 constraints = csVector3(child->GetAttributeValueAsFloat("x"),
                                        child->GetAttributeValueAsFloat("y"),
                                        child->GetAttributeValueAsFloat("z"));
                        bone->GetRagdollInfo()->SetJointMinTransContraints(constraints);
                      }
                      break;
                      case XMLTOKEN_MAX:
                      {
                        csVector3 constraints = csVector3(child->GetAttributeValueAsFloat("x"),
                                        child->GetAttributeValueAsFloat("y"),
                                        child->GetAttributeValueAsFloat("z"));
                        bone->GetRagdollInfo()->SetJointMaxTransContraints(constraints);
                      }
                      break;
                    }
                  }
                }
                break;
              }
            }
          }
          break;
          case XMLTOKEN_ATTACHTOPARENT:
          {
            bone->GetRagdollInfo()->SetAttachToParent(true);
          }
          break;
        }
      }
    }
    break;
    default: return 0;
  }
  }
  return 0;
}

const char *csSkeletonFactoryLoader::ParseScript (iDocumentNode* node, 
  iSkeletonFactory *skel_fact)
{

  const char* script_name = node->GetAttributeValue ("name");

  if (!script_name)
    return "Name of the script is missing!";

  iSkeletonAnimation *script = skel_fact->CreateAnimation (script_name);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LOOP:
        {
      script->SetLoop(true);
        }
        break;
      case XMLTOKEN_FRAME:
        {
      ParseFrame(child, skel_fact, script);
        }
        break;
  }
  }
  script->RecalcSpline();
  return 0;
}

const char *csSkeletonFactoryLoader::ParseFrame (iDocumentNode* node, 
  iSkeletonFactory *skel_fact, iSkeletonAnimation *script)
{
  const char* frame_name = node->GetAttributeValue ("name");
  if (!frame_name)
  {
    frame_name = "";
  }
  int duration = node->GetAttributeValueAsInt ("duration");

  iSkeletonAnimationKeyFrame *frame = script->CreateFrame(frame_name);
  frame->SetDuration(duration);

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
        {
      const char *bone_name = child->GetAttributeValue ("name");
      csReversibleTransform key_transform;
	  bool relative = false;

      iSkeletonBoneFactory *bone_fact = skel_fact->FindBone(bone_name);
      if (!bone_fact)
      {
      continue;
      }
      csRef<iDocumentNodeIterator> it = child->GetNodes ();
      while (it->HasNext ())
      {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
        case XMLTOKEN_RELATIVE:
        {
          relative = true;
        }
        break;
        case XMLTOKEN_MOVE:
        {
          csRef<iSyntaxService> SyntaxService = csQueryRegistry<iSyntaxService> (object_reg);
          csRef<iDocumentNode> vector_node = child->GetNode ("v");
          if (vector_node)
          {
          csVector3 v;
          if (!SyntaxService->ParseVector (vector_node, v))
            return false;
          key_transform.SetOrigin (v);
          }

          csRef<iDocumentNode> matrix_node = child->GetNode ("matrix");
          if (matrix_node)
          {
          csMatrix3 m;
          if (!SyntaxService->ParseMatrix (matrix_node, m))
            return false;
          key_transform.SetO2T (m);
          }
        }
        break;
      }
      }
        frame->AddTransform(bone_fact, key_transform, relative);
        }
        break;
    }
  }
  return 0;
}

csPtr<iBase> csSkeletonFactoryLoader::Parse (iDocumentNode* node,
  iStreamSource*, iLoaderContext* /*ldr_context*/, iBase* /*context*/)
{
  csRef<iSkeletonGraveyard> graveyard = csLoadPluginCheck<iSkeletonGraveyard> (
  	object_reg, "crystalspace.graveyard", false);
  if (!graveyard)
  {
    synldr->ReportError (
    "crystalspace.skelfactoryloader.setup.objecttype",
    node, "Could not load the graveyard plugin!");
    return 0;
  }

  csRef<iDocumentNode> skelfact_node = node->GetNode("skelfact");

  const char* fact_name = skelfact_node->GetAttributeValue ("name");
  iSkeletonFactory *skel_fact = graveyard->CreateFactory (fact_name);

  csRef<iDocumentNodeIterator> it = skelfact_node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_BONE:
        ParseBone (child, skel_fact, 0);
        break;
      case XMLTOKEN_SCRIPT:
        ParseScript (child, skel_fact);
        break;
    }
  }

  skel_fact->IncRef ();
  return csPtr<iBase> ((iBase*)skel_fact);
}
