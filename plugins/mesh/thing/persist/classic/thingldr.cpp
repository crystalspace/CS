/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "thingldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "iutil/plugin.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/polygon.h"
#include "imesh/thing/portal.h"
#include "imesh/thing/polytmap.h"
#include "imesh/thing/ptextype.h"
#include "imesh/thing/curve.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "ivideo/material.h"
#include "iengine/material.h"
#include "iutil/objreg.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "imap/services.h"

CS_IMPLEMENT_PLUGIN

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (CIRCLE)
  CS_TOKEN_DEF (CLONE)
  CS_TOKEN_DEF (CURVE)
  CS_TOKEN_DEF (CURVECENTER)
  CS_TOKEN_DEF (CURVECONTROL)
  CS_TOKEN_DEF (CURVESCALE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (FASTMESH)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (FIRST_LEN)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (LEN)
  CS_TOKEN_DEF (MAT_SET_SELECT)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MOVEABLE)
  CS_TOKEN_DEF (NAME)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (PART)
  CS_TOKEN_DEF (POLYGON)
  CS_TOKEN_DEF (P)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (SECOND)
  CS_TOKEN_DEF (SECOND_LEN)
  CS_TOKEN_DEF (TEMPLATE)
  CS_TOKEN_DEF (TEXLEN)
  CS_TOKEN_DEF (UVEC)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (VVEC)
  CS_TOKEN_DEF (VISTREE)
  CS_TOKEN_DEF (VBLOCK)
  CS_TOKEN_DEF (VROOM)
  CS_TOKEN_DEF (W)
  CS_TOKEN_DEF (V)
CS_TOKEN_DEF_END

SCF_IMPLEMENT_IBASE (csThingLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csThingSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csThingSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csPlaneLoader)
  SCF_IMPLEMENTS_INTERFACE (iLoaderPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPlaneLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE (csPlaneSaver)
  SCF_IMPLEMENTS_INTERFACE (iSaverPlugin)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csPlaneSaver::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

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

SCF_IMPLEMENT_FACTORY (csThingLoader)
SCF_IMPLEMENT_FACTORY (csThingSaver)
SCF_IMPLEMENT_FACTORY (csPlaneLoader)
SCF_IMPLEMENT_FACTORY (csPlaneSaver)
SCF_IMPLEMENT_FACTORY (csBezierLoader)
SCF_IMPLEMENT_FACTORY (csBezierSaver)

SCF_EXPORT_CLASS_TABLE (thingldr)
  SCF_EXPORT_CLASS (csThingLoader, "crystalspace.mesh.loader.factory.thing",
    "Crystal Space Thing Mesh Factory Loader")
  SCF_EXPORT_CLASS (csThingSaver, "crystalspace.mesh.saver.factory.thing",
    "Crystal Space Thing Mesh Factory Saver")
  SCF_EXPORT_CLASS (csThingLoader, "crystalspace.mesh.loader.thing",
    "Crystal Space Thing Mesh Loader")
  SCF_EXPORT_CLASS (csThingSaver, "crystalspace.mesh.saver.thing",
    "Crystal Space Thing Mesh Saver")
  SCF_EXPORT_CLASS (csPlaneLoader, "crystalspace.mesh.loader.thing.plane",
    "Crystal Space Thing Plane Loader")
  SCF_EXPORT_CLASS (csPlaneSaver, "crystalspace.mesh.saver.thing.plane",
    "Crystal Space Thing Plane Saver")
  SCF_EXPORT_CLASS (csBezierLoader, "crystalspace.mesh.loader.thing.bezier",
    "Crystal Space Thing Bezier Loader")
  SCF_EXPORT_CLASS (csBezierSaver, "crystalspace.mesh.saver.thing.bezier",
    "Crystal Space Thing Bezier Saver")
SCF_EXPORT_CLASS_TABLE_END

#define MAXLINE 200 /* max number of chars per line... */

//---------------------------------------------------------------------------

csThingLoader::csThingLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  synldr = NULL;
}

