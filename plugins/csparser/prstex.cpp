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
#include "csutil/parser.h"
#include "csloader.h"
#include "csutil/scanstr.h"
#include "iutil/document.h"
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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

  while ((cmd = parser.GetObject (&buf, commands, &name, &params)) > 0)
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
  if (!Engine) return NULL;

  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (TYPE)
  CS_TOKEN_TABLE_END

  long cmd;
  char *params;
  csProcTexture* pt = NULL;

  while ((cmd = parser.GetCommand (&buf, commands, &params)) > 0)
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
  char filename[256];
  strcpy (filename, name);
  char *params;
  csColor transp (0, 0, 0);
  bool do_transp = false;
  int flags = CS_TEXTURE_3D;

  while ((cmd = parser.GetCommand (&buf, commands, &params)) > 0)
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
        csScanStr (params, "%s", filename);
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

  if (!tex)
  {
    ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Could not load texture '%s'\n", name);
  }

  return tex;
}

iMaterialWrapper* csLoader::ParseMaterial (char *name, char* buf, const char *prefix)
{
  if (!Engine) return NULL;

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

  while ((cmd = parser.GetCommand (&buf, commands, &params)) > 0)
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
	  while ((cmd = parser.GetCommand (&params, layerCommands,
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
		if (!SyntaxService->ParseMixmode (&parser, params2, 
		  layers[num_txt_layer].mode))
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

// XML versions -------------------------------------------------------------

bool csLoader::ParseMaterialList (iDocumentNode* node, const char* prefix)
{
  if (!Engine) return false;

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
        if (!ParseMaterial (child, prefix))
	  return false;
        break;
      default:
        TokenError ("materials", value);
	return false;
    }
  }

  return true;
}

bool csLoader::ParseTextureList (iDocumentNode* node)
{
  if (!Engine || !ImageLoader) return false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_TEXTURE:
        if (!ParseTexture (child))
	  return false;
        break;
      case XMLTOKEN_HEIGHTGEN:
        if (!ParseHeightgen (child))
	  return false;
        break;
      case XMLTOKEN_PROCTEX:
        if (!ParseProcTex (child))
	  return false;
	break;
      default:
	TokenError ("textures", value);
	return false;
    }
  }

  return true;
}

iTextureWrapper* csLoader::ParseTexture (iDocumentNode* node)
{
  const char* txtname = node->GetAttributeValue ("name");

  char filename[256];
  strcpy (filename, txtname);
  csColor transp (0, 0, 0);
  bool do_transp = false;
  int flags = CS_TEXTURE_3D;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FOR2D:
	{
	  bool for2d;
	  if (!SyntaxService->ParseBool (child, for2d, true))
	  {
	    ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Invalid 'for2d' value, 'yes' or 'no' expected!");
	    return NULL;
	  }
          if (for2d)
            flags |= CS_TEXTURE_2D;
          else
            flags &= ~CS_TEXTURE_2D;
	}
        break;
      case XMLTOKEN_FOR3D:
	{
	  bool for3d;
	  if (!SyntaxService->ParseBool (child, for3d, true))
	  {
	    ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Invalid 'for3d' value, 'yes' or 'no' expected!");
	    return NULL;
	  }
          if (for3d)
            flags |= CS_TEXTURE_3D;
          else
            flags &= ~CS_TEXTURE_3D;
	}
        break;
      case XMLTOKEN_PERSISTENT:
        flags |= CS_TEXTURE_PROC_PERSISTENT;
        break;
      case XMLTOKEN_PROCEDURAL:
        flags |= CS_TEXTURE_PROC;
        break;
      case XMLTOKEN_TRANSPARENT:
        do_transp = true;
	if (!SyntaxService->ParseColor (child, transp))
	  return NULL;
        break;
      case XMLTOKEN_FILE:
	{
	  const char* fname = child->GetContentsValue ();
	  if (!fname)
	  {
	    ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Expected VFS filename for 'file'!");
	    return NULL;
	  }
          strcpy (filename, fname);
	}
        break;
      case XMLTOKEN_MIPMAP:
	{
	  bool mm;
	  if (!SyntaxService->ParseBool (child, mm, true))
	  {
	    ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Invalid 'mipmap' value, 'yes' or 'no' expected!");
	    return NULL;
	  }
          if (mm)
            flags &= ~CS_TEXTURE_NOMIPMAPS;
          else
            flags |= CS_TEXTURE_NOMIPMAPS;
	}
        break;
      case XMLTOKEN_DITHER:
	{
	  bool di;
	  if (!SyntaxService->ParseBool (child, di, true))
	  {
	    ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Invalid 'dither' value, 'yes' or 'no' expected!");
	    return NULL;
	  }
          if (di)
            flags |= CS_TEXTURE_DITHER;
          else
            flags &= ~CS_TEXTURE_DITHER;
	}
        break;
      default:
	TokenError ("a texture specification", value);
	return NULL;
    }
  }

  // The size of image should be checked before registering it with
  // the 3D or 2D driver... if the texture is used for 2D only, it can
  // not have power-of-two dimensions...

  iTextureWrapper *tex = LoadTexture (txtname, filename, flags);
  if (tex && do_transp)
    tex->SetKeyColor (QInt (transp.red * 255.99),
      QInt (transp.green * 255.99), QInt (transp.blue * 255.99));

  if (!tex)
  {
    ReportError (
	      "crystalspace.maploader.parse.texture",
	      "Could not load texture '%s'\n", txtname);
  }

  return tex;
}

