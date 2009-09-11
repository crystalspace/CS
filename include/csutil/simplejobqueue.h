/*
    Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_CSUTIL_SIMPLEJOBQUEUE_H__
#define __CS_CSUTIL_SIMPLEJOBQUEUE_H__

/**\file
* Implementation of iJobQueue that runs the jobs directly on queueing.
*/

#include "csextern.h"
#include "csutil/scf_implementation.h"
#include "iutil/job.h"


namespace CS
{
namespace Utility
{

class CS_CRYSTALSPACE_EXPORT SimpleJobQueue :
  public scfImplementation1<SimpleJobQueue, iJobQueue>
{
public:
  SimpleJobQueue ();

  virtual void Enqueue (iJob* job, bool lowPriority = false);
  virtual void PullAndRun (iJob* job);
  virtual void Unqueue (iJob* job, bool waitIfCurrent = true);
  virtual bool IsFinished ();
  virtual int32 GetQueueCount();
  void Wait(iJob*);
  virtual void WaitAll ();
};


}
}

#endif
