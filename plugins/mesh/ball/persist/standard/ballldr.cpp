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
#include "cssys/sysfunc.h"
#include "csgeom/math3d.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "csutil/util.h"
#include "ballldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "imesh/ball.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
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
  XMLTOKEN_LIGHTING,
  XMLTOKEN_COLOR,
  XMLTOKEN_NUMRIM,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_FACTORY,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_RADIUS,
  XMLTOKEN_SHIFT,
  XMLTOKEN_REVERSED,
  XMLTOKEN_TOPONLY,
  XMLTOKEN_CYLINDRICAL
};

SCF_IMPLEMENT_IBASE (csBallFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBallFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBallLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBallSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBallSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csBallFactoryLoader)
SCF_IMPLEMENT_FACTORY (csBallFactorySaver)
SCF_IMPLEMENT_FACTORY (csBallLoader)
SCF_IMPLEMENT_FACTORY (csBallSaver)

SCF_EXPORT_CLASS_TABLE (ballldr)
  SCF_EXPORT_CLASS (csBallFactoryLoader,
    "crystalspace.mesh.loader.factory.ball",
    "Crystal Space Ball Factory Loader")
  SCF_EXPORT_CLASS (csBallFactorySaver, "crystalspace.mesh.saver.factory.ball",
    "Crystal Space Ball Factory Saver")
  SCF_EXPORT_CLASS (csBallLoader, "crystalspace.mesh.loader.ball",
		    "Crystal Space Ball Mesh Loader")
  SCF_EXPORT_CLASS (csBallSaver, "crystalspace.mesh.saver.ball",
    "Crystal Space Ball Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

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

csBallFactoryLoader::csBallFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBallFactoryLoader::~csBallFactoryLoader ()
{
}

bool csBallFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csBallFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

csPtr<iBase> csBallFactoryLoader::Parse (iDocumentNode*,
			     iLoaderContext*, iBase*)
{
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.ball", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.ball",
    	iMeshObjectType);
  }
  if (!type)
  {
    ReportError (reporter,
		"crystalspace.ballfactoryloader.setup.objecttype",
		"Could not load the ball mesh object plugin!");
    return NULL;
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csBallFactorySaver::csBallFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBallFactorySaver::~csBallFactorySaver ()
{
}

bool csBallFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csBallFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csBallFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // no params
}

//---------------------------------------------------------------------------

csBallLoader::csBallLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBallLoader::~csBallLoader ()
{
}

bool csBallLoader::Initialize (iObjectRegistry* object_reg)
{
  csBallLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("numrim", XMLTOKEN_NUMRIM);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("shift", XMLTOKEN_SHIFT);
  xmltokens.Register ("reversed", XMLTOKEN_REVERSED);
  xmltokens.Register ("toponly", XMLTOKEN_TOPONLY);
  xmltokens.Register ("cylindrical", XMLTOKEN_CYLINDRICAL);
  return true;
}

csPtr<iBase> csBallLoader::Parse (iDocumentNode* node,
			     iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iBallState> ballstate;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_REVERSED:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return NULL;
	  ballstate->SetReversed (r);
	}
	break;
      case XMLTOKEN_TOPONLY:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return NULL;
	  ballstate->SetTopOnly (r);
	}
	break;
      case XMLTOKEN_CYLINDRICAL:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return NULL;
	  ballstate->SetCylindricalMapping (r);
	}
	break;
      case XMLTOKEN_LIGHTING:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return NULL;
	  ballstate->SetLighting (r);
	}
	break;
      case XMLTOKEN_COLOR:
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return NULL;
	  ballstate->SetColor (col);
	}
	break;
      case XMLTOKEN_RADIUS:
	{
	  csVector3 rad;
	  if (!synldr->ParseVector (child, rad))
	    return NULL;
	  ballstate->SetRadius (rad.x, rad.y, rad.z);
	}
	break;
      case XMLTOKEN_SHIFT:
	{
	  csVector3 rad;
	  if (!synldr->ParseVector (child, rad))
	    return NULL;
	  ballstate->SetShift (rad.x, rad.y, rad.z);
	}
	break;
      case XMLTOKEN_NUMRIM:
	{
	  int f = child->GetContentsValueAsInt ();
	  ballstate->SetRimVertices (f);
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError ("crystalspace.ballloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          ballstate = SCF_QUERY_INTERFACE (mesh, iBallState);
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.ballloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return NULL;
	  }
	  ballstate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
	{
	  uint mm;
	  if (!synldr->ParseMixmode (child, mm))
	    return NULL;
          ballstate->SetMixMode (mm);
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

csBallSaver::csBallSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBallSaver::~csBallSaver ()
{
}

bool csBallSaver::Initialize (iObjectRegistry* object_reg)
{
  csBallSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csBallSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  csRef<iFactory> fact (SCF_QUERY_INTERFACE (this, iFactory));
  csRef<iMeshObject> mesh (SCF_QUERY_INTERFACE(obj, iMeshObject));
  if(!mesh)
  {
    ReportError (reporter,
		 "crystalspace.ballsaver",
		 "Error: non-mesh given to %s.\n", fact->QueryDescription () );
    return;
  }
  csRef<iBallState> state (SCF_QUERY_INTERFACE(obj, iBallState));
  if(!state)
  {
    ReportError (reporter,
	 "crystalspace.ballsaver",
	 "Error: invalid mesh given to %s.\n", fact->QueryDescription () );
    return;
  }

  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str.Append (buf);
  if(state->GetMixMode() != CS_FX_COPY)
    str.Append (synldr->MixmodeToText (state->GetMixMode(), 0));

  // Mesh information
  float x=0, y=0, z=0;
  state->GetRadius(x, y, z);
  str.Append(synldr->VectorToText ("RADIUS", x, y, z, 0));
  str.Append(synldr->VectorToText ("SHIFT", state->GetShift (), 0));
  sprintf(buf, "NUMRIM (%d)\n", state->GetRimVertices());
  str.Append(buf);
  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str.Append (buf);
  str.Append (synldr->BoolToText ("LIGHTING",state->IsLighting (),0));
  str.Append (synldr->BoolToText ("REVERSED",state->IsReversed (),0));
  str.Append (synldr->BoolToText ("TOPONLY",state->IsTopOnly (),0));
  str.Append (synldr->BoolToText ("CYLINDRICAL",state->IsCylindricalMapping (),0));
  csColor col = state->GetColor ();
  str.Append (synldr->VectorToText ("COLOR", col.red, col.green, col.blue, 0));

  file->Write ((const char*)str, str.Length ());
}

