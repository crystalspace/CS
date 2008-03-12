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
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/plugin.h"
#include "imap/ldrctxt.h"
#include "imesh/skeleton2.h"
#include "imesh/skeleton2anim.h"

#include "skeleton2ldr.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2Ldr)
{
  SCF_IMPLEMENT_FACTORY(SkeletonLoader);


  SkeletonLoader::SkeletonLoader (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  csPtr<iBase> SkeletonLoader::Parse (iDocumentNode* node,
    iStreamSource* ssource, iLoaderContext* ldr_context,
    iBase* context)
  {
    static const char* msgid = "crystalspace.skeletonloader";

    if (!skelManager)
    {
      synldr->ReportError (msgid, node, "Couldn't get any skeleton system");
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
      case XMLTOKEN_SKELETON:
        if (!ParseSkeleton (child))
        {
          return 0;
        }

        break;
      case XMLTOKEN_ANIMATIONPACKET:
        if (!ParseAnimPacket (child))
        {
          return 0;
        }

        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }
    
    return csPtr<iBase> (skelManager);
  }
  
  bool SkeletonLoader::Initialize (iObjectRegistry* objReg)
  {
    object_reg = objReg;

    synldr = csQueryRegistry<iSyntaxService> (object_reg);

    skelManager = csQueryRegistryOrLoad<iSkeletonManager2> (object_reg,
      "crystalspace.skeletalanimation");

    InitTokenTable (xmltokens);
    return true;
  }

  bool SkeletonLoader::ParseSkeleton (iDocumentNode* node)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseskeleton";

    iSkeletonFactory2* factory = 0;

    // Get common properties
    const char* name = node->GetAttributeValue ("name");
    if (!name)
    {
      synldr->ReportError (msgid, node, "No name set for skeleton");
      return false;
    }

    factory = skelManager->CreateSkeletonFactory (name);
    if (!factory)
    {
      synldr->ReportError (msgid, node, 
        "Could not create skeleton, another skeleton with same name might already exist.");
      return false;
    }


    // Load bones etc..
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
        if (!ParseBone (child, factory, InvalidBoneID))
        {
          return false;
        }
        break;
      case XMLTOKEN_ANIMATIONPACKET:
        {
          csRef<iSkeletonAnimPacketFactory2> packet;
          const char* name = child->GetContentsValue ();

          packet = skelManager->FindAnimPacketFactory (name);

          if (!packet)
          {
            synldr->ReportError (msgid, child, "Animation packet not found!");
            return false;
          }
          factory->SetAnimationPacket (packet);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }
    
    return true;
  }


  bool SkeletonLoader::ParseBone (iDocumentNode* node, iSkeletonFactory2* factory, BoneID parent)
  {
    static const char* msgid = "crystalspace.skeletonloader.parsebone";

    BoneID boneId = factory->CreateBone (parent);

    const char* name = node->GetAttributeValue ("name");
    if (name)
    {
      factory->SetBoneName (boneId, name);
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
      case XMLTOKEN_BONE:
        if (!ParseBone (child, factory, boneId))
        {
          synldr->ReportError (msgid, child, "Couldn't parse bone");
          return false;
        }
        break;
      case XMLTOKEN_TRANSFORM:
        {
          csVector3 offs;
          if (!synldr->ParseVector (child, offs))
          {
            synldr->ReportError (msgid, child, "Couldn't parse transform");
            return false;
          }

          // Get the rotation part
          csQuaternion q;
          q.v.x = child->GetAttributeValueAsFloat ("qx");
          q.v.y = child->GetAttributeValueAsFloat ("qy");
          q.v.z = child->GetAttributeValueAsFloat ("qz");
          q.w = child->GetAttributeValueAsFloat ("qw");

          factory->SetTransformBoneSpace (boneId, q, offs);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool SkeletonLoader::ParseAnimPacket (iDocumentNode* node)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimpacket";

    // Get common properties
    const char* name = node->GetAttributeValue ("name");
    if (!name)
    {
      synldr->ReportError (msgid, node, "No name set for animation packet");
      return false;
    }

    iSkeletonAnimPacketFactory2* packet = skelManager->CreateAnimPacketFactory (name);
    if (!packet)
    {
      synldr->ReportError (msgid, node, 
        "Could not create packet, another packet with same name might already exist.");
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
      case XMLTOKEN_ANIMATION:
        {
          if (!ParseAnimation (child, packet))
          {
            synldr->ReportError (msgid, child, "Error loading animation.");
            return false;
          }
        }
        break;
      case XMLTOKEN_NODE:
        {
          csRef<iSkeletonAnimNodeFactory2> nodeFact = ParseAnimTreeNode (child, packet);

          if (!nodeFact)
          {
            synldr->ReportError (msgid, child, "Error loading animation node.");
            return false;
          }
          // New root
          packet->SetAnimationRoot (nodeFact);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  csPtr<iSkeletonAnimNodeFactory2> SkeletonLoader::ParseAnimTreeNode (iDocumentNode* node,
    iSkeletonAnimPacketFactory2* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimtreenode";

    csRef<iSkeletonAnimNodeFactory2> result;

    const char* type = node->GetAttributeValue ("type");

    // Split up on type...
    csStringID id = xmltokens.Request (type);
    switch (id)
    {
    case XMLTOKEN_ANIMATION:
      {
        csRef<iSkeletonAnimationFactory2> anode;
        anode = ParseAnimation (node, packet);
        if (!anode)
        {
          synldr->ReportError (msgid, node, "Couldn't load animation");
          return 0;
        }
        result = anode;
      }
      break;
    case XMLTOKEN_BLEND:
      {
        csRef<iSkeletonBlendNodeFactory2> bnode;

        const char* name = node->GetAttributeValue ("name");

        bnode = packet->CreateBlendNode (name);

        csRef<iDocumentNodeIterator> it = node->GetNodes ();
        while (it->HasNext ())
        {
          csRef<iDocumentNode> child = it->Next ();
          if (child->GetType () != CS_NODE_ELEMENT) continue;
          const char* value = child->GetValue ();
          csStringID id = xmltokens.Request (value);
          switch (id)
          {
          case XMLTOKEN_NODE:
            {
              csRef<iSkeletonAnimNodeFactory2> childFact = 
                ParseAnimTreeNode (child, packet);

              float weight = child->GetAttributeValueAsFloat ("weight");
              bnode->AddNode (childFact, weight);
            }
            break;
          default:
            synldr->ReportBadToken (child);
            return 0;
          }       
        };

        result = bnode;
      }
      break;
    default:
      synldr->ReportError (msgid, node, "Invalid node type %s", type);
      return 0;
    }

    return csPtr<iSkeletonAnimNodeFactory2> (result);    
  }

  iSkeletonAnimationFactory2* SkeletonLoader::ParseAnimation (iDocumentNode* node, 
    iSkeletonAnimPacketFactory2* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimation";

    // Check if it is a ref..
    const char* ref = node->GetAttributeValue ("ref");
    if (ref)
    {
      iSkeletonAnimationFactory2* fact = packet->FindAnimation (ref);
      if (!fact)
      {      
        synldr->ReportError (msgid, node, "Referenced animation %s not found", ref);
        return 0;
      }

      return fact;
    }

    const char* name = node->GetAttributeValue ("name");
    if (!name)
    {
      synldr->ReportError (msgid, node, "No name set for animation");
      return false;
    }

    iSkeletonAnimationFactory2* fact = packet->CreateAnimation (name);
    if (!fact)
    {
      synldr->ReportError (msgid, node, 
        "Could not create animation, another animation with same name might already exist");
      return false;
    }
    
    // Handle "separate-file" loading...


    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_CHANNEL:
        {
          int boneid = child->GetAttributeValueAsInt ("bone");

          CS::Animation::ChannelID cID = fact->AddChannel (boneid);

          csRef<iDocumentNodeIterator> it = child->GetNodes ();
          while (it->HasNext ())
          {
            csRef<iDocumentNode> child = it->Next ();
            if (child->GetType () != CS_NODE_ELEMENT) continue;
            const char* value = child->GetValue ();
            csStringID id = xmltokens.Request (value);
            switch (id)
            {
            case XMLTOKEN_KEY:
              {
                csVector3 offs;
                if (!synldr->ParseVector (child, offs))
                {
                  synldr->ReportError (msgid, child, "Couldn't parse transform");
                  return 0;
                }

                // Get the rotation part
                csQuaternion q;
                q.v.x = child->GetAttributeValueAsFloat ("qx");
                q.v.y = child->GetAttributeValueAsFloat ("qy");
                q.v.z = child->GetAttributeValueAsFloat ("qz");
                q.w = child->GetAttributeValueAsFloat ("qw");

                float time = child->GetAttributeValueAsFloat ("time");

                fact->AddKeyFrame (cID, time, q, offs);
              }
              break;
            default:
              synldr->ReportBadToken (child);
              return 0;
            }
          }
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }

    return fact;
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2Ldr)

