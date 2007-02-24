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

#include <ctype.h>

#include "csgeom/math3d.h"
#include "csgeom/tri.h"
#include "csgeom/vector2.h"
#include "csgeom/vector4.h"
#include "csgeom/sphere.h"
#include "csutil/cscolor.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"
#include "csutil/scanstr.h"
#include "csutil/sysfunc.h"
#include "csutil/stringreader.h"

#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "imap/ldrctxt.h"
#include "imap/services.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "iutil/comp.h"
#include "iutil/document.h"
#include "iutil/eventh.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"

#include "gmeshldr.h"

CS_IMPLEMENT_PLUGIN

CS_PLUGIN_NAMESPACE_BEGIN(GenMeshLoader)
{

SCF_IMPLEMENT_FACTORY (csGeneralFactoryLoader)
SCF_IMPLEMENT_FACTORY (csGeneralFactorySaver)
SCF_IMPLEMENT_FACTORY (csGeneralMeshLoader)
SCF_IMPLEMENT_FACTORY (csGeneralMeshSaver)

csGeneralFactoryLoader::csGeneralFactoryLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csGeneralFactoryLoader::~csGeneralFactoryLoader ()
{
}

bool csGeneralFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csGeneralFactoryLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  InitTokenTable (xmltokens);
  return true;
}

