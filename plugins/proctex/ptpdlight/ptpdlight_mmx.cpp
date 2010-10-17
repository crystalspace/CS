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

#include "cssysdef.h"
#include <limits.h>

#include "ivaria/profile.h"

#include "csgeom/math.h"

#include "ptpdlight.h"
#include "ptpdlight_actual.h"

#include <mmintrin.h>

CS_DECLARE_PROFILER
CS_DECLARE_PROFILER_ZONE(ProctexPDLight_Animate_MMX_inner)
CS_DECLARE_PROFILER_ZONE(ProctexPDLight_Animate_MMX_Blit)

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

/* This file is always compiled on MSVC, but it does not support MMX on all
   platforms */
#ifdef CS_SUPPORTS_MMX

struct Map_uint8
{
  typedef uint8 type;

  static __m64 CS_FORCEINLINE ExpandValues (const type*& vp)
  {
    const uint8 v1 = *vp++;
    const uint8 v2 = *vp++;
    return _mm_set_pi32 (v2*0x01010101, v1*0x01010101);
  }
};

struct Map_Lumel
{
  typedef ProctexPDLight::Lumel type;

  static __m64 CS_FORCEINLINE ExpandValues (const type*& vp)
  {
    __m64 ret = *((__m64*)vp);
    vp += 2;
    return ret;
  }
};

template<typename Map, bool overflowCheck>
static void MultiplyAddLumels (__m64* dst, size_t dstPitch,
                               const typename Map::type* map, 
                               size_t mapPitch, int columns, int rows,
                               __m64 lightColor)
{
  register const __m64 overflow_saturate = _mm_set1_pi32 (0x7fff7fff);

  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < columns; x+=2)
    {
      __m64 colors = Map::ExpandValues (map);
      __m64 color1 = _m_punpckhbw (colors, _mm_setzero_si64());
      __m64 color2 = _m_punpcklbw (colors, _mm_setzero_si64());

      if (overflowCheck)
      {
        // Find out what components didn't overflow
        __m64 overflowed =
          _m_pcmpeqw (_m_pmulhw (color1, lightColor), _mm_setzero_si64());
        // Multiply color with light
        color1 = _m_psrlwi (_m_pmullw (color1, lightColor), 8);
        // Saturate overflowed components
        overflowed = _m_pandn (overflowed, overflow_saturate);
        // The final color
        color1 = _m_por (color1, overflowed);

        // Find out what components didn't overflow
        overflowed =
          _m_pcmpeqw (_m_pmulhw (color2, lightColor), _mm_setzero_si64());
        // Multiply color with light
        color2 = _m_psrlwi (_m_pmullw (color2, lightColor), 8);
        // Saturate overflowed components
        overflowed = _m_pandn (overflowed, overflow_saturate);
        // The final color
        color2 = _m_por (color2, overflowed);
      }
      else
      {
        // Multiply colors
        color1 = _m_psrlwi (_m_pmullw (color1, lightColor), 8);
        color2 = _m_psrlwi (_m_pmullw (color2, lightColor), 8);
      }

      colors = _m_packuswb (color2, color1);
      *dst = _m_paddusb (*dst, colors);
      dst++;
    }
    dst += dstPitch;
    map += mapPitch;
  }
}

struct PreApplyEMMS
{
  void Perform (iTextureHandle* texh, uint8* buf) 
  { 
    if (texh->GetBufferNature (buf) == iTextureHandle::natureIndirect)
      _m_empty(); 
  }
};

struct ProctexPDLight_MMX : public ProctexPDLight_Actual<ProctexPDLight_MMX>
{
  ProctexPDLight_MMX (ProctexPDLightLoader* loader, iImage* img)
   : ProctexPDLight_Actual<ProctexPDLight_MMX> (loader, img) {}
  ProctexPDLight_MMX (ProctexPDLightLoader* loader, int w, int h)
   : ProctexPDLight_Actual<ProctexPDLight_MMX> (loader, w, h) {}
   
  void RealAnimate ();
};

