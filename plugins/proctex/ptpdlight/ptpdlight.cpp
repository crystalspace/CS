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
#include <limits.h>

#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/texture.h"
#include "csutil/scfstr.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "imap/loader.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "itexture/itexloaderctx.h"
#include "csgeom/fixed.h"
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
  InitTokenTable (tokens);
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
  csRef<iSyntaxService> synsrv = csQueryRegistry<iSyntaxService> (object_reg);
  if (!synsrv) 
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 0, "No syntax services");
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
    csRef<iDocumentNode> file = 
      node ? node->GetNode ("file") : csRef<iDocumentNode> (0);
    if (file) 
    {
      const char* fname;
      if (!(fname = file->GetContentsValue())) 
      {
        Report (CS_REPORTER_SEVERITY_WARNING, file, "Empty <file> node");
      }
      else
      {
        img = LevelLoader->LoadImage (fname);
        if (!img) 
        {
          Report (CS_REPORTER_SEVERITY_WARNING, file, 
	    "Couldn't load image '%s'", fname);
        }
      }
    }
  }

  csRef<ProctexPDLight> pt;
  if (img.IsValid())
    pt.AttachNew (new ProctexPDLight (img));
  else
  {
    if (ctx)
    {
      if (ctx->HasSize())
      {
	  int w, h;
	  ctx->GetSize (w, h);
        pt.AttachNew (new ProctexPDLight (w, h));
      }
    }
  }
  if (!pt.IsValid())
  {
    Report (CS_REPORTER_SEVERITY_WARNING, node, 
      "Please provide a <file> node in the <texture> or <params> block or "
      "a <size> node in the <texture> block");
    return 0;
  }

  if (pt->Initialize (object_reg))
  {
    if (node)
    {
      csRef<iDocumentNodeIterator> nodes = node->GetNodes ();
      while (nodes->HasNext())
      {
        csRef<iDocumentNode> child = nodes->Next ();
        if (child->GetType() != CS_NODE_ELEMENT) continue;
        csStringID id = tokens.Request (child->GetValue());
        switch (id)
        {
          case XMLTOKEN_MAP:
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
              light.lightId = new char[16];
              if (!HexToLightID (light.lightId, lightId))
              {
                Report (CS_REPORTER_SEVERITY_WARNING, child, 
                  "Invalid light ID '%s'", lightId);
              }
              else
              {
                const char* err = pt->AddLight (light);
                if (err != 0)
                {
                  Report (CS_REPORTER_SEVERITY_WARNING, child, 
                    "Couldn't add map '%s' for light '%s': %s", image, lightId, err);
                }
              }
            }
            break;
          case XMLTOKEN_BASECOLOR:
            {
              csColor col;
              if (synsrv->ParseColor (child, col))
              {
                csRGBcolor baseColor;
                baseColor.Set (col.red * 255.99f, col.green * 255.99f, 
                  col.blue * 255.99f);
                pt->SetBaseColor (baseColor);
              }
            }
            break;
          default:
            synsrv->ReportBadToken (child);
            return false;
        }
      }
    }
    pt->FinishLoad();
    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();

    csRef<iTextureManager> tm = pt->GetG3D()->GetTextureManager();
    if (!tm) return 0;
    tw->Register (tm);

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

bool ProctexPDLightLoader::HexToLightID (char* lightID, const char* lightIDHex)
{
  bool valid = strlen (lightIDHex) == 32;
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
  return valid;
}

//---------------------------------------------------------------------------

CS_IMPLEMENT_STATIC_CLASSVAR_REF(ProctexPDLight, lightmapScratch, GetScratch,
                                 ProctexPDLight::LightmapScratch, ());

void ProctexPDLight::PDMap::ComputeValueBounds ()
{
  csRect r (0, 0, imageW, imageH);
  ComputeValueBounds (r);
}

void ProctexPDLight::PDMap::ComputeValueBounds (const csRect& area)
{
  maxValue.Set (0, 0, 0);
  nonNullArea.Set (INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  if (!imageData) return;

  const int width = imageW;
  const Lumel* map = imageData->GetData() + area.ymin * width + area.xmin;
  int mapPitch = width - area.Width ();
  for (int y = area.ymin; y < area.ymax; y++)
  {
    for (int x = area.xmin; x < area.xmax; x++)
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
}

void ProctexPDLight::PDMap::SetImage (iImage* img)
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
  ComputeValueBounds (); 
}

void ProctexPDLight::Report (int severity, const char* msg, ...)
{
  static const char msgId[] = "crystalspace.proctex.pdlight";

  va_list arg;
  va_start (arg, msg);
  csReportV (object_reg, severity, msgId, msg, arg);
  va_end (arg);
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
  if ((light.map.imageW != mat_w)
    || (light.map.imageH != mat_h))
    return "PD lightmap dimensions don't correspond to base lightmap dimensions";

  if (light.map.nonNullArea.IsEmpty ())
    return 0; //Silently ignore totally black maps

  lights.Push (light);
  state.Set (stateAffectedAreaDirty);
  return 0;
}

ProctexPDLight::ProctexPDLight (iImage* img) : 
  scfImplementationType (this, (iTextureFactory*)0, img), baseMap (img), 
  state (stateAffectedAreaDirty | stateDirty), baseColor (0, 0, 0)
{
  mat_w = img->GetWidth();
  mat_h = img->GetHeight();
}

ProctexPDLight::ProctexPDLight (int w, int h) : 
  scfImplementationType (this), state (stateAffectedAreaDirty | stateDirty), 
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
      csRGBcolor scratchMax;
      {
        int lines = totalAffectedAreas.Height();
        Lumel* scratchPtr = scratch.GetArray();
        if (baseMap.imageData.IsValid())
        {
          Lumel* basePtr = (baseMap.imageData->GetData()) +
            totalAffectedAreas.ymin * mat_w + totalAffectedAreas.xmin;
          for (int y = 0; y < lines; y++)
          {
            memcpy (scratchPtr, basePtr, scratchW * sizeof (Lumel));
            scratchPtr += scratchW;
            basePtr += mat_w;
          }
          scratchMax = baseMap.maxValue;
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
        csFixed16 lightR = light.light->GetColor ().red;
        csFixed16 lightG = light.light->GetColor ().green;
        csFixed16 lightB = light.light->GetColor ().blue;

        int mapW = light.map.nonNullArea.Width();
        const Lumel* mapPtr = (light.map.imageData->GetData()) +
          light.map.nonNullArea.ymin * mat_w +
          light.map.nonNullArea.xmin;
        int lines = light.map.nonNullArea.Height();
        int mapPitch = mat_w - mapW;

        Lumel* scratchPtr = scratch.GetArray() + 
          (light.map.nonNullArea.ymin - totalAffectedAreas.ymin) * scratchW +
           light.map.nonNullArea.xmin - totalAffectedAreas.xmin;
        int scratchPitch = scratchW - mapW;

        csRGBcolor mapMax = light.map.maxValue;
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

      tex->GetTextureHandle ()->Blit (totalAffectedAreas.xmin, 
        totalAffectedAreas.ymin, 
        totalAffectedAreas.Width(), totalAffectedAreas.Height(),
        (uint8*)scratch.GetArray(),
        iTextureHandle::BGRA8888);
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
      state.Set (stateDirty);
      return;
    }
  }
}

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)