bool csGeneralFactoryLoader::ParseSubMesh(iDocumentNode *node,
                                          iGeneralFactoryState* factstate,
                                          iLoaderContext* ldr_context)
{
  if(!node) return false;

  csRef<iMaterialWrapper> material;
  bool do_mixmode = false;
  uint mixmode = CS_FX_COPY;
  csRef<iRenderBuffer> indexbuffer;
  csRefArray<csShaderVariable> shadervars;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
    case XMLTOKEN_MIXMODE:
      if (!synldr->ParseMixmode (child, mixmode))
        return 0;
      do_mixmode = true;
      break;
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
    case XMLTOKEN_INDEXBUFFER:
      {
        indexbuffer = synldr->ParseRenderBuffer (child);
        if (!indexbuffer.IsValid()) return false;
        if (!indexbuffer->IsIndexBuffer())
        {
          synldr->ReportError (
            "crystalspace.genmeshloader.parse.buffertype",
            child, "Buffer is not an index buffer");
          return false;
        }
        break;
      }
    case XMLTOKEN_SHADERVAR:
      {
        csRef<csShaderVariable> sv;
        sv.AttachNew (new csShaderVariable);
        if (!synldr->ParseShaderVar (child, *sv)) return false;
        shadervars.Push (sv);
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

  iGeneralMeshSubMesh* submesh = factstate->AddSubMesh (indexbuffer, material, 
    node->GetAttributeValue ("name"), do_mixmode ? mixmode : (uint)~0);
  csRef<iShaderVariableContext> svc = 
    scfQueryInterface<iShaderVariableContext> (submesh);
  for (size_t i = 0; i < shadervars.GetSize(); i++)
  {
    svc->AddVariable (shadervars[i]);
  }

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

  bool checkElementCount = true;
  {
    const char* check = node->GetAttributeValue("checkelementcount");
    if (check && *check)
    {
      checkElementCount = ((strcmp (check, "no") != 0)
			  && (strcmp (check, "false") != 0)
			  && (strcmp (check, "off") != 0));
    }
  }

  size_t rbElem = buf->GetElementCount();
  if (checkElementCount && ((size_t)state->GetVertexCount() != rbElem))
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

static float GetDef (iDocumentNode* node, const char* attrname, float def)
{
  csRef<iDocumentAttribute> attr = node->GetAttribute (attrname);
  if (attr)
    return attr->GetValueAsFloat ();
  else
    return def;
}

static bool GetFloat (char*& p, float& f)
{
  while (*p && isspace (*p)) p++;
  if (!*p) return false;
  char* start = p;
  while (*p && !isspace (*p)) p++;
  char old = *p;
  *p = 0;
  f = atof (start);
  *p = old;
  return true;
}

static bool GetInt (char*& p, int& f)
{
  while (*p && isspace (*p)) p++;
  if (!*p) return false;
  f = 0;
  while (*p && !isspace (*p))
  {
    f = f * 10 + (*p - '0');
    p++;
  }
  return true;
}

static bool GetVector3 (char*& p, csVector3& v)
{
  if (!GetFloat (p, v.x)) return false;
  if (!GetFloat (p, v.y)) return false;
  if (!GetFloat (p, v.z)) return false;
  return true;
}

static bool GetVector2 (char*& p, csVector2& v)
{
  if (!GetFloat (p, v.x)) return false;
  if (!GetFloat (p, v.y)) return false;
  return true;
}

static bool GetColor (char*& p, csColor4& v)
{
  if (!GetFloat (p, v.red)) return false;
  if (!GetFloat (p, v.green)) return false;
  if (!GetFloat (p, v.blue)) return false;
  if (!GetFloat (p, v.alpha)) return false;
  return true;
}

static bool GetTri (char*& p, csTriangle& v)
{
  if (!GetInt (p, v.a)) return false;
  if (!GetInt (p, v.b)) return false;
  if (!GetInt (p, v.c)) return false;
  return true;
}

csPtr<iBase> csGeneralFactoryLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase* /* context */)
{
  csRef<iMeshObjectType> type = csLoadPluginCheck<iMeshObjectType> (
  	object_reg, "crystalspace.mesh.object.genmesh", false);
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

  bool num_tri_given = false;
  bool num_vt_given = false;
  int num_tri = 0;
  int num_nor = 0;
  int num_col = 0;
  int num_vt = 0;
  bool auto_normals = false;
  bool auto_normals_nocompress = false;
  bool compress = false;

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
          fact->SetMixMode (mm);
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
	  fact->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_BOX:
        {
	  num_vt_given = true;
	  num_tri_given = true;
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return 0;
	  state->GenerateBox (box);
	  num_vt = state->GetVertexCount ();
	  num_tri = state->GetTriangleCount ();
	}
        break;
      case XMLTOKEN_SPHERE:
        {
	  num_vt_given = true;
	  num_tri_given = true;
	  csVector3 center (0, 0, 0);
	  int rim_vertices = 8;
	  csEllipsoid ellips;
	  csRef<iDocumentAttribute> attr;
	  csRef<iDocumentNode> c = child->GetNode ("center");
	  if (c)
	    if (!synldr->ParseVector (c, ellips.GetCenter ()))
	      return 0;
	  c = child->GetNode ("radius");
	  if (c)
	  {
	    if (!synldr->ParseVector (c, ellips.GetRadius ()))
	      return 0;
	  }
	  else
	  {
	    attr = child->GetAttribute ("radius");
	    float radius;
	    if (attr) radius = attr->GetValueAsFloat ();
	    else radius = 1.0f;
	    ellips.SetRadius (csVector3 (radius, radius, radius));
	  }
	  attr = child->GetAttribute ("rimvertices");
	  if (attr) rim_vertices = attr->GetValueAsInt ();
	  bool cylmapping, toponly, reversed;
	  if (!synldr->ParseBoolAttribute (child, "cylindrical", cylmapping,
	  	false, false))
  	    return 0;
	  if (!synldr->ParseBoolAttribute (child, "toponly", toponly,
	  	false, false))
  	    return 0;
	  if (!synldr->ParseBoolAttribute (child, "reversed", reversed,
	  	false, false))
  	    return 0;
	  state->GenerateSphere (ellips, rim_vertices,
	      cylmapping, toponly, reversed);
	  num_vt = state->GetVertexCount ();
	  num_tri = state->GetTriangleCount ();
	}
        break;
      case XMLTOKEN_AUTONORMALS:
        if (!synldr->ParseBool (child, auto_normals, true))
	  return 0;
	break;
      case XMLTOKEN_NORMALNOCOMPRESS:
        if (!synldr->ParseBool (child, auto_normals_nocompress, true))
	  return 0;
	break;
      case XMLTOKEN_COMPRESS:
        if (!synldr->ParseBool (child, compress, true))
	  return 0;
	break;
      case XMLTOKEN_NUMTRI:
	num_tri_given = true;
        state->SetTriangleCount (child->GetContentsValueAsInt ());
	break;
      case XMLTOKEN_NUMVT:
	num_vt_given = true;
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
      case XMLTOKEN_TRIANGLES:
	if (num_tri_given)
	{
	  synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		child, "Don't use 'numtri' in combination with 'triangles'!");
	  return 0;
	}
	else
	{
	  char* tris = (char*)child->GetContentsValue ();
	  csTriangle tri;
	  while (true)
	  {
	    if (!GetTri (tris, tri)) break;
	    if (tri.a >= state->GetVertexCount () ||
	        tri.b >= state->GetVertexCount () ||
	        tri.c >= state->GetVertexCount ())
	    {
	      synldr->ReportError (
		      "crystalspace.genmeshfactoryloader.parse.frame.badvt",
		      child, "Bad vertex index for triangle in genmesh factory!"
		      );
	      return 0;
	    }
	    state->AddTriangle (tri);
	  }
	}
	break;
      case XMLTOKEN_T:
	{
	  if (num_tri_given)
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
	  else
	  {
	    csTriangle tri;
	    tri.a = child->GetAttributeValueAsInt ("v1");
	    tri.b = child->GetAttributeValueAsInt ("v2");
	    tri.c = child->GetAttributeValueAsInt ("v3");
	    if (tri.a >= state->GetVertexCount () ||
	        tri.b >= state->GetVertexCount () ||
	        tri.c >= state->GetVertexCount ())
	    {
	      synldr->ReportError (
		      "crystalspace.genmeshfactoryloader.parse.frame.badvt",
		      child, "Bad vertex index for triangle in genmesh factory!"
		      );
	      return 0;
	    }
	    state->AddTriangle (tri);
	  }
	}
        break;
      case XMLTOKEN_N:
        {
	  if (!num_vt_given)
	  {
	    synldr->ReportError (
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    child, "You didn't specify 'numvt'. You should add normal information to the vertex!");
	    return 0;
	  }
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
	  if (!num_vt_given)
	  {
	    synldr->ReportError (
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    child, "You didn't specify 'numvt'. You should add color information to the vertex!");
	    return 0;
	  }
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
      case XMLTOKEN_VERTICES:
	if (num_vt_given)
	{
	  synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		child, "Don't use 'numvt' in combination with 'vertices'!");
	  return 0;
	}
	else
	{
	  const char* format = child->GetAttributeValue ("format");
	  // @@@ Handle format.
	  if (*format != 'v')
	  {
	    synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		child,
		"Format specifier must start with 'v'. Vertices are not optional!");
	    return 0;
	  }
	  format++;
	  bool douv = false;
	  if (*format == 'u') { douv = true; format++; }
	  bool donormals = false;
	  if (*format == 'n') { donormals = true; format++; }
	  bool docolors = false;
	  if (*format == 'c') { docolors = true; format++; }
	  if (*format != 0)
	  {
	    synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		child,
		"Wrong format specifier. Must be a string similar to 'vunc'!");
	    return 0;
	  }

	  char* vertices = (char*)child->GetContentsValue ();
	  csVector3 n (0);
	  csVector2 uv (0);
	  csColor4 col (0, 0, 0, 1);
	  while (true)
	  {
	    csVector3 v;
	    if (!GetVector3 (vertices, v)) break;
	    if (douv)
	      if (!GetVector2 (vertices, uv)) break;
	    if (donormals)
	      if (!GetVector3 (vertices, n)) break;
	    if (docolors)
	      if (!GetColor (vertices, col)) break;
	    state->AddVertex (v, uv, n, col);
	  }
	}
	break;
      case XMLTOKEN_V:
        {
	  if (num_vt_given)
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
	  else
	  {
	    csVector3 v, n;
	    csVector2 uv;
	    csColor4 col;
	    v.x = child->GetAttributeValueAsFloat ("x");
	    v.y = child->GetAttributeValueAsFloat ("y");
	    v.z = child->GetAttributeValueAsFloat ("z");
	    uv.x = child->GetAttributeValueAsFloat ("u");
	    uv.y = child->GetAttributeValueAsFloat ("v");
	    n.x = GetDef (child, "nx", 0.0f);
	    n.y = GetDef (child, "ny", 0.0f);
	    n.z = GetDef (child, "nz", 0.0f);
	    col.red = GetDef (child, "red", 0.0f);
	    col.green = GetDef (child, "green", 0.0f);
	    col.blue = GetDef (child, "blue", 0.0f);
	    col.alpha = GetDef (child, "alpha", 1.0f);
	    state->AddVertex (v, uv, n, col);
	  }
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
	  	csLoadPluginCheck<iGenMeshAnimationControlType> (
		object_reg, pluginname, false);
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
      case XMLTOKEN_SUBMESH:
        if (!state)
        {
          synldr->ReportError ("crystalspace.genmeshloader.parse",
            node, "Submesh must be specified _after_ factory tag.");
          return false;
        }
        ParseSubMesh (child, state, ldr_context);
        break;
      default:
	synldr->ReportBadToken (child);
	return 0;
    }
  }

  if (num_vt_given)
    if (num_vt != state->GetVertexCount ())
    {
      synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse", node,
		"Number of vertices (%d) doesn't match real number (%d)!",
		num_vt, state->GetVertexCount ());
      return 0;
    }
  if (num_tri_given)
    if (num_tri != state->GetTriangleCount ())
    {
      synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse", node,
		"Number of triangles (%d) doesn't match real number (%d)!",
		num_tri, state->GetTriangleCount ());
      return 0;
    }

  if (compress)
    state->Compress ();
  if (auto_normals)
    state->CalculateNormals (!auto_normals_nocompress);

  return csPtr<iBase> (fact);
}

