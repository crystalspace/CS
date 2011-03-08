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

#include "csplugincommon/rendermanager/autofx_framebuffertex.h"
#include "csplugincommon/rendermanager/autofx_reflrefr.h"
#include "csplugincommon/rendermanager/debugcommon.h"
#include "csplugincommon/rendermanager/hdrexposure.h"
#include "csplugincommon/rendermanager/posteffectssupport.h"
#include "csplugincommon/rendermanager/viscullcommon.h"
#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iengine/rendermanager.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMUnshadowed)
{
  typedef CS::RenderManager::RenderTree<
    CS::RenderManager::RenderTreeLightingTraits> RenderTreeType;
    
  template<typename RenderTreeType, typename LayerConfigType>
  class StandardContextSetup;

  class RMUnshadowed : public scfImplementation6<RMUnshadowed, 
                                                 iRenderManager, 
                                                 iRenderManagerTargets,
                                                 scfFakeInterface<iRenderManagerVisCull>,
                                                 scfFakeInterface<iRenderManagerPostEffects>,
                                                 iComponent,
                                                 scfFakeInterface<iDebugHelper> >,
                       public CS::RenderManager::RMDebugCommon<RenderTreeType>,
		       public CS::RenderManager::PostEffectsSupport,
		       public CS::RenderManager::RMViscullCommon
  {
  public:
    RMUnshadowed (iBase* parent);

    //---- iRenderManager ----
    virtual bool RenderView (iView* view);
    virtual bool PrecacheView (iView* view);

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
    virtual void MarkAsUsed (iTextureHandle* target)
    {
      targets.MarkAsUsed (target);
    }

    //---- iComponent ----
    virtual bool Initialize (iObjectRegistry*);

    // Target manager handler
    bool HandleTargetSetup (CS::ShaderVarStringID svName, csShaderVariable* sv, 
      iTextureHandle* textureHandle, iView*& localView)
    {
      return false;
    }

    typedef StandardContextSetup<RenderTreeType, 
      CS::RenderManager::MultipleRenderLayer> ContextSetupType;

    typedef CS::RenderManager::StandardPortalSetup<RenderTreeType, 
      ContextSetupType> PortalSetupType;

    typedef CS::RenderManager::DependentTargetManager<RenderTreeType, RMUnshadowed>
      TargetManagerType;

    typedef CS::RenderManager::LightSetup<RenderTreeType, 
      CS::RenderManager::MultipleRenderLayer> LightSetupType;

    typedef CS::RenderManager::AutoFX::ReflectRefract<RenderTreeType, 
      ContextSetupType> AutoReflectRefractType;

    typedef CS::RenderManager::AutoFX::FramebufferTex<RenderTreeType>
      AutoFramebufferTexType;
  public:
    iObjectRegistry* objectReg;

    bool RenderView (iView* view, bool recursePortals);
    bool HandleTarget (RenderTreeType& renderTree, 
      const TargetManagerType::TargetSettings& settings,
      bool recursePortals);

    RenderTreeType::PersistentData treePersistent;
    PortalSetupType::PersistentData portalPersistent;
    LightSetupType::PersistentData lightPersistent;
    AutoReflectRefractType::PersistentData reflectRefractPersistent;
    AutoFramebufferTexType::PersistentData framebufferTexPersistent;

    CS::RenderManager::HDRHelper hdr;
    CS::RenderManager::HDR::Exposure::Configurable hdrExposure;
    bool doHDRExposure;
    int maxPortalRecurse;

    csRef<iShaderVarStringSet>  svNameStringSet;
    csRef<iStringSet>           stringSet;
    csRef<iShaderManager>       shaderManager;
    csRef<iEngine>              engine;
    csRef<iLightManager>        lightManager;

    CS::RenderManager::MultipleRenderLayer renderLayer;
    CS::RenderManager::MultipleRenderLayer renderLayerReflect;
    CS::RenderManager::MultipleRenderLayer renderLayerRefract;

    TargetManagerType targets;
    csSet<RenderTreeType::ContextNode*> contextsScannedForTargets;
    
    uint dbgFlagClipPlanes;
  };

}
CS_PLUGIN_NAMESPACE_END(RMUnshadowed)

template<>
class csHashComputer<CS_PLUGIN_NAMESPACE_NAME(RMUnshadowed)::RenderTreeType::ContextNode*> : 
  public csHashComputerIntegral<CS_PLUGIN_NAMESPACE_NAME(RMUnshadowed)::RenderTreeType::ContextNode*> 
{};

#endif // __CS_RM_UNSHADOWED_H__
