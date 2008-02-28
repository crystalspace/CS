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
#include "csgeom/vector4.h"
#include "csutil/event.h"
#include "csutil/eventnames.h"
#include "csutil/util.h"
#include "imap/services.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iutil/evdefs.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"

#include "skeleton.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton)
{

SCF_IMPLEMENT_FACTORY (Graveyard)

void Bone::SetRotation (const csQuaternion &r)
{
  rot = r;
}
const csQuaternion &Bone::GetRotation () const
{
  return rot;
}

void Bone::SetPosition (const csVector3 &p)
{
  pos = p;
}
const csVector3 &Bone::GetPosition () const
{
  return pos;
}

::Skeleton::iSkeletonFactory::iBoneFactory* Bone::GetFactory () const
{
  return fact;
}

void Bone::CalculateLocalTransform ()
{
  transform.SetOrigin (pos);
  transform.SetO2T (csMatrix3 (rot));
  //transform.SetOrigin (pos);
}
void Bone::UpdateTransform ()
{
  for (csArray<iBone*>::Iterator it = children.GetIterator (); it.HasNext (); )
  {
    iBone* &b = it.Next ();
    b->UpdateTransform (transform);
  }
}
void Bone::UpdateTransform (const csReversibleTransform &trans)
{
  transform = transform * trans;
  UpdateTransform ();
}

const csReversibleTransform &Bone::GetTransform () const
{
  return transform;
}

void Bone::SetParent (iBone* p)
{
  parent = p;
}

const size_t Bone::GetChildrenCount () const
{
  return children.GetSize ();
}
const ::Skeleton::iSkeleton::iBone* Bone::GetChild (size_t i) const
{
  return children.Get (i);
}

Skeleton::Skeleton (const char* name)
  : scfImplementationType (this), name (name), fact (0)
{
  animation_layer.AttachNew (new SkeletonAnimation::AnimationLayer ());
}

void Skeleton::SetName (const char* n)
{
  name = n;
}
const char* Skeleton::GetName ()
{
  return name;
}

void Skeleton::Update (float delta_time)
{
  animation_layer->UpdateSkeleton (this, delta_time);
  for (csArray<Bone>::Iterator it = bones.GetIterator (); it.HasNext (); )
  {
    Bone &b = it.Next ();
    b.CalculateLocalTransform ();
  }
  bones.Get (root_bone).UpdateTransform ();
}
void Skeleton::DrawDebugBones (iGraphics3D* g3d) const
{
  for (csArray<Bone>::ConstIterator it = bones.GetIterator (); it.HasNext (); )
  {
    const Bone &b = it.Next ();
    const csReversibleTransform &rt = b.GetTransform ();
    float length = 0.1f;
    if (b.GetChildrenCount () > 0)
    {
      const iBone *firstchild = b.GetChild (0);
      length = (firstchild->GetTransform ().GetOrigin () - rt.GetOrigin ()).Norm ();
    }
    DrawBone (g3d, length, rt);
    for (size_t j = 0; j < b.GetChildrenCount (); j++)
    {
      const iBone* bone = b.GetChild (j);
      csSimpleRenderMesh mesh;
      mesh.object2world.Identity ();
      csVector3 verts[2];
      verts[0] = rt.GetOrigin ();
      verts[1] = bone->GetTransform ().GetOrigin ();
      csVector4 colours[2];
      colours[0].Set (1, 0, 0, 1);
      colours[1].Set (0, 0, 1, 1);
      mesh.vertices = verts;
      mesh.colors = colours;
      mesh.vertexCount = 2;
      mesh.meshtype = CS_MESHTYPE_LINES;
      g3d->DrawSimpleMesh (mesh, 0);
    }
  }
}
void Skeleton::DrawBone (iGraphics3D* g3d, float length, const csReversibleTransform &rt) const
{
  csSimpleRenderMesh mesh;
  mesh.object2world = rt;

  float w = length/10;
  csVector3 verts[16];
  verts[0].Set (0, -w, -w);
  verts[1].Set (0, w, -w);
  verts[2].Set (0, -w, w);
  verts[3].Set (0, w, w);
  verts[4].Set (0, -w, -w);
  verts[5].Set (0, -w, w);
  verts[6].Set (0, w, -w);
  verts[7].Set (0, w, w);

  verts[8].Set (0, -w, -w);
  verts[9].Set (length, 0, 0);
  verts[10].Set (0, w, -w);
  verts[11].Set (length, 0, 0);
  verts[12].Set (0, -w, w);
  verts[13].Set (length, 0, 0);
  verts[14].Set (0, w, w);
  verts[15].Set (length, 0, 0);

  mesh.vertices = verts;
  mesh.vertexCount = 16;
  mesh.meshtype = CS_MESHTYPE_LINES;
  g3d->DrawSimpleMesh (mesh, 0);
}

const size_t Skeleton::GetChildrenCount () const
{
  return bones.GetSize ();
}
const ::Skeleton::iSkeleton::iBone* Skeleton::GetChild (size_t i) const
{
  return &bones.Get (i);
}
::Skeleton::iSkeleton::iBone* Skeleton::GetChild (size_t i)
{
  return &bones.Get (i);
}

::Skeleton::iSkeleton::iBone* Skeleton::FindBoneByName (const char* name)
{
  for (csArray<Bone>::Iterator it = bones.GetIterator (); it.HasNext ();)
  {
    Bone *bone = &it.Next ();
    if (!strcmp (bone->GetFactory ()->GetName (), name))
      return bone;
  }
  return 0;
}
int Skeleton::FindBoneIDByName (const char* name) const
{
  for (size_t i = 0; i < bones.GetSize (); i++)
  {
    if (!strcmp (bones.Get (i).GetFactory ()->GetName (), name))
      return i;
  }
  return -1;
}
size_t Skeleton::GetBoneCount ()
{
  return bones.GetSize ();
}
::Skeleton::iSkeleton::iBone* Skeleton::GetBone (size_t i)
{
  return &bones[i];
}

::Skeleton::iSkeletonFactory* Skeleton::GetFactory () const
{
  return fact;
}
::Skeleton::Animation::iAnimationLayer* Skeleton::GetAnimationLayer ()
{
  return animation_layer;
}

BoneFactory::BoneFactory (const char* name)
  : name (name), parent (0)
{
}
BoneFactory::BoneFactory ()
  : parent (0)
{
}

void BoneFactory::SetRotation (const csQuaternion &r)
{
  rot = r;
}
const csQuaternion &BoneFactory::GetRotation ()
{
  return rot;
}

void BoneFactory::SetPosition (const csVector3 &p)
{
  pos = p;
}
const csVector3 &BoneFactory::GetPosition ()
{
  return pos;
}

void BoneFactory::SetName (const char* n)
{
  name = n;
}
const char* BoneFactory::GetName () const
{
  return name;
}

void SkeletonFactory::SetNumberOfBones (size_t n)
{
  bones.SetSize (n);
  bones.ShrinkBestFit ();
}

void BoneFactory::SetParent (::Skeleton::iSkeletonFactory::iBoneFactory* p)
{
  parent = p;
}
::Skeleton::iSkeletonFactory::iBoneFactory* BoneFactory::GetParent ()
{
  return parent;
}

void BoneFactory::AddChild (::Skeleton::iSkeletonFactory::iBoneFactory* child)
{
  children.PushSmart (child);
  child->SetParent (this);
}

const size_t BoneFactory::GetChildrenCount () const
{
  return children.GetSize ();
}
const ::Skeleton::iSkeletonFactory::iBoneFactory* BoneFactory::GetChild (size_t i) const
{
  return children.Get (i);
}

SkeletonFactory::SkeletonFactory (const char* name)
  : scfImplementationType (this), name (name)
{
  animation_factory_layer.AttachNew (
    new SkeletonAnimation::AnimationFactoryLayer ());
}

::Skeleton::iSkeleton* SkeletonFactory::CreateSkeleton (const char* name)
{
  Skeleton* skel = new Skeleton (name);
  skel->fact = this;
  skel->root_bone = root_bone;
  skel->bones.SetSize (bones.GetSize ());
  skel->bones.ShrinkBestFit ();
  bones.ShrinkBestFit ();
  for (size_t i = 0; i < bones.GetSize (); i++)
  {
    BoneFactory &bonefact = bones.Get (i);
    Bone &bone = skel->bones.Get (i);
    bone.SetPosition (bonefact.GetPosition ());
    bone.SetRotation (bonefact.GetRotation ());
    bone.fact = &bonefact;
  }
  // now create the hierarchy
  for (size_t i = 0; i < bones.GetSize (); i++)
  {
    BoneFactory &bonefact = bones.Get (i);
    Bone &bone = skel->bones.Get (i);
    if (!bonefact.GetParent ())
      bone.parent = 0;
    else
      bone.parent = skel->FindBoneByName (bonefact.GetParent ()->GetName ());
    for (size_t j = 0; j < bonefact.GetChildrenCount (); j++)
    {
      const ::Skeleton::iSkeletonFactory::iBoneFactory* childfact = bonefact.GetChild (j);
      ::Skeleton::iSkeleton::iBone* child = skel->FindBoneByName (childfact->GetName ());
      bone.children.Push (child);
      child->SetParent (&bone);
    }
  }
  return skel;
}

void SkeletonFactory::SetName (const char* n)
{
  name = n;
}
const char* SkeletonFactory::GetName ()
{
  return name;
}

::Skeleton::iSkeletonFactory::iBoneFactory*
  SkeletonFactory::FindBoneFactoryByName (const char* name)
{
  for (csArray<BoneFactory>::Iterator it = bones.GetIterator (); it.HasNext ();)
  {
    BoneFactory *bonefact = &it.Next ();
    if (!strcmp (bonefact->GetName (), name))
      return bonefact;
  }
  return 0;
}
int SkeletonFactory::FindBoneFactoryIDByName (const char* name) const
{
  for (size_t i = 0; i < bones.GetSize (); i++)
  {
    if (!strcmp (bones.Get (i).GetName (), name))
      return i;
  }
  return -1;
}
size_t SkeletonFactory::GetBoneFactoryCount ()
{
  return bones.GetSize ();
}
::Skeleton::iSkeletonFactory::iBoneFactory*
  SkeletonFactory::GetBoneFactory (size_t i)
{
  return &bones[i];
}

void SkeletonFactory::SetRootBone (size_t root)
{
  root_bone = root;
}
size_t SkeletonFactory::GetRootBone ()
{
  return root_bone;
}

::Skeleton::Animation::iAnimationFactoryLayer*
  SkeletonFactory::GetAnimationFactoryLayer ()
{
  return animation_factory_layer;
}

Graveyard::Graveyard (iBase *parent)
  : scfImplementationType (this, parent), last_time (0)
{
}
Graveyard::~Graveyard ()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q = csQueryRegistry<iEventQueue> (object_reg);
    if (q)
      q->RemoveListener (scfiEventHandler);
  }
}

