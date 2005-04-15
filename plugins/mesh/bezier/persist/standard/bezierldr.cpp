/*
    Copyright (C) 2003 by Jorrit Tyberghein

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
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csutil/util.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "csutil/csstring.h"
#include "iutil/vfs.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/plugin.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "imesh/object.h"
#include "imesh/bezier.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "imap/loader.h"
#include "ivaria/reporter.h"
#include "bezierldr.h"
#include "csqint.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_CLONE = 1,
  XMLTOKEN_COSFACT,
  XMLTOKEN_CURVE,
  XMLTOKEN_CURVECENTER,
  XMLTOKEN_CURVECONTROL,
  XMLTOKEN_CURVESCALE,
  XMLTOKEN_FACTORY,
  XMLTOKEN_FOG,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_PART,
  XMLTOKEN_RADIUS,
  XMLTOKEN_V,

  // Below is for plane loader.
  XMLTOKEN_ORIG,
  XMLTOKEN_FIRSTLEN,
  XMLTOKEN_FIRST,
  XMLTOKEN_SECONDLEN,
  XMLTOKEN_SECOND,
  XMLTOKEN_MATRIX,
  XMLTOKEN_NAME,
  XMLTOKEN_UVEC,
  XMLTOKEN_VVEC
};

SCF_IMPLEMENT_IBASE (csBezierLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBezierLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csBezierSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csBezierSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csBezierLoader)
SCF_IMPLEMENT_FACTORY (csBezierSaver)


#define MAXLINE 200 /* max number of chars per line... */

//---------------------------------------------------------------------------

csBezierLoader::csBezierLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBezierLoader::~csBezierLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csBezierLoader::Initialize (iObjectRegistry* object_reg)
{
  csBezierLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("clone", XMLTOKEN_CLONE);
  xmltokens.Register ("cosfact", XMLTOKEN_COSFACT);
  xmltokens.Register ("curve", XMLTOKEN_CURVE);
  xmltokens.Register ("curvecenter", XMLTOKEN_CURVECENTER);
  xmltokens.Register ("curvecontrol", XMLTOKEN_CURVECONTROL);
  xmltokens.Register ("curvescale", XMLTOKEN_CURVESCALE);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("fog", XMLTOKEN_FOG);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("part", XMLTOKEN_PART);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("v", XMLTOKEN_V);
  return true;
}

bool csBezierLoader::ParseCurve (iCurve* curve, iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  int num_v = 0;
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
          if (mat == 0)
          {
	    synldr->ReportError (
	      "crystalspace.bezierloader.parse.material",
              child, "Couldn't find material named '%s'!", matname);
            return false;
          }
          curve->SetMaterial (mat);
	}
        break;
      case XMLTOKEN_V:
        {
          if (num_v >= 9)
          {
	    synldr->ReportError (
	      "crystalspace.bezierloader.parse.vertices",
              child, "Wrong number of vertices to bezier! Should be 9!");
            return false;
          }
	  curve->SetVertex (num_v, child->GetContentsValueAsInt ());
	  num_v++;
        }
        break;
      default:
	synldr->ReportBadToken (child);
	return false;
    }
  }
  
  if (num_v != 9)
  {
    synldr->ReportError (
      "crystalspace.bezierloader.parse.vertices",
      node, "Wrong number of vertices to bezier! %d should be 9!", num_v);
    return false;
  }
  return true;
}

