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

#include "csgfx/shadervarcontext.h"
#include "csutil/array.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/genericresourcecache.h"
#include "csutil/parray.h"
#include "csutil/ref.h"
#include "imap/services.h"
#include "ivideo/shader/shader.h"

struct iGraphics3D;
struct iLoader;
struct iObjectRegistry;
struct iRenderView;
struct iShader;
struct iSyntaxService;
struct iTextureHandle;
struct iView;

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
  class CS_CRYSTALSPACE_EXPORT PostEffectManager :
    public CS::Memory::CustomAllocatedDerived<csRefCount>
  {
  public:
    class Layer;
    struct LayerOptions
    {
      /// Generate mipmaps for this layer
      bool mipmap;
      /// Highest mipmap level to generate
      int maxMipmap;
      /**
       * Reduce output size. Each downsample step reduces the output by half 
       * in each dimensions.
       */
      int downsample;
      /// Prevent texture reuse. Useful for readback or feedback effects.
      bool noTextureReuse;
      
      LayerOptions() : mipmap (false), maxMipmap (-1), downsample (0),
        noTextureReuse (false) {}
      
      bool operator==(const LayerOptions& other) const
      { 
        return (mipmap == other.mipmap)
          && (maxMipmap == other.maxMipmap)
          && (downsample == other.downsample)
          && (noTextureReuse == other.noTextureReuse);
      }
    };

    /// Custom input mapping for a layer
    struct LayerInputMap
    {
      /// Input layer
      Layer* inputLayer;
      /// Name of the shader variable to provide the input layer texture in
      csString textureName;
      /**
       * Name of the shader variable to provide the texture coordinates for the
       * input layer texture in
       */
      csString texcoordName;
      
      LayerInputMap() : inputLayer (0), textureName ("tex diffuse"),
        texcoordName ("texture coordinate 0") {}
    };
    
    /// An effect layer.
    class Layer
    {
    private:
      friend class PostEffectManager;
      csRef<iShader> effectShader;
      int outTextureNum;
      csArray<LayerInputMap> inputs;
      csRef<iShaderVariableContext> svContext;
      LayerOptions options;
      
      Layer()
      {
        svContext.AttachNew (new csShaderVariableContext);
      }
      bool IsInput (const Layer* layer) const;
    public:
      /// Get the shader variables for this layer.
      iShaderVariableContext* GetSVContext() { return svContext; }
      const csArray<LayerInputMap>& GetInputs() { return inputs; }
      int GetOutTextureNum () const { return outTextureNum; }
      
      const LayerOptions& GetOptions() const { return options; }
      void SetOptions (const LayerOptions& opt) { options = opt; }
    };
  
    PostEffectManager ();
    ~PostEffectManager ();

    /// Initialize
    void Initialize (iObjectRegistry* objectReg);
    
    /// Set the texture format for the intermediate textures used.
    void SetIntermediateTargetFormat (const char* textureFmt);

    /// Set up post processing manager for a view
    void SetupView (iView* view);
    void SetupView (uint width, uint height);

    /// Get the texture to render a scene to for post processing.
    iTextureHandle* GetScreenTarget ();

    /**
     * Draw post processing effects after the scene was rendered to
     * the handle returned by GetScreenTarget().
     */
    void DrawPostEffects ();
    
    //@{
    /// Add an effect pass. Uses last added layer as the input
    Layer* AddLayer (iShader* shader);
    Layer* AddLayer (iShader* shader, const LayerOptions& opt);
    //@}
    //@{
    /// Add an effect pass with custom input mappings.
    Layer* AddLayer (iShader* shader, size_t numMaps, const LayerInputMap* maps);
    Layer* AddLayer (iShader* shader, const LayerOptions& opt, size_t numMaps,
      const LayerInputMap* maps);
    //@}
    /// Remove all layers
    void ClearLayers();
    
    /// Get the layer representing the "screen" a scene is rendered to.
    Layer* GetScreenLayer() { return postLayers[0]; }
    
    /// Get the layer that was added last
    Layer* GetLastLayer() { return lastLayer; }
    
    /// Get the output texture of a layer.
    iTextureHandle* GetLayerOutput (const Layer* layer);
    
    /// Set the render target used to ultimatively render to.
    void SetEffectsOutputTarget (iTextureHandle* tex)
    { target = tex; }
    /// Get the render target used to ultimatively render to.
    iTextureHandle* GetEffectsOutputTarget () const
    { return target; }
  private:
    uint frameNum;
    csRef<iGraphics3D> graphics3D;
    csRef<iShaderVarStringSet> svStrings;
    bool keepAllIntermediates;
    csRef<iRenderBuffer> indices;
    csRef<iTextureHandle> target;

    csSimpleRenderMesh fullscreenQuad;
    void SetupScreenQuad ();

    struct Dimensions
    {
      uint x, y;
    };
    /// All the data needed for one target dimension
    struct DimensionData
    {
      Dimensions dim;
      /**
       * Textures which have the same properties are managed
       * in one "bucket"
       */
      struct TexturesBucket
      {
	csRef<iRenderBuffer> vertBuf;
	csRef<iRenderBuffer> texcoordBuf;
	float textureCoordinateX, textureCoordinateY, textureOffsetX, textureOffsetY;
	csRef<csShaderVariable> svPixelSize;
	csRefArray<iTextureHandle> textures;
	
	TexturesBucket() : textureCoordinateX (1), textureCoordinateY (1), 
	  textureOffsetX (0), textureOffsetY (0) { }
      };
      csArray<TexturesBucket> buckets;
      csRefArray<iShaderVariableContext> layerSVs;
      csRefArray<csRenderBufferHolder> buffers;
      
      void AllocatePingpongTextures (PostEffectManager& pfx);
      void UpdateSVContexts (PostEffectManager& pfx);
    
      void SetupScreenQuad (PostEffectManager& pfx);
    };

    struct DimensionCacheSorting
    {
      typedef Dimensions KeyType;

      static bool IsLargerEqual (const DimensionData& b1, 
                                 const DimensionData& b2)
      {
	return (b1.dim.x >= b2.dim.x) || (b1.dim.y >= b2.dim.y);
      }
    
      static bool IsEqual (const DimensionData& b1, 
                           const DimensionData& b2)
      {
	return (b1.dim.x == b2.dim.x) && (b1.dim.y == b2.dim.y);
      }
    
      static bool IsLargerEqual (const DimensionData& b1, 
                                 const Dimensions& b2)
      {
	return (b1.dim.x >= b2.x) || (b1.dim.y >= b2.y);
      }
    
      static bool IsEqual (const DimensionData& b1, 
                           const Dimensions& b2)
      {
	return (b1.dim.x == b2.x) && (b1.dim.y == b2.y);
      }
    
    };
    CS::Utility::GenericResourceCache<DimensionData,
      uint, DimensionCacheSorting, 
      CS::Utility::ResourceCache::ReuseConditionFlagged> dimCache;
    DimensionData* currentDimData;
      
    uint currentWidth, currentHeight;

    bool textureDistributionDirty;
    void UpdateTextureDistribution();
      
    csString textureFmt;
    Layer* lastLayer;
    csPDelArray<Layer> postLayers;
    bool layersDirty;
    void UpdateLayers();
    
    struct BucketsCommon
    {
      LayerOptions options;
      size_t textureNum;
    };
    csArray<BucketsCommon> buckets;
    size_t GetBucketIndex (const LayerOptions& options);
    BucketsCommon& GetBucket (const LayerOptions& options)
    { return buckets[GetBucketIndex (options)]; }
  };
  
  /// Helper to parse post processing effect configurations.
  class CS_CRYSTALSPACE_EXPORT PostEffectLayersParser
  {
    csStringHash xmltokens;
    iObjectRegistry* objReg;
    csRef<iSyntaxService> synldr;
    csRef<iLoader> loader;
    
    typedef csHash<PostEffectManager::Layer*, csString> ParsedLayers;
    typedef csDirtyAccessArray<PostEffectManager::LayerInputMap> InputsArray;
    typedef csHash<csRef<iShader>, csString> ShadersLayers;
    
    bool ParseInputs (iDocumentNode* node, PostEffectManager& effects,
                      ParsedLayers& layers, ShadersLayers& shaders,
                      InputsArray& inputs);
    bool ParseLayer (iDocumentNode* node, PostEffectManager& effects,
                     ParsedLayers& layers, ShadersLayers& shaders);
  public:
    /// Create.
    PostEffectLayersParser (iObjectRegistry* objReg);
  
    /// Parse from a document node,
    bool AddLayersFromDocument (iDocumentNode* node, PostEffectManager& effects);
    /// Parse from file. Document node must be "posteffect"
    bool AddLayersFromFile (const char* filename, PostEffectManager& effects);
  };

}
}

#endif
