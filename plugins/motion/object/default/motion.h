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

#include "iengine/motion.h"
#include "csutil/csvector.h"
#include "csgeom/quaterni.h"
#include "csgeom/matrix3.h"

struct csMotionFrame
{
  cs_time keyframe;

  int *qlinks;
  int numqlinks;
  unsigned int *qaffector;

  int *mlinks;
  int nummlinks;
  unsigned int *maffector;

  int *vlinks;
  int numvlinks;
  unsigned int *vaffector;
};

struct csAppliedFrame
{
  cs_time keyframe;

  csQuaternion **qlinks;
  int numqlinks;
  iSkeletonBone **qaffector;

  csMatrix3 **mlinks;
  int nummlinks;
  iSkeletonBone **maffector;

  csVector3 **vlinks;
  int numvlinks;
  iSkeletonBone **vaffector;
};

DECLARE_TYPED_VECTOR( csAppliedFrameVector, csAppliedFrame );

///
class csMotion:public iMotion
{
public:
  char* name;

  unsigned int hash;

  csQuaternion* transquat;
  int numtransquat;
  
  csMatrix3* transmat;
  int numtransmat;
  
  csVector3* translate;
  int numtranslate;

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
  virtual bool AddAnim (const csVector3 &vec);
  
  virtual int AddFrame (int frametime);
  ///
  virtual void AddFrameQLink (int frameindex, const char* affector, int link);

  virtual void AddFrameMLink (int frameindex, const char* affector, int link);

  virtual void AddFrameVLink (int frameindex, const char* affector, int link);
  ///
  unsigned int GetHash() { return hash; }
};

DECLARE_TYPED_VECTOR(csMotionVectorBase, csMotion);

class csMotionVector : public csMotionVectorBase
{
public:
  csMotionVector (int ilimit = 8, int ithreshold = 16) 
    : csMotionVectorBase(ilimit, ithreshold) {}
  virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
    { int id1 = ((csMotion*)Item1)->GetHash(), id2 = ((csMotion*)Item2)->GetHash();
      return id1 - id2; }

  virtual int CompareKey (csSome Item1, csConstSome Key, int /*Mode*/) const
    { int id1 = ((csMotion*)Item1)->GetHash(), id2 = (unsigned int)Key; return id1 - id2; }
};

struct csAppliedMotion
{
  char *name;
  iSkeletonBone *skel;
  csMotion* curmotion;
  cs_time curtime;
  int numframes;
  int curframe;
  csAppliedFrameVector frames;
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
  virtual bool ApplyMotion(iSkeletonBone *skel, const char* motion, int time);
  ///
  virtual void UpdateAll();

  virtual void UpdateAll(int time);
  ///
  csMotion* FindClassByName (const char* name);

  void UpdateTransform(iSkeletonBone *bone, csQuaternion *quat);
  void UpdateTransform(iSkeletonBone *bone, csMatrix3 *mat);
  void UpdateTransform(iSkeletonBone *bone, csVector3 *vec);
  void UpdateAppliedFrame(csAppliedFrame *fr);
  bool UpdateAppliedMotion(csAppliedMotion *am, cs_time elapsedtime);
  void CompileMotion( csAppliedMotion *motion );
};

iSkeletonBone *FindBone( iSkeletonBone *bone, unsigned int hash );

#endif


