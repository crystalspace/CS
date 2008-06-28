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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_REFLREFR_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_REFLREFR_H__

#include "csplugincommon/rendermanager/posteffects.h"

namespace CS
{
  namespace RenderManager
  {

    /**
     * Class for automatic plane reflection/refraction textures.
     *
     * Usage: Functor for TraverseUsedSVSets
     */
    template<typename RenderTree, typename ContextSetup>
    class AutoFX_ReflectRefract
    {
    public:
      struct PersistentData
      {
        CS::ShaderVarStringID svTexPlaneRefl;
        CS::ShaderVarStringID svTexPlaneRefr;
        CS::ShaderVarStringID svTexPlaneReflDepth;
        CS::ShaderVarStringID svTexPlaneRefrDepth;
        
        CS::ShaderVarStringID svPlaneRefl;
        CS::ShaderVarStringID svClipPlaneReflRefr;
        
        CS::ShaderVarStringID svReflXform;
        csRef<csShaderVariable> reflXformSV;
      
        TextureCache texCache;
        TextureCache texCacheDepth;
        struct ReflectRefractSVs
        {
          csRef<csShaderVariable> reflectSV;
          csRef<csShaderVariable> refractSV;
          csRef<csShaderVariable> reflectDepthSV;
          csRef<csShaderVariable> refractDepthSV;
          
          csRef<iShaderVariableContext> clipPlaneReflContext;
          csRef<csShaderVariable> clipPlaneReflSV;
          csRef<iShaderVariableContext> clipPlaneRefrContext;
          csRef<csShaderVariable> clipPlaneRefrSV;
        };
        csHash<ReflectRefractSVs, csPtrKey<iMeshWrapper> >
          reflRefrCache;
        
        PersistentData() :
	  texCache (csimg2D, "rgb8",  // @@@ FIXME: Use same format as main view ...
	    CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	     | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP,
	    "target", 0),
	  texCacheDepth (csimg2D, "d32",
	    CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	     | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP,
	    "target", 0)
	{
	}
        
	void Initialize (iObjectRegistry* objReg,
			 RenderTreeBase::DebugPersistent& dbgPersist,
			 PostEffectManager* postEffects)
	{
	  csRef<iShaderManager> shaderManager =
	    csQueryRegistry<iShaderManager> (objReg);
	  
	  iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	  svTexPlaneRefl = strings->Request ("tex plane reflect");
	  svTexPlaneRefr = strings->Request ("tex plane refract");
	  svTexPlaneReflDepth = strings->Request ("tex plane reflect depth");
	  svTexPlaneRefrDepth = strings->Request ("tex plane refract depth");
	  
	  svPlaneRefl = strings->Request ("plane reflection");
	  svClipPlaneReflRefr = strings->Request ("clip plane reflection");
	  
	  svReflXform = strings->Request ("reflection coord xform");
	  reflXformSV.AttachNew (new csShaderVariable (svReflXform));
	  bool screenFlipped = postEffects ? postEffects->ScreenSpaceYFlipped() : false;
	  reflXformSV->SetValue (csVector4 (0.5f, 
	    screenFlipped ? 0.5f : -0.5f, 0.5f, 0.5f));
	  
	  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objReg);
	  texCache.SetG3D (g3d);
	  texCacheDepth.SetG3D (g3d);
	}
      
	void UpdateNewFrame ()
	{
	  reflRefrCache.Empty();
	  texCache.AdvanceFrame (csGetTicks ());
	  texCacheDepth.AdvanceFrame (csGetTicks ());
	}
      };
      
      AutoFX_ReflectRefract (PersistentData& persist,
        ContextSetup& contextFunction) : persist (persist),
        contextFunction (contextFunction)
      {}
    
      void operator() (typename RenderTree::MeshNode* node,
                       size_t layer,
                       typename RenderTree::MeshNode::SingleMesh& mesh,
                       const csBitArray& names)
      {
        // Check if reflection or refraction is needed
        if (!names.IsBitSetTolerant (persist.svTexPlaneRefl)
            && !names.IsBitSetTolerant (persist.svTexPlaneRefr))
          return;
        
        /* @@@ NOTE:The same mesh, if rendered in multiple views, will get
           the same reflection/refraction texture. This is wrong.
           However, ignore for now, until someone complains */
        typename PersistentData::ReflectRefractSVs& meshReflectRefract =
          persist.reflRefrCache.GetOrCreate (mesh.meshWrapper);
        
	typename RenderTree::ContextNode& context = node->owner;
	RenderTree& renderTree = context.owner;
        RenderView* rview = context.renderView;
        
	csShaderVariableStack localStack;
	context.svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);

        bool usesReflTex = names.IsBitSetTolerant (persist.svTexPlaneRefl);
        bool usesReflDepthTex = names.IsBitSetTolerant (persist.svTexPlaneReflDepth);
        bool needReflTex = (usesReflTex && !meshReflectRefract.reflectSV.IsValid())
          || (usesReflDepthTex && !meshReflectRefract.reflectDepthSV.IsValid());
          
