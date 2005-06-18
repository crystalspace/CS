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
#include "csutil/dirtyaccessarray.h"

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
  XMLTOKEN_BACK2FRONT,
  XMLTOKEN_ANIMCONTROL,
  XMLTOKEN_SUBMESH
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
  xmltokens.Register ("animcontrol", XMLTOKEN_ANIMCONTROL);

  xmltokens.Register ("mixmode", XMLTOKEN_MIXMODE);
  xmltokens.Register ("manualcolors", XMLTOKEN_MANUALCOLORS);
  xmltokens.Register ("defaultcolor", XMLTOKEN_DEFAULTCOLOR);
  xmltokens.Register ("lighting", XMLTOKEN_LIGHTING);
  xmltokens.Register ("noshadows", XMLTOKEN_NOSHADOWS);
  xmltokens.Register ("localshadows", XMLTOKEN_LOCALSHADOWS);
  return true;
}

bool csGeneralFactoryLoader::ParseRenderBuffer(iDocumentNode *node,
	iGeneralFactoryState* state)
{
  if(!node) return false;
  if(!state) return false;

  const char *name = node->GetAttributeValue("name");
  if ((name == 0) || (*name == 0))
  {
    synldr->ReportError ("crystalspace.genmeshfactoryloader.parse",
      node, "<renderbuffer>s must have names");
    return false;
  }
  csRef<iRenderBuffer> buf = synldr->ParseRenderBuffer (node);
  if (!buf.IsValid()) return false;

  size_t rbElem = buf->GetElementCount();
  if ((size_t)state->GetVertexCount() != rbElem)
  {
    synldr->ReportError ("crystalspace.genmeshfactoryloader.parse",
      node, "Render buffer vertex count(%zu) different from "
      "factory vertex count (%d)", rbElem, state->GetVertexCount());
    return false;
  }

  if (!state->AddRenderBuffer (name, buf))
  {
    synldr->ReportError ("crystalspace.genmeshfactoryloader.parse",
      node, "A <renderbuffer> of name '%s' was already provided",
      name);
    return false;
  }
  return true;
}

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
        ParseRenderBuffer(child, state);
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
	  csColor4* co = state->GetColors ();
	  if (num_col >= state->GetVertexCount ())
	  {
	    synldr->ReportError (
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    child, "Too many colors for a general mesh factory!");
	    return 0;
	  }
	  float r, g, b, alpha;
	  r = child->GetAttributeValueAsFloat ("red");
	  g = child->GetAttributeValueAsFloat ("green");
	  b = child->GetAttributeValueAsFloat ("blue");
	  csRef<iDocumentAttribute> attr_alpha = child->GetAttribute ("alpha");
	  if (attr_alpha)
	    alpha = child->GetAttributeValueAsFloat ("alpha");
	  else
	    alpha = 1.0f;
	  co[num_col].Set (r, g, b, alpha);
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
      case XMLTOKEN_ANIMCONTROL:
        {
	  const char* pluginname = child->GetAttributeValue ("plugin");
	  if (!pluginname)
	  {
	    synldr->ReportError (
		    "crystalspace.genmeshfactoryloader.parse",
		    child, "Plugin name missing for <animcontrol>!");
	    return 0;
	  }
	  csRef<iGenMeshAnimationControlType> type =
	  	CS_QUERY_PLUGIN_CLASS (plugin_mgr, pluginname,
		iGenMeshAnimationControlType);
	  if (!type)
	  {
	    type = CS_LOAD_PLUGIN (plugin_mgr, pluginname,
	    	iGenMeshAnimationControlType);
	  }
	  if (!type)
	  {
	    synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse",
		child, "Could not load animation control plugin '%s'!",
		pluginname);
	    return 0;
    	  }
	  csRef<iGenMeshAnimationControlFactory> anim_ctrl_fact = type->
	  	CreateAnimationControlFactory ();
	  const char* error = anim_ctrl_fact->Load (child);
	  if (error)
	  {
	    synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse",
		child, "Error loading animation control factory: '%s'!",
		error);
	    return 0;
	  }
	  state->SetAnimationControlFactory (anim_ctrl_fact);
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
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csGeneralFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iGeneralFactoryState> gfact = 
      SCF_QUERY_INTERFACE (obj, iGeneralFactoryState);
    csRef<iMeshObjectFactory> meshfact = 
      SCF_QUERY_INTERFACE (obj, iMeshObjectFactory);
    if (!gfact) return false;
    if (!meshfact) return false;

    //Write NumVt Tag
    csRef<iDocumentNode> numvtNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    numvtNode->SetValue("numvt");
    numvtNode->CreateNodeBefore(CS_NODE_TEXT, 0)
      ->SetValueAsInt(gfact->GetVertexCount());

    //Write NumTri Tag
    csRef<iDocumentNode> numtriNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    numtriNode->SetValue("numtri");
    numtriNode->CreateNodeBefore(CS_NODE_TEXT, 0)
      ->SetValueAsInt(gfact->GetTriangleCount());

    int i;
    //Write Vertex Tags
    for (i=0; i<gfact->GetVertexCount(); i++)
    {
      csRef<iDocumentNode> triaNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      triaNode->SetValue("v");
      csVector3 vertex = gfact->GetVertices()[i];
      csVector2 texel = gfact->GetTexels()[i];
      synldr->WriteVector(triaNode, &vertex);
      triaNode->SetAttributeAsFloat("u", texel.x);
      triaNode->SetAttributeAsFloat("v", texel.y);
    }

    //Write Color Tags
    if (!gfact->IsLighting())
    {
      for (i=0; i<gfact->GetVertexCount(); i++)
      {
        csRef<iDocumentNode> colorNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        colorNode->SetValue("color");
        csColor4 color = gfact->GetColors()[i];
        synldr->WriteColor(colorNode, &color);
      }
    }

    //Write Triangle Tags
    for (i=0; i<gfact->GetTriangleCount(); i++)
    {
      csRef<iDocumentNode> triaNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      triaNode->SetValue("t");
      csTriangle tria = gfact->GetTriangles()[i];
      triaNode->SetAttributeAsInt("v1", tria.a);
      triaNode->SetAttributeAsInt("v2", tria.b);
      triaNode->SetAttributeAsInt("v3", tria.c);
    }

    //Writedown DefaultColor tag
    csColor col = gfact->GetColor();
    if (col.red != 0 || col.green != 0 || col.blue != 0)
    {
      csRef<iDocumentNode> colorNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      colorNode->SetValue("defaultcolor");
      synldr->WriteColor(colorNode, &col);
    }

    //Writedown Material tag
    iMaterialWrapper* mat = gfact->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }    
    }    

    //Writedown Mixmode tag
    int mixmode = gfact->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);

    //Writedown Back2Front tag
    if (gfact->IsBack2Front())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)
        ->SetValue("back2front");

    //Writedown Lighting tag
    synldr->WriteBool(paramsNode, "lighting", gfact->IsLighting(), true);

    //Writedown NoShadows tag
    if (!gfact->IsShadowCasting())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)->SetValue("noshadows");

    //Writedown LocalShadows tag
    if (gfact->IsShadowReceiving())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)
        ->SetValue("localshadows");

    //Writedown ManualColor tag
    if (gfact->IsManualColors())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)
        ->SetValue("manualcolors");

    //Writedown AnimationControl tag
    iGenMeshAnimationControlFactory* aniconfact = 
      gfact->GetAnimationControlFactory();
    if (aniconfact)
    {
      csRef<iDocumentNode> aniconNode = 
        paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
      aniconNode->SetValue("animcontrol");
      const char* ret = aniconfact->Save(aniconNode);
      if (ret)
      {
        aniconNode->CreateNodeBefore(CS_NODE_COMMENT, 0)->SetValue(ret);
      }
    }

    if (gfact->IsAutoNormals())
    {
      //Write Autonormals Tag
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)->SetValue("autonormals");
    }
    else
    {
      //Write Normal Tags
      for (i=0; i<gfact->GetVertexCount(); i++)
      {
        csRef<iDocumentNode> normalNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        normalNode->SetValue("n");
        csVector3 normal = gfact->GetNormals()[i];
        synldr->WriteVector(normalNode, &normal);
      }
    }

    //TBD: Writedown box tag

    //TBD: Writedown renderbuffer tag
  }
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
  xmltokens.Register ("renderbuffer", XMLTOKEN_RENDERBUFFER);
  xmltokens.Register ("submesh", XMLTOKEN_SUBMESH);
  xmltokens.Register ("t", XMLTOKEN_T);
  return true;
}

