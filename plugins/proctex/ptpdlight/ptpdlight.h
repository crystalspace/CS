/*
    Copyright (C) 2003-2006 by Jorrit Tyberghein
	      (C) 2003-2007 by Frank Richter

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

#ifndef __CS_PTPDLIGHT_H__
#define __CS_PTPDLIGHT_H__

#include "iengine/light.h"
#include "ivideo/texture.h"

#include "csgeom/csrect.h"
#include "csgfx/rgbpixel.h"
#include "csutil/allocator.h"
#include "csutil/bitarray.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/flags.h"
#include "csutil/set.h"
#include "csutil/weakref.h"
#include "cstool/proctex.h"

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

class TileHelper
{
  int w, h, tx;
public:
  // The texture data is uploaded in tiles of this size
  // @@@ Make configurable?
  static const int tileSizeX = 128;
  static const int tileSizeY = 128;

  TileHelper (int w, int h);

  size_t ComputeTileCount () const;
  void MarkTilesBits (const csRect& r, csBitArray& bits) const;
  void GetTileRect (size_t n, csRect& r) const;
};

class ProctexPDLightLoader;

class ProctexPDLight : 
  public scfImplementationExt1<ProctexPDLight, 
                               csProcTexture,
                               iLightCallback>
{
public:
  struct Lumel
  {
    union
    {
      struct
      {
        uint8 blue, green, red, alpha;
      } c;
      uint32 ui;
    };
  };
  struct LumelBufferBase : public csRefCount
  {
  private:
    bool gray;
  protected:
    LumelBufferBase (bool gray) : gray (gray) {}
  public:
    bool IsGray() const { return gray; }
  };

#include "csutil/custom_new_disable.h"

  struct LumelBufferRGB : public LumelBufferBase
  {
    static CS_FORCEINLINE size_t LumelAlign (size_t n)
    {
      static const size_t align = 8; // For MMX
      return ((n + align - 1) / align) * align;
    }
  public:
    LumelBufferRGB () : LumelBufferBase (false) {}
    CS_FORCEINLINE Lumel* GetData ()
    { 
      return reinterpret_cast<Lumel*> (
        (reinterpret_cast<uint8*> (this)) + LumelAlign (sizeof (*this))); 
    }
    
    inline void* operator new (size_t n, size_t lumels)
    { 
      CS_ASSERT (n == sizeof (LumelBufferRGB));
      size_t allocSize = 
        LumelAlign (sizeof (LumelBufferRGB)) + lumels * sizeof (Lumel);
      return cs_malloc (allocSize);
    }
    inline void operator delete (void* p, size_t lumels) 
    {
      cs_free (p);
    }
    inline void operator delete (void* p) 
    {
      cs_free (p);
    }

  };

  struct LumelBufferGray : public LumelBufferBase
  {
  public:
    LumelBufferGray () : LumelBufferBase (true) {}
    CS_FORCEINLINE uint8* GetData ()
    { 
      return (reinterpret_cast<uint8*> (this)) + sizeof (*this); 
    }
    
    inline void* operator new (size_t n, size_t lumels)
    { 
      CS_ASSERT (n == sizeof (LumelBufferGray));
      size_t allocSize = 
        sizeof (LumelBufferGray) + lumels;
      return cs_malloc (allocSize);
    }
    inline void operator delete (void* p, size_t lumels) 
    {
      cs_free (p);
    }
    inline void operator delete (void* p) 
    {
      cs_free (p);
    }

  };

#include "csutil/custom_new_enable.h"

  class PDMap
  {
    friend class ProctexPDLight;

    csPtr<LumelBufferRGB> CropLumels (LumelBufferRGB* lumels, 
      const csRect& lumelsRect, const csRect& cropRect);

    void ComputeValueBounds (const csRect& area, 
      csRGBcolor& maxValue, csRect& nonNullArea);
    void ComputeValueBounds (const TileHelper& tiles);
    void ComputeValueBounds (const TileHelper& tiles, const csRect& area);
  public:
    csBitArray tileNonNull;
    int imageX, imageY, imageW, imageH;
    csRef<LumelBufferBase> imageData;
    struct Tile
    {
      csRGBcolor maxValue;

      int tilePartX, tilePartY;
      int tilePartW, tilePartH, tilePartPitch;
      void* tilePartData;

      Tile() : maxValue (0, 0, 0) {}
    };
    csArray<Tile> tiles;
    csArray<csRect> nonNullAreas;

    PDMap (size_t tilesNum) : imageX (0), imageY (0), imageW (0), imageH (0),
      imageData (0) 
    { 
      tileNonNull.SetSize (tilesNum);
      tiles.SetSize (tilesNum);
      nonNullAreas.SetSize (tilesNum, 
        csRect (INT_MAX, INT_MAX, INT_MIN, INT_MIN));
    }
    PDMap (size_t tilesNum, const TileHelper& tiles, iImage* img) : 
      imageX (0), imageY (0), imageData (0)
    { 
      tileNonNull.SetSize (tilesNum);
      this->tiles.SetSize (tilesNum);
      SetImage (tiles, img); 
    }
    void SetImage (const TileHelper& tiles, iImage* img);
    void Crop ();
    void GetMaxValue (csRGBcolor& maxValue);

    void UpdateTiles (const TileHelper& helper);
  };
  struct LightIdentity : public CS::Memory::CustomAllocated
  {
    csString sectorName, lightName;
    uint8 lightId[16];
  };
  struct MappedLight
  {
    PDMap map;
    LightIdentity* lightId;
    csWeakRef<iLight> light;

    MappedLight (size_t tilesNum, const TileHelper& tiles, iImage* img) : 
      map (tilesNum, tiles, img), lightId (0) {}
    MappedLight (const MappedLight& other) : map (other.map), light (other.light)
    {
      if (other.lightId != 0)
      {
        lightId = new LightIdentity (*(other.lightId));
      }
      else
        lightId = 0;
    }
    ~MappedLight() { delete lightId; }
  };
protected:
  csRef<ProctexPDLightLoader> loader;
  TileHelper tiles;
  csBitArray tilesDirty;
  csRGBcolor baseColor;
  PDMap baseMap;
  csSafeCopyArray<MappedLight> lights;
  csBitArray lightBits;
  csSet<csConstPtrKey<iLight> > dirtyLights;
  enum
  {
    stateDirty = 1 << 0,
    statePrepared = 1 << 1
  };
  csFlags state;
  struct LightColorState
  {
    // Color at the time the PT texture was last updated.
    csColor lastColor;
    /* Minimum difference of light color to lastColor before the "texture
     * dirty" flag is set. */
    csColor minChangeThresh;
  };
  csHash<LightColorState, csConstPtrKey<iLight> > lightColorStates;

  void Report (int severity, const char* msg, ...);

  struct PreApplyNoop
  {
    void Perform (iTextureHandle*, uint8*) {}
  };

  template<typename PreApply = PreApplyNoop>
  struct BlitBufHelper
  {
  private:
    PreApply preApply;
    iTextureHandle* texh;
    uint8* lastBuf;

    void DoApplyLast ()
    {
      preApply.Perform (texh, lastBuf);
      texh->ApplyBlitBuffer (lastBuf);
    }
  public:
    BlitBufHelper (iTextureHandle* texh) : texh (texh), lastBuf (0) {}
    ~BlitBufHelper ()
    {
      if (lastBuf != 0) DoApplyLast ();
    }

    uint8* QueryBlitBuffer (int x, int y, int width, int height,
                            size_t& pitch)
    {
      uint8* blitBuf = texh->QueryBlitBuffer (x, y, width, height,
        pitch, iTextureHandle::BGRA8888,
        iTextureHandle::blitbufReadable);
      if (lastBuf != 0) DoApplyLast ();
      lastBuf = blitBuf;
      return blitBuf;
    }
  };
