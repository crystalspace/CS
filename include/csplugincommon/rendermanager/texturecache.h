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
   * Generic cache for caching precreated textures
   */
  template<
    typename ReuseCondition = CS::Utility::ResourceCache::ReuseConditionAfterTime<csTicks>,
    typename PurgeCondition = CS::Utility::ResourceCache::PurgeConditionAfterTime<csTicks> >
  class TextureCacheT
  {    
  public:
    enum
    {
      tcacheExactSizeMatch = 1,
      tcachePowerOfTwo = 2
    };

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

    void AdvanceFrame (csTicks currentTime)
    {
      backend.AdvanceTime (currentTime);
    }

    /**
     * Get a texture with size that is at least width x height.
     * The real size of the texture acquired is stored in real_w/real_h.
     */
    iTextureHandle* QueryUnusedTexture (int width, int height, csTicks lifetime,
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
     * Get a texture with size that is at least width x height.     
     */
    iTextureHandle* QueryUnusedTexture (int width, int height, csTicks lifetime)
    {
      int dummyW, dummyH;
      return QueryUnusedTexture (width, height, lifetime, dummyW, dummyH);
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
  
  typedef TextureCacheT<> TextureCache;
} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_TEXTURECACHE_H__
