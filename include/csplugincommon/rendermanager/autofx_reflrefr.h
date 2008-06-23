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
        
        //CS::ShaderVarStringID svClipPlaneReflRefr;
      
        TextureCache texCache;
        struct ReflectRefractSVs
        {
          csRef<csShaderVariable> reflectSV;
          csRef<csShaderVariable> refractSV;
        };
        csHash<ReflectRefractSVs, csPtrKey<iMeshWrapper> >
          reflRefrCache;
        
        PersistentData() :
	  texCache (csimg2D, "rgb8",  // @@@ FIXME: Use same format as main view ...
	    CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	     | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP,
	    "target", 0)
	{
	}
        
	void Initialize (iObjectRegistry* objReg,
			RenderTreeBase::DebugPersistent& dbgPersist)
	{
	  csRef<iShaderManager> shaderManager =
	    csQueryRegistry<iShaderManager> (objReg);
	  
	  iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	  svTexPlaneRefl = strings->Request ("tex plane reflect");
	  svTexPlaneRefr = strings->Request ("tex plane refract");
	  
	  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objReg);
	  texCache.SetG3D (g3d);
	}
      
	void UpdateNewFrame ()
	{
	  reflRefrCache.Empty();
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

        // Compute reflect/refract plane
        // @@ FIXME: Obviously use a plane from the object
        csPlane3 reflRefrPlane (csVector3 (0, -1, 0), -14.2);
        
        if (names.IsBitSetTolerant (persist.svTexPlaneRefl))
        {
	  csRef<csShaderVariable> svReflection;
	  
	  if (meshReflectRefract.reflectSV.IsValid())
	  {
	    svReflection = meshReflectRefract.reflectSV;
	  }
	  else
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
	    csRef<iTextureHandle> tex = 
	      persist.texCache.QueryUnusedTexture (txt_w, txt_h, 0);
	    
	    // Set up context for reflection, clipped to plane
	    csBox2 clipBox (0, 0, txt_w, txt_h);
	    csRef<iClipper2D> newView;
	    newView.AttachNew (new csBoxClipper (clipBox));
	    reflView->SetClipper (newView);
  
	    typename RenderTree::ContextNode* reflCtx = 
	      renderTree.CreateContext (reflView);
	    reflCtx->renderTargets[rtaColor0].texHandle = tex;
	    reflCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
	      
	    // Attach reflection texture to mesh
	    svReflection.AttachNew (new csShaderVariable (
	      persist.svTexPlaneRefl));
	    svReflection->SetValue (tex);
	    meshReflectRefract.reflectSV = svReflection;
	    
	    renderTree.AddDebugTexture (tex);
    
	    // Setup the new context
	    contextFunction (*reflCtx);
	  }
	  
	  csShaderVariableStack localStack;
	  context.svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);
	  localStack[persist.svTexPlaneRefl] = svReflection;
	}
        
        if (names.IsBitSetTolerant (persist.svTexPlaneRefr))
        {
	  csRef<csShaderVariable> svRefraction;
	  
	  if (meshReflectRefract.refractSV.IsValid())
	  {
	    svRefraction = meshReflectRefract.refractSV;
	  }
	  else
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
	    csRef<iTextureHandle> tex = 
	      persist.texCache.QueryUnusedTexture (txt_w, txt_h, 0);
	    
	    // Set up context for reflection, clipped to plane
	    csBox2 clipBox (0, 0, txt_w, txt_h);
	    csRef<iClipper2D> newView;
	    newView.AttachNew (new csBoxClipper (clipBox));
	    refrView->SetClipper (newView);
  
	    typename RenderTree::ContextNode* refrCtx = 
	      renderTree.CreateContext (refrView);
	    refrCtx->renderTargets[rtaColor0].texHandle = tex;
	      
	    // Attach reflection texture to mesh
	    svRefraction.AttachNew (new csShaderVariable (
	      persist.svTexPlaneRefr));
	    svRefraction->SetValue (tex);
	    meshReflectRefract.refractSV = svRefraction;
	    
	    renderTree.AddDebugTexture (tex);
    
	    // Attach refraction texture to mesh
	    contextFunction (*refrCtx);
	  }
	  
	  csShaderVariableStack localStack;
	  context.svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);
	  localStack[persist.svTexPlaneRefr] = svRefraction;
	}
      }
    protected:
      PersistentData& persist;
      ContextSetup& contextFunction;
    };

  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_REFLREFR_H__
