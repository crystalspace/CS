/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef __MOTION_H__
#define __MOTION_H__

#include "iengine/imotion.h"
#include "csutil/csvector.h"
#include "csgeom/quaterni.h"
#include "csgeom/matrix3.h"

struct csMotionFrame
{
  cs_time keyframe;

  int size;
  int *links;
  unsigned int *affectors;
};

///
class csMotion:public iMotion
{
public:
  char* name;
  char matrixmode;

  unsigned int hash;

  void* transforms;
  int numtransforms;

  csMotionFrame* frames;
  int numframes;

  DECLARE_IBASE;

  ///
  csMotion();
  ///
  virtual ~csMotion();
  ///
  virtual const char* GetName ();
  ///
  virtual void SetName (const char* name); 
  ///
  virtual bool AddAnim (const csQuaternion &quat);
  ///
  virtual bool AddAnim (const csMatrix3 &mat);
  ///
  virtual int AddFrame (int framenumber);
  ///
  virtual void AddFrameLink (int frameindex, const char* affector, int link);
  ///
  unsigned int GetHash() { return hash; }
};

class csMotionVectorBase:public csVector {
public:
  csMotionVectorBase (int ilimit = 8, int ithreshold = 16) 
    : csVector(ilimit, ithreshold) {}
  virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
    { int id1 = ((csMotion*)Item1)->GetHash(), id2 = ((csMotion*)Item2)->GetHash();
      return id1 - id2; }

  virtual int CompareKey (csSome Item1, csConstSome Key, int /*Mode*/) const
    { int id1 = ((csMotion*)Item1)->GetHash(), id2 = (unsigned int)Key; return id1 - id2; }
};

DECLARE_TYPED_VECTOR_WITH_BASE(csMotionVector,csMotion,csMotionVectorBase); 

struct csAppliedMotion
{
  iSkeletonBone* skel;
  csMotion* curmotion;
  cs_time curtime;
  csMotionFrame* curframe;
  csMotionFrame* nextframe;
};

DECLARE_TYPED_VECTOR(csAppliedMotionVector,csAppliedMotion); 

///
class csMotionManager:public iMotionManager
{
  csMotionVector motions;
  csAppliedMotionVector skels;
  cs_time oldtime;
  iSystem* iSys;
public:
  DECLARE_IBASE;

  ///
  csMotionManager(iBase *iParent);
  ///
  virtual ~csMotionManager();

  ///
  virtual bool Initialize (iSystem *iSys);
  ///
  virtual iMotion* FindByName (const char* name)
  {
    return FindClassByName(name);  
  }
  ///
  virtual iMotion* AddMotion (const char* name);
  ///
  virtual bool ApplyMotion(iSkeletonBone *skel, const char* motion);
  ///
  virtual void UpdateAll();

  ///
  csMotion* FindClassByName (const char* name);

  void UpdateTransform(csAppliedMotion *am, iSkeletonBone *bone, int link, int link2);
  bool UpdateBone(csAppliedMotion *am, iSkeletonBone *bone, unsigned int hash);
  void UpdateAppliedBones(csAppliedMotion *am, iSkeletonBone *bone);
  bool UpdateAppliedMotion(csAppliedMotion *am, cs_time elapsedtime);
};

#endif


