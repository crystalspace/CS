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
#include "csqint.h"
#include "ivideo/graph3d.h"
#include "csloader.h"
#include "imap/ldrctxt.h"
#include "imap/reader.h"
#include "csutil/array.h"
#include "csutil/scanstr.h"
#include "iutil/document.h"
#include "iutil/strset.h"
#include "cstool/proctex.h"
#include "cstool/prdots.h"
#include "cstool/prfire.h"
#include "cstool/prplasma.h"
#include "cstool/prwater.h"
#include "csgfx/rgbpixel.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "iengine/region.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"
#include "ivaria/keyval.h"
#include "igraphic/animimg.h"
#include "csgfx/csimgvec.h"
#include "loadtex.h"
#include "ivideo/shader/shader.h"

#define PLUGIN_LEGACY_TEXTYPE_PREFIX  "crystalspace.texture.loader."

bool csLoader::ParseMaterialList (iLoaderContext* ldr_context,
	iDocumentNode* node, const char* prefix)
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
	if (!ParseMaterial (ldr_context, child, prefix))
	  return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  return true;
}

bool csLoader::ParseTextureList (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  if (!ImageLoader)
  {
    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.textures",
	      node, "Image loader is missing!");
    return false;
  }
  static bool proctex_deprecated_warned = false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_PROCTEX:
/*        if (!ParseProcTex (child))
	  return false;
	break;*/
	if (!proctex_deprecated_warned)
	{
	  SyntaxService->Report (
	    "crystalspace.maploader.parse.texture",
	    CS_REPORTER_SEVERITY_NOTIFY,
	    child,
	    "Use of <proctex> is deprecated. "
	    "Procedural textures can now be specified with the <texture> node as well.");
	  proctex_deprecated_warned = true;
	}
      case XMLTOKEN_TEXTURE:
        if (!ParseTexture (ldr_context, child))
	        return false;
        break;
      case XMLTOKEN_HEIGHTGEN:
        if (!ParseHeightgen (ldr_context, child))
	        return false;
        break;
      case XMLTOKEN_CUBEMAP:
#ifdef CS_USE_NEW_RENDERER
        if (!ParseCubemap (ldr_context, child))
          return false;
#endif
        break;
      case XMLTOKEN_TEXTURE3D:
#ifdef CS_USE_NEW_RENDERER
        if (!ParseTexture3D (ldr_context, child))
          return false;
#endif // CS_USE_NEW_RENDERER
        break;
      default:
        SyntaxService->ReportBadToken (child);
	      return false;
    }
  }

  return true;
}

