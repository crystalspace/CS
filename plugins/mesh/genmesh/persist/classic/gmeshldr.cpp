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
#include "imesh/genmesh.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "iengine/material.h"
#include "ivaria/reporter.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
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
  reporter = NULL;
  plugin_mgr = NULL;
}

csGeneralFactoryLoader::~csGeneralFactoryLoader ()
{
  if (reporter) reporter->DecRef ();
  if (plugin_mgr) plugin_mgr->DecRef ();
}

bool csGeneralFactoryLoader::Initialize (iObjectRegistry* object_reg)
{
  csGeneralFactoryLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

iBase* csGeneralFactoryLoader::Parse (const char* string,
	iMaterialList*, iMeshFactoryList*, iBase* /* context */)
{
  CS_TOKEN_TABLE_START (commands)
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

  iMeshObjectType* type = CS_QUERY_PLUGIN_CLASS (plugin_mgr,
  	"crystalspace.mesh.object.genmesh", iMeshObjectType);
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
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();

  iGeneralFactoryState* state = SCF_QUERY_INTERFACE (fact,
  	iGeneralFactoryState);

  bool auto_normals = false;
  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.badformat",
		"Bad format while parsing general mesh factory!");
      state->DecRef ();
      fact->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
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

          while ((cmd = csGetObject (&params, tok_tri, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		"Bad format while parsing TRIANGLES for general mesh factory!");
	      state->DecRef ();
	      fact->DecRef ();
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
	          state->DecRef ();
	          fact->DecRef ();
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
		csGetLastOffender ());
	    state->DecRef ();
	    fact->DecRef ();
	    return NULL;
          }
	}
	break;
      case CS_TOKEN_NORMALS:
        {
	  int i = 0;
	  csVector3* no = state->GetNormals ();

          while ((cmd = csGetObject (&params, tok_frame, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		"Bad format while parsing NORMALS for general mesh factory!");
	      state->DecRef ();
	      fact->DecRef ();
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
	          state->DecRef ();
	          fact->DecRef ();
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
		csGetLastOffender ());
	    state->DecRef ();
	    fact->DecRef ();
	    return NULL;
          }
	}
	break;
      case CS_TOKEN_COLORS:
        {
	  int i = 0;
	  csColor* co = state->GetColors ();

          while ((cmd = csGetObject (&params, tok_colors, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		"Bad format while parsing COLORS for general mesh factory!");
	      state->DecRef ();
	      fact->DecRef ();
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
	          state->DecRef ();
	          fact->DecRef ();
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
		csGetLastOffender ());
	    state->DecRef ();
	    fact->DecRef ();
	    return NULL;
          }
	}
	break;
      case CS_TOKEN_VERTICES:
        {
	  int i = 0;
	  csVector3* vt = state->GetVertices ();
	  csVector2* te = state->GetTexels ();

          while ((cmd = csGetObject (&params, tok_frame, &name, &params2)) > 0)
          {
            if (!params2)
            {
	      ReportError (reporter,
		"crystalspace.genmeshfactoryloader.parse.frame.badformat",
		"Bad format while parsing VERTICES for general mesh factory!");
	      state->DecRef ();
	      fact->DecRef ();
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
	          state->DecRef ();
	          fact->DecRef ();
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
		csGetLastOffender ());
	    state->DecRef ();
	    fact->DecRef ();
	    return NULL;
          }
	}
        break;
    }
  }

  if (auto_normals)
    state->CalculateNormals ();

  state->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csGeneralFactorySaver::csGeneralFactorySaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  plugin_mgr = NULL;
}

csGeneralFactorySaver::~csGeneralFactorySaver ()
{
  if (reporter) reporter->DecRef ();
  if (plugin_mgr) plugin_mgr->DecRef ();
}

bool csGeneralFactorySaver::Initialize (iObjectRegistry* object_reg)
{
  csGeneralFactorySaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csGeneralFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/)
{
  // no params
  // @@@ NOT IMPLEMENTED!
}

//---------------------------------------------------------------------------

csGeneralMeshLoader::csGeneralMeshLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  synldr = NULL;
  plugin_mgr = NULL;
}

csGeneralMeshLoader::~csGeneralMeshLoader ()
{
  SCF_DEC_REF (reporter);
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (plugin_mgr);
}

bool csGeneralMeshLoader::Initialize (iObjectRegistry* object_reg)
{
  csGeneralMeshLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!synldr)
    {
      ReportError (reporter,
	"crystalspace.genmeshloader.parse.initialize",
	"Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (synldr, "iSyntaxService"))
    {
      ReportError (reporter,
	"crystalspace.genmeshloader.parse.initialize",
	"Could not register the syntax services!");
      return false;
    }
  }
  return true;
}

iBase* csGeneralMeshLoader::Parse (const char* string, iMaterialList* matlist,
	iMeshFactoryList* factlist,
	iBase*)
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

  iMeshObject* mesh = NULL;
  iGeneralMeshState* meshstate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (reporter,
		"crystalspace.genmeshloader.parse.badformat",
		"Bad format while parsing general mesh object!");
      if (meshstate) meshstate->DecRef ();
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
	  iMeshFactoryWrapper* fact = factlist->FindByName (str);
	  if (!fact)
	  {
      	    ReportError (reporter,
		"crystalspace.genmeshloader.parse.unknownfactory",
		"Couldn't find factory '%s'!", str);
	    if (meshstate) meshstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          meshstate = SCF_QUERY_INTERFACE (mesh, iGeneralMeshState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          csScanStr (params, "%s", str);
          iMaterialWrapper* mat = matlist->FindByName (str);
	  if (!mat)
	  {
      	    ReportError (reporter,
		"crystalspace.genmeshloader.parse.unknownmaterial",
		"Couldn't find material '%s'!", str);
            mesh->DecRef ();
            return NULL;
	  }
	  meshstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
	uint mm;
	if (!synldr->ParseMixmode (params, mm))
	{
	  ReportError (reporter, "crystalspace.genmeshloader.parse.mixmode",
	  	"Error parsing mixmode!");
	  if (meshstate) meshstate->DecRef ();
	  mesh->DecRef ();
	  return NULL;
	}
        meshstate->SetMixMode (mm);
	break;
    }
  }

  if (meshstate) meshstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

csGeneralMeshSaver::csGeneralMeshSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  reporter = NULL;
  synldr = NULL;
  plugin_mgr = NULL;
}

csGeneralMeshSaver::~csGeneralMeshSaver ()
{
  SCF_DEC_REF (reporter);
  SCF_DEC_REF (synldr);
  SCF_DEC_REF (plugin_mgr);
}

bool csGeneralMeshSaver::Initialize (iObjectRegistry* object_reg)
{
  csGeneralMeshSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  reporter = CS_QUERY_REGISTRY (object_reg, iReporter);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", iSyntaxService);
    if (!synldr)
    {
      ReportError (reporter,
	"crystalspace.genmeshsaver.parse.initialize",
	"Could not load the syntax services!");
      return false;
    }
    if (!object_reg->Register (synldr, "iSyntaxService"))
    {
      ReportError (reporter,
	"crystalspace.genmeshsaver.parse.initialize",
	"Could not register the syntax services!");
      return false;
    }
  }
  return true;
}

void csGeneralMeshSaver::WriteDown (iBase*, iStrVector*)
{
  // @@@ Not implemented!
}
