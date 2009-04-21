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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_COMMON_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_COMMON_H__

/**\file
 * Shadow handlers common helpers
 */

#include "ivideo/graph3d.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/rendermanager/texturecache.h"
#include "csutil/customallocated.h"
#include "csutil/parray.h"

namespace CS
{
  namespace RenderManager
  {
    class PostEffectManager;
  
    /// Helper to read shadow handler settings
    class CS_CRYSTALSPACE_EXPORT ShadowSettings
    {
    public:
      /// Shadow map target
      struct Target : public CS::Memory::CustomAllocated
      {
        /// Render target attachment for a shadowmap
        csRenderTargetAttachment attachment;
        /// SV name for a shadow map
        CS::ShaderVarStringID svName;
        /// Cache to obtain textures from
        TextureCache texCache;
        
        Target (csRenderTargetAttachment attachment,
          CS::ShaderVarStringID svName,
          const char* format, uint texFlags)
          : attachment (attachment), svName (svName),
	    texCache (csimg2D, format, 
	      CS_TEXTURE_3D | CS_TEXTURE_CLAMP | texFlags,
	      "shadowmap", 
	      TextureCache::tcachePowerOfTwo | TextureCache::tcacheExactSizeMatch)
	  {}
      };
      typedef csPDelArray<Target> TargetArray;
      
      /// Targets fir shadow maps
      TargetArray targets;
      /// Default shader for rendering to shadow map
      csRef<iShader> shadowDefaultShader;
      /// Shader type for rendering to shadow map
      csStringID shadowShaderType;
      /// Whether to provide IDs for each shadowed mesh
      bool provideIDs;
      /// Shader variable taking ID for a mesh
      CS::ShaderVarStringID svMeshIDName;
      
      /// Post processing effects to apply to shadow map
      csRef<PostEffectManager> postEffects;
      
      /**
       * Read settings from configuration (such as targets, default shader etc.).
       * \a shadowType is used as a part of the settings configuration keys
       * (e.g. <tt>RenderManager.Shadows.(type).Shader.Type</tt>). See
       * <tt>data/config-plugins/shadows.cfg</tt> for shadow settings examples.
       */
      void ReadSettings (iObjectRegistry* objReg, const char* shadowType);
      /**
       * Do per-frame house keeping - \b MUST be called every frame/
       * RenderView() execution, typically from the shadow handler's
       * persistent data UpdateNewFrame() method.
       */
      void AdvanceFrame (csTicks time);
    protected:
      bool ReadTargets (TargetArray& targets, iConfigFile* cfg,
        const char* prefixed, iShaderVarStringSet* svStrings, 
        iObjectRegistry* objReg);
    };
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_COMMON_H__
