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
	  iHazeHullBox *ebox = hullcreate->CreateBox (box.Min (), box.Max ());
	  result = SCF_QUERY_INTERFACE (ebox, iHazeHull);
	  CS_ASSERT (result);
          ebox->DecRef ();
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
	  iHazeHullCone *econe = hullcreate->CreateCone (number,
	    box.Min (), box.Max (), p, q);
	  result = SCF_QUERY_INTERFACE (econe, iHazeHull);
	  CS_ASSERT (result);
          econe->DecRef ();
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
    printf ("Load TYPE plugin crystalspace.mesh.object.haze\n");
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

#if 0

#define MAXLINE 100 /* max number of chars per line... */

/// write hull to string
static void WriteHull(csString& str, iHazeHull *hull)
{
  char buf[MAXLINE];
  csVector3 a,b;
  int nr;
  float p,q;
  csRef<iHazeHullBox> ebox (SCF_QUERY_INTERFACE(hull, iHazeHullBox));
  if(ebox)
  {
    ebox->GetSettings(a, b);
    sprintf(buf, "  HAZEBOX(%g,%g,%g, %g,%g,%g)\n", a.x,a.y,a.z, b.x,b.y,b.z);
    str.Append(buf);
    return;
  }
  csRef<iHazeHullCone> econe (SCF_QUERY_INTERFACE(hull, iHazeHullCone));
  if(econe)
  {
    econe->GetSettings(nr, a, b, p, q);
    sprintf(buf, "  HAZEBOX(%d, %g,%g,%g, %g,%g,%g, %g, %g)\n", nr,
      a.x,a.y,a.z, b.x,b.y,b.z, p, q);
    str.Append(buf);
    return;
  }
  printf ("Unknown hazehull type, cannot writedown!\n");
}

#endif
//TBD
bool csHazeFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  paramsNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet supported for haze mesh");
  paramsNode=0;
  
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
  return true;
}
//TBD
bool csHazeSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  paramsNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet supported for haze mesh");
  paramsNode=0;
  
  return true;
}

