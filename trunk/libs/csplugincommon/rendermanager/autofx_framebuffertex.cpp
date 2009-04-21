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

#include "csplugincommon/rendermanager/autofx_framebuffertex.h"

#include "csutil/cfgacc.h"
#include "csutil/objreg.h"
#include "imap/loader.h"

namespace CS
{
  namespace RenderManager
  {
    namespace AutoFX
    {
#define FTBPD   FramebufferTex_Base::PersistentData
  
      FTBPD::PersistentData() :
	texCacheColor (csimg2D, "abgr8",
	  CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	    | CS_TEXTURE_NPOTS | CS_TEXTURE_SCALE_UP,
	  "target", 0,
	  CS::Utility::ResourceCache::ReuseAlways ()),
	texCacheDepth (csimg2D, "d32",
	  CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	    | CS_TEXTURE_NPOTS | CS_TEXTURE_SCALE_UP | CS_TEXTURE_NOFILTER,
	  "target", 0,
	  CS::Utility::ResourceCache::ReuseAlways ()),
	svAlloc (1024)
      {
      }
  
      void FTBPD::Initialize (iObjectRegistry* objReg,
				PostEffectManager* postEffects)
      {
	csRef<iShaderManager> shaderManager =
	  csQueryRegistry<iShaderManager> (objReg);
	
	iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	const char* const svNames[rtaNumAttachments] = {"depth", "color"};
	for (size_t i = 0; i < rtaNumAttachments; i++)
	{
	  csString svName;
	  svName.Format ("tex framebuffer %s", svNames[i]);
	  svTexFramebuffer[i] = strings->Request (svName);
	}
	svFramebufferCoordXform = strings->Request ("framebuffer coord xform");
	
	if (postEffects)
	  texCacheColor.SetFormat (postEffects->GetIntermediateTargetFormat());
	csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objReg);
	  
	texCacheColor.SetG3D (g3d);
	texCacheDepth.SetG3D (g3d);
      }
  
      iTextureHandle* FTBPD::GetFramebufferTex (size_t n, int width,
						  int height)
      {
	if (!framebufferTex[n].IsValid())
	{
	  csRef<iTextureHandle> newTex;
	  switch (n)
	  {
	  case rtaColor0:
	    newTex = texCacheColor.QueryUnusedTexture (width, height);
	    break;
	  case rtaDepth: 
	    newTex = texCacheDepth.QueryUnusedTexture (width, height);
	    break;
	  default:
	    CS_ASSERT(false);
	  }
	
	  framebufferTex[n] = newTex;
	}
	return framebufferTex[n];
      }
  
      void FTBPD::UpdateNewFrame ()
      {
	svKeeper.Empty();
	csTicks currentTicks = csGetTicks ();
	texCacheColor.AdvanceFrame (currentTicks);
	texCacheDepth.AdvanceFrame (currentTicks);
      }
    } // namespace AutoFX
  } // namespace RenderManager
} // namespace CS
