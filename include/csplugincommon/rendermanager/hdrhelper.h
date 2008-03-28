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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_HDRHELPER_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_HDRHELPER_H__

#include "csplugincommon/rendermanager/posteffects.h"

namespace CS
{
  namespace RenderManager
  {
    class HDRHelper
    {
    public:
      /// Level of HDR quality
      enum Quality
      {
        /**
         * Use 8-bit integers: fastest, but looks total crap. Use only
         * when desperate
         */
        qualInt8,
        /**
         * Use 16-bit integers: fast and usually good enough;
         * range is still limited. Recommended.
         */
        qualInt16,
        /**
         * Use 16-bit floats: slower than integers, but wider range.
         * Use if you should run into color precision issues with integer.
         */
        qualFloat16,
        /**
         * Use 32-bit floats: slowest, but also highest range and precision.
         * However, rarely needed.
         */
        qualFloat32
      };
    
      bool Setup (iObjectRegistry* objectReg, 
        Quality quality, int colorRange,
        PostEffectManager& postEffects, bool addDefaultMappingShader = true)
      {
        const char* textureFmt;
        switch (quality)
        {
          /* @@@ QUESTION: With or without alpha? Some shader may want destination
           * alpha. But without is prolly faster, and post proc shaders are less
           * likely to need it. So perhaps allow different formats in one post
           * effect manager ... */
          case qualInt8:    textureFmt = "rgb8"; break;
          case qualInt16:   textureFmt = "rgb16"; break;
          case qualFloat16: textureFmt = "rgb16_f"; break;
          case qualFloat32: textureFmt = "rgb32_f"; break;
          default: return false;
        }
        postEffects.SetIntermediateTargetFormat (textureFmt);
        
	csRef<iShaderManager> shaderManager =
	  csQueryRegistry<iShaderManager> (objectReg);
	if (!shaderManager) return false;
	csRef<iShaderVarStringSet> svNameStringSet = 
	  csQueryRegistryTagInterface<iShaderVarStringSet> (objectReg,
	    "crystalspace.shader.variablenameset");
	if (!svNameStringSet) return false;
	    
	csShaderVariable* svHdrScale =
	  shaderManager->GetVariableAdd (svNameStringSet->Request (
	    "hdr scale"));
        if ((quality == qualInt8) || (quality == qualInt16))
          svHdrScale->SetValue (csVector2 (colorRange, 1.0f/colorRange));
        else
          svHdrScale->SetValue (csVector2 (1, 1));
          
        if (addDefaultMappingShader)
        {
          csRef<iLoader> loader (csQueryRegistry<iLoader> (objectReg));
          if (!loader) return false;
	  csRef<iShader> map =
	    loader->LoadShader ("/shader/postproc/hdr/default-map.xml");
          if (!map) return false;
	  postEffects.AddLayer (map);
        }
        return true;
      }
    private:
    };
  
  } // namespace RenderManager
} // namespace CS

#endif
