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

#include "cssysdef.h"

#include "csplugincommon/rendermanager/lightsetup.h"
#include "csplugincommon/rendermanager/portalsetup.h"
#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/viscull.h"

#include "osm.h"

#include "iutil/verbositymanager.h"
#include "ivaria/reporter.h"
#include "csutil/cfgacc.h"
#include "csutil/stringquote.h"
#include "csutil/scfstr.h"

using namespace CS::RenderManager;

CS_PLUGIN_NAMESPACE_BEGIN(RMOSM)
{

SCF_IMPLEMENT_FACTORY(RMOSM)

/* Template magic to deal with different initializers for different
   ShadowType::ShadowParameter types. */
template<typename ShadowType>
struct WrapShadowParams
{
  static typename ShadowType::ShadowParameters Create (
    RMOSM::ShadowType::PersistentData& persist,
    CS::RenderManager::RenderView* rview)
  {
    return typename ShadowType::ShadowParameters ();
  }
};

template<>
struct WrapShadowParams<RMOSM::ShadowType>
{
  static RMOSM::ShadowType::ViewSetup Create (
    RMOSM::ShadowType::PersistentData& shadowPersist,
    CS::RenderManager::RenderView* rview)
  {
    return RMOSM::ShadowType::ViewSetup (
      shadowPersist, rview);
  }
};

  template<typename RenderTreeType, typename LayerConfigType,
    typename LightSetupType>
  class StandardContextSetup
  {
  public:
    typedef StandardContextSetup<RenderTreeType, LayerConfigType, 
      LightSetupType> ThisType;
    typedef StandardPortalSetup<RenderTreeType, ThisType> PortalSetupType;
    typedef typename LightSetupType::ShadowHandlerType ShadowType;

    StandardContextSetup (RMOSM* rmanager, const LayerConfigType& layerConfig)
      : rmanager (rmanager), layerConfig (layerConfig)
    {
    }

    void operator() (typename RenderTreeType::ContextNode& context,
      typename PortalSetupType::ContextSetupData& portalSetupData,
      bool recursePortals = true)
    {
      CS::RenderManager::RenderView* rview = context.renderView;
      iSector* sector = rview->GetThisSector ();

      // @@@ This is somewhat "boilerplate" sector/rview setup.
      sector->PrepareDraw (rview);
      // Make sure the clip-planes are ok
      CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());

      if (context.owner.IsDebugFlagEnabled (rmanager->dbgFlagClipPlanes))
        context.owner.AddDebugClipPlanes (rview);

      // Do the culling
      iVisibilityCuller* culler = sector->GetVisibilityCuller ();
      Viscull<RenderTreeType> (context, rview, culler);

      HandleContextMeshes (context);
    }

    void HandleContextMeshes (typename RenderTreeType::ContextNode& context)
    {
      CS::RenderManager::RenderView* rview = context.renderView;
      iShaderManager* shaderManager = rmanager->shaderManager;
      iSector* sector = rview->GetThisSector ();

      typename ShadowType::ShadowParameters shadowViewSetup (
        WrapShadowParams<ShadowType>::Create (
        rmanager->lightPersistent.shadowPersist, rview));

      // Sort the mesh lists  
      {
        StandardMeshSorter<RenderTreeType> mySorter (rview->GetEngine ());
        mySorter.SetupCameraLocation (rview->GetCamera ()->GetTransform ().GetOrigin ());
        ForEachMeshNode (context, mySorter);
      }

      // After sorting, assign in-context per-mesh indices
      {
        SingleMeshContextNumbering<RenderTreeType> numbering;
        ForEachMeshNode (context, numbering);
      }

      // Setup the SV arrays
      // Push the default stuff
      SetupStandardSVs (context, layerConfig, shaderManager, sector);

      // Setup the material&mesh SVs
      {
        StandardSVSetup<RenderTreeType, MultipleRenderLayer> svSetup (
          context.svArrays, layerConfig);

        ForEachMeshNode (context, svSetup);
      }

      SetupStandardShader (context, shaderManager, layerConfig);

      RMOSM::LightSetupType lightSetup (
        rmanager->lightPersistent, rmanager->lightManager,
        context.svArrays, layerConfig, shadowViewSetup);

      ForEachMeshNode (context, lightSetup);

      // Setup shaders and tickets
      SetupStandardTicket (context, shaderManager,
        lightSetup.GetPostLightingLayers());
    }

  private:
    RMOSM* rmanager;
    const LayerConfigType& layerConfig;

  };

  RMOSM::RMOSM (iBase* parent)
    : scfImplementationType (this, parent)
  {
    SetTreePersistent (treePersistent);
  }

  bool RMOSM::RenderView (iView* view, bool recursePortals)
  {
    // Setup a rendering view
    view->UpdateClipper ();

    csRef<CS::RenderManager::RenderView> rview;
    rview = treePersistent.renderViews.GetRenderView (view);
    iGraphics3D* G3D = rview->GetGraphics3D ();
    int frameWidth = G3D->GetWidth ();
    int frameHeight = G3D->GetHeight ();
    view->GetCamera () ->SetViewportSize (frameWidth, frameHeight);
    view->GetEngine ()->UpdateNewFrame ();  
    view->GetEngine ()->FireStartFrame (rview);

    // Computes the left, right, top, and bottom of the view frustum.
    iPerspectiveCamera *camera = view->GetPerspectiveCamera ();
    float invFov = camera->GetInvFOV ();
    float l = -invFov * camera->GetShiftX ();
    float r =  invFov * (frameWidth - camera->GetShiftX ());
    float t = -invFov * camera->GetShiftY ();
    float b =  invFov * (frameHeight - camera->GetShiftY ());
    rview->SetFrustum (l, r, t, b);

    lightPersistent.UpdateNewFrame ();

    iSector* startSector = rview->GetThisSector ();

    if (!startSector)
      return false;

    // Pre-setup culling graph
    RenderTreeType renderTree (treePersistent);

    RenderTreeType::ContextNode* startContext = renderTree.CreateContext (rview);
    startContext->drawFlags |= (CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER);

    // Setup the main context
    {
      ContextSetupType contextSetup (this, renderLayer);
      ContextSetupType::PortalSetupType::ContextSetupData portalData (startContext);

      contextSetup (*startContext, portalData, recursePortals);
    }

    // Render all contexts, back to front
    {
      view->GetContext()->SetZMode (CS_ZBUF_MESH);

      SimpleTreeRenderer<RenderTreeType> render (rview->GetGraphics3D (),
        shaderManager);
      ForEachContextReverse (renderTree, render);
    }

    // Output the final result to the backbuffer.
    DebugFrameRender (rview, renderTree);

    return true;
  }

  bool RMOSM::RenderView (iView* view)
  {
    return RenderView (view, true);
  }

  bool RMOSM::PrecacheView (iView* view)
  {
    if (!RenderView (view, false)) return false;

    return true;
  }

  bool RMOSM::Initialize(iObjectRegistry* objectReg)
  {
    const char messageID[] = "crystalspace.rendermanager.osm";

    this->objectReg = objectReg;

    shaderManager = csQueryRegistry<iShaderManager> (objectReg);
    lightManager = csQueryRegistry<iLightManager> (objectReg);

    csConfigAccess cfg (objectReg);
    bool layersValid = false;
    const char* layersFile = cfg->GetStr ("RenderManager.OSM.Layers", 0);
    if (layersFile)
    {
      layersValid = CS::RenderManager::AddLayersFromFile (objectReg, layersFile,
        renderLayer);
      if (!layersValid) renderLayer.Clear();
    }

    csRef<iLoader> loader (csQueryRegistry<iLoader> (objectReg));
    if (!layersValid)
    {
      if (!loader->LoadShader ("/shader/lighting/lighting_default.xml"))
        csReport (objectReg, CS_REPORTER_SEVERITY_WARNING,
        messageID, "Could not load lighting_default shader");

      CS::RenderManager::AddDefaultBaseLayers (objectReg, renderLayer);
    }

    csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objectReg);
    treePersistent.Initialize (shaderManager);
    dbgFlagClipPlanes =
      treePersistent.debugPersist.RegisterDebugFlag ("draw.clipplanes.view");

    lightPersistent.shadowPersist.SetConfigPrefix ("RenderManager.OSM");
    lightPersistent.Initialize (objectReg, treePersistent.debugPersist);

    RMViscullCommon::Initialize (objectReg, "RenderManager.OSM");

    return true;
  }


  //----------------------------------------------------------------------
  bool RMOSM::DebugCommand(const char *cmd)
  {
    if (strcmp (cmd, "reset") == 0)
    {
      uint flag = treePersistent.debugPersist.QueryDebugFlag("draw.osm.choose.split");
      treePersistent.debugPersist.EnableDebugFlag(flag,true);
      return true;
    }
    return false;
  }

}
CS_PLUGIN_NAMESPACE_END(RMOSM)
