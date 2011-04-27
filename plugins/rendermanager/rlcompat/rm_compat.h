/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_RM_COMPAT_H__
#define __CS_RM_COMPAT_H__

#include "csplugincommon/rendermanager/rendertree.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"

#include "iengine/rendermanager.h"
#include "iutil/comp.h"
#include "iutil/dbghelp.h"

struct iEngine;

CS_PLUGIN_NAMESPACE_BEGIN(RM_RLCompat)
{
  class RMCompat : public scfImplementation3<RMCompat,
					     iRenderManager,
					     iComponent,
					     iDebugHelper>
  {
    CS::RenderManager::RenderTreeBase::DebugPersistent debugPersist;
    uint dbgDebugClearScreen;
  public:
    RMCompat (iBase* parent) : scfImplementationType (this, parent) {}
      
    bool Initialize (iObjectRegistry* objReg);
  
    bool RenderView (iView* view);
    bool PrecacheView (iView* view);
  
    void RegisterRenderTarget (iTextureHandle* target, 
      iView* view, int subtexture = 0, uint flags = 0) {}
    void UnregisterRenderTarget (iTextureHandle* target,
      int subtexture = 0) {}
  
    /**\name iDebugHelper implementation
    * @{ */
    csTicks Benchmark (int num_iterations) { return 0; }
    bool DebugCommand (const char *cmd);
    void Dump (iGraphics3D *g3d) {}
    csPtr<iString> Dump () { return 0; }
    int GetSupportedTests () const { return 0; }
    csPtr<iString> StateTest () { return  0; }
    /** @} */
  };
}
CS_PLUGIN_NAMESPACE_END(RM_RLCompat)

#endif // __CS_RM_COMPAT_H__