iTextureWrapper* csLoader::ParseTexture (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  const char* txtname = node->GetAttributeValue ("name");
  if (ldr_context->CheckDupes ())
  {
    iTextureWrapper* t = Engine->FindTexture (txtname);
    if (t) return t;
  }

  static bool deprecated_warned = false;

  csRef<iTextureWrapper> tex;
  csRef<iLoaderPlugin> plugin;

  char filename[256] = "";
  csColor transp (0, 0, 0);
  bool do_transp = false;
  bool keep_image = false;
  bool always_animate = false;
  TextureLoaderContext context (txtname);
  csRef<iDocumentNode> ParamsNode;
  char* type = 0;

  csRefArray<iDocumentNode> key_nodes;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_KEY:
	key_nodes.Push (child);
	break;
      case XMLTOKEN_FOR2D:
	{
	  bool for2d;
	  if (!SyntaxService->ParseBool (child, for2d, true))
	    goto error;
          if (for2d)
	    context.SetFlags (context.GetFlags() | CS_TEXTURE_2D);
          else
	    context.SetFlags (context.GetFlags() & ~CS_TEXTURE_2D);
	}
        break;
      case XMLTOKEN_FOR3D:
	{
	  bool for3d;
	  if (!SyntaxService->ParseBool (child, for3d, true))
	    goto error;
          if (for3d)
	    context.SetFlags (context.GetFlags() | CS_TEXTURE_3D);
          else
	    context.SetFlags (context.GetFlags() & ~CS_TEXTURE_3D);
	}
        break;
      case XMLTOKEN_TRANSPARENT:
        do_transp = true;
	if (!SyntaxService->ParseColor (child, transp))
	  goto error;
        break;
      case XMLTOKEN_FILE:
	{
	  const char* fname = child->GetContentsValue ();
	  if (!fname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.texture",
	      child, "Expected VFS filename for 'file'!");
	    goto error;
	  }
          strcpy (filename, fname);
	}
        break;
      case XMLTOKEN_MIPMAP:
	{
	  bool mm;
	  if (!SyntaxService->ParseBool (child, mm, true))
	    goto error;
          if (!mm)
	    context.SetFlags (context.GetFlags() | CS_TEXTURE_NOMIPMAPS);
          else
	    context.SetFlags (context.GetFlags() & ~CS_TEXTURE_NOMIPMAPS);
	}
        break;
      case XMLTOKEN_DITHER:
	{
	  bool di;
	  if (!SyntaxService->ParseBool (child, di, true))
	    goto error;
          if (di)
	    context.SetFlags (context.GetFlags() | CS_TEXTURE_DITHER);
          else
	    context.SetFlags (context.GetFlags() & ~CS_TEXTURE_DITHER);
	}
        break;
      case XMLTOKEN_KEEPIMAGE:
        {
	  if (!SyntaxService->ParseBool (child, keep_image, true))
	    goto error;
	}
	break;
      case XMLTOKEN_PARAMS:
	ParamsNode = child;
	break;
      case XMLTOKEN_TYPE:
	type = csStrNew (child->GetContentsValue ());
	if (!type)
	{
	  SyntaxService->ReportError (
	    "crystalspace.maploader.parse.texture",
	    child, "Expected plugin ID for <type>!");
	  goto error;
	}
	break;
      case XMLTOKEN_SIZE:
	{
	  csRef<iDocumentAttribute> attr_w, attr_h;
	  if ((attr_w = child->GetAttribute ("width")) &&
	    (attr_h = child->GetAttribute ("height")))
	  {
	    context.SetSize (attr_w->GetValueAsInt(),
	      attr_h->GetValueAsInt());
	  }
	}
	break;
      case XMLTOKEN_ALWAYSANIMATE:
	if (!SyntaxService->ParseBool (child, always_animate, true))
	  goto error;
	break;
      case XMLTOKEN_CLAMP:
        {
          bool c;
          if (!SyntaxService->ParseBool (child, c, true))
            goto error;
          if (c)
            context.SetFlags (context.GetFlags() | CS_TEXTURE_CLAMP);
          else
            context.SetFlags (context.GetFlags() & ~CS_TEXTURE_CLAMP);
        }
        break;
      case XMLTOKEN_FILTER:
        {
          bool c;
          if (!SyntaxService->ParseBool (child, c, true))
            goto error;
          if (c)
            context.SetFlags (context.GetFlags() & ~CS_TEXTURE_NOFILTER);
          else
            context.SetFlags (context.GetFlags() | CS_TEXTURE_NOFILTER);
        }
        break;
      default:
        SyntaxService->ReportBadToken (child);
	goto error;
    }
  }

  // @@@ some more comments
  if ((!type || (*type == 0)) && (filename[0] == 0))
  {
    strcpy (filename, txtname);
  }

  iTextureManager* tm;
  tm = G3D ? G3D->GetTextureManager() : 0;
  int Format;
  Format = tm ? tm->GetTextureFormat () : CS_IMGFMT_TRUECOLOR;
  if (filename && (filename[0] != 0))
  {
    csRef<iImage> image = LoadImage (filename, Format);
    context.SetImage (image);
    if (image && !type)
    {
      // special treatment for animated textures
      csRef<iAnimatedImage> anim = csPtr<iAnimatedImage>
	(SCF_QUERY_INTERFACE (image, iAnimatedImage));
      if (anim && anim->IsAnimated())
      {
	type = csStrNew (PLUGIN_TEXTURELOADER_ANIMIMG);
      }
      else
      {
	// shortcut, no need to go through the plugin list facility
	if (!BuiltinImageTexLoader)
	{
	  csImageTextureLoader* itl = new csImageTextureLoader (0);
	  itl->Initialize (object_reg);
	  BuiltinImageTexLoader.AttachNew (itl);
	}
	plugin = BuiltinImageTexLoader;
      }
    }
  }
  
  iLoaderPlugin* Plug; Plug = 0;
  iBinaryLoaderPlugin* Binplug;
  if (type && !plugin)
  {
    iDocumentNode* defaults = 0;
    if (!loaded_plugins.FindPlugin (type, Plug, Binplug, defaults))
    {
      if ((!strcasecmp (type, "dots")) ||
	  (!strcasecmp (type, "plasma")) ||
	  (!strcasecmp (type, "water")) ||
	  (!strcasecmp (type, "fire")))
      {
	// old style proctex type
	if (!deprecated_warned)
	{
	  SyntaxService->Report (
	    "crystalspace.maploader.parse.texture",
	    CS_REPORTER_SEVERITY_NOTIFY,
	    node,
	    "Deprecated syntax used for proctex! "
	    "Specify a plugin classid or map the old types to their "
	    "plugin counterparts in the <plugins> node.");
	  deprecated_warned = true;
	}

	char* newtype = new char[
		strlen (PLUGIN_LEGACY_TEXTYPE_PREFIX) +
		strlen (type) + 1];
	strcpy (newtype, PLUGIN_LEGACY_TEXTYPE_PREFIX);
	strcat (newtype, type);
	delete[] type;
	type = newtype;

	loaded_plugins.FindPlugin (type, Plug, Binplug, defaults);
      }
    }
    plugin = Plug;

    if (defaults != 0)
    {
      ReportWarning (
	        "crystalspace.maploader.parse.texture",
                node, "'defaults' section is ignored for textures!");
    }
  }

  if (type && !plugin)
  {
    SyntaxService->Report (
      "crystalspace.maploader.parse.texture",
      CS_REPORTER_SEVERITY_WARNING,
      node, "Could not get plugin '%s', using default", type);

    if (!BuiltinImageTexLoader)
    {
      csImageTextureLoader* itl = new csImageTextureLoader (0);
      itl->Initialize (object_reg);
      BuiltinImageTexLoader.AttachNew (itl);
    }
    plugin = BuiltinImageTexLoader;
  }
  if (plugin)
  {
    csRef<iBase> b = plugin->Parse (ParamsNode,
      ldr_context, CS_STATIC_CAST(iBase*, &context));
    if (b) tex = SCF_QUERY_INTERFACE (b, iTextureWrapper);
  }

  if (!tex)
  {
    SyntaxService->Report (
      "crystalspace.maploader.parse.texture",
      CS_REPORTER_SEVERITY_WARNING,
      node, "Could not load texture '%s', using checkerboard instead", txtname);

    if (!BuiltinCheckerTexLoader)
    {
      csCheckerTextureLoader* ctl = new csCheckerTextureLoader (0);
      ctl->Initialize (object_reg);
      BuiltinCheckerTexLoader.AttachNew (ctl);
    }
    csRef<iBase> b = BuiltinCheckerTexLoader->Parse (ParamsNode,
      ldr_context, CS_STATIC_CAST(iBase*, &context));
    tex = SCF_QUERY_INTERFACE (b, iTextureWrapper);
  }

  delete[] type;

  if (tex)
  {
    tex->QueryObject ()->SetName (txtname);
    tex->SetKeepImage (keep_image);
    if (do_transp)
      tex->SetKeyColor (csQint (transp.red * 255.99),
        csQint (transp.green * 255.99), csQint (transp.blue * 255.99));

    csRef<iProcTexture> ipt = csPtr<iProcTexture>
      (SCF_QUERY_INTERFACE (tex, iProcTexture));
    if (ipt)
    {
      ipt->SetAlwaysAnimate (always_animate);
    }
    AddToRegion (ldr_context, tex->QueryObject ());

    size_t i;
    for (i = 0 ; i < key_nodes.Length () ; i++)
    {
      iKeyValuePair* kvp = ParseKey (key_nodes[i], tex->QueryObject());
      if (kvp)
	kvp->DecRef ();
      else
	return 0;
    }
  }

  return tex;