//---------------------------------------------------------------------------

csGeneralFactorySaver::csGeneralFactorySaver (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csGeneralFactorySaver::~csGeneralFactorySaver ()
{
}

bool csGeneralFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csGeneralFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

static void WriteSubMesh (iSyntaxService* synldr, iGeneralMeshSubMesh* submesh, 
                          iDocumentNode* submeshNode)
{
  const char* submeshName = submesh->GetName ();
  if (submeshName != 0)
    submeshNode->SetAttribute ("name", submeshName);

  csRef<iDocumentNode> materialNode = 
    submeshNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  materialNode->SetValue("material");
  csRef<iDocumentNode> materialNameNode = 
    materialNode->CreateNodeBefore(CS_NODE_TEXT, 0);
  materialNameNode->SetValue (
    submesh->GetMaterial()->QueryObject()->GetName());

  uint mixmode = submesh->GetMixmode ();
  if (mixmode != (uint)~0)
  {
    csRef<iDocumentNode> mixmodeNode = 
      submeshNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue ("mixmode");
    synldr->WriteMixmode (mixmodeNode, mixmode, true);
  }

  csRef<iDocumentNode> indexBufferNode = 
    submeshNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
  indexBufferNode->SetValue ("indexbuffer");
  synldr->WriteRenderBuffer (indexBufferNode, 
    submesh->GetIndices());

  csRef<iShaderVariableContext> svc = 
    scfQueryInterface<iShaderVariableContext> (submesh);
  const csRefArray<csShaderVariable>& shadervars = svc->GetShaderVariables ();
  for (size_t i = 0; i < shadervars.GetSize(); i++)
  {
    csRef<iDocumentNode> shadervarNode = 
      submeshNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
    shadervarNode->SetValue ("shadervar");
    synldr->WriteShaderVar (shadervarNode, *(shadervars[i]));
  }
}

bool csGeneralFactorySaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
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
      synldr->WriteVector(triaNode, vertex);
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
        synldr->WriteColor(colorNode, color);
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
      synldr->WriteColor(colorNode, col);
    }

    //Writedown Material tag
    iMaterialWrapper* mat = meshfact->GetMaterialWrapper();
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
    int mixmode = meshfact->GetMixMode();
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
        synldr->WriteVector(normalNode, normal);
      }
    }

    //TBD: Writedown box tag

    // Write submeshes
    if (gfact->GetSubMeshCount() > 0)
    {
      size_t smc = gfact->GetSubMeshCount();
      for (size_t s = 0; s < smc; s++)
      {
        csRef<iDocumentNode> submeshNode = 
          paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
        submeshNode->SetValue("submesh");

        iGeneralMeshSubMesh* submesh = gfact->GetSubMesh (s);
        WriteSubMesh (synldr, submesh, submeshNode);
      }
    }

    // Write render buffers
    int rbufCount = gfact->GetRenderBufferCount ();
    for (i = 0; i < rbufCount; ++i)
    {
      csRef<iDocumentNode> rbufNode = 
        paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      rbufNode->SetValue ("renderbuffer");
      csRef<iString> name = gfact->GetRenderBufferName (i);
      rbufNode->SetAttribute ("name", name->GetData ());
      csRef<iRenderBuffer> buffer = gfact->GetRenderBuffer (i);
      synldr->WriteRenderBuffer (rbufNode, buffer);
    }
  }
  return true;
}

