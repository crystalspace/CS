/*
    Copyright (C) 2007 by Marten Svanfeldt

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

#ifndef __CS_RM_TEST1_H__
#define __CS_RM_TEST1_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iengine/rendermanager.h"
#include "csplugincommon.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMTest1)
{

  class RMTest1 : public scfImplementation2<RMTest1, 
                                            iRenderManager, 
                                            iComponent>
  {
  public:
    RMTest1 (iBase* parent);

    //---- iRenderManager ----
    virtual bool RenderView (iView* view);

    virtual void RegisterRenderTarget (iTextureHandle* target, 
      iView* view, int subtexture = 0, uint flags = 0)
    {
      targets.RegisterRenderTarget (target, view, subtexture, flags);
    }
    virtual void UnregisterRenderTarget (iTextureHandle* target,
      int subtexture = 0)
    {
      targets.UnregisterRenderTarget (target, subtexture);
    }

    //---- iComponent ----
    virtual bool Initialize (iObjectRegistry*);

    // Target manager handler
    bool HandleTargetSetup (csStringID svName, csShaderVariable* sv, 
      iTextureHandle* textureHandle, iView*& localView)
    {
      return false;
    }

    typedef CS::RenderManager::RenderTree<> RenderTreeType;
  protected:    

    bool HandleTarget (RenderTreeType& renderTree, csStringID svName, 
      RenderTreeType::ContextsContainer* contexts);

    CS::RenderManager::RenderView::Pool renderViewPool;

    RenderTreeType::PersistentData treePersistent;

    csRef<iStringSet>       svNameStringSet;
    csRef<iStringSet>       stringSet;
    csRef<iShaderManager>   shaderManager;

    csStringID              defaultShaderName;
    csRef<iShader>          defaultShader;

    CS::RenderManager::DependentTargetManager<RenderTreeType, RMTest1> targets;

  };


}
CS_PLUGIN_NAMESPACE_END(RMTest1)

#endif
