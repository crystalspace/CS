/*
    Copyright (C) 2009 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_FRAMEBUFFERTEX_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_FRAMEBUFFERTEX_H__

/**\file
 * Automatic framebuffer contents.
 */

//#include "iengine/mesh.h"
#include "csgfx/shadervarblockalloc.h"

#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/texturecache.h"

namespace CS
{
  namespace RenderManager
  {
    namespace AutoFX
    {
      /**
      * Base class for FramebufferTex, containing types and
      * members which are independent of the template arguments that can be
      * provided to FramebufferTex.
      */
      class FramebufferTex_Base
      {
      public:
	/// Flags to pass for GetUsedShaderVars() for this effect
	enum { svUserFlags = iShader::svuTextures };
	
	/**
	* Data used by the helper that needs to persist over multiple frames.
	* Render managers must store an instance of this class and provide
	* it to the helper upon instantiation.
	*/
	class CS_CRYSTALSPACE_EXPORT PersistentData
	{
	  csRef<iTextureHandle> framebufferTex[rtaNumAttachments];
	  
	  TextureCacheT<CS::Utility::ResourceCache::ReuseAlways> texCacheColor;
	  TextureCacheT<CS::Utility::ResourceCache::ReuseAlways> texCacheDepth;
	public:
	  csShaderVarBlockAlloc<csBlockAllocatorDisposeLeaky<csShaderVariable> >
	      svAlloc;
	  /* A number of SVs have to be kept alive even past the expiration
	  * of the actual step */
	  csRefArray<csShaderVariable> svKeeper;
	  
	  CS::ShaderVarStringID svTexFramebuffer[rtaNumAttachments];
	  CS::ShaderVarStringID svFramebufferCoordXform;
	    
	  /// Construct helper
	  PersistentData();
	  
	  /**
	  * Initialize helper. Fetches various required values from objects in
	  * the object registry and reads configuration settings. Must be called
	  * when the RenderManager plugin is initialized.
	  * \a postEffects must be the post effects manager used by the render
	  * manager, if any, or 0.
	  */
	  void Initialize (iObjectRegistry* objReg,
			  PostEffectManager* postEffects);
	  
	  iTextureHandle* GetFramebufferTex (size_t n, int width, int height);
	
	  /**
	  * Do per-frame house keeping - \b MUST be called every frame/
	  * RenderView() execution.
	  */
	  void UpdateNewFrame ();
	};
      
	FramebufferTex_Base (PersistentData& persist) : persist (persist)
	{}
      protected:
	PersistentData& persist;
      };
      
      /**
      * Render manager helper for automatic framebuffer content
      * textures.
      *
      * When some shader used in a render tree context uses a framebuffer
      * content texture (SV name <tt>tex framebuffer color</tt> or
      * <tt>tex framebuffer depth</tt>) the contents of the framebuffer
      * before the mesh using the shader is rendered are copied to the textures.
      *
      * Usage: Functor for TraverseUsedSVSets. Application must happen after 
      * shader and ticket setup (e.g. SetupStandardTicket()).
      * Example:
      * \code
      * // Define type using rendermanager-dependent render tree and context setup
      * typedef CS::RenderManager::AutoFX::FramebufferTex<RenderTreeType, 
      *   ContextSetupType> AutoFramebufferTexType;
      *
      * // Instantiate helper in rendering
      * RenderManagerType::AutoFramebufferTexType fxFB (
      *   rmanager->framebufferTexPersistent);
      * // Set up a traverser for the sets of shader vars used over each mesh
      * typedef TraverseUsedSVSets<RenderTreeType,
      *   RenderManagerType::AutoFramebufferTexType> SVTraverseType;
      * SVTraverseType svTraverser
      *   (fxFB, shaderManager->GetSVNameStringset ()->GetSize (), fxFB.svUserFlags);
      * // Do the actual traversal.
      * ForEachMeshNode (context, svTraverser);
      * \endcode
      *
      * The template parameter \a RenderTree gives the render tree type.
      */
      template<typename RenderTree>
      class FramebufferTex : public FramebufferTex_Base
      {
      public:
	FramebufferTex (PersistentData& persist) : 
	  FramebufferTex_Base (persist)
	{}
      
	/**
	* Operator doing the actual work. If one of the reflection or refraction
	* textures is detected in the set of shader vars used (\a names) by
	* \a mesh textures are set up.
	*/
	void operator() (typename RenderTree::MeshNode* node,
			size_t layer,
			typename RenderTree::MeshNode::SingleMesh& mesh,
			const csBitArray& names)
	{
	  size_t n = 0;
	  int renderW = -1, renderH;
	  
	  typename RenderTree::ContextNode& context = node->GetOwner();
	  csShaderVariableStack localStack;
	  context.svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);
	  
	  csRef<csShaderVariable> svFBCoordXform;
	  for (size_t i = 0; i < rtaNumAttachments; i++)
	  {
	    CS::ShaderVarStringID svName = persist.svTexFramebuffer[i];
	    if (names.IsBitSetTolerant (svName))
	    {
	      if (renderW < 0)
	      {
		renderW = renderH = 512;
		context.GetTargetDimensions (renderW, renderH);
	      }
	      mesh.preCopyAttachments[n] = (csRenderTargetAttachment)i;
	      iTextureHandle* tex = persist.GetFramebufferTex (i, renderW, renderH);
	      mesh.preCopyTextures[n] = tex;
	      
	      if (!svFBCoordXform.IsValid())
	      {
		svFBCoordXform.AttachNew (new csShaderVariable (
		  persist.svFramebufferCoordXform));
		  
		if (tex->GetTextureType() == iTextureHandle::texTypeRect)
		{
		  svFBCoordXform->SetValue (
		    csVector4 (0.5f*renderW, 0.5f*renderH,
			      0.5f*renderW, 0.5f*renderH));
		}
		else
		{
		  int txt_w, txt_h;
		  tex->GetRendererDimensions (txt_w, txt_h);
		  svFBCoordXform->SetValue (
		    csVector4 (0.5f*renderW/float (txt_w), 0.5f*renderH/float (txt_h), 
			      0.5f, 0.5f));
		}
	      
		persist.svKeeper.Push (svFBCoordXform);
		localStack[persist.svFramebufferCoordXform] = svFBCoordXform;
	      }
	      
	      csRef<csShaderVariable> svFramebufferTex;
	      svFramebufferTex.AttachNew (new csShaderVariable (svName));
	      persist.svKeeper.Push (svFramebufferTex);
	      svFramebufferTex->SetValue (tex);
	      localStack[svName] = svFramebufferTex;
	      
	      n++;
	    }
	  }
	  mesh.preCopyNum = n;
	}
      };

    } // namespace AutoFX
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_FRAMEBUFFERTEX_H__