csThingLoader::~csThingLoader ()
{
  SCF_DEC_REF (synldr);
}

bool csThingLoader::Initialize (iObjectRegistry* object_reg)
{
  csThingLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", 
	CS_FUNCID_SYNTAXSERVICE, iSyntaxService);
    object_reg->Register (synldr);
  }
  else
    synldr->IncRef ();
  return synldr != NULL;
}

class ThingLoadInfo
{
public:
  iMaterialWrapper* default_material;
  float default_texlen;
  bool use_mat_set;
  char* mat_set_name;
  
  ThingLoadInfo () : default_material (NULL),
    default_texlen (1),
    use_mat_set (false), mat_set_name (NULL)
    {}

  void SetTextureSet (const char* name)
  {
    delete [] mat_set_name;
    mat_set_name = new char [strlen (name) + 1];
    strcpy (mat_set_name, name);
  }   
};

static bool load_thing_part (iSyntaxService *synldr, ThingLoadInfo& info, iMeshWrapper* imeshwrap,
	iEngine* engine,
	iThingState* thing_state, char* buf, int vt_offset, bool isParent)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (VBLOCK)
    CS_TOKEN_TABLE (VROOM)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (CIRCLE)
    CS_TOKEN_TABLE (POLYGON)
    CS_TOKEN_TABLE (CURVECENTER)
    CS_TOKEN_TABLE (CURVESCALE)
    CS_TOKEN_TABLE (CURVECONTROL)
    CS_TOKEN_TABLE (CURVE)
    CS_TOKEN_TABLE (MAT_SET_SELECT)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (TEXLEN)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (MOVEABLE)
    CS_TOKEN_TABLE (TEMPLATE)
    CS_TOKEN_TABLE (CLONE)
    CS_TOKEN_TABLE (VISTREE)
    CS_TOKEN_TABLE (FASTMESH)
    CS_TOKEN_TABLE (PART)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (P)
  CS_TOKEN_TABLE_END

  char* name = NULL;
  char* xname;
  long cmd;
  char* params;
  char str[255];

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_VISTREE:
        if (!isParent)
	{
	  printf ("VISTREE flag only for top-level thing!\n");
	  return false;
	}
        else thing_state->GetFlags ().Set (CS_THING_VISTREE);
        break;
      case CS_TOKEN_FASTMESH:
        if (!isParent)
	{
	  printf ("FASTMESH flag only for top-level thing!\n");
	  return false;
	}
        else thing_state->GetFlags ().Set (CS_THING_FASTMESH);
        break;
      case CS_TOKEN_MOVEABLE:
        if (!isParent)
	{
	  printf ("MOVEABLE flag only for top-level thing!\n");
	  return false;
	}
        else thing_state->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
        break;
      case CS_TOKEN_TEMPLATE:
      case CS_TOKEN_FACTORY:
        if (!isParent)
	{
	  printf ("FACTORY or TEMPLATE statement only for top-level thing!\n");
	  return false;
	}
	else
        {
          csScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->GetMeshFactories ()
	  	->FindByName (str);
          if (!fact)
          {
            printf ("Couldn't find thing factory '%s'!\n", str);
            return false;
          }
	  iThingState* tmpl_thing_state = SCF_QUERY_INTERFACE (
	  	fact->GetMeshObjectFactory (), iThingState);
	  if (cmd == CS_TOKEN_FACTORY && imeshwrap)
	    imeshwrap->SetFactory (fact);
	  if (!tmpl_thing_state)
	  {
            printf ("Object '%s' is not a thing!\n", str);
            return false;
	  }
	  thing_state->MergeTemplate (tmpl_thing_state, info.default_material);
	  tmpl_thing_state->DecRef ();
	  if (info.use_mat_set)
          {
	    thing_state->ReplaceMaterials (engine->GetMaterialList (),
	      info.mat_set_name);
	    info.use_mat_set = false;
	  }
        }
        break;
      case CS_TOKEN_CLONE:
        if (!isParent)
	{
	  printf ("CLONE statement only for top-level thing!\n");
	  return false;
	}
	else
        {
          csScanStr (params, "%s", str);
	  iMeshWrapper* wrap = engine->GetMeshes ()->FindByName (str);
          if (!wrap)
          {
            printf ("Couldn't find thing '%s'!\n", str);
            return false;
          }

	  iThingState* tmpl_thing_state = SCF_QUERY_INTERFACE (
	  	wrap->GetMeshObject (), iThingState);
	  if (!tmpl_thing_state)
	  {
            printf ("Object '%s' is not a thing!\n", str);
            return false;
	  }
	  thing_state->MergeTemplate (tmpl_thing_state, info.default_material);
	  tmpl_thing_state->DecRef ();
	  if (info.use_mat_set)
          {
	    thing_state->ReplaceMaterials (engine->GetMaterialList (),
	      info.mat_set_name);
	    info.use_mat_set = false;
	  }
        }
        break;
      case CS_TOKEN_PART:
	if (!load_thing_part (synldr, info, imeshwrap, engine,
		thing_state, params, thing_state->GetVertexCount (), false))
	  return false;
        break;
      case CS_TOKEN_V:
      case CS_TOKEN_VERTEX:
        {
	  csVector3 v;
          csScanStr (params, "%f,%f,%f", &v.x, &v.y, &v.z);
          thing_state->CreateVertex (v);
        }
        break;
      case CS_TOKEN_CIRCLE:
        {
          float x, y, z, rx, ry, rz;
          int num, dir;
          csScanStr (params, "%f,%f,%f:%f,%f,%f,%d",
	  	&x, &y, &z, &rx, &ry, &rz, &num);
          if (num < 0) { num = -num; dir = -1; }
          else dir = 1;
		  {
		  int i;
          for (i = 0 ; i < num ; i++)
          {
            float rad;
            if (dir == 1) rad = TWO_PI * (num-i-1)/(float)num;
            else rad = TWO_PI * i / (float)num;

            float cx = 0, cy = 0, cz = 0;
            float cc = cos (rad);
            float ss = sin (rad);
            if      (ABS (rx) < SMALL_EPSILON)
	    { cx = x; cy = y+cc*ry; cz = z+ss*rz; }
            else if (ABS (ry) < SMALL_EPSILON)
	    { cy = y; cx = x+cc*rx; cz = z+ss*rz; }
            else if (ABS (rz) < SMALL_EPSILON)
	    { cz = z; cx = x+cc*rx; cy = y+ss*ry; }
            thing_state->CreateVertex (csVector3 (cx, cy, cz));
          }
		  }
        }
        break;
      case CS_TOKEN_FOG:
      	printf ("FOG for things is currently not supported!\n\
Nag to Jorrit about this feature if you want it.\n");
#if 0
//@@@
        if (!isParent)
	{
	  printf ("FOG statement only for top-level thing!\n");
	  return false;
	}
	else
        {
          csFog& f = thing->GetFog ();
          f.enabled = true;
          csScanStr (params, "%f,%f,%f,%f",
	  	&f.red, &f.green, &f.blue, &f.density);
        }
