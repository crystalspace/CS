/*
    Copyright (C) 2006 by Hristo Hristov

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

#ifndef __CS_SKELETON_H__
#define __CS_SKELETON_H__

#define __CS_SKELETAL_DEBUG

#include "csgeom/transfrm.h"
#include "csutil/array.h"
#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/virtclk.h"
#include "imesh/skeleton_new.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton)
{



class SkeletonGraveyard :
  public scfImplementation2<SkeletonGraveyard, iAmirSkeletonGraveyard,
    iComponent>
{
public:
  SkeletonGraveyard (iBase *parent);
  virtual ~SkeletonGraveyard ();

  bool Initialize (iObjectRegistry* object_reg);
  bool HandleEvent (iEvent& ev);

  virtual iAmirSkeletonFactory* CreateFactory ();
private:

  class SkelEventHandler :
    public scfImplementation1<SkelEventHandler, iEventHandler>
  {
  private:
    SkeletonGraveyard* parent;

  public:
    SkelEventHandler (SkeletonGraveyard* parent)
      : scfImplementationType (this), parent (parent) { }
    virtual ~SkelEventHandler () { }
    virtual bool HandleEvent (iEvent& ev)
    {
      //return parent->HandleEvent (ev);
      return false;
    }

    CS_EVENTHANDLER_NAMES("crystalspace.skeleton.graveyard")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  };
  csRef<SkelEventHandler> evhandler;

  void Update (csTicks time);

  iObjectRegistry* object_reg;
  csRef<iVirtualClock> vc;
  //csRefArray<Skeleton> skeletons;
  //csRefArray<SkeletonFactory> factories;
  csEventID preprocess;
};

}
CS_PLUGIN_NAMESPACE_END(Skeleton)

#endif // __CS_SKELETON_H__
