/*
    Copyright (C) 2003-2006 by Jorrit Tyberghein
	      (C) 2003-2006 by Frank Richter

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

#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/texture.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "imap/loader.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "itexture/itexloaderctx.h"
#include "csgeom/math.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "csutil/scf.h"

#include "ptpdlight.h"

// Plugin stuff

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

SCF_IMPLEMENT_FACTORY(ProctexPDLightLoader)


//----------------------------------------------------------------------------

ProctexPDLightLoader::ProctexPDLightLoader (iBase *p) :
  scfImplementationType(this, p)
{
}

ProctexPDLightLoader::~ProctexPDLightLoader ()
{
}

bool ProctexPDLightLoader::Initialize(iObjectRegistry *object_reg)
{
  ProctexPDLightLoader::object_reg = object_reg;

  return true;
}

csPtr<iBase> ProctexPDLightLoader::Parse (iDocumentNode* node,  
				          iStreamSource*,
					  iLoaderContext* /*ldr_context*/,
  					  iBase* context)
{
  csRef<iLoader> LevelLoader = csQueryRegistry<iLoader> (object_reg);
  if (!LevelLoader) 
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 0, "No level loader");
    return false;
  }

  csRef<iTextureLoaderContext> ctx;
  if (context)
  {
    ctx = csPtr<iTextureLoaderContext>
      (scfQueryInterface<iTextureLoaderContext> (context));
  }

  csRef<iImage> img = (ctx && ctx->HasImage()) ? ctx->GetImage() : 0;
  if (!img)
  {
    csRef<iDocumentNode> file = node ? node->GetNode ("file") : 0;
    if (!file) 
    {
      Report (CS_REPORTER_SEVERITY_WARNING, node, 
	"Please provide a <file> node in the <texture> or <params> block");
      return 0;
    }
    const char* fname;
    if (!(fname = file->GetContentsValue())) 
    {
      Report (CS_REPORTER_SEVERITY_WARNING, file, "Empty <file> node");
      return 0;
    }

    img = LevelLoader->LoadImage (fname);
    if (!img) 
    {
      Report (CS_REPORTER_SEVERITY_WARNING, file, 
	"Couldn't load image '%s'", fname);
      return 0;
    }
  }

  csRef<ProctexPDLight> pt;
  pt.AttachNew (new ProctexPDLight (img));
  if (pt->Initialize (object_reg))
  {
    csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
    if (!G3D) return 0;
    csRef<iTextureManager> tm = G3D->GetTextureManager();
    if (!tm) return 0;
    int texFlags = (ctx && ctx->HasFlags()) ? ctx->GetFlags() : CS_TEXTURE_3D;
    csRef<iTextureHandle> TexHandle (tm->RegisterTexture (img, 
      texFlags));
    if (!TexHandle) return 0;

    pt->GetTextureWrapper()->SetTextureHandle (TexHandle);

    if (node)
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ();
      while (nodes->HasNext())
      {
        csRef<iDocumentNode> child = nodes->Next ();
        if (child->GetType() != CS_NODE_ELEMENT) continue;
        if (strcmp (child->GetValue(), "map") == 0)
        {
          const char* lightId = child->GetAttributeValue ("lightid");
          const char* image = child->GetContentsValue ();
          csRef<iImage> map = LevelLoader->LoadImage (image, 
            CS_IMGFMT_TRUECOLOR);
          if (!map)
          {
            Report (CS_REPORTER_SEVERITY_WARNING, child, 
	      "Couldn't load image '%s'", image);
            return 0;
          }
          ProctexPDLight::MappedLight light;
          light.map = map;
          light.lightId = new csString (lightId);
          const char* err = pt->AddLight (light);
          if (err != 0)
          {
            Report (CS_REPORTER_SEVERITY_WARNING, child, 
              "Couldn't add map '%s' for light '%s': %s", image, lightId, err);
          }
        }
      }
    }

    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();
    return csPtr<iBase> (tw);
  }
  return 0;
}