bool csGeneralMeshLoader::ParseRenderBuffer(iDocumentNode *node,
	iGeneralMeshState* state, iGeneralFactoryState* factstate)
{
  if(!node) return false;
  if(!state) return false;

  const char *name = node->GetAttributeValue("name");
  if ((name == 0) || (*name == 0))
  {
    synldr->ReportError ("crystalspace.genmeshloader.parse",
      node, "<renderbuffer>s must have names");
    return false;
  }
  csRef<iRenderBuffer> buf = synldr->ParseRenderBuffer (node);
  if (!buf.IsValid()) return false;

  size_t rbElem = buf->GetElementCount();
  if ((size_t)factstate->GetVertexCount() != rbElem)
  {
    synldr->ReportError ("crystalspace.genmeshloader.parse",
      node, "Render buffer vertex count(%zu) different from "
      "factory vertex count (%d)", rbElem, factstate->GetVertexCount());
    return false;
  }

  if (!state->AddRenderBuffer (name, buf))
  {
    synldr->ReportError ("crystalspace.genmeshloader.parse",
      node, "A <renderbuffer> of name '%s' was already provided",
      name);
    return false;
  }
  return true;
}

bool csGeneralMeshLoader::ParseSubMesh(iDocumentNode *node,
                                       iGeneralMeshState* state, 
                                       iGeneralFactoryState* factstate,
                                       iLoaderContext* ldr_context)
{
  if(!node) return false;
  if (!state)
  {
    synldr->ReportError ("crystalspace.genmeshloader.parse",
      node, "Submesh must be specified _after_ factory tag.");
    return false;
  }

  csDirtyAccessArray<unsigned int> triangles;
  csRef<iMaterialWrapper> material;

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
        int tri = child->GetContentsValueAsInt ();
        if (tri > factstate->GetTriangleCount ())
        {
          synldr->ReportError (
            "crystalspace.genmeshloader.parse.invalidindex",
            child, "Invalid triangle index in genmesh submesh!");
          return false;
        }
        triangles.Push (tri);
        break;
      }
    case XMLTOKEN_MATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        material = ldr_context->FindMaterial (matname);
        if (!material.IsValid ())
        {
          synldr->ReportError (
            "crystalspace.genmeshloader.parse.unknownmaterial",
            node, "Couldn't find material '%s'!", matname);
          return false;
        }
        break;
      }
    default:
      synldr->ReportBadToken (child);
    }
  }

  if (!material.IsValid ())
  {
    synldr->ReportError (
      "crystalspace.genmeshloader.parse.unknownmaterial",
      node, "No material specified in genmesh submesh!");
    return false;
  }

  state->AddSubMesh (triangles.GetArray (), (int)triangles.Length (), material);

  return true;
}

