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
#include "csgfx/gradient.h"

#include "prplasma.h"
#include "stdproctex.h"
#include "plasma.h"

SCF_IMPLEMENT_FACTORY(csPtPlasmaType)
SCF_IMPLEMENT_FACTORY(csPtPlasmaLoader)
SCF_IMPLEMENT_FACTORY(csPtPlasmaSaver)

#define CLASSID_PLASMATYPE "crystalspace.texture.type.plasma"

csPtPlasmaType::csPtPlasmaType (iBase* p) : csBaseProctexType(p)
{
}

csPtr<iTextureFactory> csPtPlasmaType::NewFactory()
{
  return csPtr<iTextureFactory> (new csPtPlasmaFactory (
    this, object_reg));
}

//---------------------------------------------------------------------------
// 'Plasma' PT factory

csPtPlasmaFactory::csPtPlasmaFactory (iTextureType* p, iObjectRegistry* object_reg) : 
    csBaseTextureFactory (p, object_reg)
{
}

csPtr<iTextureWrapper> csPtPlasmaFactory::Generate ()
{
  csRef<csProcTexture> pt = 
    csPtr<csProcTexture> (new csProcPlasma (this));

  if (pt->Initialize (object_reg))
  {
    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();
    return csPtr<iTextureWrapper> (tw);
  }

  return 0;
}

//---------------------------------------------------------------------------
// 'Plasma' loader.

csPtPlasmaLoader::csPtPlasmaLoader(iBase *p) : csBaseProctexLoader(p)
{
//  init_token_table (tokens);
}

csPtr<iBase> csPtPlasmaLoader::Parse (iDocumentNode* node, 
				    iLoaderContext* ldr_context,
  				    iBase* context)
{
  /*
    Going through the plugin manager to retrieve the texture type
    isn't really necessary here, as we could just instantiate csPtPlasmaType
    with new. It's just an 'exercise'.
   */
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iTextureType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	CLASSID_PLASMATYPE, iTextureType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, CLASSID_PLASMATYPE,
    	iTextureType);
  }
  csRef<iSyntaxService> synldr = 
    CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  csRef<iTextureFactory> plasmaFact = type->NewFactory();

  csRef<iTextureLoaderContext> ctx;
  if (context)
  {
    ctx = csPtr<iTextureLoaderContext>
      (SCF_QUERY_INTERFACE (context, iTextureLoaderContext));

    if (ctx)
    {
      if (ctx->HasSize())
      {
	int w, h;
	ctx->GetSize (w, h);
	plasmaFact->SetSize (w, h);
      }
    }
  }
  csRef<iTextureWrapper> tex = plasmaFact->Generate();

  return csPtr<iBase> (tex);
}

//---------------------------------------------------------------------------
// 'Plasma' saver.

csPtPlasmaSaver::csPtPlasmaSaver (iBase* p) : csBaseProctexSaver(p)
{
}

bool csPtPlasmaSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  return true;
}
