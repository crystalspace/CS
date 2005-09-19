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

#include "prsky.h"
#include "stdproctex.h"
#include "sky.h"

SCF_IMPLEMENT_FACTORY(csPtSkyType)
SCF_IMPLEMENT_FACTORY(csPtSkyLoader)
SCF_IMPLEMENT_FACTORY(csPtSkySaver)

#define CLASSID_SKYTYPE "crystalspace.texture.type.sky"

csPtSkyType::csPtSkyType (iBase* p) : csBaseProctexType(p)
{
}

csPtr<iTextureFactory> csPtSkyType::NewFactory()
{
  return csPtr<iTextureFactory> (new csPtSkyFactory (
    this, object_reg));
}

//---------------------------------------------------------------------------
// 'Sky' PT factory

csPtSkyFactory::csPtSkyFactory (iTextureType* p, iObjectRegistry* object_reg) : 
    csBaseTextureFactory (p, object_reg)
{
}

csPtr<iTextureWrapper> csPtSkyFactory::Generate ()
{

  csProcSky* sky = new csProcSky();

  csRef<csProcTexture> pt = 
    csPtr<csProcTexture> (new csProcSkyTexture (this, sky));

  if (pt->Initialize (object_reg))
  {
    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();
    return csPtr<iTextureWrapper> (tw);
  }

  return 0;
}

//---------------------------------------------------------------------------
// 'Sky' loader.

csPtSkyLoader::csPtSkyLoader(iBase *p) : csBaseProctexLoader(p)
{
//  init_token_table (tokens);
}

csPtr<iBase> csPtSkyLoader::Parse (iDocumentNode* node, 
				    iStreamSource*, iLoaderContext* ldr_context,
  				    iBase* context)
{
  /*
    Going through the plugin manager to retrieve the texture type
    isn't really necessary here, as we could just instantiate csPtSkyType
    with new. It's just an 'exercise'.
   */
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iTextureType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	CLASSID_SKYTYPE, iTextureType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, CLASSID_SKYTYPE,
    	iTextureType);
  }
  csRef<iSyntaxService> synldr = 
    CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  csRef<iTextureFactory> skyFact = type->NewFactory();

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
	skyFact->SetSize (w, h);
      }
    }
  }
  csRef<iTextureWrapper> tex = skyFact->Generate();

  csRef<iGraphics3D> G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!G3D) return 0;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return 0;
  tex->Register (tm);

  return csPtr<iBase> (tex);
}

//---------------------------------------------------------------------------
// 'Sky' saver.

csPtSkySaver::csPtSkySaver (iBase* p) : csBaseProctexSaver(p)
{
}

bool csPtSkySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  return true;
}
