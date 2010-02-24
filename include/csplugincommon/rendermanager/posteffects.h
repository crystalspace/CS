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

/**\file
 * Post processing effects manager
 */

#include "csgfx/shadervarcontext.h"
#include "csplugincommon/rendermanager/rendertree.h"
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
   * Helper for post processing effects usage in render managers.
   * Provides a simple way to render the screen to a texture and then use
   * a number of full screen passes with settable shader to get the desired
   * effect.
   *
   * To use post processing effects, rendering of the main context has to be
   * redirected to a target managed by the post processing manager. After
   * drawing the scene another call applies the effects.
   * Example:
   * \code
   * // Set up post processing manager for the given view
   * postEffects.SetupView (renderView);
   *
   * // Set up start context,
   * RenderTreeType::ContextNode* startContext = renderTree.CreateContext (renderView);
   * // render to a target for later postprocessing
   * startContext->renderTargets[rtaColor0].texHandle = postEffects.GetScreenTarget ();
   *
   * // ... draw stuff ...
   *
   * // Apply post processing effects
   * postEffects.DrawPostEffects ();
   * \endcode
   *
   * Post processing setups are a graph of effects (with nodes called "layers"
   * for historic reasons). Each node has one output and multiple inputs.
   * Inputs can be the output of another node or the render of the current
   * scene.
   *
   * Post processing setups are usually read from an external source
   * by using PostEffectLayersParser.
   * Example:
   * \code
   * const char* effectsFile = cfg->GetStr ("MyRenderManager.Effects", 0);
   * if (effectsFile)
   * {
   *   PostEffectLayersParser postEffectsParser (objectReg);
   *   postEffectsParser.AddLayersFromFile (effectsFile, postEffects);
   * }
   * \endcode
   * A setup is not required to use a post processing manager. If no setup is
   * provided the scene will just be drawn to the screen.
   *
   * Post processing managers can be "chained" which means the output of a
   * manager serves as the input of the following, "chained" post processing
   * manager instead of the normal rendered scene. Notably, using HDR exposure
   * effects involved chaining an post processing manager for HDR to a
   * another post processing manager. Example:
   * \code
   * hdr.Setup (...);
   * // Chain HDR post processing effects to normal effects
   * postEffects.SetChainedOutput (hdr.GetHDRPostEffects());
   * // Just use postEffects as usual, chained effects are applied transparently
   * \endcode
   */
  class CS_CRYSTALSPACE_EXPORT PostEffectManager :
    public CS::Memory::CustomAllocatedDerived<csRefCount>
  {
    struct DimensionData;
  public:
    class Layer;
    /**
     * Options for a postprocessing layer
     */
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
      /**
       * Manually provide a texture to render on.
       * Means mipmap, maxMipmap, downsample and noTextureReuse are ignored.
       */
      csRef<iTextureHandle> manualTarget;
      /// If not empty render to this rectangle of the target texture
      csRect targetRect;
      /**
       * If given renders onto the specified layer as well.
       * This means all other options except targetRect are ignored.
       */
      Layer* renderOn;
      /**
       * This layer will later be read back. Sets the #CSDRAW_READBACK draw
       * flag.
       */
      bool readback;
      
      LayerOptions() : mipmap (false), maxMipmap (-1), downsample (0),
        noTextureReuse (false), renderOn (0), readback (false) {}
      
      bool operator==(const LayerOptions& other) const
      { 
        return (mipmap == other.mipmap)
          && (maxMipmap == other.maxMipmap)
          && (downsample == other.downsample)
          && (noTextureReuse == other.noTextureReuse)
          && (manualTarget == other.manualTarget)
          && (targetRect == other.targetRect)
          && (renderOn == other.renderOn)
          && (readback == other.readback);
      }
    };

    /// Custom input mapping for a layer
    struct LayerInputMap
    {
      /**
       * Shader variable for manually specifying an inout texture.
       * Takes precedence over inputLayer and textureName if given.
       */
      csRef<csShaderVariable> manualInput;
      /// Input layer
      Layer* inputLayer;
      /// Name of the shader variable to provide the input layer texture in
      csString textureName;
      /**
       * Name of the shader variable to provide the texture coordinates for the
       * input layer texture in
       */
      csString texcoordName;
      /**
       * If not empty the SV with that name receives the 'pixel size'
       * (values to add to X/Y to get next input pixel) for this input.
       */
      csString inputPixelSizeName;
      /**
       * If not empty specifies the rectangle of the input texture, in pixels,
       * the be used as input for the layer.
       */
      csRect sourceRect;
      
      LayerInputMap() : inputLayer (0), textureName ("tex diffuse"),
        texcoordName ("texture coordinate 0") {}
    };
    
    /// An effect layer.
    class Layer
    {
    private:
      friend class PostEffectManager;
      friend struct DimensionData;
      
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
      iShaderVariableContext* GetSVContext() const { return svContext; }
      /// Get inputs to this layer
      const csArray<LayerInputMap>& GetInputs() const { return inputs; }
      
      /// Get layer options
      const LayerOptions& GetOptions() const { return options; }
      /// Set layer options
      void SetOptions (const LayerOptions& opt) { options = opt; }

      /// Get layer shader
      void SetShader (iShader* shader) { effectShader = shader; }
      /// Set layer shader
      iShader* GetShader () const { return effectShader; }
      /// @@@ Document me?
      int GetOutTextureNum () const { return outTextureNum; }
    };
  
    PostEffectManager ();
    ~PostEffectManager ();

    /// Initialize
    void Initialize (iObjectRegistry* objectReg);
    
    /// Set the texture format for the intermediate textures used.
    void SetIntermediateTargetFormat (const char* textureFmt);
    /// Get the texture format for the intermediate textures used.
    const char* GetIntermediateTargetFormat ();

    //@{
    /**
     * Set up post processing manager for a view.
     * \returns Whether the manager has changed. If \c true some values,
     *   such as the screen texture, must be reobtained from the manager.
     *   \a perspectiveFixup returns a matrix that should be applied
     *   after the normal perspective matrix (this is needed as the
     *   screen texture may be larger than the desired viewport and thus
     *   the projection must be corrected for that).
     */
    bool SetupView (iView* view, CS::Math::Matrix4& perspectiveFixup);
    bool SetupView (uint width, uint height,
      CS::Math::Matrix4& perspectiveFixup);
    //@}

    /**
     * Discard (and thus cause recreation of) all intermediate textures.
     */
    void ClearIntermediates();

    /// Get the texture to render a scene to for post processing.
    iTextureHandle* GetScreenTarget ();

    /**
     * Draw post processing effects after the scene was rendered to
     * the handle returned by GetScreenTarget().
     */
    void DrawPostEffects (RenderTreeBase& renderTree);
    
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
    /// Remove a layer
    bool RemoveLayer (Layer* layer);
    /// Remove all layers
    void ClearLayers();
    
    /// Get the layer representing the "screen" a scene is rendered to.
    Layer* GetScreenLayer() { return postLayers[0]; }
    
    /// Get the layer that was added last
    Layer* GetLastLayer() { return lastLayer; }
    
    /// Get the output texture of a layer.
    iTextureHandle* GetLayerOutput (const Layer* layer);
    
    /**
     * Get SV context used for rendering.
     */
    void GetLayerRenderSVs (const Layer* layer, csShaderVariableStack& svStack) const;
    
    /**
     * Set the render target used to ultimatively render to.
     * Setting this on a post effects manager in a chain effectively sets
     * the output target of the last chain member.
     */
    void SetEffectsOutputTarget (iTextureHandle* tex)
    {
      if (chainedEffects)
	chainedEffects->SetEffectsOutputTarget (tex);
      else
	target = tex;
    }
    /// Get the render target used to ultimatively render to.
    iTextureHandle* GetEffectsOutputTarget () const
    {
      if (chainedEffects) return chainedEffects->GetEffectsOutputTarget ();
      return target;
    }

    //@{
    /**
     * Chain another post effects manager to the this one. The output of
     * this manager is automatically used as input to the next.
     */
    void SetChainedOutput (PostEffectManager* nextEffects);
    void SetChainedOutput (PostEffectManager& nextEffects)
    { SetChainedOutput (&nextEffects); }
    //@}
    
    /**
     * Returns whether the screen space is flipped in Y direction. This usually
     * happens when rendering to a texture due post effects.
     */
    bool ScreenSpaceYFlipped ();
  private:
    uint frameNum;
    csRef<iGraphics3D> graphics3D;
    csRef<iShaderVarStringSet> svStrings;
    bool keepAllIntermediates;
    csRef<iRenderBuffer> indices;
    csRef<iTextureHandle> target;
    PostEffectManager* chainedEffects;
    uint dbgIntermediateTextures;

    void SetupScreenQuad ();
    const Layer& GetRealOutputLayer (const Layer& layer) const
    { 
      return layer.options.renderOn 
        ? GetRealOutputLayer (*(layer.options.renderOn))
        : layer;
    }

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
        /// Textures in this bucket
	csRefArray<iTextureHandle> textures;
	/**
	 * Maximum X/Y coords (normalized for 2D textures, unnormalized for
	 * RECT textures)
	 */
	float texMaxX, texMaxY;
	
	TexturesBucket() : texMaxX (1), texMaxY (1) { }
      };
      csArray<TexturesBucket> buckets;
      
      struct LayerRenderInfo
      {
        /// 'Pixel size' (values to add to X/Y to get next input pixel)
	csRef<csShaderVariable> svPixelSize;
	/// Input vertices for layer
	csRef<iRenderBuffer> vertBuf;
	/// Shader vars
	csRef<iShaderVariableContext> layerSVs;
	/// Render buffers
	csRef<csRenderBufferHolder> buffers;
	/// Render mesh for layer
        csSimpleRenderMesh fullscreenQuad;
      };
      /// Render information for all layers
      csArray<LayerRenderInfo> layerRenderInfos;

      void AllocatePingpongTextures (PostEffectManager& pfx);
      void UpdateSVContexts (PostEffectManager& pfx);
    
      void SetupRenderInfo (PostEffectManager& pfx);
    protected:
      csPtr<iRenderBuffer> ComputeTexCoords (iTextureHandle* tex,
        const csRect& rect, const csRect& targetRect,
        float& pixSizeX, float& pixSizeY);
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
  
  // @@@ TODO: give a simple example
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
    /// Parse from XML file. Document root node must be "posteffect"
    bool AddLayersFromFile (const char* filename, PostEffectManager& effects);
  };

}
}

#endif
