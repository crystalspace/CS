/*
Copyright (C) 2008 by Pavel Krajcevski

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
#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector4.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/object.h"
#include "imesh/watermesh.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"

#include "watermeshldr.h"

using namespace CS::Plugins::WaterMeshLoader;


SCF_IMPLEMENT_FACTORY (csWaterFactoryLoader)
SCF_IMPLEMENT_FACTORY (csWaterFactorySaver)
SCF_IMPLEMENT_FACTORY (csWaterMeshLoader)
SCF_IMPLEMENT_FACTORY (csWaterMeshSaver)


csWaterFactoryLoader::csWaterFactoryLoader (iBase* pParent) :
  scfImplementationType (this, pParent)
{
}

csWaterFactoryLoader::~csWaterFactoryLoader ()
{
}

bool csWaterFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csWaterFactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  return true;
}

csPtr<iBase> csWaterFactoryLoader::Parse (iDocumentNode* node,
                                          iStreamSource*, 
                                          iLoaderContext* /*ldr_context*/, 
                                          iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr = 
    csQueryRegistry<iPluginManager> (object_reg);
  csRef<iMeshObjectType> type = csQueryPluginClass<iMeshObjectType> (
    plugin_mgr, "crystalspace.mesh.object.watermesh");
  if (!type)
  {
    type = csLoadPlugin<iMeshObjectType> (plugin_mgr, 
      "crystalspace.mesh.object.watermesh");
  }
  if (!type)
  {
    synldr->ReportError (
		"crystalspace.watermeshfactoryloader.setup.objecttype",
		node, "Could not load the general mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact;
  csRef<iWaterFactoryState> state;

  fact = type->NewFactory ();
  state = scfQueryInterface<iWaterFactoryState> (fact);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LENGTH:
        {
			int len = child->GetContentsValueAsInt();
			state->SetLength(len);
        }
	break;
	  case XMLTOKEN_WIDTH:
        {
			int wid = child->GetContentsValueAsInt();
			state->SetWidth(wid);
        }
	break;
	  case XMLTOKEN_GRAN:
        {
			int gran = child->GetContentsValueAsInt();
			state->SetGranularity(gran);
        }
	break;
	  case XMLTOKEN_MURK:
        {
			float murk = child->GetContentsValueAsFloat();
			state->SetMurkiness(murk);
        }
	break;
	  case XMLTOKEN_ISOCEAN:
	    {
			bool makeOcean;
			synldr->ParseBool(child, makeOcean, false);
			if(makeOcean) 
				state->SetWaterType(iWaterFactoryState::WATER_TYPE_OCEAN);
			else
				state->SetWaterType(iWaterFactoryState::WATER_TYPE_LOCAL);
        }
		break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (fact);
}
//---------------------------------------------------------------------------

csWaterFactorySaver::csWaterFactorySaver (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

csWaterFactorySaver::~csWaterFactorySaver ()
{
}

bool csWaterFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csWaterFactorySaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csWaterFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iWaterFactoryState> gfact = 
      scfQueryInterface<iWaterFactoryState> (obj);
    csRef<iMeshObjectFactory> meshfact = 
      scfQueryInterface<iMeshObjectFactory> (obj);
    if (!gfact) return false;
    if (!meshfact) return false;

    csRef<iDocumentNode> vNode = 
      paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    vNode->SetValue ("width");
    vNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (gfact->GetWidth());

	vNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	vNode->SetValue ("length");
    vNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (gfact->GetLength());

    vNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	vNode->SetValue ("gran");
    vNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (gfact->GetGranularity());

    vNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	vNode->SetValue ("murk");
    vNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValueAsFloat (gfact->GetMurkiness());

    vNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
	vNode->SetValue ("isocean");
    vNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue ((gfact->isOcean())? "yes" : "no");
  }
  return true;
}

//---------------------------------------------------------------------------

csWaterMeshLoader::csWaterMeshLoader (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

csWaterMeshLoader::~csWaterMeshLoader ()
{
}

bool csWaterMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csWaterMeshLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  return true;
}

#define CHECK_MESH(m) \
  if (!m) { \
    synldr->ReportError ( \
	"crystalspace.watermeshloader.parse.unknownfactory", \
	child, "Specify the factory first!"); \
    return 0; \
  }


csPtr<iBase> csWaterMeshLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iWaterMeshState> meshstate;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.watermeshloader.parse.unknownfactory",
		child, "Couldn't find factory %s!", CS::Quote::Single (factname));
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  CS_ASSERT (mesh != 0);
          meshstate = scfQueryInterface<iWaterMeshState> (mesh);
	  if (!meshstate)
	  {
      	    synldr->ReportError (
		"crystalspace.watermeshloader.parse.badfactory",
		child, "Factory %s doesn't appear to be a watermesh factory!",
		CS::Quote::Single (factname));
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
		"crystalspace.watermeshloader.parse.unknownmaterial",
		child, "Couldn't find material %s!", CS::Quote::Single (matname));
            return 0;
	  }
	  CHECK_MESH (mesh);
	  mesh->SetMaterialWrapper (mat);
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

csWaterMeshSaver::csWaterMeshSaver (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

csWaterMeshSaver::~csWaterMeshSaver ()
{
}

bool csWaterMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  csWaterMeshSaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csWaterMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iWaterMeshState> gmesh = 
      scfQueryInterface<iWaterMeshState> (obj);
    csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (obj);
    if (!gmesh) return false;
    if (!mesh) return false;

    // Writedown Factory tag
    iMeshFactoryWrapper* fact = mesh->GetFactory()->GetMeshFactoryWrapper ();
    if (fact)
    {
      const char* factname = fact->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = 
          paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
        factNode->SetValue ("factory");
        factNode->CreateNodeBefore (CS_NODE_TEXT, 0)->SetValue (factname);
      }    
    }

    // Writedown Material tag
    iMaterialWrapper* mat = mesh->GetMaterialWrapper ();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName ();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = 
          paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
        matNode->SetValue ("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore (CS_NODE_TEXT, 0);
        matnameNode->SetValue (matname);
      }    
    }    
  }
  return true;
}
