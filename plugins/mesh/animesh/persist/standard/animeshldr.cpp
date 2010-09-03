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

#include "csutil/ref.h"
#include "iengine/engine.h"
#include "imap/services.h"
#include "imesh/animesh.h"
#include "imesh/object.h"
#include "iutil/stringarray.h"
#include "iutil/document.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "imap/ldrctxt.h"
#include "iengine/mesh.h"
#include "imesh/skeleton2.h"
#include "imesh/animnode/skeleton2anim.h"

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
            synldr->ReportError (msgidFactory, child, "Couldn't find material '%s'!", 
              matname);
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

          CS::Mesh::csAnimatedMeshBoneInfluence* bi = amfact->GetBoneInfluences ();

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
                  synldr->ReportError (msgidFactory, child2, "Couldn't find material '%s'!", 
                    matname);
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

  bool AnimeshFactorySaver::WriteDown (iBase *obj, iDocumentNode* parent,
    iStreamSource*)
  {
    return false;
  }
  
  bool AnimeshFactorySaver::Initialize (iObjectRegistry*)
  {
    return false;
  }




  AnimeshObjectLoader::AnimeshObjectLoader (iBase* parent)
    : scfImplementationType (this, parent)
  {}

  csPtr<iBase> AnimeshObjectLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context,
    iBase* context)
  {
    static const char* msgid = "crystalspace.animeshloader";

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
              "Couldn't find factory '%s'!", factname);
            return 0;
          }

          csRef<CS::Mesh::iAnimatedMeshFactory> amfact = 
            scfQueryInterface<CS::Mesh::iAnimatedMeshFactory> (fact->GetMeshObjectFactory ());
          if (!amfact)
          {
            synldr->ReportError (msgid, child, 
              "Factory '%s' doesn't appear to be a animesh factory!", factname);
            return 0;
          }

          mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          ammesh = scfQueryInterface<CS::Mesh::iAnimatedMesh> (mesh);
          if (!ammesh)
          {
            synldr->ReportError (msgid, child, 
              "Factory '%s' doesn't appear to be a animesh factory!", factname);
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
            synldr->ReportError (msgid, child, "Couldn't find material '%s'!", 
              matname);
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
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }

    return csPtr<iBase> (mesh);    
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

