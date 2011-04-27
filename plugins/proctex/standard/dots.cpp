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
#include "csutil/scf.h"

#include "prdots.h"
#include "stdproctex.h"
#include "dots.h"

SCF_IMPLEMENT_FACTORY(csPtDotsType)
SCF_IMPLEMENT_FACTORY(csPtDotsLoader)
SCF_IMPLEMENT_FACTORY(csPtDotsSaver)

#define CLASSID_DOTSTYPE "crystalspace.texture.type.dots"

csPtDotsType::csPtDotsType (iBase* p) :
  scfImplementationType(this, p)
{
}

csPtr<iTextureFactory> csPtDotsType::NewFactory()
{
  return csPtr<iTextureFactory> (new csPtDotsFactory (
    this, object_reg));
}

//---------------------------------------------------------------------------
// 'Dots' PT factory

csPtDotsFactory::csPtDotsFactory (iTextureType* p,
  iObjectRegistry* object_reg) : 
  scfImplementationType(this, p, object_reg)
{
}

csPtr<iTextureWrapper> csPtDotsFactory::Generate ()
{
  csRef<csProcTexture> pt = 
    csPtr<csProcTexture> (new csProcDots (this));

  if (pt->Initialize (object_reg))
  {
    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();
    return csPtr<iTextureWrapper> (tw);
  }
  
  return 0;
}

//---------------------------------------------------------------------------
// 'Dots' loader.

csPtDotsLoader::csPtDotsLoader(iBase *p) :
  scfImplementationType(this, p)
{
//  init_token_table (tokens);
}

csPtr<iBase> csPtDotsLoader::Parse (iDocumentNode* /*node*/, 
				    iStreamSource*, iLoaderContext* /*ldr_context*/,
  				    iBase* context)
{
  /*
    Going through the plugin manager to retrieve the texture type
    isn't really necessary here, as we could just instantiate csPtDotsType
    with new. It's just an 'exercise'.
   */
  csRef<iTextureType> type = csLoadPluginCheck<iTextureType> (
  	object_reg, CLASSID_DOTSTYPE);
  if (!type) return 0;
  csRef<iSyntaxService> synldr = 
    csQueryRegistry<iSyntaxService> (object_reg);

  csRef<iTextureFactory> dotsFact = type->NewFactory();

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
	dotsFact->SetSize (w, h);
      }
    }
  }
  csRef<iTextureWrapper> tex = dotsFact->Generate();

  csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
  if (!G3D) return 0;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return 0;
  tex->Register (tm);

  return csPtr<iBase> (tex);
}

//---------------------------------------------------------------------------
// 'Dots' saver.

csPtDotsSaver::csPtDotsSaver (iBase* p) :
  scfImplementationType(this, p)
{
}

bool csPtDotsSaver::WriteDown (iBase* /*obj*/, iDocumentNode* /*parent*/,
	iStreamSource*)
{
  return true;
}
