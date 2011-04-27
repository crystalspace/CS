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

#include "cssysdef.h"

#include "csplugincommon/rendermanager/hdrexposure_luminance.h"

#include "iutil/string.h"
#include "ivaria/profile.h"
#include "csutil/fifo.h"

namespace CS
{
  namespace RenderManager
  {
    namespace HDR
    {
      namespace Luminance
      {
	CS_DECLARE_PROFILER
	CS_DECLARE_PROFILER_ZONE(HDRLuminance_GetResultData);
	CS_DECLARE_PROFILER_ZONE(HDRLuminance_GetResultData_Readback);
	CS_DECLARE_PROFILER_ZONE(HDRLuminance_GetResultData_DrawFX);

	void BaseHierarchical::Initialize (iObjectRegistry* objReg,
	  HDRHelper& hdr,
	  const char* firstShader, const char* stepShader)
	{
	  this->hdr = &hdr;
	  measureLayer = hdr.GetMeasureLayer();
	  PostEffectManager::LayerOptions measureOpts = measureLayer->GetOptions();
	  measureOpts.noTextureReuse = true;
	  measureLayer->SetOptions (measureOpts);
	  
	  graphics3D = csQueryRegistry<iGraphics3D> (objReg);
      
	  csRef<iLoader> loader (csQueryRegistry<iLoader> (objReg));
	  CS_ASSERT(loader);
	  svNameStringSet = 
	    csQueryRegistryTagInterface<iShaderVarStringSet> (objReg,
	      "crystalspace.shader.variablenameset");
	  CS_ASSERT (svNameStringSet);
	  
	  shaderManager = csQueryRegistry<iShaderManager> (objReg);
	  CS_ASSERT (shaderManager);
	      
	  computeFX.Initialize (objReg);
	  computeFX.SetIntermediateTargetFormat (intermediateTextureFormat);
	
	  computeShader1 =
	    loader->LoadShader (firstShader);
	  computeShaderN =
	    loader->LoadShader (stepShader);
	}
	  
	csPtr<iDataBuffer> BaseHierarchical::GetResultData (RenderTreeBase& renderTree,
	                                                    iView* view,
	                                                    int& resultW, int& resultH,
	                                                    float& usedColorScale)
	{
	  if (!measureLayer || !hdr) return 0;
	  
	  CS_PROFILER_ZONE(HDRLuminance_GetResultData);
	  
	  iTextureHandle* measureTex =
	    hdr->GetHDRPostEffects().GetLayerOutput (measureLayer);

	  // (Re-)create computeTarget if not created/view dimensions changed
	  if ((computeStages.GetSize() == 0)
	      || (view->GetContext ()->GetWidth () != lastTargetW)
	      || (view->GetContext ()->GetHeight () != lastTargetH)
	      || (lastMeasureTex != measureTex))
	  {
	    lastTargetW = view->GetContext ()->GetWidth ();
	    lastTargetH = view->GetContext ()->GetHeight ();
	    SetupStages (lastTargetW, lastTargetH, measureTex);
	    lastMeasureTex = measureTex;
	  }
	  
	  measureTex = computeStages[computeStages.GetSize()-1].target;
	  {
	    CS_PROFILER_ZONE(HDRLuminance_GetResultData_DrawFX);
	    computeFX.DrawPostEffects (renderTree);
	  }
	  
	  int newW, newH;
	  measureTex->GetRendererDimensions (newW, newH);
	  
	  csRef<iDataBuffer> newData;
	  {
	    CS_PROFILER_ZONE(HDRLuminance_GetResultData_Readback);
	    newData = measureTex->Readback (readbackFmt, 0);
	  }
	  
	  //lastData = newData;
	  //lastW = newW; lastH = newH;
	  //lastColorScale = colorScale;
	  
	  csRef<iDataBuffer> ret;
	  if (lastData.IsValid())
	  {
	    resultW = lastW;
	    resultH = lastH;
	    ret = lastData;
	    usedColorScale = lastColorScale;
	  }
	  
	  lastData = newData;
	  lastW = newW; lastH = newH;
	  lastColorScale = colorScale;
	  
	  return csPtr<iDataBuffer> (ret);
	}
	
