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

#include "ivideo/texture.h"
#include "ivaria/profile.h"

#include "csgeom/fixed.h"
#include "csgeom/math.h"

#include "ptpdlight.h"
#include "ptpdlight_actual.h"

CS_DECLARE_PROFILER
CS_DECLARE_PROFILER_ZONE(ProctexPDLight_Animate_generic_inner)
CS_DECLARE_PROFILER_ZONE(ProctexPDLight_Animate_generic_Blit)

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

#if defined(CS_LITTLE_ENDIAN)
static const int shiftR = 16;
static const int shiftG =  8;
static const int shiftB =  0;
static const uint32 grayToRGBmul = 0x00010101;
static const uint32 grayToRGBalpha = 0xff000000;
#else
static const int shiftR =  8;
static const int shiftG = 16;
static const int shiftB = 24;
static const uint32 grayToRGBmul = 0x01010100;
static const uint32 grayToRGBalpha = 0x000000ff;
#endif

template<int shift>
static void ComputeLUT (csFixed16 v, uint32* lut)
{
  for (int x = 0; x < 256; x++)
  {
    *lut++ = csMin (int (v * x), 255) << shift;
  }
}

enum
{
  safeR = 1,
  safeG = 2,
  safeB = 4,

  safeAll = safeR | safeG | safeB
};

template<int safeMask>
static void MultiplyAddLumels (ProctexPDLight::Lumel* dst, size_t dstPitch,
                               const ProctexPDLight::Lumel* map, size_t mapPitch,
                               int columns, int rows,
                               const uint32* lutR, 
                               const uint32* lutG, 
                               const uint32* lutB)
{
  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < columns; x++)
    {
      const ProctexPDLight::Lumel mapVal = *map++;
      uint32 dstVal = dst->ui;
      if (safeMask & safeR)
      {
        // Possible overflow may occur
        uint32 r = dstVal & (0xff << shiftR);
        r += lutR[mapVal.c.red];
        if (r > (0xff << shiftR)) r = (0xff << shiftR);
        dstVal &= ~(0xff << shiftR);
        dstVal |= r;
      }
      else
        // Overflow can't occur
        dstVal += lutR[mapVal.c.red];

      if (safeMask & safeG)
      {
        uint32 g = dstVal & (0xff << shiftG);
        g += lutG[mapVal.c.green];
        if (g > (0xff << shiftG)) g = (0xff << shiftG);
        dstVal &= ~(0xff << shiftG);
        dstVal |= g;
      }
      else
        dstVal += lutG[mapVal.c.green];

      if (safeMask & safeB)
      {
        uint32 b = dstVal & (0xff << shiftB);
        b += lutB[mapVal.c.blue];
        if (b > (0xff << shiftB)) b = (0xff << shiftB);
        dstVal &= ~(0xff << shiftB);
        dstVal |= b;
      }
      else
        dstVal += lutB[mapVal.c.blue];

      (dst++)->ui = dstVal;
    }
    dst += dstPitch;
    map += mapPitch;
  }
}

template<int safeMask>
static void MultiplyAddLumels8 (ProctexPDLight::Lumel* dst, size_t dstPitch,
                                const uint8* map, size_t mapPitch,
                                int columns, int rows,
                                const uint32* lutR, 
                                const uint32* lutG, 
                                const uint32* lutB)
{
  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < columns; x++)
    {
      const uint8 v = *map++;
      uint32 dstVal = dst->ui;
      if (safeMask & safeR)
      {
        // Possible overflow may occur
        uint32 r = dstVal & (0xff << shiftR);
        r += lutR[v];
        if (r > (0xff << shiftR)) r = (0xff << shiftR);
        dstVal &= ~(0xff << shiftR);
        dstVal |= r;
      }
      else
        // Overflow can't occur
        dstVal += lutR[v];

      if (safeMask & safeG)
      {
        uint32 g = dstVal & (0xff << shiftG);
        g += lutG[v];
        if (g > (0xff << shiftG)) g = (0xff << shiftG);
        dstVal &= ~(0xff << shiftG);
        dstVal |= g;
      }
      else
        dstVal += lutG[v];

      if (safeMask & safeB)
      {
        uint32 b = dstVal & (0xff << shiftB);
        b += lutB[v];
        if (b > (0xff << shiftB)) b = (0xff << shiftB);
        dstVal &= ~(0xff << shiftB);
        dstVal |= b;
      }
      else
        dstVal += lutB[v];

      (dst++)->ui = dstVal;
    }
    dst += dstPitch;
    map += mapPitch;
  }
}

