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
  int keyframe;

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

struct csFrameSet {

  unsigned int name;
  int numframes;
  int totaltime;
  csMotionFrame *frames;
  
};

DECLARE_TYPED_VECTOR( csAppliedFrameVector, csAppliedFrame );

///
class csMotion : public iMotion
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

  DECLARE_TYPED_VECTOR( csFrameSetVector, csFrameSet ) framesets;

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
  
  virtual void AddFrameSet( const char *name, int time );
  
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
  iSkeletonBone *skel;
  csMotion* curmotion;
  int curframeset;
  int curtime;
  int totaltime;  // Total time for complete cycle
  int numframes;
  int curframe;
  int nextframe;
  bool Reverse;
  bool Loop;
  bool Sweep;
  float Rate;
  csAppliedFrameVector frames;
};

DECLARE_TYPED_VECTOR(csAppliedMotionVector,csAppliedMotion); 

///
class csMotionManager : public iMotionManager
{
  csMotionVector motions;
  csAppliedMotionVector skels;
  csAppliedMotionVector cache;
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

  virtual void DeleteMotion (const char* name );
  
  /// Apply motion returns an index to the new motion for use with the motion control API
  /// If the cache flag is set then the compiled motion is cached rather than being
  /// active.
  virtual int ApplyMotion(iSkeletonBone *skel, const char* motion, 
	const char *frameset, bool reverse, bool loop, bool sweep, float rate, 
	int start_time, bool cache);
	
  /// If the skeletal structure for a sprite is modified, then the compiled
  /// will become invalid. Recompile motion 'purifies' the motion so
  /// that it can be used with the sprite again. This applies to active
  /// sprites as well. This assumes the sprite in question still exists.
  virtual void RecompileMotion( int idx, bool cached );

  /// Reserve motion takes an active motion and places it on the cached list.
  virtual int ReserveMotion( int idx );
  /// Restore motion takes a cached motion and places it on the active list.
  virtual int RestoreMotion( int idx );
  /// Delete motion. If cache is true then the motion is delete from the cached list
  /// otherwise it is deleted from the active motions list. 
  virtual void DeleteAppliedMotion( int motion_index, bool cache);
  
  virtual void SetActiveMotion( int idx, bool reverse, bool loop, bool sweep, float rate ) 
  { 
	skels[idx]->Reverse = reverse; 
	skels[idx]->Loop = loop;
	skels[idx]->Sweep = sweep;
	skels[idx]->Rate = rate;
  }
  virtual void SetCachedMotion( int idx, bool reverse, bool loop, bool sweep, float rate, int time )
  {
	cache[idx]->Reverse = reverse;
	cache[idx]->Loop = loop;
	cache[idx]->Sweep = sweep;
	cache[idx]->Rate = rate;
	cache[idx]->curtime = time;
  }
  
  virtual void SetReverse( int idx, bool reverse ) { skels[idx]->Reverse = reverse; }
  
  virtual void SetLoop( int idx, bool loop ) { skels[idx]->Loop = loop; }

  virtual void SetSweep( int idx, bool sweep ) { skels[idx]->Sweep = sweep; }
  
  virtual void SetRate( int idx, float rate) { skels[idx]->Rate = rate; }
  
  virtual void SetTime( int idx, int time) { skels[idx]->curtime = time; }

  virtual bool GetReverse( int idx ) { return skels[idx]->Reverse; }
  
  virtual bool GetLoop( int idx ) { return skels[idx]->Loop; }
  
  virtual bool GetSweep( int idx ) { return skels[idx]->Sweep; }
  
  virtual float GetRate( int idx ) { return skels[idx]->Rate; }
  
  virtual int GetTime( int idx ) { return skels[idx]->curtime; }
  
  virtual bool IsFirst( int idx ) { return !(skels[idx]->curframe); }
  
  virtual bool IsLast( int idx ) 
  { return !(skels[idx]->numframes - skels[idx]->curframe - 1); }

  virtual void UpdateAll();

  virtual void UpdateAll(int time);
  ///
  
  csMotion* FindClassByName (const char* name);
  void UpdateTransform(iSkeletonBone *bone, csQuaternion *quat);
  void UpdateTransform(iSkeletonBone *bone, csMatrix3 *mat);
  void UpdateTransform(iSkeletonBone *bone, csVector3 *vec);
  void UpdateAppliedFrame(csAppliedFrame *fr, csAppliedFrame *next);
  bool UpdateAppliedMotion(csAppliedMotion *am, cs_time elapsedtime);
  void CompileMotion( csAppliedMotion *motion );
};

iSkeletonBone *FindBone( iSkeletonBone *bone, unsigned int hash );
int FindFrameSet( csMotion *mot, unsigned int hash );

#endif


