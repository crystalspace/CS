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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "fountldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/fountain.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "csutil/csstring.h"
#include "iutil/vfs.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ACCEL)
  CS_TOKEN_DEF (AZIMUTH)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (ELEVATION)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FALLTIME)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (OPENING)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (SPEED)
CS_TOKEN_DEF_END

enum
{
  XMLTOKEN_ACCEL = 1,
  XMLTOKEN_AZIMUTH,
  XMLTOKEN_COLOR,
  XMLTOKEN_DROPSIZE,
  XMLTOKEN_ELEVATION,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FALLTIME,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_NUMBER,
  XMLTOKEN_OPENING,
  XMLTOKEN_ORIGIN,
  XMLTOKEN_SPEED
};

SCF_IMPLEMENT_IBASE (csFountainFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFountainFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFountainLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csFountainSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csFountainSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csFountainFactoryLoader)
SCF_IMPLEMENT_FACTORY (csFountainFactorySaver)
SCF_IMPLEMENT_FACTORY (csFountainLoader)
SCF_IMPLEMENT_FACTORY (csFountainSaver)

SCF_EXPORT_CLASS_TABLE (fountldr)
  SCF_EXPORT_CLASS (csFountainFactoryLoader,
  	"crystalspace.mesh.loader.factory.fountain",
	"Crystal Space Fountain Factory Loader")
  SCF_EXPORT_CLASS (csFountainFactorySaver,
  	"crystalspace.mesh.saver.factory.fountain",
	"Crystal Space Fountain Factory Saver")
  SCF_EXPORT_CLASS (csFountainLoader, "crystalspace.mesh.loader.fountain",
    "Crystal Space Fountain Mesh Loader")
  SCF_EXPORT_CLASS (csFountainSaver, "crystalspace.mesh.saver.fountain",
    "Crystal Space Fountain Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csFountainFactoryLoader::csFountainFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFountainFactoryLoader::~csFountainFactoryLoader ()
{
}

bool csFountainFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csFountainFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csFountainFactoryLoader::Parse (const char* /*string*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.fountain", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.fountain",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

csPtr<iBase> csFountainFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.fountain", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.fountain",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csFountainFactorySaver::csFountainFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFountainFactorySaver::~csFountainFactorySaver ()
{
}

bool csFountainFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csFountainFactorySaver::object_reg = object_reg;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csFountainFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // nothing to do
}

//---------------------------------------------------------------------------

csFountainLoader::csFountainLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFountainLoader::~csFountainLoader ()
{
}

bool csFountainLoader::Initialize (iObjectRegistry* object_reg)
{
  csFountainLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  xmltokens.Register ("accel", XMLTOKEN_ACCEL);
  xmltokens.Register ("azimuth", XMLTOKEN_AZIMUTH);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("dropsize", XMLTOKEN_DROPSIZE);
  xmltokens.Register ("elevation", XMLTOKEN_ELEVATION);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("falltime", XMLTOKEN_FALLTIME);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("number", XMLTOKEN_NUMBER);
  xmltokens.Register ("opening", XMLTOKEN_OPENING);
  xmltokens.Register ("origin", XMLTOKEN_ORIGIN);
  xmltokens.Register ("speed", XMLTOKEN_SPEED);
  return true;
}

csPtr<iBase> csFountainLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (ACCEL)
    CS_TOKEN_TABLE (SPEED)
    CS_TOKEN_TABLE (FALLTIME)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (OPENING)
    CS_TOKEN_TABLE (AZIMUTH)
    CS_TOKEN_TABLE (ELEVATION)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  csRef<iMeshObject> mesh;
  csRef<iParticleState> partstate;
  csRef<iFountainState> fountstate;

  csParser* parser = ldr_context->GetParser ();

  char* buf = (char*)string;
  while ((cmd = parser->GetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_COLOR:
	{
	  csColor color;
	  csScanStr (params, "%f,%f,%f", &color.red, &color.green, &color.blue);
	  partstate->SetColor (color);
	}
	break;
      case CS_TOKEN_DROPSIZE:
	{
	  float dw, dh;
	  csScanStr (params, "%f,%f", &dw, &dh);
	  fountstate->SetDropSize (dw, dh);
	}
	break;
      case CS_TOKEN_ORIGIN:
	{
	  csVector3 origin;
	  csScanStr (params, "%f,%f,%f", &origin.x, &origin.y, &origin.z);
	  fountstate->SetOrigin (origin);
	}
	break;
      case CS_TOKEN_ACCEL:
	{
	  csVector3 accel;
	  csScanStr (params, "%f,%f,%f", &accel.x, &accel.y, &accel.z);
	  fountstate->SetAcceleration (accel);
	}
	break;
      case CS_TOKEN_SPEED:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetSpeed (f);
	}
	break;
      case CS_TOKEN_OPENING:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetOpening (f);
	}
	break;
      case CS_TOKEN_AZIMUTH:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetAzimuth (f);
	}
	break;
      case CS_TOKEN_ELEVATION:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetElevation (f);
	}
	break;
      case CS_TOKEN_FALLTIME:
	{
	  float f;
	  csScanStr (params, "%f", &f);
	  fountstate->SetFallTime (f);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          fountstate = SCF_QUERY_INTERFACE (mesh, iFountainState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
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
          fountstate->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_NUMBER:
        {
          int nr;
          csScanStr (params, "%d", &nr);
          fountstate->SetParticleCount (nr);
        }
        break;
    }
  }

  return csPtr<iBase> (mesh);
}

csPtr<iBase> csFountainLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iParticleState> partstate;
  csRef<iFountainState> fountstate;

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
	  fountstate->SetDropSize (dw, dh);
	}
	break;
      case XMLTOKEN_ORIGIN:
	{
	  csVector3 origin;
	  origin.x = child->GetAttributeValueAsFloat ("x");
	  origin.y = child->GetAttributeValueAsFloat ("y");
	  origin.z = child->GetAttributeValueAsFloat ("z");
	  fountstate->SetOrigin (origin);
	}
	break;
      case XMLTOKEN_ACCEL:
	{
	  csVector3 accel;
	  accel.x = child->GetAttributeValueAsFloat ("x");
	  accel.y = child->GetAttributeValueAsFloat ("y");
	  accel.z = child->GetAttributeValueAsFloat ("z");
	  fountstate->SetAcceleration (accel);
	}
	break;
      case XMLTOKEN_SPEED:
	fountstate->SetSpeed (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_OPENING:
	fountstate->SetOpening (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_AZIMUTH:
	fountstate->SetAzimuth (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_ELEVATION:
	fountstate->SetElevation (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_FALLTIME:
	fountstate->SetFallTime (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.fountloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          fountstate = SCF_QUERY_INTERFACE (mesh, iFountainState);
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.fountloader.parse.unknownmaterial",
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
          fountstate->SetLighting (do_lighting);
        }
        break;
      case XMLTOKEN_NUMBER:
        fountstate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      default:
	synldr->ReportBadToken (child);
	return NULL;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------


csFountainSaver::csFountainSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFountainSaver::~csFountainSaver ()
{
}

bool csFountainSaver::Initialize (iObjectRegistry* object_reg)
{
  csFountainSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csFountainSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  csRef<iFactory> fact (SCF_QUERY_INTERFACE (this, iFactory));
  csRef<iParticleState> partstate (SCF_QUERY_INTERFACE (obj, iParticleState));
  csRef<iFountainState> state (SCF_QUERY_INTERFACE (obj, iFountainState));
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str.Append(buf);

  if(partstate->GetMixMode() != CS_FX_COPY)
  {
    str.Append (synldr->MixmodeToText(partstate->GetMixMode(), 2, true));
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
  sprintf(buf, "ORIGIN (%g, %g, %g)\n", state->GetOrigin().x,
    state->GetOrigin().y, state->GetOrigin().z);
  str.Append(buf);
  float sx = 0.0, sy = 0.0;
  state->GetDropSize(sx, sy);
  sprintf(buf, "DROPSIZE (%g, %g)\n", sx, sy);
  str.Append(buf);
  sprintf(buf, "ACCEL (%g, %g, %g)\n", state->GetAcceleration().x,
    state->GetAcceleration().y, state->GetAcceleration().z);
  str.Append(buf);
  sprintf(buf, "SPEED (%g)\n", state->GetSpeed());
  str.Append(buf);
  sprintf(buf, "OPENING (%g)\n", state->GetOpening());
  str.Append(buf);
  sprintf(buf, "AZIMUTH (%g)\n", state->GetAzimuth());
  str.Append(buf);
  sprintf(buf, "ELEVATION (%g)\n", state->GetElevation());
  str.Append(buf);
  sprintf(buf, "FALLTIME (%g)\n", state->GetFallTime());
  str.Append(buf);

  file->Write ((const char*)str, str.Length ());
}

//---------------------------------------------------------------------------

