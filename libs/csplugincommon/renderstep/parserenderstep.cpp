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
#include "iengine/engine.h"
#include "iengine/renderloop.h"
#include "iengine/rendersteps/irenderstep.h"
#include "imap/services.h"
#include "imap/reader.h"
#include "ivaria/reporter.h"

#include "csplugincommon/renderstep/parserenderstep.h"

CS_LEAKGUARD_IMPLEMENT (csRenderStepParser);

bool csRenderStepParser::Initialize(iObjectRegistry *object_reg)
{
  tokens.Register ("step", XMLTOKEN_STEP);

  csRenderStepParser::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  plugmgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);

  return (plugmgr != 0);
}

csPtr<iRenderStep> csRenderStepParser::Parse (
  iObjectRegistry* object_reg,
  iDocumentNode* node)
{
  csRef<iDocumentAttribute> pluginAttr = node->GetAttribute ("plugin");
  const char* pluginID = pluginAttr ? pluginAttr->GetValue () : 0;
  if (!pluginID)
  {
    if (synldr)
    {
      synldr->ReportError (
	"crystalspace.renderloop.step.parser",
	node,
	"'plugin' attribute missing");
    }					  
    return 0;
  }

  csRef<iLoaderPlugin> loader =
    CS_LOAD_PLUGIN (plugmgr, pluginID, iLoaderPlugin);
  /*
    @@@ This means a full ClassID has to be specified in <plugin>.
    Would be nice if the shortcuts from the loader could be used as
    well.
   */

  if (!loader)
  {
    if (synldr)
    {
      synldr->ReportError (
	"crystalspace.renderloop.step.parser",
	node,
	"Could not retrieve plugin '%s'",
	pluginID);
    }					  
    return 0;
  }

  csRef<iBase> b = loader->Parse (node, 0, 0);
  if (!b)
  {
    return 0;
  }
  csRef<iRenderStep> step =
    SCF_QUERY_INTERFACE (b, iRenderStep);
  if (!step)
  {
    if (synldr)
    {
      synldr->ReportError (
	"crystalspace.renderloop.step.parser",
	node,
	"Plugin didn't return render step!");
    }					  
    return 0;
  }

  return csPtr<iRenderStep> (step);
}

bool csRenderStepParser::ParseRenderSteps (iRenderStepContainer* container, 
					   iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_STEP:
	{
	  csRef<iRenderStep> step = Parse (object_reg, child);
	  if (!step)
	  {
	    return false;
	  }
	  size_t idx;
	  if ((idx = container->AddStep (step)) == csArrayItemNotFound)
	  {
	    if (synldr)
	    {
	      synldr->ReportError (
		"crystalspace.renderloop.steps.parser",
		node,
		"Render step container refused to add step. (%d)",
		idx);
	    }					  
	  }
	}
	break;
      default:
	if (synldr) synldr->ReportBadToken (child);
	return false;
    }
  }

  return true;
}

