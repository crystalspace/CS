/*
    Copyright (C) 2000 by Jorrit Tyberghein
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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "spr2dldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "imesh/sprite2d.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iobject/object.h"
#include "iengine/material.h"
#include "csengine/material.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)

  CS_TOKEN_DEF (COLORS)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (UV)
  CS_TOKEN_DEF (VERTICES)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csSprite2DFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSprite2DFactorySaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSprite2DLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSprite2DSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSprite2DFactoryLoader)
IMPLEMENT_FACTORY (csSprite2DFactorySaver)
IMPLEMENT_FACTORY (csSprite2DLoader)
IMPLEMENT_FACTORY (csSprite2DSaver)

EXPORT_CLASS_TABLE (spr2dldr)
  EXPORT_CLASS (csSprite2DFactoryLoader, "crystalspace.mesh.loader.factory.sprite.2d",
    "Crystal Space Sprite2D Mesh Factory Loader")
  EXPORT_CLASS (csSprite2DFactorySaver, "crystalspace.mesh.saver.factory.sprite.2d",
    "Crystal Space Sprite2D Mesh Factory Saver")
  EXPORT_CLASS (csSprite2DLoader, "crystalspace.mesh.loader.sprite.2d",
    "Crystal Space Sprite2D Mesh Loader")
  EXPORT_CLASS (csSprite2DSaver, "crystalspace.mesh.saver.sprite.2d",
    "Crystal Space Sprite2D Mesh Saver")
EXPORT_CLASS_TABLE_END

csSprite2DFactoryLoader::csSprite2DFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite2DFactoryLoader::~csSprite2DFactoryLoader ()
{
}

bool csSprite2DFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

static UInt ParseMixmode (char* buf)
{
  CS_TOKEN_TABLE_START (modes)
    CS_TOKEN_TABLE (COPY)
    CS_TOKEN_TABLE (MULTIPLY2)
    CS_TOKEN_TABLE (MULTIPLY)
    CS_TOKEN_TABLE (ADD)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (KEYCOLOR)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  UInt Mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return 0;
    }
    switch (cmd)
    {
      case CS_TOKEN_COPY: Mixmode |= CS_FX_COPY; break;
      case CS_TOKEN_MULTIPLY: Mixmode |= CS_FX_MULTIPLY; break;
      case CS_TOKEN_MULTIPLY2: Mixmode |= CS_FX_MULTIPLY2; break;
      case CS_TOKEN_ADD: Mixmode |= CS_FX_ADD; break;
      case CS_TOKEN_ALPHA:
	Mixmode &= ~CS_FX_MASK_ALPHA;
	float alpha;
	int ialpha;
        ScanStr (params, "%f", &alpha);
	ialpha = QInt (alpha * 255.99);
	Mixmode |= CS_FX_SETALPHA(ialpha);
	break;
      case CS_TOKEN_TRANSPARENT: Mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: Mixmode |= CS_FX_KEYCOLOR; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing the modes!\n", csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}

iBase* csSprite2DFactoryLoader::Parse (const char* string, iEngine* engine, iBase* /* context */)
{
  // @@@ Implement MIXMODE
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.sprite.2d", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.sprite.2d", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.sprite.2d\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  iSprite2DFactoryState* spr2dLook = QUERY_INTERFACE (fact, iSprite2DFactoryState);

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      fact->DecRef ();
      spr2dLook->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
	{
          ScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
            fact->DecRef ();
	    spr2dLook->DecRef ();
            return NULL;
	  }
	  spr2dLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          spr2dLook->SetLighting (do_lighting);
        }
	break;
      case CS_TOKEN_MIXMODE:
        spr2dLook->SetMixMode (ParseMixmode (params));
	break;
    }
  }

  spr2dLook->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------
csSprite2DFactorySaver::csSprite2DFactorySaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite2DFactorySaver::~csSprite2DFactorySaver ()
{
}

bool csSprite2DFactorySaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

static void WriteMixmode(iStrVector *str, UInt mixmode)
{
  str->Push(strnew("  MIXMODE ("));
  if(mixmode&CS_FX_COPY) str->Push(strnew(" COPY ()"));
  if(mixmode&CS_FX_ADD) str->Push(strnew(" ADD ()"));
  if(mixmode&CS_FX_MULTIPLY) str->Push(strnew(" MULTIPLY ()"));
  if(mixmode&CS_FX_MULTIPLY2) str->Push(strnew(" MULTIPLY2 ()"));
  if(mixmode&CS_FX_KEYCOLOR) str->Push(strnew(" KEYCOLOR ()"));
  if(mixmode&CS_FX_TRANSPARENT) str->Push(strnew(" TRANSPARENT ()"));
  if(mixmode&CS_FX_ALPHA) {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(strnew(buf));
  }
  str->Push(strnew(")"));
}

