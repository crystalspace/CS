/*
    Copyright (C) 2007-2008 by Marten Svanfeldt

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_POSTEFFECTS_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_POSTEFFECTS_H__

#include "csutil/ref.h"
#include "csutil/array.h"
#include "csutil/parray.h"
#include "ivideo/shader/shader.h"

struct iShader;
struct iTextureHandle;
struct iView;
struct iGraphics3D;

namespace CS
{
namespace RenderManager
{
  /**
   * Helper for "simple" post effects usage in render manager.
   * Provides a simple way to render the screen to a texture and then use
   * a number of full screen passes with settable shader to get the desired
   * effect.
   */
  class CS_CRYSTALSPACE_EXPORT PostEffectManager
  {
  public:
    class Layer;
    struct LayerOptions
    {
      bool mipmap;
      int maxMipmap;
      int downsample;
      
      LayerOptions() : mipmap (false), maxMipmap (-1), downsample (0) {}
      
      bool operator==(const LayerOptions& other) const
      { 
        return (mipmap == other.mipmap) && (maxMipmap == other.maxMipmap)
          && (downsample == other.downsample); 
      }
    };

    struct LayerInputMap
    {
      Layer* inputLayer;
      const char* textureName;
      const char* texcoordName;
      
      LayerInputMap() : inputLayer (0), textureName ("tex diffuse"),
        texcoordName ("texture coordinate 0") {}
    };
    
    class Layer
    {
    private:
      friend class PostEffectManager;
      csRef<iShader> effectShader;
      int outTextureNum;
      csArray<LayerInputMap> inputs;
      csRef<iShaderVariableContext> svContext;
      LayerOptions options;
      csRef<csRenderBufferHolder> buffers;
      
      Layer()
      {
        svContext.AttachNew (new csShaderVariableContext);
      }
      bool IsInput (const Layer* layer) const;
    public:
      iShaderVariableContext* GetSVContext() { return svContext; }
    };
  
    PostEffectManager ();
    ~PostEffectManager ();

    void Initialize (iObjectRegistry* objectReg);
    
    void SetIntermediateTargetFormat (const char* textureFmt);

    void SetupView (iView* view);

    iTextureHandle* GetScreenTarget ();

    void DrawPostEffects ();
    
    Layer* AddLayer (iShader* shader);
    Layer* AddLayer (iShader* shader, const LayerOptions& opt);
    Layer* AddLayer (iShader* shader, size_t numMaps, const LayerInputMap* maps);
    Layer* AddLayer (iShader* shader, const LayerOptions& opt, size_t numMaps,
      const LayerInputMap* maps);
    
    Layer* GetScreenLayer() { return postLayers[0]; }
  private:
    void SetupScreenQuad (unsigned int width, unsigned int height);

    void AllocatePingpongTextures ();
    
    csRef<iGraphics3D> graphics3D;
    csRef<iShaderVarStringSet> svStrings;
    bool keepAllIntermediates;
    csRef<iRenderBuffer> indices;

    csSimpleRenderMesh fullscreenQuad;

    unsigned int currentWidth, currentHeight;
    struct TexturesBucket
    {
      LayerOptions options;
      csRef<iRenderBuffer> vertBuf;
      csRef<iRenderBuffer> texcoordBuf;
      float textureCoordinateX, textureCoordinateY, textureOffsetX, textureOffsetY;
      csRef<csShaderVariable> svPixelSize;
      csRefArray<iTextureHandle> textures;
      
      TexturesBucket() : textureCoordinateX (1), textureCoordinateY (1), 
        textureOffsetX (0), textureOffsetY (0) { }
    };
    csArray<TexturesBucket> buckets;

    const char* textureFmt;
    Layer* lastLayer;
    csPDelArray<Layer> postLayers;
    
    bool textureDistributionDirty;
    void UpdateTextureDistribution();
    
    void UpdateSVContexts ();
  
    size_t GetBucketIndex (const LayerOptions& options);
    TexturesBucket& GetBucket (const LayerOptions& options)
    { return buckets[GetBucketIndex (options)]; }
};

}
}

#endif