	bool BaseHierarchical::FindBlockSize (iShader* shader,
					      size_t pticket,
					      const CS::Graphics::RenderMeshModes& modes,
					      const csShaderVariableStack& stack,
					      int maxW, int maxH,
					      int& blockSizeX, int& blockSizeY,
					      csRef<iShader>* usedShader)
	{
	  // Iterate over techniques, find block size
	  csRef<iShaderPriorityList> shaderPrios (
	    shader->GetAvailablePriorities (pticket));
	  for (size_t i = 0; i < shaderPrios->GetCount(); i++)
	  {
	    int prio = shaderPrios->GetPriority (i);
	    csRef<iString> filterSizeXStr (
	      computeShader1->GetTechniqueMetadata (prio, "filterSizeX"));
	    if (!filterSizeXStr.IsValid()) continue;
	    csRef<iString> filterSizeYStr (
	      computeShader1->GetTechniqueMetadata (prio, "filterSizeY"));
	    if (!filterSizeYStr.IsValid()) continue;
	
	    int w, h;
	    char dummy;
	    if (sscanf (filterSizeXStr->GetData(), "%d%c", &w, 
		&dummy) != 1)
	      continue;
	    if (sscanf (filterSizeYStr->GetData(), "%d%c", &h, 
		&dummy) != 1)
	      continue;
	    
	    csRef<iShader> prioShader (shader->ForceTechnique (prio));
	    /* 'Validate' shader (ie, is there actually a technique for the
	       priotity) */
	    size_t ticket = prioShader->GetTicket (modes, stack);
	    if (ticket == csArrayItemNotFound) continue;
	      
	    if ((w <= maxW) && (h <= maxH))
	    {
	      blockSizeX = w;
	      blockSizeY = h;
	      if (usedShader) *usedShader = prioShader;
	      return true;
	    }
	  }
	  return false;
	}
	
	namespace
	{
	  struct ProcessingPart
	  {
	    csRect sourceRect;
	    uint destOffsX, destOffsY;
	  };
	  struct ProcessingPartFinal
	  {
	    csRef<iShader> shader;
	    csRect sourceRect;
	    csRect destRect;
	  };
	}
	  
