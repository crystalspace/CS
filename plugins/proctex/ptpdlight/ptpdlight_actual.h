/*
    Copyright (C) 2010 by Frank Richter

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

#ifndef __CS_PTPDLIGHT_ACTUAL_H__
#define __CS_PTPDLIGHT_ACTUAL_H__

#include "ptpdlight.h"
#include "ptpdlight_loader.h"

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

template<typename ThisClass>
class ProctexPDLight_Actual : public ProctexPDLight
{
public:
  ProctexPDLight_Actual (ProctexPDLightLoader* loader, iImage* img)
   : ProctexPDLight (loader, img) {}
  ProctexPDLight_Actual (ProctexPDLightLoader* loader, int w, int h)
   : ProctexPDLight (loader, w, h) {}
   
  void Animate (csTicks current_time)
  {
    if (state.Check (stateDirty))
    {
      if (!loader->UpdatePT (this, current_time)) return;

      ProctexPDLight_Actual::Animate();
    }
  }

  void Animate ()
  {
    csTicks startTime = csGetTicks();

    CS_PROFILER_ZONE(ProctexPDLight_Animate)
    
    static_cast<ThisClass*> (this)->RealAnimate ();

    state.Reset (stateDirty);
    dirtyLights.DeleteAll ();
    tilesDirty.Clear ();

    csTicks endTime = csGetTicks();
    loader->RecordUpdateTime (endTime-startTime);
  }
  
};

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)

#endif // __CS_PTPDLIGHT_ACTUAL_H__
