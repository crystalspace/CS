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

#ifndef __ISKELETON_H__
#define __ISKELETON_H__

#include "csutil/scf.h"
#include "iplugin.h"
class csTransform;

SCF_VERSION (iSkeletonBone, 0, 0, 1);

/// temporary - subject to change
struct iSkeletonBone : public iBase
{
  ///
  virtual iSkeletonBone* GetNext () = 0;
  ///
  virtual iSkeletonBone* GetChildren () = 0;
  ///
  virtual const char* GetName () = 0;
  ///
  virtual void SetTransformation (const csTransform& tr) = 0;
};

#endif