bool Graveyard::Initialize (iObjectRegistry* obj_reg)
{
  object_reg = obj_reg;
  clock = csQueryRegistry<iVirtualClock> (object_reg);
  if (!clock)
    return false;
  preprocess = csevPreProcess (object_reg);
  csRef<iEventQueue> eq (csQueryRegistry<iEventQueue> (object_reg));
  if (!eq)
    return false;
  scfiEventHandler.AttachNew (new EventHandler (this));
  eq->RegisterListener (scfiEventHandler, preprocess);
  return true;
}

::Skeleton::iSkeletonFactory* Graveyard::CreateFactory (const char* name)
{
  csRef<SkeletonFactory> fact;
  fact.AttachNew (new SkeletonFactory (name));
  factories.Push (fact);
  return fact;
}

::Skeleton::iSkeleton* Graveyard::CreateSkeleton (const char* name,
  const char* factname)
{
  ::Skeleton::iSkeletonFactory* fact = FindFactory (factname);
  if (!fact)
    return 0;
  csRef<Skeleton> skel;
  skel.AttachNew (static_cast<Skeleton*>(fact->CreateSkeleton (name)));
  skeletons.Push (skel);
  return skel;
}

::Skeleton::iSkeletonFactory* Graveyard::FindFactory (const char* name)
{
  for (csRefArray<SkeletonFactory>::Iterator it = factories.GetIterator ();
    it.HasNext (); )
  {
    SkeletonFactory *fact = it.Next ();
    if (!strcmp (fact->GetName (), name))
      return fact;
  }
  return 0;
}

