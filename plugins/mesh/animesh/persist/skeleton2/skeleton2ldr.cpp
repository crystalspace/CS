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

    skelManager = csQueryRegistryOrLoad<CS::Animation::iSkeletonManager> (object_reg,
      "crystalspace.skeletalanimation");

    InitTokenTable (xmltokens);
    return true;
  }

  bool SkeletonLoader::ParseSkeleton (iDocumentNode* node)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseskeleton";

    CS::Animation::iSkeletonFactory* factory = 0;

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
        if (!ParseBone (child, factory, CS::Animation::InvalidBoneID))
        {
          return false;
        }
        break;
      case XMLTOKEN_ANIMATIONPACKET:
        {
          csRef<CS::Animation::iSkeletonAnimPacketFactory> packet;
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


  bool SkeletonLoader::ParseBone (iDocumentNode* node, CS::Animation::iSkeletonFactory* factory, CS::Animation::BoneID parent)
  {
    static const char* msgid = "crystalspace.skeletonloader.parsebone";

    CS::Animation::BoneID boneId = factory->CreateBone (parent);

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

    CS::Animation::iSkeletonAnimPacketFactory* packet = skelManager->CreateAnimPacketFactory (name);
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
          csRef<CS::Animation::iSkeletonAnimNodeFactory> nodeFact = ParseAnimTreeNode (child, packet);

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

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseAnimTreeNode (iDocumentNode* node,
    CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimtreenode";

    csRef<CS::Animation::iSkeletonAnimNodeFactory> result;

    const char* type = node->GetAttributeValue ("type");

    // Split up on type...
    csStringID id = xmltokens.Request (type);
    switch (id)
    {
    case XMLTOKEN_ANIMATION:
      {
        result = ParseAnimationNode (node, packet);
      }
      break;
    case XMLTOKEN_BLEND:
      {
        result = ParseBlendNode (node, packet);
      }
      break;
    case XMLTOKEN_PRIORITY:
      {
        result = ParsePriorityNode (node, packet);
      } 
      break;
    case XMLTOKEN_RANDOM:
      {
        result = ParseRandomNode (node, packet);
      }
      break;
    case XMLTOKEN_FSM:
      {
        result = ParseFSMNode (node, packet);
      }
      break;
    default:
      synldr->ReportError (msgid, node, "Invalid node type %s", type);
      return 0;
    }

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (result);    
  }

  CS::Animation::iSkeletonAnimation* SkeletonLoader::ParseAnimation (iDocumentNode* node, 
    CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimation";

    // Check if it is a ref..
    const char* ref = node->GetAttributeValue ("ref");
    if (ref)
    {
      CS::Animation::iSkeletonAnimation* fact = packet->FindAnimation (ref);
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

    CS::Animation::iSkeletonAnimation* fact = packet->CreateAnimation (name);
    if (!fact)
    {
      synldr->ReportError (msgid, node, 
        "Could not create animation, another animation with same name already exist");
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

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseAnimationNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimationnode";

    csRef<CS::Animation::iSkeletonAnimationNodeFactory> factnode;

    const char* name = node->GetAttributeValue ("name");

    // Get the animation itself
    const char* animName = node->GetAttributeValue ("animation");
    if (!animName)
    {
      synldr->ReportError (msgid, node, "No animation specified");
      return 0;
    }

    // Reuse the animation name if no node-name set
    if (!name)
      name = animName;

    factnode = packet->CreateAnimationNode (name);


    CS::Animation::iSkeletonAnimation* anim = packet->FindAnimation (animName);
    if (!anim)
    {
      synldr->ReportError (msgid, node, "Animation \"%s\" not found", animName);
      return 0;
    }
    factnode->SetAnimation (anim);

    // Properties..
    bool isCyclic, reset, autostop;
    if (synldr->ParseBoolAttribute (node, "cyclic", isCyclic, true, false))
    {
      factnode->SetCyclic (isCyclic);
    }

    if (synldr->ParseBoolAttribute (node, "autoreset", reset, false, false))
    {
      factnode->SetAutomaticReset (reset);
    }

    if (synldr->ParseBoolAttribute (node, "autostop", autostop, true, false))
    {
      factnode->SetAutomaticStop (autostop);
    }

    if (node->GetAttribute ("speed"))
    {
      float speed = node->GetAttributeValueAsFloat ("speed");
      factnode->SetPlaybackSpeed (speed);
    }

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseBlendNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    //static const char* msgid = "crystalspace.skeletonloader.parseblendnode";

    csRef<CS::Animation::iSkeletonBlendNodeFactory> factnode;

    // Name & node creation
    const char* name = node->GetAttributeValue ("name");
    factnode = packet->CreateBlendNode (name);

    // Get sync mode
    const char* sync = node->GetAttributeValue ("sync");
    if (sync)
    {
      csStringID id = xmltokens.Request (sync);
      CS::Animation::SynchronizationMode mode = CS::Animation::SYNC_NONE;

      switch (id)
      {
      case XMLTOKEN_NONE:
        mode = CS::Animation::SYNC_NONE;
        break;
      case XMLTOKEN_FIRSTFRAME:
        mode = CS::Animation::SYNC_FIRSTFRAME;
        break;
      }

      factnode->SetSynchronizationMode (mode);
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
      case XMLTOKEN_NODE:
        {
          csRef<CS::Animation::iSkeletonAnimNodeFactory> childFact = 
            ParseAnimTreeNode (child, packet);

          float weight = 1.0;
          if (child->GetAttribute ("weight"))
          {
            weight = child->GetAttributeValueAsFloat ("weight");
          }
          factnode->AddNode (childFact, weight);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    };

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParsePriorityNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    //static const char* msgid = "crystalspace.skeletonloader.parseprioritynode";

    csRef<CS::Animation::iSkeletonPriorityNodeFactory> factnode;

    const char* name = node->GetAttributeValue ("name");
    factnode = packet->CreatePriorityNode (name);

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
          csRef<CS::Animation::iSkeletonAnimNodeFactory> childFact = 
            ParseAnimTreeNode (child, packet);

          int prio = 1;
          if (child->GetAttribute ("priority"))
          {
            prio = child->GetAttributeValueAsInt ("priority");
          }

          factnode->AddNode (childFact, prio);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    };

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseRandomNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    //static const char* msgid = "crystalspace.skeletonloader.parserandomnode";

    csRef<CS::Animation::iSkeletonRandomNodeFactory> factnode;

    const char* name = node->GetAttributeValue ("name");
    factnode = packet->CreateRandomNode (name);

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
          csRef<CS::Animation::iSkeletonAnimNodeFactory> childFact = 
            ParseAnimTreeNode (child, packet);

          float prob = 1.0f;
          if (child->GetAttribute("probability"))
          {
            prob = child->GetAttributeValueAsFloat ("probability");
          }

          factnode->AddNode (childFact, prob);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    };

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseFSMNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parsefsmnode";

    csRef<CS::Animation::iSkeletonFSMNodeFactory> factnode;

    const char* name = node->GetAttributeValue ("name");
    factnode = packet->CreateFSMNode (name);

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_STATE:
        {
          CS::Animation::StateID stateID = factnode->AddState ();

          const char* name = child->GetAttributeValue ("name");
          factnode->SetStateName (stateID, name);
          
          csRef<iDocumentNodeIterator> it = child->GetNodes ();
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
                csRef<CS::Animation::iSkeletonAnimNodeFactory> node =
                  ParseAnimTreeNode (child, packet);

                factnode->SetStateNode (stateID, node);
              }
              break;
            default:
              synldr->ReportBadToken (child);
              return 0;
            }       
          };
        }
        break;
      case XMLTOKEN_TRANSITION:
        {
          const char* fromStateName = child->GetAttributeValue ("from");
          const char* toStateName = child->GetAttributeValue ("to");

          CS::Animation::StateID fromState = factnode->FindState (fromStateName);
          CS::Animation::StateID toState = factnode->FindState (toStateName);

          if (fromState == CS::Animation::InvalidStateID)
          {
            synldr->ReportError (msgid, child, 
              "Invalid from state %s", fromStateName);
          }

          if (toState == CS::Animation::InvalidStateID)
          {
            synldr->ReportError (msgid, child, 
              "Invalid to state %s", toStateName);
          }

          csRef<iDocumentNode> nodedoc = child->GetNode (
            xmltokens.Request (XMLTOKEN_NODE));
          if (nodedoc)
          {
            csRef<CS::Animation::iSkeletonAnimNodeFactory> node =
              ParseAnimTreeNode (nodedoc, packet);

            factnode->SetStateTransition (fromState, toState, node);
          }

          const float time1 = child->GetAttributeValueAsFloat ("time1");
          const float time2 = child->GetAttributeValueAsFloat ("time2");

          if(time1 > 0.0f || time2 > 0.0f)
          {
            factnode->SetTransitionCrossfade (fromState, toState, time1, time2);
          }

          bool automatic = child->GetAttributeValueAsBool ("automatic", false);
	  if (automatic)
	    factnode->SetAutomaticTransition (fromState, toState, true);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    };

    if (node->GetAttribute ("start"))
    {
      int start = node->GetAttributeValueAsInt ("start");
      factnode->SetStartState (start);
    }

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2Ldr)

