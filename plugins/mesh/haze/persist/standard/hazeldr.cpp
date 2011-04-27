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
#include "hazeldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/partsys.h"
#include "imesh/haze.h"
#include "ivideo/graph3d.h"
#include "csqint.h"
#include "iutil/vfs.h"
#include "csutil/csstring.h"
#include "csutil/sysfunc.h"
#include "iutil/object.h"
#include "iutil/document.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/stringarray.h"
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"



enum
{
  XMLTOKEN_DIRECTIONAL = 1,
  XMLTOKEN_FACTORY,
  XMLTOKEN_HAZEBOX,
  XMLTOKEN_HAZECONE,
  XMLTOKEN_LAYER,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_ORIGIN,
  XMLTOKEN_SCALE
};

SCF_IMPLEMENT_FACTORY (csHazeFactoryLoader)
SCF_IMPLEMENT_FACTORY (csHazeFactorySaver)
SCF_IMPLEMENT_FACTORY (csHazeLoader)
SCF_IMPLEMENT_FACTORY (csHazeSaver)

csHazeFactoryLoader::csHazeFactoryLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csHazeFactoryLoader::~csHazeFactoryLoader ()
{
}

bool csHazeFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csHazeFactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  reporter = csQueryRegistry<iReporter> (object_reg);

  xmltokens.Register ("directional", XMLTOKEN_DIRECTIONAL);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("hazebox", XMLTOKEN_HAZEBOX);
  xmltokens.Register ("hazecone", XMLTOKEN_HAZECONE);
  xmltokens.Register ("layer", XMLTOKEN_LAYER);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("origin", XMLTOKEN_ORIGIN);
  xmltokens.Register ("scale", XMLTOKEN_SCALE);
  return true;
}

static iHazeHull* ParseHull (csStringHash& xmltokens, iReporter*,
			     iSyntaxService* synldr,
			     iDocumentNode* node,
			     iHazeFactoryState *fstate, float &s)
{
  csRef<iHazeHull> result;
  int number;
  float p, q;

  csRef<iHazeHullCreation> hullcreate (
  	scfQueryInterface<iHazeHullCreation> (fstate));

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_HAZEBOX:
        {
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  csRef<iHazeHullBox> ebox =
	    hullcreate->CreateBox (box.Min (), box.Max ());
	  result = scfQueryInterface<iHazeHull> (ebox);
	  CS_ASSERT (result);
	}
	break;
      case XMLTOKEN_HAZECONE:
        {
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  number = child->GetAttributeValueAsInt ("number");
	  p = child->GetAttributeValueAsFloat ("p");
	  q = child->GetAttributeValueAsFloat ("q");
	  csRef<iHazeHullCone> econe =
	    hullcreate->CreateCone (number, box.Min (), box.Max (), p, q);
	  result = scfQueryInterface<iHazeHull> (econe);
	  CS_ASSERT (result);
	}
	break;
      case XMLTOKEN_SCALE:
        s = child->GetContentsValueAsFloat ();
	break;
      default:
        synldr->ReportBadToken (child);
	return 0;
    }
  }
  result->IncRef ();	// Prevent smart pointer release.
  return result;
}

