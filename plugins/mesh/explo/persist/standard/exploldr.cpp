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
#include "exploldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/explode.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_CENTER = 1,
  XMLTOKEN_COLOR,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FADE,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_NUMBER,
  XMLTOKEN_NRSIDES,
  XMLTOKEN_PARTRADIUS,
  XMLTOKEN_PUSH,
  XMLTOKEN_SPREADPOS,
  XMLTOKEN_SPREADSPEED,
  XMLTOKEN_SPREADACCEL
};

SCF_IMPLEMENT_IBASE (csExplosionFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExplosionFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csExplosionFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExplosionFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csExplosionLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExplosionLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csExplosionSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csExplosionSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csExplosionFactoryLoader)
SCF_IMPLEMENT_FACTORY (csExplosionFactorySaver)
SCF_IMPLEMENT_FACTORY (csExplosionLoader)
SCF_IMPLEMENT_FACTORY (csExplosionSaver)

SCF_EXPORT_CLASS_TABLE (exploldr)
  SCF_EXPORT_CLASS (csExplosionFactoryLoader,
    "crystalspace.mesh.loader.factory.explosion",
    "Crystal Space Explosion Factory Loader")
  SCF_EXPORT_CLASS (csExplosionFactorySaver,
    "crystalspace.mesh.saver.factory.explosion",
    "Crystal Space Explosion Factory Saver")
  SCF_EXPORT_CLASS (csExplosionLoader, "crystalspace.mesh.loader.explosion",
    "Crystal Space Explosion Mesh Loader")
  SCF_EXPORT_CLASS (csExplosionSaver, "crystalspace.mesh.saver.explosion",
    "Crystal Space Explosion Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

csExplosionFactoryLoader::csExplosionFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionFactoryLoader::~csExplosionFactoryLoader ()
{
}

bool csExplosionFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csExplosionFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csExplosionFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.explosion", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.explosion",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csExplosionFactorySaver::csExplosionFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionFactorySaver::~csExplosionFactorySaver ()
{
}

bool csExplosionFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csExplosionFactorySaver::object_reg = object_reg;
  return true;
}

void csExplosionFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // nothing to do
}

//---------------------------------------------------------------------------

csExplosionLoader::csExplosionLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionLoader::~csExplosionLoader ()
{
}

bool csExplosionLoader::Initialize (iObjectRegistry* object_reg)
{
  csExplosionLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  xmltokens.Register ("center", XMLTOKEN_CENTER);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("fade", XMLTOKEN_FADE);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("number", XMLTOKEN_NUMBER);
  xmltokens.Register ("nrsides", XMLTOKEN_NRSIDES);
  xmltokens.Register ("partradius", XMLTOKEN_PARTRADIUS);
  xmltokens.Register ("push", XMLTOKEN_PUSH);
  xmltokens.Register ("spreadpos", XMLTOKEN_SPREADPOS);
  xmltokens.Register ("spreadspeed", XMLTOKEN_SPREADSPEED);
  xmltokens.Register ("spreadaccel", XMLTOKEN_SPREADACCEL);

  return true;
}

csPtr<iBase> csExplosionLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iParticleState> partstate;
  csRef<iExplosionState> explostate;

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
      case XMLTOKEN_CENTER:
	{
	  csVector3 center;
	  if (!synldr->ParseVector (child, center))
	    return NULL;
	  explostate->SetCenter (center);
	}
	break;
      case XMLTOKEN_PUSH:
	{
	  csVector3 push;
	  if (!synldr->ParseVector (child, push))
	    return NULL;
	  explostate->SetPush (push);
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError ("crystalspace.exploader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          explostate = SCF_QUERY_INTERFACE (mesh, iExplosionState);
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError ("crystalspace.exploader.parse.unknownmaterial",
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
          explostate->SetLighting (do_lighting);
        }
        break;
      case XMLTOKEN_NUMBER:
        explostate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_NRSIDES:
        explostate->SetNrSides (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_FADE:
        explostate->SetFadeSprites (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_PARTRADIUS:
        explostate->SetPartRadius (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_SPREADPOS:
        explostate->SetSpreadPos (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_SPREADSPEED:
        explostate->SetSpreadSpeed (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_SPREADACCEL:
        explostate->SetSpreadAcceleration (child->GetContentsValueAsFloat ());
        break;
      default:
        synldr->ReportBadToken (child);
	return NULL;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------


csExplosionSaver::csExplosionSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionSaver::~csExplosionSaver ()
{
}

bool csExplosionSaver::Initialize (iObjectRegistry* object_reg)
{
  csExplosionSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

#define MAXLINE	    80

void csExplosionSaver::WriteDown (iBase* obj, iFile *file)
{
  csString str;
  csRef<iFactory> fact (SCF_QUERY_INTERFACE (this, iFactory));
  csRef<iParticleState> partstate (SCF_QUERY_INTERFACE (obj, iParticleState));
  csRef<iExplosionState> explostate (
  	SCF_QUERY_INTERFACE (obj, iExplosionState));
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

  sprintf(buf, "CENTER (%g, %g, %g)\n", explostate->GetCenter().x,
    explostate->GetCenter().y, explostate->GetCenter().z);
  str.Append(buf);
  sprintf(buf, "PUSH (%g, %g, %g)\n", explostate->GetPush().x,
    explostate->GetPush().y, explostate->GetPush().z);
  str.Append(buf);
  sprintf(buf, "SPREADPOS (%g)\n", explostate->GetSpreadPos());
  str.Append(buf);
  sprintf(buf, "SPREADSPEED (%g)\n", explostate->GetSpreadSpeed());
  str.Append(buf);
  sprintf(buf, "SPREADACCEL (%g)\n", explostate->GetSpreadAcceleration());
  str.Append(buf);
  sprintf(buf, "NUMBER (%d)\n", explostate->GetParticleCount());
  str.Append(buf);
  sprintf(buf, "NRSIDES (%d)\n", explostate->GetNrSides());
  str.Append(buf);
  sprintf(buf, "PARTRADIUS (%g)\n", explostate->GetPartRadius());
  str.Append(buf);
  sprintf(buf, "LIGHTING (%s)\n", explostate->GetLighting()?"true":"false");
  str.Append(buf);
  csTicks fade_time = 0;
  if(explostate->GetFadeSprites(fade_time))
  {
    sprintf(buf, "FADE (%d)\n", (int)fade_time);
    str.Append(buf);
  }

  file->Write ((const char*)str, str.Length ());
}

//---------------------------------------------------------------------------

