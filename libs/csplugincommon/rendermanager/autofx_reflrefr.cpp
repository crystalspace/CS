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

#include "csplugincommon/rendermanager/autofx_reflrefr.h"

#include "csutil/cfgacc.h"
#include "csutil/objreg.h"
#include "imap/loader.h"

namespace CS
{
  namespace RenderManager
  {
    namespace AutoFX
    {
#define RRBPD   ReflectRefract_Base::PersistentData
  
      RRBPD::PersistentData() :
	currentFrame (0),
	texCache (csimg2D, "rgb8",  // @@@ FIXME: Use same format as main view ...
	  CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	    | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP,
	  "target", 0,
	  CS::Utility::ResourceCache::ReuseIfOnlyOneRef ()),
	texCacheDepth (csimg2D, "d32",
	  CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	    | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP | CS_TEXTURE_NOFILTER,
	  "target", 0,
	  CS::Utility::ResourceCache::ReuseIfOnlyOneRef ())
      {
      }
  
      void RRBPD::Initialize (iObjectRegistry* objReg,
				RenderTreeBase::DebugPersistent& dbgPersist,
				PostEffectManager* postEffects)
      {
	dbgReflRefrTex = dbgPersist.RegisterDebugFlag ("textures.reflrefr");
      
	csRef<iShaderManager> shaderManager =
	  csQueryRegistry<iShaderManager> (objReg);
	
	iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	svTexPlaneRefl = strings->Request ("tex plane reflect");
	svTexPlaneRefr = strings->Request ("tex plane refract");
	svTexPlaneReflDepth = strings->Request ("tex plane reflect depth");
	svTexPlaneRefrDepth = strings->Request ("tex plane refract depth");
	
	svPlaneRefl = strings->Request ("plane reflection");
	svClipPlaneReflRefr = strings->Request ("clip plane reflection");
	
	csConfigAccess config (objReg);
	resolutionReduceRefl = config->GetInt (
	  "RenderManager.Reflections.Downsample", 1);
	resolutionReduceRefr = config->GetInt (
	  "RenderManager.Refractions.Downsample", resolutionReduceRefl);
	texUpdateInterval = config->GetInt (
	  "RenderManager.Reflections.UpdateInterval", 0);
	maxUpdatesPerFrame = config->GetInt (
	  "RenderManager.Reflections.MaxUpdatesPerFrame", 0);
	mappingStretch = config->GetFloat (
	  "RenderManager.Reflections.MappingStretch", 1.0f);
	cameraChangeThresh = config->GetFloat (
	  "RenderManager.Reflections.CameraChangeThreshold", 0.01f);
	
	svReflXform = strings->Request ("reflection coord xform");
	reflXformSV.AttachNew (new csShaderVariable (svReflXform));
	svRefrXform = strings->Request ("refraction coord xform");
	refrXformSV.AttachNew (new csShaderVariable (svRefrXform));
	screenFlipped = postEffects ? postEffects->ScreenSpaceYFlipped() : false;
	  
	csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objReg);
	texCache.SetG3D (g3d);
	texCacheDepth.SetG3D (g3d);
      }
  
      void RRBPD::UpdateNewFrame ()
      {
	csTicks currentTicks = csGetTicks ();
	ReflRefrCache::GlobalIterator reflRefrIt (
	  reflRefrCache.GetIterator ());
	while (reflRefrIt.HasNext())
	{
	  ReflectRefractSVs& meshReflectRefract = reflRefrIt.NextNoAdvance();
	  // Don't remove if update interval hasn't passed yet
	  if ((texUpdateInterval > 0)
	    && ((currentTicks - meshReflectRefract.lastUpdate) <=
	      texUpdateInterval))
	  {
	    reflRefrIt.Next();
	    continue;
	  }
	  // Don't remove if not in line for round-robin update
	  if ((maxUpdatesPerFrame > 0)
	    && (currentFrame - meshReflectRefract.lastUpdateFrame < 
	      ((reflRefrCache.GetSize()+maxUpdatesPerFrame-1) / maxUpdatesPerFrame)))
	  {
	    reflRefrIt.Next();
	    continue;
	  }
	  reflRefrCache.DeleteElement (reflRefrIt);
	}
	
	currentFrame++;
	updatesThisFrame = 0;
      
	texCache.AdvanceFrame (currentTicks);
	texCacheDepth.AdvanceFrame (currentTicks);
      }
    } // namespace AutoFX
  } // namespace RenderManager
} // namespace CS