        bool usesRefrTex = names.IsBitSetTolerant (persist.svTexPlaneRefr);
        bool usesRefrDepthTex = names.IsBitSetTolerant (persist.svTexPlaneRefrDepth);
        bool needRefrTex = usesRefrTex && !meshReflectRefract.refractSV.IsValid()
          || (usesRefrDepthTex && !meshReflectRefract.refractDepthSV.IsValid());
                
        // Compute reflect/refract plane
        csPlane3 reflRefrPlane;
        if (needReflTex || needRefrTex)
        {
          if ((localStack.GetSize() > persist.svPlaneRefl)
              && (localStack[persist.svPlaneRefl] != 0))
          {
            // Grab reflection plane from a SV
            csShaderVariable* planeSV = localStack[persist.svPlaneRefl];
            csVector4 v;
            planeSV->GetValue (v);
            reflRefrPlane.Set (v.x, v.y, v.z, v.w);
          }
          else
          {
	    /* Guess reflection plane from mesh bbox:
	      Take smallest dimension of object space bounding box, make that
	      the durection of reflect plane */
	    const csBox3& objBB =
	      mesh.meshWrapper->GetMeshObject()->GetObjectModel()->GetObjectBoundingBox();
	    const csVector3& bbSize = objBB.GetSize();
	    
	    int axis;
	    if ((bbSize[0] < bbSize[1]) && (bbSize[0] < bbSize[2]))
	    {
	      axis = 0;
	    }
	    else if (bbSize[1] < bbSize[2])
	    {
	      axis = 1;
	    }
	    else
	    {
	      axis = 2;
	    }
	    
	    csVector3 planeNorm (0);
	    planeNorm[axis] = -1;
            reflRefrPlane.Set (planeNorm, 0);
            reflRefrPlane.SetOrigin (objBB.GetCenter());
	  }
          
          reflRefrPlane =
            mesh.meshWrapper->GetMovable()->GetFullTransform().This2Other (reflRefrPlane);
            
	  meshReflectRefract.clipPlaneReflSV.AttachNew (new csShaderVariable (
	    persist.svClipPlaneReflRefr));
	  meshReflectRefract.clipPlaneReflSV->SetValue (csVector4 (
	    -reflRefrPlane.A(),
	    -reflRefrPlane.B(),
	    -reflRefrPlane.C(),
	    -reflRefrPlane.D()));
	  meshReflectRefract.clipPlaneReflContext.AttachNew (
	    new csShaderVariableContext);
	  meshReflectRefract.clipPlaneReflContext->AddVariable (
	    meshReflectRefract.clipPlaneReflSV);
            
	  meshReflectRefract.clipPlaneRefrSV.AttachNew (new csShaderVariable (
	    persist.svClipPlaneReflRefr));
	  meshReflectRefract.clipPlaneRefrSV->SetValue (csVector4 (
	    reflRefrPlane.A(),
	    reflRefrPlane.B(),
	    reflRefrPlane.C(),
	    reflRefrPlane.D()));
	  meshReflectRefract.clipPlaneRefrContext.AttachNew (
	    new csShaderVariableContext);
	  meshReflectRefract.clipPlaneRefrContext->AddVariable (
	    meshReflectRefract.clipPlaneRefrSV);
        }
        
        typename RenderTree::ContextNode* reflCtx = 0;
        typename RenderTree::ContextNode* refrCtx = 0;
        
