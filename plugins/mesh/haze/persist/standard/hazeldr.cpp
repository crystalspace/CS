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
#include "imap/ldrctxt.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

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

SCF_IMPLEMENT_IBASE (csHazeFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csHazeFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csHazeLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csHazeSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csHazeSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csHazeFactoryLoader)
SCF_IMPLEMENT_FACTORY (csHazeFactorySaver)
SCF_IMPLEMENT_FACTORY (csHazeLoader)
SCF_IMPLEMENT_FACTORY (csHazeSaver)


csHazeFactoryLoader::csHazeFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csHazeFactoryLoader::~csHazeFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csHazeFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csHazeFactoryLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

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
  	SCF_QUERY_INTERFACE (fstate, iHazeHullCreation));

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
	  result = SCF_QUERY_INTERFACE (ebox, iHazeHull);
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
	  result = SCF_QUERY_INTERFACE (econe, iHazeHull);
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
	iLoaderContext* ldr_context,
	iBase* /* context */)
{
  csVector3 a;

  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.haze", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.haze",
    	iMeshObjectType);
    csPrintf ("Load TYPE plugin crystalspace.mesh.object.haze\n");
  }
  csRef<iMeshObjectFactory> fact;
  fact = type->NewFactory ();
  csRef<iHazeFactoryState> hazefactorystate (
  	SCF_QUERY_INTERFACE (fact, iHazeFactoryState));
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
		child, "Could not find material '%s'!", matname);
            return 0;
	  }
	  hazefactorystate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
	{
	  uint mode;
	  if (!synldr->ParseMixmode (child, mode))
	    return 0;
          hazefactorystate->SetMixMode (mode);
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

csHazeFactorySaver::csHazeFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csHazeFactorySaver::~csHazeFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csHazeFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csHazeFactorySaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csHazeFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iHazeFactoryState> haze = SCF_QUERY_INTERFACE (obj, iHazeFactoryState);
  csRef<iMeshObjectFactory> mesh = SCF_QUERY_INTERFACE (obj, iMeshObjectFactory);

  if (mesh && haze)
  {
    //Writedown Material tag
    iMaterialWrapper* mat = haze->GetMaterialWrapper();
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
    synldr->WriteVector(directNode, &direct);

    //Writedown Origin tag
    csVector3 orig = haze->GetOrigin();
    csRef<iDocumentNode> origNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    origNode->SetValue("origin");
    synldr->WriteVector(origNode, &orig);

    for (int i=0; i<haze->GetLayerCount(); i++)
    {
      csRef<iDocumentNode> layerNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      layerNode->SetValue("layer");

      float scale = haze->GetLayerScale(i);
      csRef<iDocumentNode> scaleNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      scaleNode->SetValue("scale");
      scaleNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(scale);

      iHazeHull* hull = haze->GetLayerHull(i);
      csRef<iHazeHullBox> hullbox = SCF_QUERY_INTERFACE(hull, iHazeHullBox);
      csRef<iHazeHullCone> hullcone = SCF_QUERY_INTERFACE(hull, iHazeHullCone);
      if (hullbox)
      {
        csVector3 min, max;
        hullbox->GetSettings(min, max);
        csRef<iDocumentNode> boxNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        boxNode->SetValue("hazebox");
        csBox3 b(min, max);
        synldr->WriteBox(boxNode,&b);
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
        synldr->WriteBox(coneNode,&b);
        coneNode->SetAttributeAsFloat("p", p);
        coneNode->SetAttributeAsFloat("q", q);
        coneNode->SetAttributeAsInt("number", number);
      }
    }
 
    //Writedown Mixmode tag
    int mixmode = haze->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
  }
  return true;
}

//---------------------------------------------------------------------------

csHazeLoader::csHazeLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csHazeLoader::~csHazeLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csHazeLoader::Initialize (iObjectRegistry* object_reg)
{
  csHazeLoader::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);

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

csPtr<iBase> csHazeLoader::Parse (iDocumentNode* node,
			    iLoaderContext* ldr_context, iBase*)
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
	  if (!fact)
	  {
	    synldr->ReportError (
		"crystalspace.hazeloader.parse.badfactory",
		child, "Could not find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          hazestate = SCF_QUERY_INTERFACE (mesh, iHazeState);
	  if (!hazestate)
	  {
      	    synldr->ReportError (
		"crystalspace.hazeloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a haze factory!",
		factname);
	    return 0;
	  }
	  hazefactorystate = SCF_QUERY_INTERFACE (
	  	fact->GetMeshObjectFactory(), iHazeFactoryState);
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
		child, "Could not find material '%s'!", matname);
	    return 0;
	  }
	  hazestate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mode;
	  if (!synldr->ParseMixmode (child, mode))
	    return 0;
          hazestate->SetMixMode (mode);
	}
	break;
      case XMLTOKEN_ORIGIN:
        if (!synldr->ParseVector (child, a))
	  return 0;
        hazestate->SetOrigin (a);
	break;
      case XMLTOKEN_DIRECTIONAL:
        if (!synldr->ParseVector (child, a))
	  return 0;
        hazestate->SetDirectional (a);
	break;
      case XMLTOKEN_LAYER:
        {
	  float layerscale = 1.0;
	  iHazeHull *hull = ParseHull (xmltokens, reporter, synldr,
	  	child, hazefactorystate, layerscale);
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


csHazeSaver::csHazeSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csHazeSaver::~csHazeSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csHazeSaver::Initialize (iObjectRegistry* object_reg)
{
  csHazeSaver::object_reg = object_reg;
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csHazeSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iHazeState> haze = SCF_QUERY_INTERFACE (obj, iHazeState);
  csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);

  if (mesh && haze)
  {
    //Writedown Factory tag
    csRef<iMeshFactoryWrapper> fact = 
      SCF_QUERY_INTERFACE(mesh->GetFactory()->GetLogicalParent(), iMeshFactoryWrapper);
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
    iMaterialWrapper* mat = haze->GetMaterialWrapper();
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
    synldr->WriteVector(directNode, &direct);

    //Writedown Origin tag
    csVector3 orig = haze->GetOrigin();
    csRef<iDocumentNode> origNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    origNode->SetValue("origin");
    synldr->WriteVector(origNode, &orig);

    for (int i=0; i<haze->GetLayerCount(); i++)
    {
      csRef<iDocumentNode> layerNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      layerNode->SetValue("layer");

      float scale = haze->GetLayerScale(i);
      csRef<iDocumentNode> scaleNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      scaleNode->SetValue("scale");
      scaleNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(scale);

      iHazeHull* hull = haze->GetLayerHull(i);
      csRef<iHazeHullBox> hullbox = SCF_QUERY_INTERFACE(hull, iHazeHullBox);
      csRef<iHazeHullCone> hullcone = SCF_QUERY_INTERFACE(hull, iHazeHullCone);
      if (hullbox)
      {
        csVector3 min, max;
        hullbox->GetSettings(min, max);
        csRef<iDocumentNode> boxNode = layerNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        boxNode->SetValue("hazebox");
        csBox3 b(min, max);
        synldr->WriteBox(boxNode,&b);
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
        synldr->WriteBox(coneNode,&b);
        coneNode->SetAttributeAsFloat("p", p);
        coneNode->SetAttributeAsFloat("q", q);
        coneNode->SetAttributeAsInt("number", number);
      }
    }
 
    //Writedown Mixmode tag
    int mixmode = haze->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
  }
  return true;
}
