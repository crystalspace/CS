/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "gmeshldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "iutil/document.h"
#include "imesh/genmesh.h"
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
  XMLTOKEN_LIGHTING,
  XMLTOKEN_COLOR,
  XMLTOKEN_DEFAULTCOLOR,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_FACTORY,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_MANUALCOLORS,
  XMLTOKEN_NUMTRI,
  XMLTOKEN_NUMVT,
  XMLTOKEN_V,
  XMLTOKEN_T,
  XMLTOKEN_N,
  XMLTOKEN_RENDERBUFFER,
  XMLTOKEN_COLORS,
  XMLTOKEN_AUTONORMALS,
  XMLTOKEN_NOSHADOWS,
  XMLTOKEN_LOCALSHADOWS,
  XMLTOKEN_BACK2FRONT
};

SCF_IMPLEMENT_IBASE (csGeneralFactoryLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGeneralFactoryLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csGeneralFactorySaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGeneralFactorySaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csGeneralMeshLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGeneralMeshLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csGeneralMeshSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csGeneralMeshSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csGeneralFactoryLoader)
SCF_IMPLEMENT_FACTORY (csGeneralFactorySaver)
SCF_IMPLEMENT_FACTORY (csGeneralMeshLoader)
SCF_IMPLEMENT_FACTORY (csGeneralMeshSaver)


csGeneralFactoryLoader::csGeneralFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csGeneralFactoryLoader::~csGeneralFactoryLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGeneralFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csGeneralFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("box", XMLTOKEN_BOX);
  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("numtri", XMLTOKEN_NUMTRI);
  xmltokens.Register ("numvt", XMLTOKEN_NUMVT);
  xmltokens.Register ("v", XMLTOKEN_V);
  xmltokens.Register ("t", XMLTOKEN_T);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("autonormals", XMLTOKEN_AUTONORMALS);
  xmltokens.Register ("n", XMLTOKEN_N);
  xmltokens.Register ("renderbuffer", XMLTOKEN_RENDERBUFFER);
  xmltokens.Register ("back2front", XMLTOKEN_BACK2FRONT);

  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("manualcolors", XMLTOKEN_MANUALCOLORS);
  xmltokens.Register ("defaultcolor", XMLTOKEN_DEFAULTCOLOR);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("noshadows", XMLTOKEN_NOSHADOWS);
  xmltokens.Register ("localshadows", XMLTOKEN_LOCALSHADOWS);
  return true;
}

#ifdef CS_USE_NEW_RENDERER

bool csGeneralFactoryLoader::ParseRenderBuffer(iDocumentNode *node,
	iGeneralFactoryState* state)
{
  if(!node) return false;
  if(!state) return false;

  csRef<iDocumentNode> child;
  csRef<iDocumentNodeIterator> children = node->GetNodes();

  if(!children.IsValid()) return false; // empty renderbuffer..
  
  const char *comptype = node->GetAttributeValue("type");
  const char *name = node->GetAttributeValue("name");
  int compcount = node->GetAttributeValueAsInt("compcount");
  int length = state->GetVertexCount();
  
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
          char attribname[12];
          attribname[0] = 'f';
          attribname[1] = '\0';
          sprintf (&attribname[1], "%d", i);

          floatarray[vertexindex * compcount + i] = child
	  	->GetAttributeValueAsFloat(attribname);
        }
        vertexindex++;
      }
    };

    state->AddRenderBuffer(name, CS_BUFCOMP_FLOAT, compcount);
    state->SetRenderBuffer(name, floatarray);
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
          char attribname[12];
          attribname[0] = 'i';
          attribname[1] = '\0';
          sprintf(&attribname[1], "%d", i);

          intarray[vertexindex * compcount + i] = child
	  	->GetAttributeValueAsInt(attribname);
        }
        vertexindex++;
      }
    };

    state->AddRenderBuffer(name, CS_BUFCOMP_INT, compcount);
    state->SetRenderBuffer(name, intarray);
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
          char attribname[12];
          attribname[0] = 's';
          attribname[1] = '\0';
          sprintf (&attribname[1], "%d", i);

          shortarray[vertexindex * compcount + i] = child
	  	->GetAttributeValueAsInt(attribname);
        }
        vertexindex++;
      }
    };

    state->AddRenderBuffer(name, CS_BUFCOMP_SHORT, compcount);
    state->SetRenderBuffer(name, (int*)shortarray);
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
          char attribname[12];
          attribname[0] = 'b';
          attribname[1] = '\0';
          sprintf (&attribname[1], "%d", i);

          bytearray[vertexindex * compcount + i] = child
	  	->GetAttributeValueAsInt(attribname);
        }
        vertexindex++;
      }
    };

    state->AddRenderBuffer(name, CS_BUFCOMP_BYTE, compcount);
    state->SetRenderBuffer(name, (int*)bytearray);
  
    delete[] bytearray;
  }
  else
    return false;
  
  return true;
}
#endif // CS_USE_NEW_RENDERER