void ProctexPDLight_MMX::RealAnimate ()
{
  BlitBufHelper<PreApplyEMMS> blitHelper (tex->GetTextureHandle ());

  lightBits.Clear();
  size_t numLights = 0;
  for (size_t l = 0; l < lights.GetSize(); )
  {
    MappedLight& light = lights[l];
    if (!light.light)
    {
      tilesDirty |= light.map.tileNonNull;
      lightColorStates.DeleteAll ((iLight*)light.light);
      lights.DeleteIndexFast (l);
      lightBits.SetSize (lights.GetSize ());
      continue;
    }
    if (dirtyLights.Contains ((iLight*)light.light))
    {
      tilesDirty |= light.map.tileNonNull;
      lightBits.SetBit (l);
      numLights++;
    }
    l++;
  }

  size_t l = 0;
  CS_ALLOC_STACK_ARRAY(uint16, lightFactors_ui16, numLights*3);
  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    if (!lightBits.IsBitSet (i)) continue;
    MappedLight& light = lights[i];

    const csColor& lightColor = light.light->GetColor ();
    lightColorStates.GetElementPointer ((iLight*)light.light)->lastColor =
      lightColor;
    lightFactors_ui16[l+0] = int (lightColor.red   * 256);
    lightFactors_ui16[l+1] = int (lightColor.green * 256);
    lightFactors_ui16[l+2] = int (lightColor.blue  * 256);
    l += 3;
  }

  l = 0;
  CS_ALLOC_STACK_ARRAY(__m64, lightFactors, numLights);
  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    if (!lightBits.IsBitSet (i)) continue;

    lightFactors[l] = _mm_set_pi16 (0,
      lightFactors_ui16[l*3+0],
      lightFactors_ui16[l*3+1], 
      lightFactors_ui16[l*3+2]);
    l++;
  }

  for (size_t t = 0; t < tilesDirty.GetSize(); t++)
  {
    if (!tilesDirty.IsBitSet (t)) continue;

    csRect tileRect;
    tiles.GetTileRect (t, tileRect);

    size_t blitPitch;
    __m64* scratch = (__m64*)blitHelper.QueryBlitBuffer (
      tileRect.xmin, tileRect.ymin, 
      TileHelper::tileSizeX, TileHelper::tileSizeY,
      blitPitch);

    int scratchW = tileRect.Width();
    {
      int lines = tileRect.Height();
      __m64* scratchPtr = scratch;
      if (baseMap.imageData.IsValid())
      {
        if (baseMap.imageData->IsGray())
        {
          uint8* basePtr = static_cast<LumelBufferGray*> (
	      (LumelBufferBase*)baseMap.imageData)->GetData()
            + tileRect.ymin * mat_w + tileRect.xmin;
          for (int y = 0; y < lines; y++)
          {
            for (int x = 0; x < scratchW; x+=2)
            {
              uint8 v1 = basePtr[x];
              uint8 v2 = basePtr[x+1];
              scratchPtr[x/2] = _mm_set_pi32 (v1*0x01010101, 
                v2*0x01010101);
            }
            scratchPtr += blitPitch / sizeof (__m64);
            basePtr += mat_w;
          }
        }
        else
        {
          Lumel* basePtr = static_cast<LumelBufferRGB*> (
      	      (LumelBufferBase*)baseMap.imageData)->GetData()
            + tileRect.ymin * mat_w + tileRect.xmin;
          for (int y = 0; y < lines; y++)
          {
            memcpy (scratchPtr, basePtr, scratchW * sizeof (uint32));
            scratchPtr += blitPitch / sizeof (__m64);
            basePtr += mat_w;
          }
        }
      }
      else
      {
        Lumel baseLumel;
        baseLumel.c.red   = baseColor.red;
        baseLumel.c.green = baseColor.green;
        baseLumel.c.blue  = baseColor.blue;
        baseLumel.c.alpha = 0;
        __m64 baseLumelPacked = 
          _mm_set1_pi32 (baseLumel.ui);
        for (int y = 0; y < lines; y++)
        {
          for (int x = 0; x < scratchW/2; x++)
          {
            scratchPtr[x] = baseLumelPacked;
          }
          scratchPtr += blitPitch / sizeof (__m64);
        }
      }
    }

    size_t l = 0;
    for (size_t i = 0; i < lights.GetSize(); i++)
    {
      if (!lightBits.IsBitSet (i)) continue;
      MappedLight& light = lights[i];
      if (!light.map.tileNonNull.IsBitSet (t))
      {
	l++;
	continue;
      }
      const PDMap::Tile mapTile = light.map.tiles[t];

      const int mapW = mapTile.tilePartW;
      const int lines = mapTile.tilePartH;
      const int mapPitch = mapTile.tilePartPitch;

      __m64* scratchPtr = scratch + 
        mapTile.tilePartY * (blitPitch / sizeof (__m64)) +
        mapTile.tilePartX / 2;
      size_t scratchPitch = blitPitch / sizeof (__m64)  - mapW/2;

      __m64 lightColor = lightFactors[l];
      csRGBcolor mapMax = mapTile.maxValue;
      int mapMax_red   = mapMax.red   * lightFactors_ui16[l*3+0];
      int mapMax_green = mapMax.green * lightFactors_ui16[l*3+1];
      int mapMax_blue  = mapMax.blue  * lightFactors_ui16[l*3+2];
      bool hasOverflow = (mapMax_red   > 0xffff)
                      || (mapMax_green > 0xffff)
                      || (mapMax_blue  > 0xffff);

      CS_PROFILER_ZONE(ProctexPDLight_Animate_MMX_inner)
      if (light.map.imageData->IsGray())
      {
        const uint8* mapPtr = (uint8*)(mapTile.tilePartData);

        if (hasOverflow)
          MultiplyAddLumels<Map_uint8, true> (scratchPtr, scratchPitch,
            mapPtr, mapPitch, mapW, lines, lightColor);
        else
          MultiplyAddLumels<Map_uint8, false> (scratchPtr, scratchPitch,
            mapPtr, mapPitch, mapW, lines, lightColor);
      }
      else
      {
        const Lumel* mapPtr = (Lumel*)(mapTile.tilePartData);
	  
        if (hasOverflow)
          MultiplyAddLumels<Map_Lumel, true> (scratchPtr, scratchPitch,
            mapPtr, mapPitch, mapW, lines, lightColor);
        else
          MultiplyAddLumels<Map_Lumel, false> (scratchPtr, scratchPitch,
            mapPtr, mapPitch, mapW, lines, lightColor);
      }
      l++;
    }
  }
  _m_empty();
}

ProctexPDLight* NewProctexPDLight_MMX (ProctexPDLightLoader* loader, iImage* img)
{
  return new ProctexPDLight_MMX (loader, img);
}

ProctexPDLight* NewProctexPDLight_MMX (ProctexPDLightLoader* loader, int w, int h)
{
  return new ProctexPDLight_MMX (loader, w, h);
}

#endif // CS_SUPPORTS_MMX

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)
