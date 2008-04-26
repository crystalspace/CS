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
         depthRenderLayer (persist.shadowShaderType, persist.defaultShadowShader),
         lastMeshID (0)
      {
	// PSSM: split layers
	
	// @@@ FIXME: arbitrary
	float near = SMALL_Z;
	float far = 100.0f;
      
	splitDists[0] = near;
	for (int i = 0; i <= NUM_PARTS; i++)
	{
	  const float n = near, f = far;
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
    
    struct CachedLightData
    {
      uint shadowMapCreateFrame;

      bool frustumsSetup;
      struct Frustum
      {
        csBox3 volume;
        CS::Math::Matrix4 world2lightNorm;
	csShaderVariable* shadowMapSV;
	csShaderVariable* shadowMapProjectSV;
	csShaderVariable* shadowMapIDSV;
	csShaderVariable* shadowMapDimSV;
	
	csArray<csBox3> containedObjects;
      };
      Frustum lightFrustums[NUM_PARTS];
      
      CachedLightData() : shadowMapCreateFrame (~0), frustumsSetup (false) {}

      void SetupFrame (ViewSetup& viewSetup, iLight* light)
      {
        if (!frustumsSetup)
        {
          CS::Math::Matrix4 world2lightNorm (
            light->GetMovable()->GetFullTransform());
        
	  LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);
	  /* Compute intersection of light frustum with view frustums:
	      1. Compute corners of split frustums
	      2. Translate corners to light space
	      3. Compute bounding box from corners
	    */
	  for (int i = 0; i < NUM_PARTS; i++)
	  {
	    csVector2 corner2D[4] = {
	      csVector2 (viewSetup.lx, viewSetup.ty),
	      csVector2 (viewSetup.lx, viewSetup.by),
	      csVector2 (viewSetup.rx, viewSetup.ty),
	      csVector2 (viewSetup.rx, viewSetup.by)
	    };
	    Frustum& lightFrustum = lightFrustums[i];
	    lightFrustum.world2lightNorm = world2lightNorm;
	    lightFrustum.volume.StartBoundingBox();
	    // Frustum corner, camera space
	    csVector3 cornerCam;
	    // Frustum corner, world space
	    csVector3 cornerWorld;
	    // Frustum corner, light space before W divide
	    csVector4 cornerUnproj;
	    // Frustum corner, light space after W divide
	    csVector3 cornerProj;
	    for (int c = 0; c < 4; c++)
	    {
	      cornerCam = viewSetup.rview->GetCamera()->InvPerspective (
		corner2D[c], viewSetup.splitDists[i]);
	      cornerWorld = viewSetup.rview->GetCamera()->GetTransform().This2Other (
		cornerCam);
	      cornerUnproj = lightFrustum.world2lightNorm * csVector4 (cornerWorld);
	      cornerProj =
		csVector3 (cornerUnproj.x, cornerUnproj.y, cornerUnproj.z);
	      cornerProj /= cornerUnproj.w;
	      lightFrustum.volume.AddBoundingVertex (cornerProj);
	      cornerCam = viewSetup.rview->GetCamera()->InvPerspective (
		corner2D[c], viewSetup.splitDists[i+1]);
	      cornerWorld = viewSetup.rview->GetCamera()->GetTransform().This2Other (
		cornerCam);
	      cornerUnproj = lightFrustum.world2lightNorm * csVector4 (cornerWorld);
	      cornerProj =
		csVector3 (cornerUnproj.x, cornerUnproj.y, cornerUnproj.z);
	      cornerProj /= cornerUnproj.w;
	      lightFrustum.volume.AddBoundingVertex (cornerProj);
	    }
	  
	    lightFrustum.shadowMapSV = lightVarsHelper.CreateTempSV (
	      viewSetup.persist.svNames.GetLightSVId (
		csLightShaderVarCache::lightShadowMap));
	    lightFrustum.shadowMapProjectSV = lightVarsHelper.CreateTempSV (
	      viewSetup.persist.svNames.GetLightSVId (
		csLightShaderVarCache::lightShadowMapProjection));
	    lightFrustum.shadowMapProjectSV->SetArraySize (4);
	    for (int i = 0; i < 4; i++)
	    {
	      csShaderVariable* item = lightVarsHelper.CreateTempSV (
		CS::InvalidShaderVarStringID);
	      lightFrustum.shadowMapProjectSV->SetArrayElement (i, item);
	    }
	    lightFrustum.shadowMapIDSV = lightVarsHelper.CreateTempSV (
	      viewSetup.persist.svNames.GetLightSVId (
		csLightShaderVarCache::lightShadowMapID));
	    lightFrustum.shadowMapDimSV = lightVarsHelper.CreateTempSV (
	      viewSetup.persist.svNames.GetLightSVId (
		csLightShaderVarCache::lightShadowMapPixelSize));
	  }
        
          frustumsSetup = true;
        }
      }
      
      void AddShadowMapTarget (typename RenderTree::MeshNode* meshNode,
	PersistentData& persist, const SingleRenderLayer& layerConfig,
	RenderTree& renderTree, iLight* light, CachedLightData& lightData,
	ViewSetup& viewSetup)
      {
        typename RenderTree::ContextNode& context = meshNode->owner;
      
        float allMinZ = HUGE_VALF;
	for (int f = 0; f < NUM_PARTS; f++)
	{
	  const Frustum& lightFrust = lightData.lightFrustums[f];
	  if (lightFrust.containedObjects.GetSize() == 0) continue;
	  
	  CS::RenderManager::RenderView* rview = context.renderView;
	#include "csutil/custom_new_disable.h"
	  csRef<CS::RenderManager::RenderView> newRenderView;
	  newRenderView.AttachNew (new (
	    renderTree.GetPersistentData().renderViewPool) RenderView);
	#include "csutil/custom_new_enable.h"
	  newRenderView->SetEngine (rview->GetEngine ());
	  newRenderView->SetThisSector (rview->GetThisSector ());
	  
	  /* Fit map to the bounding box of all shadowed objects.
	     Serves two purposes:
	     - If some objects cut the split plane (ie extend over the initial
	       light frustum) this makes sure they're mapped to the SM entirely.
	     - If the shadowed objects are smaller than the light frustum in some
	       dimension makes sure the shadow map is used optimally. */
	  csBox3 allObjsBox (lightFrust.containedObjects[0]);
	  for (size_t i = 1; i < lightFrust.containedObjects.GetSize(); i++)
	  {
	    allObjsBox += lightFrust.containedObjects[i];
	  }
	  
	  const float frustW = allObjsBox.MaxX() - allObjsBox.MinX();
	  const float frustH = allObjsBox.MaxY() - allObjsBox.MinY();
	  CS::Math::Matrix4 crop (
	    2.0f/frustW, 0, 0, 
	      (-1.0f * (allObjsBox.MaxX() + allObjsBox.MinX()))/frustW,
	    0, -2.0f/frustH, 0,
	      (1.0f * (allObjsBox.MaxY() + allObjsBox.MinY()))/frustH,
	    0, 0, 1, 0,
	    0, 0, 0, 1);
	  /* The minimum Z over all parts is used to avoid clipping shadows of 
	     casters closer to the light than the split plane */
	  if (allObjsBox.MinZ() < allMinZ) allMinZ = allObjsBox.MinZ();
	  /* Consider using DepthRange? */
	  float n = -allObjsBox.MaxZ(); //-1.0f;
	  float f = -allMinZ;//10.0f;
          CS::Math::Matrix4 Mortho = CS::Math::Projections::Ortho (-1, 1, 1, -1, n, f);
	  CS::Math::Matrix4 matrix = crop * Mortho;
	  
	  int shadowMapSize;
	  csReversibleTransform view = light->GetMovable()->GetFullTransform ();
	  
	  csRef<iCustomMatrixCamera> shadowViewCam =
	    newRenderView->GetEngine()->CreateCustomMatrixCamera();
	  newRenderView->SetCamera (shadowViewCam->GetCamera());
	  shadowViewCam->SetProjectionMatrix (matrix);
	  shadowViewCam->GetCamera()->SetTransform (view);
	
	  for (int i = 0; i < 4; i++)
	  {
	    csShaderVariable* item = lightFrust.shadowMapProjectSV->GetArrayElement (i);
	    item->SetValue (matrix.Row (i));
	  }
	      
	  shadowMapSize = 1024;
	  lightFrust.shadowMapDimSV->SetValue (csVector4 (1.0f/shadowMapSize,
	    1.0f/shadowMapSize, shadowMapSize, shadowMapSize));
  
	  iTextureHandle* shadowTex = persist.texCacheDepth.QueryUnusedTexture (
	    shadowMapSize, shadowMapSize, 0);
	  lightFrust.shadowMapSV->SetValue (shadowTex);
	  renderTree.AddDebugTexture (shadowTex);
	  
	  iTextureHandle* shadowIDTex = 0;
	  if (persist.doIDTexture)
	  {
	    shadowIDTex = persist.texCacheID.QueryUnusedTexture (
	      shadowMapSize, shadowMapSize, 0);
	    lightFrust.shadowMapIDSV->SetValue (shadowIDTex);
	    renderTree.AddDebugTexture (shadowIDTex);
	  }
  
	  csBox2 clipBox (0, 0, shadowMapSize, shadowMapSize);
	  csRef<iClipper2D> newView;
	  newView.AttachNew (new csBoxClipper (clipBox));
	  newRenderView->SetClipper (newView);
  
	  // Create a new context for shadow map w/ computed view
	  typename RenderTree::ContextNode* shadowMapCtx = 
	    renderTree.CreateContext (newRenderView);
	  shadowMapCtx->renderTargets[rtaColor0].texHandle = shadowIDTex;
	  shadowMapCtx->renderTargets[rtaDepth].texHandle = shadowTex;
	  shadowMapCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
  
	  // Setup the new context
	  ShadowmapContextSetup contextFunction (layerConfig,
	    persist.shaderManager, viewSetup, persist.doIDTexture);
	  contextFunction (*shadowMapCtx);
	}
      }

      void ClearFrameData()
      {
        frustumsSetup = false;
        for (int i = 0; i < NUM_PARTS; i++)
          lightFrustums[i].containedObjects.DeleteAll();
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
      uint frameNum;
      csLightShaderVarCache svNames;
      LightingSorter::PersistentData lightSorterPersist;
      LightingVariablesHelper::PersistentData lightVarsPersist;
      iShaderManager* shaderManager;
      csStringID shadowShaderType;
      csRef<iStringSet> strings;
      csRef<iShader> defaultShadowShader;
      CS::ShaderVarStringID svMeshIDName;

      TextureCache texCacheDepth;
      TextureCache texCacheID;
      
      bool doIDTexture;

      PersistentData() : frameNum (0),
        texCacheDepth (csimg2D, "d32", 
          CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP,
          "shadowmap", 
          TextureCache::tcachePowerOfTwo | TextureCache::tcacheExactSizeMatch),
        texCacheID (csimg2D, "argb8", 
          CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NOFILTER,
          "shadowmap", 
          TextureCache::tcachePowerOfTwo | TextureCache::tcacheExactSizeMatch),
        doIDTexture (false)
      {
      }

      ~PersistentData()
      {
      }
      
      void Initialize (iObjectRegistry* objectReg)
      {
        csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objectReg);
        csRef<iShaderManager> shaderManager =
          csQueryRegistry<iShaderManager> (objectReg);
	strings = csQueryRegistryTagInterface<iStringSet> (objectReg,
	  "crystalspace.shared.stringset");
      
        texCacheDepth.SetG3D (g3d);
        texCacheID.SetG3D (g3d);
        this->shaderManager = shaderManager;
        iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	svNames.SetStrings (strings);
	svMeshIDName = strings->Request ("shadowmap mesh id");
      }
      void UpdateNewFrame ()
      {
        frameNum++;
        csTicks time = csGetTicks ();
        texCacheDepth.AdvanceFrame (time);
        texCacheID.AdvanceFrame (time);
      }

      const char* GetShadowShaderType () const
      { return strings->Request (shadowShaderType); }
      void SetShadowShaderType (const char* shaderType)
      { shadowShaderType = strings->Request (shaderType); }
      
      iShader* GetDefaultShadowShader() const { return defaultShadowShader; }
      void SetDefaultShadowShader (iShader* shader)
      { defaultShadowShader = shader; }
    };
    
    typedef ViewSetup ShadowParameters;

    ShadowPSSM (PersistentData& persist,
      const LayerConfigType& layerConfig,
      typename RenderTree::MeshNode* node, 
      ViewSetup& viewSetup) 
      : persist (persist), layerConfig (layerConfig), 
        renderTree (node->owner.owner), meshNode (node),
        viewSetup (viewSetup) { }

    void HandleOneLight (typename RenderTree::MeshNode::SingleMesh& singleMesh,
                         iLight* light, CachedLightData& lightData,
                         csShaderVariableStack& lightStack,
                         uint lightNum)
    {
      lightData.SetupFrame (viewSetup, light);
      LightingVariablesHelper lightVarsHelper (viewSetup.persist.lightVarsPersist);
      
      if (persist.doIDTexture && !singleMesh.svMeshID.IsValid())
      {
        singleMesh.svMeshID = lightVarsHelper.CreateTempSV (
	  viewSetup.persist.svMeshIDName);
        lightStack[singleMesh.svMeshID->GetName()] = singleMesh.svMeshID;
        uint meshID = ++viewSetup.lastMeshID;
        singleMesh.svMeshID->SetValue ((int)meshID);
        viewSetup.meshIDs.Put (singleMesh.meshObjSVs, singleMesh.svMeshID);
      }
      
      const csReversibleTransform& world2light (
        light->GetMovable()->GetFullTransform());
      
      csBox3 meshBboxLight;
      meshBboxLight.StartBoundingBox (world2light.Other2This (
        singleMesh.bbox.GetCorner (0)));
      for (int c = 1; c < 8; c++)
      {
	meshBboxLight.AddBoundingVertex (world2light.Other2This (
	  singleMesh.bbox.GetCorner (c)));
      }
      for (int f = 0; f < NUM_PARTS; f++)
      {
        if (!lightData.lightFrustums[f].volume.TestIntersect (meshBboxLight))
          continue;
      
        lightData.lightFrustums[f].containedObjects.Push (meshBboxLight);
      
	// Add shadow map SVs
	lightVarsHelper.MergeAsArrayItem (lightStack, 
	  lightData.lightFrustums[f].shadowMapSV, lightNum);
	lightVarsHelper.MergeAsArrayItem (lightStack,
	  lightData.lightFrustums[f].shadowMapProjectSV, lightNum);
	lightVarsHelper.MergeAsArrayItem (lightStack, 
	  lightData.lightFrustums[f].shadowMapIDSV, lightNum);
	lightVarsHelper.MergeAsArrayItem (lightStack, 
	  lightData.lightFrustums[f].shadowMapDimSV, lightNum);
	  
	break;
      }
    }
    
    static bool NeedFinalHandleLight() { return true; }
    void FinalHandleLight (iLight* light, CachedLightData& lightData)
    {
      lightData.AddShadowMapTarget (meshNode, persist,
        viewSetup.depthRenderLayer, renderTree, light, lightData, viewSetup);
      
      lightData.ClearFrameData();
    }

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
