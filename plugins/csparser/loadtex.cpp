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

/* This file contains functions to load textures from image files. They do
 * not handle parsing of texture statements in any way.
 */

#include <ctype.h>
#include "cssysdef.h"
#include "csloader.h"
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "iengine/engine.h"
#include "iengine/texture.h"
#include "iengine/material.h"
#include "ivideo/graph3d.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "imap/reader.h"
#include "csgfx/xorpat.h"

#include "loadtex.h"

csPtr<iImage> csLoader::LoadImage (const char* fname, int Format)
{
  if (!ImageLoader)
     return NULL;

  if (Format & CS_IMGFMT_INVALID)
  {
    if (Engine)
      Format = Engine->GetTextureFormat ();
    else if (G3D)
      Format = G3D->GetTextureManager()->GetTextureFormat();
    else
      Format = CS_IMGFMT_TRUECOLOR;
  }

  csRef<iDataBuffer> buf (VFS->ReadFile (fname, false));
  if (!buf || !buf->GetSize ())
  {
    ReportWarning (
	"crystalspace.maploader.parse.image",
    	"Could not open image file '%s' on VFS!", fname);
    return NULL;
  }

  // we don't use csRef because we need to return an Increfed object later
  csRef<iImage> image (
    ImageLoader->Load (buf->GetUint8 (), buf->GetSize (), Format));
  if (!image)
  {
    ReportWarning (
	"crystalspace.maploader.parse.image",
	"Could not load image '%s'. Unknown format!",
	fname);
    return NULL;
  }
  
  csRef<iDataBuffer> xname (VFS->ExpandPath (fname));
  image->SetName (**xname);

  return csPtr<iImage> (image);
}

csPtr<iTextureHandle> csLoader::LoadTexture (const char *fname, int Flags,
	iTextureManager *tm, iImage **img)
{
  if (!tm && G3D)
  {
    tm = G3D->GetTextureManager();
  }
  int Format;
  if (tm)
    Format = tm->GetTextureFormat ();
  else
    Format = CS_IMGFMT_TRUECOLOR;

  csRef<iImage> Image = LoadImage (fname, Format);
  if (!Image)
  {
    ReportWarning (
	"crystalspace.maploader.parse.texture",
	"Couldn't load image '%s', using checkerboard instead!",
	fname);
    Image = csCreateXORPatternImage (32, 32, 5);
    if (!Image)
      return NULL;
  }

  if(img) *img=Image;

  if (!tm)
    return NULL;
  
  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (Image, Flags));
  if (!TexHandle)
  {
    ReportError (
	"crystalspace.maploader.parse.texture",
	"Cannot create texture from '%s'!", fname);
    return NULL;
  }

  return csPtr<iTextureHandle> (TexHandle);
}

iTextureWrapper* csLoader::LoadTexture (const char *name,
	const char *fname, int Flags, iTextureManager *tm, bool reg,
	bool create_material)
{
  if (!Engine)
    return NULL;

  iImage *img;
  csRef<iTextureHandle> TexHandle (LoadTexture (fname, Flags, tm, &img));
  if (!TexHandle)
    return NULL;

  iTextureWrapper *TexWrapper =
	Engine->GetTextureList ()->NewTexture(TexHandle);
  TexWrapper->QueryObject ()->SetName (name);
  TexWrapper->SetImageFile(img);

  iMaterialWrapper* matwrap = NULL;
  if (create_material)
  {
    csRef<iMaterial> material (Engine->CreateBaseMaterial (TexWrapper));
    matwrap = Engine->GetMaterialList ()->NewMaterial (material);
    matwrap->QueryObject ()->SetName (name);
  }

  if (reg && tm)
  {
    TexWrapper->Register (tm);
    TexWrapper->GetTextureHandle()->Prepare ();
    if (matwrap)
    {
      matwrap->Register (tm);
      matwrap->GetMaterialHandle ()->Prepare ();
    }
  }

  return TexWrapper;
}

//----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(TextureLoaderContext);
  SCF_IMPLEMENTS_INTERFACE(iTextureLoaderContext);
SCF_IMPLEMENT_IBASE_END;

TextureLoaderContext::TextureLoaderContext ()
{
  SCF_CONSTRUCT_IBASE (NULL);

  has_flags = false;
  flags = CS_TEXTURE_3D;

  has_image = false;
  image = NULL;

  has_size = false;
  width = height = 128;
}

