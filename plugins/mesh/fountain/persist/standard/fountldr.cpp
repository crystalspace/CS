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
#include "fountldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/fountain.h"
#include "ivideo/graph3d.h"
#include "csqint.h"
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


csFountainFactoryLoader::csFountainFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFountainFactoryLoader::~csFountainFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFountainFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csFountainFactoryLoader::object_reg = object_reg;
  return true;
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
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFountainFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csFountainFactorySaver::object_reg = object_reg;
  return true;
}

bool csFountainFactorySaver::WriteDown (iBase* /*obj*/, iDocumentNode* parent)
{
  //Nothing gets parsed in the loader, so nothing gets saved here!
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  return true;
}

//---------------------------------------------------------------------------

csFountainLoader::csFountainLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csFountainLoader::~csFountainLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
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
	    return 0;
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
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          fountstate = SCF_QUERY_INTERFACE (mesh, iFountainState);
	  if (!fountstate)
	  {
      	    synldr->ReportError (
		"crystalspace.fountstate.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a fountain factory!",
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
		"crystalspace.fountloader.parse.unknownmaterial",
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
          fountstate->SetLighting (do_lighting);
        }
        break;
      case XMLTOKEN_NUMBER:
        fountstate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
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
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csFountainSaver::Initialize (iObjectRegistry* object_reg)
{
  csFountainSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csFountainSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iParticleState> partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  csRef<iFountainState> fountainstate = SCF_QUERY_INTERFACE (obj, iFountainState);
  csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);

  if ( partstate && fountainstate && mesh)
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

    //Writedown DropSize tag
    float dw, dh;
    fountainstate->GetDropSize(dw, dh);
    csRef<iDocumentNode> dropsizeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    dropsizeNode->SetValue("dropsize");
    dropsizeNode->SetAttributeAsFloat("w", dw);
    dropsizeNode->SetAttributeAsFloat("h", dh);

    //Writedown OriginBox tag
    csRef<iDocumentNode> originboxNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    originboxNode->SetValue("origin");
    csBox3 originbox = fountainstate->GetOrigin();
    synldr->WriteBox(originboxNode, &originbox);

    //Writedown Accel tag
    csVector3 accel = fountainstate->GetAcceleration();
    csRef<iDocumentNode> accelNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    accelNode->SetValue("accel");
    synldr->WriteVector(accelNode, &accel);

    //Writedown Speed tag
    float speed = fountainstate->GetSpeed();
    csRef<iDocumentNode> speedNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    speedNode->SetValue("speed");
    speedNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(speed);

    //Writedown Opening tag
    float opening = fountainstate->GetOpening();
    csRef<iDocumentNode> openingNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    openingNode->SetValue("opening");
    openingNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(opening);

    //Writedown Azimuth tag
    float azimuth = fountainstate->GetAzimuth();
    csRef<iDocumentNode> azimuthNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    azimuthNode->SetValue("azimuth");
    azimuthNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(azimuth);

    //Writedown Elevation tag
    float elevation = fountainstate->GetElevation();
    csRef<iDocumentNode> elevationNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    elevationNode->SetValue("elevation");
    elevationNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(elevation);

    //Writedown FallTime tag
    float falltime = fountainstate->GetFallTime();
    csRef<iDocumentNode> falltimeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    falltimeNode->SetValue("falltime");
    falltimeNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(falltime);

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
    synldr->WriteBool(paramsNode, "lighting", fountainstate->GetLighting(), true);

    //Writedown Number tag
    int number = fountainstate->GetParticleCount();
    csRef<iDocumentNode> numberNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    numberNode->SetValue("number");
    csRef<iDocumentNode> numberValueNode = numberNode->CreateNodeBefore(CS_NODE_TEXT, 0);
    numberValueNode->SetValueAsInt(number);
  }

  paramsNode=0;
  
  return true;
}

//---------------------------------------------------------------------------

