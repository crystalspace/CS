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

#include "imotion.h"
#include "csutil/csvector.h"

///
class csMotionAnim:public iMotion {
public:
  DECLARE_IBASE;

	csMotionAnim(iBase *iParent);
	virtual ~csMotionAnim();
};

class csMotionAnimVectorBase:public csVector {
public:
	virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
		{ int id1 = ((csMotionAnim*)Item1)->GetHash(), id2 = ((csMotionAnim*)Item2)->GetHash();
		return id1 - id2; }

	virtual int CompareKey (csSome Item1, csConstSome Key, int /*Mode*/) const
		{ int id1 = ((csMotionAnim*)Item1)->GetHash(), id2 = (int)Key; return id1 - id2; }
};

DECLARE_TYPED_VECTOR_WITH_BASE(csMotionAnimVector,csMotionAnim,csMotionAnimVectorBase);

///
class csMotionFrame:public iMotion {
public:
  DECLARE_IBASE;

	csMotionFrame(iBase *iParent);
	virtual ~csMotionFrame();
};

class csMotionFrameVectorBase:public csVector {
public:
	virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
		{ int id1 = ((csMotionAnim*)Item1)->GetNumber(), id2 = ((csMotionAnim*)Item2)->GetNumber();
		return id1 - id2; }

	virtual int CompareKey (csSome Item1, csConstSome Key, int /*Mode*/) const
		{ int id1 = ((csMotionAnim*)Item1)->GetNumber(), id2 = (int)Key; return id1 - id2; }
};

DECLARE_TYPED_VECTOR_WITH_BASE(csMotionFrameVector,csMotionFrame,csMotionFrameVectorBase);

///
class csMotion:public iMotion {
	char* name;

	csMotionFrameVector frames;

	csMotionAnimVector anims;
public:
  DECLARE_IBASE;

	csMotion(iBase *iParent);
	virtual ~csMotion();
};

class csMotionVectorBase:public csVector {
public:
	virtual int Compare (csSome Item1, csSome Item2, int /*Mode*/) const
		{ int id1 = ((csMotion*)Item1)->GetHash(), id2 = ((csMotion*)Item2)->GetHash();
		return id1 - id2; }

	virtual int CompareKey (csSome Item1, csConstSome Key, int /*Mode*/) const
		{ int id1 = ((csMotion*)Item1)->GetHash(), id2 = (int)Key; return id1 - id2; }
};

DECLARE_TYPED_VECTOR_WITH_BASE(csMotionVector,csMotion,csMotionVectorBase); 

///
class csMotionManager:public iMotionManager {
	csMotionVector motions;

public:
  DECLARE_IBASE;

	///
	csMotionManager(iBase *iParent);
	///
	virtual ~csMotionManager();

	///
	virtual const char* GetName ();
	///
	virtual void SetName (const char* name);
	///
	virtual bool Initialize (iSystem *iSys);
  ///
  virtual iMotion* FindByName (const char* name);
  ///
  virtual iMotion* AddMotion (const char* name);
};

#endif


