/*
    Copyright (C) 2010 by Joe Forte

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __DEFERRED_H__
#define __DEFERRED_H__

#include "cssysdef.h"

#include "csplugincommon/rendermanager/standardtreetraits.h"
#include "csplugincommon/rendermanager/dependenttarget.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/debugcommon.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/posteffectssupport.h"
#include "csplugincommon/rendermanager/viscullcommon.h"
#include "csplugincommon/rendermanager/hdrexposure.h"

#include "iutil/comp.h"
#include "csutil/scf_implementation.h"
#include "iengine/rendermanager.h"
#include "itexture.h"

#include "gbuffer.h"
#include "deferredtreetraits.h"
#include "deferredlightrender.h"
#include "globalillum.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{
  typedef CS::RenderManager::RenderTree<RenderTreeDeferredTraits> 
    RenderTreeType;

  template<typename RenderTreeType, typename LayerConfigType>
  class StandardContextSetup;

  class RMDeferred : public scfImplementation6<RMDeferred, 
                                               iRenderManager,
                                               scfFakeInterface<iRenderManagerVisCull>,
                                               iComponent,
                                               scfFakeInterface<iRenderManagerPostEffects>,
                                               iRenderManagerGlobalIllum,
                                               scfFakeInterface<iDebugHelper> >,
                     public CS::RenderManager::RMDebugCommon<RenderTreeType>,
		     public CS::RenderManager::PostEffectsSupport,
		     public CS::RenderManager::RMViscullCommon
  {
  public:

    enum csDebugBuffer {
      CS_DEPTH_BUFFER,
      CS_DIFFUSE_BUFFER,
      CS_NORMAL_BUFFER,
      CS_AMBIENT_BUFFER,
      CS_SPECULAR_BUFFER,
      CS_VERTEX_NORMALS_BUFFER,
      CS_LINEAR_DEPTH_BUFFER,
      CS_AMBOCC_BUFFER,
      CS_INDLIGHT_BUFFER
    };

    /// Constructor.
    RMDeferred(iBase *parent);

    //---- iComponent Interface ----
    virtual bool Initialize(iObjectRegistry *registry);

    //---- iRenderManager Interface ----
    virtual bool RenderView(iView *view);
    virtual bool PrecacheView(iView *view);

    typedef StandardContextSetup<RenderTreeType, CS::RenderManager::MultipleRenderLayer> 
      ContextSetupType;

    typedef CS::RenderManager::StandardPortalSetup<RenderTreeType, ContextSetupType> 
      PortalSetupType;

    typedef CS::RenderManager::LightSetup<RenderTreeType, CS::RenderManager::MultipleRenderLayer> 
      LightSetupType;

    //---- iDebugHelper Interface ----
    virtual bool DebugCommand(const char *cmd);

    //---- iRenderManagerGlobalIllum Interface ----
    virtual void EnableGlobalIllumination(bool enable);
    virtual void ChangeBufferResolution(const char *bufferResolution);
    virtual void EnableBlurPass(bool enableBlur);
    virtual void ChangeNormalsAndDepthResolution (const char *resolution);
    virtual csShaderVariable* GetGlobalIllumVariableAdd(const char *svName);
    virtual csShaderVariable* GetBlurVariableAdd(const char *svName);
    virtual csShaderVariable* GetCompositionVariableAdd(const char *svName);

  public:

    bool RenderView(iView *view, bool recursePortals);
    void AddDeferredLayer(CS::RenderManager::MultipleRenderLayer &layers, int &addedLayer);
    void AddZOnlyLayer(CS::RenderManager::MultipleRenderLayer &layers, int &addedLayer);

    void LoadDebugLayer();
    void UpdateDebugBufferSV();

    int LocateDeferredLayer(const CS::RenderManager::MultipleRenderLayer &layers);
    int LocateZOnlyLayer(const CS::RenderManager::MultipleRenderLayer &layers);
    int LocateLayer(const CS::RenderManager::MultipleRenderLayer &layers,
                    csStringID shaderType);

    void ShowGBuffer(RenderTreeType &tree);

    bool CreateDirectLightBuffer();

    iObjectRegistry *objRegistry;

    csRef<iGraphics3D> graphics3D;

    RenderTreeType::PersistentData treePersistent;
    PortalSetupType::PersistentData portalPersistent;
    LightSetupType::PersistentData lightPersistent;
    DeferredLightRenderer::PersistentData lightRenderPersistent;

    CS::RenderManager::MultipleRenderLayer renderLayer;

    csRef<iShaderManager> shaderManager;
    csRef<iLightManager> lightManager;
    csRef<iStringSet> stringSet;
    csRef<iShaderVarStringSet> svStringSet;

    const char *accumFmt;
    csRef<iTextureHandle> accumBuffer;
    csRef<iTextureHandle> directLightBuffer;

    GBuffer gbuffer;
    csGlobalIllumRenderer globalIllum;

    CS::RenderManager::HDRHelper hdr;
    CS::RenderManager::HDR::Exposure::Configurable hdrExposure;
    bool isHDREnabled;
    bool doHDRExposure;

    int deferredLayer;
    int zonlyLayer;
    int maxPortalRecurse;

    bool hasPostEffects;
    bool showGBuffer;
    bool drawLightVolumes;
    
    bool isDebugActive;
    csDebugBuffer debugBuffer;
    csRef<iShader> debugShader;
    CS::RenderManager::PostEffectManager::Layer *debugLayer;

    const char *messageID;
  };
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __DEFERRED_H__
