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

#include "cssysdef.h"

#include "csplugincommon/rendermanager/hdrhelper.h"

#include "iutil/cfgfile.h"

namespace CS
{
  namespace RenderManager
  {
    bool HDRHelper::Setup (iObjectRegistry* objectReg, 
      Quality quality, int colorRange)
    {
      postEffects.Initialize (objectReg);

      const char* textureFmt;
      switch (quality)
      {
	/* @@@ QUESTION: With or without alpha? Some shader may want destination
	  * alpha. But without is prolly faster, and post proc shaders are less
	  * likely to need it. So perhaps allow different formats in one post
	  * effect manager ... */
	// Should work everywhere
	case qualInt8:    textureFmt = "argb8"; break;
	/* NV G80: works. (Note: 'Pure' rgb10 does *not* work well)
	   ATI: presumably also, minimum HW unknown.
	   Others: unknown. Probably not.
	   */
	case qualInt10:   textureFmt = "a2rgb10"; break;
	/* NV G80: Does not work. (Variations in component order or adding
	           alpha does not improve things. 
	   ATI: posibbly. Must be tested.
	   Others: unknown.*/
	case qualInt16:   textureFmt = "rgb16"; break;
	case qualFloat16: textureFmt = "bgr16_f"; break;
	case qualFloat32: textureFmt = "bgr32_f"; break;
	default: return false;
      }
      postEffects.SetIntermediateTargetFormat (textureFmt);
      this->quality = quality;
      
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
      if ((quality == qualInt8) || (quality == qualInt10)
          || (quality == qualInt16))
	svHdrScale->SetValue (csVector4 (colorRange, 1.0f/colorRange, 0, 0));
      else
	svHdrScale->SetValue (csVector4 (1, 1, 0, 0));
	
      csRef<iLoader> loader (csQueryRegistry<iLoader> (objectReg));
      if (!loader) return false;
      csRef<iShader> map = loader->LoadShader ("/shader/postproc/hdr/default-map.xml");
      if (!map) return false;
      measureLayer = postEffects.GetLastLayer();
      mappingLayer = postEffects.AddLayer (map);
    
      return true;
    }

    void HDRHelper::SetMappingShader (iShader* shader)
    {
      mappingLayer->SetShader (shader);
    }

    iShader* HDRHelper::GetMappingShader ()
    {
      return mappingLayer->GetShader();
    }

    iShaderVariableContext* HDRHelper::GetMappingShaderVarContext()
    {
      return mappingLayer->GetSVContext();
    }

    //-----------------------------------------------------------------------

    HDRSettings::HDRSettings (iConfigFile* config, const char* prefix)
      : config (config), prefix (prefix) {}
    
    bool HDRSettings::IsEnabled()
    {
      return config->GetBool (csString ().Format ("%s.HDR.Enabled",
        prefix.GetData()), false);
    }
    
    HDRHelper::Quality HDRSettings::GetQuality()
    {
      HDRHelper::Quality qual = HDRHelper::qualInt10;
      
      const char* qualStr = config->GetStr (
        csString ().Format ("%s.HDR.Quality", prefix.GetData()), 0);
      if (qualStr)
      {
        if (strcmp (qualStr, "int8") == 0)
          qual = HDRHelper::qualInt8;
        else if (strcmp (qualStr, "int10") == 0)
          qual = HDRHelper::qualInt10;
        else if (strcmp (qualStr, "int16") == 0)
          qual = HDRHelper::qualInt16;
        else if (strcmp (qualStr, "float16") == 0)
          qual = HDRHelper::qualFloat16;
        else if (strcmp (qualStr, "float32") == 0)
          qual = HDRHelper::qualFloat32;
      }
      
      return qual;
    }
    
    int HDRSettings::GetColorRange()
    {
      return config->GetInt (csString ().Format ("%s.HDR.ColorRange",
        prefix.GetData()), 4);
    }
    
    const char* HDRSettings::GetExposureMethod() const
    {
      return config->GetStr (
        csString ().Format ("%s.HDR.Exposure", prefix.GetData()), 0);
    }
  } // namespace RenderManager
} // namespace CS

