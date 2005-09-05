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
#include "csqint.h"
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


csExplosionFactoryLoader::csExplosionFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionFactoryLoader::~csExplosionFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
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
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csExplosionFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csExplosionFactorySaver::object_reg = object_reg;
  return true;
}

bool csExplosionFactorySaver::WriteDown (iBase* /*obj*/, iDocumentNode* parent)
{
  //Nothing gets parsed in the loader, so nothing gets saved here!
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  return true;
}

//---------------------------------------------------------------------------

csExplosionLoader::csExplosionLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csExplosionLoader::~csExplosionLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
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
	    return 0;
	  partstate->SetColor (color);
	}
	break;
      case XMLTOKEN_CENTER:
	{
	  csVector3 center;
	  if (!synldr->ParseVector (child, center))
	    return 0;
	  explostate->SetCenter (center);
	}
	break;
      case XMLTOKEN_PUSH:
	{
	  csVector3 push;
	  if (!synldr->ParseVector (child, push))
	    return 0;
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
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          explostate = SCF_QUERY_INTERFACE (mesh, iExplosionState);
	  if (!explostate)
	  {
      	    synldr->ReportError (
		"crystalspace.exploader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be an explosion factory!",
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
      	    synldr->ReportError ("crystalspace.exploader.parse.unknownmaterial",
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
	return 0;
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
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csExplosionSaver::Initialize (iObjectRegistry* object_reg)
{
  csExplosionSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csExplosionSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iParticleState> partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  csRef<iExplosionState> explosionstate = SCF_QUERY_INTERFACE (obj, iExplosionState);
  csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);

  if ( partstate && explosionstate && mesh )
  {
    //Writedown Factory tag
    iMeshFactoryWrapper* fact = mesh->GetFactory()->GetMeshFactoryWrapper();
    if (fact)
    {
      const char* factname = fact->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        factNode->SetValue("factory");
        csRef<iDocumentNode> factnameNode = factNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        factnameNode->SetValue(factname);
      }    
    }    

    //Writedown Color tag
    csColor col = partstate->GetColor();
    csRef<iDocumentNode> colorNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    colorNode->SetValue("color");
    synldr->WriteColor(colorNode, &col);

    //Writedown Center tag
    csVector3 center = explosionstate->GetCenter();
    csRef<iDocumentNode> centerNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    centerNode->SetValue("center");
    synldr->WriteVector(centerNode, &center);

    //Writedown Push tag
    csVector3 push = explosionstate->GetPush();
    csRef<iDocumentNode> pushNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    pushNode->SetValue("push");
    synldr->WriteVector(pushNode, &push);

    //Writedown PartRadius tag
    float partradius = explosionstate->GetPartRadius();
    csRef<iDocumentNode> partradiusNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    partradiusNode->SetValue("partradius");
    partradiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(partradius);

    //Writedown SpreadPos tag
    float spreadpos = explosionstate->GetSpreadPos();
    csRef<iDocumentNode> spreadposNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    spreadposNode->SetValue("spreadpos");
    spreadposNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(spreadpos);

    //Writedown SpreadSpeed tag
    float spreadspeed = explosionstate->GetSpreadSpeed();
    csRef<iDocumentNode> spreadspeedNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    spreadspeedNode->SetValue("spreadspeed");
    spreadspeedNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(spreadspeed);

    //Writedown SpreadAcceleration tag
    float spreadacceleration = explosionstate->GetSpreadAcceleration();
    csRef<iDocumentNode> spreadaccelerationNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    spreadaccelerationNode->SetValue("spreadacceleration");
    spreadaccelerationNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(spreadacceleration);

    //Writedown Fade tag
    csTicks fade;
    explosionstate->GetFadeSprites(fade);
    csRef<iDocumentNode> fadeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    fadeNode->SetValue("fade");
    fadeNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(fade);

    //Writedown NrSides tag
    int nrsides = explosionstate->GetNrSides();
    csRef<iDocumentNode> nrsidesNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    nrsidesNode->SetValue("nrsides");
    nrsidesNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(nrsides);

    //Writedown Material tag
    iMaterialWrapper* mat = partstate->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }
    }

    //Writedown Mixmode tag
    int mixmode = partstate->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
	  
    //Writedown Lighting tag
    synldr->WriteBool(paramsNode, "lighting", explosionstate->GetLighting(), true);

    //Writedown Number tag
    int number = explosionstate->GetParticleCount();
    csRef<iDocumentNode> numberNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    numberNode->SetValue("number");
    numberNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(number);
  }

  paramsNode=0;

  return true;
}

//---------------------------------------------------------------------------
