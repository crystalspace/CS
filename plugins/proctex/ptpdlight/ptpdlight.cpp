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

#include "iengine/engine.h"
#include "iengine/sector.h"
#include "ivaria/profile.h"
#include "ivaria/reporter.h"

#include "csgeom/math.h"
#include "csgfx/imageautoconvert.h"
#include "csutil/stringquote.h"

#include "ptpdlight.h"
#include "ptpdlight_loader.h"

CS_DECLARE_PROFILER
CS_DECLARE_PROFILER_ZONE(ProctexPDLight_Animate)

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

void ProctexPDLight::PDMap::ComputeValueBounds (const csRect& area, 
                                                csRGBcolor& maxValue, 
                                                csRect& nonNullArea)
{
  const int width = imageW;
  maxValue.Set (0, 0, 0);
  nonNullArea.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  
  int mapPitch = width - area.Width ();
  if (imageData->IsGray())
  {
    const uint8* map = 
      static_cast<LumelBufferGray*> ((LumelBufferBase*)imageData)->GetData() 
      + area.ymin * width + area.xmin;
    for (int y = area.ymin; y < area.ymax; y++)
    {
      for (int x = area.xmin; x < area.xmax; x++)
      {
	uint8 v = *map++;
  
	if (v > maxValue.red)
          maxValue.Set (v, v, v);
  
	if (v > 0)
	{
	  nonNullArea.Extend (x, y);
	}
      }
      map += mapPitch;
    }
  }
  else
  {
    const Lumel* map = 
      static_cast<LumelBufferRGB*> ((LumelBufferBase*)imageData)->GetData() 
      + area.ymin * width + area.xmin;
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

  // Align line start+end on 2-pixel-boundary (for MMX)
  nonNullArea.xmin &= ~1;
  nonNullArea.xmax = (nonNullArea.xmax+1) & ~1;
}

void ProctexPDLight::PDMap::ComputeValueBounds (const TileHelper& tiles)
{
  csRect r (0, 0, imageW, imageH);
  ComputeValueBounds (tiles, r);
}

void ProctexPDLight::PDMap::ComputeValueBounds (const TileHelper& tiles, 
                                                const csRect& area)
{
  nonNullAreas.SetSize (tileNonNull.GetSize());
  if (!imageData) 
  {
    for (size_t t = 0; t < tileNonNull.GetSize(); t++)
    {
      this->tiles[t].maxValue.Set (0, 0, 0);
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
      this->tiles[t].maxValue.Set (0, 0, 0);
      nonNullAreas[t].Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
      continue;
    }

    csRGBcolor maxValue;
    csRect nonNullArea;
    ComputeValueBounds (r, maxValue, nonNullArea);
    this->tiles[t].maxValue = maxValue;
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

#include "csutil/custom_new_disable.h"

void ProctexPDLight::PDMap::SetImage (const TileHelper& tiles, iImage* img)
{
  imageW = img->GetWidth();
  imageH = img->GetHeight();
  size_t numPixels = imageW * imageH;
  if ((img->GetFormat() & CS_IMGFMT_MASK) == CS_IMGFMT_PALETTED8)
  {
    // Assume grayscale map
    LumelBufferGray* imageDataGray = new (numPixels) LumelBufferGray;
    imageData.AttachNew (imageDataGray);
    const uint8* src = (uint8*)img->GetImageData();
    uint8* dst = imageDataGray->GetData();
    memcpy (dst, src, numPixels);
  }
  else
  {
    LumelBufferRGB* imageDataRGB = new (numPixels) LumelBufferRGB;
    imageData.AttachNew (imageDataRGB);
    const csRGBpixel* src = (csRGBpixel*)img->GetImageData();
    Lumel* dst = imageDataRGB->GetData();
    while (numPixels-- > 0)
    {
      dst->c.red = src->red;
      dst->c.green = src->green;
      dst->c.blue = src->blue;
      dst->c.alpha = 0;
      dst++;
      src++;
    }
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

  if (imageData->IsGray())
  {
    csRef<LumelBufferGray> newData;
    newData.AttachNew (
      new (nonNullArea.Width() * nonNullArea.Height()) LumelBufferGray);
    const uint8* srcMap = 
      static_cast<LumelBufferGray*> ((LumelBufferBase*)imageData)->GetData() 
      + nonNullArea.ymin * imageW + nonNullArea.xmin;
    int rowSize = nonNullArea.Width ();
    uint8* dstMap = newData->GetData();
    for (int y = nonNullArea.Height(); y-- > 0; )
    {
      memcpy (dstMap, srcMap, rowSize);
      dstMap += rowSize;
      srcMap += imageW;
    }
  
    imageData = newData;
  }
  else
  {
    csRef<LumelBufferRGB> newData;
    newData.AttachNew (
      new (nonNullArea.Width() * nonNullArea.Height()) LumelBufferRGB);
    const Lumel* srcMap = 
      static_cast<LumelBufferRGB*> ((LumelBufferBase*)imageData)->GetData() 
      + nonNullArea.ymin * imageW + nonNullArea.xmin;
    int rowSize = nonNullArea.Width ();
    Lumel* dstMap = newData->GetData();
    for (int y = nonNullArea.Height(); y-- > 0; )
    {
      memcpy (dstMap, srcMap, rowSize * sizeof (Lumel));
      dstMap += rowSize;
      srcMap += imageW;
    }
  
    imageData = newData;
  }
  imageX = nonNullArea.xmin;
  imageY = nonNullArea.ymin;
  imageW = nonNullArea.Width();
  imageH = nonNullArea.Height();
}

#include "csutil/custom_new_enable.h"

void ProctexPDLight::PDMap::GetMaxValue (csRGBcolor& maxValue)
{
  maxValue.Set (0, 0, 0);
  for (size_t m = 0; m < tiles.GetSize(); m++)
  {
    const csRGBcolor& color = tiles[m].maxValue;
    if (color.red > maxValue.red) maxValue.red = color.red;
    if (color.green > maxValue.green) maxValue.green = color.green;
    if (color.blue > maxValue.blue) maxValue.blue = color.blue;
  }
}

void ProctexPDLight::PDMap::UpdateTiles (const TileHelper& helper)
{
  for (size_t t = 0; t < tiles.GetSize(); t++)
  {
    Tile& tile = tiles[t];
    
    csRect tileRect;
    helper.GetTileRect (t, tileRect);

    const csRect& lightInTile (nonNullAreas[t]);

    tile.tilePartX = (lightInTile.xmin - tileRect.xmin);
    tile.tilePartY = (lightInTile.ymin - tileRect.ymin);

    tile.tilePartW = csMin (lightInTile.xmax, imageX + imageW)
      - csMax (lightInTile.xmin, imageX);
    tile.tilePartH = csMin (lightInTile.ymax, imageY + imageH)
      - csMax (lightInTile.ymin, imageY);
    tile.tilePartPitch = imageW - tile.tilePartW;

    if (imageData->IsGray())
    {
      tile.tilePartData = static_cast<LumelBufferGray*> (
	  (LumelBufferBase*)imageData)->GetData()
        + (lightInTile.ymin - imageY) * imageW
        + (lightInTile.xmin - imageX);
    }
    else
    {
      tile.tilePartData = static_cast<LumelBufferRGB*> (
	  (LumelBufferBase*)imageData)->GetData()
        + (lightInTile.ymin - imageY) * imageW
        + (lightInTile.xmin - imageX);
    }
  }
  tiles.ShrinkBestFit ();
  nonNullAreas.DeleteAll ();
}

// --------------------------------------------------------------------------

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

ProctexPDLight::ProctexPDLight (ProctexPDLightLoader* loader, iImage* img) : 
  scfImplementationType (this, (iTextureFactory*)0, img), loader (loader),
  tiles (img->GetWidth(), img->GetHeight()),
  tilesDirty (tiles.ComputeTileCount()),
  baseColor (0, 0, 0), baseMap (tilesDirty.GetSize(), tiles, img),
  state (stateDirty)
{
  mat_w = img->GetWidth();
  mat_h = img->GetHeight();
}

ProctexPDLight::ProctexPDLight (ProctexPDLightLoader* loader, int w, int h) : 
  scfImplementationType (this), loader (loader),
  tiles (w, h), tilesDirty (tiles.ComputeTileCount()),
  baseColor (0, 0, 0), baseMap (tilesDirty.GetSize()), 
  state (0)
{
  mat_w = w;
  mat_h = h;
}

ProctexPDLight::~ProctexPDLight ()
{
  loader->UnqueuePT (this);
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

  size_t lightmapSize = mat_w * mat_h;
  if (lightmapSize == 0) return false;
  for (size_t i = 0; i < lights.GetSize(); )
  {
    MappedLight& light = lights[i];
    bool success = false;
    if (!light.lightId->sectorName.IsEmpty()
      && !light.lightId->lightName.IsEmpty())
    {
      iSector* sector = engine->FindSector (light.lightId->sectorName);
      if (sector)
      {
        iLight* engLight = sector->GetLights()->FindByName (
          light.lightId->lightName);
        light.light = engLight;
        if (!light.light)
        {
          Report (CS_REPORTER_SEVERITY_WARNING, 
            "Could not find light %s in sector %s", 
            CS::Quote::Single (light.lightId->lightName.GetData()),
            CS::Quote::Single (light.lightId->sectorName.GetData()));
        }
      }
      else
      {
        Report (CS_REPORTER_SEVERITY_WARNING, 
          "Could not find sector %s for light %s", 
          CS::Quote::Single (light.lightId->sectorName.GetData()),
          CS::Quote::Single (light.lightId->lightName.GetData()));
      }
    }
    else
    {
      light.light = engine->FindLightID ((const char*)light.lightId->lightId);
      if (!light.light)
      {
        csString hexId;
        for (int i = 0; i < 16; i++)
          hexId.AppendFmt ("%02x", light.lightId->lightId[i]);
        Report (CS_REPORTER_SEVERITY_WARNING, 
          "Could not find light with ID %s", CS::Quote::Single (hexId.GetData()));
      }
    }
    if (light.light)
    {
      success = true;
      light.light->SetLightCallback(this);
      dirtyLights.Add ((iLight*)light.light);

      LightColorState colorState;
      colorState.lastColor.Set (0, 0, 0);
      csRGBcolor maxValue;
      light.map.GetMaxValue (maxValue);
      /* When a component changes by this amount the change is visible
       * (assuming 8 bits per component precision). */
      colorState.minChangeThresh.Set (1.0f/maxValue.red, 
        1.0f/maxValue.green,
        1.0f/maxValue.blue);
      lightColorStates.Put ((iLight*)light.light, colorState);
    }
    delete light.lightId; light.lightId = 0;
    if (success)
    {
      i++;
    }
    else
      lights.DeleteIndexFast (i);
  }
  lights.ShrinkBestFit();
  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    lights[i].map.UpdateTiles (tiles);
  }
  lightBits.SetSize (lights.GetSize ());
  state.Set (statePrepared);
  tilesDirty.FlipAllBits();
  
  // Initially fill texture (starts out with garbage)
  Animate();
  
  return true;
}

void ProctexPDLight::OnColorChange (iLight* light, const csColor& newcolor)
{
  dirtyLights.Add (light);
  const LightColorState& colorState = *lightColorStates.GetElementPointer (light);
  /* When the light color difference to the value last used at updating
     is below the amount needed for a visible difference an update of the
     texture because of this light isn't needed. */
  if ((fabsf (colorState.lastColor.red - newcolor.red) 
      >= colorState.minChangeThresh.red)
    || (fabsf (colorState.lastColor.green - newcolor.green) 
      >= colorState.minChangeThresh.green)
    || (fabsf (colorState.lastColor.blue - newcolor.blue) 
      >= colorState.minChangeThresh.blue))
    state.Set (stateDirty); 
}

void ProctexPDLight::OnDestroy (iLight* light)
{
  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    if (lights[i].light == light)
    {
      lights.DeleteIndexFast (i);
      lightColorStates.DeleteAll (light);
      state.Set (stateDirty);
      dirtyLights.Add (light);
      lightBits.SetSize (lights.GetSize ());
      tilesDirty.Clear();
      tilesDirty.FlipAllBits();
      return;
    }
  }
}

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)