bool csBezierLoader::LoadThingPart (iDocumentNode* node,
	iLoaderContext* ldr_context,
	iObjectRegistry* object_reg, iReporter* reporter,
	iSyntaxService *synldr, BezierLoadInfo& info,
	iEngine* engine, iBezierState* thing_state,
	iBezierFactoryState* thing_fact_state,
	bool isParent)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_COSFACT:
        if (!isParent)
	{
	  synldr->ReportError (
	    "crystalspace.bezierloader.parse.fastmesh",
	    child, "'cosfact' flag only for top-level thing!");
	  return false;
	}
        else thing_fact_state->SetCosinusFactor (
		child->GetContentsValueAsFloat ());
        break;
      case XMLTOKEN_FACTORY:
        if (!isParent)
	{
	  synldr->ReportError (
	    "crystalspace.bezierloader.parse.factory",
	    child, "'factory' statement only for top-level thing!");
	  return false;
	}
	else
        {
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
          if (!fact)
          {
	    synldr->ReportError (
	      "crystalspace.bezierloader.parse.factory",
              child, "Couldn't find bezier mesh factory '%s'!", factname);
            return false;
          }
	  csRef<iBezierFactoryState> tmpl_thing_state (SCF_QUERY_INTERFACE (
	  	fact->GetMeshObjectFactory (), iBezierFactoryState));
	  if (!tmpl_thing_state)
	  {
	    synldr->ReportError (
	      "crystalspace.bezierloader.parse.factory",
              child, "Factory '%s' is not a bezier mesh factory!", factname);
            return false;
	  }
	  thing_fact_state->MergeTemplate (tmpl_thing_state,
	  	info.default_material);
        }
        break;
      case XMLTOKEN_CLONE:
        if (!isParent)
	{
	  synldr->ReportError (
	    "crystalspace.bezierloader.parse.clone",
	    child, "CLONE statement only for top-level thing!");
	  return false;
	}
	else
        {
	  const char* meshname = child->GetContentsValue ();
	  iMeshWrapper* wrap = ldr_context->FindMeshObject (meshname);
          if (!wrap)
          {
	    synldr->ReportError (
	      "crystalspace.bezierloader.parse.clone",
              child, "Couldn't find thing '%s'!", meshname);
            return false;
          }

	  csRef<iBezierFactoryState> tmpl_thing_state (SCF_QUERY_INTERFACE (
	  	wrap->GetMeshObject (), iBezierFactoryState));
	  if (!tmpl_thing_state)
	  {
	    synldr->ReportError (
	      "crystalspace.bezierloader.parse.clone",
              child, "Object '%s' is not a thing!", meshname);
            return false;
	  }
	  thing_fact_state->MergeTemplate (tmpl_thing_state,
	  	info.default_material);
        }
        break;
      case XMLTOKEN_PART:
	if (!LoadThingPart (child, ldr_context, object_reg, reporter,
		synldr, info, engine, thing_state, thing_fact_state,
		false))
	  return false;
        break;
      case XMLTOKEN_FOG:
	synldr->ReportError (
	      "crystalspace.bezierloader.parse.fog",
      	      child, "FOG for things is currently not supported!\n\
Nag to Jorrit about this feature if you want it.");
	return false;

      case XMLTOKEN_CURVE:
        {
	  iCurve* p = thing_fact_state->CreateCurve ();
	  p->QueryObject()->SetName (child->GetAttributeValue ("name"));
	  if (!ParseCurve (p, ldr_context, child))
	    return false;
        }
        break;

      case XMLTOKEN_CURVECENTER:
        {
          csVector3 c;
	  if (!synldr->ParseVector (child, c))
	    return false;
          thing_fact_state->SetCurvesCenter (c);
        }
        break;
      case XMLTOKEN_CURVESCALE:
        {
	  float f = child->GetContentsValueAsFloat ();
	  thing_fact_state->SetCurvesScale (f);
          break;
        }
      case XMLTOKEN_CURVECONTROL:
        {
          csVector3 v;
          csVector2 t;
	  v.x = child->GetAttributeValueAsFloat ("x");
	  v.y = child->GetAttributeValueAsFloat ("y");
	  v.z = child->GetAttributeValueAsFloat ("z");
	  t.x = child->GetAttributeValueAsFloat ("u");
	  t.y = child->GetAttributeValueAsFloat ("v");
          thing_fact_state->AddCurveVertex (v, t);
        }
        break;

      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          info.default_material = ldr_context->FindMaterial (matname);
          if (info.default_material == 0)
          {
	    synldr->ReportError (
	        "crystalspace.bezierloader.parse.material",
                child, "Couldn't find material named '%s'!", matname);
            return false;
          }
	}
        break;
      default:
        synldr->ReportBadToken (child);
	return false;
    }
  }
  return true;
}

csPtr<iBase> csBezierLoader::Parse (iDocumentNode* node,
			     iLoaderContext* ldr_context, iBase*)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.bezier", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.bezier",
    	iMeshObjectType);
  }
  if (!type)
  {
    synldr->ReportError (
		"crystalspace.bezierloader.setup.objecttype",
		node, "Could not load the bezier mesh object plugin!");
    return 0;
  }
  csRef<iEngine> engine = CS_QUERY_REGISTRY (object_reg, iEngine);

  csRef<iMeshObjectFactory> fact;
  csRef<iBezierState> thing_state;
  csRef<iBezierFactoryState> thing_fact_state;

  // We always do NewFactory() even for mesh objects.
  // That's because csThing implements both so a factory is a mesh object.
  fact = type->NewFactory ();
  thing_state = SCF_QUERY_INTERFACE (fact, iBezierState);
  thing_fact_state = SCF_QUERY_INTERFACE (fact, iBezierFactoryState);

  BezierLoadInfo info;
  if (!LoadThingPart (node, ldr_context, object_reg, reporter, synldr, info,
  	engine, thing_state, thing_fact_state, true))
  {
    fact = 0;
  }
  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csBezierSaver::csBezierSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBezierSaver::~csBezierSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csBezierSaver::Initialize (iObjectRegistry* object_reg)
{
  csBezierSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}
