/*
    Based on genmesh.cpp, Copyright (C) 2002 by Jorrit Tyberghein
    Copyright (C) 2003 by Mat Sutcliffe

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
#include "csutil/sysfunc.h"
#include "csgeom/math3d.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "nmeshldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "imesh/nullmesh.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "csqint.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"
#include "csgeom/vector2.h"
#include "csgeom/vector4.h"

CS_IMPLEMENT_PLUGIN

enum
{
  XMLTOKEN_BOX = 1,
  XMLTOKEN_RADIUS,
  XMLTOKEN_FACTORY,
  XMLTOKEN_RENDERBUFFER
};

SCF_IMPLEMENT_IBASE (csNullFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csNullFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csNullMeshLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullMeshLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csNullMeshSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csNullMeshSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csNullFactoryLoader)
SCF_IMPLEMENT_FACTORY (csNullFactorySaver)
SCF_IMPLEMENT_FACTORY (csNullMeshLoader)
SCF_IMPLEMENT_FACTORY (csNullMeshSaver)


csNullFactoryLoader::csNullFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csNullFactoryLoader::~csNullFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csNullFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csNullFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("box", XMLTOKEN_BOX);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("renderbuffer", XMLTOKEN_RENDERBUFFER);
  return true;
}


bool csNullFactoryLoader::ParseRenderBuffer(iDocumentNode *node,
	iNullFactoryState* state)
{
  if(!node) return false;
  if(!state) return false;

  csRef<iDocumentNode> child;
  csRef<iDocumentNodeIterator> children = node->GetNodes();

  if(!children.IsValid()) return false; // empty renderbuffer..
  
  const char *comptype = node->GetAttributeValue("type");
  //@@@ Jorrit:foo:  const char *name = node->GetAttributeValue("name");
  int compcount = node->GetAttributeValueAsInt("compcount");
  //@@@ Jorrit: why is this here? int length = state->GetVertexCount();
  int length = 0;
  
  if(strcmp(comptype, "float") == 0)
  {
    float *floatarray = new float[length * compcount];
    int vertexindex = 0;
    while(children->HasNext())
    {
      child = children->Next();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      
      if(strcmp("va", child->GetValue ()) == 0)
      {
        for(int i = 0; i < compcount; i++)
        {
          csString attribname;
          attribname.Format ("f%d", i);

          floatarray[vertexindex * compcount + i] = child
	  	->GetAttributeValueAsFloat(attribname);
        }
        vertexindex++;
      }
    };

    //@@@ Jorrit: doesn't compile!!! state->AddRenderBuffer(name, CS_BUFCOMP_FLOAT, compcount);
    //@@@state->SetRenderBuffer(name, floatarray);
    delete[] floatarray;
  }
  if(strcmp(comptype, "int") == 0)
  {
    int *intarray = new int[length * compcount];
    int vertexindex = 0;
    while(children->HasNext())
    {
      child = children->Next();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      
      if(strcmp("va", child->GetValue()) == 0)
      {
        for(int i = 0; i < compcount; i++)
        {
          csString attribname;
          attribname.Format ("i%d", i);

          intarray[vertexindex * compcount + i] = child
	  	->GetAttributeValueAsInt(attribname);
        }
        vertexindex++;
      }
    };

    //@@@ Jorrit: state->AddRenderBuffer(name, CS_BUFCOMP_INT, compcount);
    //@@@ Jorrit: state->SetRenderBuffer(name, intarray);
    delete[] intarray;
  }
  else if(strcmp(comptype, "short") == 0)
  {
    compcount += (compcount % 2);
    short *shortarray = new short[length * compcount];
    int vertexindex = 0;
    while(children->HasNext())
    {
      child = children->Next();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      
      if(strcmp("va", child->GetValue()) == 0)
      {
        for(int i = 0; i < compcount; i++)
        {
          csString attribname;
          attribname.Format ("s%d", i);

          shortarray[vertexindex * compcount + i] = child
	  	->GetAttributeValueAsInt(attribname);
        }
        vertexindex++;
      }
    };

    //@@@ state->AddRenderBuffer(name, CS_BUFCOMP_SHORT, compcount);
    //@@@ state->SetRenderBuffer(name, (int*)shortarray);
    delete[] shortarray;
  }
  else if(strcmp(comptype, "byte") == 0)
  {
    compcount += (compcount % 4);
    unsigned char *bytearray = new unsigned char[length * compcount];
    int vertexindex = 0;
    while(children->HasNext())
    {
      child = children->Next();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      
      if(strcmp("va", child->GetValue()) == 0)
      {
        for(int i = 0; i < compcount; i++)
        {
          csString attribname;
          attribname.Format ("b%d", i);

          bytearray[vertexindex * compcount + i] = child
	  	->GetAttributeValueAsInt(attribname);
        }
        vertexindex++;
      }
    };

    //@@@ Jorrit: state->AddRenderBuffer(name, CS_BUFCOMP_BYTE, compcount);
    //@@@ Jorrit: state->SetRenderBuffer(name, (int*)bytearray);
  
    delete[] bytearray;
  }
  else
    return false;
  
  return true;
}

csPtr<iBase> csNullFactoryLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase* context)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.null", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.null",
    	iMeshObjectType);
  }
  if (!type)
  {
    synldr->ReportError (
		"crystalspace.nullmeshfactoryloader.setup.objecttype",
		node, "Could not load the null mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact;
  csRef<iNullFactoryState> state;

  fact = type->NewFactory ();
  state = SCF_QUERY_INTERFACE (fact, iNullFactoryState);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_BOX:
        {
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  state->SetBoundingBox (box);
	}
        break;
      case XMLTOKEN_RADIUS:
        state->SetRadius (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_RENDERBUFFER:
        ParseRenderBuffer(child, state);
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (fact);
}
//---------------------------------------------------------------------------

csNullFactorySaver::csNullFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csNullFactorySaver::~csNullFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csNullFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csNullFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csNullFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iNullFactoryState> nullfact = SCF_QUERY_INTERFACE (obj, iNullFactoryState);
  csRef<iMeshObjectFactory> meshfact = SCF_QUERY_INTERFACE (obj, iMeshObjectFactory);

  if ( nullfact && meshfact )
  {
    //Writedown Box tag
    csBox3 box;
    nullfact->GetBoundingBox(box);
    csRef<iDocumentNode> boxNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    boxNode->SetValue ("box");
    synldr->WriteBox(boxNode, &box);

    //Writedown Radius tag
    float radius = nullfact->GetRadius();
    csRef<iDocumentNode> radiusNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    radiusNode->SetValue("radius");
    radiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(radius);

    //TBD: RenderBuffer stuff
  }
  return true;
}

//---------------------------------------------------------------------------

csNullMeshLoader::csNullMeshLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csNullMeshLoader::~csNullMeshLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csNullMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csNullMeshLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("box", XMLTOKEN_BOX);
  xmltokens.Register ("radius", XMLTOKEN_RADIUS);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  return true;
}

csPtr<iBase> csNullMeshLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iNullMeshState> state;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_BOX:
        {
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  state->SetBoundingBox (box);
	}
        break;
      case XMLTOKEN_RADIUS:
        state->SetRadius (child->GetContentsValueAsFloat ());
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.nullmeshloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          state = SCF_QUERY_INTERFACE (mesh, iNullMeshState);
	  if (!state)
	  {
      	    synldr->ReportError (
		"crystalspace.nullmeshloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a null factory!",
		factname);
	    return 0;
	  }
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

csNullMeshSaver::csNullMeshSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csNullMeshSaver::~csNullMeshSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csNullMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  csNullMeshSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csNullMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  if (!obj)    return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  csRef<iNullMeshState> nullstate = SCF_QUERY_INTERFACE (obj, iNullMeshState);
  csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);

  if ( nullstate && mesh )
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

    iMeshObjectFactory* meshfact = fact->GetMeshObjectFactory();
    csRef<iNullFactoryState> nullfact = SCF_QUERY_INTERFACE(meshfact, iNullFactoryState);

    //Writedown Box tag
    csBox3 box, boxfact;
    nullstate->GetBoundingBox(box);
    nullfact->GetBoundingBox(boxfact);
    if (boxfact != box)
    {
      csRef<iDocumentNode> boxNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      boxNode->SetValue("box");
      synldr->WriteBox(boxNode, &box);
    }

    //Writedown Radius tag
    float radius = nullstate->GetRadius();
    float radiusfact = nullfact->GetRadius();
    if (radius != radiusfact)
    {
      csRef<iDocumentNode> radiusNode = paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      radiusNode->SetValue("radius");
      radiusNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValueAsFloat(radius);
    }
  }
  return true;
}

