/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_IUTIL_JOB_H__
#define __CS_IUTIL_JOB_H__

#include "csutil/scf.h"

SCF_VERSION (iJob, 0, 0, 1);

struct iJob : public iBase
{
  virtual void Run() = 0;
};

SCF_VERSION (iJobQueue, 0, 0, 1);

struct iJobQueue : public iBase
{
  virtual void Enqueue (iJob* job) = 0;
  virtual void PullAndRun (iJob* job) = 0;
};

#endif // __CS_IUTIL_JOB_H__
