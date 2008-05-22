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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_PSSM_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_PSSM_H__

#include "ivideo/shader/shader.h"

#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/shadow_common.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/viscull.h"

#include "csgeom/matrix4.h"

class csShaderVariable;

// Number of view frustum parts
#define NUM_PARTS 3

namespace CS
{
namespace RenderManager
{
  struct ShadowPSSMExtraMeshData
  {
    csRef<csShaderVariable> svMeshID;
  };

  template<typename RenderTree, typename LayerConfigType>
  class ShadowPSSM
  {
  public:
    struct PersistentData;
    
    class ViewSetup
    {
    public:
      PersistentData& persist;
      
      CS::RenderManager::RenderView* rview;
      float splitDists[NUM_PARTS+1];
      float lx, rx, ty, by;
      
      SingleRenderLayer depthRenderLayer;
      uint lastMeshID;
      csHash<csRef<csShaderVariable>, csPtrKey<iShaderVariableContext> > meshIDs;
      
      ViewSetup (PersistentData& persist, CS::RenderManager::RenderView* rview)
       : persist (persist), rview (rview),
         depthRenderLayer (persist.settings.shadowShaderType, 
           persist.settings.shadowDefaultShader),
         lastMeshID (0)
      {
	// PSSM: split layers
	
	// @@@ FIXME: arbitrary
	float _near = SMALL_Z;
	float _far = 300.0f;
      
	splitDists[0] = _near;
	for (int i = 0; i <= NUM_PARTS; i++)
	{
	  const float n = _near, f = _far;
	  const float iFrac = (float)i/(float)NUM_PARTS;
	  splitDists[i] = (n * pow (f/n, iFrac) + n + (f-n)*iFrac)*0.5f;
	}
	// Get visible frustum in to lx/ty,rx/by
	{
	  int frameWidth = rview->GetGraphics3D()->GetWidth ();
	  int frameHeight = rview->GetGraphics3D()->GetHeight ();
	  lx = 0;
	  rx = frameWidth;
	  ty = frameHeight;
	  by = 0;
	}
      }
    
      void PostLightSetup (typename RenderTree::ContextNode& context,
        const LayerConfigType& layerConfig)
      {
      }
    };
    
