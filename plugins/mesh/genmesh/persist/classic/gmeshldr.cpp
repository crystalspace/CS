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
#include "cssys/sysfunc.h"
#include "csgeom/math3d.h"
#include "csutil/parser.h"
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
#include "qint.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"
#include "imap/ldrctxt.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (BOX)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (MANUALCOLORS)
  CS_TOKEN_DEF (NUM)
  CS_TOKEN_DEF (C)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (T)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (COLORS)
  CS_TOKEN_DEF (NORMALS)
  CS_TOKEN_DEF (TRIANGLES)
  CS_TOKEN_DEF (AUTONORMALS)
CS_TOKEN_DEF_END

enum
{
  XMLTOKEN_BOX = 1,
  XMLTOKEN_LIGHTING,
  XMLTOKEN_COLOR,
  XMLTOKEN_MATERIAL,
  XMLTOKEN_FACTORY,
  XMLTOKEN_MIXMODE,
  XMLTOKEN_MANUALCOLORS,
  XMLTOKEN_NUMTRI,
  XMLTOKEN_NUMVT,
  XMLTOKEN_V,
  XMLTOKEN_T,
  XMLTOKEN_N,
  XMLTOKEN_COLORS,
  XMLTOKEN_AUTONORMALS
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

SCF_EXPORT_CLASS_TABLE (gmeshldr)
  SCF_EXPORT_CLASS (csGeneralFactoryLoader,
    "crystalspace.mesh.loader.factory.genmesh",
    "Crystal Space General Mesh Factory Loader")
  SCF_EXPORT_CLASS (csGeneralFactorySaver, "crystalspace.mesh.saver.factory.genmesh",
    "Crystal Space General Mesh Factory Saver")
  SCF_EXPORT_CLASS (csGeneralMeshLoader, "crystalspace.mesh.loader.genmesh",
		    "Crystal Space General Mesh Mesh Loader")
  SCF_EXPORT_CLASS (csGeneralMeshSaver, "crystalspace.mesh.saver.genmesh",
    "Crystal Space General Mesh Mesh Saver")
SCF_EXPORT_CLASS_TABLE_END

static void ReportError (iReporter* reporter, const char* id,
	const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  if (reporter)
  {
    reporter->ReportV (CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  }
  else
  {
    char buf[1024];
    vsprintf (buf, description, arg);
    csPrintf ("Error ID: %s\n", id);
    csPrintf ("Description: %s\n", buf);
  }
  va_end (arg);
}

csGeneralFactoryLoader::csGeneralFactoryLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csGeneralFactoryLoader::~csGeneralFactoryLoader ()
{
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
  return true;
}

csPtr<iBase> csGeneralFactoryLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase* /* context */)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (BOX)
    CS_TOKEN_TABLE (NUM)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (COLORS)
    CS_TOKEN_TABLE (NORMALS)
    CS_TOKEN_TABLE (AUTONORMALS)
    CS_TOKEN_TABLE (TRIANGLES)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_frame)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_tri)
    CS_TOKEN_TABLE (T)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (tok_colors)
    CS_TOKEN_TABLE (C)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params, * params2;

  csParser* parser = ldr_context->GetParser ();

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
    ReportError (reporter,
		"crystalspace.genmeshfactoryloader.setup.objecttype",
		"Could not load the general mesh object plugin!");
    return NULL;
  }
  csRef<iMeshObjectFactory> fact (type->NewFactory ());
  csRef<iGeneralFactoryState> state (SCF_QUERY_INTERFACE (fact,
  	iGeneralFactoryState));

  bool auto_normals = false;
  char* buf = (char*)string;
  while ((cmd = parser->GetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.badformat",
		"Bad format while parsing general mesh factory!");
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
	{
	  char str[255];
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
      	    ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.unknownmaterial",
		"Couldn't find material '%s'!", str);
            return NULL;
	  }
	  state->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_BOX:
        {
	  csVector3 miv, mav;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	  	&miv.x, &miv.y, &miv.z, &mav.x, &mav.y, &mav.z);
          csBox3 b (miv, mav);
	  state->GenerateBox (b);
	}
        break;
      case CS_TOKEN_AUTONORMALS:
        auto_normals = true;
	break;
      case CS_TOKEN_NUM:
	{
	  int n, t;
	  csScanStr (params, "%d,%d", &n, &t);
	  state->SetVertexCount (n);
	  state->SetTriangleCount (t);
	}
	break;
      case CS_TOKEN_TRIANGLES:
        {
	  int i = 0;
	  csTriangle* tr = state->GetTriangles ();

          while ((cmd = parser->GetObject (&params, tok_tri, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		"Bad format while parsing TRIANGLES for general mesh factory!");
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_T:
	        if (i >= state->GetTriangleCount ())
		{
	          ReportError (reporter,
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    "Too many TRIANGLES for a general mesh factory!");
	          return NULL;
		}
		int a, b, c;
                csScanStr (params2, "%d,%d,%d", &a, &b, &c);
		tr[i].a = a;
		tr[i].b = b;
		tr[i].c = c;
		i++;
                break;
            }
          }
          if (cmd == CS_PARSERR_TOKENNOTFOUND)
          {
	    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.frame.badtoken",
		"Token '%s' not found while parsing triangles!",
		parser->GetLastOffender ());
	    return NULL;
          }
	}
	break;
      case CS_TOKEN_NORMALS:
        {
	  int i = 0;
	  csVector3* no = state->GetNormals ();

          while ((cmd = parser->GetObject (&params, tok_frame, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		"Bad format while parsing NORMALS for general mesh factory!");
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_V:
	        if (i >= state->GetVertexCount ())
		{
	          ReportError (reporter,
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    "Too many COLORS for a general mesh factory!");
	          return NULL;
		}
		float x, y, z;
                csScanStr (params2, "%f,%f,%f", &x, &y, &z);
		no[i].Set (x, y, z);
		i++;
                break;
            }
          }
          if (cmd == CS_PARSERR_TOKENNOTFOUND)
          {
	    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.frame.badtoken",
		"Token '%s' not found while parsing normals!",
		parser->GetLastOffender ());
	    return NULL;
          }
	}
	break;
      case CS_TOKEN_COLORS:
        {
	  int i = 0;
	  csColor* co = state->GetColors ();

          while ((cmd = parser->GetObject (&params, tok_colors, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		"Bad format while parsing COLORS for general mesh factory!");
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_C:
	        if (i >= state->GetVertexCount ())
		{
	          ReportError (reporter,
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    "Too many COLORS for a general mesh factory!");
	          return NULL;
		}
		float r, g, b;
                csScanStr (params2, "%f,%f,%f", &r, &g, &b);
		co[i].Set (r, g, b);
		i++;
                break;
            }
          }
          if (cmd == CS_PARSERR_TOKENNOTFOUND)
          {
	    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.frame.badtoken",
		"Token '%s' not found while parsing colors!",
		parser->GetLastOffender ());
	    return NULL;
          }
	}
	break;
      case CS_TOKEN_VERTICES:
        {
	  int i = 0;
	  csVector3* vt = state->GetVertices ();
	  csVector2* te = state->GetTexels ();

          while ((cmd = parser->GetObject (&params, tok_frame, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		"Bad format while parsing VERTICES for general mesh factory!");
	      return NULL;
            }
            switch (cmd)
            {
              case CS_TOKEN_V:
	        if (i >= state->GetVertexCount ())
		{
	          ReportError (reporter,
		    "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		    "Too many VERTICES for a general mesh factory!");
	          return NULL;
		}
		float x, y, z, u, v;
                csScanStr (params2, "%f,%f,%f:%f,%f", &x, &y, &z, &u, &v);
		vt[i].Set (x, y, z);
		te[i].Set (u, v);
		i++;
                break;
            }
          }
          if (cmd == CS_PARSERR_TOKENNOTFOUND)
          {
	    ReportError (reporter,
		"crystalspace.sprite3dfactoryloader.parse.frame.badtoken",
		"Token '%s' not found while parsing vertices!",
		parser->GetLastOffender ());
	    return NULL;
          }
	}
        break;
    }
  }

  if (auto_normals)
    state->CalculateNormals ();

  if (fact) fact->IncRef ();	// Prevent smart pointer release.
  return csPtr<iBase> (fact);
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
    return NULL;
  }
  csRef<iMeshObjectFactory> fact;
  csRef<iGeneralFactoryState> state;

  fact = type->NewFactory ();
  state = SCF_QUERY_INTERFACE (fact, iGeneralFactoryState);

  type->DecRef ();

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
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetContentsValue ();
          iMaterialWrapper* mat = ldr_context->FindMaterial (matname);
	  if (!mat)
	  {
      	    synldr->ReportError (
		"crystalspace.genmeshfactoryloader.parse.unknownmaterial",
		child, "Couldn't find material '%s'!", matname);
            return NULL;
	  }
	  state->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_BOX:
        {
	  csBox3 box;
	  if (!synldr->ParseBox (child, box))
	    return NULL;
	  state->GenerateBox (box);
	}
        break;
      case XMLTOKEN_AUTONORMALS:
        if (!synldr->ParseBool (child, auto_normals, true))
	  return NULL;
	break;
      case XMLTOKEN_NUMTRI:
        state->SetTriangleCount (child->GetContentsValueAsInt ());
	break;
      case XMLTOKEN_NUMVT:
        state->SetVertexCount (child->GetContentsValueAsInt ());
	break;
      case XMLTOKEN_T:
	{
	  csTriangle* tr = state->GetTriangles ();
	  if (num_tri >= state->GetTriangleCount ())
	  {
	    synldr->ReportError (
		      "crystalspace.genmeshfactoryloader.parse.frame.badformat",
		      child, "Too many triangles for a general mesh factory!");
	    return NULL;
	  }
	  tr[num_tri].a = child->GetAttributeValueAsInt ("v1");
	  tr[num_tri].b = child->GetAttributeValueAsInt ("v2");
	  tr[num_tri].c = child->GetAttributeValueAsInt ("v3");
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
	    return NULL;
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
	    return NULL;
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
		    child, "Too many colors for a general mesh factory!");
	    return NULL;
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
	return NULL;
    }
  }

  if (auto_normals)
    state->CalculateNormals ();

  // Incref to prevent smart pointer from releasing object.
  if (fact) fact->IncRef ();
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
}

