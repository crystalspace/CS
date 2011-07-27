/*
    Copyright (C) 2011 by Santiago Sanchez

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __GLOBALILLUM_H__
#define __GLOBALILLUM_H__

#include "cssysdef.h"

#include "csgfx/shadervar.h"
#include "csgfx/imagememory.h"
#include "igraphic/imageio.h"
#include "imap/loader.h"
#include "csutil/cfgacc.h"
#include "csutil/scfstr.h"
#include "ivideo/shader/shader.h"
#include "deferredrender.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

  /**
   * Generates a halton number
   */
  inline float halton(const int base, int index) 
  {
	  float x = 0.0f;
	  float f = 1.0f / base;

	  while(index) 
    {
		  x += f * (float) (index % base);
		  index /= base;
		  f *= 1.0f / base;
	  }
	  return x;
  }

  /**
   * Renderer for screen-space global illumination techniques such as SSAO and SSDO.
   */
  class csGlobalIllumRenderer
  {
  public:

    csGlobalIllumRenderer() : graphics3D (nullptr), globalIllumBuffer (nullptr), compositionBuffer (nullptr),
      intermediateBuffer (nullptr), gbuffer (nullptr), enabled (true), isInitialized (false) 
    {         
    }
    
    ~csGlobalIllumRenderer() 
    {
      CS_ASSERT (!isGlobalIllumBufferAttached && !isCompositionBufferAttached & !isIntermediateBufferAttached);
    }
    
    bool Initialize(iGraphics3D *g3D, iObjectRegistry *objRegistry, GBuffer *gbuffer, 
      bool readEnableKeyFromConfig = true)
    {
      reporterMessageID = "crystalspace.rendermanager.globalillum";

      if (isInitialized)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, reporterMessageID,
          "Component already initialized!");
        return true;
      }

      graphics3D = g3D;
      isGlobalIllumBufferAttached = false;
      isCompositionBufferAttached = false;
      isIntermediateBufferAttached = false;
      
      objectRegistry = objRegistry;
      csConfigAccess cfg (objRegistry);

      if (readEnableKeyFromConfig)
        enabled = cfg->GetBool ("RenderManager.Deferred.GlobalIllum.Enable", true);

      if (!enabled)
      {        
        return false;
      }

      this->gbuffer = gbuffer;

      if (!InitRenderBuffers (cfg))
      {
        enabled = false;
        return false;
      }     
      
      csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
      shaderManager = csQueryRegistry<iShaderManager> (objRegistry);
      svStringSet = shaderManager->GetSVNameStringset ();

      globalIllumShader = loader->LoadShader ("/shader/deferred/globalillum.xml");
      if (!globalIllumShader)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load deferred_globalillum shader");
        enabled = false;
        return false;
      }

      horizontalBlurShader = loader->LoadShader ("/shader/deferred/horizontal_blur.xml");
      if (!horizontalBlurShader)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load deferred_horizontal_blur shader");
        enabled = false;
        return false;
      }

      verticalBlurShader = loader->LoadShader ("/shader/deferred/vertical_blur.xml");
      if (!verticalBlurShader)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load deferred_vertical_blur shader");
        enabled = false;
        return false;
      }

      lightCompositionShader = loader->LoadShader ("/shader/deferred/composition.xml");
      if (!lightCompositionShader)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load deferred_composition shader");
        enabled = false;
        return false;
      }

      LoadRandomNormalsTexture(loader, graphics3D);
      LoadIrradianceEnvironmentMap(loader, graphics3D);
      //GenerateSampleDirections (graphics3D);
      
      SetupShaderVars(cfg);           

      CreateFullscreenQuad();

      isInitialized = true;
      applyBlur = true;
      return true;
    }    

    void UpdateShaderVars()
    {
      if (!enabled)
        return;     

      lightCompositionShader->GetVariableAdd (
        svStringSet->Request ("debug show ambocc"))->SetValue ((int)showAmbientOcclusion);
      lightCompositionShader->GetVariableAdd (
        svStringSet->Request ("debug show globalillum"))->SetValue ((int)showGlobalIllumination);
    }

    void RenderGlobalIllum(int contextDrawFlags)
    {
      if (!enabled)
        return;

      // Draw (directional) ambient occlusion + indirect light
      AttachGlobalIllumBuffer();
      {
        graphics3D->SetZMode (CS_ZBUF_MESH);        

        DrawFullscreenQuad (globalIllumShader);
      }
      DetachGlobalIllumBuffer();

      // Apply edge-aware blur
      if (applyBlur)
      {
        AttachIntermediateBuffer();
        {
          graphics3D->SetZMode (CS_ZBUF_MESH);

          DrawFullscreenQuad (horizontalBlurShader);
        }
        DetachIntermediateBuffer();
	          
        AttachGlobalIllumBuffer();
        {
          graphics3D->SetZMode (CS_ZBUF_MESH);        

		      DrawFullscreenQuad (verticalBlurShader);
        }
        DetachGlobalIllumBuffer();
      }
      
      AttachCompositionBuffer();      
      {
        graphics3D->SetZMode (CS_ZBUF_MESH);
        
        DrawFullscreenQuad (lightCompositionShader);
      }
      DetachCompositionBuffer();
    }

    bool AttachGlobalIllumBuffer(bool useGBufferDepth = false)
    {
      if (!enabled || isGlobalIllumBufferAttached)
        return false;

      if (!graphics3D->SetRenderTarget (globalIllumBuffer, false, 0, rtaColor0))
          return false;

	  if (useGBufferDepth && gbuffer->HasDepthBuffer())
      {
        if (!graphics3D->SetRenderTarget (gbuffer->GetDepthBuffer(), false, 0, rtaDepth))
          return false;
      }

      if (!graphics3D->ValidateRenderTargets())
        return false;

      isGlobalIllumBufferAttached = true;
      return true;
    }

    void DetachGlobalIllumBuffer()
    {
      if (enabled && isGlobalIllumBufferAttached)
      {
        graphics3D->UnsetRenderTargets();
        isGlobalIllumBufferAttached = false;
      }      
    }

    bool AttachIntermediateBuffer(bool useGBufferDepth = false)
    {
      if (!enabled || isIntermediateBufferAttached)
        return false;

      if (!graphics3D->SetRenderTarget (intermediateBuffer, false, 0, rtaColor0))
          return false;

      if (useGBufferDepth && gbuffer->HasDepthBuffer())
      {
        if (!graphics3D->SetRenderTarget (gbuffer->GetDepthBuffer(), false, 0, rtaDepth))
          return false;
      }

      if (!graphics3D->ValidateRenderTargets())
        return false;

      isIntermediateBufferAttached = true;
      return true;
    }

    void DetachIntermediateBuffer()
    {
      if (enabled && isIntermediateBufferAttached)
      {
        graphics3D->UnsetRenderTargets();
        isIntermediateBufferAttached = false;
      }      
    }

    bool AttachCompositionBuffer(bool useGBufferDepth = false)
    {
      if (!enabled || isCompositionBufferAttached)
        return false;

      if (!graphics3D->SetRenderTarget (compositionBuffer, false, 0, rtaColor0))
          return false;

      if (useGBufferDepth && gbuffer->HasDepthBuffer())
      {
        if (!graphics3D->SetRenderTarget (gbuffer->GetDepthBuffer(), false, 0, rtaDepth))
          return false;
      }

      if (!graphics3D->ValidateRenderTargets())
        return false;

      isCompositionBufferAttached = true;
      return true;
    }

    void DetachCompositionBuffer()
    {
      if (enabled && isCompositionBufferAttached)
      {
        graphics3D->UnsetRenderTargets();
        isCompositionBufferAttached = false;
      }      
    }

    iTextureHandle *GetGlobalIllumBuffer()
    {
      return globalIllumBuffer;
    }

    iTextureHandle *GetIntermediateBuffer()
    {
      return intermediateBuffer;
    }

    iTextureHandle *GetLightCompositionBuffer()
    {
      return compositionBuffer;
    }

    bool IsInitialized()
    {
      return isInitialized;
    }

    bool IsEnabled() 
    { 
      return this->enabled;
    }

    void SetEnabled(bool value)
    {
      this->enabled = value;
    }    

  private:

    bool InitRenderBuffers(csConfigAccess &cfg)
    {
      globalIllumBufferFormat = cfg->GetStr ("RenderManager.Deferred.GlobalIllum.GlobalIllumBufferFormat",
        "rgba16_f");
      compositionBufferFormat = cfg->GetStr ("RenderManager.Deferred.GlobalIllum.CompositionBufferFormat", 
        "rgb16_f");
      float giBufferRes = 0.5f;

      const int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NPOTS;
      scfString errString;
      iGraphics2D *g2D = graphics3D->GetDriver2D();

      if (!globalIllumBuffer)
      {
        globalIllumBuffer = graphics3D->GetTextureManager()->CreateTexture (g2D->GetWidth() * giBufferRes, 
          g2D->GetHeight() * giBufferRes, csimg2D, globalIllumBufferFormat, flags, &errString);
      }
      if (!globalIllumBuffer)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
          "Could not create global illumination buffer! %s", errString.GetCsString().GetDataSafe());
        return false;
      }

      if (!intermediateBuffer)
      {
        intermediateBuffer = graphics3D->GetTextureManager()->CreateTexture (g2D->GetWidth() * giBufferRes,
          g2D->GetHeight() * giBufferRes, csimg2D, globalIllumBufferFormat, flags, &errString);
      }
      if (!intermediateBuffer)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
          "Could not create intermediate buffer! %s", errString.GetCsString().GetDataSafe());        
        return false;
      }

      if (!compositionBuffer)
      {
        compositionBuffer = graphics3D->GetTextureManager()->CreateTexture (g2D->GetWidth(), g2D->GetHeight(), 
          csimg2D, compositionBufferFormat, flags, &errString);
      }
      if (!compositionBuffer)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
          "Could not create composition buffer! %s", errString.GetCsString().GetDataSafe());        
        return false;
      }

      // Test if the buffer formats are supported.
      if (!AttachGlobalIllumBuffer())
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
            "Failed to attach global illumination buffer to the device!");
        return false;
      }

      if (!graphics3D->ValidateRenderTargets())
      {
        DetachGlobalIllumBuffer();
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
            "Global illumination buffer format is not supported by the device!");
        return false;
      }
      DetachGlobalIllumBuffer();

      if (!AttachIntermediateBuffer())
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
            "Failed to attach intermediate buffer to the device!");
        return false;
      }

      if (!graphics3D->ValidateRenderTargets())
      {
        DetachIntermediateBuffer();
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
            "Intermediate buffer format is not supported by the device!");
        return false;
      }
      DetachIntermediateBuffer();

      if (!AttachCompositionBuffer())
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
            "Failed to attach composition buffer to the device!");
        return false;
      }

      if (!graphics3D->ValidateRenderTargets())
      {
        DetachCompositionBuffer();
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
            "Composition buffer format is not supported by the device!");
        return false;
      }
      DetachCompositionBuffer();

      return true;
    }

    void SetupShaderVars(csConfigAccess &cfg)
    { 
      // Add variables to shaders
      globalIllumBufferSV = shaderManager->GetVariableAdd (
          svStringSet->Request ("tex global illumination"));
      globalIllumBufferSV->SetValue (globalIllumBuffer);

      intermediateBufferSV = verticalBlurShader->GetVariableAdd (
          svStringSet->Request ("tex horizontal blur"));
      intermediateBufferSV->SetValue (intermediateBuffer);
      
      //compositionBufferSV = lightCompositionShader->GetVariableAdd (
      //    svStringSet->Request ("tex light composition"));
      //compositionBufferSV->SetValue (compositionBuffer);
      
      csRef<csShaderVariable> shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("tex random normals"));      
      shaderVar->SetValue (randomNormalsTexture);
      
      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("tex globalillum envmap"));
      shaderVar->SetValue (irradianceEnvironmentMap);

      /*shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("pattern size"));
      int patternSize = cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.SamplingPatternSize", 4);
      shaderVar->SetValue (patternSize);*/

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("num passes"));
      int sampleCount = cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.NumPasses", 2);
      shaderVar->SetValue (sampleCount);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("sample radius"));
      float sampleRadius = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.SampleRadius", 0.1f);
      shaderVar->SetValue (sampleRadius);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("sample radius far"));
      float sampleRadiusFar = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.SampleRadiusFar", 0.6f);
      shaderVar->SetValue (sampleRadiusFar);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("self occlusion"));
      float depthBias = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.SelfOcclusion", 0.0f);
      shaderVar->SetValue (depthBias);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("occlusion strength"));
      float occlusionStrength = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.OcclusionStrength",
        0.7f);
      shaderVar->SetValue (occlusionStrength);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("max occluder distance"));
      float maxOccluderDistance = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.MaxOccluderDistance",
        0.8f);
      shaderVar->SetValue (maxOccluderDistance);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("occluder angle bias"));
      float lightRotationAngle = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.OccluderAngleBias",
        0.0f);
      shaderVar->SetValue (lightRotationAngle);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("bounce strength"));
      float bounceStrength = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.BounceStrength", 6.0f);
      shaderVar->SetValue (bounceStrength);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("enable ambient occlusion"));
      float enableAO = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.EnableAO", 1.0f);
      shaderVar->SetValue (enableAO);

      shaderVar = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("enable indirect light"));
      float enableIL = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.EnableIndirectLight", 1.0f);
      shaderVar->SetValue (enableIL);

      shaderVar = shaderManager->GetVariableAdd (
          svStringSet->Request ("ssao blur kernelsize"));
      int blurKernelSize = cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.BlurKernelSize", 3);
      shaderVar->SetValue (blurKernelSize);

      shaderVar = shaderManager->GetVariableAdd (
          svStringSet->Request ("ssao blur position threshold"));
      float blurPositionThreshold = cfg->GetFloat (
        "RenderManager.Deferred.GlobalIllum.SSDO.BlurPositionThreshold", 0.5f);
      shaderVar->SetValue (blurPositionThreshold);

      shaderVar = shaderManager->GetVariableAdd (
          svStringSet->Request ("ssao blur normal threshold"));
      float blurNormalThreshold = cfg->GetFloat (
        "RenderManager.Deferred.GlobalIllum.SSDO.BlurNormalThreshold", 0.1f);
      shaderVar->SetValue (blurNormalThreshold);

      showAmbientOcclusion = false;
      showGlobalIllumination = false;
      shaderVar = lightCompositionShader->GetVariableAdd (
          svStringSet->Request ("debug show ambocc"));
      shaderVar->SetValue ((int)showAmbientOcclusion);

      shaderVar = lightCompositionShader->GetVariableAdd (
          svStringSet->Request ("debug show globalillum"));
      shaderVar->SetValue ((int)showGlobalIllumination);
    }

    void CreateFullscreenQuad()
    {
      // Builds the quad.
      float w = graphics3D->GetDriver2D ()->GetWidth ();
      float h = graphics3D->GetDriver2D ()->GetHeight ();

      quadVerts[0] = csVector3 (0.0f, 0.0f, 0.0f);
      quadVerts[1] = csVector3 (0.0f,    h, 0.0f);
      quadVerts[2] = csVector3 (   w,    h, 0.0f);
      quadVerts[3] = csVector3 (   w, 0.0f, 0.0f);

      uint mixModeNoBlending = CS_MIXMODE_BLEND(ONE, ZERO) | CS_MIXMODE_ALPHATEST_DISABLE;

      quadMesh.meshtype = CS_MESHTYPE_TRIANGLEFAN;
      quadMesh.vertices = quadVerts;
      quadMesh.vertexCount = 4;
      quadMesh.z_buf_mode = CS_ZBUF_NONE;
      quadMesh.mixmode = mixModeNoBlending;
      quadMesh.alphaType.autoAlphaMode = false;
      quadMesh.alphaType.alphaType = csAlphaMode::alphaNone;
    }

    void LoadRandomNormalsTexture(iLoader *loader, iGraphics3D *graphics3D)
    {
      csRef<iImage> image;
      int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS;

      randomNormalsTexture = loader->LoadTexture ("/data/regularSphereJittered16.png", flags, 
        graphics3D->GetTextureManager(), &image);
      if (!randomNormalsTexture)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load random_normals.png texture!");
      }

      // save tex to file (remove, just for debug!)
      csRef<iImageIO> imageIO = csQueryRegistry<iImageIO> (objectRegistry); 
      csRef<iDataBuffer> data = imageIO->Save (image, "image/png");
      if (!data)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not export random_normals image to format png!");
      }

      csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectRegistry);
      vfs->ChDir ("/data");
      if (!vfs->WriteFile ("random_normals_debug", data->GetData(), data->GetSize()))
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not save random normals texture!");
      }
    }

    /**
      * Generates random positions inside a unit hemisphere based on halton numbers
      */
    /*void GenerateSampleDirections(iGraphics3D *graphics3D)
    {
	    int patternSizeSquared = patternSize * patternSize;

	    srand(0);

	    int haltonIndex = 0;
	    float* seedPixels = new float[3 * maxSamples * patternSizeSquared];
	
	    for(int i = 0; i < patternSizeSquared; i++) 
      {
		    for(int j = 0; j < maxSamples; j++)
        {
			    csVector3 sample;
			    do {
				    sample = csVector3(2.0f * halton(2, haltonIndex) - 1.0f,
							                  2.0f * halton(3, haltonIndex) - 1.0f, 
							                  halton(5, haltonIndex));
				    haltonIndex++;
				      
			    } while(sample.Norm() > 1.0);

			    seedPixels[(i * maxSamples + j) * 3 + 0] = sample.x;
			    seedPixels[(i * maxSamples + j) * 3 + 1] = sample.y;
			    seedPixels[(i * maxSamples + j) * 3 + 2] = sample.z;
		    }
	    }
                
      csRef<iImage> seedImage;
      seedImage.AttachNew (new csImageMemory (maxSamples, patternSizeSquared, seedPixels, 
        CS_IMGFMT_TRUECOLOR, (const csRGBpixel *)0));

      int flags = CS_TEXTURE_2D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_NPOTS;
      scfString failReason;        
      
      sampleDirectionsTexture = graphics3D->GetTextureManager()->RegisterTexture (seedImage,
        flags, &failReason);
      //sampleDirectionsTexture = graphics3D->GetTextureManager()->CreateTexture (maxSampleCount, 
      //  patternSizeSquared, csimg2D, "rgb16_f", flags, &failReason);

      if (!sampleDirectionsTexture)
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
          "Could not register sample directions texture: %s!", failReason.GetCsString().GetDataSafe());        
      }

      //sampleDirectionsTexture->Blit (0, 0, maxSampleCount, patternSizeSquared, (unsigned char*)seedPixels);

      csRef<iImageIO> imageIO = csQueryRegistry<iImageIO> (objectRegistry);
      csRef<iDataBuffer> data = imageIO->Save (seedImage, "image/png");

      if (!data)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not export ssdo_sampledirs image to format png");
      }

      csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectRegistry);
      vfs->ChDir ("/data");
      if (!vfs->WriteFile ("ssdo_sampledirs", data->GetData(), data->GetSize()))
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not save ssdo sample dirs!");
      }   
          
      delete seedPixels;
    }*/

    void LoadIrradianceEnvironmentMap(iLoader *loader, iGraphics3D *graphics3D)
    {
      csRef<iImage> image;
      int flags = CS_TEXTURE_2D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS;

      irradianceEnvironmentMap = loader->LoadTexture ("/data/blurred_grace.tga", flags, 
        graphics3D->GetTextureManager(), &image);
      if (!irradianceEnvironmentMap)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load blurred_grace.tga texture!");
      }

      // save tex to file (remove, just for debug!)
      csRef<iImageIO> imageIO = csQueryRegistry<iImageIO> (objectRegistry); 
      csRef<iDataBuffer> data = imageIO->Save (image, "image/bmp");
      if (!data)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not export blurred_grace image to format bmp!");
      }

      csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectRegistry);
      vfs->ChDir ("/data");
      if (!vfs->WriteFile ("blurred_grace_debug", data->GetData(), data->GetSize()))
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not save blurred_grace texture!");
      }
    }

    /**
      * Load hdr image in PFM format.
      */
    bool LoadImagePFM(const char* filename, iGraphics3D *graphics3D)
    { 
      // init some variables 
      const int MAX_LINE = 1024;
      char imageformat[ MAX_LINE ];
      float f[1]; 
	 
      // open the file handle
      FILE* infile = fopen( filename, "rb" ); 
 
      if ( infile == NULL ) { 
        printf("Error loading %s !\n", filename); 
        return false; 
      } 
 
      int envMapWidth, envMapHeight;
      // read the header
      fscanf( infile," %s %d %d ", &imageformat, &envMapWidth, &envMapHeight ); 
	 
      // set member variables 
      // assert( width > 0 && height > 0 ); 
      printf("Image format %s Width %d Height %d\n", imageformat, envMapWidth, envMapHeight);
   
      float* const envMapPixels = new float[envMapWidth * envMapHeight * 3];

      // go ahead with the data 
      fscanf( infile,"%f", &f[0] ); 
      fgetc( infile );
 
      float red, green, blue; 
     
      float *p = envMapPixels; 
      //float LMax = 0.f;
      // read the values and store them 
      for ( int j = 0; j < envMapHeight ; j++ ) 
      { 
		    for ( int i = 0; i < envMapWidth ; i++ ) 
        {     
			    fread( f, 4, 1, infile ); 
			    red = f[0]; 
		     
			    fread( f, 4, 1, infile ); 
			    green = f[0]; 
		     
			    fread( f, 4, 1, infile ); 
			    blue = f[0]; 
		     
			    *p++ = red; 
			    *p++ = green; 
			    *p++ = blue; 
	 
			    /*float L = (red + green + blue) / 3.0;
			    if (L > LMax)
			      LMax = L; */
		    } 
      }

      fclose (infile);
      p = NULL;
                    
      csRef<iImage> image;
      image.AttachNew (new csImageMemory (envMapWidth, envMapHeight, envMapPixels, 
        CS_IMGFMT_TRUECOLOR, (const csRGBpixel *)0));

      int flags = CS_TEXTURE_2D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_NPOTS;
      scfString failReason;        
        
      irradianceEnvironmentMap = graphics3D->GetTextureManager()->RegisterTexture (image, 
        flags, &failReason);
      /*irradianceEnvironmentMap = graphics3D->GetTextureManager()->CreateTexture (envMapWidth,
        envMapHeight, csimg2D, "rgb16_f", flags, &failReason);*/

      if (!irradianceEnvironmentMap)
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
          "Could not register irradiance environment map: %s!", failReason.GetCsString().GetDataSafe());
        return false;
      }

      //irradianceEnvironmentMap->Blit (0, 0, envMapWidth, envMapHeight, (unsigned char*)envMapPixels);

      csRef<iImageIO> imageIO = csQueryRegistry<iImageIO> (objectRegistry);
      csRef<iDataBuffer> data = imageIO->Save (image, "image/png");
      if (!data)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not export ssdo_envmap image to format png");	         
      }

      csRef<iVFS> vfs = csQueryRegistry<iVFS> (objectRegistry);
      vfs->ChDir ("/data");
      if (!vfs->WriteFile ("ssdo_envmap", data->GetData(), data->GetSize()))
      {
        csReport(objectRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not save ssdo envmap!");
      }
          
      printf("Loading Envmap finished\n");
      //float revGamma = 1.0 / 2.2;
      delete envMapPixels;

	    return true;
    }

    void DrawFullscreenQuad(iShader *shader)
    {
      // Switches to using orthographic projection. 
      csReversibleTransform oldView = graphics3D->GetWorldToCamera ();
      CS::Math::Matrix4 oldProj = graphics3D->GetProjectionMatrix ();

      graphics3D->SetWorldToCamera (csReversibleTransform ());
      graphics3D->SetProjectionMatrix (CreateOrthoProj (graphics3D));
      
      quadMesh.shader = shader;  
      
      graphics3D->DrawSimpleMesh (quadMesh, 0);

      // Restores old transforms.
      graphics3D->SetWorldToCamera (oldView);
      graphics3D->SetProjectionMatrix (oldProj);
    }

  public:
    /*int patternSize;
    int maxSamples;
    int sampleCount;
    float sampleRadius;
    float depthBias;
    float occlusionStrength;
    float maxOccluderDistance;
    float lightRotationAngle;
    float bounceStrength;

    int blurKernelSize;
    float blurPositionThreshold;
    float blurNormalThreshold;*/

    bool showAmbientOcclusion;
    bool showGlobalIllumination;
    bool applyBlur;
  
    csRef<iShader> globalIllumShader;
    csRef<iShader> horizontalBlurShader;    
    csRef<iShader> verticalBlurShader;
    csRef<iShader> lightCompositionShader;

  private:
    csSimpleRenderMesh quadMesh;
    csVector3 quadVerts[4];

    bool enabled;
    bool isInitialized;    

    GBuffer *gbuffer;
    
    csRef<iTextureHandle> globalIllumBuffer;
    bool isGlobalIllumBufferAttached;
    const char *globalIllumBufferFormat;
    
    csRef<iTextureHandle> intermediateBuffer;
    bool isIntermediateBufferAttached;

    csRef<iTextureHandle> compositionBuffer;
    bool isCompositionBufferAttached;
    const char *compositionBufferFormat;

    csRef<iTextureHandle> randomNormalsTexture;
    csRef<iTextureHandle> sampleDirectionsTexture;
    csRef<iTextureHandle> irradianceEnvironmentMap;
    //iTextureHandle *directLightBuffer;
    
    csRef<csShaderVariable> globalIllumBufferSV;
    csRef<csShaderVariable> intermediateBufferSV;
    csRef<csShaderVariable> compositionBufferSV;
    /*csRef<csShaderVariable> randomNormalsTextureSV;
    csRef<csShaderVariable> sampleDirectionsTextureSV;
    csRef<csShaderVariable> irradianceEnvironmentMapSV;

    csRef<csShaderVariable> patternSizeSV;
    csRef<csShaderVariable> sampleCountSV;
    csRef<csShaderVariable> sampleRadiusSV;
    csRef<csShaderVariable> depthBiasSV;
    csRef<csShaderVariable> occlusionStrengthSV;
    csRef<csShaderVariable> maxOccluderDistanceSV;
    csRef<csShaderVariable> lightRotationAngleSV;
    csRef<csShaderVariable> bounceStrengthSV;
    
    csRef<csShaderVariable> blurKernelSizeSV;
    csRef<csShaderVariable> blurPositionThresholdSV;
    csRef<csShaderVariable> blurNormalThresholdSV;

    csRef<csShaderVariable> showAmbientOcclusionSV;
    csRef<csShaderVariable> showGlobalIlluminationSV;*/

    iGraphics3D *graphics3D;
    csRef<iShaderManager> shaderManager;
    iObjectRegistry *objectRegistry;
    csRef<iShaderVarStringSet> svStringSet;

    const char *reporterMessageID;
  };
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif //__GLOBALILLUM_H__