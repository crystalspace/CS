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
#include "csloader.h"
#include "csutil/scanstr.h"
#include "iutil/document.h"
#include "csgfx/rgbpixel.h"
#include "cstool/gentrtex.h"
#include "ivideo/graph3d.h"
#include "ivideo/texture.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iutil/object.h"
#include "csutil/csstring.h"
#include "csutil/databuf.h"
#include "igraphic/imageio.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/cache.h"

struct PrsHeightMapData : public iGenerateImageFunction
{
  csRef<iImage> im;
  int iw, ih;	// Image width and height.
  float w, h;	// Image width and height.
  csRGBpixel* p;
  float hscale, hshift;
  bool slope;
  bool flipx, flipy;
  SCF_DECLARE_IBASE;

  PrsHeightMapData (bool s) : slope (s)
  {
    flipx=false; flipy=false;
    SCF_CONSTRUCT_IBASE (0);
  }
  virtual ~PrsHeightMapData ()
  {
    SCF_DESTRUCT_IBASE();
  }
  float GetHeight (float dx, float dy);
  float GetSlope (float dx, float dy);
  virtual float GetValue (float dx, float dy);
};

SCF_IMPLEMENT_IBASE (PrsHeightMapData)
  SCF_IMPLEMENTS_INTERFACE (iGenerateImageFunction)
SCF_IMPLEMENT_IBASE_END

float PrsHeightMapData::GetSlope (float x, float y)
{
  float div = 0.02f;
  float mx = x - 0.01f; if (mx < 0) { mx = x; div = 0.01f; }
  float px = x + 0.01f; if (px > 1) { px = x; div = 0.01f; }
  float dhdx = GetHeight (px, y) - GetHeight (mx, y);
  dhdx /= div;
  div = 0.02f;
  float my = y - 0.01f; if (my < 0) { my = y; div = 0.01f; }
  float py = y + 0.01f; if (py > 1) { py = y; div = 0.01f; }
  float dhdy = GetHeight (x, py) - GetHeight (x, my);
  dhdy /= div;
  //csPrintf ("x=%g y=%g dhdx=%g dhdy=%g slope=%g , %g\n", x, y, dhdx, dhdy,
    //fabs((dhdx+dhdy)/2.), fabs(dhdx)/2.+fabs(dhdy)/2.); fflush (stdout);
  //return fabs ((dhdx+dhdy)/2.);
  return (fabs(dhdx)+fabs(dhdy))/2.;
}

float PrsHeightMapData::GetHeight (float x, float y)
{
  float dw = fmod (x*(w-1), 1.0f);
  float dh = fmod (y*(h-1), 1.0f);
  int ix = int (x*(w-1));
  int iy = int (y*(h-1));
  int idx = iy * iw + ix;
  float col00, col01, col10, col11;
  col00 = float (p[idx].red + p[idx].green + p[idx].blue)/3.;
  if (ix < iw-1)
    col10 = float (p[idx+1].red + p[idx+1].green + p[idx+1].blue)/3.;
  else
    col10 = col00;
  if (iy < ih-1)
    col01 = float (p[idx+iw].red + p[idx+iw].green + p[idx+iw].blue)/3.;
  else
    col01 = col00;
  if (ix < iw-1 && iy < ih-1)
    col11 = float (p[idx+iw+1].red + p[idx+iw+1].green + p[idx+iw+1].blue)/3.;
  else
    col11 = col00;
  float col0010 = col00 * (1-dw) + col10 * dw;
  float col0111 = col01 * (1-dw) + col11 * dw;
  float col = col0010 * (1-dh) + col0111 * dh;
  return col * hscale + hshift;
}

float PrsHeightMapData::GetValue (float x, float y)
{
  if(flipx) x = 1.f-x;
  if(flipy) y = 1.f-y;
  if (slope) return GetSlope (x, y);
  else return GetHeight (x, y);
}