void csSprite2DFactorySaver::WriteDown (iBase* obj, iStrVector * str,
  iEngine* /*engine*/)
{
  iSprite2DFactoryState *state = QUERY_INTERFACE (obj, iSprite2DFactoryState);
  char buf[MAXLINE];

  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
    GetPrivateObject()->GetName());
  str->Push(strnew(buf));
  if(state->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, state->GetMixMode());
  }
  sprintf(buf, "LIGHTING (%s)\n", state->HasLighting()?"true":"false");
  str->Push(strnew(buf));

  state->DecRef();
}
//---------------------------------------------------------------------------

csSprite2DLoader::csSprite2DLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite2DLoader::~csSprite2DLoader ()
{
}

bool csSprite2DLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csSprite2DLoader::Parse (const char* string, iEngine* engine, iBase* /* context */)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (UV)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (COLORS)
    CS_TOKEN_TABLE (LIGHTING)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iSprite2DState* spr2dLook = NULL;
  csColoredVertices* verts = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (spr2dLook) spr2dLook->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_FACTORY:
	{
          ScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    if (spr2dLook) spr2dLook->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          spr2dLook = QUERY_INTERFACE (mesh, iSprite2DState);
	  verts = &(spr2dLook->GetVertices ());
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          ScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
            mesh->DecRef ();
	    if (spr2dLook) spr2dLook->DecRef ();
            return NULL;
	  }
	  spr2dLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        spr2dLook->SetMixMode (ParseMixmode (params));
	break;
      case CS_TOKEN_VERTICES:
        {
          float list[100];
	  int num;
          ScanStr (params, "%F", list, &num);
	  num /= 2;
	  verts->SetLength (num);
          for (int i = 0 ; i < num ; i++)
	  {
	    (*verts)[i].pos.x = list[i*2+0];
	    (*verts)[i].pos.y = list[i*2+1];
	    (*verts)[i].color_init.Set (0, 0, 0);
	    (*verts)[i].color.Set (0, 0, 0);
	  }
        }
        break;
      case CS_TOKEN_UV:
        {
          float list[100];
	  int num;
          ScanStr (params, "%F", list, &num);
	  num /= 2;
	  verts->SetLength (num);
          for (int i = 0 ; i < num ; i++)
	  {
	    (*verts)[i].u = list[i*2+0];
	    (*verts)[i].v = list[i*2+1];
	  }
        }
        break;
      case CS_TOKEN_LIGHTING:
        {
          int do_lighting;
          ScanStr (params, "%b", &do_lighting);
          spr2dLook->SetLighting (do_lighting);
        }
        break;
      case CS_TOKEN_COLORS:
        {
          float list[100];
	  int num;
          ScanStr (params, "%F", list, &num);
	  num /= 3;
	  verts->SetLength (num);
          for (int i = 0 ; i < num ; i++)
	  {
	    (*verts)[i].color_init.red = list[i*3+0];
	    (*verts)[i].color_init.green = list[i*3+1];
	    (*verts)[i].color_init.blue = list[i*3+2];
	  }
        }
        break;
    }
  }

  if (spr2dLook) spr2dLook->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------

csSprite2DSaver::csSprite2DSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite2DSaver::~csSprite2DSaver ()
{
}

bool csSprite2DSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

void csSprite2DSaver::WriteDown (iBase* obj, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  iSprite2DState *state = QUERY_INTERFACE (obj, iSprite2DState);
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(strnew(buf));

  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterialWrapper()->
    GetPrivateObject()->GetName());
  str->Push(strnew(buf));
  sprintf(buf, "LIGHTING (%s)\n", state->HasLighting()?"true":"false");
  str->Push(strnew(buf));
  if(state->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, state->GetMixMode());
  }

  csColoredVertices& vs = state->GetVertices();
  int i;
  str->Push(strnew("VERTICES("));
  for(i=0; vs.Length(); i++)
  {
    sprintf(buf, "%g,%g%s", vs[i].pos.x, vs[i].pos.y,
      (i==vs.Length()-1)?"":", ");
    str->Push(strnew(buf));
  }
  str->Push(strnew(")\n"));

  str->Push(strnew("UV("));
  for(i=0; vs.Length(); i++)
  {
    sprintf(buf, "%g,%g%s", vs[i].u, vs[i].v, (i==vs.Length()-1)?"":", ");
    str->Push(strnew(buf));
  }
  str->Push(strnew(")\n"));

  str->Push(strnew("COLORS("));
  for(i=0; vs.Length(); i++)
  {
    sprintf(buf, "%g,%g,%g%s", vs[i].color_init.red, vs[i].color_init.green,
      vs[i].color_init.blue, (i==vs.Length()-1)?"":", ");
    str->Push(strnew(buf));
  }
  str->Push(strnew(")\n"));

  fact->DecRef();
  state->DecRef();
}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