//---------------------------------------------------------------------------

csGeneralMeshLoader::csGeneralMeshLoader (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csGeneralMeshLoader::~csGeneralMeshLoader ()
{
}

bool csGeneralMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csGeneralMeshLoader::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  InitTokenTable (xmltokens);
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
    
  bool checkElementCount = true;
  {
    const char* check = node->GetAttributeValue("checkelementcount");
    if (check && *check)
    {
      checkElementCount = ((strcmp (check, "no") != 0)
			  && (strcmp (check, "false") != 0)
			  && (strcmp (check, "off") != 0));
    }
  }

  size_t rbElem = buf->GetElementCount();
  if (checkElementCount && ((size_t)factstate->GetVertexCount() != rbElem))
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

#include "csutil/win32/msvc_deprecated_warn_off.h"

bool csGeneralMeshLoader::ParseLegacySubMesh(iDocumentNode *node,
                                             iGeneralMeshState* state, 
                                             iLoaderContext* ldr_context)
{
  if(!node) return false;
  if (!state)
  {
    synldr->ReportError ("crystalspace.genmeshloader.parse",
      node, "Submesh must be specified _after_ factory tag.");
    return false;
  }

  csRef<iMeshObject> mo = scfQueryInterface<iMeshObject> (state);
  csRef<iGeneralFactoryState> factstate =
    scfQueryInterface<iGeneralFactoryState> (mo->GetFactory ());

  csDirtyAccessArray<unsigned int> triangles;
  csRef<iMaterialWrapper> material;
  bool do_mixmode = false;
  uint mixmode = CS_FX_COPY;

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
    case XMLTOKEN_MIXMODE:
      if (!synldr->ParseMixmode (child, mixmode))
        return 0;
      do_mixmode = true;
      break;
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

  if (do_mixmode)
    state->AddSubMesh (triangles.GetArray (), (int)triangles.Length (),
  	  material, mixmode);
  else
    state->AddSubMesh (triangles.GetArray (), (int)triangles.Length (),
  	  material);

  return true;
}

#include "csutil/win32/msvc_deprecated_warn_on.h"

bool csGeneralMeshLoader::ParseSubMesh(iDocumentNode *node,
                                       iGeneralMeshState* state,
                                       iLoaderContext* ldr_context)
{
  if(!node) return false;

  csRef<iDocumentNode> Tnode = node->GetNode ("t");
  if (Tnode.IsValid()) 
    return ParseLegacySubMesh(node, state, ldr_context);

  const char* name = node->GetAttributeValue ("name");
  csRef<iGeneralMeshSubMesh> subMesh = state->FindSubMesh (name);
  if (!subMesh)
  {
    synldr->ReportError (
      "crystalspace.genmeshloader.parse.invalidsubmeshname",
      node, "No submesh of name '%s'", name);
    return false;
  }

  csRef<iShaderVariableContext> svc = 
    scfQueryInterface<iShaderVariableContext> (subMesh);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
#if 0
    case XMLTOKEN_MIXMODE:
      {
        uint mixmode;
        if (!synldr->ParseMixmode (child, mixmode))
          return false;
        subMesh->SetMixMode (mixmode);
      }
      break;
#endif
    case XMLTOKEN_MATERIAL:
      {
        const char* matname = child->GetContentsValue ();
        csRef<iMaterialWrapper> material = 
          ldr_context->FindMaterial (matname);
        if (!material.IsValid ())
        {
          synldr->ReportError (
            "crystalspace.genmeshloader.parse.unknownmaterial",
            node, "Couldn't find material '%s'!", matname);
          return false;
        }
        subMesh->SetMaterial (material);
        break;
      }
    case XMLTOKEN_SHADERVAR:
      {
        csRef<csShaderVariable> sv;
        sv.AttachNew (new csShaderVariable);
        if (!synldr->ParseShaderVar (child, *sv)) return false;
        svc->AddVariable (sv);
        break;
      }
    default:
      synldr->ReportBadToken (child);
    }
  }

  return true;
}

