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
#include "ivideo/graph2d.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "imap/loader.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "itexture/itexloaderctx.h"
#include "csutil/csstring.h"

#include "ptanimimg.h"

// Plugin stuff

SCF_IMPLEMENT_IBASE(csAnimateProctexLoader);
  SCF_IMPLEMENTS_INTERFACE(iLoaderPlugin);
  SCF_IMPLEMENTS_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csAnimateProctexLoader)


//----------------------------------------------------------------------------

csAnimateProctexLoader::csAnimateProctexLoader (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
}

csAnimateProctexLoader::~csAnimateProctexLoader ()
{
  SCF_DESTRUCT_IBASE();
}

bool csAnimateProctexLoader::Initialize(iObjectRegistry *object_reg)
{
  csAnimateProctexLoader::object_reg = object_reg;

  return true;
}

csPtr<iBase> csAnimateProctexLoader::Parse (iDocumentNode* node, 
					    iLoaderContext* ldr_context,
  					    iBase* context)
{
  csRef<iTextureLoaderContext> ctx;
  if (context)
  {
    ctx = csPtr<iTextureLoaderContext>
      (SCF_QUERY_INTERFACE (context, iTextureLoaderContext));
  }

  csRef<iImage> img = (ctx && ctx->HasImage()) ? ctx->GetImage() : 0;
  if (!img)
  {
    if (!node)
    {
      Report (CS_REPORTER_SEVERITY_WARNING, node, 
	"Please provide a <file> node in the <texture> or <params> block");
      return 0;
    }

    csRef<iLoader> LevelLoader = CS_QUERY_REGISTRY (object_reg, iLoader);
    if (!LevelLoader) 
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 0, "No level loader");
      return 0;
    }

    csRef<iDocumentNode> file = node->GetNode ("file");
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

    img = LevelLoader->LoadImage (fname,
      CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA);
    if (!img) 
    {
      Report (CS_REPORTER_SEVERITY_WARNING, file, 
	"Couldn't load image '%s'", fname);
      return 0;
    }
  }

  csRef<csProcTexture> pt = csPtr<csProcTexture> (new csProcAnimated (img));
  if (pt->Initialize (object_reg))
  {
    csRef<iTextureWrapper> tw = pt->GetTextureWrapper ();
    return csPtr<iBase> (tw);
  }

  return 0;
}

void csAnimateProctexLoader::Report (int severity, iDocumentNode* node,
				     const char* msg, ...)
{
  va_list arg;
  va_start (arg, msg);

  csString text;
  text.FormatV (msg, arg);

  csRef<iSyntaxService> synserv;

  if (node)
    synserv = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  if (node && synserv)
  {
    synserv->Report ("crystalspace.proctex.loader.animimg",
      severity, node, "%s", (const char*)text);
  }
  else
  {
    csReport (object_reg, severity, "crystalspace.proctex.loader.animimg",
      "%s", (const char*)text);
  }

  va_end (arg);
}