    struct CachedLightData :
      public CS::Memory::CustomAllocated
    {
      bool frustumsSetup;
      // Transform light space to post-project light space
      CS::Math::Matrix4 lightProject;
      struct SuperFrustum : public CS::Utility::FastRefCount<SuperFrustum>
      {
	// Transform world space to light space
	csReversibleTransform world2light_base;
	csReversibleTransform world2light_rotated;
	csMatrix3 frustumRotation;
	struct Frustum
	{
	  // Volume (in post-project light space)
	  csBox3 volumeLS;
	  csBox3 volumePP;
	  csShaderVariable* shadowMapProjectSV;
	  csShaderVariable* shadowMapUnscaleSV;
	  csShaderVariable* shadowMapDimSV;
	  csShaderVariable* shadowClipSV;
	  csShaderVariable** textureSVs;
	  
	  // Object bboxes in post-project light space
	  csArray<csBox3> containedObjectsPP;
	  
	  Frustum() : textureSVs (0) {}
	  ~Frustum() { delete[] textureSVs; }
	};
	Frustum frustums[NUM_PARTS];
      };
      csRefArray<SuperFrustum> lightFrustums;
      
      CachedLightData() : frustumsSetup (false) {}
      
      uint GetSublightNum() const { return (uint)lightFrustums.GetSize(); }

      void SetupFrame (RenderTree& tree, ShadowPSSM& shadows, iLight* light)
      {
        if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;
        
        ViewSetup& viewSetup = shadows.viewSetup;
        csRef<iCamera> camera (viewSetup.rview->GetCamera());
        if (!frustumsSetup)
        {
          float lightNear = SMALL_Z;
          float lightCutoff = light->GetCutoffDistance();
          CS::Math::Matrix4 lightProject;
          csLightType ltype = light->GetType();
          int numFrustums;
          switch (ltype)
          {
            case CS_LIGHT_DIRECTIONAL:
              numFrustums = 1;
              {
		lightProject = CS::Math::Matrix4 (
		  1.0f/lightCutoff, 0, 0, 0,
		  0, 1.0f/lightCutoff, 0, 0,
		  0, 0, -1, 0,
		  0, 0, 0, 1);
		/*CS::Math::Matrix4 flipZW (
		  1, 0, 0, 0,
		  0, 1, 0, 0,
		  0, 0, -1, 0,
		  0, 0, 0, -1);
		lightProject = flipZW * CS::Math::Projections::Ortho (
		  -lightCutoff, lightNear, -lightCutoff, lightCutoff,
		  lightNear, lightCutoff);*/
	      }
              break;
            case CS_LIGHT_POINTLIGHT:
            case CS_LIGHT_SPOTLIGHT:
              numFrustums = (ltype == CS_LIGHT_POINTLIGHT) ? 6 : 1;
              {
		CS::Math::Matrix4 flipZW (
		  1, 0, 0, 0,
		  0, 1, 0, 0,
		  0, 0, -1, 0,
		  0, 0, 0, -1);
		lightProject = flipZW * CS::Math::Projections::Frustum (
		  -lightNear, lightNear, -lightNear, lightNear, // @@@ TODO: use spot angle
		  lightNear, lightCutoff);
              }
              break;
          }
          const csReversibleTransform& world2light_base (
            light->GetMovable()->GetFullTransform());
	  this->lightProject = lightProject;
            
          const csBox3& lightBBox = light->GetLocalBBox();
          if (shadows.persist.debugFlags
              & ShadowPSSM::PersistentData::dbgLightBBox)
          {
	    tree.AddDebugBBox (lightBBox,
	      world2light_base.GetInverse(),
	      csColor (1, 1, 1));
	  }

          static const csMatrix3 frustRotationMatrices[6] =
          {
            csMatrix3 (), // must be identity
            csMatrix3 (0, 0, 1,  0, 1, 0,  -1, 0, 0),
            csMatrix3 (1, 0, 0,  0, 0, 1,  0, -1, 0),
            csMatrix3 (0, 0, -1,  0, 1, 0,  1, 0, 0),
            csMatrix3 (1, 0, 0,  0, 0, -1,  0, 1, 0),
            csMatrix3 (1, 0, 0,  0, -1, 0,  0, 0, -1) // last must be 180 deg into other direction
          };
	  csVector2 corner2D[4] = {
	    csVector2 (viewSetup.lx, viewSetup.ty),
	    csVector2 (viewSetup.lx, viewSetup.by),
	    csVector2 (viewSetup.rx, viewSetup.ty),
	    csVector2 (viewSetup.rx, viewSetup.by)
	  };

	  LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);
	  for (int f = 0; f < numFrustums; f++)
	  {
	    /* Compute intersection of light frustum with view frustums:
		1. Compute corners of split frustums
		2. Translate corners to light space
		3. Compute bounding box from corners
	      */
	    csRef<SuperFrustum> newFrust;
	    newFrust.AttachNew (new SuperFrustum);
	    SuperFrustum& superFrustum = *(lightFrustums[lightFrustums.Push (
	      newFrust)]);
	    superFrustum.world2light_base = world2light_base;
	    superFrustum.world2light_rotated = world2light_base;
	    superFrustum.frustumRotation = frustRotationMatrices[f];
	    superFrustum.world2light_rotated.SetO2T (
	      superFrustum.world2light_rotated.GetO2T()
	      * frustRotationMatrices[f]);
	    
	    for (int i = 0; i < NUM_PARTS; i++)
	    {
	      typename SuperFrustum::Frustum& lightFrustum =
	        superFrustum.frustums[i];
	    
	      // Frustum corner, camera space
	      csVector3 cornerCam;
	      // Frustum corner, world space
	      csVector3 cornerWorld;
	      // Frustum corner, light space
	      csVector3 cornerLight;
	      // Frustum slice bbox, light space
	      csBox3 frustumLight;
	      csBox3 frustumCam;
	      frustumLight.StartBoundingBox();
	      frustumCam.StartBoundingBox();
	      for (int c = 0; c < 4; c++)
	      {
		cornerCam = camera->InvPerspective (
		  corner2D[c], viewSetup.splitDists[i]);
		frustumCam.AddBoundingVertex (cornerCam);
		cornerWorld = camera->GetTransform().This2Other (
		  cornerCam);
		cornerLight = superFrustum.world2light_rotated.Other2This (cornerWorld);
		frustumLight.AddBoundingVertex (cornerLight);
		
		cornerCam = camera->InvPerspective (
		  corner2D[c], viewSetup.splitDists[i+1]);
		frustumCam.AddBoundingVertex (cornerCam);
		cornerWorld = camera->GetTransform().This2Other (
		  cornerCam);
		cornerLight = superFrustum.world2light_rotated.Other2This (cornerWorld);
		frustumLight.AddBoundingVertex (cornerLight);
	      }
	      if (shadows.persist.debugFlags
		  & ShadowPSSM::PersistentData::dbgSplitFrustumCam)
	      {
		tree.AddDebugBBox (frustumCam,
		  camera->GetTransform().GetInverse(),
		  csColor (1, 0, 1));
	      }
	      if (shadows.persist.debugFlags
		  & ShadowPSSM::PersistentData::dbgSplitFrustumLight)
	      {
		tree.AddDebugBBox (frustumLight,
		  superFrustum.world2light_rotated.GetInverse(),
		  csColor (0, 1, 0));
	      }
	      frustumLight *= lightBBox;
	      if (shadows.persist.debugFlags
		  & ShadowPSSM::PersistentData::dbgSplitFrustumLight)
	      {
		tree.AddDebugBBox (frustumLight,
		  superFrustum.world2light_rotated.GetInverse(),
		  csColor (0, 1, 1));
	      }
	      
	      // Frustum slice corner, light space before W divide
	      csVector4 cornerUndiv;
	      // Frustum slice corner, light space after W divide
	      csVector3 cornerDiv;
	      
	      lightFrustum.volumeLS.StartBoundingBox();
	      lightFrustum.volumePP.StartBoundingBox();
	      for (int c = 0; c < 7; c++)
	      {
		cornerLight = frustumLight.GetCorner (c);
		lightFrustum.volumeLS.AddBoundingVertex (cornerLight);
		cornerUndiv = lightProject * cornerLight;
		cornerDiv =
		  csVector3 (cornerUndiv.x, cornerUndiv.y, cornerUndiv.z);
		cornerDiv /= cornerUndiv.w;
		lightFrustum.volumePP.AddBoundingVertex (cornerDiv);
	      }
	      tree.AddDebugBBox (lightFrustum.volumeLS,
	        superFrustum.world2light_rotated.GetInverse(),
	        csColor (0, 0, 1));
	    
	      lightFrustum.shadowMapProjectSV = lightVarsHelper.CreateTempSV (
		viewSetup.persist.svNames.GetLightSVId (
		  csLightShaderVarCache::lightShadowMapProjection));
	      lightFrustum.shadowMapProjectSV->SetArraySize (4);
	      for (int j = 0; j < 4; j++)
	      {
		csShaderVariable* item = lightVarsHelper.CreateTempSV (
		  CS::InvalidShaderVarStringID);
		lightFrustum.shadowMapProjectSV->SetArrayElement (j, item);
	      }
	      lightFrustum.shadowMapUnscaleSV = lightVarsHelper.CreateTempSV (
		viewSetup.persist.unscaleSVName);
	      lightFrustum.shadowMapDimSV = lightVarsHelper.CreateTempSV (
		viewSetup.persist.svNames.GetLightSVId (
		  csLightShaderVarCache::lightShadowMapPixelSize));
		  
	      lightFrustum.shadowClipSV = lightVarsHelper.CreateTempSV (
		viewSetup.persist.shadowClipSVName);
	      lightFrustum.shadowClipSV->SetValue (csVector2 (
		viewSetup.splitDists[i], viewSetup.splitDists[i+1]));
		  
	      size_t numTex = viewSetup.persist.settings.targets.GetSize();
	      if (lightFrustum.textureSVs == 0)
	      {
		lightFrustum.textureSVs = new csShaderVariable*[numTex];
	      }
	      for (size_t t = 0; t < numTex; t++)
	      {
		lightFrustum.textureSVs[t] = lightVarsHelper.CreateTempSV (
		  viewSetup.persist.settings.targets[t]->svName);
	      }
	    }
	  }
        
          frustumsSetup = true;
        }
      }
      
