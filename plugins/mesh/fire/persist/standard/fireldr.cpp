/*
    Copyright (C) 2001-2002 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "csgeom/math3d.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "csutil/util.h"
#include "fireldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "imesh/partsys.h"
#include "imesh/fire.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_COLORSCALE = 1,
  XMLTOKEN_COLOR,
  XMLTOKEN_DIRECTION,
  XMLTOKEN_DROPSIZE,
  XMLTOKEN_FACTORY,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_NUMBER,
  XMLTOKEN_ORIGIN,
  XMLTOKEN_ORIGINBOX,
  XMLTOKEN_SWIRL,
  XMLTOKEN_TOTALTIME
};

SCF_IMPLEMENT_IBASE (csFireFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFireFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFireFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFireFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFireLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFireLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFireSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFireSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csFireFactoryLoader)
SCF_IMPLEMENT_FACTORY (csFireFactorySaver)
SCF_IMPLEMENT_FACTORY (csFireLoader)
SCF_IMPLEMENT_FACTORY (csFireSaver)


csFireFactoryLoader::csFireFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFireFactoryLoader::~csFireFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFireFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csFireFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csFireFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.fire", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.fire",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csFireFactorySaver::csFireFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFireFactorySaver::~csFireFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFireFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csFireFactorySaver::object_reg = object_reg;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csFireFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // nothing to do
}

//---------------------------------------------------------------------------

csFireLoader::csFireLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFireLoader::~csFireLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFireLoader::Initialize (iObjectRegistry* object_reg)
{
  csFireLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  xmltokens.Register ("colorscale", XMLTOKEN_COLORSCALE);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("direction", XMLTOKEN_DIRECTION);
  xmltokens.Register ("dropsize", XMLTOKEN_DROPSIZE);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("number", XMLTOKEN_NUMBER);
  xmltokens.Register ("origin", XMLTOKEN_ORIGIN);
  xmltokens.Register ("originbox", XMLTOKEN_ORIGINBOX);
  xmltokens.Register ("swirl", XMLTOKEN_SWIRL);
  xmltokens.Register ("totaltime", XMLTOKEN_TOTALTIME);
  return true;
}

csPtr<iBase> csFireLoader::Parse (iDocumentNode* node,
			    iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iParticleState> partstate;
  csRef<iFireState> firestate;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COLOR:
	{
	  csColor color;
	  if (!synldr->ParseColor (child, color))
	    return 0;
	  partstate->SetColor (color);
	}
	break;
      case XMLTOKEN_DROPSIZE:
	{
	  float dw, dh;
	  dw = child->GetAttributeValueAsFloat ("w");
	  dh = child->GetAttributeValueAsFloat ("h");
	  firestate->SetDropSize (dw, dh);
	}
	break;
      case XMLTOKEN_ORIGINBOX:
	{
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  firestate->SetOrigin (box);
	}
	break;
      case XMLTOKEN_ORIGIN:
	{
	  csVector3 origin;
	  if (!synldr->ParseVector (child, origin))
	    return 0;
	  firestate->SetOrigin (origin);
	}
	break;
      case XMLTOKEN_DIRECTION:
	{
	  csVector3 dir;
	  if (!synldr->ParseVector (child, dir))
	    return 0;
	  firestate->SetDirection (dir);
	}
	break;
      case XMLTOKEN_SWIRL:
	firestate->SetSwirl (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_COLORSCALE:
	firestate->SetColorScale (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_TOTALTIME:
	firestate->SetTotalTime (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError ("crystalspace.fireloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          firestate = SCF_QUERY_INTERFACE (mesh, iFireState);
	  if (!firestate)
	  {
      	    synldr->ReportError (
		"crystalspace.fireloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a fire factory!",
		factname);
	    return 0;
	  }
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.fireloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
	    return 0;
	  }
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mode;
	  if (!synldr->ParseMixmode (child, mode))
	    return 0;
          partstate->SetMixMode (mode);
	}
	break;
      case XMLTOKEN_LIGHTING:
        {
          bool do_lighting;
	  if (!synldr->ParseBool (child, do_lighting, true))
	    return 0;
          firestate->SetLighting (do_lighting);
        }
        break;
      case XMLTOKEN_NUMBER:
        firestate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      default:
      	synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------


csFireSaver::csFireSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFireSaver::~csFireSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFireSaver::Initialize (iObjectRegistry* object_reg)
{
  csFireSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csFireSaver::WriteDown (iBase*, iFile*)
{
}

