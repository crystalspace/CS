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

  const int width = imageW;
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

    csRGBcolor maxValue (0, 0, 0);
    csRect nonNullArea (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
    const Lumel* map = imageData->GetData() + r.ymin * width + r.xmin;
    int mapPitch = width - r.Width ();
    for (int y = r.ymin; y < r.ymax; y++)
    {
      for (int x = r.xmin; x < r.xmax; x++)
      {
        const Lumel& p = *map++;

        if (p.red > maxValue.red)
          maxValue.red = p.red;
        if (p.green > maxValue.green)
          maxValue.green = p.green;
        if (p.blue > maxValue.blue)
          maxValue.blue = p.blue;

        if (p.red + p.green + p.blue > 0)
        {
          nonNullArea.Extend (x, y);
        }
      }
      map += mapPitch;
    }
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
    dst->red = src->red;
    dst->green = src->green;
    dst->blue = src->blue;
    dst->alpha = 0xff;
    dst++;
    src++;
  }
  ComputeValueBounds (tiles); 
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

  lights.Push (light);
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
            for (int y = 0; y < lines; y++)
            {
              for (int x = 0; x < scratchW; x++)
              {
                scratchPtr->red = baseColor.red;
                scratchPtr->green = baseColor.green;
                scratchPtr->blue = baseColor.blue;
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

          csFixed16 lightR = light.light->GetColor ().red;
          csFixed16 lightG = light.light->GetColor ().green;
          csFixed16 lightB = light.light->GetColor ().blue;

          int mapW = lightInTile.Width();
          const Lumel* mapPtr = (light.map.imageData->GetData()) +
            lightInTile.ymin * mat_w +
            lightInTile.xmin;
          int lines = lightInTile.Height();
          int mapPitch = mat_w - mapW;

          Lumel* scratchPtr = scratch.GetArray() + 
            (lightInTile.ymin - tileRect.ymin) * scratchW +
             lightInTile.xmin - tileRect.xmin;
          int scratchPitch = scratchW - mapW;

          csRGBcolor mapMax = light.map.maxValues[t];
          mapMax.red = int (lightR * int (mapMax.red));
          mapMax.green = int (lightG * int (mapMax.green));
          mapMax.blue = int (lightB * int (mapMax.blue));
          if ((scratchMax.red + mapMax.red <= 255)
            && (scratchMax.green + mapMax.green <= 255)
            && (scratchMax.blue + mapMax.blue <= 255))
          {
            // Safe to add values w/o overflow check
            for (int y = 0; y < lines; y++)
            {
              for (int x = 0; x < mapW; x++)
              {
                scratchPtr->UnsafeAdd (
                  int (lightR * int (mapPtr->red)),
                  int (lightG * int (mapPtr->green)),
                  int (lightB * int (mapPtr->blue)));
                scratchPtr++;
                mapPtr++;
              }
              scratchPtr += scratchPitch;
              mapPtr += mapPitch;
            }
          }
          else
          {
            // Need overflow check for each pixel
            for (int y = 0; y < lines; y++)
            {
              for (int x = 0; x < mapW; x++)
              {
                scratchPtr->SafeAdd (
                  int (lightR * int (mapPtr->red)),
                  int (lightG * int (mapPtr->green)),
                  int (lightB * int (mapPtr->blue)));
                scratchPtr++;
                mapPtr++;
              }
              scratchPtr += scratchPitch;
              mapPtr += mapPitch;
            }
          }
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