      void AddShadowMapTarget (typename RenderTree::MeshNode* meshNode,
	PersistentData& persist, const SingleRenderLayer& layerConfig,
	RenderTree& renderTree, iLight* light, CachedLightData& lightData,
	ViewSetup& viewSetup)
      {
        if (light->GetFlags().Check (CS_LIGHT_NOSHADOWS)) return;
        
        typename RenderTree::ContextNode& context = meshNode->owner;
      
        CS_ALLOC_STACK_ARRAY(iTextureHandle*, texHandles,
          persist.settings.targets.GetSize());
        float allMinZ = HUGE_VALF;
	for (size_t l = 0; l < lightData.lightFrustums.GetSize(); l++)
	{
	  const SuperFrustum& superFrust = *(lightData.lightFrustums[l]);
	  for (int frustNum = 0; frustNum < NUM_PARTS; frustNum++)
	  {
	    const typename SuperFrustum::Frustum& lightFrust = superFrust.frustums[frustNum];
	    if (lightFrust.containedObjectsPP.GetSize() == 0) continue;
	    
	    /* Fit map to the bounding box of all shadowed objects.
	      - If the shadowed objects are smaller than the light frustum in some
		dimension makes sure the shadow map is used optimally. */
	    csBox3 allObjsBoxPP (lightFrust.containedObjectsPP[0]);
	    for (size_t i = 1; i < lightFrust.containedObjectsPP.GetSize(); i++)
	    {
	      allObjsBoxPP += lightFrust.containedObjectsPP[i];
	    }
	    allObjsBoxPP *= lightFrust.volumePP;
	    csBox3 clipToView;
	    if (light->GetType() == CS_LIGHT_DIRECTIONAL)
	      clipToView = csBox3 (csVector3 (-1, -1, -HUGE_VALF),
	        csVector3 (1, 1, 0));
	    else
	      clipToView = csBox3 (csVector3 (-1, -1, 0),
	        csVector3 (1, 1, HUGE_VALF));
	    allObjsBoxPP *= clipToView;
	    if (allObjsBoxPP.Empty())
	    {
	      /*csPrintf ("lightFrust.containedObjectsPP.GetSize() = %zu\n",
	        lightFrust.containedObjectsPP.GetSize());
	      csPrintf ("allObjsBoxPP = %s\n",
	        allObjsBoxPP.Description().GetData());*/
	      //continue;
	    }
	    
	    CS::RenderManager::RenderView* rview = context.renderView;
	  #include "csutil/custom_new_disable.h"
	    csRef<CS::RenderManager::RenderView> newRenderView;
	    newRenderView.AttachNew (new (
	      renderTree.GetPersistentData().renderViewPool) RenderView);
	  #include "csutil/custom_new_enable.h"
	    newRenderView->SetEngine (rview->GetEngine ());
	    newRenderView->SetThisSector (rview->GetThisSector ());
	    
	    const float frustW = allObjsBoxPP.MaxX() - allObjsBoxPP.MinX();
	    const float frustH = allObjsBoxPP.MaxY() - allObjsBoxPP.MinY();
	    const float cropScaleX = 2.0f/frustW;
	    const float cropScaleY = -2.0f/frustH;
	    const float cropShiftX =
	      (-1.0f * (allObjsBoxPP.MaxX() + allObjsBoxPP.MinX()))/frustW;
	    const float cropShiftY =
	      (1.0f * (allObjsBoxPP.MaxY() + allObjsBoxPP.MinY()))/frustH;
	    CS::Math::Matrix4 crop (
	      cropScaleX, 0, 0, cropShiftX,
	      0, cropScaleY, 0, cropShiftY,
	      0, 0, 1, 0,
	      0, 0, 0, 1);
	    //csPrintf (" crop = %s\n", crop.Description().GetData());
	    //csPrintf (" allObjsBoxPP = %s\n", allObjsBoxPP.Description().GetData());
	      
	    /* The minimum Z over all parts is used to avoid clipping shadows of 
	      casters closer to the light than the split plane */
	    if (allObjsBoxPP.MinZ() < allMinZ) allMinZ = allObjsBoxPP.MinZ();
	    //allMinZ = csMax (allMinZ, 0.0f);
	    float maxZ = allObjsBoxPP.MaxZ();
	    /* Consider using DepthRange? */
	    float n = allObjsBoxPP.MinZ();//allMinZ;
	    float f = maxZ + EPSILON;
	    //csPrintf (" n = %f  f = %f\n", n, f);
	    /* Sometimes n==f which would result in an invalid matrix w/o EPSILON */
	    CS::Math::Matrix4 Mortho = CS::Math::Projections::Ortho (-1, 1, 1, -1, -n, -f);
	    CS::Math::Matrix4 matrix = ((Mortho * crop) * lightData.lightProject)
	     * CS::Math::Matrix4 (superFrust.frustumRotation);
	    //csPrintf ("matrix = %s\n", matrix.Description().GetData());
	    
	    int shadowMapSize;
	    csReversibleTransform view = superFrust.world2light_base;
	    
	    csRef<iCustomMatrixCamera> shadowViewCam =
	      newRenderView->GetEngine()->CreateCustomMatrixCamera();
	    newRenderView->SetCamera (shadowViewCam->GetCamera());
	    shadowViewCam->SetProjectionMatrix (matrix);
	    shadowViewCam->GetCamera()->SetTransform (view);
	    
	    //CS::Math::Matrix4 viewM (view);
	  
	    for (int i = 0; i < 4; i++)
	    {
	      csShaderVariable* item = lightFrust.shadowMapProjectSV->GetArrayElement (i);
	      item->SetValue (matrix.Row (i));
	    }
	    {
	      const float uncropScaleX = 1.0f/cropScaleX;
	      const float uncropScaleY = 1.0f/cropScaleY;
	      const float uncropShiftX = -cropShiftX/cropScaleX;
	      const float uncropShiftY = -cropShiftY/cropScaleY;
	      lightFrust.shadowMapUnscaleSV->SetValue (csVector4 (
	        uncropScaleX, uncropScaleY, uncropShiftX, uncropShiftY));
	    }
		
            // @@@ FIXME: That should be configurable! Prolly per light type.
	    shadowMapSize = 512;
	    lightFrust.shadowMapDimSV->SetValue (csVector4 (1.0f/shadowMapSize,
	      1.0f/shadowMapSize, shadowMapSize, shadowMapSize));
    
	    for (size_t t = 0; t < persist.settings.targets.GetSize(); t++)
	    {
	      iTextureHandle* tex =
		persist.settings.targets[t]->texCache.QueryUnusedTexture (
		  shadowMapSize, shadowMapSize, 0);
	      lightFrust.textureSVs[t]->SetValue (tex);
	      renderTree.AddDebugTexture (tex);
	      texHandles[t] = tex;
	    }
	    
	    csBox2 clipBox (0, 0, shadowMapSize, shadowMapSize);
	    csRef<iClipper2D> newView;
	    newView.AttachNew (new csBoxClipper (clipBox));
	    newRenderView->SetClipper (newView);
    
	    // Create a new context for shadow map w/ computed view
	    typename RenderTree::ContextNode* shadowMapCtx = 
	      renderTree.CreateContext (newRenderView);
	    for (size_t t = 0; t < persist.settings.targets.GetSize(); t++)
	    {
	      shadowMapCtx->renderTargets[
		persist.settings.targets[t]->attachment].texHandle = 
		  texHandles[t];
	    }
	    shadowMapCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
	    shadowMapCtx->postEffects = persist.settings.postEffects;
	    
	    /* @@@ FIXME: This will break as soon as the shadowmaps have
	      different resolutions!
	      Probably the post effects manager should be changed to handle
	      changing resolutions well */
	    if (shadowMapCtx->postEffects.IsValid())
	      shadowMapCtx->postEffects->SetupView (shadowMapSize, shadowMapSize);
    
	    // Setup the new context
	    ShadowmapContextSetup contextFunction (layerConfig,
	      persist.shaderManager, viewSetup, persist.settings.provideIDs);
	    contextFunction (*shadowMapCtx);
	  }
	}
      }

