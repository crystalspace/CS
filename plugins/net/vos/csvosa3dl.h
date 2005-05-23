/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
   $Id$

    This file is part of Crystal Space Virtual Object System Abstract
    3D Layer plugin (csvosa3dl).

    Copyright (C) 2004 Peter Amstutz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _CSA3DL_H_
#define _CSA3DL_H_

// Hack: Work around problems caused by #defining 'new'.
#if defined(CS_EXTENSIVE_MEMDEBUG) || defined(CS_MEMORY_TRACKER)
# undef new
#endif
#include <new>

#include <vos/vos/site.hh>
#include <vos/vutil/taskqueue.hh>

#include "inetwork/vosa3dl.h"
#include "inetwork/vosapi.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "iutil/virtclk.h"
#include "ivaria/dynamics.h"
#include "ivaria/reporter.h"

#ifndef M_PI
# ifdef PI
#  define M_PI PI
# else
#  define M_PI 3.141592
# endif
#endif

class csVosA3DL : public iComponent, public iEventHandler,
    public iVosA3DL, public iVosApi
{
private:
  csRef<iEventQueue> eventq;
  csRef<iDynamicSystem> dynsys;
  csRef<iProgressMeter> progress;
  csRef<iVirtualClock> clock;

  iObjectRegistry *objreg;

  VUtil::vRef<VOS::Site> localsite;

  int relightCounter;
  boost::mutex relightCounterMutex;

public:
  SCF_DECLARE_IBASE;

  VUtil::SynchronizedQueue<VUtil::Task*> mainThreadTasks;

  csVosA3DL (iBase *);
  virtual ~csVosA3DL();

  virtual csRef<iVosSector> GetSector(const char* s);
  virtual bool Initialize (iObjectRegistry *objreg);
  virtual bool HandleEvent (iEvent &ev);

  iObjectRegistry* GetObjectRegistry()
  {
    return objreg;
  }

  csRef<iDynamicSystem> GetDynSys ()
  {
    return dynsys;
  }

  virtual VUtil::vRef<VOS::Vobject> GetVobject();

  void setProgressMeter(iProgressMeter* meter);

  void incrementRelightCounter();
  void decrementRelightCounter();

};

#endif
