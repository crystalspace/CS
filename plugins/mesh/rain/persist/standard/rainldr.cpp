/*
    Copyright (C) 2001 by Jorrit Tyberghein
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
#include "rainldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/rain.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iutil/vfs.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_COLOR = 1,
  XMLTOKEN_DROPSIZE,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FALLSPEED,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_NUMBER,
  XMLTOKEN_COLLDET,
  XMLTOKEN_BOX
};

SCF_IMPLEMENT_IBASE (csRainFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csRainFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csRainLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csRainSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csRainSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csRainFactoryLoader)
SCF_IMPLEMENT_FACTORY (csRainFactorySaver)
SCF_IMPLEMENT_FACTORY (csRainLoader)
SCF_IMPLEMENT_FACTORY (csRainSaver)


csRainFactoryLoader::csRainFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csRainFactoryLoader::~csRainFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csRainFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csRainFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csRainFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.rain", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.rain",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------
csRainFactorySaver::csRainFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csRainFactorySaver::~csRainFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csRainFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csRainFactorySaver::object_reg = object_reg;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csRainFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // nothing to do
}
//---------------------------------------------------------------------------

csRainLoader::csRainLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csRainLoader::~csRainLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csRainLoader::Initialize (iObjectRegistry* object_reg)
{
  csRainLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("dropsize", XMLTOKEN_DROPSIZE);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("fallspeed", XMLTOKEN_FALLSPEED);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("number", XMLTOKEN_NUMBER);
  xmltokens.Register ("box", XMLTOKEN_BOX);
  xmltokens.Register ("colldet", XMLTOKEN_COLLDET);
  return true;
}

csPtr<iBase> csRainLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iRainState> rainstate;

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
	  rainstate->SetColor (color);
	}
	break;
      case XMLTOKEN_DROPSIZE:
	{
	  float dw, dh;
	  dw = child->GetAttributeValueAsFloat ("w");
	  dh = child->GetAttributeValueAsFloat ("h");
	  rainstate->SetDropSize (dw, dh);
	}
	break;
      case XMLTOKEN_BOX:
	{
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  rainstate->SetBox (box.Min (), box.Max ());
	}
	break;
      case XMLTOKEN_FALLSPEED:
	{
	  csVector3 s;
	  if (!synldr->ParseVector (child, s))
	    return 0;
	  rainstate->SetFallSpeed (s);
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.rainloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  rainstate = SCF_QUERY_INTERFACE (mesh, iRainState);
	  if (!rainstate)
	  {
      	    synldr->ReportError (
		"crystalspace.rainloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a rain factory!",
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
		"crystalspace.rainloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return 0;
	  }
	  rainstate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
	{
	  uint mode;
	  if (!synldr->ParseMixmode (child, mode))
	    return 0;
          rainstate->SetMixMode (mode);
	}
	break;
      case XMLTOKEN_COLLDET:
        {
          bool do_cd;
	  if (!synldr->ParseBool (child, do_cd, true))
	    return 0;
          rainstate->SetCollisionDetection (do_cd);
        }
        break;
      case XMLTOKEN_LIGHTING:
        {
          bool do_lighting;
	  if (!synldr->ParseBool (child, do_lighting, true))
	    return 0;
          rainstate->SetLighting (do_lighting);
        }
        break;
      case XMLTOKEN_NUMBER:
        rainstate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------

csRainSaver::csRainSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csRainSaver::~csRainSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csRainSaver::Initialize (iObjectRegistry* object_reg)
{
  csRainSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csRainSaver::WriteDown (iBase*, iFile*)
{
}

