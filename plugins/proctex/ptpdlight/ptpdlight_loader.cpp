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

#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/verbositymanager.h"
#include "imap/loader.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "itexture/itexloaderctx.h"

#include "csutil/cfgacc.h"
#include "csutil/cscolor.h"
#include "csutil/processorspecdetection.h"

#include "ptpdlight.h"
#include "ptpdlight_loader.h"

// Plugin stuff



CS_PLUGIN_NAMESPACE_BEGIN(PTPDLight)
{

SCF_IMPLEMENT_FACTORY(ProctexPDLightLoader)

ProctexPDLightLoader::ProctexPDLightLoader (iBase *p) :
  scfImplementationType(this, p), doMMX (false)
{
  InitTokenTable (tokens);
}

ProctexPDLightLoader::~ProctexPDLightLoader ()
{
}

bool ProctexPDLightLoader::Initialize(iObjectRegistry *object_reg)
{
  ProctexPDLightLoader::object_reg = object_reg;

  csConfigAccess cfg (object_reg);
  if (cfg->GetBool ("Texture.PTPDLight.UseScheduling", true))
  {
    sched.SetBudget (cfg->GetInt ("Texture.PTPDLight.TimePerFrame", 25));
  }

  CS::Platform::ProcessorSpecDetection procSpec;
#ifdef CS_SUPPORTS_MMX
  doMMX = procSpec.HasMMX()
    && cfg->GetBool ("Texture.PTPDLight.UseMMX", true);
#endif
  csRef<iVerbosityManager> verbosemgr (
    csQueryRegistry<iVerbosityManager> (object_reg));
  if (verbosemgr && verbosemgr->Enabled ("proctex.pdlight")) 
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, 0,
      "PD light texture computation implementation: %s",
      doMMX ? "MMX" : "generic");
  }

  return true;
}

csPtr<iBase> ProctexPDLightLoader::Parse (iDocumentNode* node,  
				          iStreamSource*, iLoaderContext* /*ldr_context*/,
  					      iBase* context)
{
  csRef<iLoader> LevelLoader = csQueryRegistry<iLoader> (object_reg);
  if (!LevelLoader) 
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 0, "No level loader");
    return 0;
  }
  csRef<iSyntaxService> synsrv = csQueryRegistry<iSyntaxService> (object_reg);
  if (!synsrv) 
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 0, "No syntax services");
    return 0;
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
    pt.AttachNew (new ProctexPDLight (this, img));
  else
  {
    if (ctx)
    {
      if (ctx->HasSize())
      {
	  int w, h;
	  ctx->GetSize (w, h);
        pt.AttachNew (new ProctexPDLight (this, w, h));
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

  if (ctx && ctx->HasFlags())
    pt->SetTexFlags (ctx->GetFlags ());

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
            ParseMap (child, pt, LevelLoader);
            break;
          case XMLTOKEN_BASECOLOR:
            {
              csColor col;
              if (synsrv->ParseColor (child, col))
              {
                csRGBcolor baseColor;
                baseColor.Set (int (col.red * 255.99f), 
                  int (col.green * 255.99f), 
                  int (col.blue * 255.99f));
                pt->SetBaseColor (baseColor);
              }
            }
            break;
          default:
            synsrv->ReportBadToken (child);
            return 0;
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

bool ProctexPDLightLoader::Scheduler::UpdatePT (ProctexPDLight* texture, 
                                                csTicks time)
{
  if (currentPT == texture) return true;

  // New frame: update state
  if (time != lastFrameTime)
  {
    lastFrameTime = time;
    frameNumber--;
    thisFrameUsedTime = 0;
  }
  // Other textures are enqueued, animate these first
  while ((!queue.IsEmpty()) && (thisFrameUsedTime < timeBudget))
  {
    QueueItem item (queue.Pop());
    ProctexPDLight* queuedTex = item.tex;

    queuedPTs.Delete (queuedTex);

    // The texture to animate is in the queue: so jump out and let it do
    if (queuedTex == texture)
      return true;

    /* Otherwise, animate. Will call RecordUpdateTime() which updates the 
     * time spent */
    currentPT = queuedTex;
    /* This will recursively call UpdatePT(), so track the current PT to 
     * quickly exit. */
    queuedTex->Animate (time);
    currentPT = 0;
  }

  if (thisFrameUsedTime < timeBudget)
    // Still have time left
    return true;
  else
  {
    // Run in a future frame.
    if (!queuedPTs.Contains (texture))
    {
      QueueItem newItem;
      newItem.prio = frameNumber;
      newItem.tex = texture;
      queue.Insert (newItem);
      queuedPTs.AddNoTest (texture);
    }
  }

  return false;
}

void ProctexPDLightLoader::Scheduler::UnqueuePT (ProctexPDLight* texture)
{
  while (queue.Delete (texture)) {}
}

bool ProctexPDLightLoader::ParseMap (iDocumentNode* node, ProctexPDLight* pt,
                                     iLoader* LevelLoader)
{
  const char* sector = node->GetAttributeValue ("lightsector");
  const char* lightName = node->GetAttributeValue ("lightname");
  bool hasSector = sector && *sector;
  bool hasLightName = lightName && *lightName;
  if ((hasSector || hasLightName) && (!hasSector || !hasLightName))
  {
    Report (CS_REPORTER_SEVERITY_WARNING, node, 
      "Both 'lightsector' and 'lightname' attributes need to be specified");
    return false;
  }
  const char* lightId = node->GetAttributeValue ("lightid");
  bool hasLightID = lightId && *lightId;
  if (!hasSector && !hasLightName && !hasLightID)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, node, 
      "'lightsector' and 'lightname' attributes or a 'lightid' attribute "
      "need to be specified");
    return false;
  }
  const char* image = node->GetContentsValue ();
  csRef<iImage> map = LevelLoader->LoadImage (image, 
    CS_IMGFMT_ANY);
  if (!map)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, node, 
      "Couldn't load image '%s'", image);
    return false;
  }
  ProctexPDLight::MappedLight light (pt->NewLight (map));;
  light.lightId = new ProctexPDLight::LightIdentity;
  if (hasSector && hasLightName)
  {
    light.lightId->sectorName = sector;
    light.lightId->lightName = lightName;
  }
  else
  {
    if (!HexToLightID (light.lightId->lightId, lightId))
    {
      Report (CS_REPORTER_SEVERITY_WARNING, node, 
        "Invalid light ID '%s'", lightId);
    }
    return false;
  }
  const char* err = pt->AddLight (light);
  if (err != 0)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, node, 
      "Couldn't add map '%s' for light '%s': %s", image, lightId, err);
  }
  
  return !err;
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

bool ProctexPDLightLoader::HexToLightID (uint8* lightID, const char* lightIDHex)
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
      lightID[i] = v;
    }
  }
  return valid;
}

}
CS_PLUGIN_NAMESPACE_END(PTPDLight)
