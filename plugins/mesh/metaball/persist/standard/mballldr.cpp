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
#include "mballldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/metaball.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
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
  XMLTOKEN_LIGHTING = 1,
  XMLTOKEN_ISOLEVEL,
  XMLTOKEN_CHARGE,
  XMLTOKEN_NUMBER,
  XMLTOKEN_TRUEMAP,
  XMLTOKEN_TEXSCALE,
  XMLTOKEN_RATE,

  XMLTOKEN_MATERIAL,
  XMLTOKEN_FACTORY,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_SHIFT
};

SCF_IMPLEMENT_IBASE (csMetaBallFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csMetaBallFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csMetaBallLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csMetaBallSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMetaBallSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csMetaBallFactoryLoader)
SCF_IMPLEMENT_FACTORY (csMetaBallFactorySaver)
SCF_IMPLEMENT_FACTORY (csMetaBallLoader)
SCF_IMPLEMENT_FACTORY (csMetaBallSaver)

SCF_EXPORT_CLASS_TABLE (mballldr)
  SCF_EXPORT_CLASS (csMetaBallFactoryLoader, "crystalspace.mesh.loader.factory.metaball",
    "Crystal Space MetaBall Factory Loader")
  SCF_EXPORT_CLASS (csMetaBallFactorySaver, "crystalspace.mesh.saver.factory.metaball",
    "Crystal Space MetaBall Factory Saver")
  SCF_EXPORT_CLASS (csMetaBallLoader, "crystalspace.mesh.loader.metaball",
    "Crystal Space MetaBall Mesh Loader")
  SCF_EXPORT_CLASS (csMetaBallSaver, "crystalspace.mesh.saver.metaball",
    "Crystal Space MetaBall Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csMetaBallFactoryLoader::csMetaBallFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csMetaBallFactoryLoader::~csMetaBallFactoryLoader ()
{
}

bool csMetaBallFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csMetaBallFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csMetaBallFactoryLoader::Parse (iDocumentNode*,
	iLoaderContext* , iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.metaball", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.metaball",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csMetaBallFactorySaver::csMetaBallFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csMetaBallFactorySaver::~csMetaBallFactorySaver ()
{
}

bool csMetaBallFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csMetaBallFactorySaver::object_reg = object_reg;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csMetaBallFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // no params
}

//---------------------------------------------------------------------------

csMetaBallLoader::csMetaBallLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csMetaBallLoader::~csMetaBallLoader ()
{
}

bool csMetaBallLoader::Initialize (iObjectRegistry* object_reg)
{
  csMetaBallLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("isolevel", XMLTOKEN_ISOLEVEL);
  xmltokens.Register ("charge", XMLTOKEN_CHARGE);
  xmltokens.Register ("number", XMLTOKEN_NUMBER);
  xmltokens.Register ("truemap", XMLTOKEN_TRUEMAP);
  xmltokens.Register ("texscale", XMLTOKEN_TEXSCALE);
  xmltokens.Register ("rate", XMLTOKEN_RATE);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("shift", XMLTOKEN_SHIFT);
  return true;
}

csPtr<iBase> csMetaBallLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iMetaBallState> ballstate;

  MetaParameters* mp = NULL;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_ISOLEVEL:
	{
	  if (!mp)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'isolevel'!");
	    return NULL;
	  }
	  mp->iso_level = child->GetContentsValueAsFloat ();
	}
	break;
      case XMLTOKEN_CHARGE:
	{
	  if (!mp)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'charge'!");
	    return NULL;
	  }
	  mp->charge = child->GetContentsValueAsFloat ();
	}
	break;
      case XMLTOKEN_NUMBER:
	{
	  if (!ballstate)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'number'!");
	    return NULL;
	  }
	  ballstate->SetMetaBallCount (child->GetContentsValueAsInt ());
	}
	break;
      case XMLTOKEN_RATE:
	{
	  if (!mp)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'rate'!");
	    return NULL;
	  }
	  mp->rate = child->GetContentsValueAsFloat ();
	}
	break;
      case XMLTOKEN_TRUEMAP:
	{
	  if (!ballstate)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'truemap'!");
	    return NULL;
	  }
	  bool m;
	  if (!synldr->ParseBool (child, m, true))
	    return NULL;
	  ballstate->SetQualityEnvironmentMapping (m);
	}
	break;
      case XMLTOKEN_TEXSCALE:
	{
	  if (!ballstate)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'texscale'!");
	    return NULL;
	  }
	  ballstate->SetEnvironmentMappingFactor (
	  	child->GetContentsValueAsFloat ());
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Can't find factory '%s'!", factname);
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          ballstate = SCF_QUERY_INTERFACE (mesh, iMetaBallState);
	  mp = ballstate->GetParameters();
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  if (!ballstate)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'material'!");
	    return NULL;
	  }
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Can't find material '%s'!", matname);
            return NULL;
	  }
	  ballstate->SetMaterial (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  if (!ballstate)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'mixmode'!");
	    return NULL;
	  }
	  uint mode;
	  if (synldr->ParseMixmode (child, mode))
	    ballstate->SetMixMode (mode);
	}
	break;
      case XMLTOKEN_LIGHTING:
	{
	  if (!ballstate)
	  {
    	    synldr->ReportError (
		"crystalspace.metaballloader.parse",
		child, "Please use 'factory' before 'lighting'!");
	    return NULL;
	  }
	  bool l;
	  if (!synldr->ParseBool (child, l, true))
	    return NULL;
	  ballstate->SetLighting (l);
	}
	break;
      default:
	synldr->ReportBadToken (child);
        return NULL;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------


