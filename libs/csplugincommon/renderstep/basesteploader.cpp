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

#include "iutil/plugin.h"
#include "ivaria/reporter.h"

#include "csplugincommon/renderstep/basesteploader.h"

CS_LEAKGUARD_IMPLEMENT (csBaseRenderStepLoader);

SCF_IMPLEMENT_IBASE (csBaseRenderStepLoader);
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin);
  SCF_IMPLEMENTS_INTERFACE (iComponent);
SCF_IMPLEMENT_IBASE_END;

csBaseRenderStepLoader::csBaseRenderStepLoader (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
}

csBaseRenderStepLoader::~csBaseRenderStepLoader ()
{
  SCF_DESTRUCT_IBASE();
}

bool csBaseRenderStepLoader::Initialize(iObjectRegistry *object_reg)
{
  csBaseRenderStepLoader::object_reg = object_reg;

  csRef<iPluginManager> plugin_mgr = 
    CS_QUERY_REGISTRY (object_reg, iPluginManager);

  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!synldr)
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.renderloop.step.common",
	"Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (synldr, "iSyntaxService"))
    {
      csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.renderloop.step.common",
	"Could not register the syntax services!");
      return false;
    }
  }

  return true;
}