error:
  delete[] type;
  return 0;
}

iMaterialWrapper* csLoader::ParseMaterial (iLoaderContext* ldr_context,
	iDocumentNode* node, const char *prefix)
{
  if (!Engine) return 0;

  const char* matname = node->GetAttributeValue ("name");
  if (ldr_context->CheckDupes ())
  {
    iMaterialWrapper* m = Engine->FindMaterial (matname);
    if (m) return m;
  }

  iTextureWrapper* texh = 0;
  bool col_set = false;
  csRGBcolor col;
  float diffuse = CS_DEFMAT_DIFFUSE;
  float ambient = CS_DEFMAT_AMBIENT;
  float reflection = CS_DEFMAT_REFLECTION;
#ifndef CS_USE_NEW_RENDERER
  int num_txt_layer = 0;
  iTextureWrapper* txt_layers[4];
  csTextureLayer layers[4];
#endif
  
#ifdef CS_USE_NEW_RENDERER
  bool shaders_mentioned = false;	// If true there were shaders.
  csArray<csStringID> shadertypes;
  //csArray<iShader*> shaders;
  csArray<iShader*> shaders;
  csRefArray<csShaderVariable> shadervars;

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
    object_reg, "crystalspace.shared.stringset", iStringSet);
#endif // CS_USE_NEW_RENDERER

  csRefArray<iDocumentNode> key_nodes;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_KEY:
	key_nodes.Push (child);
	break;
      case XMLTOKEN_TEXTURE:
        {
	  const char* txtname = child->GetContentsValue ();
	  texh = ldr_context->FindTexture (txtname);
          if (!texh)
          {
 	    ReportError (
 	      "crystalspace.maploader.parse.material",
 	      "Cannot find texture '%s' for material `%s'", txtname, matname);
 	    return 0;
          }
	}
        break;
      case XMLTOKEN_COLOR:
	{
          col_set = true;
	  csColor color;
          if (!SyntaxService->ParseColor (child, color))
	    return 0;
	  col.red = csQint (color.red * 255.99f);
	  col.green = csQint (color.green * 255.99f);
	  col.blue = csQint (color.blue * 255.99f);
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
        // @@@ TODO: add shortcut to support NR!
#ifndef CS_USE_NEW_RENDERER
	{
	  if (num_txt_layer >= 4)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.material",
	      child, "Only four texture layers supported!");
	    return 0;
	  }
	  txt_layers[num_txt_layer] = 0;
	  layers[num_txt_layer].txt_handle = 0;
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
		  if (ldr_context->GetRegion ()
		  	&& ldr_context->CurrentRegionOnly ())
		    texh = ldr_context->GetRegion ()->FindTexture (txtname);
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
		    return 0;
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
		  return 0;
	        break;
	      default:
	        SyntaxService->ReportBadToken (layer_child);
		return 0;
	    }
	  }
	  num_txt_layer++;
	}