      void ClearFrameData()
      {
        frustumsSetup = false;
        lightFrustums.Empty();
      }
    };
private:
    class ShadowmapContextSetup
    {
      class MeshIDSVSetup
      {
      public:    
	MeshIDSVSetup (SVArrayHolder& svArrays, ViewSetup& viewSetup) 
	 : svArrays (svArrays), viewSetup (viewSetup)
	{
	  tempStack.Setup (svArrays.GetNumSVNames ());
	}
    
	void operator() (typename RenderTree::MeshNode* node)
	{
	  for (size_t i = 0; i < node->meshes.GetSize (); ++i)
	  {
	    typename RenderTree::MeshNode::SingleMesh& mesh = node->meshes[i];
	    csShaderVariable* svMeshID = viewSetup.meshIDs.Get (mesh.meshObjSVs, 0);
	    if (!svMeshID) continue;
	    
	    tempStack[svMeshID->GetName()] = svMeshID;
	    
	    for (size_t layer = 0; layer < svArrays.GetNumLayers(); ++layer)
	    {
	      // Back-merge it onto the real one
	      csShaderVariableStack localStack;
	      svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);
	      localStack.MergeFront (tempStack);
	    }
	  }
	}
    
      private:
	SVArrayHolder& svArrays; 
	csShaderVariableStack tempStack;
	ViewSetup& viewSetup;
      };
    public:
      ShadowmapContextSetup (const SingleRenderLayer& layerConfig,
        iShaderManager* shaderManager, ViewSetup& viewSetup,
        bool doIDTexture)
	: layerConfig (layerConfig), shaderManager (shaderManager),
	  viewSetup (viewSetup), doIDTexture (doIDTexture)
      {
    
      }
    
