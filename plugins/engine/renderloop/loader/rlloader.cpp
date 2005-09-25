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

#include "csutil/util.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iengine/engine.h"
#include "iengine/renderloop.h"
#include "imap/services.h"
#include "ivaria/reporter.h"
#include "iengine/rendersteps/irenderstep.h"
#include "iengine/rendersteps/icontainer.h"

#include "rlloader.h"

CS_LEAKGUARD_IMPLEMENT (csRenderLoopLoader);

// Plugin stuff

SCF_IMPLEMENT_IBASE(csRenderLoopLoader);
  SCF_IMPLEMENTS_INTERFACE(iLoaderPlugin);
  SCF_IMPLEMENTS_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csRenderLoopLoader)


//---------------------------------------------------------------------------

csRenderLoopLoader::csRenderLoopLoader (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  InitTokenTable (tokens);
}

csRenderLoopLoader::~csRenderLoopLoader ()
{
  SCF_DESTRUCT_IBASE();
}

bool csRenderLoopLoader::Initialize(iObjectRegistry *object_reg)
{
  csRenderLoopLoader::object_reg = object_reg;
  
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  rsp.Initialize (object_reg);

  return true;
}

bool csRenderLoopLoader::ParseRenderSteps (iRenderLoop* loop, 
					   iDocumentNode* node)
{
  csRef<iRenderStepContainer> cont =
    scfQueryInterface<iRenderStepContainer> (loop);
  if (!cont)
  {
    if (synldr)
      synldr->ReportError (
	      "crystalspace.renderloop.load",
              node, "Internal error: doesn't implement iRenderStepContainer!");
    return false;
  }

  return rsp.ParseRenderSteps (cont, node);
}

csPtr<iBase> csRenderLoopLoader::Parse (iDocumentNode* node, 
  iStreamSource*, iLoaderContext* ldr_context, iBase* context)
{
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!engine)
  {
    if (synldr)
      synldr->ReportError (
	      "crystalspace.renderloop.load",
              node, "Can't find engine!");
    return 0;
  }

  iRenderLoopManager* loopmgr = engine->GetRenderLoopManager ();
  if (!loopmgr)
  {
    if (synldr)
      synldr->ReportError (
	      "crystalspace.renderloop.load",
              node, "Engine doesn't have a render loop manager!");
    return 0;
  }

  csRef<iRenderLoop> loop = loopmgr->Create ();

  char* loopName = 0;

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
	case XMLTOKEN_NAME:
	  {
	    loopName = csStrNew (child->GetContentsValue ());
	  }
	  break;
	case XMLTOKEN_STEPS:
	  {
	    if (!ParseRenderSteps (loop, child))
	    {
	      goto error;
	    }
	  }
	  break;
	default:
	  if (synldr) synldr->ReportBadToken (child);
	  goto error;
      }
    }
  }

  if (loopName)
  {
    if (!loopmgr->Register (loopName, loop))
    {
      if (synldr) 
      {
	synldr->Report (
	  "crystalspace.renderloop.loop.loader",
	  CS_REPORTER_SEVERITY_WARNING,
	  node,
	  "Couldn't register render loop '%s'. Maybe a loop of the same "
	  "name already exists?",
	  loopName);
      }
    }
  }
  else
  {
    if (synldr) 
    {
      synldr->Report (
	"crystalspace.renderloop.loop.loader",
	CS_REPORTER_SEVERITY_WARNING,
	node,
	"Render loop has no name and is therefore inaccessible. "
	"This may not be what you want.");
    }
  }

  if (loop->GetStepCount () == 0)
  {
    if (synldr) 
    {
      synldr->Report (
	"crystalspace.renderloop.loop.loader",
	CS_REPORTER_SEVERITY_WARNING,
	node,
	"Render loop has no steps. "
	"This may not be what you want.");
    }
  }

  delete[] loopName;
  return csPtr<iBase> (loop);

error:
  delete[] loopName;
  return 0;
}


