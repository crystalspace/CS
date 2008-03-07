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

#ifndef __CS_NSKELETON_H__
#define __CS_NSKELETON_H__

#define __CS_SKELETAL_DEBUG

#include "csgeom/transfrm.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "imesh/nskeleton.h"

#include "animation.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton)
{

class SkeletonFactory;

class Bone : public ::Skeleton::iSkeleton::iBone
{
public:
  void SetRotation (const csQuaternion &rot);
  const csQuaternion &GetRotation () const;

  void SetPosition (const csVector3 &pos);
  const csVector3 &GetPosition () const;

  ::Skeleton::iSkeletonFactory::iBoneFactory* GetFactory () const;

  void CalculateLocalTransform ();
  void UpdateTransform ();
  void UpdateTransform (const csReversibleTransform &trans);

  const csReversibleTransform &GetTransform () const;

  void SetParent (iBone* p);

  const size_t GetChildrenCount () const;
  const iBone* GetChild (size_t i) const;

  /// @@ GENJIX @@
  void Debug ();
private:
  friend class SkeletonFactory;
  ::Skeleton::iSkeletonFactory::iBoneFactory* fact;
  csQuaternion rot;
  csVector3 pos;
  csReversibleTransform transform;

  iBone* parent;
  csArray<iBone*> children;
};

class Skeleton :
  public scfImplementation1<Skeleton, ::Skeleton::iSkeleton>
{
public:
  Skeleton (const char* name);

  void SetName (const char* name);
  const char* GetName ();

  void Update (float delta_time);
  void DrawDebugBones (iGraphics3D* g3d) const;
  void DrawBone (iGraphics3D* g3d, float length, const csReversibleTransform &rt) const;

  const size_t GetChildrenCount () const;
  const iBone* GetChild (size_t i) const;
  iBone* GetChild (size_t i);

  iBone* FindBoneByName (const char* name);
  int FindBoneIDByName (const char* name) const;
  size_t GetBoneCount ();
  iBone* GetBone (size_t i);

  ::Skeleton::iSkeletonFactory* GetFactory () const;
  ::Skeleton::Animation::iAnimationLayer* GetAnimationLayer ();

  /// @@ GENJIX @@
  void Debug ();
private:
  csString name;
  csArray<Bone> bones;
  size_t root_bone;
  friend class SkeletonFactory;
  SkeletonFactory* fact;
  csRef<SkeletonAnimation::AnimationLayer> animation_layer;
};

class BoneFactory : public ::Skeleton::iSkeletonFactory::iBoneFactory
{
public:
  BoneFactory (const char* name);
  BoneFactory ();

  void SetRotation (const csQuaternion &rot);
  const csQuaternion &GetRotation ();

  void SetPosition (const csVector3 &pos);
  const csVector3 &GetPosition ();

  void SetName (const char* name);
  const char* GetName () const;

  void SetParent (iBoneFactory* parent);
  iBoneFactory* GetParent ();

  void AddChild (iBoneFactory* child);

  const size_t GetChildrenCount () const;
  const iBoneFactory* GetChild (size_t i) const;

  /// @@ GENJIX @@
  void Debug ();
private:
  csString name;
  csQuaternion rot;
  csVector3 pos;

  iBoneFactory* parent;
  csArray<iBoneFactory*> children;
};

class SkeletonFactory :
  public scfImplementation1<SkeletonFactory, ::Skeleton::iSkeletonFactory>
{
public:
  SkeletonFactory (const char* name);

  csPtr< ::Skeleton::iSkeleton> CreateSkeleton (const char* name);

  void SetName (const char* name);
  const char* GetName ();

  void SetNumberOfBones (size_t n);

  iBoneFactory* FindBoneFactoryByName (const char* name);
  int FindBoneFactoryIDByName (const char* name) const;
  iBoneFactory* CreateBoneFactory (const char* name);
  size_t GetBoneFactoryCount ();
  iBoneFactory* GetBoneFactory (size_t i);

  void SetRootBone (size_t root);
  size_t GetRootBone ();

  ::Skeleton::Animation::iAnimationFactoryLayer* GetAnimationFactoryLayer ();

  /// @@ GENJIX @@
  void Debug ();
private:
  csString name;
  csArray<BoneFactory> bones;
  csRef<SkeletonAnimation::AnimationFactoryLayer> animation_factory_layer;
  size_t root_bone;
};

class Graveyard :
  public scfImplementation2<Graveyard, ::Skeleton::iGraveyard, iComponent>
{
public:
  Graveyard (iBase *parent);
  virtual ~Graveyard ();

  bool Initialize (iObjectRegistry* object_reg);

  ::Skeleton::iSkeletonFactory* CreateFactory (const char* name);
  ::Skeleton::iSkeleton* CreateSkeleton (const char* name,
    const char* factname);

  ::Skeleton::iSkeletonFactory* FindFactory (const char* name);

  bool HandleEvent (iEvent& Event);

  /// @@ GENJIX @@
  void Debug ();

  struct EventHandler : public scfImplementation1<EventHandler, iEventHandler>
  {
  private:
    csWeakRef<Graveyard> parent;
  public:
    EventHandler (Graveyard* parent) : scfImplementationType (this),
      parent (parent) {}
    virtual ~EventHandler () {}
    virtual bool HandleEvent (iEvent& ev)
    {
      return parent ? parent->HandleEvent (ev) : false;
    }
    CS_EVENTHANDLER_NAMES("crystalspace.nskeleton.graveyard")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  };
  csRef<EventHandler> scfiEventHandler;
private:
  csRefArray<SkeletonFactory> factories;
  csRefArray<Skeleton> skeletons;

  iObjectRegistry* object_reg;
  csRef<iVirtualClock> clock;
  size_t last_time;
  csEventID preprocess;
};

}
CS_PLUGIN_NAMESPACE_END(Skeleton)

#endif // __CS_NSKELETON_H__
