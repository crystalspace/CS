/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#include "csutil/csmd5.h"
#include "csutil/ref.h"
#include "csgfx/memimage.h"
#include "csgfx/shaderexp.h"
#include "csgfx/shadervarcontext.h"

#include "iengine/engine.h"
#include "iengine/texture.h"
#include "igraphic/imageio.h"
#include "iutil/cache.h"
#include "iutil/document.h"
#include "iutil/strset.h"
#include "imap/reader.h"
#include "imap/services.h"
#include "itexture/itexloaderctx.h"
#include "ivaria/reporter.h"
#include "ivideo/texture.h"
#include "ivideo/txtmgr.h"

#include "tlfunc.h"

CS_LEAKGUARD_IMPLEMENT (csFuncTexLoader);

SCF_IMPLEMENT_IBASE(csFuncTexLoader);
  SCF_IMPLEMENTS_INTERFACE(iLoaderPlugin);
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

SCF_IMPLEMENT_EMBEDDED_IBASE (csFuncTexLoader::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY(csFuncTexLoader);

//---------------------------------------------------------------------------

csFuncTexLoader::csFuncTexLoader (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  InitTokenTable (tokens);
}

csFuncTexLoader::~csFuncTexLoader()
{
  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE();
}

bool csFuncTexLoader::Initialize(iObjectRegistry *object_reg)
{
  csFuncTexLoader::object_reg = object_reg;
  return true;
}

static void CrudeDocumentFlattener (iDocumentNode* node, csString& str)
{
  str << node->GetValue ();
  csRef<iDocumentAttributeIterator> attrIter = node->GetAttributes ();
  if (attrIter)
  {
    str << '[';
    while (attrIter->HasNext())
    {
      csRef<iDocumentAttribute> attr = attrIter->Next();
      str << attr->GetName () << '=' << attr->GetValue() << ',';
    }
    str << ']';
  }
  str << '(';
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () == CS_NODE_COMMENT) continue;
    CrudeDocumentFlattener (child, str);
    str << ',';
  }
  str << ')';
}

csPtr<iBase> csFuncTexLoader::Parse (iDocumentNode* node, 
				     iLoaderContext* ldr_context,
				     iBase* context)
{
  csRef<iSyntaxService> synldr = 
    CS_QUERY_REGISTRY (object_reg, iSyntaxService);

  int w = 256, h = 256;
  csRef<iTextureLoaderContext> ctx;
  if (context)
  {
    ctx = csPtr<iTextureLoaderContext>
      (SCF_QUERY_INTERFACE (context, iTextureLoaderContext));
    if (ctx) 
    {
      if (ctx->HasSize())
      {
	ctx->GetSize (w, h);
      }
    }
  }

  csRef<iDocumentNode> exprNode;
  if (node)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      csStringID id = tokens.Request (child->GetValue ());
      switch (id)
      {
	case XMLTOKEN_EXPRESSION:
	  exprNode = child; 
	  break;
	default:
	  if (synldr) synldr->ReportBadToken (child);
	  return 0;
      }
    }
  }

  // Get the first child node of the <expression> node
  if (exprNode)
  {
    csRef<iDocumentNodeIterator> it = exprNode->GetNodes ();
    exprNode = 0;
    while (it->HasNext())
    {
      csRef<iDocumentNode> newNode = it->Next();
      if (newNode->GetType() == CS_NODE_COMMENT) continue;
      exprNode = newNode;
      break;
    }
    while (it->HasNext())
    {
      csRef<iDocumentNode> newNode = it->Next();
      if (newNode->GetType() != CS_NODE_COMMENT)
      {
	synldr->Report ("crystalspace.texture.loader.func",
	  CS_REPORTER_SEVERITY_WARNING,
	  exprNode,
	  "Subsequent expressions are ignored");
	break;
      }
    }
  }
  else
  {
    synldr->Report ("crystalspace.texture.loader.func",
      CS_REPORTER_SEVERITY_WARNING,
      node,
      "No expression found");
  }

  // Cache stuff
  csRef<iImage> Image;
  csRef<iEngine> Engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!Engine)
    return 0;
  const char* cache_type = "tlfunc";
  csString cache_scope (ctx->GetName ());
  csRef<iCacheManager> cache = Engine->GetCacheManager();
  csRef<iImageIO> imageio (CS_QUERY_REGISTRY (object_reg, iImageIO));
  bool do_cache = imageio && cache && cache_scope;

  if (exprNode)
  {
    csString flattened;
    CrudeDocumentFlattener (exprNode, flattened);

    csMD5::Digest md5 (csMD5::Encode (flattened));
    cache_scope << md5.HexString ();
  }

  if (do_cache)
  {
    csRef<iDataBuffer> data = cache->ReadCache(cache_type,
      cache_scope, ~0);

    if (data)
    {
      Image = imageio->Load (data, Engine->GetTextureFormat());
    }
    do_cache = !Image.IsValid();
  }
  
  if (!Image.IsValid())
  {
    csRGBpixel* pixdata = new csRGBpixel[w * h];

    if (exprNode)
    {
      csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
	object_reg, "crystalspace.shared.stringset", iStringSet);

      csShaderExpression expr (object_reg);
      
      csRef<iShaderVariableContext> context;
      context.AttachNew (new csShaderVariableContext ());
      csRef<csShaderVariable> currentPos;
      currentPos.AttachNew (new csShaderVariable (strings->Request ("position")));
      context->AddVariable (currentPos);

      if (expr.Parse (exprNode, context))
      {
	csRef<csShaderVariable> result;
	result.AttachNew (new csShaderVariable (csInvalidStringID));
	result->SetType (csShaderVariable::VECTOR4);
	for (int y = 0; y < h; y++)
	{
	  csRGBpixel* line = pixdata + (y * w);
	  float fY = (float)y / (float)h;
	  for (int x = 0; x < w; x++)
	  {
	    currentPos->SetValue (csVector2 ((float)x / (float)w, fY));
	    if (expr.Evaluate (result))
	    {
	      csVector4 v;
	      result->GetValue (v);
	      line->red   = csQint (v.x * 255.99f);
	      line->green = csQint (v.y * 255.99f);
	      line->blue  = csQint (v.z * 255.99f);
	      line->alpha = csQint (v.w * 255.99f);
	    }
	    line++;
	  }
	}
      }
      else
	synldr->Report ("crystalspace.texture.loader.func",
	  CS_REPORTER_SEVERITY_WARNING,
	  exprNode,
	  "Error parsing expression");
    }

    Image.AttachNew (new csImageMemory (w, h, pixdata, true));
  }

  if (do_cache)
  {
    csRef<iDataBuffer> data = imageio->Save (Image, "image/png");
    if (!data.IsValid())
      data = imageio->Save (Image, "image/tga");
    if (data.IsValid())
    {
      cache->CacheData (data->GetData(), data->GetSize(),
	cache_type, cache_scope, ~0);
      cache->Flush ();
    }
  }

  csRef<iGraphics3D> G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!G3D) return 0;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return 0;

  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (Image, 
    (ctx && ctx->HasFlags()) ? ctx->GetFlags() : CS_TEXTURE_3D));

  csRef<iTextureWrapper> TexWrapper =
	Engine->GetTextureList ()->NewTexture(TexHandle);
  TexWrapper->SetImageFile (Image);

  return csPtr<iBase> (TexWrapper);

}

