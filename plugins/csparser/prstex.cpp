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
#include "imap/ldrctxt.h"
#include "csutil/scanstr.h"
#include "iutil/document.h"
#include "cstool/proctex.h"
#include "cstool/prdots.h"
#include "cstool/prfire.h"
#include "cstool/prplasma.h"
#include "cstool/prwater.h"
#include "csgfx/rgbpixel.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/region.h"
#include "iutil/objreg.h"

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
        //if (!ParseMaterial (child, prefix))
	//  return false;
	ParseMaterial (child, prefix);
        break;
      default:
        SyntaxService->ReportBadToken (child);
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
        SyntaxService->ReportBadToken (child);
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
	    return NULL;
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
	    return NULL;
          if (for3d)
            flags |= CS_TEXTURE_3D;
          else
            flags &= ~CS_TEXTURE_3D;
	}
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
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.texture",
	      child, "Expected VFS filename for 'file'!");
	    return NULL;
	  }
          strcpy (filename, fname);
	}
        break;
      case XMLTOKEN_MIPMAP:
	{
	  bool mm;
	  if (!SyntaxService->ParseBool (child, mm, true))
	    return NULL;
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
	    return NULL;
          if (di)
            flags |= CS_TEXTURE_DITHER;
          else
            flags &= ~CS_TEXTURE_DITHER;
	}
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return NULL;
    }
  }

  // The size of image should be checked before registering it with
  // the 3D or 2D driver... if the texture is used for 2D only, it can
  // not have power-of-two dimensions...

  csRef<iTextureWrapper> tex (LoadTexture (txtname, filename, flags,
  	NULL, false, false));	// Do not create a default material.
  if (tex && do_transp)
    tex->SetKeyColor (QInt (transp.red * 255.99),
      QInt (transp.green * 255.99), QInt (transp.blue * 255.99));

  if (!tex)
  {
    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.texture",
	      node, "Could not load texture '%s'\n", txtname);
  }

  if (tex) tex->IncRef ();	// To avoid smart pointer release.
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
	  SyntaxService->ReportError (
	      "crystalspace.maploader.parse.proctex",
	      child, "'type' of proctex already specified!");
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
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.proctex",
	      child, "Unknown 'type' '%s' of proctex!", type);
	    return NULL;
	  }
	}
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return NULL;
    }
  }

  if (pt == NULL)
  {
    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.proctex",
	      node, "'type' of proctex not given!");
    return NULL;
  }

  iMaterialWrapper *mw = pt->Initialize (object_reg, Engine,
	 G3D ? G3D->GetTextureManager () : NULL,
	 node->GetAttributeValue ("name"));
  mw->QueryObject ()->ObjAdd (pt);
  pt->DecRef ();
  return pt->GetTextureWrapper ();
}

iMaterialWrapper* csLoader::ParseMaterial (iDocumentNode* node,
	const char *prefix)
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
  
  csRef<iEffectDefinition> efdef ;

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
	  texh = GetLoaderContext()->FindTexture (txtname);
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
      case XMLTOKEN_EFFECT:
        {
          csRef<iEffectServer> efs = CS_QUERY_REGISTRY(object_reg, iEffectServer);
          if(!efs.IsValid())
          {
            ReportNotify ("Effectserver isn't found. Ignoring effect-directive in material %s",matname);
            break;
          }
          const char* efname = child->GetContentsValue();
          efdef = efs->GetEffect(efname);
          if(!efdef.IsValid())
          {
            ReportNotify ("Effect (%s) couldn't be found for material %s, ignoring it", efname,matname);
            break;
          }

        }
        break;
      case XMLTOKEN_LAYER:
	{
	  if (num_txt_layer >= 4)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.material",
	      child, "Only four texture layers supported!");
	    return NULL;
	  }
	  txt_layers[num_txt_layer] = NULL;
	  layers[num_txt_layer].txt_handle = NULL;
	  layers[num_txt_layer].uscale = 1;
	  layers[num_txt_layer].vscale = 1;
	  layers[num_txt_layer].ushift = 0;
	  layers[num_txt_layer].vshift = 0;
#ifndef CS_USE_NEW_RENDERER
          layers[num_txt_layer].mode = CS_FX_ADD | CS_FX_TILING;
#endif // CS_USE_NEW_RENDERER
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
		    SyntaxService->ReportError (
			"crystalspace.maploader.parse.material",
		    	child, "Cannot find texture `%s' for material `%s'!",
			txtname, matname);
		    return NULL;
                  }
		}
		break;
	      case XMLTOKEN_SCALE:
		layers[num_txt_layer].uscale = layer_child
				->GetAttributeValueAsFloat ("u");
		layers[num_txt_layer].vscale = layer_child
				->GetAttributeValueAsFloat ("v");
	        break;
	      case XMLTOKEN_SHIFT:
		layers[num_txt_layer].ushift = layer_child
				->GetAttributeValueAsFloat ("u");
		layers[num_txt_layer].vshift = layer_child
				->GetAttributeValueAsFloat ("v");
	        break;
	      case XMLTOKEN_MIXMODE:
		if (!SyntaxService->ParseMixmode (layer_child,
		    layers[num_txt_layer].mode))
		  return NULL;
	        break;
	      default:
	        SyntaxService->ReportBadToken (layer_child);
		return NULL;
	    }
	  }
	  num_txt_layer++;
	}
        break;
      default:
	SyntaxService->ReportBadToken (child);
	return NULL;
    }
  }

  csRef<iMaterial> material (Engine->CreateBaseMaterial (texh,
  	num_txt_layer, txt_layers, layers));
  if (col_set)
    material->SetFlatColor (col);
  material->SetReflection (diffuse, ambient, reflection);

  if(efdef.IsValid())
    material->SetEffect(efdef);

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

  return mat;
}