//TBD
bool csBezierSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);

  if (!mesh) return false;

  csRef<iMeshWrapper> objwrap = 
    SCF_QUERY_INTERFACE (mesh->GetLogicalParent(), iMeshWrapper);

  if (objwrap) return WriteObject(obj, paramsNode);

  csRef<iMeshFactoryWrapper> factwrap = 
    SCF_QUERY_INTERFACE (mesh->GetLogicalParent(), iMeshFactoryWrapper);

  if (factwrap) return WriteFactory(obj, paramsNode);

  return false;
}

bool csBezierSaver::WriteObject (iBase* obj, iDocumentNode* parent)
{
  csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);
  csRef<iBezierState> state = SCF_QUERY_INTERFACE (obj, iBezierState);

  parent->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet fully supported for bezier mesh");


  if (mesh && state)
  {
    //Writedown Factory tag
    csRef<iMeshFactoryWrapper> fact = 
      SCF_QUERY_INTERFACE(mesh->GetFactory()->GetLogicalParent(), 
        iMeshFactoryWrapper);
    if (fact)
    {
      const char* factname = fact->QueryObject()->GetName();
      if (factname && *factname)
      {
        csRef<iDocumentNode> factNode = 
          parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        factNode->SetValue("factory");
        csRef<iDocumentNode> factnameNode = 
          factNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        factnameNode->SetValue(factname);
      }
    }
    return true;
  }
  return false;
}

bool csBezierSaver::WriteFactory (iBase* obj, iDocumentNode* parent)
{
  csRef<iBezierFactoryState> fact = 
    SCF_QUERY_INTERFACE (obj, iBezierFactoryState);

  if (fact)
  {
    //Writedown CurveCenter tag
    csVector3 curvecenter = fact->GetCurvesCenter();
    csRef<iDocumentNode> curvecenterNode = 
      parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    curvecenterNode->SetValue("curvecenter");
    synldr->WriteVector(curvecenterNode, &curvecenter);

    //Writedown CurveScale tag
    float curvescale = fact->GetCurvesScale();
    csRef<iDocumentNode> curvescaleNode = 
      parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    curvescaleNode->SetValue("curvescale");
    curvescaleNode->CreateNodeBefore(CS_NODE_TEXT, 0)
      ->SetValueAsFloat(curvescale);

    //Writedown CurveControl tag
    for (int i=0; i< fact->GetCurveVertexCount(); i++)
    {
      csVector3 cc_v3 = fact->GetCurveVertex(i);
      csVector2 cc_v2 = fact->GetCurveTexel(i);
      csRef<iDocumentNode> ccNode = 
        parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      ccNode->SetValue("curvecontrol");
      synldr->WriteVector(ccNode, &cc_v3);
      ccNode->SetAttributeAsFloat("u", cc_v2.x);
      ccNode->SetAttributeAsFloat("v", cc_v2.y);
    }

    //Writedown Curve tag
    for (int j=0; j< fact->GetCurveCount(); j++)
    {
      iCurve* curve = fact->GetCurve(j);
      csRef<iDocumentNode> curveNode = 
        parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      curveNode->SetValue("curve");
      curveNode->SetAttribute("name", curve->QueryObject()->GetName());

      //Writedown Material tag
      iMaterialWrapper* mat = curve->GetMaterial();
      if (mat)
      {
        const char* matname = mat->QueryObject()->GetName();
        if (matname && *matname)
        {
          csRef<iDocumentNode> matNode = 
            curveNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
          matNode->SetValue("material");
          csRef<iDocumentNode> matnameNode = 
            matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
          matnameNode->SetValue(matname);
        }
        for (int i=0; i< curve->GetVertexCount(); i++)
        {
          int v = curve->GetVertex(i);
          csRef<iDocumentNode> vNode = 
            curveNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
          vNode->SetValue("v");
          vNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsInt(v);
        }
      }
    }
  }

  return true;
}
