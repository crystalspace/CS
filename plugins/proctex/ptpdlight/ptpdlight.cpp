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
#include "iengine/engine.h"
#include "ivaria/reporter.h"

#include "csgeom/fixed.h"
#include "csgeom/math.h"
#include "csgfx/imageautoconvert.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"

#include "ptpdlight.h"

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

  TileHelper::TileHelper (int w, int h)
  {
    this->w = w; this->h = h;
    tx = (w + tileSizeX - 1) / tileSizeX;
  }

  size_t TileHelper::ComputeTileCount () const
  {
    int ty = (h + tileSizeY - 1) / tileSizeY;
    return tx * ty;
  }

  void TileHelper::MarkTilesBits (const csRect& r, csBitArray& bits) const
  {
    int x1 = (r.xmin) / tileSizeX;
    int y1 = (r.ymin) / tileSizeY;
    int x2 = (r.xmax + tileSizeX - 1) / tileSizeX;
    int y2 = (r.ymax + tileSizeY - 1) / tileSizeY;
    for (int y = y1; y < y2; y++)
    {
      for (int x = x1; x < x2; x++)
      {
        bits.SetBit (y * tx + x);
      }
    }
  }

  void TileHelper::GetTileRect (size_t n, csRect& r) const
  {
    int x = int (n) % tx;
    int y = int (n) / tx;
    r.xmin = x * tileSizeX;
    r.ymin = y * tileSizeY;
    r.xmax = csMin ((x+1) * tileSizeX, w);
    r.ymax = csMin ((y+1) * tileSizeY, h);
  }

//---------------------------------------------------------------------------

CS_IMPLEMENT_STATIC_CLASSVAR_REF(ProctexPDLight, lightmapScratch, GetScratch,
                                 ProctexPDLight::LightmapScratch, ());

void ProctexPDLight::PDMap::ComputeValueBounds (const csRect& area, 
                                                csRGBcolor& maxValue, 
                                                csRect& nonNullArea)
{
  const int width = imageW;
  maxValue.Set (0, 0, 0);
  nonNullArea.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  const Lumel* map = imageData->GetData() + area.ymin * width + area.xmin;
  int mapPitch = width - area.Width ();
  for (int y = area.ymin; y < area.ymax; y++)
  {
    for (int x = area.xmin; x < area.xmax; x++)
    {
      const Lumel& p = *map++;

      if (p.c.red > maxValue.red)
        maxValue.red = p.c.red;
      if (p.c.green > maxValue.green)
        maxValue.green = p.c.green;
      if (p.c.blue > maxValue.blue)
        maxValue.blue = p.c.blue;

      if (p.c.red + p.c.green + p.c.blue > 0)
      {
        nonNullArea.Extend (x, y);
      }
    }
    map += mapPitch;
  }
}

void ProctexPDLight::PDMap::ComputeValueBounds (const TileHelper& tiles)
{
  csRect r (0, 0, imageW, imageH);
  ComputeValueBounds (tiles, r);
}

void ProctexPDLight::PDMap::ComputeValueBounds (const TileHelper& tiles, 
                                                const csRect& area)
{
  maxValues.SetSize (tileNonNull.GetSize());
  nonNullAreas.SetSize (tileNonNull.GetSize());
  if (!imageData) 
  {
    for (size_t t = 0; t < tileNonNull.GetSize(); t++)
    {
      maxValues[t].Set (0, 0, 0);
      nonNullAreas[t].Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
    }
    return;
  }

  for (size_t t = 0; t < tileNonNull.GetSize(); t++)
  {
    csRect r;
    tiles.GetTileRect (t, r);
    r.Intersect (area);
    if (r.IsEmpty()) 
    {
      maxValues[t].Set (0, 0, 0);
      nonNullAreas[t].Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
      continue;
    }

    csRGBcolor maxValue;
    csRect nonNullArea;
    ComputeValueBounds (r, maxValue, nonNullArea);
    maxValues[t] = maxValue;
    nonNullAreas[t] = nonNullArea;
    if (maxValue.red + maxValue.green + maxValue.blue > 0)
    {
      CS_ASSERT (!nonNullArea.IsEmpty());
      tileNonNull.SetBit (t);
    }
    else
    {
      CS_ASSERT (nonNullArea.IsEmpty());
    }
  }
}

void ProctexPDLight::PDMap::SetImage (const TileHelper& tiles, iImage* img)
{
  CS::ImageAutoConvert useImage (img, CS_IMGFMT_TRUECOLOR);
  imageW = useImage->GetWidth();
  imageH = useImage->GetHeight();
  size_t numPixels = imageW * imageH;
  imageData.AttachNew (new (numPixels) LumelBuffer);
  const csRGBpixel* src = (csRGBpixel*)useImage->GetImageData();
  Lumel* dst = imageData->GetData();
  while (numPixels-- > 0)
  {
    dst->c.red = src->red;
    dst->c.green = src->green;
    dst->c.blue = src->blue;
    dst->c.alpha = 0;
    dst++;
    src++;
  }
  ComputeValueBounds (tiles); 
}

