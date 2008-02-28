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
#include "imesh/nskeleton.h"
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

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_BONE = 1,
  XMLTOKEN_POSITION,
  XMLTOKEN_ROTATION,
  XMLTOKEN_ANIMATION,
  XMLTOKEN_CHANNEL,
  XMLTOKEN_FRAME
};

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton)
{

SCF_IMPLEMENT_FACTORY (Loader)

Loader::Loader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

Loader::~Loader ()
{
}

bool Loader::Initialize (iObjectRegistry* object_reg)
{
  Loader::object_reg = object_reg;
  reporter = csQueryRegistry<iReporter> (object_reg);
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  xmltokens.Register ("bone", XMLTOKEN_BONE);
  xmltokens.Register ("position", XMLTOKEN_POSITION);
  xmltokens.Register ("rotation", XMLTOKEN_ROTATION);
  xmltokens.Register ("animation", XMLTOKEN_ANIMATION);
  xmltokens.Register ("channel", XMLTOKEN_CHANNEL);
  xmltokens.Register ("frame", XMLTOKEN_FRAME);

  return true;
}

bool Loader::ParseBone (iDocumentNode* node,
  ::Skeleton::iSkeletonFactory* skelfact,
  ::Skeleton::iSkeletonFactory::iBoneFactory* parent)
{
  if (!parent)  // the root bone!
  {
    int size = node->GetAttributeValueAsInt ("numbones");
    skelfact->SetNumberOfBones (size);
  }

  const char* bonename = node->GetAttributeValue ("name");
  ::Skeleton::iSkeletonFactory::iBoneFactory* bone =
      skelfact->GetBoneFactory (currbone);
  bone->SetName (bonename);
  currbone++;
  csQuaternion rot;
  csVector3 pos (0);

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
        ParseBone (child, skelfact, bone);
        break;
      case XMLTOKEN_POSITION:
        pos.x = child->GetAttributeValueAsFloat ("x");
        pos.y = child->GetAttributeValueAsFloat ("y");
        pos.z = child->GetAttributeValueAsFloat ("z");
        break;
      case XMLTOKEN_ROTATION:
        rot.v.x = child->GetAttributeValueAsFloat ("x");
        rot.v.y = child->GetAttributeValueAsFloat ("y");
        rot.v.z = child->GetAttributeValueAsFloat ("z");
        rot.w = child->GetAttributeValueAsFloat ("w");
        break;
    }
  }
  bone->SetRotation (rot);
  bone->SetPosition (pos);
  if (parent)
    parent->AddChild (bone);
  return true;
}

bool Loader::ParseAnimation (iDocumentNode* node,
  ::Skeleton::iSkeletonFactory* skelfact)
{
  csRef< ::Skeleton::Animation::iAnimationFactoryLayer> animfactlay =
    skelfact->GetAnimationFactoryLayer ();
  csRef< ::Skeleton::Animation::iAnimationFactory> animfact =
    animfactlay->CreateAnimationFactory ();
  const char* animname = node->GetAttributeValue ("name");
  animfact->SetName (animname);
  float animlength = 0.0f;

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
        csRef< ::Skeleton::Animation::iChannel> animchan =
          animfactlay->CreateAnimationFactoryChannel ();
        const char* channame = child->GetAttributeValue ("name");
        int boneid = skelfact->FindBoneFactoryIDByName (channame);
        animchan->SetID (boneid);
        float chanlength = 0.0f;
        ParseChannel (child, animchan, chanlength);
        if (chanlength > animlength)
          animlength = chanlength;
        animfact->AddChannel (animchan);
        break;
      }
    }
  }
  animfact->SetAnimationLength (animlength);
  return true;
}

bool Loader::ParseChannel (iDocumentNode* node,
  csRef< ::Skeleton::Animation::iChannel> animchan, float &chanlength)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FRAME:
      {
        csVector3 pos;
        csQuaternion rot;
        ParsePositionRotation (child, pos, rot);
        float time = child->GetAttributeValueAsFloat ("time");
        if (time > chanlength)
          chanlength = time;
        animchan->AddKeyframe (time,
          csTuple2<csQuaternion, csVector3> (rot, pos));
        break;
      }
    }
  }
  return true;
}

bool Loader::ParsePositionRotation (iDocumentNode* node, csVector3 &pos,
  csQuaternion &rot)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_POSITION:
        pos.x = child->GetAttributeValueAsFloat ("x");
        pos.y = child->GetAttributeValueAsFloat ("y");
        pos.z = child->GetAttributeValueAsFloat ("z");
        break;
      case XMLTOKEN_ROTATION:
        rot.v.x = child->GetAttributeValueAsFloat ("x");
        rot.v.y = child->GetAttributeValueAsFloat ("y");
        rot.v.z = child->GetAttributeValueAsFloat ("z");
        rot.w = child->GetAttributeValueAsFloat ("w");
        break;
    }
  }
  return true;
}

csPtr<iBase> Loader::Parse (iDocumentNode* node,
  iStreamSource*, iLoaderContext* ldr_context, iBase* context)
{
  csRef< ::Skeleton::iGraveyard> skelgrave =
    csQueryRegistry< ::Skeleton::iGraveyard> (object_reg);
  if (!skelgrave)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.isotest", "Can't find the graveyard!");
    return false;
  }

  csRef<iDocumentNode> skelfact_node = node->GetNode("skeletonfactory");
  if (!skelfact_node)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
      "crystalspace.application.isotest",
      "Badly formed Xml. Missing <skeletonfactory>!");
    return false;
  }

  const char* fact_name = skelfact_node->GetAttributeValue ("name");
  if (!fact_name)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_WARNING,
      "crystalspace.application.isotest",
      "Missing name for <skeletonfactory>!");
  }
  ::Skeleton::iSkeletonFactory *skelfact = skelgrave->CreateFactory (fact_name);
  bool bones_loaded = false;
  currbone = 0;

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
      {
        /* ignore every <bone> markup except the first since we
        only allow one root bone */
        if (!bones_loaded)
        {
          ParseBone (child, skelfact, 0);
          skelfact->SetRootBone (0);
          bones_loaded = true;
        }
        break;
      }
      case XMLTOKEN_ANIMATION:
        ParseAnimation (child, skelfact);
        break;
    }
  }
  skelfact->IncRef ();
  return csPtr<iBase> ((iBase*)skelfact);
}

}
CS_PLUGIN_NAMESPACE_END(Skeleton)