        if (usesReflTex || usesReflDepthTex)
        {
	  csRef<csShaderVariable> svReflection;
	  csRef<csShaderVariable> svReflectionDepth;
	  
	  if (needReflTex)
	  {
	    // Compute reflection view
	    iCamera* cam = rview->GetCamera();
	    // Create a new view
	    csRef<CS::RenderManager::RenderView> reflView;
	    csRef<iCamera> newCam (cam->Clone());
	    iCamera* inewcam = newCam;
    #include "csutil/custom_new_disable.h"
	    reflView.AttachNew (
	      new (renderTree.GetPersistentData().renderViewPool) RenderView (
	        *rview));
    #include "csutil/custom_new_enable.h"
            reflView->SetCamera (inewcam);
            reflView->GetMeshFilter().AddFilterMesh (mesh.meshWrapper);
	    
	    // Change the camera transform to be a reflection across reflRefrPlane
	    csPlane3 reflRefrPlane_cam = 
	      inewcam->GetTransform().Other2This (reflRefrPlane);
	    csReversibleTransform reflection (csTransform::GetReflect (reflRefrPlane));
	    csTransform reflection_cam (csTransform::GetReflect (reflRefrPlane_cam));
	    const csTransform& world2cam (inewcam->GetTransform());
	    csTransform reflected;
	    reflected.SetO2T (reflection_cam.GetO2T() * world2cam.GetO2T());
	    reflected.SetOrigin (reflection.Other2This (world2cam.GetOrigin()));
	    inewcam->SetTransform (reflected);
	    inewcam->SetMirrored (true);
	    
	    int txt_w = 512, txt_h = 512;
	    csRef<iTextureHandle> tex;
	    csRef<iTextureHandle> texDepth;
	    if (usesReflTex)
	    {
	      tex = 
	        persist.texCache.QueryUnusedTexture (txt_w, txt_h, 0);
	    }
	    if (usesReflDepthTex)
	    {
	      texDepth = 
	        persist.texCacheDepth.QueryUnusedTexture (txt_w, txt_h, 0);
	    }
	    
	    // Set up context for reflection, clipped to plane
	    csBox2 clipBox (0, 0, txt_w, txt_h);
	    csRef<iClipper2D> newView;
	    newView.AttachNew (new csBoxClipper (clipBox));
	    reflView->SetClipper (newView);
  
	    reflCtx = renderTree.CreateContext (reflView);
	    reflCtx->renderTargets[rtaColor0].texHandle = tex;
	    reflCtx->renderTargets[rtaDepth].texHandle = texDepth;
	    reflCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
	    reflCtx->shadervars = meshReflectRefract.clipPlaneReflContext;
	      
	    svReflection.AttachNew (new csShaderVariable (
	      persist.svTexPlaneRefl));
	    svReflection->SetValue (tex);
	    meshReflectRefract.reflectSV = svReflection;
	    
	    svReflectionDepth.AttachNew (new csShaderVariable (
	      persist.svTexPlaneReflDepth));
	    svReflectionDepth->SetValue (texDepth);
	    meshReflectRefract.reflectDepthSV = svReflectionDepth;
	    
	    renderTree.AddDebugTexture (tex);
	    renderTree.AddDebugTexture (texDepth);
	  }
	  else
	  {
	    svReflection = meshReflectRefract.reflectSV;
	    svReflectionDepth = meshReflectRefract.reflectDepthSV;
	  }
	  
	  // Attach reflection texture to mesh
	  localStack[persist.svTexPlaneRefl] = svReflection;
	  localStack[persist.svTexPlaneReflDepth] = svReflectionDepth;
	  localStack[persist.svReflXform] = persist.reflXformSV;
	}
        
        if (usesRefrTex || usesRefrDepthTex)
        {
	  csRef<csShaderVariable> svRefraction;
	  csRef<csShaderVariable> svRefractionDepth;
	  
	  if (needRefrTex)
	  {
	    // Set up context for refraction, clipped to plane
	    
	    // Create a new view
	    csRef<CS::RenderManager::RenderView> refrView;
    #include "csutil/custom_new_disable.h"
	    refrView.AttachNew (
	      new (renderTree.GetPersistentData().renderViewPool) RenderView (
		*rview));
    #include "csutil/custom_new_enable.h"
	    
	    int txt_w = 512, txt_h = 512;
	    csRef<iTextureHandle> tex;
	    csRef<iTextureHandle> texDepth;
	    if (usesRefrTex)
	    {
	      tex = 
	        persist.texCache.QueryUnusedTexture (txt_w, txt_h, 0);
	    }
	    if (usesRefrDepthTex)
	    {
	      texDepth = 
	        persist.texCacheDepth.QueryUnusedTexture (txt_w, txt_h, 0);
	    }
	    
	    // Set up context for reflection, clipped to plane
	    csBox2 clipBox (0, 0, txt_w, txt_h);
	    csRef<iClipper2D> newView;
	    newView.AttachNew (new csBoxClipper (clipBox));
	    refrView->SetClipper (newView);
            refrView->GetMeshFilter().AddFilterMesh (mesh.meshWrapper);
  
	    refrCtx = renderTree.CreateContext (refrView);
	    refrCtx->renderTargets[rtaColor0].texHandle = tex;
	    refrCtx->renderTargets[rtaDepth].texHandle = texDepth;
	    refrCtx->shadervars = meshReflectRefract.clipPlaneRefrContext;
	      
	    // Attach reflection texture to mesh
	    svRefraction.AttachNew (new csShaderVariable (
	      persist.svTexPlaneRefr));
	    svRefraction->SetValue (tex);
	    meshReflectRefract.refractSV = svRefraction;
	    
	    svRefractionDepth.AttachNew (new csShaderVariable (
	      persist.svTexPlaneRefrDepth));
	    svRefractionDepth->SetValue (texDepth);
	    meshReflectRefract.refractDepthSV = svRefractionDepth;
	    
	    renderTree.AddDebugTexture (tex);
	    renderTree.AddDebugTexture (texDepth);
	  }
  	  else
	  {
	    svRefraction = meshReflectRefract.refractSV;
	    svRefractionDepth = meshReflectRefract.refractDepthSV;
	  }
	  
          // Attach refraction texture to mesh
	  localStack[persist.svTexPlaneRefr] = svRefraction;
	  localStack[persist.svTexPlaneRefrDepth] = svRefractionDepth;
	  localStack[persist.svReflXform] = persist.reflXformSV;
	}
        // Setup the new contexts
	if (reflCtx) contextFunction (*reflCtx);
	if (refrCtx) contextFunction (*refrCtx);
      }
    protected:
      PersistentData& persist;
      ContextSetup& contextFunction;
    };

  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_REFLREFR_H__