#define CHECK_MESH(m) \
  if (!m) { \
    synldr->ReportError ( \
	"crystalspace.genmeshloader.parse.unknownfactory", \
	child, "Specify the factory first!"); \
    return 0; \
  }

csPtr<iBase> csGeneralMeshLoader::Parse (iDocumentNode* node,
	iStreamSource*, iLoaderContext* ldr_context, iBase*)
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
	  CHECK_MESH(meshstate);
	  meshstate->SetManualColors (r);
	}
	break;
      case XMLTOKEN_NOSHADOWS:
	{
	  CHECK_MESH(meshstate);
	  meshstate->SetShadowCasting (false);
	}
	break;
      case XMLTOKEN_LOCALSHADOWS:
	{
	  CHECK_MESH(meshstate);
	  meshstate->SetShadowReceiving (true);
	}
	break;
      case XMLTOKEN_LIGHTING:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return 0;
	  CHECK_MESH(meshstate);
	  meshstate->SetLighting (r);
	}
	break;
      case XMLTOKEN_COLOR:
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return 0;
	  CHECK_MESH(meshstate);
	  mesh->SetColor (col);
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
	  CHECK_MESH(meshstate);
	  mesh->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mm;
	  if (!synldr->ParseMixmode (child, mm))
	    return 0;
	  CHECK_MESH(meshstate);
          mesh->SetMixMode (mm);
	}
	break;
      case XMLTOKEN_RENDERBUFFER:
	CHECK_MESH(meshstate);
        ParseRenderBuffer (child, meshstate, factstate);
        break;
      case XMLTOKEN_SUBMESH:
	CHECK_MESH(meshstate);
        ParseSubMesh (child, meshstate, ldr_context);
        break;
      default:
        synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (mesh);
}

