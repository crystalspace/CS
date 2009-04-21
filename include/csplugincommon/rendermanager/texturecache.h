/*
    Copyright (C) 2007 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_TEXTURECACHE_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_TEXTURECACHE_H__

/**\file
 * Texture cache
 */

#include "igraphic/image.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "csutil/genericresourcecache.h"

struct iTextureHandle;

namespace CS
{
namespace RenderManager
{
  namespace Implementation
  {
    /**
     * Helper for generic resource cache storing texture handles.
     */
    struct TextureSizeConstraint
    {
      struct KeyType
      {
        int w, h;
      };

      static bool IsLargerEqual (const csRef<iTextureHandle>& t1, 
                                 const csRef<iTextureHandle>& t2)
      {
        int tw1, th1, tw2, th2;
        t1->GetRendererDimensions (tw1, th1);
        t2->GetRendererDimensions (tw2, th2);
        
        return ((tw1 >= tw2) || (th1 >= th2));
      }
    
      static bool IsEqual (const csRef<iTextureHandle>& t1, 
                           const csRef<iTextureHandle>& t2)
      {
        int tw1, th1, tw2, th2;
        t1->GetRendererDimensions (tw1, th1);
        t2->GetRendererDimensions (tw2, th2);
        
        if ((tw1 == tw2) && (th1 == th2)) return true;
        return false;
      }
    
      static bool IsLargerEqual (const csRef<iTextureHandle>& t1, 
                                 const KeyType& t2)
      {
        int tw1, th1;
        t1->GetRendererDimensions (tw1, th1);
        
        if ((tw1 >= t2.w) || (th1 >= t2.h)) return true;
        return false;
      }
    
      static bool IsEqual (const csRef<iTextureHandle>& t1, 
                           const KeyType& t2)
      {
        int tw1, th1;
        t1->GetRendererDimensions (tw1, th1);
        
        if ((tw1 == t2.w) && (th1 == t2.h)) return true;
        return false;
      }
    
    };
  } // namespace Implementation

  /**
   * Cache for caching precreated textures.
   * When a texture is needed somewhere, the texture cache will either return
   * an existing textures which can be used, if available, or create a new
   * texture. Texture format, flags and size requirements can be user-controlled.
   * Before first use, SetG3D() has to be called.
   *
   * The template arguments \a ReuseCondition controls when a texture can be
   * reused, \a PurgeCondition controls when it's purged (ie completely freed).
   * The requirements are the same as for GenericResourceCache.
   */
  template<
    typename ReuseCondition = CS::Utility::ResourceCache::ReuseConditionAfterTime<csTicks>,
    typename PurgeCondition = CS::Utility::ResourceCache::PurgeConditionAfterTime<csTicks> >
  class TextureCacheT
  {    
  public:
    /// Options for querying textures
    enum
    {
      /**
       * The sizes of returned textures should exactly fit the requested size.
       * If not set, a larger texture may be returned.
       */
      tcacheExactSizeMatch = 1,
      /**
       * The textures created should have power of two dimensions.
       * This flag is stronger than #tcacheExactSizeMatch: returned textures
       * will always have power of two dimensions, even though it may mean
       * non-exact sizes.
       */
      tcachePowerOfTwo = 2
    };

    /**
     * Construct texture cache.
     * \param imgtype Image type of textures to create.
     * \param format Format of textures to create.
     * \param textureFlags Flags of textures to create.
     * \param texClass Class of textures to create.
     * \param options Options for cache behaviour when querying textures.
     *   Can be #tcacheExactSizeMatch, #tcachePowerOfTwo.
     * \param reuse Options for the reuse condition to use.
     * \param purge Options for the purge condition to use.
     * \sa iTextureManager::CreateTexture();
     */
    TextureCacheT (csImageType imgtype, const char* format, int textureFlags, 
      const char* texClass, uint options,
      const ReuseCondition& reuse =
        CS::Utility::ResourceCache::ReuseConditionAfterTime<uint> (),
      const PurgeCondition& purge =
        CS::Utility::ResourceCache::PurgeConditionAfterTime<uint> (10000)) : g3d (0), 
      backend (reuse, purge),
      imgtype (imgtype),
      format (format), textureFlags (textureFlags), texClass (texClass), 
      options (options) 
    {
      backend.agedPurgeInterval = 5000;
    }
    
