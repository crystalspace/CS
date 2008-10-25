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
#include "imap/services.h"
#include "itexture/itexloaderctx.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "csutil/scf.h"

#include "prwater.h"
#include "stdproctex.h"
#include "water.h"

SCF_IMPLEMENT_FACTORY(csPtWaterType)
SCF_IMPLEMENT_FACTORY(csPtWaterLoader)
SCF_IMPLEMENT_FACTORY(csPtWaterSaver)

#define CLASSID_WATERTYPE "crystalspace.texture.type.water"

csPtWaterType::csPtWaterType (iBase* p) :
  scfImplementationType(this, p)
{
}

csPtr<iTextureFactory> csPtWaterType::NewFactory()
{
  return csPtr<iTextureFactory> (new csPtWaterFactory (
    this, object_reg));
}

//---------------------------------------------------------------------------
// 'Water' PT factory

csPtWaterFactory::csPtWaterFactory (iTextureType* p, iObjectRegistry* object_reg) : 
  scfImplementationType(this, p, object_reg)
{
}

csPtr<iTextureWrapper> csPtWaterFactory::Generate ()
{
  csRef<csProcTexture> pt = 
    csPtr<csProcTexture> (new csProcWater (this));

  if (pt->Initialize (object_reg))
  {
    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();
    return csPtr<iTextureWrapper> (tw);
  }

  return 0;
}

//---------------------------------------------------------------------------
// 'Water' loader.

csPtWaterLoader::csPtWaterLoader(iBase *p) :
  scfImplementationType(this, p)
{
//  init_token_table (tokens);
}

csPtr<iBase> csPtWaterLoader::Parse (iDocumentNode* /*node*/, 
				    iStreamSource*, iLoaderContext* /*ldr_context*/,
  				    iBase* context)
{
  /*
    Going through the plugin manager to retrieve the texture type
    isn't really necessary here, as we could just instantiate csPtWaterType
    with new. It's just an 'exercise'.
   */
  csRef<iTextureType> type = csLoadPluginCheck<iTextureType> (
  	object_reg, CLASSID_WATERTYPE);
  if (!type) return 0;
  csRef<iSyntaxService> synldr = 
    csQueryRegistry<iSyntaxService> (object_reg);

  csRef<iTextureFactory> waterFact = type->NewFactory();

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
	waterFact->SetSize (w, h);
      }
    }
  }
  csRef<iTextureWrapper> tex = waterFact->Generate();

  csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
  if (!G3D) return 0;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return 0;
  tex->Register (tm);

  return csPtr<iBase> (tex);
}

//---------------------------------------------------------------------------
// 'Water' saver.

csPtWaterSaver::csPtWaterSaver (iBase* p) :
  scfImplementationType(this, p)
{
}

bool csPtWaterSaver::WriteDown (iBase* /*obj*/, iDocumentNode* /*parent*/,
	iStreamSource*)
{
  return true;
}
