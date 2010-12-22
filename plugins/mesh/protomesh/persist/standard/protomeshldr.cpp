/*
    Copyright (C) 2004 by Jorrit Tyberghein

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
#include "imesh/protomesh.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/stringarray.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"

#include "protomeshldr.h"



using namespace CS::Plugins::ProtoMeshLoader;

SCF_IMPLEMENT_FACTORY (csProtoFactoryLoader)
SCF_IMPLEMENT_FACTORY (csProtoFactorySaver)
SCF_IMPLEMENT_FACTORY (csProtoMeshLoader)
SCF_IMPLEMENT_FACTORY (csProtoMeshSaver)


csProtoFactoryLoader::csProtoFactoryLoader (iBase* pParent) :
  scfImplementationType (this, pParent)
{
}

csProtoFactoryLoader::~csProtoFactoryLoader ()
{
}

bool csProtoFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csProtoFactoryLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  return true;
}

csPtr<iBase> csProtoFactoryLoader::Parse (iDocumentNode* node,
                                          iStreamSource*, 
                                          iLoaderContext* /*ldr_context*/, 
                                          iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr = 
    csQueryRegistry<iPluginManager> (object_reg);
  csRef<iMeshObjectType> type = csQueryPluginClass<iMeshObjectType> (
    plugin_mgr, "crystalspace.mesh.object.protomesh");
  if (!type)
  {
    type = csLoadPlugin<iMeshObjectType> (plugin_mgr, 
      "crystalspace.mesh.object.protomesh");
  }
  if (!type)
  {
    synldr->ReportError (
		"crystalspace.protomeshfactoryloader.setup.objecttype",
		node, "Could not load the general mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact;
  csRef<iProtoFactoryState> state;

  fact = type->NewFactory ();
  state = scfQueryInterface<iProtoFactoryState> (fact);

  int idx = 0;
  int triidx = 0;
  csVector3* vertices = state->GetVertices ();
  csVector2* texels = state->GetTexels ();
  csVector3* normals = state->GetNormals ();
  csColor* colors = state->GetColors ();
  csTriangle* triangles = state->GetTriangles ();

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_T:
        {
	  if (triidx >= 12)
	  {
	    synldr->ReportError (
		"crystalspace.protomeshfactoryloader.parsetriangle",
		node, "Only twelve triangles allowed!");
	    return 0;
	  }
	  triangles[triidx].a = child->GetAttributeValueAsInt ("v1");
	  triangles[triidx].b = child->GetAttributeValueAsInt ("v2");
	  triangles[triidx].c = child->GetAttributeValueAsInt ("v3");
	  triidx++;
        }
	break;
      case XMLTOKEN_V:
        {
	  if (idx >= 8)
	  {
	    synldr->ReportError (
		"crystalspace.protomeshfactoryloader.parsevertex",
		node, "Only eight vertices allowed!");
	    return 0;
	  }
	  vertices[idx].x = child->GetAttributeValueAsFloat ("x");
	  vertices[idx].y = child->GetAttributeValueAsFloat ("y");
	  vertices[idx].z = child->GetAttributeValueAsFloat ("z");
	  texels[idx].x = child->GetAttributeValueAsFloat ("u");
	  texels[idx].y = child->GetAttributeValueAsFloat ("v");
	  normals[idx].x = child->GetAttributeValueAsFloat ("nx");
	  normals[idx].y = child->GetAttributeValueAsFloat ("ny");
	  normals[idx].z = child->GetAttributeValueAsFloat ("nz");
	  colors[idx].red = child->GetAttributeValueAsFloat ("red");
	  colors[idx].green = child->GetAttributeValueAsFloat ("green");
	  colors[idx].blue = child->GetAttributeValueAsFloat ("blue");
	  idx++;
	}
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }
  if (idx != 8)
  {
    synldr->ReportError (
		"crystalspace.protomeshfactoryloader.parsevertex",
		node, "Eight vertices are needed!");
    return 0;
  }
  if (triidx != 12)
  {
    synldr->ReportError (
		"crystalspace.protomeshfactoryloader.parsetriangle",
		node, "Twelve triangles are needed!");
    return 0;
  }

  return csPtr<iBase> (fact);
}
//---------------------------------------------------------------------------

