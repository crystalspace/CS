/*
    Copyright (C) 2008-2009 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_LUMINANCE_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_LUMINANCE_H__

#include "csgfx/textureformatstrings.h"
#include "csplugincommon/rendermanager/hdrhelper.h"
#include "csplugincommon/rendermanager/posteffects.h"

namespace CS
{
  namespace RenderManager
  {
    namespace HDR
    {
      namespace Luminance
      {
        class CS_CRYSTALSPACE_EXPORT BaseHierarchical
        {
        protected:
	  float colorScale;
	  
	  const char* intermediateTextureFormat;
	  CS::StructuredTextureFormat readbackFmt;
	  PostEffectManager::Layer* measureLayer;
	  HDRHelper* hdr;
	  csRef<iGraphics3D> graphics3D;
	  csRef<iShaderVarStringSet> svNameStringSet;
	  csRef<iShaderManager> shaderManager;
	  
	  csRef<iShader> computeShader1;
	  csRef<iShader> computeShaderN;
	  struct LuminanceComputeStage
	  {
	    csArray<PostEffectManager::Layer*> layers;
	    csRef<csShaderVariable> svInput;
	    csRef<csShaderVariable> svWeightCoeff;
	    csRef<iTextureHandle> target;
	    int targetW, targetH;
	    
	    LuminanceComputeStage() {}
	  };
	  csArray<LuminanceComputeStage> computeStages;
	  PostEffectManager computeFX;
	  
	  int lastTargetW, lastTargetH;
	  csRef<iDataBuffer> lastData;
	  int lastW, lastH;
	  
	  BaseHierarchical (const char* intermediateTextureFormat,
	    const char* outputTextureFormat) : colorScale (1.0f), 
	    intermediateTextureFormat (intermediateTextureFormat),
	    readbackFmt (CS::TextureFormatStrings::ConvertStructured (outputTextureFormat)),
	    measureLayer (0), hdr (0) {}
	   
	  /// Set up HDR exposure control for a post effects manager
	  void Initialize (iObjectRegistry* objReg,
	    HDRHelper& hdr,
	    const char* firstShader, const char* stepShader);
	    
	  /// Obtain rendered image
	  csPtr<iDataBuffer> GetResultData (RenderTreeBase& renderTree, 
	    iView* view, int& resultW, int& resultH);
	private:
	  bool FindBlockSize (iShader* shader, size_t pticket,
	    int maxW, int maxH,
	    int& blockSizeX, int& blockSizeY, csRef<iShader>* usedShader);
	  bool SetupStage (LuminanceComputeStage& stage,
	    int inputW, int inputH, int minSize, iTextureHandle* inputTex,
	    iShader* computeShader);
	  void SetupStages (int targetW, int targetH);
	public:
	  float GetColorScale () const { return colorScale; }
	  void SetColorScale (float scale) { colorScale = scale; }
        };
        
        class CS_CRYSTALSPACE_EXPORT Average : public BaseHierarchical
        {
        public:
          Average() : BaseHierarchical ("abgr8", "abgr8") {}
        
	  void Initialize (iObjectRegistry* objReg,
	    HDRHelper& hdr);
	    
	  bool ComputeLuminance (RenderTreeBase& renderTree, iView* view,
	    float& averageLuminance, float& maxLuminance);
        };
      } // namespace Luminance
    } // namespace HDR
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_HDREXPOSURE_LUMINANCE_H__