#endif
        break;
      case XMLTOKEN_SHADER:
#ifdef CS_USE_NEW_RENDERER
        {
	  shaders_mentioned = true;
          csRef<iShaderManager> shaderMgr = CS_QUERY_REGISTRY (object_reg,
	  	iShaderManager);
          if (!shaderMgr)
          {
            ReportNotify ("iShaderManager not found, ignoring shader!");
            break;
          }
          const char* shadername = child->GetContentsValue ();
	  iShader* shader = ldr_context->FindShader (shadername);
          if (!shader)
          {
            ReportNotify (
	    	"Shader (%s) couldn't be found for material %s, ignoring it",
		shadername, matname);
            break;
          }
          const char* shadertype = child->GetAttributeValue ("type");
          if (!shadertype)
          {
            ReportNotify (
	    	"No shadertype for shader %s in material %s, ignoring it",
		shadername, matname);
            break;
          }
          shadertypes.Push (strings->Request(shadertype));
          //shaders.Push (shader);
	  //csRef<iShaderWrapper> wrapper = shaderMgr->CreateWrapper (shader);
	  shaders.Push (shader);
        }
#endif //CS_USE_NEW_RENDERER
        break;
      case XMLTOKEN_SHADERVAR:
#ifdef CS_USE_NEW_RENDERER
        {
          csRef<iShaderManager> shaderMgr = CS_QUERY_REGISTRY (object_reg,
            iShaderManager);
          if (!shaderMgr)
          {
            ReportNotify ("iShaderManager not found, ignoring shadervar!");
            break;
          }
          //create a new variable
          const char* varname = child->GetAttributeValue ("name");
          /*csRef<csShaderVariable> var = 
            shaderMgr->CreateVariable (strings->Request (varname));*/
	  csRef<csShaderVariable> var;
	  var.AttachNew (new csShaderVariable (strings->Request (varname)));

          if (!SyntaxService->ParseShaderVar (child, *var))
          {
            ReportNotify ("Error loading shader variable '%s' in material '%s'.", 
              varname, matname);
            break;
          }
          shadervars.Push (var);
        }