csMetaBallSaver::csMetaBallSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csMetaBallSaver::~csMetaBallSaver ()
{
}

bool csMetaBallSaver::Initialize (iObjectRegistry* object_reg)
{
  csMetaBallSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csMetaBallSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  csRef<iFactory> fact (SCF_QUERY_INTERFACE (this, iFactory));
  csRef<iMeshObject> mesh (SCF_QUERY_INTERFACE(obj, iMeshObject));
  if(!mesh)
  {
    printf("Error: non-mesh given to %s.\n",
      fact->QueryDescription () );
    return;
  }
  csRef<iMetaBallState> state (SCF_QUERY_INTERFACE(obj, iMetaBallState));
  if(!state)
  {
    printf("Error: invalid mesh given to %s.\n",
      fact->QueryDescription () );
    return;
  }

  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str.Append(buf);
  if(state->GetMixMode() != CS_FX_COPY)
  {
    str.Append (synldr->MixmodeToText (state->GetMixMode(), 0, true));
  }

  // Mesh information
  MetaParameters *mp = state->GetParameters();
  sprintf(buf, "NUMBER (%d)\n", state->GetMetaBallCount());
  str.Append(buf);
  sprintf(buf, "ISO_LEVEL (%f)\n",mp->iso_level );
  str.Append(buf);
  sprintf(buf, "CHARGE (%f)\n", mp->charge);
  str.Append(buf);
  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterial()->
    QueryObject ()->GetName());
  str.Append(buf);
  sprintf(buf, "LIGHTING(%s)\n",(state->IsLighting())? "true" : "false");
  str.Append (buf);
  sprintf(buf, "NUMBER (%d)\n", state->GetMetaBallCount());
  str.Append(buf);
  sprintf(buf, "RATE (%f)\n",mp->rate);
  str.Append(buf);
  sprintf(buf, "TRUE_MAP (%s)\n",(state->GetQualityEnvironmentMapping())?"true":"false");
  str.Append(buf);
  sprintf(buf, "TEX_SCALE (%f)\n",state->GetEnvironmentMappingFactor());
  str.Append(buf);

  file->Write ((const char*)str, str.Length ());
}

