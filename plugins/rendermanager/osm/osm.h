/*
  Copyright (C) 2011 Alexandru - Teodor Voicu
      Imperial College London
      http://www3.imperial.ac.uk/

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

#ifndef __CS_RM_OSM_H__
#define __CS_RM_OSM_H__

#include "csplugincommon/rendermanager/debugcommon.h"
#include "imap/loader.h"
#include "csplugincommon/rendermanager/viscullcommon.h"
#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iengine/rendermanager.h"

#include "shadow_osm.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMOSM)
{
  template<typename RenderTreeType, typename LayerConfigType,
    typename LightSetupType>
  class StandardContextSetup;

  typedef CS::RenderManager::RenderTree<
    CS::RenderManager::RenderTreeLightingTraits> RenderTreeType;

  class RMOSM : public scfImplementation4<RMOSM, 
                                          iRenderManager, 
                                          scfFakeInterface<iRenderManagerVisCull>,
                                          iComponent,
                                          scfFakeInterface<iDebugHelper> >,
                public CS::RenderManager::RMDebugCommon<RenderTreeType>,
                public CS::RenderManager::RMViscullCommon
  {
  public:
    RMOSM (iBase* parent);

    //---- iRenderManager ----
    virtual bool RenderView (iView* view);
    virtual bool PrecacheView (iView* view);

    //---- iComponent ----
    virtual bool Initialize (iObjectRegistry*);

    typedef CS::RenderManager::ShadowOSM<RenderTreeType,
      CS::RenderManager::MultipleRenderLayer> ShadowType;
    typedef CS::RenderManager::LightSetup<RenderTreeType, 
      CS::RenderManager::MultipleRenderLayer, ShadowType> LightSetupType;

    typedef StandardContextSetup<RenderTreeType, 
      CS::RenderManager::MultipleRenderLayer, LightSetupType> ContextSetupType;
    typedef CS::RenderManager::StandardPortalSetup<RenderTreeType, 
      ContextSetupType> PortalSetupType;

    //---- iDebugHelper Interface ----
    virtual bool DebugCommand(const char *cmd);

  public:
    iObjectRegistry* objectReg;

    bool RenderView (iView* view, bool recursePortals);

    RenderTreeType::PersistentData treePersistent;
    LightSetupType::PersistentData lightPersistent;

    csRef<iShaderManager> shaderManager;
    csRef<iLightManager> lightManager;

    csRef<iTextureHandle> accumBuffer;

    CS::RenderManager::MultipleRenderLayer renderLayer;

    uint dbgFlagClipPlanes;
  };

}
CS_PLUGIN_NAMESPACE_END(RMOSM)

#endif // __CS_RM_OSM_H__
