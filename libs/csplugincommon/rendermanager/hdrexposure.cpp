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

#include "csplugincommon/rendermanager/hdrexposure.h"

#include "csplugincommon/rendermanager/posteffects.h"
#include "csutil/objreg.h"
#include "csutil/sysfunc.h"
#include "imap/loader.h"
#include "iutil/databuff.h"

namespace CS
{
  namespace RenderManager
  {
    void HDRExposureLinear::Initialize (iObjectRegistry* objReg,
      PostEffectManager& postEffects)
    {
      PostEffectManager::LayerOptions screenOpts =
        postEffects.GetScreenLayer()->GetOptions();
      screenOpts.mipmap = true;
      screenOpts.maxMipmap = csMax (screenOpts.maxMipmap, 2);
      screenOpts.noTextureReuse = true;
      postEffects.GetScreenLayer()->SetOptions (screenOpts);
      
      csRef<iLoader> loader (csQueryRegistry<iLoader> (objReg));
      CS_ASSERT(loader);
      csRef<iShader> tonemap =
	loader->LoadShader ("/shader/postproc/hdr/identity-map.xml");
      postEffects.AddLayer (tonemap);
    
      csRef<iShaderManager> shaderManager =
	csQueryRegistry<iShaderManager> (objReg);
      CS_ASSERT (shaderManager);
      csRef<iShaderVarStringSet> svNameStringSet = 
	csQueryRegistryTagInterface<iShaderVarStringSet> (objReg,
	  "crystalspace.shader.variablenameset");
      CS_ASSERT (svNameStringSet);
	  
      svHDRScale = shaderManager->GetVariableAdd (svNameStringSet->Request (
	"hdr scale"));
      svHDRScale->SetValue (csVector4 (1.0f/exposure, exposure, 0, 0));
    }
    
    void HDRExposureLinear::ApplyExposure (PostEffectManager& postEffects)
    {
      iTextureHandle* screenTex = postEffects.GetScreenTarget();
      csRef<iDataBuffer> newData = screenTex->Readback (readbackFmt, 2);
      int newW, newH;
      screenTex->GetRendererDimensions (newW, newH);
      
      csTicks currentTime = csGetTicks();
      if (lastData.IsValid() && (lastTime != 0))
      {
        const uint8* rgba = lastData->GetUint8();
        int numPixels = lastW * lastH;
        float totalLum = 0;
        for (int i = 0; i < numPixels; i++)
        {
          int r = *rgba++;
          int g = *rgba++;
          int b = *rgba++;
          /*int a = **/rgba++;
          float lum = r*(0.2126f/255) + g*(0.7152f/255) + b*(0.722f/255);
          totalLum += lum;
        }
        
        const float exposureAdjust = csMin (0.5f*(currentTime-lastTime)/1000.0f, 0.2f);
        const float targetLum = 0.8f;
        float avgLum = totalLum / numPixels;
        if (avgLum >= targetLum+0.1f)
          exposure -= exposureAdjust;
        else if (avgLum <= targetLum-0.1f)
          exposure += exposureAdjust;
          
        svHDRScale->SetValue (csVector4 (1.0f/exposure, exposure, 0, 0));
      }
      
      lastData = newData;
      lastW = newW >> 2; lastH = newH >> 2;
      lastTime = currentTime;
    }
  
  } // namespace RenderManager
} // namespace CS
