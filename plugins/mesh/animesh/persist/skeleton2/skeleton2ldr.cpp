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

#include "cstool/mocapparser.h"
#include "csutil/ref.h"
#include "imap/services.h"
#include "iutil/document.h"
#include "iutil/plugin.h"
#include "imap/ldrctxt.h"
#include "imesh/bodymesh.h"
#include "imesh/skeleton2.h"
#include "imesh/animnode/debug.h"
#include "imesh/animnode/ik.h"
#include "imesh/animnode/lookat.h"
#include "imesh/animnode/ragdoll.h"
#include "imesh/animnode/retarget.h"
#include "imesh/animnode/skeleton2anim.h"
#include "imesh/animnode/speed.h"

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
    if (!skelManager)
      return false;

    bodyManager = csQueryRegistryOrLoad<CS::Animation::iBodyManager> (object_reg,
      "crystalspace.mesh.animesh.body");
    if (!bodyManager)
      return false;

    InitTokenTable (xmltokens);
    return true;
  }

  bool SkeletonLoader::ParseSkeleton (iDocumentNode* node)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseskeleton";

    CS::Animation::iSkeletonFactory* factory = nullptr;

    // Find or create the skeleton
    const char* ref = node->GetAttributeValue ("ref");
    if (ref)
    {
      factory = skelManager->FindSkeletonFactory (ref);
      if (!factory)
      {
	synldr->Report (msgid, CS_REPORTER_SEVERITY_WARNING, node, 
			"Could not find referenced skeleton %s.",
			CS::Quote::Single (ref));
	return false;
      }
    }

    else
    {
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
			     "Could not create skeleton %s.",
			     CS::Quote::Single (name));
	return false;
      }
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
      case XMLTOKEN_STARTANIMATION:
        {
	  bool automatic = child->GetAttributeValueAsBool ("automatic", true);
	  if (!automatic)
	    factory->SetAutoStart (false);
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

    CS::Animation::iSkeletonAnimPacketFactory* packet = nullptr;

    // Check if this a motion capture packet
    const char* type = node->GetAttributeValue ("type");
    if (type && strcmp (type, "mocap") == 0)
    {
      const char* name = node->GetAttributeValue ("name");
      if (!name)
      {
	synldr->ReportError (msgid, node, "No name set for animation packet");
	return false;
      }

      CS::Animation::BVHMocapParser mocapParser (object_reg);
      mocapParser.SetPacketName (name);

      const char* file = node->GetAttributeValue ("file");
      if (file) mocapParser.SetRessourceFile (file);

      const char* skelName = node->GetAttributeValue ("skelname");
      if (skelName) mocapParser.SetSkeletonName (skelName);

      const char* animName = node->GetAttributeValue ("animname");
      if (animName) mocapParser.SetAnimationName (animName);

      if (node->GetAttribute ("start"))
      {
	int startFrame = node->GetAttributeValueAsInt ("start");
	if (startFrame > 0)
	  mocapParser.SetStartFrame (startFrame);
      }

      if (node->GetAttribute ("end"))
      {
	int endFrame = node->GetAttributeValueAsInt ("end");
	if (endFrame > 0)
	  mocapParser.SetEndFrame (endFrame);
      }

      if (node->GetAttribute ("scale"))
      {
	float scale = node->GetAttributeValueAsFloat ("scale");
	if (abs (scale) > EPSILON) 
	  mocapParser.SetGlobalScale (scale);
      }

      CS::Animation::MocapParserResult parsingResult = mocapParser.ParseData ();
      if (!parsingResult.result)
	return false;

      packet = parsingResult.animPacketFactory;
    }

    // Find or create the packet
    else
    {
      const char* ref = node->GetAttributeValue ("ref");
      if (ref)
      {
	packet = skelManager->FindAnimPacketFactory (ref);
	if (!packet)
	{
	  synldr->Report (msgid, CS_REPORTER_SEVERITY_WARNING, node, 
			  "Could not find referenced packet %s.",
			  CS::Quote::Single (ref));
	  return false;
	}
      }

      else
      {
	const char* name = node->GetAttributeValue ("name");
	if (!name)
	{
	  synldr->ReportError (msgid, node, "No name set for animation packet");
	  return false;
	}

	packet = skelManager->CreateAnimPacketFactory (name);
	if (!packet)
	{
	  synldr->ReportError (msgid, node, 
			       "Could not create packet %s.",
			       CS::Quote::Single (name));
	  return false;
	}
      }
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
          csRef<CS::Animation::iSkeletonAnimNodeFactory> nodeFact =
	    ParseAnimTreeNode (child, packet);
          if (!nodeFact)
            return false;

          // Set new root
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
    case XMLTOKEN_BLENDTREE:
      {
	if (!node->GetAttribute ("packet"))
	{
	  synldr->ReportError (msgid, node, "No animation packet provided");
	  return 0;
	}

	const char* packetName = node->GetAttributeValue ("packet");
	CS::Animation::iSkeletonAnimPacketFactory* refPacket =
	  skelManager->FindAnimPacketFactory (packetName);

	if (!refPacket)
	{
	  synldr->ReportError (msgid, node, "Animation packet %s not found",
			       CS::Quote::Single (packetName));
	  return 0;
	}

	if (refPacket == packet)
	{
	  synldr->ReportError (msgid, node,
			       "The referenced packet is not different from the current one");
	  return 0;
	}

        result = refPacket->GetAnimationRoot ();
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
    case XMLTOKEN_DEBUG:
      {
        result = ParseDebugNode (node, packet);
      }
      break;
    case XMLTOKEN_IKCCD:
    case XMLTOKEN_IKPHYSICAL:
      {
        result = ParseIKNode (node, packet);
      }
      break;
    case XMLTOKEN_LOOKAT:
      {
        result = ParseLookAtNode (node, packet);
      }
      break;
    case XMLTOKEN_RAGDOLL:
      {
        result = ParseRagdollNode (node, packet);
      }
      break;
    case XMLTOKEN_RETARGET:
      {
        result = ParseRetargetNode (node, packet);
      }
      break;
    case XMLTOKEN_SPEED:
      {
        result = ParseSpeedNode (node, packet);
      }
      break;
    default:
      synldr->ReportError (msgid, node, "Invalid node type %s", CS::Quote::Single (type));
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
        synldr->ReportError (msgid, node, "Could not find referenced animation %s",
			     CS::Quote::Single (ref));
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

    // Check if we the animation is in another packet
    CS::Animation::iSkeletonAnimPacketFactory* sourcePacket;
    const char* packetName = node->GetAttributeValue ("packet");
    if (packetName)
    {
      sourcePacket = skelManager->FindAnimPacketFactory (packetName);
      if (!sourcePacket)
      {
	synldr->ReportError (msgid, node, "Animation packet %s not found",
			     CS::Quote::Single (packetName));
	return 0;
      }
    }

    else
      sourcePacket = packet;

    CS::Animation::iSkeletonAnimation* anim = sourcePacket->FindAnimation (animName);
    if (!anim)
    {
      synldr->ReportError (msgid, node, "Animation %s not found",
			   CS::Quote::Single (animName));
      return 0;
    }
    factnode->SetAnimation (anim);

    // Properties..
    bool isCyclic, reset, autostop;
    if (synldr->ParseBoolAttribute (node, "cyclic", isCyclic, false, false))
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
              "Invalid from state %s", CS::Quote::Single (fromStateName));
          }

          if (toState == CS::Animation::InvalidStateID)
          {
            synldr->ReportError (msgid, child, 
              "Invalid to state %s", CS::Quote::Single (toStateName));
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

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseDebugNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parsedebugnode";

    csRef<CS::Animation::iSkeletonDebugNodeFactory> factnode;

    csRef<CS::Animation::iSkeletonDebugNodeManager> debugManager =
      csQueryRegistryOrLoad<CS::Animation::iSkeletonDebugNodeManager> (object_reg,
      "crystalspace.mesh.animesh.animnode.debug");

    const char* name = node->GetAttributeValue ("name");
    factnode = debugManager->CreateAnimNodeFactory (name);

    CS::Animation::SkeletonDebugMode debugMode = CS::Animation::DEBUG_NONE;
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
	  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode =
	    ParseAnimTreeNode (child, packet);

	  if (childNode && factnode->GetChildNode ())
	    synldr->Report (msgid, CS_REPORTER_SEVERITY_WARNING, node,
			    "The Debug node can only handle one child node");

	  else if (childNode)
	    factnode->SetChildNode (childNode);
	}
	break;

      case XMLTOKEN_MODE:
        {
	  const char* type = child->GetAttributeValue ("type");
	  if (strcmp (type, "2d_lines") == 0)
	    debugMode = (CS::Animation::SkeletonDebugMode) (debugMode & CS::Animation::DEBUG_2DLINES);
	  else if (strcmp (type, "squares") == 0)
	    debugMode = (CS::Animation::SkeletonDebugMode) (debugMode & CS::Animation::DEBUG_SQUARES);
	  else
	    synldr->Report (msgid, CS_REPORTER_SEVERITY_WARNING, node,
			    "Unsupported debug mode %s", CS::Quote::Single (type));
	}
	break;

      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    }

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseIKNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseiknode";

    csRef<CS::Animation::iSkeletonIKNodeFactory> factnode;

    const char* type = node->GetAttributeValue ("type");
    bool isCCD = strcmp (type, "ikccd") == 0;

    csRef<CS::Animation::iSkeletonIKNodeManager> IKManager = isCCD ?
      csLoadPluginCheck<CS::Animation::iSkeletonIKNodeManager>
      (object_reg, "crystalspace.mesh.animesh.animnode.ik.ccd")
      : csLoadPluginCheck<CS::Animation::iSkeletonIKNodeManager>
      (object_reg, "crystalspace.mesh.animesh.animnode.ik.physical");

    const char* name = node->GetAttributeValue ("name");
    factnode = IKManager->CreateAnimNodeFactory (name);

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
	  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode =
	    ParseAnimTreeNode (child, packet);

	  if (childNode && factnode->GetChildNode ())
	    synldr->Report (msgid, CS_REPORTER_SEVERITY_WARNING, node,
			    "The IK node can only handle one child node");

	  else if (childNode)
	    factnode->SetChildNode (childNode);
	}
	break;

      case XMLTOKEN_EFFECTOR:
        {
	  if (!ParseEffector (child, factnode))
	    return 0;
	}
	break;

      default:
	break;
      }       
    }

    if (isCCD)
    {
      if (!ParseIKCCDNode (node, factnode))
	return 0;
    }

    else if (!ParseIKPhysicalNode (node, factnode))
      return 0;

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  bool SkeletonLoader::ParseIKCCDNode (iDocumentNode* node, CS::Animation::iSkeletonIKNodeFactory* factnode)
  {
    csRef<CS::Animation::iSkeletonIKCCDNodeFactory> factory =
      scfQueryInterface<CS::Animation::iSkeletonIKCCDNodeFactory> (factnode);

    if (node->GetAttribute ("jointinit"))
    {
      bool init = node->GetAttributeValueAsBool ("jointinit");
      factory->SetJointInitialization (init);
    }

    if (node->GetAttribute ("iterations"))
    {
      int iter = node->GetAttributeValueAsInt ("iterations");
      factory->SetMaximumIterations (iter);
    }

    if (node->GetAttribute ("motionratio"))
    {
      float ratio = node->GetAttributeValueAsFloat ("motionratio");
      factory->SetMotionRatio (ratio);
    }

    if (node->GetAttribute ("distance"))
    {
      float distance = node->GetAttributeValueAsFloat ("distance");
      factory->SetTargetDistance (distance);
    }

    if (node->GetAttribute ("upward"))
    {
      bool upward = node->GetAttributeValueAsBool ("upward");
      factory->SetUpwardIterations (upward);
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
      case XMLTOKEN_EFFECTOR:
	break;

      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool SkeletonLoader::ParseIKPhysicalNode (iDocumentNode* node, CS::Animation::iSkeletonIKNodeFactory* factnode)
  {
    csRef<CS::Animation::iSkeletonIKPhysicalNodeFactory> factory =
      scfQueryInterface<CS::Animation::iSkeletonIKPhysicalNodeFactory> (factnode);

    if (node->GetAttribute ("chainreset"))
    {
      bool reset = node->GetAttributeValueAsBool ("chainreset");
      factory->SetChainAutoReset (reset);
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
      case XMLTOKEN_EFFECTOR:
	break;

      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  bool SkeletonLoader::ParseEffector (iDocumentNode* node,
				      CS::Animation::iSkeletonIKNodeFactory* factory)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseeffector";

    const char* body = node->GetAttributeValue ("body");
    const char* chain = node->GetAttributeValue ("chain");

    CS::Animation::iBodySkeleton* bodySkeleton = bodyManager->FindBodySkeleton (body);
    if (!bodySkeleton)
    {
      synldr->ReportError (msgid, node, "Could not find body skeleton %s",
			   CS::Quote::Single (body));
      return false;
    }

    CS::Animation::iBodyChain* bodyChain = bodySkeleton->FindBodyChain (chain);
    if (!bodyChain)
    {
      synldr->ReportError (msgid, node, "Could not find body chain %s within skeleton %s",
			   CS::Quote::Single (chain), CS::Quote::Single (body));
      return false;
    }

    const char* bone = node->GetAttributeValue ("bone");
    CS::Animation::BoneID boneID = bodySkeleton->GetSkeletonFactory ()->FindBone (bone);
    if (boneID == CS::Animation::InvalidBoneID)
    {
      synldr->ReportError (msgid, node, "Could not find bone %s in skeleton %s",
			   CS::Quote::Single (bone), body);
      return 0;
    }

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

    factory->AddEffector (bodyChain, boneID, t);

    return true;
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseLookAtNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parselookatnode";

    csRef<CS::Animation::iSkeletonLookAtNodeFactory> factnode;

    csRef<CS::Animation::iSkeletonLookAtNodeManager> lookAtManager =
      csQueryRegistryOrLoad<CS::Animation::iSkeletonLookAtNodeManager> (object_reg,
      "crystalspace.mesh.animesh.animnode.lookat");

    const char* name = node->GetAttributeValue ("name");
    factnode = lookAtManager->CreateAnimNodeFactory (name);

    const char* body = node->GetAttributeValue ("body");
    CS::Animation::iBodySkeleton* bodySkeleton = nullptr;
    if (body)
    {
      bodySkeleton = bodyManager->FindBodySkeleton (body);
      if (!bodySkeleton)
      {
	synldr->ReportError (msgid, node, "Could not find body skeleton %s",
			     CS::Quote::Single (body));
	return 0;
      }
      factnode->SetBodySkeleton (bodySkeleton);
    }

    const char* bone = node->GetAttributeValue ("bone");
    if (bone)
    {
      // Search for a skeleton to find the bone ID
      CS::Animation::BoneID boneID;
      const char* skel = node->GetAttributeValue ("skeleton");
      if (skel)
      {
	CS::Animation::iSkeletonFactory* skeleton = skelManager->FindSkeletonFactory (skel);
	if (!skeleton)
	{
	  synldr->ReportError (msgid, node, "Could not find target skeleton %s",
			       CS::Quote::Single (skel));
	  return 0;
	}

	boneID = skeleton->FindBone (bone);
	if (boneID == CS::Animation::InvalidBoneID)
	{
	  synldr->ReportError (msgid, node, "Could not find bone %s in skeleton %s",
			       CS::Quote::Single (bone), skel);
	  return 0;
	}
      }

      else if (bodySkeleton && bodySkeleton->GetSkeletonFactory ())
      {
	CS::Animation::iSkeletonFactory* skeleton = bodySkeleton->GetSkeletonFactory ();

	boneID = skeleton->FindBone (bone);
	if (boneID == CS::Animation::InvalidBoneID)
	{
	  synldr->ReportError (msgid, node, "Could not find bone %s in bodymesh's skeleton",
			       CS::Quote::Single (bone));
	  return 0;
	}
      }

      else
      {
	synldr->ReportError (msgid, node, "No skeleton factory of bodymesh provided while defining bone %s", bone);
	return 0;
      }

      factnode->SetBone (boneID);
    }

    if (node->GetAttribute ("maxspeed"))
    {
      float speed = node->GetAttributeValueAsFloat ("maxspeed");
      factnode->SetMaximumSpeed (speed);
    }

    if (node->GetAttribute ("alwaysrot"))
    {
      bool rotate = node->GetAttributeValueAsBool ("alwaysrot");
      factnode->SetAlwaysRotate (rotate);
    }

    if (node->GetAttribute ("delay"))
    {
      float delay = node->GetAttributeValueAsFloat ("delay");
      factnode->SetListenerDelay (delay);
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
	  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode =
	    ParseAnimTreeNode (child, packet);

	  if (childNode && factnode->GetChildNode ())
	    synldr->Report (msgid, CS_REPORTER_SEVERITY_WARNING, node,
			    "The LookAt node can only handle one child node");

	  else if (childNode)
	    factnode->SetChildNode (childNode);
	}
	break;

      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    }

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseRagdollNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseragdollnode";

    csRef<CS::Animation::iSkeletonRagdollNodeFactory> factnode;

    csRef<CS::Animation::iSkeletonRagdollNodeManager> ragdollManager =
      csQueryRegistryOrLoad<CS::Animation::iSkeletonRagdollNodeManager> (object_reg,
      "crystalspace.mesh.animesh.animnode.ragdoll");

    const char* name = node->GetAttributeValue ("name");
    factnode = ragdollManager->CreateAnimNodeFactory (name);

    const char* body = node->GetAttributeValue ("body");
    CS::Animation::iBodySkeleton* bodySkeleton = nullptr;
    if (body)
    {
      bodySkeleton = bodyManager->FindBodySkeleton (body);
      if (!bodySkeleton)
      {
	synldr->ReportError (msgid, node, "Could not find body skeleton %s",
			     CS::Quote::Single (body));
	return 0;
      }
      factnode->SetBodySkeleton (bodySkeleton);
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
	  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode =
	    ParseAnimTreeNode (child, packet);

	  if (childNode && factnode->GetChildNode ())
	    synldr->ReportError (msgid, node, "The Ragdoll node can only handle one child node");

	  else if (childNode)
	    factnode->SetChildNode (childNode);
	}
        break;

      case XMLTOKEN_CHAIN:
        {
	  const char* body = child->GetAttributeValue ("body");
	  const char* name = child->GetAttributeValue ("name");

	  CS::Animation::iBodySkeleton* bodySkeleton = bodyManager->FindBodySkeleton (body);
	  if (!bodySkeleton)
	  {
	    synldr->ReportError (msgid, node, "Could not find body skeleton %s",
				 CS::Quote::Single (body));
	    return 0;
	  }

	  CS::Animation::iBodyChain* bodyChain = bodySkeleton->FindBodyChain (name);
	  if (!bodyChain)
	  {
	    synldr->ReportError (msgid, node, "Could not find body chain %s within skeleton %s",
				 CS::Quote::Single (name), CS::Quote::Single (body));
	    return 0;
	  }

	  const char* state = child->GetAttributeValue ("state");
	  CS::Animation::RagdollState ragdollState = CS::Animation::STATE_INACTIVE;
	  if (strcmp (state, "dynamic") == 0)
	    ragdollState = CS::Animation::STATE_DYNAMIC;
	  else if (strcmp (state, "kinematic") == 0)
	    ragdollState = CS::Animation::STATE_KINEMATIC;

	  factnode->AddBodyChain (bodyChain, ragdollState);
	}
        break;

      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    }

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseRetargetNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseretargetnode";

    csRef<CS::Animation::iSkeletonRetargetNodeFactory> factnode;

    csRef<CS::Animation::iSkeletonRetargetNodeManager> retargetManager =
      csQueryRegistryOrLoad<CS::Animation::iSkeletonRetargetNodeManager> (object_reg,
      "crystalspace.mesh.animesh.animnode.retarget");

    const char* name = node->GetAttributeValue ("name");
    factnode = retargetManager->CreateAnimNodeFactory (name);

    const char* skelSource = node->GetAttributeValue ("skelsource");
    CS::Animation::iSkeletonFactory* sourceSkeleton = skelManager->FindSkeletonFactory (skelSource);
    if (!sourceSkeleton)
    {
      synldr->ReportError (msgid, node, "Could not find source skeleton %s",
			   CS::Quote::Single (skelSource));
      return 0;
    }
    factnode->SetSourceSkeleton (sourceSkeleton);

    const char* skelTarget = node->GetAttributeValue ("skeltarget");
    CS::Animation::iSkeletonFactory* targetSkeleton = skelManager->FindSkeletonFactory (skelTarget);
    if (!targetSkeleton)
    {
      synldr->ReportError (msgid, node, "Could not find target skeleton %s",
			   CS::Quote::Single (skelTarget));
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
      case XMLTOKEN_NODE:
        {
	  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode =
	    ParseAnimTreeNode (child, packet);

	  if (childNode && factnode->GetChildNode ())
	    synldr->ReportError (msgid, node, "The Retarget node can only handle one child node");

	  else if (childNode)
	    factnode->SetChildNode (childNode);
	}
        break;

      case XMLTOKEN_CHAIN:
        {
	  const char* body = child->GetAttributeValue ("body");
	  const char* name = child->GetAttributeValue ("name");

	  CS::Animation::iBodySkeleton* bodySkeleton = bodyManager->FindBodySkeleton (body);
	  if (!bodySkeleton)
	  {
	    synldr->ReportError (msgid, node, "Could not find body skeleton %s",
				 CS::Quote::Single (body));
	    return 0;
	  }

	  CS::Animation::iBodyChain* bodyChain = bodySkeleton->FindBodyChain (name);
	  if (!bodyChain)
	  {
	    synldr->ReportError (msgid, node, "Could not find body chain %s within skeleton %s",
				 CS::Quote::Single (name), CS::Quote::Single (body));
	    return 0;
	  }

	  factnode->AddBodyChain (bodyChain);
	}
        break;

      case XMLTOKEN_MAPPING:
        {
	  CS::Animation::BoneMapping mapping;
	  if (!ParseBoneMapping (child, mapping, sourceSkeleton, targetSkeleton))
	    return 0;

	  factnode->SetBoneMapping (mapping);
	}
        break;

      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    }

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

  bool SkeletonLoader::ParseBoneMapping (iDocumentNode* node, CS::Animation::BoneMapping& mapping,
					 CS::Animation::iSkeletonFactory* sourceSkeleton,
					 CS::Animation::iSkeletonFactory* targetSkeleton)
  {
    static const char* msgid = "crystalspace.skeletonloader.parsemapping";

    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
      case XMLTOKEN_NAMEMAP:
	CS::Animation::NameBoneMappingHelper::GenerateMapping (mapping, sourceSkeleton, targetSkeleton);
        break;

      case XMLTOKEN_BONEMAP:
      case XMLTOKEN_NOBONEMAP:
        {
	  const char* source = child->GetAttributeValue ("source");
	  CS::Animation::BoneID sourceID = sourceSkeleton->FindBone (source);
	  if (sourceID == CS::Animation::InvalidBoneID)
	  {
	    synldr->ReportError (msgid, node, "Could not find bone %s in source skeleton",
				 CS::Quote::Single (source));
	    return false;
	  }

	  const char* target = child->GetAttributeValue ("target");
	  CS::Animation::BoneID targetID = targetSkeleton->FindBone (target);
	  if (targetID == CS::Animation::InvalidBoneID)
	  {
	    synldr->ReportError (msgid, node, "Could not find bone %s in target skeleton",
				 CS::Quote::Single (target));
	    return false;
	  }

	  if (id == XMLTOKEN_BONEMAP)
	    mapping.AddMapping (sourceID, targetID);
	  else
	    mapping.RemoveMapping (sourceID, targetID);
	}
        break;

      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

  csPtr<CS::Animation::iSkeletonAnimNodeFactory> SkeletonLoader::ParseSpeedNode (
    iDocumentNode* node, CS::Animation::iSkeletonAnimPacketFactory* packet)
  {
    static const char* msgid = "crystalspace.skeletonloader.parsespeednode";

    csRef<CS::Animation::iSkeletonSpeedNodeFactory> factnode;

    csRef<CS::Animation::iSkeletonSpeedNodeManager> speedManager =
      csQueryRegistryOrLoad<CS::Animation::iSkeletonSpeedNodeManager> (object_reg,
      "crystalspace.mesh.animesh.animnode.speed");

    const char* name = node->GetAttributeValue ("name");
    factnode = speedManager->CreateAnimNodeFactory (name);

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
	  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNode =
	    ParseAnimTreeNode (child, packet);

	  if (!childNode)
	    break;

	  if (!child->GetAttribute ("nodespeed"))
	  {
	    synldr->ReportError (msgid, node, "No speed provided for child of node %s",
				 CS::Quote::Single (name));
	    return 0;
	  }

	  float speed = node->GetAttributeValueAsFloat ("nodespeed");
	  factnode->AddNode (childNode, speed);
	}
        break;

      default:
        synldr->ReportBadToken (child);
        return 0;
      }       
    }

    return csPtr<CS::Animation::iSkeletonAnimNodeFactory> (factnode);
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2Ldr)

