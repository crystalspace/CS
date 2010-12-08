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
#include "csutil/scf.h"

#include "debug.h"
#include "csqint.h"
#include "cstool/cspixmap.h"
#include "iutil/objreg.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/movable.h"
#include "iengine/mesh.h"
#include "iengine/scenenode.h"
#include "imesh/object.h"
#include "imesh/genmesh.h"
#include "ivaria/reporter.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "cstool/materialbuilder.h"

CS_PLUGIN_NAMESPACE_BEGIN(DebugNode)
{

  /********************
   *  DebugNodeManager
   ********************/

  SCF_IMPLEMENT_FACTORY(DebugNodeManager);

  CS_LEAKGUARD_IMPLEMENT(DebugNodeManager);

  DebugNodeManager::DebugNodeManager (iBase* parent)
    : scfImplementationType (this, parent)
  {
  }

  CS::Animation::iSkeletonDebugNodeFactory* DebugNodeManager::CreateAnimNodeFactory (const char* name)
  {
    csRef<CS::Animation::iSkeletonDebugNodeFactory> newFact;
    newFact.AttachNew (new DebugNodeFactory (this, name));

    return debugFactories.PutUnique (name, newFact);
  }

  CS::Animation::iSkeletonDebugNodeFactory* DebugNodeManager::FindAnimNodeFactory (const char* name)
  {
    return debugFactories.Get (name, 0);
  }

  void DebugNodeManager::ClearAnimNodeFactories ()
  {
    debugFactories.DeleteAll ();
  }

  bool DebugNodeManager::Initialize (iObjectRegistry* object_reg)
  {
    this->object_reg = object_reg;
    return true;
  }

  void DebugNodeManager::Report (int severity, const char* msg, ...) const
  {
    va_list arg;
    va_start (arg, msg);
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));
    if (rep)
      rep->ReportV (severity,
		    "crystalspace.mesh.animesh.animnode.debug",
		    msg, arg);
    else
    {
      csPrintfV (msg, arg);
      csPrintf ("\n");
    }
    va_end (arg);
  }

  /********************
   *  DebugNodeFactory
   ********************/

  CS_LEAKGUARD_IMPLEMENT(DebugNodeFactory);

  DebugNodeFactory::DebugNodeFactory (DebugNodeManager* manager, const char *name)
    : scfImplementationType (this), manager (manager), name (name),
    modes (CS::Animation::DEBUG_SQUARES), image (nullptr), boneMaskUsed (false),
    leafBonesDisplayed (true)
  {
  }

  void DebugNodeFactory::SetDebugModes (CS::Animation::SkeletonDebugMode modes)
  {
    this->modes = modes;
  }

  CS::Animation::SkeletonDebugMode DebugNodeFactory::GetDebugModes ()
  {
    return modes;
  }

  void DebugNodeFactory::SetDebugImage (csPixmap* image)
  {
    this->image = image;
  }

  void DebugNodeFactory::SetBoneMask (csBitArray& boneMask)
  {
    boneMaskUsed = true;
    this->boneMask = boneMask;
  }

  void DebugNodeFactory::UnsetBoneMask ()
  {
    boneMaskUsed = false;
  }

  void DebugNodeFactory::SetLeafBonesDisplayed (bool displayed)
  {
    leafBonesDisplayed = displayed;
  }

  void DebugNodeFactory::SetChildNode (CS::Animation::iSkeletonAnimNodeFactory* factory)
  {
    subFactory = factory;
  }

  CS::Animation::iSkeletonAnimNodeFactory* DebugNodeFactory::GetChildNode () const
  {
    return subFactory;
  }

  csPtr<CS::Animation::iSkeletonAnimNode> DebugNodeFactory::CreateInstance
    (CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton)
  {
    // create the node instance
    csRef<DebugNode> newP;
    newP.AttachNew (new DebugNode (this, skeleton));

    if (subFactory)
    {
      csRef<CS::Animation::iSkeletonAnimNode> node =
	subFactory->CreateInstance (packet, skeleton);
      newP->subNode = node;
    }

    return csPtr<CS::Animation::iSkeletonAnimNode> (newP);
  }

  const char* DebugNodeFactory::GetNodeName () const
  {
    return name;
  }

  CS::Animation::iSkeletonAnimNodeFactory* DebugNodeFactory::FindNode (const char* name)
  {
    if (this->name == name)
      return this;

    if (subFactory)
      return subFactory->FindNode (name);

    return 0;
  }

  /********************
   *  DebugNode
   ********************/

  CS_LEAKGUARD_IMPLEMENT(DebugNode);

  DebugNode::DebugNode (DebugNodeFactory* factory, CS::Animation::iSkeleton* skeleton)
    : scfImplementationType (this), factory (factory), skeleton (skeleton)
  {
  }

  void DebugNode::Draw (iCamera* camera, csColor color)
  {
    if (!factory->modes)
      return;

    csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (factory->manager->object_reg);
    csRef<iGraphics2D> g2d = csQueryRegistry<iGraphics2D> (factory->manager->object_reg);
    CS_ASSERT(g3d && g2d);

    // Tell the 3D driver we're going to display 2D things.
    if (!g3d->BeginDraw (CSDRAW_2DGRAPHICS))
      return;

    CS::Animation::iSkeletonFactory* fact = skeleton->GetFactory ();
    float fov = g2d->GetHeight ();
    csReversibleTransform tr_o2c =
      camera->GetTransform () / skeleton->GetSceneNode ()->GetMovable ()->GetFullTransform ();

    // Setup the "end" positions and child count of all bones
    const CS::Animation::BoneID lastId = fact->GetTopBoneID ();
    csArray<csVector3> childPos;
    csArray<int> numChild;

    childPos.SetSize (lastId+1, csVector3 (0));
    numChild.SetSize (lastId+1, 0);

    for (CS::Animation::BoneID i = 0; i < lastId+1; ++i)
    {
      if (!fact->HasBone (i))
	continue;

      CS::Animation::BoneID parent = fact->GetBoneParent (i);
      if (parent != CS::Animation::InvalidBoneID)
      {
	csQuaternion q;
	csVector3 v;
	skeleton->GetTransformBoneSpace (i, q, v);

	childPos[parent] += v;
	numChild[parent] += 1;
      }
    }

    // Now draw the bones
    for (CS::Animation::BoneID i = 0; i <= lastId; i++)
    {
      if (!fact->HasBone (i)
	  || (factory->boneMaskUsed
	      && (factory->boneMask.GetSize () <= i
		  || !factory->boneMask.IsBitSet (i)))
	  || (!factory->leafBonesDisplayed && numChild[i] == 0))
	continue;

      csQuaternion rotation;
      csVector3 position;
      skeleton->GetTransformAbsSpace (i, rotation, position);

      csVector3 bonePosition = tr_o2c * position;

      if (factory->modes & CS::Animation::DEBUG_IMAGES
	  && factory->image)
      {
	float x1 = bonePosition.x, y1 = bonePosition.y, z1 = bonePosition.z;
	float iz1 = fov / z1;
	int px1 = csQint (x1 * iz1 + (g2d->GetWidth ()  / 2));
	int py1 = g2d->GetHeight () - 1 - csQint (y1 * iz1 + (g2d->GetHeight () / 2));
 
	// TODO: images are not drawn correctly
	if (iz1 > 0.0f)
	  factory->image->Draw (g3d, px1 - factory->image->Width () / 2,
				py1 - factory->image->Height () / 2);
      }

      if (factory->modes & CS::Animation::DEBUG_2DLINES)
      {
	csVector3 endLocal;

	if (numChild[i] > 0)
	  endLocal = childPos[i] / numChild[i];
	else
	{
	  endLocal = csVector3 (0.0f, 0.0f, 1.0f);

	  CS::Animation::BoneID parent = fact->GetBoneParent (i);
	  if (parent != CS::Animation::InvalidBoneID)
	    endLocal *= (childPos[parent] / numChild[parent]).Norm ();
	}

	csVector3 endGlobal = position + rotation.Rotate (endLocal);
	csVector3 boneEnd = tr_o2c * endGlobal;

	g3d->DrawLine (bonePosition, boneEnd, fov, g2d->FindRGB (((int) color[0] * 255.0f),
								 ((int) color[1] * 255.0f),
								 ((int) color[2] * 255.0f)));
      }

      if (factory->modes & CS::Animation::DEBUG_SQUARES)
      {
	int colorI = g2d->FindRGB (255.0f * color[0],
				   255.0f * color[1],
				   255.0f * color[2]);

	float x1 = bonePosition.x, y1 = bonePosition.y, z1 = bonePosition.z;
	float iz1 = fov / z1;
	int px1 = csQint (x1 * iz1 + (g2d->GetWidth ()  / 2));
	int py1 = g2d->GetHeight () - 1 - csQint (y1 * iz1 + (g2d->GetHeight () / 2));
 
	if (iz1 > 0.0f)
	{
	  size_t size = 5;
	  for (size_t i = 0; i < size; i++)
	    for (size_t j = 0; j < size; j++)
	      g3d->GetDriver2D ()->DrawPixel (px1 - size / 2 + i,
					      py1 - size / 2 + j,
					      colorI);
	}
      }
    }
  }

  void DebugNode::Play ()
  {
    if (subNode)
      subNode->Play ();
  }

  void DebugNode::Stop ()
  {
    if (subNode)
      subNode->Stop ();
  }

  void DebugNode::SetPlaybackPosition (float time)
  {
    if (subNode)
      subNode->SetPlaybackPosition (time);
  }

  float DebugNode::GetPlaybackPosition () const
  {
    if (subNode)
      return subNode->GetPlaybackPosition ();

    return 0.0f;
  }

  float DebugNode::GetDuration () const
  {
    if (subNode)
      return subNode->GetDuration ();

    return 0.0f;
  }

  void DebugNode::SetPlaybackSpeed (float speed)
  {
    if (subNode)
      subNode->SetPlaybackSpeed (speed);
  }

  float DebugNode::GetPlaybackSpeed () const
  {
    if (subNode)
      return subNode->GetPlaybackSpeed ();

    return 0.0f;
  }

  void DebugNode::BlendState (CS::Animation::csSkeletalState* state,
			      float baseWeight)
  {
    if (subNode)
      subNode->BlendState (state, baseWeight);
  }

  void DebugNode::TickAnimation (float dt)
  {
    if (subNode)
      subNode->TickAnimation (dt);
  }

  bool DebugNode::IsActive () const
  {
    if (subNode)
      return subNode->IsActive ();

    return false;
  }

  CS::Animation::iSkeletonAnimNodeFactory* DebugNode::GetFactory () const
  {
    return factory;
  }

  CS::Animation::iSkeletonAnimNode* DebugNode::FindNode (const char* name)
  {
    if (factory->name == name)
      return this;

    if (subNode)
      return subNode->FindNode (name);

    return 0;
  }

  void DebugNode::AddAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    if (subNode)
      subNode->AddAnimationCallback (callback);
  }

  void DebugNode::RemoveAnimationCallback (CS::Animation::iSkeletonAnimCallback* callback)
  {
    if (subNode)
      subNode->RemoveAnimationCallback (callback);
  }

}
CS_PLUGIN_NAMESPACE_END(DebugNode)