//---------------------------------------------------------------------------

csGeneralMeshSaver::csGeneralMeshSaver (iBase* pParent) :
  scfImplementationType(this, pParent)
{
}

csGeneralMeshSaver::~csGeneralMeshSaver ()
{
}

bool csGeneralMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  csGeneralMeshSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

bool csGeneralMeshSaver::WriteDown (iBase* obj, iDocumentNode* parent,
	iStreamSource*)
{
  if (!parent) return false; //you never know...
  
  csRef<iDocumentNode> paramsNode = 
    parent->CreateNodeBefore(CS_NODE_ELEMENT, 0);
  paramsNode->SetValue("params");

  if (obj)
  {
    csRef<iGeneralMeshState> gmesh = 
      scfQueryInterface<iGeneralMeshState> (obj);
    if (!gmesh) return false;
    csRef<iMeshObject> mesh = 
      scfQueryInterface<iMeshObject> (obj);
    if (!mesh) return false;
    csRef<iGeneralFactoryState> gfact = 
      scfQueryInterface<iGeneralFactoryState> (mesh->GetFactory());

    //Writedown Factory tag
    iMeshFactoryWrapper* fact = mesh->GetFactory()->GetMeshFactoryWrapper ();
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
    if (gmesh->IsShadowReceiving())
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0)
        ->SetValue("localshadows");

    //Writedown Color tag
    csColor col;
    mesh->GetColor(col);
    csRef<iDocumentNode> colorNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    colorNode->SetValue("color");
    synldr->WriteColor(colorNode, col);

    //Writedown ManualColor tag
    synldr->WriteBool(paramsNode, "manualcolors",
                      gmesh->IsManualColors(), true);

    //Writedown Material tag
    iMaterialWrapper* mat = mesh->GetMaterialWrapper();
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
    int mixmode = mesh->GetMixMode();
    csRef<iDocumentNode> mixmodeNode = 
      paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
    mixmodeNode->SetValue("mixmode");
    synldr->WriteMixmode(mixmodeNode, mixmode, true);

    // Write submeshes
    if (gfact->GetSubMeshCount() > 0)
    {
      size_t smc = gfact->GetSubMeshCount();
      for (size_t s = 0; s < smc; s++)
      {
        iGeneralMeshSubMesh* factSubMesh = gfact->GetSubMesh (s);
        iGeneralMeshSubMesh* objSubMesh = 
          gmesh->FindSubMesh (factSubMesh->GetName());
        if (objSubMesh)
        {
          csRef<iShaderVariableContext> svc = 
            scfQueryInterface<iShaderVariableContext> (objSubMesh);
          const csRefArray<csShaderVariable>& shadervars = 
            svc->GetShaderVariables ();

          iMaterialWrapper* smMaterial = objSubMesh->GetMaterial();
          /* @@@ FIXME: shadervars.IsEmpty() only works for same reasons as 
           * below */
          bool interesting = !shadervars.IsEmpty() 
            || (smMaterial != factSubMesh->GetMaterial());

          if (interesting)
          {
            csRef<iDocumentNode> submeshNode = 
              paramsNode->CreateNodeBefore(CS_NODE_ELEMENT, 0);
            submeshNode->SetValue("submesh");
            submeshNode->SetAttribute ("name", objSubMesh->GetName ());

            if (smMaterial != 0)
            {
              csRef<iDocumentNode> materialNode = 
                submeshNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
              materialNode->SetValue ("material");
              csRef<iDocumentNode> materialContents = 
                materialNode->CreateNodeBefore (CS_NODE_TEXT, 0);
              materialContents->SetValue (
                smMaterial->QueryObject()->GetName());
            }

            /* @@@ FIXME: This loop only works b/c GetShaderVariables() does 
             * not return parent's SVs. Once it does, this code needs to 
             * change, since only really the different SVs should be written 
             * out. */
            for (size_t i = 0; i < shadervars.GetSize(); i++)
            {
              csRef<iDocumentNode> shadervarNode = 
                submeshNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
              shadervarNode->SetValue ("shadervar");
              synldr->WriteShaderVar (shadervarNode, *(shadervars[i]));
            }
          }
        }
      }
    }

    // Write render buffers
    int rbufCount = gmesh->GetRenderBufferCount ();
    for (int i = 0; i < rbufCount; ++i)
    {
      csRef<iDocumentNode> rbufNode = 
        paramsNode->CreateNodeBefore (CS_NODE_ELEMENT, 0);
      rbufNode->SetValue ("renderbuffer");
      csRef<iString> name = gmesh->GetRenderBufferName (i);
      rbufNode->SetAttribute ("name", name->GetData ());
      csRef<iRenderBuffer> buffer = gmesh->GetRenderBuffer (i);
      synldr->WriteRenderBuffer (rbufNode, buffer);
    }
  }
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(GenMeshLoader)
