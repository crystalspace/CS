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
#include "imesh/lighting.h"

#include "csgeom/csrect.h"
#include "csgfx/rgbpixel.h"
#include "csutil/bitarray.h"
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
                               iLightingInfo>
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

    void UnsafeAdd (int R, int G, int B)
    {
      c.red   = (unsigned char)(c.red   + R);
      c.green = (unsigned char)(c.green + G);
      c.blue  = (unsigned char)(c.blue  + B);
    }
    void SafeAdd (int R, int G, int B)
    {
      int color = c.red + R;
      c.red   = (unsigned char)(color > 255 ? 255 : color);
      color = c.green + G;
      c.green = (unsigned char)(color > 255 ? 255 : color);
      color = c.blue + B;
      c.blue  = (unsigned char)(color > 255 ? 255 : color);
    }
  };
  struct LumelBuffer : public csRefCount
  {
    static CS_FORCEINLINE size_t LumelAlign (size_t n)
    {
      static const size_t align = sizeof (Lumel);
      return ((n + align - 1) / align) * align;
    }
  public:
    CS_FORCEINLINE Lumel* GetData () 
    { 
      return reinterpret_cast<Lumel*> (
        (reinterpret_cast<uint8*> (this)) + LumelAlign (sizeof (*this))); 
    }
    
    inline void* operator new (size_t n, size_t lumels)
    { 
      CS_ASSERT (n == sizeof (LumelBuffer));
      size_t allocSize = 
        LumelAlign (sizeof (LumelBuffer)) + lumels * sizeof (Lumel);
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
  class PDMap
  {
    friend class ProctexPDLight;

    csPtr<LumelBuffer> CropLumels (LumelBuffer* lumels, 
      const csRect& lumelsRect, const csRect& cropRect);

    void ComputeValueBounds (const csRect& area, 
      csRGBcolor& maxValue, csRect& nonNullArea);
    void ComputeValueBounds (const TileHelper& tiles);
    void ComputeValueBounds (const TileHelper& tiles, const csRect& area);
  public:
    csArray<csRGBcolor> maxValues;
    csBitArray tileNonNull;
    csArray<csRect> nonNullAreas;
    int imageX, imageY, imageW, imageH;
    csRef<LumelBuffer> imageData;

    PDMap (size_t tilesNum) : imageX (0), imageY (0), imageW (0), imageH (0),
      imageData (0) 
    { 
      maxValues.SetSize (tilesNum, csRGBcolor (0, 0, 0));
      tileNonNull.SetSize (tilesNum);
      nonNullAreas.SetSize (tilesNum, 
        csRect (INT_MAX, INT_MAX, INT_MIN, INT_MIN));
    }
    PDMap (size_t tilesNum, const TileHelper& tiles, iImage* img) : 
      imageX (0), imageY (0), imageData (0)
    { 
      tileNonNull.SetSize (tilesNum);
      SetImage (tiles, img); 
    }
    void SetImage (const TileHelper& tiles, iImage* img);
    void Crop ();
    void GetMaxValue (csRGBcolor& maxValue);
  };
  struct MappedLight
  {
    PDMap map;
    char* lightId;
    csWeakRef<iLight> light;

    MappedLight (size_t tilesNum, const TileHelper& tiles, iImage* img) : 
      map (tilesNum, tiles, img), lightId (0) {}
    MappedLight (const MappedLight& other) : map (other.map), light (other.light)
    {
      if (other.lightId != 0)
      {
        lightId = new char[16];
        memcpy (lightId, other.lightId, 16);
      }
      else
        lightId = 0;
    }
    ~MappedLight() { delete[] lightId; }
  };
private:
  typedef csDirtyAccessArray<Lumel> LightmapScratch;
  CS_DECLARE_STATIC_CLASSVAR_REF(lightmapScratch, GetScratch, LightmapScratch);

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
    statePrepared = 1 << 1,
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

  ProctexPDLight (ProctexPDLightLoader* loader, iImage* img);
  ProctexPDLight (ProctexPDLightLoader* loader, int w, int h);
  virtual ~ProctexPDLight ();

  virtual bool PrepareAnim ();

  virtual void Animate (csTicks /*current_time*/);

  /**\name iLightingInfo implementation
   * @{ */
  void DisconnectAllLights ();
  void InitializeDefault (bool /*clear*/) {}
  void LightChanged (iLight* light);
  void LightDisconnect (iLight* light);
  void PrepareLighting () {}
  bool ReadFromCache (iCacheManager* /*cache_mgr*/) { return true; }
  bool WriteToCache (iCacheManager* /*cache_mgr*/) { return true; }
  /** @} */

  virtual void UseTexture (iTextureWrapper*)
  { 
    if (!PrepareAnim ()) return;
    Animate (0);
  }
};

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)

#endif // __CS_PTPDLIGHT_H__