void ProctexPDLight::PDMap::Crop ()
{
  csRect nonNullArea (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  for (size_t t = 0; t < tileNonNull.GetSize(); t++)
  {
    if (!tileNonNull.IsBitSet (t)) continue;
    nonNullArea.Union (nonNullAreas[t]);
  }
  if (nonNullArea.IsEmpty()) return;

  csRef<LumelBuffer> newData;
  newData.AttachNew (
    new (nonNullArea.Width() * nonNullArea.Height()) LumelBuffer);
  const Lumel* srcMap = 
    imageData->GetData() + nonNullArea.ymin * imageW + nonNullArea.xmin;
  int rowSize = nonNullArea.Width ();
  Lumel* dstMap = newData->GetData();
  for (int y = nonNullArea.Height(); y-- > 0; )
  {
    memcpy (dstMap, srcMap, rowSize * sizeof (Lumel));
    dstMap += rowSize;
    srcMap += imageW;
  }

  imageData = newData;
  imageX = nonNullArea.xmin;
  imageY = nonNullArea.ymin;
  imageW = nonNullArea.Width();
  imageH = nonNullArea.Height();
}

void ProctexPDLight::Report (int severity, const char* msg, ...)
{
  static const char msgId[] = "crystalspace.proctex.pdlight";

  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, msgId, msg, arg);
  va_end (arg);
}

const char* ProctexPDLight::AddLight (const MappedLight& light)
{
  if ((light.map.imageW != mat_w)
    || (light.map.imageH != mat_h))
    return "PD lightmap dimensions don't correspond to base lightmap dimensions";

  if (light.map.tileNonNull.AllBitsFalse())
    return 0; //Silently ignore totally black maps

  size_t n = lights.Push (light);
  lights[n].map.Crop();
  return 0;
}

ProctexPDLight::ProctexPDLight (iImage* img) : 
  scfImplementationType (this, (iTextureFactory*)0, img), 
  tiles (img->GetWidth(), img->GetHeight()),
  tilesDirty (tiles.ComputeTileCount()),
  baseMap (tilesDirty.GetSize(), tiles, img), 
  state (stateDirty), baseColor (0, 0, 0)
{
  mat_w = img->GetWidth();
  mat_h = img->GetHeight();
}

ProctexPDLight::ProctexPDLight (int w, int h) : 
  scfImplementationType (this), 
  tiles (w, h), tilesDirty (tiles.ComputeTileCount()),
  baseMap (tilesDirty.GetSize()), 
  state (stateDirty), 
  baseColor (0, 0, 0)
{
  mat_w = w;
  mat_h = h;
}

ProctexPDLight::~ProctexPDLight ()
{
}

bool ProctexPDLight::PrepareAnim ()
{
  if (state.Check (statePrepared)) return true;

  if (!csProcTexture::PrepareAnim()) return false;

  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, "No iEngine!");
    return false;
  }

  lightmapSize = mat_w * mat_h;
  if (lightmapSize == 0) return false;
  for (size_t i = 0; i < lights.GetSize(); )
  {
    MappedLight& light = lights[i];
    bool success = false;
    light.light = engine->FindLightID (light.lightId);
    if (light.light)
    {
      success = true;
      light.light->AddAffectedLightingInfo (
        static_cast<iLightingInfo*> (this));
      dirtyLights.Add ((iLight*)light.light);
    }
    else
    {
      csString hexId;
      for (int i = 0; i < 16; i++)
        hexId.AppendFmt ("%02x", light.lightId[i]);
      Report (CS_REPORTER_SEVERITY_WARNING, 
        "Could not find light with ID '%s'", hexId.GetData());
    }
    delete[] light.lightId; light.lightId = 0;
    if (success)
    {
      i++;
    }
    else
      lights.DeleteIndexFast (i);
  }
  lights.ShrinkBestFit();
  lightBits.SetSize (lights.GetSize ());
  state.Set (statePrepared);
  return true;
}

#if defined(CS_LITTLE_ENDIAN)
static const int shiftR = 16;
static const int shiftG =  8;
static const int shiftB =  0;
#else
static const int shiftR =  8;
static const int shiftG = 16;
static const int shiftB = 24;
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
        if (g > (0xff << shiftR)) g = (0xff << shiftG);
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