bool Graveyard::HandleEvent (iEvent& event)
{
  if (event.Name == preprocess)
  {
    size_t timenow = clock->GetCurrentTicks (), delta = timenow - last_time;
    for (csRefArray<Skeleton>::Iterator it = skeletons.GetIterator ();
      it.HasNext (); )
    {
      Skeleton *skel = it.Next ();
      skel->Update (delta);
    }
    last_time = timenow;
    return true;
  }
  return false;
}

/// @@ GENJIX @@
void Graveyard::Debug ()
{
  for (csRefArray<SkeletonFactory>::Iterator it = factories.GetIterator (); it.HasNext (); )
  {
    SkeletonFactory *fact = it.Next ();
    fact->Debug ();
  }
  for (csRefArray<Skeleton>::Iterator it = skeletons.GetIterator (); it.HasNext (); )
  {
    Skeleton *skel = it.Next ();
    skel->Debug ();
  }
}

/// @@ GENJIX @@
void SkeletonFactory::Debug ()
{
  printf ("Factory %s\n", name.GetData ());
  printf ("  Root %u\n", root_bone);
  for (csArray<BoneFactory>::Iterator it = bones.GetIterator (); it.HasNext (); )
  {
    BoneFactory &bf = it.Next ();
    //bf.Debug ();
  }
  animation_factory_layer->Debug ();
}