csGenerateImageValue* csLoader::ParseHeightgenValue (iDocumentNode* node)
{
  csGenerateImageValue* v = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_CONSTANT:
        {
	  csGenerateImageValueFuncConst* vt =
	  	new csGenerateImageValueFuncConst ();
	  vt->constant = child->GetContentsValueAsFloat ();
	  v = vt;
	}
	break;
      case XMLTOKEN_HEIGHTMAP:
        {
	  csGenerateImageValueFunc* vf = new csGenerateImageValueFunc ();
	  float hscale = 1, hshift = 0;
	  bool flipx = false, flipy = false;
	  csRef<iDocumentNode> imagenode = child->GetNode ("image");
	  if (!imagenode)
	  {
	    ReportError (
	      "crystalspace.maploader.parse.heightgen",
              "Expected 'image' token in 'heightmap'!");
	  }
	  const char* heightmap = imagenode->GetContentsValue ();;
	  csRef<iDocumentNode> scalenode = child->GetNode ("scale");
	  if (scalenode)
	    hscale = scalenode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> shiftnode = child->GetNode ("shift");
	  if (shiftnode)
	    hshift = shiftnode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> flipxnode = child->GetNode ("flipx");
	  if(flipxnode)
	    if(!SyntaxService->ParseBool(flipxnode, flipx, true))
	    {
	      ReportError (
	        "crystalspace.maploader.parse.heightgen",
                "bad flipx argument.");
	      return 0;
	    }
	  csRef<iDocumentNode> flipynode = child->GetNode ("flipy");
	  if(flipynode)
	    if(!SyntaxService->ParseBool(flipynode, flipy, true))
	    {
	      ReportError (
	        "crystalspace.maploader.parse.heightgen",
                "bad flipy argument.");
	      return 0;
	    }

	  csRef<iImage> img (LoadImage (heightmap, CS_IMGFMT_TRUECOLOR));
	  if (!img) return 0;
	  PrsHeightMapData* data = new PrsHeightMapData (false);
  	  data->im = img;
  	  data->iw = img->GetWidth ();
  	  data->ih = img->GetHeight ();
  	  data->w = float (data->iw);
  	  data->h = float (data->ih);
  	  data->p = (csRGBpixel*)(img->GetImageData ());
  	  data->hscale = hscale;
  	  data->hshift = hshift;
	  data->flipx = flipx;
	  data->flipy = flipy;
	  vf->SetFunction (data);
	  data->DecRef ();
	  v = vf;
	}
	break;
      case XMLTOKEN_SLOPE:
        {
	  csGenerateImageValueFunc* vf = new csGenerateImageValueFunc ();
	  float hscale = 1, hshift = 0;
	  bool flipx = false, flipy = false;
	  csRef<iDocumentNode> imagenode = child->GetNode ("image");
	  if (!imagenode)
	  {
	    ReportError (
	      "crystalspace.maploader.parse.heightgen",
              "Expected 'image' token in 'slope'!");
	  }
	  const char* heightmap = imagenode->GetContentsValue ();;
	  csRef<iDocumentNode> scalenode = child->GetNode ("scale");
	  if (scalenode)
	    hscale = scalenode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> shiftnode = child->GetNode ("shift");
	  if (shiftnode)
	    hshift = shiftnode->GetContentsValueAsFloat ();
	  csRef<iDocumentNode> flipxnode = child->GetNode ("flipx");
	  if(flipxnode)
	    if(!SyntaxService->ParseBool(flipxnode, flipx, true))
	    {
	      ReportError (
	        "crystalspace.maploader.parse.heightgen",
                "bad flipx argument.");
	      return 0;
	    }
	  csRef<iDocumentNode> flipynode = child->GetNode ("flipy");
	  if(flipynode)
	    if(!SyntaxService->ParseBool(flipynode, flipy, true))
	    {
	      ReportError (
	        "crystalspace.maploader.parse.heightgen",
                "bad flipy argument.");
	      return 0;
	    }
	  csRef<iImage> img (LoadImage (heightmap, CS_IMGFMT_TRUECOLOR));
	  if (!img) return 0;
	  PrsHeightMapData* data = new PrsHeightMapData (true);
  	  data->im = img;
  	  data->iw = img->GetWidth ();
  	  data->ih = img->GetHeight ();
  	  data->w = float (data->iw);
  	  data->h = float (data->ih);
  	  data->p = (csRGBpixel*)(img->GetImageData ());
  	  data->hscale = hscale;
  	  data->hshift = hshift;
	  data->flipx = flipx;
	  data->flipy = flipy;
	  vf->SetFunction (data);
	  data->DecRef ();
	  v = vf;
	}
	break;
      case XMLTOKEN_TEXTURE:
	{
	  csGenerateImageValueFuncTex* vf = new csGenerateImageValueFuncTex ();
	  vf->tex = ParseHeightgenTexture (child);
	  v = vf;
	}
	break;
      default:
	SyntaxService->ReportBadToken (child);
	return 0;
    }
  }
  return v;
}

