/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein
    Copyright (C) 1998-2000 by Ivan Avramovic <ivan@avramovic.com>

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
#include "qint.h"
#include "csloader.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "cstool/proctex.h"
#include "cstool/prdots.h"
#include "cstool/prfire.h"
#include "cstool/prplasma.h"
#include "cstool/prwater.h"
#include "csgfx/rgbpixel.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/region.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (AMBIENT)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (DIFFUSE)
  CS_TOKEN_DEF (DITHER)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (FOR_2D)
  CS_TOKEN_DEF (FOR_3D)
  CS_TOKEN_DEF (HEIGHTGEN)
  CS_TOKEN_DEF (LAYER)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIPMAP)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (PERSISTENT)
  CS_TOKEN_DEF (PROCEDURAL)
  CS_TOKEN_DEF (PROCTEX)
  CS_TOKEN_DEF (REFLECTION)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (SHIFT)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (TYPE)
CS_TOKEN_DEF_END

bool csLoader::ParseTextureList (char* buf)
{
  if (!Engine || !ImageLoader) return false;

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (HEIGHTGEN)
    CS_TOKEN_TABLE (PROCTEX)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing textures!", buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_TEXTURE:
        if (!ParseTexture (name, params))
	  return false;
        break;
      case CS_TOKEN_HEIGHTGEN:
        if (!ParseHeightgen (params))
	  return false;
        break;
      case CS_TOKEN_PROCTEX:
        if (!ParseProcTex (name, params))
	  return false;
	break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("textures");
    return false;
  }

  return true;
}

bool csLoader::ParseMaterialList (char* buf, const char* prefix)
{
  if (!Engine) return false;

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      ReportError (
	  "crystalspace.maploader.parse.badformat",
	  "Expected parameters instead of '%s' while parsing materials!", buf);
      return false;
    }
    switch (cmd)
    {
      case CS_TOKEN_MATERIAL:
        if (!ParseMaterial (name, params, prefix))
	  return false;
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("materials");
    return false;
  }

  return true;
}

iTextureWrapper* csLoader::ParseProcTex (char *name, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TYPE)
  CS_TOKEN_TABLE_END

  long cmd;
  char *params;
  csProcTexture* pt = NULL;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_TYPE:
        if (pt)
	{
	  ReportError (
	      "crystalspace.maploader.parse.proctex",
	      "TYPE of proctex already specified!");
	  return NULL;
	}
	else
	{
          if (!strcmp (params, "DOTS"))
	    pt = new csProcDots ();
	  else if (!strcmp (params, "PLASMA"))
	    pt = new csProcPlasma ();
	  else if (!strcmp (params, "WATER"))
	    pt = new csProcWater ();
	  else if (!strcmp (params, "FIRE"))
	    pt = new csProcFire ();
	  else
	  {
	    ReportError (
	      "crystalspace.maploader.parse.proctex",
	      "Unknown TYPE '%s' of proctex!", params);
	    return NULL;
	  }
	}
        break;
    }
  }

  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("a proctex specification");
    return NULL;
  }

  if (pt == NULL)
  {
    ReportError (
	      "crystalspace.maploader.parse.proctex",
	      "TYPE of proctex not given!");
    return NULL;
  }

  iMaterialWrapper *mw = pt->Initialize (object_reg, Engine,
					 G3D ? G3D->GetTextureManager () : NULL, name);
  mw->QueryObject ()->ObjAdd (pt);
  pt->DecRef ();
  return pt->GetTextureWrapper ();
}

iTextureWrapper* csLoader::ParseTexture (char *name, char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (FILE)
    CS_TOKEN_TABLE (MIPMAP)
    CS_TOKEN_TABLE (DITHER)
    CS_TOKEN_TABLE (PROCEDURAL)
    CS_TOKEN_TABLE (PERSISTENT)
    CS_TOKEN_TABLE (FOR_2D)
    CS_TOKEN_TABLE (FOR_3D)
  CS_TOKEN_TABLE_END

  long cmd;
  const char *filename = name;
  char *params;
  csColor transp (0, 0, 0);
  bool do_transp = false;
  int flags = CS_TEXTURE_3D;

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_FOR_2D:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_2D;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_2D;
        else
	{
	  ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Invalid FOR_2D() value, 'yes' or 'no' expected!");
	  return NULL;
	}
        break;
      case CS_TOKEN_FOR_3D:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_3D;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_3D;
        else
	{
	  ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Invalid FOR_3D() value, 'yes' or 'no' expected!");
	  return NULL;
	}
        break;
      case CS_TOKEN_PERSISTENT:
        flags |= CS_TEXTURE_PROC_PERSISTENT;
        break;
      case CS_TOKEN_PROCEDURAL:
        flags |= CS_TEXTURE_PROC;
        break;
      case CS_TOKEN_TRANSPARENT:
        do_transp = true;
        csScanStr (params, "%f,%f,%f", &transp.red, &transp.green,
		&transp.blue);
        break;
      case CS_TOKEN_FILE:
        filename = params;
        break;
      case CS_TOKEN_MIPMAP:
        if (strcasecmp (params, "yes") == 0)
          flags &= ~CS_TEXTURE_NOMIPMAPS;
        else if (strcasecmp (params, "no") == 0)
          flags |= CS_TEXTURE_NOMIPMAPS;
        else
	{
	  ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Invalid MIPMAP() value, 'yes' or 'no' expected!");
	  return NULL;
	}
        break;
      case CS_TOKEN_DITHER:
        if (strcasecmp (params, "yes") == 0)
          flags |= CS_TEXTURE_DITHER;
        else if (strcasecmp (params, "no") == 0)
          flags &= ~CS_TEXTURE_DITHER;
        else
	{
	  ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Invalid DITHER() value, 'yes' or 'no' expected!");
	  return NULL;
	}
        break;
    }
  }

  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("a texture specification");
    return NULL;
  }

  // The size of image should be checked before registering it with
  // the 3D or 2D driver... if the texture is used for 2D only, it can
  // not have power-of-two dimensions...

  iTextureWrapper *tex = LoadTexture (name, filename, flags);
  if (tex && do_transp)
    tex->SetKeyColor (QInt (transp.red * 255.99),
      QInt (transp.green * 255.99), QInt (transp.blue * 255.99));

  return tex;
}

