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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "spr3dldr.h"
#include "imeshobj.h"
#include "iengine.h"
#include "isystem.h"
#include "imspr3d.h"
#include "igraph3d.h"
#include "qint.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)

  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csSprite3DFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csSprite3DLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csSprite3DFactoryLoader)
IMPLEMENT_FACTORY (csSprite3DLoader)

EXPORT_CLASS_TABLE (spr3dldr)
  EXPORT_CLASS (csSprite3DFactoryLoader, "crystalspace.mesh.loader.factory.sprite.3d",
    "Crystal Space Sprite3D Mesh Factory Loader")
  EXPORT_CLASS (csSprite3DLoader, "crystalspace.mesh.loader.sprite.3d",
    "Crystal Space Sprite3D Mesh Loader")
EXPORT_CLASS_TABLE_END

csSprite3DFactoryLoader::csSprite3DFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite3DFactoryLoader::~csSprite3DFactoryLoader ()
{
}

bool csSprite3DFactoryLoader::Initialize (iSystem* system)
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

iBase* csSprite3DFactoryLoader::Parse (const char* string, iEngine* engine)
{
  // @@@ Implement MIXMODE
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys, "crystalspace.mesh.object.sprite.3d", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.sprite.3d", "MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.sprite.3d\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  iSprite3DFactoryState* spr3dLook = QUERY_INTERFACE (fact, iSprite3DFactoryState);

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      fact->DecRef ();
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
            return NULL;
	  }
	  spr3dLook->SetMaterialWrapper (mat);
	}
	break;
    }
  }

  return fact;
}

//---------------------------------------------------------------------------

csSprite3DLoader::csSprite3DLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csSprite3DLoader::~csSprite3DLoader ()
{
}

bool csSprite3DLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csSprite3DLoader::Parse (const char* string, iEngine* engine)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iSprite3DState* spr3dLook = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
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
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
          spr3dLook = QUERY_INTERFACE (mesh, iSprite3DState);
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
            return NULL;
	  }
	  spr3dLook->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        spr3dLook->SetMixMode (ParseMixmode (params));
	break;
    }
  }

  return mesh;
}

//---------------------------------------------------------------------------