iTextureWrapper* csLoader::ParseProcTex (iDocumentNode* node)
{
  if (!Engine) return NULL;

  csProcTexture* pt = NULL;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_TYPE:
        if (pt)
	{
	  ReportError (
	      "crystalspace.maploader.parse.proctex",
	      "'type' of proctex already specified!");
	  return NULL;
	}
	else
	{
	  const char* type = child->GetContentsValue ();
          if (!strcasecmp (type, "dots"))
	    pt = new csProcDots ();
	  else if (!strcasecmp (type, "plasma"))
	    pt = new csProcPlasma ();
	  else if (!strcasecmp (type, "water"))
	    pt = new csProcWater ();
	  else if (!strcasecmp (type, "fire"))
	    pt = new csProcFire ();
	  else
	  {
	    ReportError (
	      "crystalspace.maploader.parse.proctex",
	      "Unknown 'type' '%s' of proctex!", type);
	    return NULL;
	  }
	}
        break;
      default:
	TokenError ("a proctex specification", value);
	return NULL;
    }
  }

  if (pt == NULL)
  {
    ReportError (
	      "crystalspace.maploader.parse.proctex",
	      "'type' of proctex not given!");
    return NULL;
  }

  iMaterialWrapper *mw = pt->Initialize (object_reg, Engine,
	 G3D ? G3D->GetTextureManager () : NULL,
	 node->GetAttributeValue ("name"));
  mw->QueryObject ()->ObjAdd (pt);
  pt->DecRef ();
  return pt->GetTextureWrapper ();
}

iMaterialWrapper* csLoader::ParseMaterial (iDocumentNode* node, const char *prefix)
{
  if (!Engine) return NULL;

  const char* matname = node->GetAttributeValue ("name");

  iTextureWrapper* texh = 0;
  bool col_set = false;
  csRGBcolor col;
  float diffuse = CS_DEFMAT_DIFFUSE;
  float ambient = CS_DEFMAT_AMBIENT;
  float reflection = CS_DEFMAT_REFLECTION;
  int num_txt_layer = 0;
  csTextureLayer layers[4];
  iTextureWrapper* txt_layers[4];

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_TEXTURE:
        {
	  const char* txtname = child->GetContentsValue ();
	  if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
	    texh = Engine->GetCurrentRegion ()->FindTexture (txtname);
	  else
            texh = Engine->GetTextureList ()->FindByName (txtname);
          if (!texh)
          {
	    ReportError (
	        "crystalspace.maploader.parse.material",
	        "Cannot find texture '%s' for material `%s'", txtname, matname);
	    return NULL;
          }
	}
        break;
      case XMLTOKEN_COLOR:
	{
          col_set = true;
	  csColor color;
          if (!SyntaxService->ParseColor (child, color))
	    return NULL;
	  col.red = QInt (color.red * 255.99f);
	  col.green = QInt (color.green * 255.99f);
	  col.blue = QInt (color.blue * 255.99f);
	}
        break;
      case XMLTOKEN_DIFFUSE:
	diffuse = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_AMBIENT:
	ambient = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_REFLECTION:
	reflection = child->GetContentsValueAsFloat ();
        break;
      case XMLTOKEN_LAYER:
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
	  csRef<iDocumentNodeIterator> layer_it = child->GetNodes ();
	  while (layer_it->HasNext ())
	  {
	    csRef<iDocumentNode> layer_child = layer_it->Next ();
	    if (layer_child->GetType () != CS_NODE_ELEMENT) continue;
	    const char* layer_value = layer_child->GetValue ();
	    csStringID layer_id = xmltokens.Request (layer_value);
	    switch (layer_id)
	    {
	      case XMLTOKEN_TEXTURE:
		{
		  const char* txtname = layer_child->GetContentsValue ();
		  iTextureWrapper* texh;
		  if (ResolveOnlyRegion && Engine->GetCurrentRegion ())
		    texh = Engine->GetCurrentRegion ()->FindTexture (txtname);
		  else
                    texh = Engine->GetTextureList ()->FindByName (txtname);
                  if (texh)
                    txt_layers[num_txt_layer] = texh;
                  else
                  {
		    ReportError (
			"crystalspace.maploader.parse.material",
		    	"Cannot find texture `%s' for material `%s'!",
			txtname, matname);
		    return NULL;
                  }
		}
		break;
	      case XMLTOKEN_SCALE:
		layers[num_txt_layer].uscale = child->GetAttributeValueAsFloat ("u");
		layers[num_txt_layer].vscale = child->GetAttributeValueAsFloat ("v");
	        break;
	      case XMLTOKEN_SHIFT:
		layers[num_txt_layer].ushift = child->GetAttributeValueAsFloat ("u");
		layers[num_txt_layer].vshift = child->GetAttributeValueAsFloat ("v");
	        break;
	      case XMLTOKEN_MIXMODE:
		if (!SyntaxService->ParseMixmode (layer_child,
		    layers[num_txt_layer].mode))
		  return NULL;
	        break;
	      default:
		TokenError ("a layer specification", layer_value);
		return NULL;
	    }
	  }
	  num_txt_layer++;
	}
        break;
      default:
        TokenError ("a material specification", value);
	return NULL;
    }
  }

  iMaterial* material = Engine->CreateBaseMaterial (texh,
  	num_txt_layer, txt_layers, layers);
  if (col_set)
    material->SetFlatColor (col);
  material->SetReflection (diffuse, ambient, reflection);
  iMaterialWrapper *mat = Engine->GetMaterialList ()->NewMaterial (material);
  if (prefix)
  {
    char *prefixedname = new char [strlen (matname) + strlen (prefix) + 2];
    strcpy (prefixedname, prefix);
    strcat (prefixedname, "_");
    strcat (prefixedname, matname);
    mat->QueryObject()->SetName (prefixedname);
    delete [] prefixedname;
  }
  else
    mat->QueryObject()->SetName (matname);
  // dereference material since mat already incremented it
  material->DecRef ();

  return mat;
}