iMaterialWrapper* csLoader::ParseMaterial (char *name, char* buf, const char *prefix)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (DIFFUSE)
    CS_TOKEN_TABLE (AMBIENT)
    CS_TOKEN_TABLE (REFLECTION)
    CS_TOKEN_TABLE (LAYER)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (layerCommands)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (SCALE)
    CS_TOKEN_TABLE (SHIFT)
  CS_TOKEN_TABLE_END

  long cmd;
  char *params;
  char str [255];

  iTextureWrapper* texh = 0;
  bool col_set = false;
  csRGBcolor col;
  float diffuse = CS_DEFMAT_DIFFUSE;
  float ambient = CS_DEFMAT_AMBIENT;
  float reflection = CS_DEFMAT_REFLECTION;
  int num_txt_layer = 0;
  csTextureLayer layers[4];
  iTextureWrapper* txt_layers[4];

  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_TEXTURE:
      {
        csScanStr (params, "%s", str);
	if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
	  texh = Engine->GetCurrentRegion ()->FindTexture (str);
	else
          texh = Engine->GetTextureList ()->FindByName (str);
        if (!texh)
        {
	  ReportError (
	      "crystalspace.maploader.parse.material",
	      "Cannot find texture '%s' for material `%s'", str, name);
	  return NULL;
        }
        break;
      }
      case CS_TOKEN_COLOR:
        col_set = true;
        if (!ParseColor (params, col))
	  return NULL;
        break;
      case CS_TOKEN_DIFFUSE:
        csScanStr (params, "%f", &diffuse);
        break;
      case CS_TOKEN_AMBIENT:
        csScanStr (params, "%f", &ambient);
        break;
      case CS_TOKEN_REFLECTION:
        csScanStr (params, "%f", &reflection);
        break;
      case CS_TOKEN_LAYER:
	{
	  if (num_txt_layer >= 4)
	  {
	    ReportError (
	      "crystalspace.maploader.parse.material",
	      "Only four texture layers supported!");
	    return NULL;
	  }
	  txt_layers[num_txt_layer] = NULL;
	  layers[num_txt_layer].txt_handle = NULL;
	  layers[num_txt_layer].uscale = 1;
	  layers[num_txt_layer].vscale = 1;
	  layers[num_txt_layer].ushift = 0;
	  layers[num_txt_layer].vshift = 0;
	  layers[num_txt_layer].mode = CS_FX_ADD | CS_FX_TILING;
	  char* params2;
	  while ((cmd = csGetCommand (&params, layerCommands,
		&params2)) > 0)
	  {
	    switch (cmd)
	    {
	      case CS_TOKEN_TEXTURE:
		{
                  csScanStr (params2, "%s", str);
		  iTextureWrapper* texh;
		  if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
		    texh = Engine->GetCurrentRegion ()->FindTexture (str);
		  else
                    texh = Engine->GetTextureList ()->FindByName (str);
                  if (texh)
                    txt_layers[num_txt_layer] = texh;
                  else
                  {
		    ReportError (
			"crystalspace.maploader.parse.material",
		    	"Cannot find texture `%s' for material `%s'!",
			str, name);
		    return NULL;
                  }
		}
		break;
	      case CS_TOKEN_SCALE:
	        csScanStr (params2, "%f,%f",
			&layers[num_txt_layer].uscale,
			&layers[num_txt_layer].vscale);
	        break;
	      case CS_TOKEN_SHIFT:
	        csScanStr (params2, "%f,%f",
			&layers[num_txt_layer].ushift,
			&layers[num_txt_layer].vshift);
	        break;
	      case CS_TOKEN_MIXMODE:
	        layers[num_txt_layer].mode = ParseMixmode (params2);
		if (layers[num_txt_layer].mode == (UInt)~0)
		  return NULL;
	        break;
	    }
	  }
	  num_txt_layer++;
	}
        break;
    }
  }

  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("a material specification");
    return NULL;
  }

  iMaterial* material = Engine->CreateBaseMaterial (texh,
  	num_txt_layer, txt_layers, layers);
  if (col_set)
    material->SetFlatColor (col);
  material->SetReflection (diffuse, ambient, reflection);
  iMaterialWrapper *mat = Engine->GetMaterialList ()->NewMaterial (material);
  if (prefix)
  {
    char *prefixedname = new char [strlen (name) + strlen (prefix) + 2];
    strcpy (prefixedname, prefix);
    strcat (prefixedname, "_");
    strcat (prefixedname, name);
    mat->QueryObject()->SetName (prefixedname);
    delete [] prefixedname;
  }
  else
    mat->QueryObject()->SetName (name);
  // dereference material since mat already incremented it
  material->DecRef ();

  return mat;
}
