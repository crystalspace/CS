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

#include "iutil/verbositymanager.h"
#include "ivaria/reporter.h"
#include "csutil/stringquote.h"

namespace CS
{
  namespace RenderManager
  {
    namespace HDR
    {
      namespace Exposure
      {
	void Linear::Initialize (iObjectRegistry* objReg, HDRHelper& hdr)
	{
	  luminance.Initialize (objReg, hdr);
	  this->hdr = &hdr;
	  
	  csRef<iLoader> loader (csQueryRegistry<iLoader> (objReg));
	  CS_ASSERT(loader);
	  csRef<iShaderVarStringSet> svNameStringSet = 
	    csQueryRegistryTagInterface<iShaderVarStringSet> (objReg,
	      "crystalspace.shader.variablenameset");
	  CS_ASSERT (svNameStringSet);
	  
	  csRef<iShaderManager> shaderManager = csQueryRegistry<iShaderManager> (objReg);
	  CS_ASSERT (shaderManager);
	      
	  csRef<iShader> tonemap = loader->LoadShader ("/shader/postproc/hdr/identity-map.xml");
	  hdr.SetMappingShader (tonemap);
	
	  svHDRScale = shaderManager->GetVariableAdd (svNameStringSet->Request (
	    "hdr scale"));
	  float exposure = luminance.GetColorScale();
	  svHDRScale->SetValue (csVector4 (1.0f/exposure, exposure, 0, 0));
	}
	
	void Linear::ApplyExposure (RenderTreeBase& renderTree, iView* view)
	{
	  if (!hdr) return;
	  
	  csTicks currentTime = csGetTicks();
	  float avgLum, maxLum, exposure;
	  if (luminance.ComputeLuminance (renderTree, view,
	      avgLum, maxLum, exposure) && (lastTime != 0))
	  {
	    uint deltaTime = csMin (currentTime-lastTime, (uint)33);
	    const float exposureAdjust = exposureChangeRate*deltaTime/1000.0f;
	    if (avgLum >= targetAvgLum+targetAvgLumTolerance)
	      exposure -= exposureAdjust;
	    else if (avgLum <= targetAvgLum-targetAvgLumTolerance)
	      exposure += exposureAdjust;
	    luminance.SetColorScale (exposure);
	      
	    svHDRScale->SetValue (csVector4 (1.0f/exposure, exposure, 0, 0));
	  }
	  
	  lastTime = currentTime;
	}
  
        //-------------------------------------------------------------------
        
	void Reinhard_Simple::Initialize (iObjectRegistry* objReg, HDRHelper& hdr)
	{
	  luminance.Initialize (objReg, hdr);
	  this->hdr = &hdr;
	  
	  csRef<iLoader> loader (csQueryRegistry<iLoader> (objReg));
	  CS_ASSERT(loader);
	  csRef<iShaderVarStringSet> svNameStringSet = 
	    csQueryRegistryTagInterface<iShaderVarStringSet> (objReg,
	      "crystalspace.shader.variablenameset");
	  CS_ASSERT (svNameStringSet);
	  
	  csRef<iShaderManager> shaderManager = csQueryRegistry<iShaderManager> (objReg);
	  CS_ASSERT (shaderManager);
	      
	  csRef<iShader> tonemap = loader->LoadShader ("/shader/postproc/hdr/reinhard_simple.xml");
	  hdr.SetMappingShader (tonemap);
	
	  svHDRScale = shaderManager->GetVariableAdd (svNameStringSet->Request (
	    "hdr scale"));
	  float exposure = luminance.GetColorScale();
	  svHDRScale->SetValue (csVector4 (1.0f/exposure, exposure, 0, 0));
	  
	  svMappingParams = shaderManager->GetVariableAdd (svNameStringSet->Request (
	    "mapping params"));
	}
	
	void Reinhard_Simple::ApplyExposure (RenderTreeBase& renderTree, iView* view)
	{
	  if (!hdr) return;
	  
	  csTicks currentTime = csGetTicks();
	  float avgLum, maxLum, maxComp, exposure;
	  if (luminance.ComputeLuminance (renderTree, view,
	      avgLum, maxLum, maxComp, exposure) && (lastTime != 0))
	  {
	    if (hdr->IsRangeLimited())
	    {
	      // Some pixels saturate, so change the HDR scaling to accomodate that
	      if (maxComp > (253.0f/255.0f))
	      {
		/* Agressively increase the range: if the range is too small
		   the scene gets a very flat look (large saturated areas that
		   turn out to be a dark gray). If the range is too large the
		   next check will gradually decrease it. */
		exposure = exposure * 0.77f;
	      }
	      else if ((maxComp > SMALL_EPSILON) && (maxComp < (250.0f/255.0f)))
	      {
		float d = (253.0f/255.0f)/maxComp;
		exposure = exposure * d;
	      }
	      exposure = csMin (exposure, 16.0f);
	      luminance.SetColorScale (exposure);
	      svHDRScale->SetValue (csVector4 (1.0f/exposure, exposure, 0, 0));
	    }
	      
	    svMappingParams->SetValue (csVector3 (avgLum, 0.18f,
	      csMax (maxLum*(254.0f/255.0f), 1.0f)));
	    
	  }
	  
	  lastTime = currentTime;
	}
  
        //-------------------------------------------------------------------
        
	Configurable::AbstractExposure* Configurable::CreateExposure (const char* name)
	{
	  if (strcmp (name, "linear") == 0)
	    return new WrapperExposure<Linear>;
	  if (strcmp (name, "reinhard_simple") == 0)
	    return new WrapperExposure<Reinhard_Simple>;
	  return 0;
	}
        
        Configurable::~Configurable()
        {
          delete exposure;
        }
  
	void Configurable::Initialize (iObjectRegistry* objReg,
                                       HDRHelper& hdr,
                                       const HDRSettings& settings)
	{
          const char messageID[] = "crystalspace.rendermanager.hdr.exposure";
  
	  csRef<iVerbosityManager> verbosity = csQueryRegistry<iVerbosityManager> (
	    objReg);
	  bool doVerbose = verbosity && verbosity->Enabled ("rendermanager.hdr.exposure");
	  
	  const char* exposureStr = settings.GetExposureMethod();
	  if (!exposureStr) exposureStr = "reinhard_simple";
	  if (doVerbose)
	  {
	    csReport (objReg, CS_REPORTER_SEVERITY_NOTIFY, messageID,
	      "Configured exposure type: %s", CS::Quote::Single (exposureStr));
	  }
	  exposure = CreateExposure (exposureStr);
	  if (!exposure)
	  {
	    csReport (objReg, CS_REPORTER_SEVERITY_WARNING, messageID,
	      "Invalid exposure type %s", CS::Quote::Single (exposureStr));
	  }
	  else
	  {
	    exposure->Initialize (objReg, hdr);
	  }
	}
	
	void Configurable::ApplyExposure (RenderTreeBase& renderTree, iView* view)
	{
	  if (exposure) exposure->ApplyExposure (renderTree, view);
	}
      } // namespace Exposure
    } // namespace HDR
  } // namespace RenderManager
} // namespace CS