    /// Set iGraphics3D to use
    void SetG3D (iGraphics3D* g3d) 
    {
      this->g3d = g3d; 
    }
    
    /**
     * Remove all textures in the cache
     */
    void Clear ()
    {
      backend.Clear ();
    }

    /**
     * Do per-frame house keeping - \b MUST be called every frame/
     * RenderView() execution.
     */
    void AdvanceFrame (csTicks currentTime)
    {
      backend.AdvanceTime (currentTime);
    }

    /**
     * Get a texture with size that is at least \a width x \a height
     * (or exactly that size if the #tcacheExactSizeMatch option is set).
     * The real size of the texture acquired is stored in \a real_w 
     * and \a real_h.
     */
    iTextureHandle* QueryUnusedTexture (int width, int height, 
                                        int& real_w, int& real_h)
    {
      Implementation::TextureSizeConstraint::KeyType queryKey;
      queryKey.w = width; queryKey.h = height;
      csRef<iTextureHandle>* tex = backend.Query (queryKey, 
        (options & tcacheExactSizeMatch));

      if (tex != 0)
      {
        (*tex)->GetRendererDimensions (real_w, real_h);
        return *tex;
      }

      if (options & tcachePowerOfTwo)
      {
        width = csFindNearestPowerOf2 (width);
        height = csFindNearestPowerOf2 (height);
      }
      real_w = width;
      real_h = height;
      
      CS_ASSERT_MSG("SetG3D () not called", g3d);
      csRef<iTextureHandle> newTex (
        g3d->GetTextureManager()->CreateTexture (
        width, height, imgtype, format, textureFlags));
      newTex->SetTextureClass (texClass);

      backend.AddActive (newTex);

      return newTex;
    }

    /**
     * Get a texture with size that is at least \a width x \a height
     * (or exactly that size if the #tcacheExactSizeMatch option is set).
     */
    iTextureHandle* QueryUnusedTexture (int width, int height)
    {
      int dummyW, dummyH;
      return QueryUnusedTexture (width, height, dummyW, dummyH);
    }

    /// Return format of textures created by a cache
    const char* GetFormat() const { return format; }
    /// Return flags of textures created by a cache
    int GetFlags() const { return textureFlags; }
    /// Return class of textures created by a cache
    const char* GetClass() const { return texClass; }
    
    /**
     * Set format of textures created by a cache.
     * \remark Will only affect newly created textures. Cache will be cleared.
     */
    void SetFormat (const char* format)
    {
      this->format = format;
      Clear();
    }
    /**
     * Set flags of textures created by a cache.
     * \remark Will only affect newly created textures. Cache will be cleared.
     */
    void SetFlags (int flags)
    {
      textureFlags = flags;
      Clear();
    }
    /**
     * Set class of textures created by a cache.
     * \remark Will only affect newly created textures. Cache will be cleared.
     */
    void SetClass (const char* texClass)
    {
      this->texClass = texClass;
      Clear();
    }
  private:
    csRef<iGraphics3D> g3d;

    CS::Utility::GenericResourceCache<csRef<iTextureHandle>,
      csTicks, Implementation::TextureSizeConstraint,
      ReuseCondition, PurgeCondition> backend;

    csImageType imgtype;
    csString format;
    int textureFlags;
    csString texClass;
    uint options;
  };
  
  /// Texture cache using time-based reuse and purge conditions.
  typedef TextureCacheT<> TextureCache;
} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_TEXTURECACHE_H__
