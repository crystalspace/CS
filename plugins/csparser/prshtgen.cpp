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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csgfx/rgbpixel.h"
#include "cstool/gentrtex.h"
#include "ivideo/graph3d.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iutil/object.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (BLEND)
  CS_TOKEN_DEF (CONSTANT)
  CS_TOKEN_DEF (GENERATE)
  CS_TOKEN_DEF (HEIGHT)
  CS_TOKEN_DEF (HEIGHTMAP)
  CS_TOKEN_DEF (LAYER)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (PARTSIZE)
  CS_TOKEN_DEF (SINGLE)
  CS_TOKEN_DEF (SIZE)
  CS_TOKEN_DEF (SLOPE)
  CS_TOKEN_DEF (SOLID)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (VALUE)
CS_TOKEN_DEF_END

struct HeightMapData : public iGenerateImageFunction
{
  iImage* im;
  int iw, ih;	// Image width and height.
  float w, h;	// Image width and height.
  csRGBpixel* p;
  float hscale, hshift;
  bool slope;
  SCF_DECLARE_IBASE;

  HeightMapData (bool s) : im (NULL), slope (s)
  {
    SCF_CONSTRUCT_IBASE (NULL);
  }
  virtual ~HeightMapData ()
  {
    if (im) im->DecRef ();
  }
  float GetHeight (float dx, float dy);
  float GetSlope (float dx, float dy);
  virtual float GetValue (float dx, float dy);
};

SCF_IMPLEMENT_IBASE (HeightMapData)
  SCF_IMPLEMENTS_INTERFACE (iGenerateImageFunction)
SCF_IMPLEMENT_IBASE_END

float HeightMapData::GetSlope (float x, float y)
{
  float div = 0.02;
  float mx = x-.01; if (mx < 0) { mx = x; div = .01; }
  float px = x+.01; if (px > 1) { px = x; div = .01; }
  float dhdx = GetHeight (px, y) - GetHeight (mx, y);
  dhdx /= div;
  div = 0.02;
  float my = y-.01; if (my < 0) { my = y; div = .01; }
  float py = y+.01; if (py > 1) { py = y; div = .01; }
  float dhdy = GetHeight (x, py) - GetHeight (x, my);
  dhdy /= div;
  //printf ("x=%g y=%g dhdx=%g dhdy=%g slope=%g , %g\n", x, y, dhdx, dhdy,
    //fabs((dhdx+dhdy)/2.), fabs(dhdx)/2.+fabs(dhdy)/2.); fflush (stdout);
  //return fabs ((dhdx+dhdy)/2.);
  return (fabs(dhdx)+fabs(dhdy))/2.;
}

float HeightMapData::GetHeight (float x, float y)
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
  //printf("Heightmap x=%g y=%g height=%g\n", x, y, col * hm->hscale + hm->hshift);
  return col * hscale + hshift;
}

float HeightMapData::GetValue (float x, float y)
{
  if (slope) return GetSlope (x, y);
  else return GetHeight (x, y);
}

csGenerateImageValue* csLoader::ParseHeightgenValue (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (CONSTANT)
    CS_TOKEN_TABLE (HEIGHTMAP)
    CS_TOKEN_TABLE (SLOPE)
    CS_TOKEN_TABLE (TEXTURE)
  CS_TOKEN_TABLE_END

  long cmd;
  char *params;
  char* name;
  csGenerateImageValue* v = NULL;

  if ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_CONSTANT:
        {
	  csGenerateImageValueFuncConst* vt =
	  	new csGenerateImageValueFuncConst ();
	  csScanStr (params, "%f", &(vt->constant));
	  v = vt;
	}
	break;
      case CS_TOKEN_HEIGHTMAP:
        {
	  csGenerateImageValueFunc* vf = new csGenerateImageValueFunc ();
	  char heightmap[255];
	  float hscale, hshift;
          csScanStr (params, "%s,%f,%f", &heightmap, &hscale, &hshift);
	  iImage* img = LoadImage (heightmap, CS_IMGFMT_TRUECOLOR);
	  if (!img) return NULL;
	  HeightMapData* data = new HeightMapData (false);
  	  data->im = img;
  	  data->iw = img->GetWidth ();
  	  data->ih = img->GetHeight ();
  	  data->w = float (data->iw);
  	  data->h = float (data->ih);
  	  data->p = (csRGBpixel*)(img->GetImageData ());
  	  data->hscale = hscale;
  	  data->hshift = hshift;
	  vf->SetFunction (data);
	  data->DecRef ();
	  v = vf;
	}
	break;
      case CS_TOKEN_SLOPE:
        {
	  csGenerateImageValueFunc* vf = new csGenerateImageValueFunc ();
	  char heightmap[255];
	  float hscale, hshift;
          csScanStr (params, "%s,%f,%f", &heightmap, &hscale, &hshift);
	  iImage* img = LoadImage (heightmap, CS_IMGFMT_TRUECOLOR);
	  if (!img) return NULL;
	  HeightMapData* data = new HeightMapData (true);
  	  data->im = img;
  	  data->iw = img->GetWidth ();
  	  data->ih = img->GetHeight ();
  	  data->w = float (data->iw);
  	  data->h = float (data->ih);
  	  data->p = (csRGBpixel*)(img->GetImageData ());
  	  data->hscale = hscale;
  	  data->hshift = hshift;
	  vf->SetFunction (data);
	  data->DecRef ();
	  v = vf;
	}
	break;
      case CS_TOKEN_TEXTURE:
	{
	  csGenerateImageValueFuncTex* vf = new csGenerateImageValueFuncTex ();
	  vf->tex = ParseHeightgenTexture (params);
	  v = vf;
	}
	break;
    }
  }
  if (!v)
  {
    ReportError (
	  "crystalspace.maploader.parse.heightgen",
          "Problem with value specification!");
  }
  return v;
}