#endif
        break;

      case CS_TOKEN_VROOM:
        {
	  float minx, miny, minz, maxx, maxy, maxz;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	    &minx, &miny, &minz,
	    &maxx, &maxy, &maxz);
          thing_state->CreateVertex (csVector3 (maxx, maxy, maxz));
          thing_state->CreateVertex (csVector3 (minx, maxy, maxz));
          thing_state->CreateVertex (csVector3 (maxx, maxy, minz));
          thing_state->CreateVertex (csVector3 (minx, maxy, minz));
          thing_state->CreateVertex (csVector3 (maxx, miny, maxz));
          thing_state->CreateVertex (csVector3 (minx, miny, maxz));
          thing_state->CreateVertex (csVector3 (maxx, miny, minz));
          thing_state->CreateVertex (csVector3 (minx, miny, minz));
	}
	break;

      case CS_TOKEN_VBLOCK:
        {
	  float minx, miny, minz, maxx, maxy, maxz;
	  csScanStr (params, "%f,%f,%f,%f,%f,%f",
	    &minx, &miny, &minz,
	    &maxx, &maxy, &maxz);
          thing_state->CreateVertex (csVector3 (minx, miny, minz));
          thing_state->CreateVertex (csVector3 (maxx, miny, minz));
          thing_state->CreateVertex (csVector3 (minx, miny, maxz));
          thing_state->CreateVertex (csVector3 (maxx, miny, maxz));
          thing_state->CreateVertex (csVector3 (minx, maxy, minz));
          thing_state->CreateVertex (csVector3 (maxx, maxy, minz));
          thing_state->CreateVertex (csVector3 (minx, maxy, maxz));
          thing_state->CreateVertex (csVector3 (maxx, maxy, maxz));
	}
	break;

      case CS_TOKEN_P:
      case CS_TOKEN_POLYGON:
        {
	  iPolygon3D* poly3d = thing_state->CreatePolygon (xname);
	  if (info.default_material)
	    poly3d->SetMaterial (info.default_material);
	  if (!synldr->ParsePoly3d (engine, poly3d, params, 
				   info.default_texlen, thing_state, vt_offset))
	  {
	    poly3d->DecRef ();
	    return false;
	  }
        }
        break;

      case CS_TOKEN_CURVE:
        {
	  char cname[100];
	  csScanStr (params, "%s", cname);
	  iThingEnvironment* te = SCF_QUERY_INTERFACE (engine->GetThingType (),
	    iThingEnvironment);
	  iCurveTemplate* ct = te->FindCurveTemplate (cname);
	  te->DecRef ();
	  iCurve* p = thing_state->CreateCurve (ct);
	  p->QueryObject()->SetName (name);
          if (!ct->GetMaterial ())
	    p->SetMaterial (info.default_material);
        }
        break;

      case CS_TOKEN_CURVECENTER:
        {
          csVector3 c;
          csScanStr (params, "%f,%f,%f", &c.x, &c.y, &c.z);
          thing_state->SetCurvesCenter (c);
        }
        break;
      case CS_TOKEN_CURVESCALE:
        {
	  float f;
          csScanStr (params, "%f", &f);
	  thing_state->SetCurvesScale (f);
          break;
        }
      case CS_TOKEN_CURVECONTROL:
        {
          csVector3 v;
          csVector2 t;
          csScanStr (params, "%f,%f,%f:%f,%f", &v.x, &v.y, &v.z,&t.x,&t.y);
          thing_state->AddCurveVertex (v, t);
        }
        break;

      case CS_TOKEN_MATERIAL:
        csScanStr (params, "%s", str);
        info.default_material = engine->GetMaterialList ()->
		FindByName (str
		/*@@@ REGIONS?, onlyRegion*/);
        if (info.default_material == NULL)
        {
          printf ("Couldn't find material named '%s'!\n", str);
          return false;
        }
        break;
      case CS_TOKEN_TEXLEN:
        csScanStr (params, "%f", &info.default_texlen);
        break;
      case CS_TOKEN_MAT_SET_SELECT:
        csScanStr (params, "%s", str);
        info.SetTextureSet (str);
        info.use_mat_set = true;
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing a thing!\n",
    	csGetLastOffender ());
    return false;
  }
  return true;
}