typedef void (*MultiplyAddProc) (ProctexPDLight::Lumel* dst, size_t dstPitch,
                                 const ProctexPDLight::Lumel* map, size_t mapPitch,
                                 int columns, int rows,
                                 const uint32* lutR, 
                                 const uint32* lutG, 
                                 const uint32* lutB);
typedef void (*MultiplyAddProc8) (ProctexPDLight::Lumel* dst, size_t dstPitch,
                                  const uint8* map, size_t mapPitch,
                                  int columns, int rows,
                                  const uint32* lutR, 
                                  const uint32* lutG, 
                                  const uint32* lutB);

static const MultiplyAddProc maProcs[8] = {
  MultiplyAddLumels<0>, MultiplyAddLumels<1>, 
  MultiplyAddLumels<2>, MultiplyAddLumels<3>, 
  MultiplyAddLumels<4>, MultiplyAddLumels<5>, 
  MultiplyAddLumels<6>, MultiplyAddLumels<7>
};

static const MultiplyAddProc8 maProcs8[8] = {
  MultiplyAddLumels8<0>, MultiplyAddLumels8<1>, 
  MultiplyAddLumels8<2>, MultiplyAddLumels8<3>, 
  MultiplyAddLumels8<4>, MultiplyAddLumels8<5>, 
  MultiplyAddLumels8<6>, MultiplyAddLumels8<7>
};

struct ProctexPDLight_Generic : public ProctexPDLight_Actual<ProctexPDLight_Generic>
{
  ProctexPDLight_Generic (ProctexPDLightLoader* loader, iImage* img)
   : ProctexPDLight_Actual<ProctexPDLight_Generic> (loader, img) {}
  ProctexPDLight_Generic (ProctexPDLightLoader* loader, int w, int h)
   : ProctexPDLight_Actual<ProctexPDLight_Generic> (loader, w, h) {}
   
  void RealAnimate ();
};