csGenerateImageTexture* csLoader::ParseHeightgenTexture (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (SOLID)
    CS_TOKEN_TABLE (SINGLE)
    CS_TOKEN_TABLE (BLEND)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (blend_commands)
    CS_TOKEN_TABLE (VALUE)
    CS_TOKEN_TABLE (LAYER)
  CS_TOKEN_TABLE_END

  CS_TOKEN_TABLE_START (layer_commands)
    CS_TOKEN_TABLE (HEIGHT)
    CS_TOKEN_TABLE (TEXTURE)
  CS_TOKEN_TABLE_END

  long cmd;
  char *params;
  char* name;
  csGenerateImageTexture* t = NULL;

  if ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_SOLID:
        {
	  csGenerateImageTextureSolid* ts = new csGenerateImageTextureSolid ();
	  csColor col;
	  csScanStr (params, "%f,%f,%f", &col.red, &col.green, &col.blue);
	  ts->color = col;
	  t = ts;
	}
	break;
      case CS_TOKEN_SINGLE:
        {
	  char imagename[255];
	  csVector2 scale, offset;
          csScanStr (params, "%s,%f,%f,%f,%f",
		imagename, &scale.x, &scale.y,
		&offset.x, &offset.y);
	  iImage* img = LoadImage (imagename, CS_IMGFMT_TRUECOLOR);
	  if (!img) return NULL;
	  csGenerateImageTextureSingle* ts =
	  	new csGenerateImageTextureSingle ();
	  ts->SetImage (img);
	  ts->scale = scale;
	  ts->offset = offset;
	  t = ts;
	}
	break;
      case CS_TOKEN_BLEND:
        {
	  csGenerateImageTextureBlend* tb = new csGenerateImageTextureBlend ();
	  char* xname;
	  char* params2;
	  while ((cmd = csGetObject (&params, blend_commands,
	  	&xname, &params2)) > 0)
	  {
	    switch (cmd)
	    {
	      case CS_TOKEN_VALUE:
	        tb->valuefunc = ParseHeightgenValue (params2);
		if (!tb->valuefunc)
		{
		  ReportError (
		    "crystalspace.maploader.parse.heightgen",
		    "Problem with returned value!");
		  return NULL;
		}
	        break;
	      case CS_TOKEN_LAYER:
	        {
		  float height = 0;
		  csGenerateImageTexture* txt = NULL;
	  	  char* yname;
	  	  char* params3;
	  	  while ((cmd = csGetObject (&params2, layer_commands,
	  		  &yname, &params3)) > 0)
	  	  {
	    	    switch (cmd)
	    	    {
	      	      case CS_TOKEN_TEXTURE:
	        	txt = ParseHeightgenTexture (params3);
			if (!tb->valuefunc)
			{
			  ReportError (
			    "crystalspace.maploader.parse.heightgen",
			    "Problem with returned texture!");
			  return NULL;
			}
	        	break;
	      	      case CS_TOKEN_HEIGHT:
		        csScanStr (params3, "%f", &height);
	        	break;
	    	    }
		  }
		  tb->AddLayer (height, txt);
	  	}
	        break;
	    }
	  }
	  t = tb;
	}
	break;
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

bool csLoader::ParseHeightgen (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (GENERATE)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (SIZE)
    CS_TOKEN_TABLE (PARTSIZE)
    CS_TOKEN_TABLE (MULTIPLY)
  CS_TOKEN_TABLE_END

  long cmd;
  char *params;
  char* name;
  int totalw = 256, totalh = 256;
  int partw = 64, parth = 64;
  int mw = 1, mh = 1;
  csGenerateImage* gen = new csGenerateImage ();

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_SIZE:
	csScanStr (params, "%d,%d", &totalw, &totalh);
	break;
      case CS_TOKEN_MULTIPLY:
	csScanStr (params, "%d,%d", &mw, &mh);
	break;
      case CS_TOKEN_PARTSIZE:
	csScanStr (params, "%d,%d", &partw, &parth);
	break;
      case CS_TOKEN_TEXTURE:
        {
          csGenerateImageTexture* txt = ParseHeightgenTexture (params);
          gen->SetTexture (txt);
	}
	break;
      case CS_TOKEN_GENERATE:
        {
	  int startx, starty;
	  csScanStr (params, "%d,%d", &startx, &starty);
	  iImage* img = gen->Generate (totalw, totalh, startx*mw, starty*mh,
	  	partw, parth);
	  iTextureHandle *TexHandle = G3D->GetTextureManager ()
	  	->RegisterTexture (img, CS_TEXTURE_3D);
	  if (!TexHandle)
	  {
	    ReportError (
	      "crystalspace.maploader.parse.heightgen",
	      "Cannot create texture!");
	    return false;
	  }
	  iTextureWrapper *TexWrapper = Engine->GetTextureList ()
	  	->NewTexture (TexHandle);
	  TexWrapper->QueryObject ()->SetName (name);
	}
	break;
    }
  }

  delete gen;
  //@@@ Memory leak!
  //if (data)
  //{
    //data->im->DecRef ();
    //delete data;
  //}

  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    TokenError ("a heightgen specification");
    return false;
  }
  return true;
}