      void operator() (typename RenderTree::ContextNode& context)
      {
	CS::RenderManager::RenderView* rview = context.renderView;
	iSector* sector = rview->GetThisSector ();
    
	// @@@ This is somewhat "boilerplate" sector/rview setup.
	rview->SetThisSector (sector);
	sector->CallSectorCallbacks (rview);
	// Make sure the clip-planes are ok
	CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());
    
	// Do the culling
	iVisibilityCuller* culler = sector->GetVisibilityCuller ();
	Viscull<RenderTree> (context, rview, culler);
    
        // TODO: portals
	
	// Sort the mesh lists  
	{
	  StandardMeshSorter<RenderTree> mySorter (rview->GetEngine ());
	  mySorter.SetupCameraLocation (rview->GetCamera ()->GetTransform ().GetOrigin ());
	  ForEachMeshNode (context, mySorter);
	}
    
	// After sorting, assign in-context per-mesh indices
	{
	  SingleMeshContextNumbering<RenderTree> numbering;
	  ForEachMeshNode (context, numbering);
	}

	// Setup the SV arrays
	// Push the default stuff
	SetupStandardSVs (context, layerConfig, shaderManager, sector);
    
	// Setup the material&mesh SVs
	{
	  StandardSVSetup<RenderTree, SingleRenderLayer> svSetup (
	    context.svArrays, layerConfig);
    
	  ForEachMeshNode (context, svSetup);
	}
	// Set up mesh ID SVs
	if (doIDTexture)
	{
	  MeshIDSVSetup svSetup (context.svArrays, viewSetup);
    
	  ForEachMeshNode (context, svSetup);
	}
    
