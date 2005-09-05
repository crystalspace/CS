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
#include "snowldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "imesh/partsys.h"
#include "imesh/snow.h"
#include "ivideo/graph3d.h"
#include "csqint.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "iutil/vfs.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
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
  XMLTOKEN_BOX,
  XMLTOKEN_SWIRL
};

SCF_IMPLEMENT_IBASE (csSnowFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSnowFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSnowFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSnowFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSnowLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSnowLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSnowSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSnowSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSnowFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSnowFactorySaver)
SCF_IMPLEMENT_FACTORY (csSnowLoader)
SCF_IMPLEMENT_FACTORY (csSnowSaver)


csSnowFactoryLoader::csSnowFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSnowFactoryLoader::~csSnowFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSnowFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSnowFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csSnowFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.snow", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.snow",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csSnowFactorySaver::csSnowFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSnowFactorySaver::~csSnowFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSnowFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSnowFactorySaver::object_reg = object_reg;
  return true;
}

bool csSnowFactorySaver::WriteDown (iBase* /*obj*/, iDocumentNode* parent)
{
  //Nothing gets parsed in the loader, so nothing gets saved here!
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  return true;
}

//---------------------------------------------------------------------------
csSnowLoader::csSnowLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSnowLoader::~csSnowLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSnowLoader::Initialize (iObjectRegistry* object_reg)
{
  csSnowLoader::object_reg = object_reg;
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
  xmltokens.Register ("swirl", XMLTOKEN_SWIRL);
  return true;
}

csPtr<iBase> csSnowLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iParticleState> partstate;
  csRef<iSnowState> snowstate;

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
	  snowstate->SetDropSize (dw, dh);
	}
	break;
      case XMLTOKEN_BOX:
	{
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  snowstate->SetBox (box.Min (), box.Max ());
	}
	break;
      case XMLTOKEN_FALLSPEED:
	{
	  csVector3 s;
	  if (!synldr->ParseVector (child, s))
	    return 0;
	  snowstate->SetFallSpeed (s);
	}
	break;
      case XMLTOKEN_SWIRL:
	snowstate->SetSwirl (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.snowloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          snowstate = SCF_QUERY_INTERFACE (mesh, iSnowState);
	  if (!snowstate)
	  {
      	    synldr->ReportError (
		"crystalspace.snowloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a snow factory!",
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
		"crystalspace.snowloader.parse.unknownmaterial",
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
          snowstate->SetLighting (do_lighting);
        }
        break;
      case XMLTOKEN_NUMBER:
        snowstate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      default:
	synldr->ReportBadToken (child);
        return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------

csSnowSaver::csSnowSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSnowSaver::~csSnowSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSnowSaver::Initialize (iObjectRegistry* object_reg)
{
  csSnowSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csSnowSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iParticleState> partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  csRef<iSnowState> snowstate = SCF_QUERY_INTERFACE (obj, iSnowState);
  csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);

  if ( partstate && snowstate && mesh )
  {
    //Writedown Factory tag
    iMeshFactoryWrapper* fact = mesh->GetFactory()->GetMeshFactoryWrapper ();
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

    //Writedown DropSize tag
    float dw, dh;
    snowstate->GetDropSize(dw, dh);
    csRef<iDocumentNode> dropsizeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    dropsizeNode->SetValue("dropsize");
    dropsizeNode->SetAttributeAsFloat("w", dw);
    dropsizeNode->SetAttributeAsFloat("h", dh);

    //Writedown Box tag
    csVector3 minBox, maxBox;
    snowstate->GetBox(minBox, maxBox);
    csBox3 box(minBox, maxBox);
    csRef<iDocumentNode> boxNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    boxNode->SetValue("box");
    synldr->WriteBox(boxNode, &box);

    //Writedown FallSpeed tag
    csVector3 fallspeed = snowstate->GetFallSpeed();
    csRef<iDocumentNode> fallspeedNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    fallspeedNode->SetValue("fallspeed");
    synldr->WriteVector(fallspeedNode, &fallspeed);

    //Writedown Swirl tag
    float swirl = snowstate->GetSwirl();
    csRef<iDocumentNode> swirlNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    swirlNode->SetValue("swirl");
    swirlNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(swirl);

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
    synldr->WriteBool(paramsNode, "lighting", snowstate->GetLighting(), true);

    //Writedown Number tag
    int number = snowstate->GetParticleCount();
    csRef<iDocumentNode> numberNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    numberNode->SetValue("number");
    csRef<iDocumentNode> numberValueNode = numberNode->CreateNodeBefore(CS_NODE_TEXT, 0);
    numberValueNode->SetValueAsInt(number);
  }

  paramsNode=0;
  
  return true;
}

