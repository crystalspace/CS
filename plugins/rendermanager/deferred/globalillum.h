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
   * Utility class for encapsulating functionality of global illumination 
   * techniques used in the deferred render manager, such as ambient occlusion
   * and indirect lighting.
   */
  class csDeferredGlobalIllumination
  {
  public:

    csDeferredGlobalIllumination()
    {
      reporterMessageID = "crystalspace.rendermanager.deferred.lightrender.globalillum";
    }
    
    bool Initialize(iObjectRegistry *objRegistry)
    {
      objectRegistry = objRegistry;
      csString shaderPath ("/shader/deferred/globalillum.xml");
      csString shaderName ("deferred_globalillum");

      csConfigAccess cfg (objRegistry);
      patternSize = cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.SamplingPatternSize", 4);
      maxSamples= cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.MaxSamples", 128);
      sampleCount = cfg->GetInt ("RenderManager.Deferred.GlobalIllum.SSDO.SampleCount", 16);
      sampleRadius = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.SampleRadius", 5.0f);
      depthBias = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.DepthBias", 1.0f);
      occlusionStrength = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.OcclusionStrength", 4.0f);
      maxOccluderDistance = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.MaxOccluderDistance", 50.0f);
      lightRotationAngle = cfg->GetFloat ("RenderManager.Deferred.GlobalIllum.SSDO.LightRotationAngle", 0.0f);
      
      csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
      csRef<iShaderManager> shaderManager = csQueryRegistry<iShaderManager> (objRegistry);
      csRef<iGraphics3D> graphics3D = csQueryRegistry<iGraphics3D> (objRegistry);      

      shader = loader->LoadShader (shaderPath);
      if (!shader)
      {
        csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING, reporterMessageID, 
          "Could not load " + shaderName + " shader");
        return false;
      }      
      
      iShaderVarStringSet *svStringSet = shaderManager->GetSVNameStringset ();
      
      randomNormalsTextureSV = shader->GetVariableAdd (
        svStringSet->Request ("tex random normals"));
      sampleDirectionsTextureSV = shader->GetVariableAdd (
          svStringSet->Request ("tex sample directions"));
      irradianceEnvironmentMapSV = shader->GetVariableAdd (
        svStringSet->Request ("tex globalillum envmap"));
      patternSizeSV = shader->GetVariableAdd (
          svStringSet->Request ("pattern size"));
      sampleCountSV = shader->GetVariableAdd (
          svStringSet->Request ("sample count"));
      sampleRadiusSV = shader->GetVariableAdd (
          svStringSet->Request ("sample radius"));
      depthBiasSV = shader->GetVariableAdd (
          svStringSet->Request ("depth bias"));
      occlusionStrengthSV = shader->GetVariableAdd (
          svStringSet->Request ("occlusion strength"));
      maxOccluderDistanceSV = shader->GetVariableAdd (
          svStringSet->Request ("max occluder distance"));
      lightRotationAngleSV = shader->GetVariableAdd (
          svStringSet->Request ("light rotation angle"));
      viewProjectionInvSV = shader->GetVariableAdd (
          svStringSet->Request ("viewproj transform inverse"));
      
      LoadRandomNormalsTexture(loader, graphics3D);
      LoadIrradianceEnvironmentMap(loader, graphics3D);
      //GenerateSampleDirections (graphics3D);

      return true;
    }

    void UpdateShaderVars(iGraphics3D *graphics3D )
    {
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

      CS::Math::Matrix4 invViewProj = (graphics3D->GetWorldToCamera() * 
        graphics3D->GetProjectionMatrix()).GetInverse();
      viewProjectionInvSV->SetValue (invViewProj);
    }    

    private:

      void LoadRandomNormalsTexture(iLoader *loader, iGraphics3D *graphics3D)
      {
        csRef<iImage> image;
        int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS;

        randomNormalsTexture = loader->LoadTexture ("/data/random_normals.png", flags, 
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

    public:
      csRef<iShader> shader;

      int patternSize;
      int maxSamples;
      int sampleCount;
      float sampleRadius;
      float depthBias;
      float occlusionStrength;
      float maxOccluderDistance;
      float lightRotationAngle;

    private:
      csRef<iTextureHandle> randomNormalsTexture;
      csRef<iTextureHandle> sampleDirectionsTexture;
      csRef<iTextureHandle> irradianceEnvironmentMap;  
    
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
      csRef<csShaderVariable> viewProjectionInvSV;

      iObjectRegistry *objectRegistry;

      const char *reporterMessageID;
  };
}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif //__GLOBALILLUM_H__