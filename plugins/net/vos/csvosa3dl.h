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
#include "ivaria/dynamics.h"

#ifndef M_PI
# ifdef PI
#  define M_PI PI
# else
#  define M_PI 3.141592
# endif
#endif


/** Use this macro to query the object registry, loading a plugin if needed.
    If an object with a given interface exists in an object registry,
    get that object from the registry. If the registry query fails,
    try to load a plugin and get the interface from there. If that succeeds,
    the interface is added to the registry for future use.

    Example use:
@code
    csRef<iDynamics> dynamicSystem;
    CS_QUERY_REGISTRY_PLUGIN(dynamicSystem, object_reg, "crystalspace.dynamics.ode", iDynamics);
@endcode

        @param obj          csRef to hold object
        @param Interface    The interface class to find
        @param object_reg   Your object registry
        @param scfID        The SCF identifier string for the implementation to load from a plugin.
        @param interfaceClassName   The interface class name as a string; this
                            is used as a "tag" in the object registry. For
                            example, if Interface is iDynamics, this string
                            should be "iDynamics".
    @todo This probably ought to be made more thread-safe by locking the
            object registry if possible.
*/
#define CS_QUERY_REGISTRY_PLUGIN(obj, object_reg, scfID, Interface) \
{ \
    obj = CS_QUERY_REGISTRY(object_reg, Interface);  \
    if(!obj.IsValid()) {  \
        csRef<iPluginManager> mgr = CS_QUERY_REGISTRY(object_reg, iPluginManager); \
        if(!mgr.IsValid()) { \
            LOG("csVosA3DL", 1, "Error loading plugin for \"" << scfID \
                << "\": Could not get plugin manager from object registry!"); \
        }  \
        LOG("csVosA3DL", 2,"Loading plugin for \"" << scfID << "\"..."); \
        obj = CS_LOAD_PLUGIN(mgr, scfID, Interface); \
        if(!obj.IsValid()) { \
            LOG("csVosA3DL", 1, "Error loading plugin " << scfID \
                << " with interface \"" << #Interface << "!"); \
        } \
        object_reg->Register(obj, #Interface); \
    } \
}

class csVosA3DL : public iComponent, public iEventHandler,
    public iVosA3DL, public iVosApi
{
private:
  csRef<iEventQueue> eventq;
  csRef<iDynamicSystem> dynsys;

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

  void incrementRelightCounter();
  void decrementRelightCounter();

};

#endif
