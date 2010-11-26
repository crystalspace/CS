/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005-2008 by Frank Richter

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

#ifndef __CS_GL_R2T_EXT_FB_O_H__
#define __CS_GL_R2T_EXT_FB_O_H__

#include "csutil/fixedsizeallocator.h"
#include "csutil/genericresourcecache.h"
#include "csutil/list.h"
#include "csutil/refcount.h"

#include "gl_r2t_framebuf.h"

CS_PLUGIN_NAMESPACE_BEGIN(gl3d)
{
  struct Dimensions
  {
    int width, height;

    Dimensions () {}
    Dimensions (int w, int h) : width (w), height (h) {}
    Dimensions (const Dimensions& other) : width (other.width), 
      height (other.height) {}
  };

  class RenderBufferWrapper : 
    public CS::Memory::CustomAllocatedDerived<
	      CS::Utility::FastRefCount <RenderBufferWrapper> >
  {
    // @@@ Would be really nice to not have to store ext with every instance
    csGLExtensionManager* ext;
    Dimensions bufSize;
    GLuint buffer;
  public:
    RenderBufferWrapper (csGLExtensionManager* ext) : ext (ext), buffer (0) { }
    RenderBufferWrapper (const RenderBufferWrapper& other) : ext (other.ext), buffer (0) 
    {
      // Only allow copying of un-setup wrappers
      CS_ASSERT (other.buffer == 0);
    }
    ~RenderBufferWrapper() { Free(); }

    void Setup (int w, int h, GLenum storage);
    void Free();

    int GetWidth() const { return bufSize.width; }
    int GetHeight() const { return bufSize.height; }
    GLuint GetBuffer() const { return buffer; }
  };

  template<class TextureKeeper = csWeakRef<iTextureHandle> >
  struct R2TAttachmentGroup
  {
    typedef csGLRender2TextureBackend::RTAttachment<TextureKeeper> RTA;
  protected:
    template<class _TextureKeeper> friend struct R2TAttachmentGroup;
  
    uint hash;
  #ifdef CS_DEBUG
    bool hashComputed;
  #endif
    RTA attachments[rtaNumAttachments];
    
    static unsigned int HashCompute (char const* s, size_t n, uint startHash)
    {
      unsigned int h = startHash;
      const char* end = s + n;
      for(const char* c = s; c != end; ++c)
	h = ((h << 5) + h) + *c;
  
      return h;
    }
  
  public:
    R2TAttachmentGroup()
    {
    #ifdef CS_DEBUG
      hashComputed = false;
    #endif
    }

    RTA& GetAttachment (csRenderTargetAttachment a)
    {
      CS_ASSERT((a >= 0) && (a < rtaNumAttachments));
    #ifdef CS_DEBUG
      hashComputed = false;
    #endif
      return attachments[a];
    }
    const RTA& GetAttachment (csRenderTargetAttachment a) const
    {
      CS_ASSERT((a >= 0) && (a < rtaNumAttachments));
      return attachments[a];
    }

    void Clear()
    {
      for (int a = 0; a < rtaNumAttachments; a++)
	attachments[a].Clear();
    }

    void ComputeHash()
    {
      hash = 0;
      for (int a = 0; a < rtaNumAttachments; a++)
      {
	// To exclude the weakref memcpy protection from the hash
	csGLRender2TextureBackend::RTAttachment<iTextureHandle*> hashAttachment (
	  attachments[a]);
	hash = HashCompute ((char const*)(&hashAttachment), 
	  sizeof (csGLRender2TextureBackend::RTAttachment<iTextureHandle*>), hash);
      }
    #ifdef CS_DEBUG
      hashComputed = true;
    #endif
    }
    uint GetHash() const
    {
    #ifdef CS_DEBUG
      CS_ASSERT(hashComputed);
    #endif
      return hash;
    }

    template<class OtherKeeper>
    bool operator== (const R2TAttachmentGroup<OtherKeeper>& other) const
    {
      if (GetHash() != other.GetHash()) return false;
      for (int a = 0; a < rtaNumAttachments; a++)
      {
	if (attachments[a] != other.attachments[a]) return false;
      }
      return true;
    }

    template<class OtherKeeper>
    R2TAttachmentGroup& operator= (const R2TAttachmentGroup<OtherKeeper>& other)
    {
      hash = other.hash;
    #ifdef CS_DEBUG
      hashComputed = other.hashComputed;
    #endif
      for (int a = 0; a < rtaNumAttachments; a++)
      {
	attachments[a] = other.attachments[a];
      }
      return *this;
    }

    bool Empty() const
    {
      for (int a = 0; a < rtaNumAttachments; a++)
      {
	if (attachments[a].IsValid()) return false;
      }
      return true;
    }
  };
  
  typedef R2TAttachmentGroup<csRef<iTextureHandle> > RRTAG;
  typedef R2TAttachmentGroup<> WRTAG;

  class FBOWrapper
  {
    // @@@ Would be really nice to not have to store ext with every instance
    csGLExtensionManager* ext;
    Dimensions fbSize;
    csRef<RenderBufferWrapper> depthRB;
    csRef<RenderBufferWrapper> stencilRB;
    GLuint framebuffer;
    GLenum fbStatus;

    void FreeBuffers();
    void Complete2 (bool& needsDepth, bool& needsStencil);
    void SetRBAttachment (GLenum attachment, RenderBufferWrapper* rb);
    void SetDrawBuffers() const;
  public:
    uint32 initialAttachments;
    R2TAttachmentGroup<> attachments;

    FBOWrapper (csGLExtensionManager* ext,
      int w, int h) : ext (ext), fbSize (w, h), framebuffer (0),
      fbStatus (GL_FRAMEBUFFER_STATUS_ERROR_EXT) { }
    FBOWrapper (const FBOWrapper& other) : ext (other.ext), 
      fbSize (other.fbSize), framebuffer (0), 
      fbStatus (GL_FRAMEBUFFER_STATUS_ERROR_EXT)
    {
      // Only allow copying of un-setup wrappers
      CS_ASSERT (other.framebuffer == 0);
    }
    ~FBOWrapper() { FreeBuffers(); }

    template<typename BufferProvider>
    void Complete (BufferProvider& bufProv)
    {
      bool needsDepth, needsStencil;
      Complete2 (needsDepth, needsStencil);
      bufProv.GetDepthStencilRBs (fbSize, needsDepth, depthRB, 
	needsStencil, stencilRB);
      SetRBAttachment (GL_DEPTH_ATTACHMENT_EXT, depthRB);
      SetRBAttachment (GL_STENCIL_ATTACHMENT_EXT, stencilRB);

      fbStatus = ext->glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
    }

    int GetWidth() const { return fbSize.width; }
    int GetHeight() const { return fbSize.height; }
    GLenum GetStatus() const { return fbStatus; }

    void Bind ();
    void Unbind ();
  };

  namespace CacheSorting
  {
    struct RenderBuffer
    {
      typedef Dimensions KeyType;

      static bool IsLargerEqual (const csRef<RenderBufferWrapper>& r1, 
                                 const csRef<RenderBufferWrapper>& r2)
      {
        if ((r1->GetWidth() >= r2->GetWidth()) 
	  && (r1->GetHeight() >= r2->GetHeight())) return true;
        return false;
      }
    
      static bool IsEqual (const csRef<RenderBufferWrapper>& r1, 
                           const csRef<RenderBufferWrapper>& r2)
      {
        if ((r1->GetWidth() == r2->GetWidth()) 
	  && (r1->GetHeight() == r2->GetHeight())) return true;
        return false;
      }
    
      static bool IsLargerEqual (const csRef<RenderBufferWrapper>& r1, 
                                 const Dimensions& r2)
      {
        if ((r1->GetWidth() >= r2.width) 
	  && (r1->GetHeight() >= r2.height)) return true;
        return false;
      }
    
      static bool IsLargerEqual (const Dimensions& r1, 
                                 const csRef<RenderBufferWrapper>& r2)
      {
        if ((r1.width >= r2->GetWidth()) 
	  && (r1.height >= r2->GetHeight())) return true;
        return false;
      }
    
      static bool IsEqual (const csRef<RenderBufferWrapper>& r1, 
                           const Dimensions& r2)
      {
        if ((r1->GetWidth() == r2.width) 
	  && (r1->GetHeight() == r2.height)) return true;
        return false;
      }
    
    };

    struct FrameBuffer
    {
      typedef RRTAG KeyType;

      static bool IsLargerEqual (const FBOWrapper& b1, 
                                 const FBOWrapper& b2)
      {
	return b1.attachments.GetHash() >= b2.attachments.GetHash();
      }
    
      static bool IsEqual (const FBOWrapper& b1, 
                           const FBOWrapper& b2)
      {
        return b1.attachments == b2.attachments;
      }
    
      static bool IsLargerEqual (const FBOWrapper& b1, 
                                 const RRTAG& b2)
      {
	return b1.attachments.GetHash() >= b2.GetHash();
      }
      
      static bool IsLargerEqual (const RRTAG& b1, 
                                 const FBOWrapper& b2)
      {
	return b1.GetHash() >= b2.attachments.GetHash();
      }
    
      static bool IsEqual (const FBOWrapper& b1, 
                           const RRTAG& b2)
      {
	return b1.attachments == b2;
      }
    
    };
  } // namespace CacheSorting

  namespace CacheReuse
  {
    class RenderBuffer
    {
    public:
      struct AddParameter
      {
        AddParameter () {}
      };
      struct StoredAuxiliaryInfo
      {
	template<typename ResourceCacheType>
	StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	  const AddParameter& param) {}
      };
      
      template<typename ResourceCacheType>
      void MarkActive (const ResourceCacheType& cache,
	  StoredAuxiliaryInfo& elementInfo)
      { }

      template<typename ResourceCacheType>
      bool IsReusable (const ResourceCacheType& cache,
	StoredAuxiliaryInfo& elementInfo,
	const typename ResourceCacheType::CachedType& data)
      {
	return true;
      }
    };
  } // namespace CacheReuse


  namespace CachePurge
  {
    class FrameBuffer :
      public CS::Utility::ResourceCache::PurgeConditionAfterTime<>
    {
      typedef CS::Utility::ResourceCache::PurgeConditionAfterTime<> Parent;
    public:
      typedef Parent::AddParameter AddParameter;
      struct StoredAuxiliaryInfo : public Parent::StoredAuxiliaryInfo
      {
	template<typename ResourceCacheType>
	StoredAuxiliaryInfo (const ResourceCacheType& cache, 
	  const AddParameter& param) 
	  : Parent::StoredAuxiliaryInfo (cache, param) {}
      };
      
      FrameBuffer (uint purgeAge = 6000) : Parent (purgeAge) {}
      
      template<typename ResourceCacheType>
      void MarkActive (const ResourceCacheType& cache,
	  StoredAuxiliaryInfo& elementInfo)
      {
        Parent::MarkActive (cache, elementInfo);
      }

      template<typename ResourceCacheType>
      bool IsPurgeable (const ResourceCacheType& cache,
	StoredAuxiliaryInfo& elementInfo,
	const typename ResourceCacheType::CachedType& data)
      {
	if (Parent::IsPurgeable (cache, elementInfo, data))
	  return true;
	
	uint32 currentAttachments = 0;
	for (int a = 0; a < rtaNumAttachments; a++)
	{
	  const WRTAG::RTA& attachment =
	    data.attachments.GetAttachment (csRenderTargetAttachment(a));
	  if (!attachment.IsValid()) continue;
	  
	  currentAttachments |= 1 << a;
	}
	return currentAttachments != data.initialAttachments;
      }
    };
  } // namespace CachePurge

