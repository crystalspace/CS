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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
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

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (COLORSCALE)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DIRECTION)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (ORIGINBOX)
  CS_TOKEN_DEF (SWIRL)
  CS_TOKEN_DEF (TOTALTIME)
CS_TOKEN_DEF_END

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

SCF_EXPORT_CLASS_TABLE (fireldr)
  SCF_EXPORT_CLASS (csFireFactoryLoader,
    "crystalspace.mesh.loader.factory.fire",
    "Crystal Space Fire Factory Loader")
  SCF_EXPORT_CLASS (csFireFactorySaver, "crystalspace.mesh.saver.factory.fire",
    "Crystal Space Fire Factory Saver")
  SCF_EXPORT_CLASS (csFireLoader, "crystalspace.mesh.loader.fire",
    "Crystal Space Fire Mesh Loader")
  SCF_EXPORT_CLASS (csFireSaver, "crystalspace.mesh.saver.fire",
    "Crystal Space Fire Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csFireFactoryLoader::csFireFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csFireFactoryLoader::~csFireFactoryLoader ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csFireFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csFireFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

iBase* csFireFactoryLoader::Parse (const char* /*string*/,
	iLoaderContext*, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.fire", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.fire",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.fire\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

iBase* csFireFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.fire", iMeshObjectType);
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.fire",
    	iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.fire\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csFireFactorySaver::csFireFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csFireFactorySaver::~csFireFactorySaver ()
{
  SCF_DEC_REF (plugin_mgr);
}

bool csFireFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csFireFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
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
  plugin_mgr = NULL;
  synldr = NULL;
  reporter = NULL;
}

csFireLoader::~csFireLoader ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (reporter);
}

bool csFireLoader::Initialize (iObjectRegistry* object_reg)
{
  csFireLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
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

iBase* csFireLoader::Parse (const char* string, 
			    iLoaderContext* ldr_context, iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (DIRECTION)
    CS_TOKEN_TABLE (ORIGINBOX)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (SWIRL)
    CS_TOKEN_TABLE (TOTALTIME)
    CS_TOKEN_TABLE (COLORSCALE)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iParticleState* partstate = NULL;
  iFireState* firestate = NULL;

  csParser* parser = ldr_context->GetParser ();

  char* buf = (char*)string;
  while ((cmd = parser->GetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (firestate) firestate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_COLOR:
	{
	  csColor color;
	  csScanStr(params, "%f,%f,%f", &color.red, &color.green, &color.blue);
	  partstate->SetColor (color);
	}
	break;
      case CS_TOKEN_DROPSIZE:
	{
	  float dw, dh;
	  csScanStr (params, "%f,%f", &dw, &dh);
	  firestate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_ORIGINBOX:
	{
	  csVector3 o1, o2;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	  	&o1.x, &o1.y, &o1.z,
		&o2.x, &o2.y, &o2.z);
	  firestate->SetOrigin (csBox3 (o1, o2));
	}
	break;
      case CS_TOKEN_ORIGIN:
	{
	  csVector3 origin;
	  csScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
	  firestate->SetOrigin (origin);
	}
	break;
      case CS_TOKEN_DIRECTION:
	{
	  csVector3 dir;
	  csScanStr (params, "%f,%f,%f", &dir.x, &dir.y, &dir.z);
	  firestate->SetDirection (dir);
	}
	break;
      case CS_TOKEN_SWIRL:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  firestate->SetSwirl (f);
	}
	break;
      case CS_TOKEN_COLORSCALE:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  firestate->SetColorScale (f);
	}
	break;
      case CS_TOKEN_TOTALTIME:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  firestate->SetTotalTime (f);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    if (partstate) partstate->DecRef ();
	    if (firestate) firestate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          firestate = SCF_QUERY_INTERFACE (mesh, iFireState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
	    printf ("Could not find material '%s'!\n", str);
            // @@@ Error handling!
            mesh->DecRef ();
	    if (partstate) partstate->DecRef ();
	    if (firestate) firestate->DecRef ();
            return NULL;
	  }
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	uint mode;
	if (synldr->ParseMixmode (parser, params, mode))
          partstate->SetMixMode (mode);
	break;
      case CS_TOKEN_LIGHTING:
        {
          bool do_lighting;
          csScanStr (params, "%b", &do_lighting);
          firestate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          csScanStr (params, "%d", &nr);
          firestate->SetParticleCount (nr);
        }
        break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (firestate) firestate->DecRef ();
  return mesh;
}

iBase* csFireLoader::Parse (iDocumentNode* node,
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
	    return NULL;
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
	    return NULL;
	  firestate->SetOrigin (box);
	}
	break;
      case XMLTOKEN_ORIGIN:
	{
	  csVector3 origin;
	  if (!synldr->ParseVector (child, origin))
	    return NULL;
	  firestate->SetOrigin (origin);
	}
	break;
      case XMLTOKEN_DIRECTION:
	{
	  csVector3 dir;
	  if (!synldr->ParseVector (child, dir))
	    return NULL;
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
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          firestate = SCF_QUERY_INTERFACE (mesh, iFireState);
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
	    return NULL;
	  }
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mode;
	  if (!synldr->ParseMixmode (child, mode))
	    return NULL;
          partstate->SetMixMode (mode);
	}
	break;
      case XMLTOKEN_LIGHTING:
        {
          bool do_lighting;
	  if (!synldr->ParseBool (child, do_lighting, true))
	    return NULL;
          firestate->SetLighting (do_lighting);
        }
        break;
      case XMLTOKEN_NUMBER:
        firestate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      default:
      	synldr->ReportBadToken (child);
	return NULL;
    }
  }

  // Incref so that smart pointer doesn't release reference.
  if (mesh) mesh->IncRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csFireSaver::csFireSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  plugin_mgr = NULL;
}