csProtoFactorySaver::csProtoFactorySaver (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

csProtoFactorySaver::~csProtoFactorySaver ()
{
}

bool csProtoFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csProtoFactorySaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csProtoFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iProtoFactoryState> gfact = 
      scfQueryInterface<iProtoFactoryState> (obj);
    csRef<iMeshObjectFactory> meshfact = 
      scfQueryInterface<iMeshObjectFactory> (obj);
    if (!gfact) return false;
    if (!meshfact) return false;

    csVector3* vertices = gfact->GetVertices ();
    csVector2* texels = gfact->GetTexels ();
    csVector3* normals = gfact->GetNormals ();
    csColor* colors = gfact->GetColors ();
    csTriangle* triangles = gfact->GetTriangles ();

    int i;
    // Writedown vertices.
    for (i = 0 ; i < 8 ; i++)
    {
      csRef<iDocumentNode> vNode = 
        paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      vNode->SetValue ("v");
      vNode->SetAttributeAsFloat ("x", vertices[i].x);
      vNode->SetAttributeAsFloat ("y", vertices[i].y);
      vNode->SetAttributeAsFloat ("z", vertices[i].z);
      vNode->SetAttributeAsFloat ("u", texels[i].x);
      vNode->SetAttributeAsFloat ("v", texels[i].y);
      vNode->SetAttributeAsFloat ("nx", normals[i].x);
      vNode->SetAttributeAsFloat ("ny", normals[i].y);
      vNode->SetAttributeAsFloat ("nz", normals[i].z);
      vNode->SetAttributeAsFloat ("red", colors[i].red);
      vNode->SetAttributeAsFloat ("green", colors[i].green);
      vNode->SetAttributeAsFloat ("blue", colors[i].blue);
    }
    // Writedown triangles.
    for (i = 0 ; i < 12 ; i++)
    {
      csRef<iDocumentNode> tNode = 
        paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      tNode->SetValue ("t");
      tNode->SetAttributeAsInt ("v1", triangles[i].a);
      tNode->SetAttributeAsInt ("v2", triangles[i].b);
      tNode->SetAttributeAsInt ("v3", triangles[i].c);
    }
  }
  return true;
}

//---------------------------------------------------------------------------

csProtoMeshLoader::csProtoMeshLoader (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

csProtoMeshLoader::~csProtoMeshLoader ()
{
}

bool csProtoMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csProtoMeshLoader::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);

  InitTokenTable (xmltokens);
  return true;
}

#define CHECK_MESH(m) \
  if (!m) { \
    synldr->ReportError ( \
	"crystalspace.protomeshloader.parse.unknownfactory", \
	child, "Specify the factory first!"); \
    return 0; \
  }


csPtr<iBase> csProtoMeshLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iProtoMeshState> meshstate;

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
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return 0;
	  CHECK_MESH (mesh);
	  mesh->SetColor (col);
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);

    if(!fact)
    {
      synldr->ReportError (
        "crystalspace.protomeshloader.parse.unknownfactory",
        child, "Couldn't find factory %s!", CS::Quote::Single (factname));
      return 0;
    }

	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  CS_ASSERT (mesh != 0);
          meshstate = scfQueryInterface<iProtoMeshState> (mesh);
	  if (!meshstate)
	  {
      	    synldr->ReportError (
		"crystalspace.protomeshloader.parse.badfactory",
		child, "Factory %s doesn't appear to be a protomesh factory!",
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
		"crystalspace.protomeshloader.parse.unknownmaterial",
		child, "Couldn't find material %s!", CS::Quote::Single (matname));
            return 0;
	  }
	  CHECK_MESH (mesh);
	  mesh->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mm;
	  if (!synldr->ParseMixmode (child, mm))
	    return 0;
	  CHECK_MESH (mesh);
          mesh->SetMixMode (mm);
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

csProtoMeshSaver::csProtoMeshSaver (iBase* pParent) : 
  scfImplementationType (this, pParent)
{
}

csProtoMeshSaver::~csProtoMeshSaver ()
{
}

bool csProtoMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  csProtoMeshSaver::object_reg = object_reg;
  synldr = csQueryRegistry<iSyntaxService> (object_reg);
  return true;
}

bool csProtoMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iProtoMeshState> gmesh = 
      scfQueryInterface<iProtoMeshState> (obj);
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

    // Writedown Color tag
    csColor col (0, 0, 0);
    mesh->GetColor (col);
    csRef<iDocumentNode> colorNode = 
      paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    colorNode->SetValue ("color");
    synldr->WriteColor (colorNode, col);

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

    // Writedown Mixmode tag
    int mixmode = mesh->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = 
      paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue ("mixmode");
    synldr->WriteMixmode (mixmodeNode, mixmode, true);
  }
  return true;
}