csPtr<iBase> csGeneralMeshLoader::Parse (iDocumentNode* node,
	iLoaderContext* ldr_context, iBase*)
{
  csRef<iMeshObject> mesh;
  csRef<iGeneralMeshState> meshstate;
  csRef<iGeneralFactoryState> factstate;

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
	  factstate = SCF_QUERY_INTERFACE (fact->GetMeshObjectFactory(), 
	    iGeneralFactoryState);
	  if (!factstate)
	  {
      	    synldr->ReportError (
		"crystalspace.genmeshloader.parse.badfactory",
		child, "Factory '%s' doesn't appear to be a genmesh factory!",
		factname);
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
      case XMLTOKEN_RENDERBUFFER:
        ParseRenderBuffer (child, meshstate, factstate);
        break;
      case XMLTOKEN_SUBMESH:
        ParseSubMesh (child, meshstate, factstate, ldr_context);
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

bool csGeneralMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iGeneralMeshState> gmesh = 
      SCF_QUERY_INTERFACE (obj, iGeneralMeshState);
    csRef<iMeshObject> mesh = SCF_QUERY_INTERFACE (obj, iMeshObject);
    if (!gmesh) return false;
    if (!mesh) return false;

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
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        factNode->SetValue("factory");
        factNode->CreateNodeBefore(CS_NODE_TEXT, 0)->SetValue(factname);
      }    
    }

    //Writedown Lighting tag
    synldr->WriteBool(paramsNode, "lighting", gmesh->IsLighting(), true);

    //Writedown NoShadows tag
    if (!gmesh->IsShadowCasting())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)->SetValue("noshadows");

    //Writedown LocalShadows tag
    if (!gmesh->IsShadowReceiving())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)
        ->SetValue("localshadows");

    //Writedown Color tag
    csColor col = gmesh->GetColor();
    csRef<iDocumentNode> colorNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    colorNode->SetValue("color");
    synldr->WriteColor(colorNode, &col);

    //Writedown ManualColor tag
    synldr->WriteBool(paramsNode, "manualcolors",
                      gmesh->IsManualColors(), true);

    //Writedown Material tag
    iMaterialWrapper* mat = gmesh->GetMaterialWrapper();
    if (mat)
    {
      const char* matname = mat->QueryObject()->GetName();
      if (matname && *matname)
      {
        csRef<iDocumentNode> matNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        matNode->SetValue("material");
        csRef<iDocumentNode> matnameNode = 
          matNode->CreateNodeBefore(CS_NODE_TEXT, 0);
        matnameNode->SetValue(matname);
      }    
    }    

    //Writedown Mixmode tag
    int mixmode = gmesh->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);
  }
  return true;
}
