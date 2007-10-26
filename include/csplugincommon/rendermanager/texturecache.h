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

struct iTextureHandle;

namespace CS
{
namespace RenderManager
{
  class TextureCache
  {
    csRef<iGraphics3D> g3d;
  
    struct HeldTexture
    {
      csRef<iTextureHandle> texture;
      csTicks deathTime;
    };
    csImageType imgtype;
    const char* format;
    int textureFlags;
    const char* texClass;
    
    uint currentTime;

    csArray<HeldTexture> texInUse;
    csArray<HeldTexture> texAvailable;
    
    static int SortHeldTexture (const HeldTexture& t1, const HeldTexture& t2)
    {
      int tw1, th1, tw2, th2;
      t1.texture->GetRendererDimensions (tw1, th1);
      t2.texture->GetRendererDimensions (tw2, th2);
      
      if (tw1 < tw2) return -1;
      else if (tw1 > tw2) return 1;
      if (th1 < th2) return -1;
      else if (th1 > th2) return 1;
      return 0;
    }
    
    uint options;
  public:
    enum
    {
      tcacheExactSizeMatch = 1,
      tcachePowerOfTwo = 2
    };

    TextureCache (csImageType imgtype, const char* format, int textureFlags, 
      const char* texClass, uint options) : g3d (0), imgtype (imgtype),
      format (format), textureFlags (textureFlags), texClass (texClass), 
      currentTime (0), options (options) {}
      
    void SetG3D (iGraphics3D* g3d) { this->g3d = g3d; }
    
    void Clear ()
    {
      texInUse.DeleteAll ();
      texAvailable.DeleteAll ();
    }
    void AdvanceFrame (csTicks currentTime)
    {
      this->currentTime = currentTime;
      size_t i = 0;
      while (i < texInUse.GetSize())
      {
        if (currentTime > texInUse[i].deathTime)
        {
          texAvailable.InsertSorted (texInUse[i], SortHeldTexture);
          texInUse.DeleteIndexFast (i);
        }
        else
          i++;
      }
    }
    iTextureHandle* QueryUnusedTexture (int width, int height, csTicks lifetime,
                                        int& real_w, int& real_h)
    {
      HeldTexture tex;
      size_t i = 0;
      while (i < texAvailable.GetSize())
      {
        const HeldTexture& oldTex = texAvailable[i];
        iTextureHandle* texh = oldTex.texture;
        int tw, th;
        texh->GetRendererDimensions (tw, th);
        if (((options & tcacheExactSizeMatch) && (tw == width) && (th == height))
          || ((tw >= width) && (th >= height)))
	{
	  tex = oldTex;
	  texAvailable.DeleteIndex (i);
          tex.deathTime = currentTime + lifetime;
          texInUse.Push (tex);
          real_w = tw;
          real_h = th;
          return tex.texture;
	}
	i++;
      }
      if (options & tcachePowerOfTwo)
      {
        width = csFindNearestPowerOf2 (width);
        height = csFindNearestPowerOf2 (height);
      }
      real_w = width;
      real_h = height;
      tex.texture = g3d->GetTextureManager()->CreateTexture (
        width, height, imgtype, format, textureFlags);
      tex.texture->SetTextureClass (texClass);
      tex.deathTime = currentTime + lifetime;
      texInUse.Push (tex);
      return tex.texture;
    }
    iTextureHandle* QueryUnusedTexture (int width, int height, csTicks lifetime)
    {
      int dummyW, dummyH;
      return QueryUnusedTexture (width, height, lifetime, dummyW, dummyH);
    }
  };
  
} // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_TEXTURECACHE_H__