void ProctexPDLight_Generic::RealAnimate ()
{
  BlitBufHelper<> blitHelper (tex->GetTextureHandle ());

  lightBits.Clear();
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
    }
    l++;
  }

  for (size_t t = 0; t < tilesDirty.GetSize(); t++)
  {
    if (!tilesDirty.IsBitSet (t)) continue;

    csRect tileRect;
    tiles.GetTileRect (t, tileRect);

    size_t blitPitch;
    Lumel* scratch = (Lumel*)blitHelper.QueryBlitBuffer (
      tileRect.xmin, tileRect.ymin, 
      TileHelper::tileSizeX, TileHelper::tileSizeY,
      blitPitch);

    int scratchW = tileRect.Width();
    csRGBcolor scratchMax;
    {
      int lines = tileRect.Height();
      Lumel* scratchPtr = scratch;
      if (baseMap.imageData.IsValid())
      {
        if (baseMap.imageData->IsGray())
        {
	  uint8* basePtr = static_cast<LumelBufferGray*> (
	      (LumelBufferBase*)baseMap.imageData)->GetData()
	    + tileRect.ymin * mat_w + tileRect.xmin;
	  for (int y = 0; y < lines; y++)
	  {
            for (int x = 0; x < scratchW; x++)
            {
              uint8 v = basePtr[x];
              scratchPtr[x].ui = (grayToRGBmul * v) | grayToRGBalpha;
            }
	    scratchPtr += blitPitch / sizeof (Lumel);
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
	    memcpy (scratchPtr, basePtr, scratchW * sizeof (Lumel));
	    scratchPtr += blitPitch / sizeof (Lumel);
	    basePtr += mat_w;
	  }
	}
	scratchMax = baseMap.tiles[t].maxValue;
      }
      else
      {
        Lumel baseLumel;
        baseLumel.c.red   = baseColor.red;
        baseLumel.c.green = baseColor.green;
        baseLumel.c.blue  = baseColor.blue;
        baseLumel.c.alpha = 0;
        for (int y = 0; y < lines; y++)
        {
          for (int x = 0; x < scratchW; x++)
          {
            scratchPtr[x].ui = baseLumel.ui;
          }
          scratchPtr += blitPitch / sizeof (Lumel);
        }
        scratchMax = baseColor;
      }
    }
    for (size_t i = 0; i < lights.GetSize(); i++)
    {
      if (!lightBits.IsBitSet (i)) continue;
      MappedLight& light = lights[i];
      if (!light.map.tileNonNull.IsBitSet (t)) continue;
      const PDMap::Tile mapTile = light.map.tiles[t];

      //const csRect& lightInTile (light.map.nonNullAreas[t]);
      uint32 lutR[256];
      uint32 lutG[256];
      uint32 lutB[256];

      const csColor& lightColor = light.light->GetColor ();
      lightColorStates.GetElementPointer ((iLight*)light.light)->lastColor =
        lightColor;
      ComputeLUT<shiftR> (lightColor.red,   lutR);
      ComputeLUT<shiftG> (lightColor.green, lutG);
      ComputeLUT<shiftB> (lightColor.blue,  lutB);

      const int mapW = mapTile.tilePartW;
      const int lines = mapTile.tilePartH;
      const int mapPitch = mapTile.tilePartPitch;

      Lumel* scratchPtr = scratch + 
        mapTile.tilePartY * (blitPitch / sizeof (Lumel)) +
        mapTile.tilePartX;
      size_t scratchPitch = (blitPitch / sizeof (Lumel)) - mapW;

      csRGBcolor mapMax = mapTile.maxValue;
      mapMax.red   = lutR[mapMax.red]   >> shiftR;
      mapMax.green = lutG[mapMax.green] >> shiftG;
      mapMax.blue  = lutB[mapMax.blue]  >> shiftB;
      
      CS_PROFILER_ZONE(ProctexPDLight_Animate_generic_inner)

      int safeMask = 0;
      if (scratchMax.red   + mapMax.red   > 255) safeMask |= safeR;
      if (scratchMax.green + mapMax.green > 255) safeMask |= safeG;
      if (scratchMax.blue  + mapMax.blue  > 255) safeMask |= safeB;

      if (light.map.imageData->IsGray())
      {
        const uint8* mapPtr = (uint8*)(mapTile.tilePartData);
	  
        MultiplyAddProc8 maProc = maProcs8[safeMask];
        maProc (scratchPtr, scratchPitch, mapPtr, mapPitch,
          mapW, lines, lutR, lutG, lutB);
      }
      else
      {
        const Lumel* mapPtr = (Lumel*)(mapTile.tilePartData);
	  
        MultiplyAddProc maProc = maProcs[safeMask];
        maProc (scratchPtr, scratchPitch, mapPtr, mapPitch,
          mapW, lines, lutR, lutG, lutB);
      }

      if (safeMask == 0)
        scratchMax.UnsafeAdd (mapMax);
      else
        scratchMax.SafeAdd (mapMax);
    }
  }
}

ProctexPDLight* NewProctexPDLight_Generic (ProctexPDLightLoader* loader, iImage* img)
{
  return new ProctexPDLight_Generic (loader, img);
}

ProctexPDLight* NewProctexPDLight_Generic (ProctexPDLightLoader* loader, int w, int h)
{
  return new ProctexPDLight_Generic (loader, w, h);
}

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)
