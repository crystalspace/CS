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

SCF_VERSION (iMotion, 0, 0, 2);

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
  virtual int AddFrame (int framenumber) = 0;
  ///
  virtual void AddFrameQLink ( int frameindex, const char* affector, int link) = 0;
  ///
  virtual void AddFrameMLink ( int frameindex, const char* affector, int link) = 0;
  ///
  virtual void AddFrameVLink ( int frameindex, const char* affector, int link) = 0;
  
};

SCF_VERSION (iMotionManager, 0, 0, 2);

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
  ///
  virtual bool ApplyMotion (iSkeletonBone *skel, const char* motion, int time) = 0;
  ///
  virtual void UpdateAll () = 0;
  
  virtual void UpdateAll ( int time ) = 0;
};

#endif
