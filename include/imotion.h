/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef __IMOTION_H__
#define __IMOTION_H__

#include "csutil/scf.h"
#include "iplugin.h"
#include "csgeom/quaterni.h"
#include "csgeom/matrix3.h"

SCF_VERSION (iMotionAnim, 0, 0, 1);

/// temporary - subject to change
struct iMotionAnim : public iBase
{
  ///
	virtual void AddAffector (const char* name) = 0;
  ///
	virtual bool Set (const csQuaternion& q) = 0;
  ///
	virtual bool Set (const csMatrix3& q) = 0;
};

SCF_VERSION (iMotionFrame, 0, 0, 1);

/// temporary - subject to change
struct iMotionFrame : public iBase
{
  ///
	virtual iMotionAnim* LinkAnim (const char* name) = 0;
};

SCF_VERSION (iMotion, 0, 0, 1);

/// temporary - subject to change
struct iMotion : public iBase
{
	///
	virtual const char* GetName () = 0;
	///
	virtual void SetName (const char* name) = 0;
  ///
  virtual iMotionAnim* FindAnimByName (const char* name) = 0;
  ///
	virtual iMotionAnim* AddAnim (const char* name) = 0;
  ///
  virtual iMotionFrame* GetFrame (int framenumber) = 0;
  ///
	virtual iMotionFrame* AddFrame (int framenumber) = 0;
};

SCF_VERSION (iMotionManager, 0, 0, 1);

/// temporary - subject to change
struct iMotionManager : public iPlugIn
{
	///
	virtual bool Initialize (iSystem *iSys) = 0;
  ///
  virtual iMotion* FindByName (const char* name) = 0;
  ///
  virtual iMotion* AddMotion (const char* name, bool matrix=0) = 0;
};

#endif
