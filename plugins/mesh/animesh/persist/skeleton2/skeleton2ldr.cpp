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
        ParseSkeleton (child);
        break;
      case XMLTOKEN_ANIMATIONTREE:
        ParseAnimTree (child);
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }
    
    return 0;
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

  iSkeletonFactory2* SkeletonLoader::ParseSkeleton (iDocumentNode* node)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseskeleton";

    iSkeletonFactory2* factory = 0;

    // Get common properties
    const char* name = node->GetAttributeValue ("name");
    if (!name)
    {
      synldr->ReportError (msgid, node, "No name set for skeleton");
      return 0;
    }

    factory = skelManager->CreateSkeletonFactory (name);

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
        ParseBone (child, factory, InvalidBoneID);
        break;
      case XMLTOKEN_ANIMATIONTREE:
        {
          csRef<iSkeletonAnimationNodeFactory2> nf;

          const char* reference = child->GetAttributeValue ("ref");          

          if (reference)
          {
            // Referencing an already loaded one, use that
            nf = skelManager->FindAnimationTree (reference);
            if (!nf)
            {
              synldr->ReportError (msgid, node, 
                "Could not find animation tree named %s", reference);
              return 0;
            }
          }
          else
          {
            nf = ParseAnimTree (child);

            if (!nf)
            {
              synldr->ReportError (msgid, node, "Error parsing animation tree");
              return 0;
            }
          }

          factory->SetAnimationRoot (nf);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return 0;
      }
    }
    
    return factory;
  }

  csRef<iSkeletonAnimationNodeFactory2> SkeletonLoader::ParseAnimTree (iDocumentNode* node)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimtree";
    csRef<iSkeletonAnimationNodeFactory2> rootNode;

    const char* name = node->GetAttributeValue ("name");

    csRef<iDocumentNodeIterator> it = node->GetNodes ("node");
    if (it->HasNext ())
    {
      rootNode = ParseaAnimTreeNode (it->Next ());
    }

    if (name && rootNode)
    {
      skelManager->RegisterAnimationTree (rootNode, name);
    }

    return rootNode;
  }

  void SkeletonLoader::ParseBone (iDocumentNode* node, iSkeletonFactory2* factory, BoneID parent)
  {
    static const char* msgid = "crystalspace.skeletonloader.parsebone";

    BoneID id = factory->CreateBone (parent);

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
        ParseBone (child, factory, id);
        break;
      case XMLTOKEN_TRANSFORM:
        {
          csVector3 offs;
          if (!synldr->ParseVector (child, offs))
          {
            synldr->ReportError (msgid, child, "Couldn't parse transform");
            return;
          }

          // Get the rotation part
          csQuaternion q;
          q.v.x = child->GetAttributeValueAsFloat ("qx");
          q.v.y = child->GetAttributeValueAsFloat ("qy");
          q.v.z = child->GetAttributeValueAsFloat ("qz");
          q.w = child->GetAttributeValueAsFloat ("qw");

          factory->SetTransformBoneSpace (id, q, offs);
        }
        break;
      default:
        synldr->ReportBadToken (child);
        return;
      }
    }
  }

  csPtr<iSkeletonAnimationNodeFactory2> SkeletonLoader::ParseaAnimTreeNode (
    iDocumentNode* node)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimtreenode";

    csRef<iSkeletonAnimationNodeFactory2> result;

    const char* type = node->GetAttributeValue ("type");

    // Split up on type...
    csStringID id = xmltokens.Request (type);
    switch (id)
    {
    case XMLTOKEN_ANIMATION:
      {
        csRef<iSkeletonAnimationFactory2> anode;
        anode = skelManager->CreateAnimationFactory ();
        if (!ParseAnimation (node, anode))
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
        bnode = skelManager->CreateBlendNodeFactory ();

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
              csRef<iSkeletonAnimationNodeFactory2> childFact = 
                ParseaAnimTreeNode (child);

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

    return csPtr<iSkeletonAnimationNodeFactory2> (result);
  }

  bool SkeletonLoader::ParseAnimation (iDocumentNode* node, iSkeletonAnimationFactory2* fact)
  {
    static const char* msgid = "crystalspace.skeletonloader.parseanimation";


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

          ChannelID cID = fact->AddChannel (boneid);

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
                  return false;
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
              return false;
            }
          }

        }
        break;      
      default:
        synldr->ReportBadToken (child);
        return false;
      }
    }

    return true;
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2Ldr)