#endif //CS_USE_NEW_RENDERER
        break;
      default:
	SyntaxService->ReportBadToken (child);
	return 0;
    }
  }

#ifdef CS_USE_NEW_RENDERER
  csRef<iMaterial> material = Engine->CreateBaseMaterial (texh);
#else
  csRef<iMaterial> material = Engine->CreateBaseMaterial (texh,
  	num_txt_layer, txt_layers, layers);
#endif
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
  {
    mat->QueryObject()->SetName (matname);
  }
  
  size_t i;
#ifdef CS_USE_NEW_RENDERER
  for (i=0; i<shaders.Length (); i++)
    //if (shaders[i]->Prepare ())
      material->SetShader (shadertypes[i], shaders[i]);
  for (i=0; i<shadervars.Length (); i++)
    material->AddVariable (shadervars[i]);
#endif // CS_USE_NEW_RENDERER
  // dereference material since mat already incremented it

  for (i = 0 ; i < key_nodes.Length () ; i++)
  {
    iKeyValuePair* kvp = ParseKey (key_nodes[i], mat->QueryObject ());
    if (kvp)
      kvp->DecRef ();
    else
      return 0;
  }
  AddToRegion (ldr_context, mat->QueryObject ());
  return mat;
}

#ifdef CS_USE_NEW_RENDERER
/// Parse a Cubemap texture definition and add the texture to the engine
iTextureWrapper* csLoader::ParseCubemap (iLoaderContext* ldr_context,
    iDocumentNode* node)
{
  csRef<csImageVector> cubetextures = new csImageVector();

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  iTextureManager* tm = G3D ? G3D->GetTextureManager() : 0;
  int Format = tm ? tm->GetTextureFormat () : CS_IMGFMT_TRUECOLOR;
 
  csRef<iTextureHandle> itex;
  csRef<iTextureWrapper> itexwrap;

  csRef<iImage> north, south, east, west, top, bottom;
  const char* texname = node->GetAttributeValue ("name");
  char *fname= 0;

  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_NORTH:
        fname = csStrNew (child->GetContentsValue ());
	if (!fname)
	{
	  SyntaxService->ReportError (
	       "crystalspace.maploader.parse.texture",
	       child, "Expected VFS filename for 'file'!");
	  goto error;
	}
      
        north = LoadImage(fname, Format);
        break;
    
      case XMLTOKEN_SOUTH:
        fname = csStrNew (child->GetContentsValue ());
	if (!fname)
	{
	  SyntaxService->ReportError (
	       "crystalspace.maploader.parse.texture",
	       child, "Expected VFS filename for 'file'!");
	  goto error;
	}
      
        south = LoadImage(fname, Format);
        break;
    
      case XMLTOKEN_EAST:
        fname = csStrNew (child->GetContentsValue ());
	if (!fname)
	{
	  SyntaxService->ReportError (
	       "crystalspace.maploader.parse.texture",
	       child, "Expected VFS filename for 'file'!");
	  goto error;
	}
      
        east = LoadImage(fname, Format);
        break;
    
      case XMLTOKEN_WEST:
        fname = csStrNew (child->GetContentsValue ());
	if (!fname)
	{
	  SyntaxService->ReportError (
	       "crystalspace.maploader.parse.texture",
	       child, "Expected VFS filename for 'file'!");
	  goto error;
	}
      
        west = LoadImage(fname, Format);
        break;
    
      case XMLTOKEN_TOP:
        fname = csStrNew (child->GetContentsValue ());
	if (!fname)
	{
	  SyntaxService->ReportError (
	       "crystalspace.maploader.parse.texture",
	       child, "Expected VFS filename for 'file'!");
	  goto error;
	}
      
        top = LoadImage(fname, Format);
        break;
    
      case XMLTOKEN_BOTTOM:
        fname = csStrNew (child->GetContentsValue ());
	if (!fname)
	{
	  SyntaxService->ReportError (
	       "crystalspace.maploader.parse.texture",
	       child, "Expected VFS filename for 'file'!");
	  goto error;
	}
      
        bottom = LoadImage(fname, Format);
        break;
    }
  }

  cubetextures->AddImage (north);
  cubetextures->AddImage (south);
  cubetextures->AddImage (east);
  cubetextures->AddImage (west);
  cubetextures->AddImage (top);
  cubetextures->AddImage (bottom);

  itex = tm->RegisterTexture(cubetextures,0,
  	iTextureHandle::CS_TEX_IMG_CUBEMAP);

  itexwrap = Engine->GetTextureList()->NewTexture(itex);
  itexwrap->QueryObject()->SetName(texname);

  delete[] fname;
  return itexwrap;