	SetupStandardShader (context, shaderManager, layerConfig);
    
	// Setup shaders and tickets
	SetupStandardTicket (context, shaderManager, layerConfig);
      }
    
    
    private:
      const SingleRenderLayer& layerConfig;
      iShaderManager* shaderManager;
      ViewSetup& viewSetup;
      bool doIDTexture;
    };
  public:

    struct PersistentData
    {
      enum
      {
        dbgSplitFrustumCam = 1,
        dbgSplitFrustumLight = 2,
        dbgLightBBox = 4
      };
      uint debugFlags;
      csLightShaderVarCache svNames;
      CS::ShaderVarStringID unscaleSVName;
      CS::ShaderVarStringID shadowClipSVName;
      LightingSorter::PersistentData lightSorterPersist;
      LightingVariablesHelper::PersistentData lightVarsPersist;
      iShaderManager* shaderManager;

      csString shadowType;
      ShadowSettings settings;

      PersistentData() : debugFlags (0), shadowType ("Depth")
      {
      }

      ~PersistentData()
      {
      }
      
      void SetShadowType (const char* shadowType)
      {
        this->shadowType = shadowType;
      }
      
      void Initialize (iObjectRegistry* objectReg)
      {
        csRef<iShaderManager> shaderManager =
          csQueryRegistry<iShaderManager> (objectReg);
      
        this->shaderManager = shaderManager;
        iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	svNames.SetStrings (strings);
	
	settings.ReadSettings (objectReg, shadowType);
	unscaleSVName = strings->Request ("light shadow map unscale");
	shadowClipSVName = strings->Request ("light shadow clip");
      }
      void UpdateNewFrame ()
      {
        csTicks time = csGetTicks ();
        settings.AdvanceFrame (time);
      }
    };
    
    typedef ViewSetup ShadowParameters;

    ShadowPSSM (PersistentData& persist,
      const LayerConfigType& layerConfig,
      typename RenderTree::MeshNode* node, 
      ViewSetup& viewSetup) 
      : persist (persist), layerConfig (layerConfig), 
        renderTree (node->owner.owner), meshNode (node),
        viewSetup (viewSetup) { }
        
    uint HandleOneLight (typename RenderTree::MeshNode::SingleMesh& singleMesh,
                         iLight* light, CachedLightData& lightData,
                         csShaderVariableStack* lightStacks,
                         uint lightNum, uint subLightNum)
    {
      LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);
      
      if (persist.settings.provideIDs && !singleMesh.svMeshID.IsValid())
      {
        singleMesh.svMeshID = lightVarsHelper.CreateTempSV (
	  viewSetup.persist.settings.svMeshIDName);
        lightStacks[0][singleMesh.svMeshID->GetName()] = singleMesh.svMeshID;
        uint meshID = ++viewSetup.lastMeshID;
        singleMesh.svMeshID->SetValue ((int)meshID);
        viewSetup.meshIDs.Put (singleMesh.meshObjSVs, singleMesh.svMeshID);
      }
      
      typename CachedLightData::SuperFrustum& superFrust =
	*(lightData.lightFrustums[subLightNum]);
      
      csBox3 meshBboxLS;
      csVector3 vLight;
      vLight = superFrust.world2light_rotated.Other2This (
        singleMesh.bbox.GetCorner (0));
      meshBboxLS.StartBoundingBox (csVector3 (vLight.x,
	vLight.y, vLight.z));
      csBox3 meshBboxLightPP;
      csVector4 vLightPP;
      vLightPP = lightData.lightProject * csVector4 (vLight);
      vLightPP /= vLightPP.w;
      meshBboxLightPP.StartBoundingBox (csVector3 (vLightPP.x,
	vLightPP.y, vLightPP.z));
      for (int c = 1; c < 8; c++)
      {
	vLight = superFrust.world2light_rotated.Other2This (
	  singleMesh.bbox.GetCorner (c));
	meshBboxLS.AddBoundingVertexSmart (csVector3 (vLight.x,
	  vLight.y, vLight.z));
	vLightPP = lightData.lightProject * csVector4 (vLight);
	vLightPP /= vLightPP.w;
	meshBboxLightPP.AddBoundingVertexSmart (csVector3 (vLightPP.x,
	  vLightPP.y, vLightPP.z));
      }
	
      uint spreadFlags = 0;
      int s = 0;
      for (int f = 0; f < NUM_PARTS; f++)
      {
	typename CachedLightData::SuperFrustum::Frustum& lightFrustum =
	  superFrust.frustums[f];
        //if (lightFrustum.volumePP.Empty()) continue;
	if (!lightFrustum.volumeLS.TestIntersect (meshBboxLS))
	//if (!lightFrustum.volumePP.TestIntersect (meshBboxLightPP))
	{
	  continue;
	}
      
	lightFrustum.containedObjectsPP.Push (meshBboxLightPP
	  /** lightFrustum.volumePP*/);
      
	// Add shadow map SVs
	lightVarsHelper.MergeAsArrayItem (lightStacks[s],
	  lightFrustum.shadowMapProjectSV, lightNum);
	lightVarsHelper.MergeAsArrayItem (lightStacks[s],
	  lightFrustum.shadowMapUnscaleSV, lightNum);
	lightVarsHelper.MergeAsArrayItem (lightStacks[s], 
	  lightFrustum.shadowMapDimSV, lightNum);
	if (lightStacks[s].GetSize() > lightFrustum.shadowClipSV->GetName())
	{
	  lightStacks[s][lightFrustum.shadowClipSV->GetName()] =
	    lightFrustum.shadowClipSV;
	 }
	  
	for (size_t t = 0; t < persist.settings.targets.GetSize(); t++)
	{
	  lightVarsHelper.MergeAsArrayItem (lightStacks[s], 
	    lightFrustum.textureSVs[t], lightNum);
	}
	spreadFlags |= (1 << s);
	s++;
      }
      return spreadFlags;
    }
    
    static bool NeedFinalHandleLight() { return true; }
    void FinalHandleLight (iLight* light, CachedLightData& lightData)
    {
      lightData.AddShadowMapTarget (meshNode, persist,
        viewSetup.depthRenderLayer, renderTree, light, lightData, viewSetup);
      
      lightData.ClearFrameData();
    }

    csFlags GetLightFlagsMask () const { return csFlags (0); }
    
    size_t GetLightLayerSpread() const { return NUM_PARTS; }
  protected:
    PersistentData& persist;
    const LayerConfigType& layerConfig;
    RenderTree& renderTree;
    typename RenderTree::MeshNode* meshNode;
    ViewSetup& viewSetup;
  };

}
}

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_PSSM_H__
