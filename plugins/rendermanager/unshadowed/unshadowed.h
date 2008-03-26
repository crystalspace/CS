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

#ifndef __CS_RM_UNSHADOWED_H__
#define __CS_RM_UNSHADOWED_H__

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iengine/rendermanager.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMUnshadowed)
{
  template<typename RenderTreeType, typename LayerConfigType>
  class StandardContextSetup;

  class RMUnshadowed : public scfImplementation3<RMUnshadowed, 
                                                 iRenderManager, 
                                                 iRenderManagerTargets,
                                                 iComponent>
  {
  public:
    RMUnshadowed (iBase* parent);

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
    bool HandleTargetSetup (CS::ShaderVarStringID svName, csShaderVariable* sv, 
      iTextureHandle* textureHandle, iView*& localView)
    {
      return false;
    }

    typedef CS::RenderManager::RenderTree<> RenderTreeType;

    typedef StandardContextSetup<RenderTreeType, 
      CS::RenderManager::MultipleRenderLayer> ContextSetupType;

    typedef CS::RenderManager::StandardPortalSetup<RenderTreeType, 
      ContextSetupType> PortalSetupType;

    typedef CS::RenderManager::DependentTargetManager<RenderTreeType, RMUnshadowed>
      TargetManagerType;

    typedef CS::RenderManager::LightSetup<RenderTreeType, 
      CS::RenderManager::MultipleRenderLayer> LightSetupType;

  public:    

    bool HandleTarget (RenderTreeType& renderTree, 
      const TargetManagerType::TargetSettings& settings);

    RenderTreeType::PersistentData treePersistent;
    PortalSetupType::PersistentData portalPersistent;
    LightSetupType::PersistentData lightPersistent;

    CS::RenderManager::PostEffectManager       postEffects;

    csRef<iShaderVarStringSet>  svNameStringSet;
    csRef<iStringSet>           stringSet;
    csRef<iShaderManager>       shaderManager;
    csRef<iEngine>              engine;
    csRef<iLightManager>        lightManager;

    CS::RenderManager::MultipleRenderLayer renderLayer;

    TargetManagerType targets;
    csSet<RenderTreeType::ContextNode*> contextsScannedForTargets;
  };  

}
CS_PLUGIN_NAMESPACE_END(RMUnshadowed)

template<>
class csHashComputer<CS_PLUGIN_NAMESPACE_NAME(RMUnshadowed)::RMUnshadowed::RenderTreeType::ContextNode*> : 
  public csHashComputerIntegral<CS_PLUGIN_NAMESPACE_NAME(RMUnshadowed)::RMUnshadowed::RenderTreeType::ContextNode*> 
{};

#endif // __CS_RM_UNSHADOWED_H__