typedef void (*MultiplyAddProc) (ProctexPDLight::Lumel* dst, size_t dstPitch,
                                 const ProctexPDLight::Lumel* map, size_t mapPitch,
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

void ProctexPDLight::Animate (csTicks /*current_time*/)
{
  if (state.Check (stateDirty))
  {
    if (lightmapSize > 0)
    {
      lightBits.Clear();
      for (size_t l = 0; l < lights.GetSize(); )
      {
        MappedLight& light = lights[l];
        if (!light.light)
        {
          tilesDirty |= light.map.tileNonNull;
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

      LightmapScratch& scratch = GetScratch();
      scratch.SetSize (TileHelper::tileSizeX * TileHelper::tileSizeY);
      for (size_t t = 0; t < tilesDirty.GetSize(); t++)
      {
        if (!tilesDirty.IsBitSet (t)) continue;

        csRect tileRect;
        tiles.GetTileRect (t, tileRect);

        int scratchW = tileRect.Width();
        csRGBcolor scratchMax;
        {
          int lines = tileRect.Height();
          Lumel* scratchPtr = scratch.GetArray();
          if (baseMap.imageData.IsValid())
          {
            Lumel* basePtr = (baseMap.imageData->GetData()) +
              tileRect.ymin * mat_w + tileRect.xmin;
            for (int y = 0; y < lines; y++)
            {
              memcpy (scratchPtr, basePtr, scratchW * sizeof (Lumel));
              scratchPtr += scratchW;
              basePtr += mat_w;
            }
            scratchMax = baseMap.maxValues[t];
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
                scratchPtr->ui = baseLumel.ui;
                scratchPtr++;
              }
            }
            scratchMax = baseColor;
          }
        }
        for (size_t i = 0; i < lights.GetSize(); i++)
        {
          if (!lightBits.IsBitSet (i)) continue;
          MappedLight& light = lights[i];
          if (!light.map.tileNonNull.IsBitSet (t)) continue;

          const csRect& lightInTile (light.map.nonNullAreas[t]);
          uint32 lutR[256];
          uint32 lutG[256];
          uint32 lutB[256];

          ComputeLUT<shiftR> (light.light->GetColor ().red,   lutR);
          ComputeLUT<shiftG> (light.light->GetColor ().green, lutG);
          ComputeLUT<shiftB> (light.light->GetColor ().blue,  lutB);

          int mapW = csMin (lightInTile.xmax, light.map.imageX + light.map.imageW)
            - csMax (lightInTile.xmin, light.map.imageX);
          const Lumel* mapPtr = (light.map.imageData->GetData()) +
            (lightInTile.ymin - light.map.imageY) * light.map.imageW +
            (lightInTile.xmin - light.map.imageX) ;
          int lines = csMin (lightInTile.ymax, light.map.imageY + light.map.imageH)
            - csMax (lightInTile.ymin, light.map.imageY);
          int mapPitch = light.map.imageW - mapW;

          Lumel* scratchPtr = scratch.GetArray() + 
            (lightInTile.ymin - tileRect.ymin) * scratchW +
             lightInTile.xmin - tileRect.xmin;
          int scratchPitch = scratchW - mapW;

          csRGBcolor mapMax = light.map.maxValues[t];
          mapMax.red   = lutR[mapMax.red]   >> shiftR;
          mapMax.green = lutG[mapMax.green] >> shiftG;
          mapMax.blue  = lutB[mapMax.blue]  >> shiftB;
          int safeMask = 0;
          if (scratchMax.red   + mapMax.red   > 255) safeMask |= safeR;
          if (scratchMax.green + mapMax.green > 255) safeMask |= safeG;
          if (scratchMax.blue  + mapMax.blue  > 255) safeMask |= safeB;
          MultiplyAddProc maProc = maProcs[safeMask];
          maProc (scratchPtr, scratchPitch, mapPtr, mapPitch,
            mapW, lines, lutR, lutG, lutB);

          if (safeMask == 0)
            scratchMax.UnsafeAdd (mapMax);
          else
            scratchMax.SafeAdd (mapMax);
        }

        tex->GetTextureHandle ()->Blit (tileRect.xmin, 
          tileRect.ymin, 
          tileRect.Width(), tileRect.Height(),
          (uint8*)scratch.GetArray(),
          iTextureHandle::BGRA8888);
      }
    }
    state.Reset (stateDirty);
    dirtyLights.DeleteAll ();
    tilesDirty.Clear ();
  }
}

void ProctexPDLight::DisconnectAllLights ()
{ 
  lights.DeleteAll ();
  lightBits.SetSize (0); 
  tilesDirty.Clear ();
  tilesDirty.FlipAllBits ();
  dirtyLights.DeleteAll ();
}

void ProctexPDLight::LightChanged (iLight* light) 
{ 
  dirtyLights.Add (light);
  state.Set (stateDirty); 
}

void ProctexPDLight::LightDisconnect (iLight* light)
{
  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    if (lights[i].light == light)
    {
      lights.DeleteIndexFast (i);
      state.Set (stateDirty);
      dirtyLights.Add (light);
      lightBits.SetSize (lights.GetSize ());
      return;
    }
  }
}

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)