csGenerateImageTexture* csLoader::ParseHeightgenTexture (iDocumentNode* node)
{
  csGenerateImageTexture* t = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SOLID:
        {
	  csColor col;
          if (!SyntaxService->ParseColor (child, col))
	    return 0;
	  csGenerateImageTextureSolid* ts = new csGenerateImageTextureSolid ();
	  ts->color = col;
	  t = ts;
	}
	break;
      case XMLTOKEN_SINGLE:
        {
	  csRef<iDocumentNode> imagenode = child->GetNode ("image");
	  if (!imagenode)
	  {
	    ReportError (
		    "crystalspace.maploader.parse.heightgen",
		    "No 'image' child specified for 'single'!");
	    return 0;
	  }
	  const char* imagename = imagenode->GetContentsValue ();
	  csVector2 scale (1, 1), offset (0, 0);
	  csRef<iDocumentNode> scalenode = child->GetNode ("scale");
	  if (scalenode)
	  {
	    scale.x = scalenode->GetAttributeValueAsFloat ("x");
	    scale.y = scalenode->GetAttributeValueAsFloat ("y");
	  }
	  csRef<iDocumentNode> offsetnode = child->GetNode ("offset");
	  if (offsetnode)
	  {
	    offset.x = offsetnode->GetAttributeValueAsFloat ("x");
	    offset.y = offsetnode->GetAttributeValueAsFloat ("y");
	  }

	  csRef<iImage> img (LoadImage (imagename, CS_IMGFMT_TRUECOLOR));
	  if (!img) return 0;
	  csGenerateImageTextureSingle* ts =
	  	new csGenerateImageTextureSingle ();
	  ts->SetImage (img);
	  ts->scale = scale;
	  ts->offset = offset;
	  t = ts;
	}
	break;
      case XMLTOKEN_BLEND:
        {
	  csGenerateImageTextureBlend* tb = new csGenerateImageTextureBlend ();
	  csRef<iDocumentNodeIterator> blend_it = child->GetNodes ();
	  while (blend_it->HasNext ())
	  {
	    csRef<iDocumentNode> blend_child = blend_it->Next ();
	    if (blend_child->GetType () != CS_NODE_ELEMENT) continue;
	    const char* blend_value = blend_child->GetValue ();
	    csStringID id = xmltokens.Request (blend_value);
	    switch (id)
	    {
	      case XMLTOKEN_VALUE:
	        tb->valuefunc = ParseHeightgenValue (blend_child);
		if (!tb->valuefunc)
		{
		  ReportError (
		    "crystalspace.maploader.parse.heightgen",
		    "Problem with returned value!");
		  return 0;
		}
	        break;
	      case XMLTOKEN_LAYER:
	        {
		  float height = 0;
		  csGenerateImageTexture* txt = 0;
		  csRef<iDocumentNode> texturenode = blend_child->GetNode (
		  	"texture");
		  if (!texturenode)
		  {
		    ReportError (
		      "crystalspace.maploader.parse.heightgen",
		      "No 'texture' specified inside 'layer'!");
		    return 0;
		  }
	          txt = ParseHeightgenTexture (texturenode);
		  if (!txt)
		  {
		    ReportError (
			    "crystalspace.maploader.parse.heightgen",
			    "Problem with returned texture!");
		    return 0;
		  }
		  csRef<iDocumentNode> heightnode = blend_child->GetNode (
		  	"height");
		  if (heightnode)
		  {
		    height = heightnode->GetContentsValueAsFloat ();
		  }
		  tb->AddLayer (height, txt);
	  	}
	        break;
	      default:
		SyntaxService->ReportBadToken (blend_child);
		return 0;
	    }
	  }
	  t = tb;
	}
	break;
      default:
	SyntaxService->ReportBadToken (child);
	return 0;
    }
  }
  if (!t)
  {
    ReportError (
	    "crystalspace.maploader.parse.heightgen",
	    "Problem with texture specification!");
  }
  return t;
}

