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
#include "spirldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/spiral.h"
#include "ivideo/graph3d.h"
#include "csqint.h"
#include "csutil/csstring.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_COLOR = 1,
  XMLTOKEN_FACTORY,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_NUMBER,
  XMLTOKEN_SOURCE,
  XMLTOKEN_PARTICLESIZE,
  XMLTOKEN_PARTICLETIME,
  XMLTOKEN_RADIALSPEED,
  XMLTOKEN_ROTATIONSPEED,
  XMLTOKEN_CLIMBSPEED
};

SCF_IMPLEMENT_IBASE (csSpiralFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpiralFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpiralFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpiralFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpiralLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpiralLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csSpiralSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSpiralSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csSpiralFactoryLoader)
SCF_IMPLEMENT_FACTORY (csSpiralFactorySaver)
SCF_IMPLEMENT_FACTORY (csSpiralLoader)
SCF_IMPLEMENT_FACTORY (csSpiralSaver)


csSpiralFactoryLoader::csSpiralFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpiralFactoryLoader::~csSpiralFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpiralFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csSpiralFactoryLoader::object_reg = object_reg;
  return true;
}

csPtr<iBase> csSpiralFactoryLoader::Parse (iDocumentNode* /*node*/,
	iLoaderContext*, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.spiral", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.spiral",
    	iMeshObjectType);
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------
csSpiralFactorySaver::csSpiralFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpiralFactorySaver::~csSpiralFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpiralFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csSpiralFactorySaver::object_reg = object_reg;
  return true;
}

bool csSpiralFactorySaver::WriteDown (iBase* /*obj*/, iDocumentNode* parent)
{
  //Nothing gets parsed in the loader, so nothing gets saved here!
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  return true;
}
//---------------------------------------------------------------------------
csSpiralLoader::csSpiralLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpiralLoader::~csSpiralLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpiralLoader::Initialize (iObjectRegistry* object_reg)
{
  csSpiralLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("number", XMLTOKEN_NUMBER);
  xmltokens.Register ("source", XMLTOKEN_SOURCE);
  xmltokens.Register ("particlesize", XMLTOKEN_PARTICLESIZE);
  xmltokens.Register ("particletime", XMLTOKEN_PARTICLETIME);
  xmltokens.Register ("radialspeed", XMLTOKEN_RADIALSPEED);
  xmltokens.Register ("rotationspeed", XMLTOKEN_ROTATIONSPEED);
  xmltokens.Register ("climbspeed", XMLTOKEN_CLIMBSPEED);
  return true;
}

csPtr<iBase> csSpiralLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iParticleState> partstate;
  csRef<iSpiralState> spiralstate;

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
      case XMLTOKEN_SOURCE:
	{
	  csVector3 s;
	  if (!synldr->ParseVector (child, s))
	    return 0;
	  spiralstate->SetSource (s);
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.spiralloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          partstate = SCF_QUERY_INTERFACE (mesh, iParticleState);
          spiralstate = SCF_QUERY_INTERFACE (mesh, iSpiralState);
	  if (!spiralstate)
	  {
      	    synldr->ReportError (
		"crystalspace.spiralloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a spiral factory!",
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
		"crystalspace.ballloader.parse.unknownmaterial",
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
      case XMLTOKEN_NUMBER:
        spiralstate->SetParticleCount (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_PARTICLESIZE:
        {
            
	  float dw, dh;
	  dw = child->GetAttributeValueAsFloat ("w");
	  dh = child->GetAttributeValueAsFloat ("h");
          spiralstate->SetParticleSize (dw, dh);
        }
        break;
      case XMLTOKEN_PARTICLETIME:
        spiralstate->SetParticleTime (child->GetContentsValueAsInt ());
        break;
      case XMLTOKEN_RADIALSPEED:
        spiralstate->SetRadialSpeed (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_ROTATIONSPEED:
        spiralstate->SetRotationSpeed (child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_CLIMBSPEED:
        spiralstate->SetClimbSpeed (child->GetContentsValueAsFloat ());
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------

csSpiralSaver::csSpiralSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csSpiralSaver::~csSpiralSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csSpiralSaver::Initialize (iObjectRegistry* object_reg)
{
  csSpiralSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csSpiralSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iParticleState> partstate = SCF_QUERY_INTERFACE (obj, iParticleState);
  csRef<iSpiralState> spiralstate = SCF_QUERY_INTERFACE (obj, iSpiralState);
  csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);

  if ( partstate && spiralstate && mesh)
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

    //Writedown Source tag
    csVector3 source = spiralstate->GetSource();
    csRef<iDocumentNode> sourceNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    sourceNode->SetValue("source");
    synldr->WriteVector(sourceNode, &source);

    //Writedown ParticleSize tag
    float dw, dh;
    spiralstate->GetParticleSize(dw, dh);
    csRef<iDocumentNode> particlesizeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    particlesizeNode->SetValue("particlesize");
    particlesizeNode->SetAttributeAsFloat("w", dw);
    particlesizeNode->SetAttributeAsFloat("h", dh);

    //Writedown ParticleTime tag
    int particletime = spiralstate->GetParticleTime();
    csRef<iDocumentNode> particletimeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    particletimeNode->SetValue("particletime");
    particletimeNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(particletime);

    //Writedown RadialSpeed tag
    float radialspeed = spiralstate->GetRadialSpeed();
    csRef<iDocumentNode> radialspeedNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    radialspeedNode->SetValue("radialspeed");
    radialspeedNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(radialspeed);

    //Writedown RotationSpeed tag
    float rotationspeed = spiralstate->GetRotationSpeed();
    csRef<iDocumentNode> rotationspeedNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    rotationspeedNode->SetValue("rotationspeed");
    rotationspeedNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(rotationspeed);

    //Writedown ClimbSpeed tag
    float climbspeed = spiralstate->GetClimbSpeed();
    csRef<iDocumentNode> climbspeedNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    climbspeedNode->SetValue("climbspeed");
    climbspeedNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(climbspeed);

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
	  
    //Writedown Number tag
    int number = spiralstate->GetParticleCount();
    csRef<iDocumentNode> numberNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    numberNode->SetValue("number");
    csRef<iDocumentNode> numberValueNode = numberNode->CreateNodeBefore(CS_NODE_TEXT, 0);
    numberValueNode->SetValueAsInt(number);
  }

  paramsNode=0;
  
  return true;
}