bool csGeneralFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csGeneralFactorySaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csGeneralFactorySaver::WriteDown (iBase* /*obj*/, iFile * /*file*/)
{
  // no params
  // @@@ NOT IMPLEMENTED!
}

//---------------------------------------------------------------------------

csGeneralMeshLoader::csGeneralMeshLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csGeneralMeshLoader::~csGeneralMeshLoader ()
{
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
  return true;
}

csPtr<iBase> csGeneralMeshLoader::Parse (const char* string,
	iLoaderContext* ldr_context, iBase*)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (MANUALCOLORS)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  csRef<iMeshObject> mesh;
  csRef<iGeneralMeshState> meshstate;

  csParser* parser = ldr_context->GetParser ();

  char* buf = (char*)string;
  while ((cmd = parser->GetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.genmeshloader.parse.badformat",
		"Bad format while parsing general mesh object!");
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MANUALCOLORS:
	{
	  bool r;
	  csScanStr (params, "%b", &r);
	  meshstate->SetManualColors (r);
	}
	break;
      case CS_TOKEN_LIGHTING:
	{
	  bool r;
	  csScanStr (params, "%b", &r);
	  meshstate->SetLighting (r);
	}
	break;
      case CS_TOKEN_COLOR:
	{
	  csColor col;
	  csScanStr (params, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  meshstate->SetColor (col);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = ldr_context->FindMeshFactory (str);
	  if (!fact)
	  {
      	    ReportError (reporter,
		"crystalspace.genmeshloader.parse.unknownfactory",
		"Couldn't find factory '%s'!", str);
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          meshstate = SCF_QUERY_INTERFACE (mesh, iGeneralMeshState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = ldr_context->FindMaterial (str);
	  if (!mat)
	  {
      	    ReportError (reporter,
		"crystalspace.genmeshloader.parse.unknownmaterial",
		"Couldn't find material '%s'!", str);
            return NULL;
	  }
	  meshstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	uint mm;
	if (!synldr->ParseMixmode (parser, params, mm))
	{
	  ReportError (reporter, "crystalspace.genmeshloader.parse.mixmode",
	  	"Error parsing mixmode!");
	  return NULL;
	}
        meshstate->SetMixMode (mm);
	break;
    }
  }

  if (mesh) mesh->IncRef ();	// Prevent smart pointer release.
  return csPtr<iBase> (mesh);
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
	    return NULL;
	  meshstate->SetManualColors (r);
	}
	break;
      case XMLTOKEN_LIGHTING:
	{
	  bool r;
	  if (!synldr->ParseBool (child, r, true))
	    return NULL;
	  meshstate->SetLighting (r);
	}
	break;
      case XMLTOKEN_COLOR:
	{
	  csColor col;
	  if (!synldr->ParseColor (child, col))
	    return NULL;
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
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          meshstate = SCF_QUERY_INTERFACE (mesh, iGeneralMeshState);
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
            return NULL;
	  }
	  meshstate->SetMaterialWrapper (mat);
	}
	break;
      case XMLTOKEN_MIXMODE:
        {
	  uint mm;
	  if (!synldr->ParseMixmode (child, mm))
	    return NULL;
          meshstate->SetMixMode (mm);
	}
	break;
      default:
        synldr->ReportBadToken (child);
	return NULL;
    }
  }

  // Incref to avoid smart pointer from releasing.
  if (mesh) mesh->IncRef ();
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
}

bool csGeneralMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  csGeneralMeshSaver::object_reg = object_reg;
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  return true;
}

void csGeneralMeshSaver::WriteDown (iBase*, iFile*)
{
  // @@@ Not implemented!
}

