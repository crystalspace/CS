/*
    Copyright (C) 2008 by Joe Forte

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

#ifndef __GBUFFER_H__
#define __GBUFFER_H__

#include "cssysdef.h"

#include "iutil/comp.h"
#include "csutil/scf_implementation.h"
#include "iengine/rendermanager.h"
#include "itexture.h"

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

  /**
   * Utility class for holding a collection of render targets used 
   * for deferred shading.
   */
  class GBuffer
  {
  public:

    /**
     * Data used to define the structure of the gbuffer.
     */
    struct Description
    {
      int width;
      int height;

      size_t colorBufferCount;
      bool hasDepthBuffer;
    };

    GBuffer() : graphics3D (NULL), isAttached (false) {}
    ~GBuffer() { Detach (); }

    /// Initializes the gbuffer.
    bool Initialize(const Description &desc, 
                    iGraphics3D *g3D, 
                    iShaderVarStringSet *stringSet,
                    iObjectRegistry *registry)
    {
      const char *messageID = "crystalspace.rendermanager.deferred.gbuffer";

      const int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NPOTS;
      const int w = desc.width;
      const int h = desc.height;

      iTextureManager *texMgr = g3D->GetTextureManager ();

      // Checks for device support. NOTE: Even if this test passes it is not guaranteed that
      // the gbuffer format will be supported on the device.
      const csGraphics3DCaps *caps = g3D->GetCaps ();
      if (desc.colorBufferCount > (size_t)caps->MaxRTColorAttachments)
      {
        csReport(registry, 
                 CS_REPORTER_SEVERITY_ERROR, 
                 messageID, 
                 "Too many color buffers requested (%d requested, device supports %d)!", 
                 desc.colorBufferCount, 
                 caps->MaxRTColorAttachments);

        return false;
      }

      size_t count = desc.colorBufferCount;
      for (size_t i = 0; i < count; i++)
      {
        csRef<iTextureHandle> colorBuffer = texMgr->CreateTexture (w, h, csimg2D, 
          "rgba16_f", flags, NULL);

        if(!colorBuffer)
        {
          csReport(registry, CS_REPORTER_SEVERITY_ERROR, messageID, 
            "Could not create color buffer %d!", i);
          return false;
        }

        char buff[64];
        cs_snprintf (buff, sizeof(char) * 64, "tex gbuffer %d", i);

        colorBuffers.Push (colorBuffer);
        colorBufferSVNames.Push (stringSet->Request (buff));
      }

      if (desc.hasDepthBuffer)
      {
        depthBuffer = texMgr->CreateTexture (w, h, csimg2D, "d24s8", flags, NULL);

        if(!depthBuffer)
        {
          csReport(registry, CS_REPORTER_SEVERITY_ERROR, messageID, 
            "Could not create depth buffer!");
          return false;
        }

        depthBufferSVName = stringSet->Request ("tex gbuffer depth");
      }

      graphics3D = g3D;

      return true;
    }

    /**
     * Attaches the gbuffer as the output render targets.
     */
    bool Attach()
    {
      if (IsAttached ())
        return false;

      size_t count = GetColorBufferCount();
      for (size_t i = 0; i < count; i++)
      {
        csRenderTargetAttachment attach = (csRenderTargetAttachment)((size_t)rtaColor0 + i);

        if(!graphics3D->SetRenderTarget (GetColorBuffer (i), false, 0, attach))
          return false;
      }

      if(!graphics3D->SetRenderTarget (GetDepthBuffer (), false, 0, rtaDepth))
        return false;
      
      if (!graphics3D->ValidateRenderTargets ())
        return false;

      isAttached = true;

      return true;
    }

    /// Detaches the gbuffer as the output render targets.
    bool Detach()
    {
      if (IsAttached ())
      {
        CS_ASSERT (graphics3D);
        
        graphics3D->UnsetRenderTargets ();
        isAttached = false;
      }

      return true;
    }

    /// Returns true if this gbuffer is attached.
    bool IsAttached() const 
    { 
      return isAttached; 
    }

    /// Updates shader variables.
    bool UpdateShaderVars(iShaderVariableContext *ctx)
    {
      size_t count = GetColorBufferCount();
      for (size_t i = 0; i < count; i++)
      {
        csShaderVariable *colorSV = ctx->GetVariableAdd (colorBufferSVNames[i]);
        colorSV->SetValue (GetColorBuffer (i));
      }

      csShaderVariable *depthSV = ctx->GetVariableAdd (depthBufferSVName);
      depthSV->SetValue (GetDepthBuffer ());

      return true;
    }

    /// Returns the number of color buffers in the gbuffer.
    size_t GetColorBufferCount() const
    {
      return colorBuffers.GetSize ();
    }

    /// Returns the ith color buffer.
    iTextureHandle *GetColorBuffer(size_t i)
    {
      CS_ASSERT(i < GetColorBufferCount());
      return colorBuffers[i];
    }

    /// Returns the depth buffer.
    iTextureHandle *GetDepthBuffer()
    {
      return depthBuffer;
    }

  private:

    csRefArray<iTextureHandle> colorBuffers;
    csRef<iTextureHandle> depthBuffer;

    // Stores the shader variable names.
    csArray<ShaderVarStringID> colorBufferSVNames;
    ShaderVarStringID depthBufferSVName;

    iGraphics3D *graphics3D;

    bool isAttached;
  };

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)

#endif // __GBUFFER_H__
