/*
    Copyright (C) 2005 by Hristo Hristov

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

#ifndef __CS_INSKELETON_H__
#define __CS_INSKELETON_H__

#include "csgeom/quaternion.h"
#include "csgeom/transfrm.h"
#include "csutil/scf_interface.h"

struct iGraphics3D;

namespace Skeleton
{

namespace Animation
{
struct iAnimationLayer;
struct iAnimationFactoryLayer;
}

struct iSkeleton;

struct iSkeletonFactory : public virtual iBase
{
  SCF_INTERFACE (iSkeletonFactory, 0, 0, 1);

  virtual iSkeleton* CreateSkeleton (const char* name) = 0;

  virtual void SetName (const char* name) = 0;
  virtual const char* GetName () = 0;

  struct iBoneFactory
  {
    virtual ~iBoneFactory () {}

    virtual void SetRotation (const csQuaternion &rot) = 0;
    virtual const csQuaternion &GetRotation () = 0;

    virtual void SetPosition (const csVector3 &pos) = 0;
    virtual const csVector3 &GetPosition () = 0;

    virtual void SetName (const char* name) = 0;
    virtual const char* GetName () const = 0;

    virtual void SetParent (iBoneFactory* parent) = 0;
    virtual iBoneFactory* GetParent () = 0;

    virtual void AddChild (iBoneFactory* child) = 0;

    /// @@ GENJIX @@
    virtual void Debug () = 0;
  };

  virtual iBoneFactory* FindBoneFactoryByName (const char* name) = 0;
  virtual int FindBoneFactoryIDByName (const char* name) const = 0;
  virtual iBoneFactory* CreateBoneFactory (const char* name) = 0;
  virtual size_t GetBoneFactoryCount () = 0;
  virtual iBoneFactory* GetBoneFactory (size_t i) = 0;

  virtual void SetRootBone (size_t root) = 0;
  virtual size_t GetRootBone () = 0;

  virtual Animation::iAnimationFactoryLayer* GetAnimationFactoryLayer () = 0;

  /// @@ GENJIX @@
  virtual void Debug () = 0;
};

struct iSkeleton : public virtual iBase
{
  SCF_INTERFACE (iSkeleton, 0, 0, 1);

  virtual void Update (float delta_time) = 0;

  /**
   * Debug function to draw the bones.
   */
  virtual void DrawDebugBones (iGraphics3D* g3d) const = 0;

  struct iBone
  {
    virtual ~iBone () {}

    virtual void SetRotation (const csQuaternion &rot) = 0;
    virtual const csQuaternion &GetRotation () const = 0;

    virtual void SetPosition (const csVector3 &pos) = 0;
    virtual const csVector3 &GetPosition () const = 0;

    virtual iSkeletonFactory::iBoneFactory* GetFactory () const = 0;

    virtual void UpdateTransform () = 0;
    virtual void UpdateTransform (const csReversibleTransform &trans) = 0;

    virtual const csReversibleTransform &GetTransform () const = 0;

    virtual void SetParent (iBone* parent) = 0;
  };

  virtual const size_t GetChildrenCount () const = 0;
  virtual const iBone* GetChild (size_t i) const = 0;
  virtual iBone* GetChild (size_t i) = 0;

  virtual iBone* FindBoneByName (const char* name) = 0;
  virtual int FindBoneIDByName (const char* name) const = 0;

  virtual iSkeletonFactory* GetFactory () const = 0;
  virtual Animation::iAnimationLayer* GetAnimationLayer () = 0;
};

struct iGraveyard : public virtual iBase
{
  SCF_INTERFACE (iGraveyard, 0, 0, 1);

  /**
   * Create a new skeleton factory.
   */
  virtual iSkeletonFactory* CreateFactory (const char* name) = 0;

  virtual iSkeleton* CreateSkeleton (const char* name, const char* factname) = 0;

  /// @@ GENJIX @@
  virtual void Debug () = 0;
};

}

#endif //__CS_INSKELETON2_H__
