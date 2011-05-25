/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/animesh.h"
#include "imesh/animnode/skeleton2anim.h"
#include "imesh/skeleton2.h"
#include "imesh/object.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"

#include "csutil/ref.h"

#include "animeshldr.h"

CS_PLUGIN_NAMESPACE_BEGIN(Animeshldr)
{
  SCF_IMPLEMENT_FACTORY(AnimeshFactoryLoader);
  SCF_IMPLEMENT_FACTORY(AnimeshObjectLoader);
  SCF_IMPLEMENT_FACTORY(AnimeshFactorySaver);
  SCF_IMPLEMENT_FACTORY(AnimeshObjectSaver);


  AnimeshFactoryLoader::AnimeshFactoryLoader (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  static const char* msgidFactory = "crystalspace.animeshfactoryloader";

  csPtr<iBase> AnimeshFactoryLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context,
    iBase* context)
  {
    csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
      object_reg, "crystalspace.mesh.object.animesh", false);

    if (!type)
    {
      synldr->ReportError (msgidFactory, node, 
        "Could not load the animesh object plugin!");
      return 0;
    }

    // Create a factory
    csRef<iMeshObjectFactory> fact = type->NewFactory ();
    csRef<CS::Mesh::iAnimatedMeshFactory> amfact = 
      scfQueryInterfaceSafe<CS::Mesh::iAnimatedMeshFactory> (fact);

    if (!amfact)
    {
      synldr->ReportError (msgidFactory, node, 
        "Could not load the animesh object plugin!");
      return 0;
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
      case XMLTOKEN_MATERIAL:
        {
          const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
          if (!mat)
          {
            synldr->ReportError (msgidFactory, child, "Couldn't find material %s!", 
              CS::Quote::Single (matname));
            return 0;
          }
          fact->SetMaterialWrapper (mat);
        }
        break;
      case XMLTOKEN_MIXMODE:
        {
          uint mm;
          if (!synldr->ParseMixmode (child, mm))
            return 0;
          fact->SetMixMode (mm);
        }
        break;      
      case XMLTOKEN_VERTEX:
        {
          csRef<iRenderBuffer> rb = synldr->ParseRenderBuffer (child);
          if (!rb)
          {
            synldr->ReportError (msgidFactory, child, "Could not parse render buffer!");
            return 0;
          }
          amfact->SetVertices (rb);
        }
        break;
      case XMLTOKEN_TEXCOORD:
        {
          csRef<iRenderBuffer> rb = synldr->ParseRenderBuffer (child);
          if (!rb)
          {
            synldr->ReportError (msgidFactory, child, "Could not parse render buffer!");
            return 0;
          }
          amfact->SetTexCoords (rb);
        }
        break;
      case XMLTOKEN_NORMAL:
        {
          csRef<iRenderBuffer> rb = synldr->ParseRenderBuffer (child);
          if (!rb)
          {
            synldr->ReportError (msgidFactory, child, "Could not parse render buffer!");
            return 0;
          }
          amfact->SetNormals (rb);
        }
        break;
      case XMLTOKEN_TANGENT:
        {
          csRef<iRenderBuffer> rb = synldr->ParseRenderBuffer (child);
          if (!rb)
          {
            synldr->ReportError (msgidFactory, child, "Could not parse render buffer!");
            return 0;
          }
          amfact->SetTangents (rb);
        }
        break;
      case XMLTOKEN_BINORMAL:
        {
          csRef<iRenderBuffer> rb = synldr->ParseRenderBuffer (child);
          if (!rb)
          {
            synldr->ReportError (msgidFactory, child, "Could not parse render buffer!");
            return 0;
          }
          amfact->SetBinormals (rb);
        }
        break;
      case XMLTOKEN_TANGENTS:
        {
	  bool automatic = child->GetAttributeValueAsBool ("automatic", false);
	  if (automatic)
	    amfact->ComputeTangents ();
        }
        break;
      case XMLTOKEN_COLOR:
        {
          csRef<iRenderBuffer> rb = synldr->ParseRenderBuffer (child);
          if (!rb)
          {
            synldr->ReportError (msgidFactory, child, "Could not parse render buffer!");
            return 0;
          }
          amfact->SetColors (rb);
        }
        break;
      case XMLTOKEN_BONEINFLUENCES:
        {
          int wantedPerVertex = child->GetAttributeValueAsInt ("bonespervertex");
          if (!wantedPerVertex)
            amfact->SetBoneInfluencesPerVertex (wantedPerVertex);

          int realPerVertex = amfact->GetBoneInfluencesPerVertex ();
          int numVerts = amfact->GetVertexCount ();
          int currInfl = 0;

          CS::Mesh::AnimatedMeshBoneInfluence* bi = amfact->GetBoneInfluences ();

          csRef<iDocumentNodeIterator> it = child->GetNodes ();
          while (it->HasNext ())
          {
            csRef<iDocumentNode> child2 = it->Next ();
            if (child2->GetType () != CS_NODE_ELEMENT) continue;
            const char* value = child2->GetValue ();
            csStringID id = xmltokens.Request (value);
            switch (id)
            {
            case XMLTOKEN_BI:
              {
                if (currInfl > numVerts*realPerVertex)
                {
                  synldr->ReportError (msgidFactory, child, 
                    "Too many bone vertex influences %d, expected %d", currInfl, numVerts*realPerVertex);
                  return 0;
                }

                bi[currInfl].bone = child2->GetAttributeValueAsInt("bone");
                bi[currInfl].influenceWeight = child2->GetAttributeValueAsFloat ("weight");
                currInfl++;
              }
              break;
            default:
              synldr->ReportBadToken (child2);
              return 0;
            }
          }
        }
        break;
      case XMLTOKEN_SUBMESH:
        {
          // Handle submesh
          csRef<iRenderBuffer> indexBuffer;
          
          csRef<iMaterialWrapper> material;

          csRef<iDocumentNodeIterator> it = child->GetNodes ();
          while (it->HasNext ())
          {
            csRef<iDocumentNode> child2 = it->Next ();
            if (child2->GetType () != CS_NODE_ELEMENT) continue;
            const char* value = child2->GetValue ();
            csStringID id = xmltokens.Request (value);
            switch (id)
            {
            case XMLTOKEN_INDEX:
              {
                indexBuffer = synldr->ParseRenderBuffer (child2);
                if (!indexBuffer)
                {
                  synldr->ReportError (msgidFactory, child2, "Could not parse render buffer!");
                  return 0;
                }
              }
              break;
            case XMLTOKEN_MATERIAL:
              {
                const char* matname = child2->GetContentsValue ();
                material = ldr_context->FindMaterial (matname);
                if (!material)
                {
                  synldr->ReportError (msgidFactory, child2, "Couldn't find material %s!", 
                    CS::Quote::Single (matname));
                  return 0;
                }
              }
              break;
            default:
              synldr->ReportBadToken (child2);
              return 0;
            }
          }

          if (indexBuffer)
          {
            CS::Mesh::iAnimatedMeshSubMeshFactory* smf = amfact->CreateSubMesh (indexBuffer,
              child->GetAttributeValue("name"), child->GetAttributeValueAsBool("visible", true));
            smf->SetMaterial(material);
          }
        }
        break;
      case XMLTOKEN_SKELETON:
        {
          if (!skelMgr)
          {
            skelMgr = csQueryRegistry<CS::Animation::iSkeletonManager> (object_reg);

            if (!skelMgr)
            {
              synldr->ReportError (msgidFactory, child, "Could not find any loaded skeletons");
              return 0;
            }
          }
          
          const char* skelName = child->GetContentsValue ();
          CS::Animation::iSkeletonFactory* skelFact = skelMgr->FindSkeletonFactory (skelName);
          if (!skelFact)
          {
            synldr->ReportError (msgidFactory, child, "Could not find skeleton %s", skelName);
            return 0;
          }

          amfact->SetSkeletonFactory (skelFact);
        }
        break;
      case XMLTOKEN_MORPHTARGET:
        if (!ParseMorphTarget (child, amfact))
	  return 0;
	break;
      case XMLTOKEN_BBOXES:
        if (!ParseBoundingBoxes (child, amfact))
	  return 0;
	break;
      case XMLTOKEN_SOCKET:
        {
	  CS::Animation::iSkeletonFactory* skelFact = amfact->GetSkeletonFactory ();
          if (!skelFact)
          {
            synldr->ReportError (msgidFactory, child, "No skeleton defined while creating socket");
            return 0;
          }

          const char* boneName = child->GetAttributeValue ("bone");
	  CS::Animation::BoneID bone = skelFact->FindBone (boneName);
          if (bone == CS::Animation::InvalidBoneID)
          {
            synldr->ReportError (msgidFactory, child, "Could not find bone %s in skeleton", boneName);
            return 0;
          }

          const char* name = child->GetAttributeValue ("name");
          csReversibleTransform transform;

          csRef<iDocumentNode> tnode = child->GetNode ("transform");
          if (tnode)
          {
            csRef<iDocumentNode> vnode = tnode->GetNode ("vector");
            if (vnode)
            {
              csVector3 v;
              synldr->ParseVector (vnode, v);
              transform.SetOrigin (v);
            }
          
            csRef<iDocumentNode> mnode = tnode->GetNode ("matrix");
            if (mnode)
            {
              csMatrix3 m;
              synldr->ParseMatrix (mnode, m);
              transform.SetO2T (m);
            }
          }
          
          amfact->CreateSocket (bone, transform, name);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }

    // Recalc stuff
    amfact->Invalidate ();

    return csPtr<iBase> (fact);
  }

  bool AnimeshFactoryLoader::ParseMorphTarget (iDocumentNode* child,
					       CS::Mesh::iAnimatedMeshFactory* amfact)
  {
    const char* name = child->GetAttributeValue ("name");

    csRef<iRenderBuffer> offsetsBuffer;

    csRef<iDocumentNodeIterator> it = child->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child2 = it->Next ();
      if (child2->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child2->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_OFFSETS:
	{
	  offsetsBuffer = synldr->ParseRenderBuffer (child2);
	  if (!offsetsBuffer)
	  {
	    synldr->ReportError (msgidFactory, child2, "Could not parse render buffer!");
	    return false;
	  }
	}
	break;
      default:
	synldr->ReportBadToken (child2);
	return false;
      }
    }

    CS::Mesh::iAnimatedMeshMorphTarget* morphTarget = amfact->CreateMorphTarget (name);
    morphTarget->SetVertexOffsets (offsetsBuffer);
    morphTarget->Invalidate();
    return true;
  }
  
  bool AnimeshFactoryLoader::ParseBoundingBoxes (iDocumentNode* child,
						 CS::Mesh::iAnimatedMeshFactory* amfact)
  {
    CS::Animation::iSkeletonFactory* skeletonFact = amfact->GetSkeletonFactory ();
    if (!skeletonFact)
    {
      synldr->ReportError (msgidFactory, child, "Factory does not have a skeleton while creating bounding boxes");
      return 0;
    }

    CS::Animation::BoneID numBones = skeletonFact->GetTopBoneID () + 1;
    CS::Animation::BoneID currBone = 0;

    csRef<iDocumentNodeIterator> it = child->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child2 = it->Next ();
      if (child2->GetType () != CS_NODE_ELEMENT) continue;   // ??????????
      const char* value = child2->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
        case XMLTOKEN_BBOX:
	{
	  currBone = child2->GetAttributeValueAsInt("bone");
	  if (currBone >= numBones)
	  {
	    synldr->ReportError (msgidFactory, child2, 
				 "Invalid bounding box index %d, expected maximum %d bones",
				 (int) currBone, (int) numBones);
	    return false;
	  }

	  csBox3 bbox;
	  if (!synldr->ParseBox (child2, bbox))
	    return false;

	  amfact->SetBoneBoundingBox (currBone, bbox);
	}
	break;
      default:
	synldr->ReportBadToken (child2);
	return false;
      }
    }
    return true;
  }

  bool AnimeshFactoryLoader::Initialize (iObjectRegistry* objReg)
  {
    object_reg = objReg;

    synldr = csQueryRegistry<iSyntaxService> (object_reg);

    InitTokenTable (xmltokens);
    return true;
  }

  //-------------------------------------------------------------------------

  AnimeshFactorySaver::AnimeshFactorySaver (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  bool AnimeshFactorySaver::Initialize (iObjectRegistry* object_reg)
  {
    AnimeshFactorySaver::object_reg = object_reg;
    reporter = csQueryRegistry<iReporter> (object_reg);
    synldr = csQueryRegistry<iSyntaxService> (object_reg);
    engine = csQueryRegistry<iEngine> (object_reg);

    return true;
  }

  bool AnimeshFactorySaver::WriteDown (iBase *obj, iDocumentNode* parent,
    iStreamSource*)
  {
    if (!parent) return false; //you never know...

    csRef<iDocumentNode> paramsNode = 
      parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    paramsNode->SetValue("params");

    if (obj)
    {
      csRef<CS::Mesh::iAnimatedMeshFactory> factory = 
	scfQueryInterface<CS::Mesh::iAnimatedMeshFactory> (obj);
      csRef<iMeshObjectFactory> meshfact = 
	scfQueryInterface<iMeshObjectFactory> (obj);
      if (!factory) return false;
      if (!meshfact) return false;

      // Write material
      iMaterialWrapper* material = nullptr;
      for (size_t i = 0; i < factory->GetSubMeshCount (); i++)
      {
	CS::Mesh::iAnimatedMeshSubMeshFactory* submesh = factory->GetSubMesh (i);
	iMaterialWrapper* submeshMaterial = submesh->GetMaterial ();
	if (submeshMaterial)
	{
	  material = submeshMaterial;
	  break;
	}
      }

      if (material)
      {
	csRef<iDocumentNode> materialNode = 
	  paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	materialNode->SetValue ("material");
	csRef<iDocumentNode> materialNameNode = 
	  materialNode->CreateNodeBefore(CS_NODE_TEXT, 0);
	materialNameNode->SetValue (material->QueryObject()->GetName());
      }

      // Write vertices render buffer
      {
	iRenderBuffer* buffer = factory->GetVertices ();
	if (buffer)
	{
	  csRef<iDocumentNode> rbufNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  rbufNode->SetValue ("vertex");
	  /* Disabled checking on this buffer b/c no vertex count is available
	   * when loading it */
	  rbufNode->SetAttribute ("checkelementcount", "no");
	  synldr->WriteRenderBuffer (rbufNode, buffer);
	}
      }
      
      // Write normals render buffer
      {
	iRenderBuffer* buffer = factory->GetNormals ();
	if (buffer)
	{
	  csRef<iDocumentNode> rbufNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  rbufNode->SetValue ("normal");
	  synldr->WriteRenderBuffer (rbufNode, buffer);
	}
      }

      // Write texture coordinates render buffer
      {
	iRenderBuffer* buffer = factory->GetTexCoords ();
	if (buffer)
	{
	  csRef<iDocumentNode> rbufNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  rbufNode->SetValue ("texcoord");
	  synldr->WriteRenderBuffer (rbufNode, buffer);
	}
      }

      // Write tangents render buffer
      {
	iRenderBuffer* buffer = factory->GetTangents ();
	if (buffer)
	{
	  csRef<iDocumentNode> rbufNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  rbufNode->SetValue ("tangent");
	  synldr->WriteRenderBuffer (rbufNode, buffer);
	}
      }
      
      // Write binormals render buffer
      {
	iRenderBuffer* buffer = factory->GetBinormals ();
	if (buffer)
	{
	  csRef<iDocumentNode> rbufNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  rbufNode->SetValue ("binormal");
	  synldr->WriteRenderBuffer (rbufNode, buffer);
	}
      }
      
      // Write colors render buffer
      {
	iRenderBuffer* buffer = factory->GetColors ();
	if (buffer)
	{
	  csRef<iDocumentNode> rbufNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  rbufNode->SetValue ("color");
	  synldr->WriteRenderBuffer (rbufNode, buffer);
	}
      }
      
      // Write skeleton
      {
	CS::Animation::iSkeletonFactory* skeletonFactory =
	  factory->GetSkeletonFactory ();
	if (skeletonFactory)
	{
	  csRef<iDocumentNode> skeletonNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  skeletonNode->SetValue ("skeleton");
	  csRef<iDocumentNode> skeletonNameNode = 
	    skeletonNode->CreateNodeBefore(CS_NODE_TEXT, 0);
	  skeletonNameNode->SetValue (skeletonFactory->GetName ());
	}
      }

      // Write bone influences
      {
	CS::Mesh::AnimatedMeshBoneInfluence* influences = factory->GetBoneInfluences ();
	if (influences)
	{
	  csRef<iDocumentNode> influenceNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  influenceNode->SetValue ("boneinfluences");

	  size_t count = factory->GetBoneInfluencesPerVertex () * factory->GetVertexCount ();
	  for (size_t i = 0; i < count; i++)
	  {
	    csRef<iDocumentNode> node = 
	      influenceNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	    node->SetValue ("bi");
	    node->SetAttributeAsInt ("bone", influences[i].bone);
	    node->SetAttributeAsFloat ("weight", influences[i].influenceWeight);
	  }
	}
      }

      // Write submeshes
      {
	for (size_t i = 0; i < factory->GetSubMeshCount (); i++)
	{
	  CS::Mesh::iAnimatedMeshSubMeshFactory* submesh = factory->GetSubMesh (i);

	  csRef<iDocumentNode> submeshNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  submeshNode->SetValue ("submesh");

	  if (submesh->GetIndexSetCount () == 0) continue;
	  iRenderBuffer* buffer = submesh->GetIndices (0);
	  // TODO: missing indice sets in loader and saver
	  if (!buffer) continue;

	  csRef<iDocumentNode> indexNode = 
	    submeshNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  indexNode->SetValue ("index");
	  indexNode->SetAttribute ("indices", "yes");
	  synldr->WriteRenderBuffer (indexNode, buffer);

	  iMaterialWrapper* material = submesh->GetMaterial ();
	  if (material)
	  {
	    csRef<iDocumentNode> materialNode = 
	      submeshNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	    materialNode->SetValue ("material");
	    csRef<iDocumentNode> materialNameNode = 
	      materialNode->CreateNodeBefore(CS_NODE_TEXT, 0);
	    materialNameNode->SetValue (material->QueryObject()->GetName());
	  }
	}
      }

      // Write morph targets
      {
	for (size_t i = 0; i < factory->GetMorphTargetCount (); i++)
	{
	  CS::Mesh::iAnimatedMeshMorphTarget* target = factory->GetMorphTarget (i);

	  csRef<iDocumentNode> targetNode = 
	    paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  targetNode->SetValue ("morphtarget");
	  targetNode->SetAttribute ("name", target->GetName ());

	  iRenderBuffer* buffer = target->GetVertexOffsets ();
	  if (!buffer) continue;

	  csRef<iDocumentNode> offsetsNode = 
	    targetNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	  offsetsNode->SetValue ("offsets");
	  synldr->WriteRenderBuffer (offsetsNode, buffer);
	}
      }

      // Write bounding boxes
      {
	CS::Animation::iSkeletonFactory* skeletonFact = factory->GetSkeletonFactory ();
	if (skeletonFact)
	{
	  CS::Animation::BoneID numBones = skeletonFact->GetTopBoneID () + 1;

	  if (numBones > 0)
	  {
	    csRef<iDocumentNode> bboxNode = 
	      paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
	    bboxNode->SetValue ("bboxes");
	
	    for (CS::Animation::BoneID i = 0; i < numBones ; i++)
	    {
	      if (skeletonFact->HasBone (i))
	      {
		csRef<iDocumentNode> node = 
		  bboxNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
		node->SetValue ("bbox");
		node->SetAttributeAsInt ("bone", i);
		csBox3 bbox = factory->GetBoneBoundingBox (i);
		synldr->WriteBox (node, bbox);
	      }
	    }
	  }
	}
      }
    }

    return true;
  }
  

  static const char* msgid = "crystalspace.animeshloader";

  AnimeshObjectLoader::AnimeshObjectLoader (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  csPtr<iBase> AnimeshObjectLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context,
    iBase* context)
  {

    csRef<iMeshObject> mesh;
    csRef<CS::Mesh::iAnimatedMesh> ammesh;

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_FACTORY:
        {
          const char* factname = child->GetContentsValue ();
          iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);

          if(!fact)
          {
            synldr->ReportError (msgid, child, 
              "Couldn't find factory %s!", CS::Quote::Single (factname));
            return 0;
          }

          csRef<CS::Mesh::iAnimatedMeshFactory> amfact = 
            scfQueryInterface<CS::Mesh::iAnimatedMeshFactory> (fact->GetMeshObjectFactory ());
          if (!amfact)
          {
            synldr->ReportError (msgid, child, 
              "Factory %s doesn't appear to be a animesh factory!", CS::Quote::Single (factname));
            return 0;
          }

          mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          ammesh = scfQueryInterface<CS::Mesh::iAnimatedMesh> (mesh);
          if (!ammesh)
          {
            synldr->ReportError (msgid, child, 
              "Factory %s doesn't appear to be a animesh factory!", CS::Quote::Single (factname));
            return 0;
          }
        }
        break;
      case XMLTOKEN_MATERIAL:
        {
          const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
          if (!mat)
          {
            synldr->ReportError (msgid, child, "Couldn't find material %s!", 
              CS::Quote::Single (matname));
            return 0;
          }
          mesh->SetMaterialWrapper (mat);
        }
        break;
      case XMLTOKEN_MIXMODE:
        {
          uint mm;
          if (!synldr->ParseMixmode (child, mm))
            return 0;
          mesh->SetMixMode (mm);
        }
        break;
      case XMLTOKEN_SKELETON:
        {
          if (!skelMgr)
          {
            skelMgr = csQueryRegistry<CS::Animation::iSkeletonManager> (object_reg);

            if (!skelMgr)
            {
              synldr->ReportError (msgid, child, "Could not find any loaded skeletons");
              return 0;
            }
          }      

          const char* skelName = child->GetContentsValue ();
          CS::Animation::iSkeletonFactory* skelFact = skelMgr->FindSkeletonFactory (skelName);
          if (!skelFact)
          {
            synldr->ReportError (msgid, child, "Could not find skeleton %s", skelName);
            return 0;
          }

          csRef<CS::Animation::iSkeleton> skeleton = skelFact->CreateSkeleton ();
          ammesh->SetSkeleton (skeleton);
        }
        break;
      case XMLTOKEN_ANIMATIONPACKET:
        {
          if (!skelMgr)
          {
            skelMgr = csQueryRegistry<CS::Animation::iSkeletonManager> (object_reg);

            if (!skelMgr)
            {
              synldr->ReportError (msgid, child, "Could not find any loaded skeletons");
              return 0;
            }
          }

          CS::Animation::iSkeleton* skeleton = ammesh->GetSkeleton ();
          if (!skeleton)
          {
            synldr->ReportError (msgid, child, "Mesh does not have a skeleton");
            return 0;
          }

          const char* packetName = child->GetContentsValue ();
          CS::Animation::iSkeletonAnimPacketFactory* packetFact = skelMgr->FindAnimPacketFactory (packetName);
          if (!packetFact)
          {
            synldr->ReportError (msgid, child, "Could not find animation packet %s", packetName);
            return 0;
          }

          csRef<CS::Animation::iSkeletonAnimPacket> packet = packetFact->CreateInstance (skeleton);
          skeleton->SetAnimationPacket (packet);
        }
        break;
      case XMLTOKEN_BBOXES:
        {
          if (!ParseBoundingBoxes (child, ammesh))
            return 0;
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }

    return csPtr<iBase> (mesh);    
  }

  bool AnimeshObjectLoader::ParseBoundingBoxes (iDocumentNode* child,
						CS::Mesh::iAnimatedMesh* amesh)
  {
    CS::Animation::iSkeleton* skeleton = amesh->GetSkeleton ();
    if (!skeleton)
    {
      synldr->ReportError (msgid, child, "Mesh does not have a skeleton while creating bounding boxes");
      return false;
    }

    CS::Animation::BoneID numBones = skeleton->GetFactory ()->GetTopBoneID () + 1;
    CS::Animation::BoneID currBone;

    csRef<iDocumentNodeIterator> it = child->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child2 = it->Next ();
      if (child2->GetType () != CS_NODE_ELEMENT) continue;   // ??????????
      const char* value = child2->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
        case XMLTOKEN_BBOX:
	{
	  currBone = child2->GetAttributeValueAsInt ("bone");
	  if (currBone >= numBones)
	  {
	    synldr->ReportError (msgid, child2, 
				 "Invalid bounding box index %d, expected maximum %d bones",
				 (int) currBone, (int) numBones);
	    return false;
	  }

	  csBox3 bbox;
	  if (!synldr->ParseBox (child2, bbox))
	    return false;

	  amesh->SetBoneBoundingBox (currBone, bbox);
	}
	break;
      default:
	synldr->ReportBadToken (child2);
	return false;
      }
    }
    return true;
  }

  bool AnimeshObjectLoader::Initialize (iObjectRegistry* objReg)
  {
    object_reg = objReg;

    synldr = csQueryRegistry<iSyntaxService> (object_reg);

    InitTokenTable (xmltokens);
    return true;
  }




  AnimeshObjectSaver::AnimeshObjectSaver (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  bool AnimeshObjectSaver::WriteDown (iBase *obj, iDocumentNode* parent,
    iStreamSource*)
  {
    return false;
  }

  bool AnimeshObjectSaver::Initialize (iObjectRegistry*)
  {
    return false;
  }

}
CS_PLUGIN_NAMESPACE_END(Animeshldr)