	bool BaseHierarchical::SetupStage (LuminanceComputeStage& stage,
					    int inputW, int inputH, int minSize,
					    iTextureHandle* inputTex,
					    iShader* computeShader)
	{
	  stage.svInput.AttachNew (new csShaderVariable (
	    svNameStringSet->Request ("tex diffuse")));
	    
	  size_t pticket;
	  CS::Graphics::RenderMeshModes modes; // Just keep defaults
	  csShaderVariableStack svstack;
    
	  {
	    PostEffectManager::Layer* tempLayer;
	    PostEffectManager::LayerInputMap inputMap;
	    inputMap.manualInput = stage.svInput;
	    tempLayer = computeFX.AddLayer (computeShader, 1, &inputMap);
	    
	    // Determine 'priority ticket' for stage
	    svstack.Setup (shaderManager->GetSVNameStringset ()->GetSize ());
	    computeFX.GetLayerRenderSVs (tempLayer, svstack);
	    pticket = computeShader->GetPrioritiesTicket (modes, svstack);
	    computeFX.RemoveLayer (tempLayer);
	  }
	  
	  int maxBlockSizeX = 16;
	  int maxBlockSizeY = 16;
	  
	  csRef<iShader> shader;
	  FindBlockSize (computeShader, pticket, modes, svstack,
	    inputW, inputH,
	    maxBlockSizeX, maxBlockSizeY, 0);
	      
	  csArray<ProcessingPartFinal> finalParts;
	  csFIFO<ProcessingPart> remainingParts;
	  {
	    ProcessingPart firstPart;
	    firstPart.sourceRect.Set (0, 0, inputW, inputH);
	    firstPart.destOffsX = 0;
	    firstPart.destOffsY = 0;
	    remainingParts.Push (firstPart);
	  }
	  while (remainingParts.GetSize() > 0)
	  {
	    ProcessingPart part = remainingParts.PopTop();
	    int blockSizeX = 16;
	    int blockSizeY = 16;
	    
	    csRef<iShader> shader;
	    FindBlockSize (computeShader, pticket, modes, svstack,
	      part.sourceRect.Width(), part.sourceRect.Height(),
	      blockSizeX, blockSizeY, &shader);
	    // @@@ Handle failure
	    
	    int destW = part.sourceRect.Width()/blockSizeX;
	    int destH = part.sourceRect.Height()/blockSizeY;
	    int coveredW = destW*blockSizeX;
	    int coveredH = destH*blockSizeY;
	    int remainderX = part.sourceRect.Width()-coveredW;
	    int remainderY = part.sourceRect.Height()-coveredH;
	    if (remainderX > 0)
	    {
	      ProcessingPart newPart;
	      newPart.sourceRect.Set (
		part.sourceRect.xmin+coveredW, 
		part.sourceRect.ymin,
		part.sourceRect.xmin+coveredW+remainderX,
		part.sourceRect.ymin+coveredH);
	      newPart.destOffsX = part.destOffsX+destW;
	      newPart.destOffsY = part.destOffsY;
	      remainingParts.Push (newPart);
	    }
	    if (remainderY > 0)
	    {
	      ProcessingPart newPart;
	      newPart.sourceRect.Set (
		part.sourceRect.xmin, 
		part.sourceRect.ymin+coveredH,
		part.sourceRect.xmin+coveredW, 
		part.sourceRect.ymin+coveredH+remainderY);
	      newPart.destOffsX = part.destOffsX;
	      newPart.destOffsY = part.destOffsY+destH;
	      remainingParts.Push (newPart);
	    }
	    if ((remainderX > 0) && (remainderY > 0))
	    {
	      ProcessingPart newPart;
	      newPart.sourceRect.Set (
		part.sourceRect.xmin+coveredW, 
		part.sourceRect.ymin+coveredH,
		part.sourceRect.xmin+coveredW+remainderX, 
		part.sourceRect.ymin+coveredH+remainderY);
	      newPart.destOffsX = part.destOffsX+destW;
	      newPart.destOffsY = part.destOffsY+destH;
	      remainingParts.Push (newPart);
	    }
	    
	    ProcessingPartFinal finalPart;
	    finalPart.sourceRect.Set (
	      part.sourceRect.xmin, part.sourceRect.ymin,
	      part.sourceRect.xmin+coveredW,
	      part.sourceRect.ymin+coveredH);
	    finalPart.destRect.Set (
	      part.destOffsX, part.destOffsY,
	      part.destOffsX+destW, part.destOffsY+destH);
	    finalPart.shader = shader;
	    
	    finalParts.Push (finalPart);
	  }
	  
	  stage.targetW = finalParts[0].destRect.xmax;
	  stage.targetH = finalParts[0].destRect.ymax;
	  for (size_t l = 1; l < finalParts.GetSize(); l++)
	  {
	    stage.targetW = csMax (stage.targetW, finalParts[l].destRect.xmax);
	    stage.targetH = csMax (stage.targetH, finalParts[l].destRect.ymax);
	  }
	  bool lastStage = (stage.targetW <= minSize) && (stage.targetH <= minSize);
	  uint texFlags =
	    CS_TEXTURE_3D | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP | CS_TEXTURE_NOMIPMAPS;
	  csString stageFormat;
	  if (lastStage)
	    stageFormat = readbackFmt.GetCanonical();
	  else
	    stageFormat = computeFX.GetIntermediateTargetFormat();
	  stage.target = graphics3D->GetTextureManager ()->CreateTexture (stage.targetW,
	    stage.targetH, csimg2D, stageFormat, texFlags);
	  
	  
	  int targetPixels = stage.targetW * maxBlockSizeX 
	    * stage.targetH * maxBlockSizeY;
	  int sourcePixels = inputW * inputH;
	  stage.svWeightCoeff.AttachNew (new csShaderVariable (
	    svNameStringSet->Request ("weight coeff")));
	  stage.svWeightCoeff->SetValue (float(targetPixels)/float(sourcePixels));
	  
	  // Set measureTex as input to first layer of computeFX
	  stage.svInput->SetValue (inputTex);
	  
	  PostEffectManager::Layer* outputLayer = 0;
	  for (size_t l = 0; l < finalParts.GetSize(); l++)
	  {
	    PostEffectManager::Layer* layer;
	    PostEffectManager::LayerInputMap inputMap;
	    inputMap.manualInput = stage.svInput;
	    inputMap.sourceRect = finalParts[l].sourceRect;
	    inputMap.inputPixelSizeName = "input pixel size";
	    PostEffectManager::LayerOptions options;
	    options.targetRect = finalParts[l].destRect;
	    if (outputLayer == 0)
	    {
	      options.manualTarget = stage.target;
	      options.readback = lastStage;
	    }
	    else
	      options.renderOn = outputLayer;
	    //inputMap.manualTexcoords = computeTexcoordBuf;
	    layer = computeFX.AddLayer (finalParts[l].shader, options, 1, &inputMap);
	    
	    layer->GetSVContext()->AddVariable (stage.svInput);
	    layer->GetSVContext()->AddVariable (stage.svWeightCoeff);
	    stage.layers.Push (layer);
	    if (outputLayer == 0) outputLayer = layer;
	  }
	  return !lastStage;
	}
	