class csGLRender2TextureEXTfbo : public csGLRender2TextureBackend
{
  bool enableFBO;
  FBOWrapper* currentFBO;
  GLenum depthStorage, stencilStorage;
  bool viewportSet;
  R2TViewportHelper viewportHelper;

  typedef CS::Utility::GenericResourceCache<csRef<RenderBufferWrapper>,
    uint, CacheSorting::RenderBuffer, CacheReuse::RenderBuffer,
    CS::Utility::ResourceCache::PurgeIfOnlyOneRef> RBCache;
  RBCache depthRBCache, stencilRBCache;
  CS::Utility::GenericResourceCache<FBOWrapper,
    uint, CacheSorting::FrameBuffer, 
    CS::Utility::ResourceCache::ReuseConditionAfterTime<>,
    CachePurge::FrameBuffer> fboCache;

  R2TAttachmentGroup<csRef<iTextureHandle> > currentAttachments;

  csString fboMsg;
  const char* FBStatusStr (GLenum status);

  void RegenerateTargetMipmaps (const WRTAG::RTA& target);

  void SelectCurrentFBO ();
public:
  csGLRender2TextureEXTfbo (csGLGraphics3D* G3D);
  virtual ~csGLRender2TextureEXTfbo();
  bool Status() { return enableFBO; }

