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

#include "iutil/comp.h"
#include "csutil/scf_implementation.h"
#include "iengine/rendermanager.h"
#include "itexture.h"

#include "gbuffer.h"
#include "deferredtreetraits.h"
#include "deferredlightrender.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{
  typedef CS::RenderManager::RenderTree<RenderTreeDeferredTraits> 
    RenderTreeType;

  template<typename RenderTreeType, typename LayerConfigType>
  class StandardContextSetup;

  class RMDeferred : public scfImplementation5<RMDeferred, 
                                               iRenderManager,
                                               scfFakeInterface<iRenderManagerVisCull>,
                                               iComponent,
                                               scfFakeInterface<iRenderManagerPostEffects>,
                                               scfFakeInterface<iDebugHelper> >,
                     public CS::RenderManager::RMDebugCommon<RenderTreeType>,
		     public CS::RenderManager::PostEffectsSupport,
		     public CS::RenderManager::RMViscullCommon
  {
  public:

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

  public:

    void AddDeferredLayer(CS::RenderManager::MultipleRenderLayer &layers, int &addedLayer);
    void AddZOnlyLayer(CS::RenderManager::MultipleRenderLayer &layers, int &addedLayer);

    int LocateDeferredLayer(const CS::RenderManager::MultipleRenderLayer &layers);
    int LocateZOnlyLayer(const CS::RenderManager::MultipleRenderLayer &layers);
    int LocateLayer(const CS::RenderManager::MultipleRenderLayer &layers,
                    csStringID shaderType);

    void ShowGBuffer(RenderTreeType &tree);

    iObjectRegistry *objRegistry;

    RenderTreeType::PersistentData treePersistent;
    PortalSetupType::PersistentData portalPersistent;
    LightSetupType::PersistentData lightPersistent;
    DeferredLightRenderer::PersistentData lightRenderPersistent;

    CS::RenderManager::MultipleRenderLayer renderLayer;

    csRef<iShaderManager> shaderManager;
    csRef<iLightManager> lightManager;
    csRef<iStringSet> stringSet;

    csRef<iTextureHandle> accumBuffer;

    GBuffer gbuffer;

    int deferredLayer;
    int zonlyLayer;
    int maxPortalRecurse;

    bool showGBuffer;
    bool drawLightVolumes;
  };
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __DEFERRED_H__