csFireSaver::~csFireSaver ()
{
  SCF_DEC_REF (plugin_mgr);
  SCF_DEC_REF (synldr);
}

bool csFireSaver::Initialize (iObjectRegistry* object_reg)
{
  csFireSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csFireSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  iParticleState *partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  iFireState *state = SCF_QUERY_INTERFACE (obj, iFireState);
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str.Append(buf);

  if(partstate->GetMixMode() != CS_FX_COPY)
  {									
    str.Append (synldr->MixmodeToText (partstate->GetMixMode(), 2, true));
  }
  sprintf(buf, "MATERIAL (%s)\n", partstate->GetMaterialWrapper()->
    QueryObject ()->GetName());
  str.Append(buf);
  sprintf(buf, "COLOR (%g, %g, %g)\n", partstate->GetColor().red,
    partstate->GetColor().green, partstate->GetColor().blue);
  str.Append(buf);

  printf(buf, "NUMBER (%d)\n", state->GetParticleCount());
  str.Append(buf);
  sprintf(buf, "LIGHTING (%s)\n", state->GetLighting()?"true":"false");
  str.Append(buf);
  sprintf(buf, "ORIGINBOX (%g,%g,%g, %g,%g,%g)\n",
    state->GetOrigin ().MinX (),
    state->GetOrigin ().MinY (),
    state->GetOrigin ().MinZ (),
    state->GetOrigin ().MaxX (),
    state->GetOrigin ().MaxY (),
    state->GetOrigin ().MaxZ ());
  str.Append(buf);
  sprintf(buf, "DIRECTION (%g,%g,%g)\n",
    state->GetDirection ().x,
    state->GetDirection ().y,
    state->GetDirection ().z);
  str.Append(buf);
  sprintf(buf, "SWIRL (%g)\n", state->GetSwirl());
  str.Append(buf);
  sprintf(buf, "TOTALTIME (%g)\n", state->GetTotalTime());
  str.Append(buf);
  sprintf(buf, "COLORSCALE (%g)\n", state->GetColorScale());
  str.Append(buf);
  float sx = 0.0, sy = 0.0;
  state->GetDropSize(sx, sy);
  sprintf(buf, "DROPSIZE (%g, %g)\n", sx, sy);
  str.Append(buf);

  fact->DecRef();
  partstate->DecRef();
  state->DecRef();
  file->Write ((const char*)str, str.Length ());
}

