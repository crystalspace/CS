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

#ifndef __CS_RM_NULL_H__
#define __CS_RM_NULL_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iengine/rendermanager.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMNull)
{
  class RMNull : public scfImplementation2<RMNull, 
                                            iRenderManager, 
                                            iComponent>
  {
  public:
    RMNull (iBase* parent);

    //---- iRenderManager ----
    virtual bool RenderView (iView* view);
    virtual bool PrecacheView (iView* view);

    //---- iComponent ----
    virtual bool Initialize (iObjectRegistry*);
  };

}
CS_PLUGIN_NAMESPACE_END(RMNull)

#endif // __CS_RM_NULL_H__