void ProctexPDLightLoader::Report (int severity, iDocumentNode* node,
				   const char* msg, ...)
{
  static const char msgId[] = "crystalspace.proctex.loader.pdlight";

  va_list arg;
  va_start (arg, msg);

  csRef<iSyntaxService> synserv;

  if (node)
    synserv = csQueryRegistry<iSyntaxService> (object_reg);

  if (node && synserv)
  {
    csString text;
    text.FormatV (msg, arg);
    synserv->Report (msgId, severity, node, "%s", (const char*)text);
  }
  else
  {
    csReportV (object_reg, severity, msgId, msg, arg);
  }

  va_end (arg);
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_STATIC_CLASSVAR_REF(ProctexPDLight, lightmapScratch, GetScratch,
                                 ProctexPDLight::LightmapScratch, ());

void ProctexPDLight::PDMap::ComputeValueBounds ()
{
  csRect r (0, 0, 
    image ? image->GetWidth() : 0, 
    image ? image->GetHeight() : 0);
  ComputeValueBounds (r);
}

void ProctexPDLight::PDMap::ComputeValueBounds (const csRect& area)
{
  maxValue.Set (0, 0, 0);
  nonNullArea.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  if (!image) return;

  const int width = image->GetWidth();
  csRGBpixel* map = ((csRGBpixel*)image->GetImageData()) +
    area.ymin * width + area.xmin;
  int mapPitch = width - area.Width ();
  for (int y = area.ymin; y < area.ymax; y++)
  {
    for (int x = area.xmin; x < area.xmax; x++)
    {
      const csRGBpixel& p = *map++;

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
}

void ProctexPDLight::Report (int severity, const char* msg, ...)
{
  static const char msgId[] = "crystalspace.proctex.pdlight";

  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, msgId, msg, arg);
  va_end (arg);
}

bool ProctexPDLight::HexToLightID (char* lightID, const csString& lightIDHex)
{
  bool valid = lightIDHex.Length() == 32;
  if (valid)
  {
    for (size_t i = 0; i < 16; i++)
    {
      uint8 v;
      char digit16 = lightIDHex[i*2];
      char digit1 = lightIDHex[i*2+1];

      if ((digit16 >= '0') && (digit16 <= '9'))
        v = 16*(digit16-'0');
      else if ((digit16 >= 'a') && (digit16 <= 'f'))
        v = 16*(digit16-'a'+10);
      else if ((digit16 >= 'A') && (digit16 <= 'F'))
        v = 16*(digit16-'A'+10);
      else
      {
        valid = false; 
        break;
      }

      if ((digit1 >= '0') && (digit1 <= '9'))
        v += (digit1-'0');
      else if ((digit1 >= 'a') && (digit1 <= 'f'))
        v += (digit1-'a'+10);
      else if ((digit1 >= 'A') && (digit1 <= 'F'))
        v += (digit1-'A'+10);
      else
      {
        valid = false; 
        break;
      }
      lightID[i] = char (v);
    }
  }
  if (valid)
    return true;
  else
  {
    Report (CS_REPORTER_SEVERITY_WARNING, 
      "Invalid light ID: '%s'", lightIDHex.GetData());
    return false;
  }
}

void ProctexPDLight::UpdateAffectedArea ()
{
  if (!state.Check (stateAffectedAreaDirty)) return;

  totalAffectedAreas.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    totalAffectedAreas.Union (lights[i].map.nonNullArea);
  }
  lightmapSize = csMax (totalAffectedAreas.Width(), 0) * 
    csMax (totalAffectedAreas.Height(), 0);
  if (lightmapSize > 0)
    baseMap.ComputeValueBounds (totalAffectedAreas);

  state.Reset (stateAffectedAreaDirty);
}

const char* ProctexPDLight::AddLight (const MappedLight& light)
{
  if ((light.map->GetWidth() != baseMap->GetWidth())
    || (light.map->GetHeight() != baseMap->GetHeight()))
    return "PD lightmap dimensions don't correspond to base lightmap dimensions";
  lights.Push (light);
  state.Set (stateAffectedAreaDirty);
  return 0;
}

ProctexPDLight::ProctexPDLight (iImage* img) : 
  scfImplementationType (this, (iTextureFactory*)0, img), baseMap (img), 
  state (stateAffectedAreaDirty | stateDirty)
{
  if (img)
  {
    mat_w = img->GetWidth();
    mat_h = img->GetHeight();
  }
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

  if (!baseMap) return false;
  lightmapSize = baseMap->GetWidth() * baseMap->GetHeight();
  for (size_t i = 0; i < lights.GetSize(); )
  {
    MappedLight& light = lights[i];
    char lightID[16];
    bool success = false;
    if (HexToLightID (lightID, *light.lightId))
    {
      light.light = engine->FindLightID (lightID);
      if (light.light)
      {
        success = true;
        light.light->AddAffectedLightingInfo (
          static_cast<iLightingInfo*> (this));
      }
      else
      {
        Report (CS_REPORTER_SEVERITY_WARNING, 
          "Could not find light with ID '%s'", light.lightId->GetData());
      }
    }
    delete light.lightId;
    if (success)
    {
      i++;
    }
    else
      lights.DeleteIndexFast (i);
  }
  lights.ShrinkBestFit();
  state.Set (statePrepared);
  return true;
}

void ProctexPDLight::Animate (csTicks /*current_time*/)
{
  if (state.Check (stateDirty))
  {
    UpdateAffectedArea ();
    if (lightmapSize > 0)
    {
      int scratchW = totalAffectedAreas.Width();
      LightmapScratch& scratch = GetScratch();
      scratch.SetSize (lightmapSize);
      {
        csRGBpixel* basePtr = ((csRGBpixel*)baseMap->GetImageData()) +
          totalAffectedAreas.ymin * mat_w + totalAffectedAreas.xmin;
        int lines = totalAffectedAreas.Height();
        csRGBpixel* scratchPtr = scratch.GetArray();
        for (int y = 0; y < lines; y++)
        {
          memcpy (scratchPtr, basePtr, scratchW * sizeof (csRGBpixel));
          scratchPtr += scratchW;
          basePtr += mat_w;
        }
      }
      csRGBcolor scratchMax = baseMap.maxValue;

      for (size_t i = 0; i < lights.GetSize(); )
      {
        MappedLight& light = lights[i];
        if (!light.light)
        {
          lights.DeleteIndexFast (i);
          state.Set (stateAffectedAreaDirty);
          continue;
        }
        else
          i++;
        const csColor& lightColor = light.light->GetColor ();

        int mapW = light.map.nonNullArea.Width();
        const csRGBpixel* mapPtr = ((csRGBpixel*)light.map->GetImageData()) +
          light.map.nonNullArea.ymin * mat_w +
          light.map.nonNullArea.xmin;
        int lines = light.map.nonNullArea.Height();
        int mapPitch = mat_w - mapW;

        csRGBpixel* scratchPtr = scratch.GetArray() + 
          (light.map.nonNullArea.ymin - totalAffectedAreas.ymin) * scratchW +
           light.map.nonNullArea.xmin - totalAffectedAreas.xmin;
        int scratchPitch = scratchW - mapW;

        csRGBcolor mapMax = light.map.maxValue;
        mapMax.red = int (mapMax.red * lightColor.red);
        mapMax.green = int (mapMax.green * lightColor.green);
        mapMax.blue = int (mapMax.blue * lightColor.blue);
        if ((scratchMax.red + mapMax.red <= 255)
          && (scratchMax.green + mapMax.green <= 255)
          && (scratchMax.blue + mapMax.blue <= 255))
        {
          // Safe to add values w/o overflow check
          for (int y = 0; y < lines; y++)
          {
            for (int x = 0; x < mapW; x++)
            {
              csRGBpixel mappedColor (int (lightColor.red * mapPtr->red),
                int (lightColor.green * mapPtr->green),
                int (lightColor.blue * mapPtr->blue));
              scratchPtr->UnsafeAdd (mappedColor);
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
              csRGBpixel mappedColor (int (lightColor.red * mapPtr->red),
                int (lightColor.green * mapPtr->green),
                int (lightColor.blue * mapPtr->blue));
              scratchPtr->SafeAdd (mappedColor);
              scratchPtr++;
              mapPtr++;
            }
            scratchPtr += scratchPitch;
            mapPtr += mapPitch;
          }
        }
        scratchMax.SafeAdd (mapMax);
      }

      tex->GetTextureHandle ()->Blit (totalAffectedAreas.xmin, 
        totalAffectedAreas.ymin, 
        totalAffectedAreas.Width(), totalAffectedAreas.Height(),
        (uint8*)scratch.GetArray());
    }

    state.Reset (stateDirty);
  }
}

void ProctexPDLight::LightDisconnect (iLight* light)
{
  for (size_t i = 0; i < lights.GetSize(); i++)
  {
    if (lights[i].light == light)
    {
      lights.DeleteIndexFast (i);
      state.Set (stateAffectedAreaDirty);
      return;
    }
  }
}

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)