bool csLoader::ParseHeightgen (iLoaderContext* ldr_context, iDocumentNode* node)
{
  int totalw = 256, totalh = 256;
  int partw = 64, parth = 64;
  int mw = 1, mh = 1;
  csGenerateImage* gen = new csGenerateImage ();

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SIZE:
	totalw = child->GetAttributeValueAsInt ("w");
	totalh = child->GetAttributeValueAsInt ("h");
	break;
      case XMLTOKEN_MULTIPLY:
	mw = child->GetAttributeValueAsInt ("w");
	mh = child->GetAttributeValueAsInt ("h");
	break;
      case XMLTOKEN_PARTSIZE:
	partw = child->GetAttributeValueAsInt ("w");
	parth = child->GetAttributeValueAsInt ("h");
	break;
      case XMLTOKEN_TEXTURE:
        {
          csGenerateImageTexture* txt = ParseHeightgenTexture (child);
          gen->SetTexture (txt);
	}
	break;
      case XMLTOKEN_GENERATE:
        {
          if (!Engine || !G3D)
	    break;

	  int startx = child->GetAttributeValueAsInt ("x");
	  int starty = child->GetAttributeValueAsInt ("y");
          csRef<iCacheManager> cache = Engine?Engine->GetCacheManager():0;
          const char* cache_type = "gentex";
          const char* cache_scope = child->GetAttributeValue ("name");
          uint32 cache_id = 0;
          
          csString cache_enable = child->GetAttributeValue ("cache");
          csRef<iImage> img;
          csRef<iImageIO> imageio (CS_QUERY_REGISTRY (object_reg, iImageIO));

          // get from cache
          if(Engine && cache_enable == "yes")
          {
            csRef<iDataBuffer> data = cache->ReadCache(cache_type,
                cache_scope, cache_id);
            if(data && !imageio)
            {
	      ReportError ("crystalspace.maploader.parse.heightgen",
	       "Cannot convert cached image - no imageIO.");
            }
            if(data && imageio)
            {
              img = imageio->Load (data, Engine->GetTextureFormat());
            }
          }

          // or, generate and cache
          if(!img) 
          {
	    img = gen->Generate (totalw, totalh, startx*mw, starty*mh,
	      partw, parth);
            if(!imageio) 
            {
	      ReportError ("crystalspace.maploader.parse.heightgen",
	        "Cannot cache image, no imageIO.");
            }
            if(imageio && Engine && cache_enable == "yes")
            {
              csRef<iDataBuffer> db (imageio->Save (img, "image/png",
                  "progressive"));
              if(!db) 
              {
	        ReportError ("crystalspace.maploader.parse.heightgen",
	          "Cache Failed: Cannot convert to imagebuffer.");
              }
              if(!cache->CacheData((void*)db->GetData(), db->GetSize(),
                  cache_type, cache_scope, cache_id))
              {
	        ReportError ("crystalspace.maploader.parse.heightgen",
	          "Cache Failed: cannot save data in cache.");
              }
	      cache->Flush ();
            }
          }

          // add texture
	  csRef<iTextureHandle> TexHandle (G3D->GetTextureManager ()
	  	->RegisterTexture (img, CS_TEXTURE_3D));
	  if (!TexHandle)
	  {
	    ReportError (
	      "crystalspace.maploader.parse.heightgen",
	      "Cannot create texture!");
	    return false;
	  }
	  iTextureWrapper *TexWrapper = Engine->GetTextureList ()
	  	->NewTexture (TexHandle);
	  TexWrapper->QueryObject ()->SetName (
	  	child->GetAttributeValue ("name"));
	  AddToRegion (ldr_context, TexWrapper->QueryObject ());
	}
	break;
      default:
	SyntaxService->ReportBadToken (child);
	delete gen;
	return false;
    }
  }

  delete gen;
  return true;
}