/// @@ GENJIX @@
void BoneFactory::Debug ()
{
  printf ("  Bone Factory %s\n", name.GetData ());
  printf ("    Rotation (%f, %f, %f, %f)\n", rot.v.x, rot.v.y, rot.v.z, rot.w);
  printf ("    Position (%s)\n", pos.Description ().GetData ());
  if (parent)
    printf ("    Parent %s\n", parent->GetName ());
  else
    puts ("    Parent (None)\n");
  for (csArray<iBoneFactory*>::Iterator it = children.GetIterator (); it.HasNext (); )
  {
    iBoneFactory* &c = it.Next ();
    printf ("    Child %s\n", c->GetName ());
  }
}

/// @@ GENJIX @@
void Skeleton::Debug ()
{
  printf ("Skeleton %s\n", name.GetData ());
  printf ("  Root %u\n", root_bone);
  for (csArray<Bone>::Iterator it = bones.GetIterator (); it.HasNext (); )
  {
    Bone &b = it.Next ();
    //b.Debug ();
  }
}

/// @@ GENJIX @@
void Bone::Debug ()
{
  printf ("  Bone %s\n", fact->GetName ());
  printf ("    Rotation (%f, %f, %f, %f)\n", rot.v.x, rot.v.y, rot.v.z, rot.w);
  printf ("    Position (%s)\n", pos.Description ().GetData ());
  if (parent)
    printf ("    Parent %s\n", parent->GetFactory ()->GetName ());
  else
    puts ("    Parent (None)\n");
  for (csArray<iBone*>::Iterator it = children.GetIterator (); it.HasNext (); )
  {
    iBone* &c = it.Next ();
    printf ("    Child %s\n", c->GetFactory ()->GetName ());
  }
}

}
CS_PLUGIN_NAMESPACE_END(Skeleton)