	void BaseHierarchical::SetupStages (int targetW, int targetH,
					    iTextureHandle* measureTex)
	{
	  computeStages.Empty();
	  computeFX.ClearLayers();

	  int currentW = targetW;
	  int currentH = targetH;
	  const int minSize = 256; /* Arbitrary
	                              @@@ In fact I [res] don't know the cut
	                              off point after which the multi-stage
	                              approach is faster than a readback of
	                              the first stage, or if that exists at
	                              all... */
	  iTextureHandle* currentTex = measureTex;
	  bool iterateStage;
	  do
	  {
	    LuminanceComputeStage newStage;
	    iterateStage = SetupStage (newStage, currentW, currentH, minSize,
	      currentTex,
	      computeStages.GetSize() == 0 ? computeShader1 : computeShaderN);
	    computeStages.Push (newStage);
	    currentW = newStage.targetW;
	    currentH = newStage.targetH;
	    currentTex = newStage.target;
	  }
	  while (iterateStage);
	  
	  computeFX.SetEffectsOutputTarget (computeStages[0].target);
	  CS::Math::Matrix4 perspectiveFixup;
	  computeFX.SetupView (computeStages[0].targetW, computeStages[0].targetH,
	    perspectiveFixup);
	}
	
	//-------------------------------------------------------------------
	
	void Average::Initialize (iObjectRegistry* objReg,
	  HDRHelper& hdr)
	{
	  BaseHierarchical::Initialize (objReg, hdr,
	    "/shader/postproc/hdr/luminance/average_1.xml",
	    "/shader/postproc/hdr/luminance/average_n.xml");
	}
	    
	bool Average::ComputeLuminance (RenderTreeBase& renderTree, iView* view,
	                                float& averageLuminance, float& maxLuminance,
	                                float& usedColorScale)
	{
	  int W, H;
	  csRef<iDataBuffer> computeData = GetResultData (renderTree, view, W, H,
	    usedColorScale);
	  if (computeData.IsValid ())
	  {
	    const uint8* bgra = computeData->GetUint8();
	    int numPixels = W * H;
	    float totalLum = 0;
	    float maxLum = 0;
	    for (int i = 0; i < numPixels; i++)
	    {
	      int b = *bgra++;
	      int g = *bgra++;
	      int r = *bgra++;
	      int a = *bgra++;
	      totalLum += (g+a)/510.0f;
	      if (b > r)
	        maxLum = csMax (maxLum, b/255.0f);
	      else
	        maxLum = csMax (maxLum, r/255.0f);
	    }
	    
	    averageLuminance = (totalLum / numPixels) * colorScale;
	    maxLuminance = maxLum;
	    return true;
	  }
	  return false;
	}
	
	//-------------------------------------------------------------------
	
	void LogAverage::Initialize (iObjectRegistry* objReg,
	  HDRHelper& hdr)
	{
	  BaseHierarchical::Initialize (objReg, hdr,
	    "/shader/postproc/hdr/luminance/logaverage_1.xml",
	    "/shader/postproc/hdr/luminance/logaverage_n.xml");
	}
	    
	bool LogAverage::ComputeLuminance (RenderTreeBase& renderTree, iView* view,
	                                   float& averageLuminance, float& maxLuminance,
	                                   float& maxComponent, float& usedColorScale)
	{
	  int W, H;
	  csRef<iDataBuffer> computeData = GetResultData (renderTree, view, W, H,
	    usedColorScale);
	  if (computeData.IsValid ())
	  {
	    const float* rgba = (float*)computeData->GetData();
	    int numPixels = W * H;
	    float lumSum = 0;
	    float maxLum = 0;
	    float maxComp = 0;
	    for (int i = 0; i < numPixels; i++)
	    {
	      float r = *rgba++;
	      float g = *rgba++;
	      float b = *rgba++;
	      rgba++;
	      lumSum += g;
	      maxLum = csMax (maxLum, r);
	      maxComp = csMax (maxComp, b);
	    }
	    
	    int numOrgPixels = view->GetContext ()->GetWidth ()
	      * view->GetContext ()->GetHeight ();
	    averageLuminance = expf (1.0f/numOrgPixels * lumSum);
	    maxLuminance = maxLum;
	    maxComponent = maxComp;
	    return true;
	  }
	  return false;
	}
      } // namespace Luminance
    } // namespace HDR
  } // namespace RenderManager
} // namespace CS
