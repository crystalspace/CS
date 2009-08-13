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

#include "csgfx/imagememory.h"
#include "csgfx/renderbuffer.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csutil/fifo.h"
#include "csutil/objreg.h"
#include "csutil/sysfunc.h"
#include "imap/loader.h"
#include "iutil/databuff.h"
#include "iutil/string.h"
#include "ivaria/profile.h"

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
	  float avgLum, maxLum;
	  if (luminance.ComputeLuminance (renderTree, view,
	      avgLum, maxLum) && (lastTime != 0))
	  {
	    uint deltaTime = csMin (currentTime-lastTime, (uint)33);
	    const float exposureAdjust = exposureChangeRate*deltaTime/1000.0f;
	    float exposure = luminance.GetColorScale ();
	    if (avgLum >= targetAvgLum+targetAvgLumTolerance)
	      exposure -= exposureAdjust;
	    else if (avgLum <= targetAvgLum-targetAvgLumTolerance)
	      exposure += exposureAdjust;
	    luminance.SetColorScale (exposure);
	      
	    svHDRScale->SetValue (csVector4 (1.0f/exposure, exposure, 0, 0));
	  }
	  
	  lastTime = currentTime;
	}
  
      } // namespace Exposure
    } // namespace HDR
  } // namespace RenderManager
} // namespace CS