void TextureLoaderContext::SetFlags (int Flags)
{
  flags = Flags;
  has_flags = true;
}

bool TextureLoaderContext::HasFlags ()
{
  return has_flags;
}

int TextureLoaderContext::GetFlags ()
{
  return flags;
}
  
void TextureLoaderContext::SetImage (iImage* Image)
{
  image = Image;
  has_image = true;
}

bool TextureLoaderContext::HasImage ()
{
  return has_image;
}

iImage* TextureLoaderContext::GetImage()
{
  return image;
}

void TextureLoaderContext::SetSize (int w, int h)
{
  has_size = true;
  width = w; height = h;
}

bool TextureLoaderContext::HasSize ()
{
  return has_size;
}

void TextureLoaderContext::GetSize (int& w, int& h)
{
  w = width; h = height;
}

//----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csBaseTextureLoader);
  SCF_IMPLEMENTS_INTERFACE(iLoaderPlugin);
  SCF_IMPLEMENTS_INTERFACE(iComponent);
SCF_IMPLEMENT_IBASE_END;

csBaseTextureLoader::csBaseTextureLoader (iBase *p)
{
  SCF_CONSTRUCT_IBASE (p);
}

csBaseTextureLoader::~csBaseTextureLoader ()
{
}

bool csBaseTextureLoader::Initialize(iObjectRegistry *object_reg)
{
  csBaseTextureLoader::object_reg = object_reg;
  return true;
}

//----------------------------------------------------------------------------

csImageTextureLoader::csImageTextureLoader (iBase *p) : 
  csBaseTextureLoader(p)
{
}

csPtr<iBase> csImageTextureLoader::Parse (iDocumentNode* node, 
					  iLoaderContext* ldr_context, 	
					  iBase* context)
{
  if (!context) return NULL;
  csRef<iTextureLoaderContext> ctx = csPtr<iTextureLoaderContext>
    (SCF_QUERY_INTERFACE (context, iTextureLoaderContext));
  if (!ctx) return NULL;
  if (!ctx->HasImage() || !ctx->GetImage())
    return NULL;

  csRef<iGraphics3D> G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!G3D) return NULL;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return NULL;
  csRef<iEngine> Engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!Engine)
    return NULL;

  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (ctx->GetImage(), 
    ctx->HasFlags() ? ctx->GetFlags() : CS_TEXTURE_3D));

  csRef<iTextureWrapper> TexWrapper =
	Engine->GetTextureList ()->NewTexture(TexHandle);
  TexWrapper->SetImageFile(ctx->GetImage());

  return csPtr<iBase> (TexWrapper);
}

//----------------------------------------------------------------------------

csCheckerTextureLoader::csCheckerTextureLoader (iBase *p) : 
  csBaseTextureLoader(p)
{
}

csPtr<iBase> csCheckerTextureLoader::Parse (iDocumentNode* node, 
					    iLoaderContext* ldr_context, 	
					    iBase* context)
{
  int w = 64, h = 64, depth = 6;
  csColor color;
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
	int a = csLog2 (w), b = csLog2 (h);
	depth = MIN (a, b);
	depth = MIN (depth, 8);
      }
    }
  }
  if (node)
  {
    csRef<iDocumentNode> depthNode = node->GetNode ("depth");
    if (depthNode)
    {
      depth = depthNode->GetContentsValueAsInt ();
    }
    csRef<iDocumentNode> colorNode = node->GetNode ("color");
    if (colorNode)
    {
      csRef<iSyntaxService> synserv = 
	CS_QUERY_REGISTRY (object_reg, iSyntaxService);
      if (synserv)
      {
	synserv->ParseColor (colorNode, color);
      }
    }
  }

  csRef<iImage> Image;
  Image.AttachNew (csCreateXORPatternImage (w, h, depth, color.red, color.green,
    color.blue));

  csRef<iGraphics3D> G3D = CS_QUERY_REGISTRY (object_reg, iGraphics3D);
  if (!G3D) return NULL;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return NULL;
  csRef<iEngine> Engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  if (!Engine)
    return NULL;

  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (Image, 
    (ctx && ctx->HasFlags()) ? ctx->GetFlags() : CS_TEXTURE_3D));

  csRef<iTextureWrapper> TexWrapper =
	Engine->GetTextureList ()->NewTexture(TexHandle);
  TexWrapper->SetImageFile(ctx->GetImage());

  return csPtr<iBase> (TexWrapper);
}
