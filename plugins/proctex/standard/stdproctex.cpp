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

#include "ivideo/graph3d.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"

#include "cstool/prdots.h"
#include "cstool/prwater.h"
#include "cstool/prfire.h"
#include "cstool/prplasma.h"

#include "stdproctex.h"
#include "fire.h"

// Plugin stuff

SCF_IMPLEMENT_IBASE(csBaseProctexLoader);
  SCF_IMPLEMENTS_INTERFACE(iLoaderPlugin);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csBaseProctexLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE(csBaseProctexType);
  SCF_IMPLEMENTS_INTERFACE(iTextureType);
  SCF_IMPLEMENTS_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csPtDotsLoader);
SCF_IMPLEMENT_FACTORY(csPtPlasmaLoader);
SCF_IMPLEMENT_FACTORY(csPtWaterLoader);


//---------------------------------------------------------------------------
// Base for all PT types

csBaseProctexType::csBaseProctexType (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
}

csBaseProctexType::~csBaseProctexType ()
{
  SCF_DESTRUCT_IBASE();
}

bool csBaseProctexType::Initialize(iObjectRegistry *object_reg)
{
  csBaseProctexType::object_reg = object_reg;

  return true;
}

//---------------------------------------------------------------------------
// Base for all PT loaders

csBaseProctexLoader::csBaseProctexLoader(iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
}

csBaseProctexLoader::~csBaseProctexLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csBaseProctexLoader::Initialize(iObjectRegistry *object_reg)
{
  csBaseProctexLoader::object_reg = object_reg;

  return true;
}

csPtr<iBase> csBaseProctexLoader::PrepareProcTex (csProcTexture* pt)
{
  if (pt->Initialize (object_reg))
  {
    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();
    return csPtr<iBase> (tw);
  }
  else
  {
    return 0;
  }
}

//---------------------------------------------------------------------------
// 'Dots' loader.

csPtDotsLoader::csPtDotsLoader(iBase *p) : csBaseProctexLoader(p)
{
}

csPtr<iBase> csPtDotsLoader::Parse (iDocumentNode* node, 
				    iLoaderContext* ldr_context,
  				    iBase* context)
{
  csRef<csProcTexture> pt = csPtr<csProcTexture> (new csProcDots ());
  return PrepareProcTex (pt);
}

//---------------------------------------------------------------------------
// 'Water' loader.

csPtWaterLoader::csPtWaterLoader(iBase *p) : csBaseProctexLoader(p)
{
}

csPtr<iBase> csPtWaterLoader::Parse (iDocumentNode* node, 
				    iLoaderContext* ldr_context,
  				    iBase* context)
{
  csRef<csProcTexture> pt = csPtr<csProcTexture> (new csProcWater ());
  return PrepareProcTex (pt);
}

//---------------------------------------------------------------------------
// 'Plasma' loader.

csPtPlasmaLoader::csPtPlasmaLoader(iBase *p) : csBaseProctexLoader(p)
{
}

csPtr<iBase> csPtPlasmaLoader::Parse (iDocumentNode* node, 
				      iLoaderContext* ldr_context,
  				      iBase* context)
{
  csRef<csProcTexture> pt = csPtr<csProcTexture> (new csProcPlasma ());
  return PrepareProcTex (pt);
}

