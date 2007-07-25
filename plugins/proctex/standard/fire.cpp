/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "itexture/itexloaderctx.h"
#include "imap/services.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"

#include "csgfx/gradient.h"
#include "csutil/scf.h"

#include "prfire.h"
#include "stdproctex.h"
#include "fire.h"

SCF_IMPLEMENT_FACTORY(csPtFireType)
SCF_IMPLEMENT_FACTORY(csPtFireLoader)
SCF_IMPLEMENT_FACTORY(csPtFireSaver)

#define CLASSID_FIRETYPE "crystalspace.texture.type.fire"

csPtFireType::csPtFireType (iBase* p) :
  scfImplementationType(this, p)
{
}

csPtr<iTextureFactory> csPtFireType::NewFactory()
{
  return csPtr<iTextureFactory> (new csPtFireFactory (
    this, object_reg));
}

//---------------------------------------------------------------------------
// 'Fire' PT factory

csPtFireFactory::csPtFireFactory (iTextureType* p, iObjectRegistry* object_reg) :
  scfImplementationType(this, p, object_reg)
{
}

csPtr<iTextureWrapper> csPtFireFactory::Generate ()
{
  csRef<csProcTexture> pt = 
    csPtr<csProcTexture> (new csProcFire (this, width, height));

  if (pt->Initialize (object_reg))
  {
    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();
    return csPtr<iTextureWrapper> (tw);
  }

  return 0;
}

//---------------------------------------------------------------------------
// 'Fire' loader.

csPtFireLoader::csPtFireLoader(iBase *p) :
  scfImplementationType(this, p)
{
  InitTokenTable (tokens);
}

csPtr<iBase> csPtFireLoader::Parse (iDocumentNode* node, 
				    iStreamSource*, iLoaderContext* /*ldr_context*/,
  				    iBase* context)
{
  /*
    Going through the plugin manager to retrieve the texture type
    isn't really necessary here, as we could just instantiate csPtFireType
    with new. It's just an 'exercise'.
   */
  csRef<iTextureType> type = csLoadPluginCheck<iTextureType> (
  	object_reg, CLASSID_FIRETYPE);
  if (!type) return 0;
  csRef<iSyntaxService> synldr = 
    csQueryRegistry<iSyntaxService> (object_reg);

  csRef<iTextureFactory> fireFact = type->NewFactory();

  csRef<iTextureLoaderContext> ctx;
  if (context)
  {
    ctx = csPtr<iTextureLoaderContext>
      (scfQueryInterface<iTextureLoaderContext> (context));

    if (ctx)
    {
      if (ctx->HasSize())
      {
	int w, h;
	ctx->GetSize (w, h);
	fireFact->SetSize (w, h);
      }
    }
  }
  csRef<iTextureWrapper> tex = fireFact->Generate();
  csRef<iFireTexture> fire = scfQueryInterface<iFireTexture> (tex);

  if (node)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      csStringID id = tokens.Request (child->GetValue ());
      switch (id)
      {
	case XMLTOKEN_POSSBURN:
	  fire->SetPossibleBurn (child->GetContentsValueAsInt());
	  break;
	case XMLTOKEN_ADDBURN:
	  fire->SetAdditionalBurn (child->GetContentsValueAsInt());
	  break;
	case XMLTOKEN_CONTBURN:
	  fire->SetContinuedBurn (child->GetContentsValueAsInt());
	  break;
	case XMLTOKEN_SMOOTHING:
	  fire->SetSmoothing (child->GetContentsValueAsInt());
	  break;
	case XMLTOKEN_EXTINGUISH:
	  fire->SetExtinguish (child->GetContentsValueAsInt());
	  break;
	case XMLTOKEN_SINGLEFLAME:
	  bool res;
	  if (synldr && synldr->ParseBool (child, res, true))
	    fire->SetSingleFlameMode (res);
	  break;
	case XMLTOKEN_HALFBASE:
	  fire->SetHalfBase (child->GetContentsValueAsInt());
	  break;
	case XMLTOKEN_POSTSMOOTH:
	  fire->SetPostSmoothing (child->GetContentsValueAsInt());
	  break;
	case XMLTOKEN_PALETTE:
	  {
	    if (!synldr) return 0;

	    csRef<iGradient> grad;
            grad.AttachNew (new csGradient);
	    if (!synldr->ParseGradient (child, grad))
	    {
	      return 0;
	    }
	    fire->SetPalette (grad);
	  }
	  break;
	default:
	  if (synldr) synldr->ReportBadToken (child);
	  return 0;
      };
    }
  }

  csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
  if (!G3D) return 0;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return 0;
  tex->Register (tm);

  return csPtr<iBase> (tex);
}

//---------------------------------------------------------------------------
// 'Fire' saver.

csPtFireSaver::csPtFireSaver (iBase* p) :
  scfImplementationType(this, p)
{
}

bool csPtFireSaver::WriteDown (iBase* /*obj*/, iDocumentNode* /*parent*/,
	iStreamSource*)
{
  return true;
}
