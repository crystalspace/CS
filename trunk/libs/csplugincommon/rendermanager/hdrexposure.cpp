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
      HDRHelper& hdr)
    {
      this->hdr = &hdr;
      measureLayer = hdr.GetMeasureLayer();
      PostEffectManager::LayerOptions measureOpts = measureLayer->GetOptions();
      measureOpts.mipmap = true;
      measureOpts.maxMipmap = csMax (measureOpts.maxMipmap, 2);
      measureOpts.noTextureReuse = true;
      measureLayer->SetOptions (measureOpts);
      
      csRef<iLoader> loader (csQueryRegistry<iLoader> (objReg));
      CS_ASSERT(loader);
      csRef<iShader> tonemap = loader->LoadShader ("/shader/postproc/hdr/identity-map.xml");
      hdr.SetMappingShader (tonemap);
    
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
    
    void HDRExposureLinear::ApplyExposure ()
    {
      if (!measureLayer || !hdr) return;
      iTextureHandle* measureTex =
	hdr->GetHDRPostEffects().GetLayerOutput (measureLayer);
      int newW, newH;
      measureTex->GetRendererDimensions (newW, newH);
      csRef<iDataBuffer> newData = measureTex->Readback (readbackFmt, 2);
      if (!newData.IsValid())
      {
	// If we can't get the mipmapped version, try to get full version
        newData = measureTex->Readback (readbackFmt, 0);
        if (!newData.IsValid()) return;
      }
      else
      {
        lastW >>= 2; lastH >>= 2;
      }
      
      csTicks currentTime = csGetTicks();
      if (lastData.IsValid() && (lastTime != 0))
      {
        const uint8* bgra = lastData->GetUint8();
        int numPixels = lastW * lastH;
        float totalLum = 0;
        for (int i = 0; i < numPixels; i++)
        {
          int b = *bgra++;
          int g = *bgra++;
          int r = *bgra++;
          bgra++;
          float lum = r*(0.2126f/255) + g*(0.7152f/255) + b*(0.722f/255);
          totalLum += lum;
        }
        
        uint deltaTime = csMin (currentTime-lastTime, (uint)33);
        const float exposureAdjust = exposureChangeRate*deltaTime/1000.0f;
        float avgLum = (totalLum / numPixels) * exposure;
        if (avgLum >= targetAvgLum+targetAvgLumTolerance)
          exposure -= exposureAdjust;
        else if (avgLum <= targetAvgLum-targetAvgLumTolerance)
          exposure += exposureAdjust;
          
        svHDRScale->SetValue (csVector4 (1.0f/exposure, exposure, 0, 0));
      }
      
      lastData = newData;
      lastW = newW; lastH = newH;
      lastTime = currentTime;
    }
  
  } // namespace RenderManager
} // namespace CS