public:
  const char* AddLight (const MappedLight& light);
  void FinishLoad()
  {
    lights.ShrinkBestFit();
  }
  void SetBaseColor (csRGBcolor col)
  {
    baseColor = col;
  }
  void SetTexFlags (int flags)
  {
    texFlags = flags;
  }
  MappedLight NewLight (iImage* img) const
  { return MappedLight (tilesDirty.GetSize(), tiles, img); }

  virtual ~ProctexPDLight ();

  virtual bool PrepareAnim ();

  virtual void Animate (csTicks current_time) = 0;
  virtual void Animate () = 0;

  /**\name iLightCallback implementation
   * @{ */
  virtual void OnColorChange (iLight* light, const csColor& newcolor);
  virtual void OnPositionChange (iLight* light, const csVector3& newpos) { };
  virtual void OnSectorChange (iLight* light, iSector* newsector) { };
  virtual void OnRadiusChange (iLight* light, float newradius) { };
  virtual void OnDestroy (iLight* light);
  virtual void OnAttenuationChange (iLight* light, int newatt) { };
  /** @} */

  using csProcTexture::UseTexture;
  virtual void UseTexture (iTextureWrapper*)
  { 
    if (!PrepareAnim ()) return;
    Animate (0);
  }
protected:
  ProctexPDLight (ProctexPDLightLoader* loader, iImage* img);
  ProctexPDLight (ProctexPDLightLoader* loader, int w, int h);
};

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)

#endif // __CS_PTPDLIGHT_H__