csPtr<iBase> csHazeFactoryLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase* /* context */)
{
  csVector3 a;

  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
  	object_reg, "crystalspace.mesh.object.haze");
  if (!type) return 0;
  csRef<iMeshObjectFactory> fact;
  fact = type->NewFactory ();
  csRef<iHazeFactoryState> hazefactorystate (
  	scfQueryInterface<iHazeFactoryState> (fact));
  CS_ASSERT (hazefactorystate);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
	    synldr->ReportError (
		"crystalspace.hazeloader.parse.badmaterial",
		child, "Could not find material %s!", CS::Quote::Single (matname));
            return 0;
	  }
	  fact->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
	{
	  uint mode;
	  if (!synldr->ParseMixmode (child, mode))
	    return 0;
          fact->SetMixMode (mode);
	}
	break;
      case XMLTOKEN_ORIGIN:
	if (!synldr->ParseVector (child, a))
	  return 0;
        hazefactorystate->SetOrigin (a);
	break;
      case XMLTOKEN_DIRECTIONAL:
	if (!synldr->ParseVector (child, a))
	  return 0;
        hazefactorystate->SetDirectional (a);
	break;
      case XMLTOKEN_LAYER:
        {
	  float layerscale = 1.0;
	  iHazeHull *hull = ParseHull (xmltokens, reporter, synldr,
	  	child, hazefactorystate, layerscale);
          hazefactorystate->AddLayer (hull, layerscale);
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

csHazeFactorySaver::csHazeFactorySaver (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csHazeFactorySaver::~csHazeFactorySaver ()
{
}

bool csHazeFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csHazeFactorySaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csHazeFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  if (!obj) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iHazeFactoryState> haze = scfQueryInterface<iHazeFactoryState> (obj);
  csRef<iMeshObjectFactory> mesh = scfQueryInterface<iMeshObjectFactory> (obj);

  if (mesh && haze)
  {
    //Writedown Material tag
    iMaterialWrapper* mat = mesh->GetMaterialWrapper();
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

    //Writedown Directional tag
    csVector3 direct = haze->GetDirectional();
    csRef<iDocumentNode> directNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    directNode->SetValue("directional");
    synldr->WriteVector(directNode, direct);

    //Writedown Origin tag
    csVector3 orig = haze->GetOrigin();
    csRef<iDocumentNode> origNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    origNode->SetValue("origin");
    synldr->WriteVector(origNode, orig);

    for (int i=0; i<(int)haze->GetLayerCount(); i++)
    {
      csRef<iDocumentNode> layerNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      layerNode->SetValue("layer");

      float scale = haze->GetLayerScale(i);
      csRef<iDocumentNode> scaleNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      scaleNode->SetValue("scale");
      scaleNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(scale);

      iHazeHull* hull = haze->GetLayerHull(i);
      csRef<iHazeHullBox> hullbox = scfQueryInterface<iHazeHullBox> (hull);
      csRef<iHazeHullCone> hullcone = scfQueryInterface<iHazeHullCone> (hull);
      if (hullbox)
      {
        csVector3 min, max;
        hullbox->GetSettings(min, max);
        csRef<iDocumentNode> boxNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        boxNode->SetValue("hazebox");
        csBox3 b(min, max);
        synldr->WriteBox(boxNode, b);
      }
      else if (hullcone)
      {
        int number;
        float p, q;
        csVector3 min, max;
        hullcone->GetSettings(number, min, max, p, q);
        csRef<iDocumentNode> coneNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        coneNode->SetValue("hazecone");
        csBox3 b(min, max);
        synldr->WriteBox(coneNode, b);
        coneNode->SetAttributeAsFloat("p", p);
        coneNode->SetAttributeAsFloat("q", q);
        coneNode->SetAttributeAsInt("number", number);
      }
    }
 
    //Writedown Mixmode tag
    int mixmode = mesh->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
  }
  return true;
}

//---------------------------------------------------------------------------

csHazeLoader::csHazeLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csHazeLoader::~csHazeLoader ()
{
}

bool csHazeLoader::Initialize (iObjectRegistry* object_reg)
{
  csHazeLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  reporter = csQueryRegistry<iReporter> (object_reg);

  xmltokens.Register ("directional", XMLTOKEN_DIRECTIONAL);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("hazebox", XMLTOKEN_HAZEBOX);
  xmltokens.Register ("hazecone", XMLTOKEN_HAZECONE);
  xmltokens.Register ("layer", XMLTOKEN_LAYER);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("origin", XMLTOKEN_ORIGIN);
  xmltokens.Register ("scale", XMLTOKEN_SCALE);
  return true;
}

#define CHECK_MESH(m) \
  if (!m) { \
    synldr->ReportError ( \
	"crystalspace.hazeloader.parse.unknownfactory", \
	child, "Specify the factory first!"); \
    return 0; \
  }

csPtr<iBase> csHazeLoader::Parse (iDocumentNode* node,
			    iStreamSource*, iLoaderContext* ldr_context,
			    iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iHazeFactoryState> hazefactorystate;
  csRef<iHazeState> hazestate;
  csVector3 a;

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

    if(!fact)
    {
      synldr->ReportError (
        "crystalspace.hazeloader.parse.badfactory",
        child, "Could not find factory %s!", CS::Quote::Single (factname));
      return 0;
    }

	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          hazestate = scfQueryInterface<iHazeState> (mesh);
	  if (!hazestate)
	  {
      	    synldr->ReportError (
		"crystalspace.hazeloader.parse.badfactory",
		child, "Factory %s doesn't appear to be a haze factory!",
		CS::Quote::Single (factname));
	    return 0;
	  }
	  hazefactorystate = scfQueryInterface<iHazeFactoryState> (
	  	fact->GetMeshObjectFactory());
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
	    synldr->ReportError (
		"crystalspace.hazeloader.parse.badmaterial",
		child, "Could not find material %s!",
		CS::Quote::Single (matname));
	    return 0;
	  }
	  CHECK_MESH (mesh);
	  mesh->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mode;
	  if (!synldr->ParseMixmode (child, mode))
	    return 0;
	  CHECK_MESH (mesh);
          mesh->SetMixMode (mode);
	}
	break;
      case XMLTOKEN_ORIGIN:
        if (!synldr->ParseVector (child, a))
	  return 0;
	CHECK_MESH (hazestate);
        hazestate->SetOrigin (a);
	break;
      case XMLTOKEN_DIRECTIONAL:
        if (!synldr->ParseVector (child, a))
	  return 0;
	CHECK_MESH (hazestate);
        hazestate->SetDirectional (a);
	break;
      case XMLTOKEN_LAYER:
        {
	  float layerscale = 1.0;
	  iHazeHull *hull = ParseHull (xmltokens, reporter, synldr,
	  	child, hazefactorystate, layerscale);
	  CHECK_MESH (hazestate);
          hazestate->AddLayer (hull, layerscale);
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


csHazeSaver::csHazeSaver (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csHazeSaver::~csHazeSaver ()
{
}

bool csHazeSaver::Initialize (iObjectRegistry* object_reg)
{
  csHazeSaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csHazeSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  if (!obj) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iHazeState> haze = scfQueryInterface<iHazeState> (obj);
  csRef<iMeshObject> mesh = scfQueryInterface<iMeshObject> (obj);

  if (mesh && haze)
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
        factNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(factname);
      }    
    }

    //Writedown Material tag
    iMaterialWrapper* mat = mesh->GetMaterialWrapper();
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

    //Writedown Directional tag
    csVector3 direct = haze->GetDirectional();
    csRef<iDocumentNode> directNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    directNode->SetValue("directional");
    synldr->WriteVector(directNode, direct);

    //Writedown Origin tag
    csVector3 orig = haze->GetOrigin();
    csRef<iDocumentNode> origNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    origNode->SetValue("origin");
    synldr->WriteVector(origNode, orig);

    for (int i=0; i<(int)haze->GetLayerCount(); i++)
    {
      csRef<iDocumentNode> layerNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      layerNode->SetValue("layer");

      float scale = haze->GetLayerScale(i);
      csRef<iDocumentNode> scaleNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      scaleNode->SetValue("scale");
      scaleNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(scale);

      iHazeHull* hull = haze->GetLayerHull(i);
      csRef<iHazeHullBox> hullbox = scfQueryInterface<iHazeHullBox> (hull);
      csRef<iHazeHullCone> hullcone = scfQueryInterface<iHazeHullCone> (hull);
      if (hullbox)
      {
        csVector3 min, max;
        hullbox->GetSettings(min, max);
        csRef<iDocumentNode> boxNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        boxNode->SetValue("hazebox");
        csBox3 b(min, max);
        synldr->WriteBox(boxNode, b);
      }
      else if (hullcone)
      {
        int number;
        float p, q;
        csVector3 min, max;
        hullcone->GetSettings(number, min, max, p, q);
        csRef<iDocumentNode> coneNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        coneNode->SetValue("hazecone");
        csBox3 b(min, max);
        synldr->WriteBox(coneNode, b);
        coneNode->SetAttributeAsFloat("p", p);
        coneNode->SetAttributeAsFloat("q", q);
        coneNode->SetAttributeAsInt("number", number);
      }
    }
 
    //Writedown Mixmode tag
    int mixmode = mesh->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
  }
  return true;
}