iBase* csThingLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  iMeshObjectFactory* fact = NULL;
  iThingState* thing_state = NULL;

  iMeshWrapper* imeshwrap = SCF_QUERY_INTERFACE (context, iMeshWrapper);
  if (imeshwrap) imeshwrap->DecRef ();
  iMeshFactoryWrapper* ifactmeshwrap = SCF_QUERY_INTERFACE (context,
  	iMeshFactoryWrapper);
  if (ifactmeshwrap) ifactmeshwrap->DecRef ();

  iMeshObjectType* type = engine->GetThingType (); // @@@ CS_LOAD_PLUGIN LATER!
  // We always do NewFactory() even for mesh objects.
  // That's because csThing implements both so a factory is a mesh object.
  fact = type->NewFactory ();
  thing_state = SCF_QUERY_INTERFACE (fact, iThingState);

  char* buf = (char*)string;
  ThingLoadInfo info;
  if (!load_thing_part (synldr, info, imeshwrap, engine, thing_state,
  	buf, 0, true))
  {
    fact->DecRef ();
    fact = NULL;
  }
  thing_state->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csThingSaver::csThingSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csThingSaver::~csThingSaver ()
{
}

bool csThingSaver::Initialize (iObjectRegistry* object_reg)
{
  csThingSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csThingSaver::WriteDown (iBase* /*obj*/, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = SCF_QUERY_INTERFACE (this, iFactory);
  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace (name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf (buf, "FACTORY ('%s')\n", name);
  str->Push (csStrNew (buf));
  fact->DecRef ();
}

//---------------------------------------------------------------------------

csPlaneLoader::csPlaneLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  synldr = NULL;
}

csPlaneLoader::~csPlaneLoader ()
{
  SCF_DEC_REF (synldr);
}

bool csPlaneLoader::Initialize (iObjectRegistry* object_reg)
{
  csPlaneLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  synldr = CS_QUERY_REGISTRY (object_reg, iSyntaxService);
  if (!synldr)
  {
    synldr = CS_LOAD_PLUGIN (plugin_mgr,
    	"crystalspace.syntax.loader.service.text", 
	CS_FUNCID_SYNTAXSERVICE, iSyntaxService);
    object_reg->Register (synldr);
  }
  else
    synldr->IncRef ();
  return synldr != NULL;
}

iBase* csPlaneLoader::Parse (const char* string, iEngine* engine,
	iBase* /*context*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (ORIG)
    CS_TOKEN_TABLE (FIRST_LEN)
    CS_TOKEN_TABLE (FIRST)
    CS_TOKEN_TABLE (SECOND_LEN)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (NAME)
    CS_TOKEN_TABLE (UVEC)
    CS_TOKEN_TABLE (VVEC)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* xname;
  long cmd;
  char* params;
  iThingEnvironment* te = SCF_QUERY_INTERFACE (engine->GetThingType (),
  	iThingEnvironment);
  iPolyTxtPlane* ppl = te->CreatePolyTxtPlane ();
  te->DecRef ();

  bool tx1_given = false, tx2_given = false;
  csVector3 tx_orig (0, 0, 0), tx1 (0, 0, 0), tx2 (0, 0, 0);
  float tx1_len = 0, tx2_len = 0;
  csMatrix3 tx_matrix;
  csVector3 tx_vector (0, 0, 0);
  char name[255]; name[0] = 0;
  char* buf = (char*)string;

  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_NAME:
        csScanStr (params, "%s", name);
	ppl->QueryObject()->SetName (name);
        break;
      case CS_TOKEN_ORIG:
        tx1_given = true;
        synldr->ParseVector (params, tx_orig);
        break;
      case CS_TOKEN_FIRST:
        tx1_given = true;
        synldr->ParseVector (params, tx1);
        break;
      case CS_TOKEN_FIRST_LEN:
        csScanStr (params, "%f", &tx1_len);
        tx1_given = true;
        break;
      case CS_TOKEN_SECOND:
        tx2_given = true;
        synldr->ParseVector (params, tx2);
        break;
      case CS_TOKEN_SECOND_LEN:
        csScanStr (params, "%f", &tx2_len);
        tx2_given = true;
        break;
      case CS_TOKEN_MATRIX:
        synldr->ParseMatrix (params, tx_matrix);
        break;
      case CS_TOKEN_V:
        synldr->ParseVector (params, tx_vector);
        break;
      case CS_TOKEN_UVEC:
        tx1_given = true;
        synldr->ParseVector (params, tx1);
        tx1_len = tx1.Norm ();
        tx1 += tx_orig;
        break;
      case CS_TOKEN_VVEC:
        tx2_given = true;
        synldr->ParseVector (params, tx2);
        tx2_len = tx2.Norm ();
        tx2 += tx_orig;
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing a plane!\n",
    	csGetLastOffender ());
    return NULL;
  }

  if (tx1_given)
    if (tx2_given)
    {
      if (!tx1_len)
      {
        printf ("Bad texture specification for PLANE '%s'\n", name);
	tx1_len = 1;
      }
      if (!tx2_len)
      {
        printf ("Bad texture specification for PLANE '%s'\n", name);
	tx2_len = 1;
      }
      if ((tx1-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else if ((tx2-tx_orig) < SMALL_EPSILON)
        printf ("Bad texture specification for PLANE '%s'\n", name);
      else ppl->SetTextureSpace (tx_orig, tx1, tx1_len, tx2, tx2_len);
    }
    else
    {
      printf ("Not supported!\n");
      return NULL;
    }
  else
    ppl->SetTextureSpace (tx_matrix, tx_vector);

  return ppl;
}

//---------------------------------------------------------------------------

csPlaneSaver::csPlaneSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csPlaneSaver::~csPlaneSaver ()
{
}

bool csPlaneSaver::Initialize (iObjectRegistry* object_reg)
{
  csPlaneSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csPlaneSaver::WriteDown (iBase* /*obj*/, iStrVector* /*str*/,
  iEngine* /*engine*/)
{
}

//---------------------------------------------------------------------------

csBezierLoader::csBezierLoader (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBezierLoader::~csBezierLoader ()
{
}

bool csBezierLoader::Initialize (iObjectRegistry* object_reg)
{
  csBezierLoader::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

iBase* csBezierLoader::Parse (const char* string, iEngine* engine,
	iBase* /*context*/)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (NAME)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char *xname;
  long cmd;
  int i;
  char *params;
  char name[100];

  iThingEnvironment* te = SCF_QUERY_INTERFACE (engine->GetThingType (),
	    iThingEnvironment);
  iCurveTemplate* tmpl = te->CreateBezierTemplate ();
  te->DecRef ();

  iMaterialWrapper* mat = NULL;
  char str[255];

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &xname, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_NAME:
        csScanStr (params, "%s", name);
	tmpl->QueryObject()->SetName (name);
        break;
      case CS_TOKEN_MATERIAL:
        csScanStr (params, "%s", str);
	//@@@ REGION SUPPORT? (below)
        mat = engine->GetMaterialList ()->
		FindByName (str/*@@@, onlyRegion*/);
        if (mat == NULL)
        {
          printf ("Couldn't find material named '%s'!\n", str);
          return NULL;
        }
        tmpl->SetMaterial (mat);
        break;
      case CS_TOKEN_V:
      case CS_TOKEN_VERTICES:
        {
          int list[100], num;
          csScanStr (params, "%D", list, &num);
          if (num != 9)
          {
            printf ("Wrong number of vertices to bezier!\n");
            return NULL;
          }
          for (i = 0 ; i < num ; i++) tmpl->SetVertex (i, list[i]);
        }
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing a bezier template!\n",
    	csGetLastOffender ());
    return NULL;
  }
  return tmpl;
}

//---------------------------------------------------------------------------

csBezierSaver::csBezierSaver (iBase* pParent)
{
  SCF_CONSTRUCT_IBASE (pParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
}

csBezierSaver::~csBezierSaver ()
{
}

bool csBezierSaver::Initialize (iObjectRegistry* object_reg)
{
  csBezierSaver::object_reg = object_reg;
  plugin_mgr = CS_QUERY_REGISTRY (object_reg, iPluginManager);
  return true;
}

void csBezierSaver::WriteDown (iBase* /*obj*/, iStrVector* /*str*/,
  iEngine* /*engine*/)
{
}