csPtr<iBase> csGeneralFactoryLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase* /* context */)
{
  csRef<iPluginManager> plugin_mgr (CS_QUERY_REGISTRY (object_reg,
  	iPluginManager));
  csRef<iMeshObjectType> type (CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.genmesh", iMeshObjectType));
  if (!type)
  {
    type = CS_LOAD_PLUGIN (plugin_mgr, "crystalspace.mesh.object.genmesh",
    	iMeshObjectType);
  }
  if (!type)
  {
    synldr->ReportError (
		"crystalspace.genmeshfactoryloader.setup.objecttype",
		node, "Could not load the general mesh object plugin!");
    return 0;
  }
  csRef<iMeshObjectFactory> fact;
  csRef<iGeneralFactoryState> state;

  fact = type->NewFactory ();
  state = SCF_QUERY_INTERFACE (fact, iGeneralFactoryState);

  int num_tri = 0;
  int num_nor = 0;
  int num_col = 0;
  int num_vt = 0;
  bool auto_normals = false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MANUALCOLORS:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return 0;
	  state->SetManualColors (r);
	}
	break;
      case XMLTOKEN_NOSHADOWS:
	{
	  state->SetShadowCasting (false);
	}
	break;
      case XMLTOKEN_LOCALSHADOWS:
	{
	  state->SetShadowReceiving (true);
	}
	break;
      case XMLTOKEN_LIGHTING:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return 0;
	  state->SetLighting (r);
	}
	break;
      case XMLTOKEN_DEFAULTCOLOR:
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return 0;
	  state->SetColor (col);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mm;
	  if (!synldr->ParseMixmode (child, mm))
	    return 0;
          state->SetMixMode (mm);
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return 0;
	  }
	  state->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_BOX:
        {
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  state->GenerateBox (box);
	}
        break;
      case XMLTOKEN_AUTONORMALS:
        if (!synldr->ParseBool (child, auto_normals, true))
	  return 0;
	break;
      case XMLTOKEN_NUMTRI:
        state->SetTriangleCount (child->GetContentsValueAsInt ());
	break;
      case XMLTOKEN_NUMVT:
        state->SetVertexCount (child->GetContentsValueAsInt ());
	break;
      case XMLTOKEN_BACK2FRONT:
	{
	  bool b2f = false;
          if (!synldr->ParseBool (child, b2f, true))
	    return 0;
	  state->SetBack2Front (b2f);
	}
	break;
      case XMLTOKEN_RENDERBUFFER:
#ifdef CS_USE_NEW_RENDERER
        ParseRenderBuffer(child, state);