  bool SetRenderTarget (iTextureHandle* handle, bool persistent,
    int subtexture, csRenderTargetAttachment attachment);
  void UnsetRenderTargets();
  bool ValidateRenderTargets ();
  bool CanSetRenderTarget (const char* format, csRenderTargetAttachment attachment);
  iTextureHandle* GetRenderTarget (csRenderTargetAttachment attachment, int* subtexture) const;
  
  void BeginDraw (int drawflags);
  CS::Math::Matrix4 FixupProjection (
    const CS::Math::Matrix4& projectionMatrix);
  void FinishDraw (bool readbackTargets);
  void SetClipRect (const csRect& clipRect);
  void SetupClipPortalDrawing ();

  virtual bool HasStencil() { return stencilStorage != 0; }
  virtual bool HasMultisample()
  {
    /* Needs EXT_framebuffer_multisample */
    return false;
  }

  void NextFrame (uint frameNum);
  void CleanupFBOs();

  void GetDepthStencilRBs (const Dimensions& fbSize, 
    bool needsDepth, csRef<RenderBufferWrapper>& depthRB, 
    bool needsStencil, csRef<RenderBufferWrapper>& stencilRB);
};

}
CS_PLUGIN_NAMESPACE_END(gl3d)

#endif // __CS_GL_R2T_EXT_FB_O_H__