error:
  delete[] fname;
  return 0;
}

iTextureWrapper* csLoader::ParseTexture3D (iLoaderContext* ldr_context,
    iDocumentNode* node)
{
   csRef<csImageVector> cubetextures = new csImageVector();

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  iTextureManager* tm = G3D ? G3D->GetTextureManager() : 0;
  int Format = tm ? tm->GetTextureFormat () : CS_IMGFMT_TRUECOLOR;
  
  csRef<iImageVector> imagevec = new csImageVector();
  csRef<iImage> image;

  const char* texname = node->GetAttributeValue ("name");

  int imagecount = 0;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LAYER:
      {
        const char* fname = child->GetContentsValue ();
	if (!fname)
	{
	  SyntaxService->ReportError (
	       "crystalspace.maploader.parse.texture",
	       child, "Expected VFS filename for 'file'!");
	  return 0;
	}
        image = LoadImage(fname, Format);
        imagevec->AddImage(image);
        imagecount++;
        break;
      }
    }
  }
  
  csRef<iTextureHandle> itex (tm->RegisterTexture(imagevec,0,
  	iTextureHandle::CS_TEX_IMG_3D));

  csRef<iTextureWrapper> itexwrap = Engine->GetTextureList()->NewTexture(itex);
  itexwrap->QueryObject()->SetName(texname);

  return itexwrap;
}
#endif // CS_USE_NEW_RENDERER
