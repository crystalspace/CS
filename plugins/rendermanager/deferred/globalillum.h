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
      gbuffer (nullptr), enabled (true), isInitialized (false) 
    {
    }
    
    ~csGlobalIllumRenderer() 
    {
      CS_ASSERT (!isGlobalIllumBufferAttached && !isCompositionBufferAttached); 
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
      
      patternSize = cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.SamplingPatternSize", 4);
      maxSamples= cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.MaxSamples", 128);
      sampleCount = cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.SampleCount", 16);
      sampleRadius = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.SampleRadius", 5.0f);
      depthBias = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.DepthBias", 1.0f);
      occlusionStrength = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.OcclusionStrength", 4.0f);
      maxOccluderDistance = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.MaxOccluderDistance", 50.0f);
      lightRotationAngle = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.LightRotationAngle", 0.0f);
      bounceStrength = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.BounceStrength", 1.0f);

      blurKernelSize = cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.BlurKernelSize", 3);
      blurPositionThreshold = cfg->GetFloat (
        "RenderManager.Deferred.GlobalIllum.SSDO.BlurPositionThreshold", 1.0f);
      blurNormalThreshold = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.BlurNormalThreshold", 0.5f);
      
      csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
      csRef<iShaderManager> shaderManager = csQueryRegistry<iShaderManager> (objRegistry);     

      globalIllumShader = loader->LoadShader ("/shader/deferred/globalillum.xml");
      if (!globalIllumShader)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load deferred_globalillum shader");
        enabled = false;
        return false;
      }

      lightCompositionShader = loader->LoadShader ("/shader/deferred/light_composition.xml");
      if (!lightCompositionShader)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load deferred_light_composition shader");
        enabled = false;
        return false;
      }

      iShaderVarStringSet *svStringSet = shaderManager->GetSVNameStringset ();
      
      globalIllumBufferSV = lightCompositionShader->GetVariableAdd (
        svStringSet->Request ("tex global illumination"));
      randomNormalsTextureSV = globalIllumShader->GetVariableAdd (
        svStringSet->Request ("tex random normals"));
      sampleDirectionsTextureSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("tex sample directions"));
      irradianceEnvironmentMapSV = globalIllumShader->GetVariableAdd (
        svStringSet->Request ("tex globalillum envmap"));

      patternSizeSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("pattern size"));
      sampleCountSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("sample count"));
      sampleRadiusSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("sample radius"));
      depthBiasSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("depth bias"));
      occlusionStrengthSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("occlusion strength"));
      maxOccluderDistanceSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("max occluder distance"));
      lightRotationAngleSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("light rotation angle"));
      bounceStrengthSV = globalIllumShader->GetVariableAdd (
          svStringSet->Request ("bounce strength"));
      
      blurKernelSizeSV = lightCompositionShader->GetVariableAdd (
          svStringSet->Request ("ssao blur kernelsize"));
      blurPositionThresholdSV = lightCompositionShader->GetVariableAdd (
          svStringSet->Request ("ssao blur position threshold"));
      blurNormalThresholdSV = lightCompositionShader->GetVariableAdd (
          svStringSet->Request ("ssao blur normal threshold"));
      
      LoadRandomNormalsTexture(loader, graphics3D);
      LoadIrradianceEnvironmentMap(loader, graphics3D);
      //GenerateSampleDirections (graphics3D);

      CreateFullscreenQuad();

      isInitialized = true;
      return true;
    }

    void UpdateShaderVars()
    {
      if (!enabled)
        return;

      globalIllumBufferSV->SetValue (globalIllumBuffer);
      randomNormalsTextureSV->SetValue (randomNormalsTexture);
      sampleDirectionsTextureSV->SetValue (sampleDirectionsTexture);
      irradianceEnvironmentMapSV->SetValue (irradianceEnvironmentMap);

      patternSizeSV->SetValue (patternSize);
      sampleCountSV->SetValue (sampleCount <= maxSamples ? sampleCount : maxSamples);
      sampleRadiusSV->SetValue (sampleRadius);
      depthBiasSV->SetValue (depthBias);
      occlusionStrengthSV->SetValue (occlusionStrength);
      maxOccluderDistanceSV->SetValue (maxOccluderDistance);
      lightRotationAngleSV->SetValue (lightRotationAngle);
      bounceStrengthSV->SetValue (bounceStrength);
      
      blurKernelSizeSV->SetValue (blurKernelSize);
      blurPositionThresholdSV->SetValue (blurPositionThreshold);
      blurNormalThresholdSV->SetValue (blurNormalThreshold);
    }

    void RenderGlobalIllum()
    {
      if (!enabled)
        return;

      // Draw directional occlusion + indirect light
      AttachGlobalIllumBuffer();
      {
        DrawFullscreenQuad (globalIllumShader);
      }
      DetachGlobalIllumBuffer();

      // Apply edge-aware blur and combine accum buffer with global illum buffer
      AttachCompositionBuffer();
      {
        DrawFullscreenQuad (lightCompositionShader);

        //graphics3D->FinishDraw();
      }
      DetachCompositionBuffer();
    }

    bool AttachGlobalIllumBuffer()
    {
      if (!enabled || isGlobalIllumBufferAttached)
        return false;

      if (!graphics3D->SetRenderTarget (globalIllumBuffer, false, 0, rtaColor0))
          return false;

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
      globalIllumBufferFormat = cfg->GetStr ("RenderManager.Deferred.GlobalIllum.GlobalIllumBufferFormat", "rgba16_f");
      compositionBufferFormat = cfg->GetStr ("RenderManager.Deferred.GlobalIllum.CompositionBufferFormat", 
        "rgb16_f");

      const int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NPOTS;
      scfString errString;
      iGraphics2D *g2D = graphics3D->GetDriver2D();

      if (!globalIllumBuffer)
      {
        globalIllumBuffer = graphics3D->GetTextureManager()->CreateTexture (g2D->GetWidth(), g2D->GetHeight(), 
          csimg2D, globalIllumBufferFormat, flags, &errString);
      }
      if (!globalIllumBuffer)
      {
        csReport (objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
          "Could not create global illumination buffer! %s", errString.GetCsString().GetDataSafe());
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
            "Failed to attach ambient buffer to the device!");
        return false;
      }

      if (!graphics3D->ValidateRenderTargets())
      {
        DetachGlobalIllumBuffer();
        csReport(objectRegistry, CS_REPORTER_SEVERITY_ERROR, reporterMessageID, 
            "Ambient buffer format is not supported by the device!");
        return false;
      }
      DetachGlobalIllumBuffer();

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

      randomNormalsTexture = loader->LoadTexture ("/data/random_normals.dds", flags, 
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
    void GenerateSampleDirections(iGraphics3D *graphics3D)
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
      /*sampleDirectionsTexture = graphics3D->GetTextureManager()->CreateTexture (maxSampleCount, 
        patternSizeSquared, csimg2D, "rgb16_f", flags, &failReason);*/

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
	  
	    /*glGenTextures (1, &sampleDirectionsTextureId);
	    glBindTexture (GL_TEXTURE_2D, sampleDirectionsTextureId);
	    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB32F_ARB, maxSampleCount, patternSizeSquared, 0, 
        GL_RGB, GL_FLOAT, seedPixels); 
	    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);*/
          
      delete seedPixels;
    }

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
    int patternSize;
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
    float blurNormalThreshold;

  private:
    csRef<iShader> globalIllumShader;
    csRef<iShader> lightCompositionShader;

    csSimpleRenderMesh quadMesh;
    csVector3 quadVerts[4];

    bool enabled;
    bool isInitialized;

    GBuffer *gbuffer;
    
    csRef<iTextureHandle> globalIllumBuffer;
    bool isGlobalIllumBufferAttached;
    const char *globalIllumBufferFormat;
    
    csRef<iTextureHandle> compositionBuffer;
    bool isCompositionBufferAttached;
    const char *compositionBufferFormat;

    csRef<iTextureHandle> randomNormalsTexture;
    csRef<iTextureHandle> sampleDirectionsTexture;
    csRef<iTextureHandle> irradianceEnvironmentMap;
    //iTextureHandle *directLightBuffer;
    
    csRef<csShaderVariable> globalIllumBufferSV;
    csRef<csShaderVariable> randomNormalsTextureSV;
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

    iGraphics3D *graphics3D;
    iObjectRegistry *objectRegistry;

    const char *reporterMessageID;
  };
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif //__GLOBALILLUM_H__