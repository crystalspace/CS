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

#ifndef __IENGINE_MOTION_H__
#define __IENGINE_MOTION_H__

#include "csutil/scf.h"
#include "isys/plugin.h"
#include "iengine/skelbone.h"

class csMatrix3;
class csQuaternion;
class csVector3;

SCF_VERSION (iMotion, 0, 1, 0);

/**
 * A motion.
 */
struct iMotion : public iBase
{
  ///
  virtual const char* GetName () = 0;
  ///
  virtual void SetName (const char* name) = 0;
  ///
  virtual bool AddAnim (const csQuaternion &quat) = 0;
  ///
  virtual bool AddAnim (const csMatrix3 &mat) = 0;
  ///
  virtual bool AddAnim (const csVector3 &vec) = 0;
  ///
  virtual void AddFrameSet( const char *name, int total_time ) = 0;

  virtual int AddFrame (int framenumber) = 0;
  ///
  virtual void AddFrameQLink ( int frameindex, const char* affector, int link) = 0;
  ///
  virtual void AddFrameMLink ( int frameindex, const char* affector, int link) = 0;
  ///
  virtual void AddFrameVLink ( int frameindex, const char* affector, int link) = 0;
  
};

SCF_VERSION (iMotionManager, 0, 1, 1);

/**
 * The motion manager.
 */
struct iMotionManager : public iPlugIn
{
  ///
  virtual bool Initialize (iSystem *iSys) = 0;
  ///
  virtual iMotion* FindByName (const char* name) = 0;
  ///
  virtual iMotion* AddMotion (const char* name) = 0;
  
  virtual void DeleteMotion (const char* name) = 0;
  ///
  virtual int ApplyMotion (iSkeletonBone *skel, const char* motion, 
	const char *frameset, bool reverse, bool loop, bool sweep, float rate, 
	  int time, bool cache) = 0;
	  
  /// If the skeletal structure for a sprite is modified, then the compiled
  /// will become invalid. Recompile motion 'purifies' the motion so
  /// that it can be used with the sprite again. This applies to active
  /// sprites as well. This assumes the sprite in question still exists.
  virtual void RecompileMotion ( int idx, bool cache ) = 0;
  
  virtual int ReserveMotion( int idx ) = 0;
  
  virtual int RestoreMotion( int idx ) = 0;
  
  virtual void DeleteAppliedMotion ( int idx, bool cached ) = 0;
  
  virtual void SetActiveMotion ( int idx, bool reverse, bool loop, 
									bool sweep, float rate ) = 0;
						
  virtual void SetCachedMotion ( int idx, bool reverse, bool loop, 
							bool sweep, float rate, int time ) = 0;
	
  virtual void SetReverse ( int idx, bool reverse ) = 0;
  
  virtual void SetLoop ( int idx, bool loop ) = 0;
  
  virtual void SetSweep ( int idx, bool sweep ) = 0;

  virtual void SetRate ( int idx, float rate ) = 0;
  
  virtual void SetTime ( int idx, int time ) = 0;
  
  virtual bool GetReverse ( int idx ) = 0;
  
  virtual bool GetLoop ( int idx ) = 0;
  
  virtual bool GetSweep ( int idx ) = 0;
  
  virtual float GetRate ( int idx ) = 0;
  
  virtual int GetTime ( int idx ) = 0;
  
  virtual bool IsFirst ( int idx ) = 0;
  
  virtual bool IsLast ( int idx ) = 0;

  virtual void UpdateAll () = 0;
  
  virtual void UpdateAll ( int time ) = 0;
};

#endif
