/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csutil/sysfunc.h"
#include "csgeom/math3d.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "starldr.h"
#include "imesh/object.h"
#include "imesh/stars.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_BOX = 1,
  XMLTOKEN_COLOR,
  XMLTOKEN_MAXCOLOR,
  XMLTOKEN_DENSITY,
  XMLTOKEN_MAXDISTANCE,
  XMLTOKEN_FACTORY
};

SCF_IMPLEMENT_IBASE (csStarFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csStarFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csStarLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csStarSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStarSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csStarFactoryLoader)
SCF_IMPLEMENT_FACTORY (csStarFactorySaver)
SCF_IMPLEMENT_FACTORY (csStarLoader)
SCF_IMPLEMENT_FACTORY (csStarSaver)


static void ReportError (iReporter* reporter, const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (reporter)
  {
    reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Error ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
}

csStarFactoryLoader::csStarFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csStarFactoryLoader::~csStarFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csStarFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csStarFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

csPtr<iBase> csStarFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.stars", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.stars",
    	iMeshObjectType);
  }
  if (!type)
  {
    ReportError (reporter,
		"crystalspace.starfactoryloader.setup.objecttype",
		"Could not load the stars mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csStarFactorySaver::csStarFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csStarFactorySaver::~csStarFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csStarFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csStarFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csStarFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // no params
}

//---------------------------------------------------------------------------

csStarLoader::csStarLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csStarLoader::~csStarLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csStarLoader::Initialize (iObjectRegistry* object_reg)
{
  csStarLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("box", XMLTOKEN_BOX);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("maxcolor", XMLTOKEN_MAXCOLOR);
  xmltokens.Register ("density", XMLTOKEN_DENSITY);
  xmltokens.Register ("maxdistance", XMLTOKEN_MAXDISTANCE);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);

  return true;
}

csPtr<iBase> csStarLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iStarsState> starstate;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_BOX:
	{
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  starstate->SetBox (box);
	}
	break;
      case XMLTOKEN_COLOR:
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return 0;
	  starstate->SetColor (col);
	}
	break;
      case XMLTOKEN_MAXCOLOR:
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return 0;
	  starstate->SetMaxColor (col);
	}
	break;
      case XMLTOKEN_DENSITY:
	starstate->SetDensity (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_MAXDISTANCE:
	starstate->SetMaxDistance (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.starloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          starstate = SCF_QUERY_INTERFACE (mesh, iStarsState);
	  if (!starstate)
	  {
      	    synldr->ReportError (
		"crystalspace.starloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a star factory!",
		factname);
	    return 0;
	  }
	}
	break;
      default:
        synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------

csStarSaver::csStarSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csStarSaver::~csStarSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csStarSaver::Initialize (iObjectRegistry* object_reg)
{
  csStarSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csStarSaver::WriteDown (iBase* /*obj*/, iFile* /*file*/)
{
  // @@@ Not implemented yet.
}
