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
  
    class CS_CRYSTALSPACE_EXPORT ShadowSettings
    {
    public:
      struct Target : public CS::Memory::CustomAllocated
      {
        csRenderTargetAttachment attachment;
        CS::ShaderVarStringID svName;
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
      
      TargetArray targets;
      csRef<iShader> shadowDefaultShader;
      csStringID shadowShaderType;
      // Support for ID shadows
      bool provideIDs;
      CS::ShaderVarStringID svMeshIDName;
      
      csRef<PostEffectManager> postEffects;
      
      void ReadSettings (iObjectRegistry* objReg, const char* shadowType);
      void AdvanceFrame (csTicks time);
    protected:
      bool ReadTargets (TargetArray& targets, iConfigFile* cfg,
        const char* prefixed, iShaderVarStringSet* svStrings, 
        iObjectRegistry* objReg);
    };
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_SHADOW_COMMON_H__