#endif
        break;
      case XMLTOKEN_T:
	{
	  csTriangle* tr = state->GetTriangles ();
	  if (num_tri >= state->GetTriangleCount ())
	  {
	    synldr->ReportError (
		      "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		      child, "Too many triangles for a general mesh factory!");
	    return 0;
	  }
	  tr[num_tri].a = child->GetAttributeValueAsInt ("v1");
	  tr[num_tri].b = child->GetAttributeValueAsInt ("v2");
	  tr[num_tri].c = child->GetAttributeValueAsInt ("v3");
	  if (tr[num_tri].a >= state->GetVertexCount () ||
	      tr[num_tri].b >= state->GetVertexCount () ||
	      tr[num_tri].c >= state->GetVertexCount ())
	  {
	    synldr->ReportError (
		      "crystalspace.genmeshfactoryloader.parse.frame.badvt",
		      child, "Bad vertex index for triangle in genmesh factory!"
		      );
	    return 0;
	  }
	  num_tri++;
	}
        break;
      case XMLTOKEN_N:
        {
	  csVector3* no = state->GetNormals ();
	  if (num_nor >= state->GetVertexCount ())
	  {
	    synldr->ReportError (
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    child, "Too many normals for a general mesh factory!");
	    return 0;
	  }
	  float x, y, z;
	  x = child->GetAttributeValueAsFloat ("x");
	  y = child->GetAttributeValueAsFloat ("y");
	  z = child->GetAttributeValueAsFloat ("z");
	  no[num_nor].Set (x, y, z);
	  num_nor++;
        }
        break;
      case XMLTOKEN_COLOR:
        {
	  csColor* co = state->GetColors ();
	  if (num_col >= state->GetVertexCount ())
	  {
	    synldr->ReportError (
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    child, "Too many colors for a general mesh factory!");
	    return 0;
	  }
	  float r, g, b;
	  r = child->GetAttributeValueAsFloat ("red");
	  g = child->GetAttributeValueAsFloat ("green");
	  b = child->GetAttributeValueAsFloat ("blue");
	  co[num_col].Set (r, g, b);
	  num_col++;
	}
	break;
      case XMLTOKEN_V:
        {
	  csVector3* vt = state->GetVertices ();
	  csVector2* te = state->GetTexels ();
	  if (num_vt >= state->GetVertexCount ())
	  {
	    synldr->ReportError (
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    child, "Too many vertices for a general mesh factory!");
	    return 0;
	  }
	  float x, y, z, u, v;
	  x = child->GetAttributeValueAsFloat ("x");
	  y = child->GetAttributeValueAsFloat ("y");
	  z = child->GetAttributeValueAsFloat ("z");
	  u = child->GetAttributeValueAsFloat ("u");
	  v = child->GetAttributeValueAsFloat ("v");
	  vt[num_vt].Set (x, y, z);
	  te[num_vt].Set (u, v);
	  num_vt++;
	}
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  if (auto_normals)
    state->CalculateNormals ();

  return csPtr<iBase> (fact);
}
//---------------------------------------------------------------------------

csGeneralFactorySaver::csGeneralFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csGeneralFactorySaver::~csGeneralFactorySaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGeneralFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csGeneralFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */
//TBD
bool csGeneralFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  paramsNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet supported for general mesh");
  paramsNode=0;
  
  return true;
}
//---------------------------------------------------------------------------

csGeneralMeshLoader::csGeneralMeshLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csGeneralMeshLoader::~csGeneralMeshLoader ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGeneralMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csGeneralMeshLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  xmltokens.Register ("material", XMLTOKEN_MATERIAL);
  xmltokens.Register ("factory", XMLTOKEN_FACTORY);
  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("manualcolors", XMLTOKEN_MANUALCOLORS);
  xmltokens.Register ("color", XMLTOKEN_COLOR);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("noshadows", XMLTOKEN_NOSHADOWS);
  xmltokens.Register ("localshadows", XMLTOKEN_LOCALSHADOWS);
  return true;
}

csPtr<iBase> csGeneralMeshLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iGeneralMeshState> meshstate;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MANUALCOLORS:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return 0;
	  meshstate->SetManualColors (r);
	}
	break;
      case XMLTOKEN_NOSHADOWS:
	{
	  meshstate->SetShadowCasting (false);
	}
	break;
      case XMLTOKEN_LOCALSHADOWS:
	{
	  meshstate->SetShadowReceiving (true);
	}
	break;
      case XMLTOKEN_LIGHTING:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return 0;
	  meshstate->SetLighting (r);
	}
	break;
      case XMLTOKEN_COLOR:
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return 0;
	  meshstate->SetColor (col);
	}
	break;
      case XMLTOKEN_FACTORY:
	{
	  const char* factname = child->GetContentsValue ();
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (factname);
	  if (!fact)
	  {
      	    synldr->ReportError (
		"crystalspace.genmeshloader.parse.unknownfactory",
		child, "Couldn't find factory '%s'!", factname);
	    return 0;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  CS_ASSERT (mesh != 0);
          meshstate = SCF_QUERY_INTERFACE (mesh, iGeneralMeshState);
	  if (!meshstate)
	  {
      	    synldr->ReportError (
		"crystalspace.genmeshloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a genmesh factory!",
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
		"crystalspace.genmeshloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return 0;
	  }
	  meshstate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mm;
	  if (!synldr->ParseMixmode (child, mm))
	    return 0;
          meshstate->SetMixMode (mm);
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

csGeneralMeshSaver::csGeneralMeshSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csGeneralMeshSaver::~csGeneralMeshSaver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool csGeneralMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  csGeneralMeshSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}
//TBD
bool csGeneralMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");
  paramsNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue
    ("iSaverPlugin not yet supported for general mesh");
  paramsNode=0;
  
  return true;
}

