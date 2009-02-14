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

#include "cssysdef.h"

#include "ivideo/graph3d.h"

#include "csqint.h"
#include "csgeom/math.h"
#include "csgfx/imagecubemapmaker.h"
#include "csgfx/imagememory.h"
#include "csgfx/imagevolumemaker.h"
#include "csgfx/rgbpixel.h"
#include "csgfx/shadervar.h"
#include "csgfx/xorpat.h"
#include "cstool/unusedresourcehelper.h"
#include "csutil/cscolor.h"
#include "csutil/scfstr.h"
#include "iengine/collection.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "iengine/texture.h"
#include "igraphic/animimg.h"
#include "igraphic/image.h"
#include "igraphic/imageio.h"
#include "imap/reader.h"
#include "itexture/iproctex.h"
#include "iutil/databuff.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"

#include "csloader.h"
#include "csthreadedloader.h"
#include "loadtex.h"

CS_PLUGIN_NAMESPACE_BEGIN(csparser)
{

static void ReportError (iObjectRegistry* object_reg,
    const char* id, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR, id, description, arg);
  va_end (arg);
}

csPtr<iImage> GenerateErrorTexture (int width, int height)
{
  static const csRGBpixel colorTable[] = 
  {csRGBpixel (0,0,0,255), csRGBpixel (255,0,0,255),
  csRGBpixel (0,255,0,255), csRGBpixel (0,0,255,255)};

  size_t colorIndex = 0;

  csRef<csImageMemory> image; 
  image.AttachNew (new csImageMemory (width, height));
  csRGBpixel *pixel = (csRGBpixel*)image->GetImagePtr();
  for (int y = 0; y < height; y+=4)
  {
    for (int y2 = 0; y2 < 4; ++y2)
    {
      for (int x = 0; x < width; x+=4)
      {
        for (int x2 = 0; x2 < 4; ++x2)
        {
          *pixel++ = colorTable[colorIndex];
        }

        colorIndex ^= 0x1;// Flip lowest bit 
      }
    }
    colorIndex ^= 0x2; // Flip higher bit
  }

  return csPtr<iImage> (image);
}

csPtr<iImage> csThreadedLoader::LoadImage (iDataBuffer* buf, const char* fname,
                                           int Format, bool do_verbose)
{
  if (!ImageLoader)
    return 0;

  if (Format & CS_IMGFMT_INVALID)
  {
    if (Engine)
      Format = Engine->GetTextureFormat ();
    else if (g3d)
      Format = g3d->GetTextureManager()->GetTextureFormat();
    else
      Format = CS_IMGFMT_TRUECOLOR;
  }

  if (!buf || !buf->GetSize ())
  {
    ReportWarning (
      "crystalspace.maploader.parse.image",
      "Could not open image file '%s' on VFS!", fname ? fname : "<unknown>");
    return 0;
  }

  // we don't use csRef because we need to return an Increfed object later
  csRef<iImage> image (ImageLoader->Load (buf, Format));
  if (!image)
  {
    ReportWarning (
      "crystalspace.maploader.parse.image",
      "Could not load image '%s'. Unknown format!",
      fname ? fname : "<unknown>");
    return 0;
  }

  if (fname)
  {
    csRef<iDataBuffer> xname = vfs->ExpandPath (fname);
    image->SetName (**xname);
  }

  return csPtr<iImage> (image);
}

THREADED_CALLABLE_IMPL3(csThreadedLoader, LoadImage, const char* fname, int Format, bool do_verbose)
{
  csRef<iDataBuffer> buf = vfs->ReadFile (fname, false);
  csRef<iImage> image = LoadImage (buf, fname, Format, do_verbose);
  if(image.IsValid())
  {
    ret->SetResult(csRef<iBase>(image));
    return true;
  }
  return false;
}

THREADED_CALLABLE_IMPL3(csThreadedLoader, LoadImage, iDataBuffer* buf, int Format, bool do_verbose)
{
  csRef<iImage> image = LoadImage (buf, 0, Format, do_verbose);
  if(image.IsValid())
  {
    ret->SetResult(csRef<iBase>(image));
    return true;
  }
  return false;
}

THREADED_CALLABLE_IMPL5(csThreadedLoader, LoadTexture, const char* fname,
  int Flags, csRef<iTextureManager> texman, csRef<iImage>* image, bool do_verbose)
{
  if (!texman && g3d)
  {
    texman = g3d->GetTextureManager();
  }

  int Format;
  if (texman)
  {
    Format = texman->GetTextureFormat ();
  }
  else
  {
    Format = CS_IMGFMT_TRUECOLOR;
  }

  csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
  LoadImageTC (itr, fname, Format, do_verbose);
  csRef<iImage> Image = scfQueryInterface<iImage>(itr->GetResultRefPtr());
  if (!Image)
  {
    ReportWarning ("crystalspace.maploader.parse.texture",
      "Couldn't load image '%s', using error texture instead!", fname);
    Image = GenerateErrorTexture (32, 32);
    if (!Image)
    {
      return false;
    }
  }

  if(image)
  {
    *image=Image;
  }

  if (!texman)
  {
    return false;
  }

  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle (texman->RegisterTexture (Image, Flags, fail_reason));
  if (!TexHandle)
  {
    ReportError ("crystalspace.maploader.parse.texture",
      "Cannot create texture from '%s': '%s'", fname, fail_reason->GetData ());
    return false;
  }

  ret->SetResult(csRef<iBase>(TexHandle));
  return true;
}

THREADED_CALLABLE_IMPL5(csThreadedLoader, LoadTexture, iDataBuffer* buf, int Flags,
                        iTextureManager* texman, csRef<iImage>* image, bool do_verbose)
{
  if (!texman && g3d)
  {
    texman = g3d->GetTextureManager();
  }

  int Format;
  if (texman)
    Format = texman->GetTextureFormat();
  else
    Format = CS_IMGFMT_TRUECOLOR;

  csRef<iImage> Image = LoadImage (buf, 0, Format, do_verbose);
  if (!Image)
  {
    ReportWarning (
      "crystalspace.maploader.parse.texture",
      "Couldn't load image. Using error texture instead!");
    //Image = csCreateXORPatternImage (32, 32, 5);
    Image = GenerateErrorTexture (32, 32);
    if (!Image)
    {
      return false;
    }
  }

  if(image)
  {
    *image=Image;
  }

  if (!texman)
  {
    return false;
  }

  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle = texman->RegisterTexture (Image, Flags, fail_reason);
  if (!TexHandle)
  {
    ReportError (
      "crystalspace.maploader.parse.texture",
      "Cannot create texture: %s",
      fail_reason->GetData ());
    return false;
  }

  ret->SetResult(csRef<iBase>(TexHandle));
  return true;
}

THREADED_CALLABLE_IMPL8(csThreadedLoader, LoadTexture, const char* Name,
                        iDataBuffer* buf, int Flags, iTextureManager* texman, bool reg, bool create_material,
                        bool free_image, bool do_verbose)
{
  if (!texman && g3d)
  {
    texman = g3d->GetTextureManager();
  }

  csRef<iImage> img;
  csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
  LoadTextureTC (itr, buf, Flags, texman, &img, do_verbose);
  if (!itr->WasSuccessful())
  {
    return false;
  }

  csRef<iTextureHandle> TexHandle = scfQueryInterface<iTextureHandle>(itr->GetResultRefPtr());
  csRef<iTextureWrapper> TexWrapper = Engine->GetTextureList()->CreateTexture(TexHandle);
  AddTextureToList(TexWrapper);
  TexWrapper->QueryObject()->SetName (Name);
  TexWrapper->SetImageFile(img);

  if (create_material)
  {
    csRef<iMaterial> material (Engine->CreateBaseMaterial (TexWrapper));
    csRef<iMaterialWrapper> matwrap = Engine->GetMaterialList ()->CreateMaterial (material, Name);
    AddMaterialToList(matwrap);
  }

  if (reg && texman)
  {
    // If we already have a texture handle then we don't register again.
    if (!TexWrapper->GetTextureHandle ())
    {
      TexWrapper->Register (texman);
    }
    if (free_image)
    {
      TexWrapper->SetImageFile (0);
    }
  }

  ret->SetResult(csRef<iBase>(TexWrapper));
  return true;
}

THREADED_CALLABLE_IMPL10(csThreadedLoader, LoadTexture, const char* name,
                        const char* FileName, int Flags, iTextureManager* texman, bool reg, bool create_material,
                        bool free_image, iCollection* collection, uint keepFlags, bool do_verbose)
{
  if (!texman && g3d)
  {
    texman = g3d->GetTextureManager();
  }

  csRef<iImage> img;
  csRef<iThreadReturn> itr = csPtr<iThreadReturn>(new csLoaderReturn(threadman));
  if (!LoadTextureTC (itr, FileName, Flags, texman, &img, do_verbose))
  {
    return false;
  }

  csRef<iTextureHandle> TexHandle = scfQueryInterface<iTextureHandle>(itr->GetResultRefPtr());
  csRef<iTextureWrapper> TexWrapper = Engine->GetTextureList ()->CreateTexture(TexHandle);
  AddTextureToList(TexWrapper);
  TexWrapper->QueryObject ()->SetName (name);
  TexWrapper->SetImageFile(img);
  if(collection)
  {
    collection->Add(TexWrapper->QueryObject());
  }

  if (create_material)
  {
    csRef<iMaterial> material (Engine->CreateBaseMaterial (TexWrapper));
    csRef<iMaterialWrapper> matwrap = Engine->GetMaterialList ()->CreateMaterial (material, name);
    AddMaterialToList(matwrap);
    if(collection)
    {
      collection->Add(matwrap->QueryObject());
    }
  }

  if (reg && texman)
  {
    // If we already have a texture handle then we don't register again.
    if (!TexWrapper->GetTextureHandle ())
    {
      TexWrapper->Register (texman);
    }
    if (free_image)
    {
      TexWrapper->SetImageFile (0);
    }
  }

  ret->SetResult(csRef<iBase>(TexWrapper));
  return true;
}

// --------------------------------------------------------------------- //

csPtr<iImage> csLoader::LoadImage (iDataBuffer* buf, const char* fname,
	int Format)
{
  if (!ImageLoader)
     return 0;

  if (Format & CS_IMGFMT_INVALID)
  {
    if (Engine)
      Format = Engine->GetTextureFormat ();
    else if (G3D)
      Format = G3D->GetTextureManager()->GetTextureFormat();
    else
      Format = CS_IMGFMT_TRUECOLOR;
  }

  if (!buf || !buf->GetSize ())
  {
    ReportWarning (
	"crystalspace.maploader.parse.image",
    	"Could not open image file '%s' on VFS!", fname ? fname : "<unknown>");
    return 0;
  }

  // we don't use csRef because we need to return an Increfed object later
  csRef<iImage> image (ImageLoader->Load (buf, Format));
  if (!image)
  {
    ReportWarning (
	"crystalspace.maploader.parse.image",
	"Could not load image '%s'. Unknown format!",
	fname ? fname : "<unknown>");
    return 0;
  }

  if (fname)
  {
    csRef<iDataBuffer> xname = VFS->ExpandPath (fname);
    image->SetName (**xname);
  }

  return csPtr<iImage> (image);
}

csPtr<iImage> csLoader::LoadImage (iDataBuffer* buf, int Format)
{
  return LoadImage (buf, 0, Format);
}

csPtr<iTextureHandle> csLoader::LoadTexture (iDataBuffer* buf, int Flags,
	iTextureManager *tm, csRef<iImage>* img)
{
  if (!tm && G3D) tm = G3D->GetTextureManager();
  int Format;
  if (tm)
    Format = tm->GetTextureFormat ();
  else
    Format = CS_IMGFMT_TRUECOLOR;

  csRef<iImage> Image = LoadImage (buf, Format);
  if (!Image)
  {
    ReportWarning (
	"crystalspace.maploader.parse.texture",
	"Couldn't load image. Using error texture instead!");
    //Image = csCreateXORPatternImage (32, 32, 5);
    Image = GenerateErrorTexture (32, 32);
    if (!Image)
      return 0;
  }

  if(img) *img=Image;

  if (!tm)
    return 0;
  
  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle = tm->RegisterTexture (Image, Flags,
      fail_reason);
  if (!TexHandle)
  {
    ReportError (
	"crystalspace.maploader.parse.texture",
	"Cannot create texture: %s",
	fail_reason->GetData ());
    return 0;
  }

  return csPtr<iTextureHandle> (TexHandle);
}

iTextureWrapper* csLoader::LoadTexture (const char *name,
	iDataBuffer* buf, int Flags, iTextureManager *tm, bool reg,
	bool create_material, bool free_image)
{
  if (!Engine)
    return 0;

  csRef<iImage> img;
  if (!tm && G3D) tm = G3D->GetTextureManager();
  csRef<iTextureHandle> TexHandle = LoadTexture (buf, Flags, tm, &img);
  if (!TexHandle)
    return 0;

  iTextureWrapper *TexWrapper =
	Engine->GetTextureList ()->NewTexture(TexHandle);
  TexWrapper->QueryObject ()->SetName (name);
  TexWrapper->SetImageFile(img);

  iMaterialWrapper* matwrap = 0;
  if (create_material)
  {
    csRef<iMaterial> material (Engine->CreateBaseMaterial (TexWrapper));
    matwrap = Engine->GetMaterialList ()->NewMaterial (material, name);
  }

  if (reg && tm)
  {
    // If we already have a texture handle then we don't register again.
    if (!TexWrapper->GetTextureHandle ())
      TexWrapper->Register (tm);
    if (free_image)
      TexWrapper->SetImageFile (0);
  }

  return TexWrapper;
}

csPtr<iImage> csLoader::LoadImage (const char* fname, int Format)
{
  csRef<iDataBuffer> buf = VFS->ReadFile (fname, false);
  return LoadImage (buf, fname, Format);
}

csPtr<iTextureHandle> csLoader::LoadTexture (const char *fname, int Flags,
	iTextureManager *tm, csRef<iImage>* img)
{
  if (!tm && G3D) tm = G3D->GetTextureManager();
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
	"Couldn't load image '%s', using error texture instead!",
	fname);
    //Image = csCreateXORPatternImage (32, 32, 5);
    Image = GenerateErrorTexture (32, 32);
    if (!Image)
      return 0;
  }

  if(img) *img=Image;

  if (!tm)
    return 0;
  
  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (Image, Flags,
	fail_reason));
  if (!TexHandle)
  {
    ReportError (
	"crystalspace.maploader.parse.texture",
	"Cannot create texture from '%s': '%s'", fname,
	fail_reason->GetData ());
    return 0;
  }

  return csPtr<iTextureHandle> (TexHandle);
}

iTextureWrapper* csLoader::LoadTexture (const char *name,
	const char *fname, int Flags, iTextureManager *tm, bool reg,
	bool create_material, bool free_image, iCollection* collection,
  uint keepFlags)
{
  if (!Engine)
    return 0;

  csRef<iImage> img;
  if (!tm && G3D) tm = G3D->GetTextureManager();
  csRef<iTextureHandle> TexHandle = LoadTexture (fname, Flags, tm, &img);
  if (!TexHandle)
    return 0;

  iTextureWrapper *TexWrapper =
    Engine->GetTextureList ()->NewTexture(TexHandle);
  TexWrapper->QueryObject ()->SetName (name);
  TexWrapper->SetImageFile(img);
  if(collection)
  {
    collection->Add(TexWrapper->QueryObject());
  }

  iMaterialWrapper* matwrap = 0;
  if (create_material)
  {
    csRef<iMaterial> material (Engine->CreateBaseMaterial (TexWrapper));
    matwrap = Engine->GetMaterialList ()->NewMaterial (material, name);
    if(collection)
    {
      collection->Add(matwrap->QueryObject());
    }
  }

  if (reg && tm)
  {
    // If we already have a texture handle then we don't register again.
    if (!TexWrapper->GetTextureHandle ())
      TexWrapper->Register (tm);
    if (free_image)
      TexWrapper->SetImageFile (0);
  }

  return TexWrapper;
}

bool csLoader::LoadProxyTextures()
{
  // Remove all unused textures and materials.
  csWeakRefArray<iTextureWrapper> texArray;

  for(uint i=0; i<proxyTextures.GetSize(); i++)
  {
    ProxyTexture& proxTex = proxyTextures.Get(i);
    if(!proxTex.textureWrapper)
    {
      proxyTextures.DeleteIndex(i);
      i--;
      continue;
    }

    texArray.Push(proxTex.textureWrapper);
  }

  CS::Utility::UnusedResourceHelper::UnloadUnusedMaterials(Engine,
    materialArray);
  CS::Utility::UnusedResourceHelper::UnloadUnusedTextures(Engine, texArray);
  materialArray.Empty();

  // Load the remaining textures.
  iTextureManager *tm = G3D->GetTextureManager();
  size_t i = proxyTextures.GetSize();
  while (i-- > 0)
  {
    ProxyTexture& proxTex = proxyTextures.Get(i);

    if(!proxTex.textureWrapper)
    {
      continue;
    }

    csRef<iImage> img = proxTex.img->GetProxiedImage();

    csRef<iAnimatedImage> anim = scfQueryInterface<iAnimatedImage>(img);
    if (anim && anim->IsAnimated())
    {
      iLoaderPlugin* plugin = NULL;
      iBinaryLoaderPlugin* Binplug = NULL;
      iDocumentNode* defaults = NULL;

      loaded_plugins.FindPlugin(PLUGIN_TEXTURELOADER_ANIMIMG, plugin, Binplug, defaults);
      if(plugin)
      {
        TextureLoaderContext context(proxTex.textureWrapper->QueryObject()->GetName());
        context.SetClass(proxTex.textureWrapper->GetTextureClass());
        context.SetFlags(proxTex.textureWrapper->GetFlags());
        context.SetImage(img);

        csRef<iBase> b = plugin->Parse(0, 0, 0, static_cast<iBase*>(&context));
        if (b)
        {
          csWeakRef<iTextureWrapper> newTex = scfQueryInterface<iTextureWrapper>(b);
          newTex->QueryObject()->SetName(proxTex.textureWrapper->QueryObject()->GetName());
          newTex->SetTextureClass(context.GetClass());

          proxTex.textureWrapper->SetTextureHandle(newTex->GetTextureHandle());
          proxTex.textureWrapper->SetUseCallback(newTex->GetUseCallback());

          csRef<iProcTexture> ipt = scfQueryInterface<iProcTexture> (proxTex.textureWrapper);
          if(ipt)
            ipt->SetAlwaysAnimate (proxTex.always_animate);
        }
      }

    }
    else
    {
      proxTex.textureWrapper->SetImageFile (img);
      proxTex.textureWrapper->Register (tm);
    }

    if(proxTex.keyColour.do_transp)
    {
      proxTex.textureWrapper->SetKeyColor(csQint(proxTex.keyColour.colours.red * 255.99),
        csQint(proxTex.keyColour.colours.green * 255.99),
        csQint(proxTex.keyColour.colours.blue * 255.99));
    }

    if(proxTex.alphaType != csAlphaMode::alphaNone)
    {
      proxTex.textureWrapper->GetTextureHandle()->SetAlphaType(proxTex.alphaType);
    }
  }
  proxyTextures.Empty ();

  return true;
}

//----------------------------------------------------------------------------

TextureLoaderContext::TextureLoaderContext (const char* texname) :
  scfImplementationType(this)
{
  has_flags = false;
  flags = CS_TEXTURE_3D;

  has_image = false;
  image = 0;

  has_size = false;
  width = height = 128;

  TextureLoaderContext::texname = texname;
}

TextureLoaderContext::~TextureLoaderContext ()
{
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

const char* TextureLoaderContext::GetName ()
{
  return texname;
}

void TextureLoaderContext::SetClass (const char* className)
{
  texClass = className;
}

const char* TextureLoaderContext::GetClass ()
{
  return texClass;
}

//----------------------------------------------------------------------------

csBaseTextureLoader::csBaseTextureLoader (iBase *p) :
  scfImplementationType(this, p)
{
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

SCF_IMPLEMENT_FACTORY(csImageTextureLoader)

csImageTextureLoader::csImageTextureLoader (iBase *p) : 
  scfImplementationType(this, p)
{
}

csPtr<iBase> csImageTextureLoader::Parse (iDocumentNode* /*node*/, 
					  iStreamSource*,
					  iLoaderContext* /*ldr_context*/, 	
					  iBase* context,
            iStringArray* failedMeshFacts)
{
  if (!context) return 0;
  csRef<iTextureLoaderContext> ctx = csPtr<iTextureLoaderContext>
    (scfQueryInterface<iTextureLoaderContext> (context));
  if (!ctx) return 0;
  if (!ctx->HasImage() || !ctx->GetImage())
    return 0;

  csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
  if (!G3D) return 0;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return 0;
  csRef<iEngine> Engine = csQueryRegistry<iEngine> (object_reg);
  if (!Engine)
    return 0;

  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (ctx->GetImage(), 
    ctx->HasFlags() ? ctx->GetFlags() : CS_TEXTURE_3D, fail_reason));
  if (!TexHandle)
  {
    ReportError (object_reg, "crystalspace.imagetextureloader",
	"Error creating texture: %s", fail_reason->GetData ());
    return 0;
  }

  csRef<iTextureWrapper> TexWrapper;
  csRef<iThreadedLoader> itl = csQueryRegistry<iThreadedLoader>(object_reg);
  if(itl.IsValid())
  {
    TexWrapper = Engine->GetTextureList ()->CreateTexture(TexHandle);
  }
  else
  {
    TexWrapper = Engine->GetTextureList ()->NewTexture(TexHandle);
  }
  TexWrapper->SetImageFile(ctx->GetImage());

  return csPtr<iBase> (TexWrapper);
}

//----------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csCheckerTextureLoader)

csCheckerTextureLoader::csCheckerTextureLoader (iBase *p) : 
  scfImplementationType(this, p)
{
}

csPtr<iBase> csCheckerTextureLoader::Parse (iDocumentNode* node, 
					    iStreamSource*,
					    iLoaderContext* /*ldr_context*/,
					    iBase* context,
              iStringArray* failedMeshFacts)
{
  int w = 64, h = 64, depth = 6;
  csColor color (1.0f, 1.0f, 1.0f);
  csRef<iTextureLoaderContext> ctx;
  if (context)
  {
    ctx = scfQueryInterface<iTextureLoaderContext> (context);
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
	csQueryRegistry<iSyntaxService> (object_reg);
      if (synserv)
      {
	synserv->ParseColor (colorNode, color);
      }
    }
  }

  csRef<iImage> Image = csCreateXORPatternImage (w, h, depth, color.red,
  	color.green, color.blue);

  csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
  if (!G3D) return 0;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return 0;
  csRef<iEngine> Engine = csQueryRegistry<iEngine> (object_reg);
  if (!Engine)
    return 0;

  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (Image, 
    (ctx && ctx->HasFlags()) ? ctx->GetFlags() : CS_TEXTURE_3D, fail_reason));
  if (!TexHandle)
  {
    ReportError (object_reg, "crystalspace.checkerloader",
	"Error creating texture: %s", fail_reason->GetData ());
    return 0;
  }

  csRef<iTextureWrapper> TexWrapper;
  csRef<iThreadedLoader> itl = csQueryRegistry<iThreadedLoader>(object_reg);
  if(itl.IsValid())
  {
    TexWrapper = Engine->GetTextureList ()->CreateTexture(TexHandle);
  }
  else
  {
    TexWrapper = Engine->GetTextureList ()->NewTexture(TexHandle);
  }
  TexWrapper->SetImageFile (Image);

  return csPtr<iBase> (TexWrapper);
}



//----------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csCubemapTextureLoader)

csCubemapTextureLoader::csCubemapTextureLoader (iBase *p) : 
  scfImplementationType(this, p)
{
  InitTokenTable (xmltokens);
}

csPtr<iBase> csCubemapTextureLoader::Parse (iDocumentNode* node, 
					    iStreamSource*,
					    iLoaderContext* /*ldr_context*/,
					    iBase* context,
              iStringArray* failedMeshFacts)
{
  if (!context) return 0;
  csRef<iTextureLoaderContext> ctx = csPtr<iTextureLoaderContext>
    (scfQueryInterface<iTextureLoaderContext> (context));
  if (!ctx) return 0;
  
  csRef<iEngine> Engine = csQueryRegistry<iEngine> (object_reg);
  csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  csRef<iLoader> loader = csQueryRegistry<iLoader> (object_reg);
  csRef<iSyntaxService> SyntaxService = 
    csQueryRegistry<iSyntaxService> (object_reg);

  csRef<csImageCubeMapMaker> cube;
  cube.AttachNew (new csImageCubeMapMaker (ctx->GetImage()));
  if (!Engine->GetSaveableFlag()) cube->SetName (0);

  int Format = tm->GetTextureFormat ();
  const char* fname;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_POSZ:
      case XMLTOKEN_NORTH:
        fname = child->GetContentsValue ();
	if (!fname)
	{
	  SyntaxService->ReportError (
	       PLUGIN_TEXTURELOADER_CUBEMAP,
	       child, "Expected VFS filename for 'file'!");
	  return 0;
	}
      
	cube->SetSubImage (4, csRef<iImage>(loader->LoadImage (fname, Format)));
        break;
    
      case XMLTOKEN_NEGZ:
      case XMLTOKEN_SOUTH:
        fname = child->GetContentsValue ();
	if (!fname)
	{
	  SyntaxService->ReportError (
	       PLUGIN_TEXTURELOADER_CUBEMAP,
	       child, "Expected VFS filename for 'file'!");
	  return 0;
	}
      
	cube->SetSubImage (5, csRef<iImage>(loader->LoadImage (fname, Format)));
        break;
    
      case XMLTOKEN_POSX:
      case XMLTOKEN_EAST:
        fname = child->GetContentsValue ();
	if (!fname)
	{
	  SyntaxService->ReportError (
	       PLUGIN_TEXTURELOADER_CUBEMAP,
	       child, "Expected VFS filename for 'file'!");
	  return 0;
	}
      
	cube->SetSubImage (0, csRef<iImage>(loader->LoadImage (fname, Format)));
        break;
    
      case XMLTOKEN_NEGX:
      case XMLTOKEN_WEST:
        fname = child->GetContentsValue ();
	if (!fname)
	{
	  SyntaxService->ReportError (
	       PLUGIN_TEXTURELOADER_CUBEMAP,
	       child, "Expected VFS filename for 'file'!");
	  return 0;
	}
      
	cube->SetSubImage (1, csRef<iImage>(loader->LoadImage (fname, Format)));
        break;
    
      case XMLTOKEN_POSY:
      case XMLTOKEN_TOP:
        fname = child->GetContentsValue ();
	if (!fname)
	{
	  SyntaxService->ReportError (
	       PLUGIN_TEXTURELOADER_CUBEMAP,
	       child, "Expected VFS filename for 'file'!");
	  return 0;
	}
      
	cube->SetSubImage (2, csRef<iImage>(loader->LoadImage (fname, Format)));
        break;
    
      case XMLTOKEN_NEGY:
      case XMLTOKEN_BOTTOM:
        fname = child->GetContentsValue ();
	if (!fname)
	{
	  SyntaxService->ReportError (
	       PLUGIN_TEXTURELOADER_CUBEMAP,
	       child, "Expected VFS filename for 'file'!");
	  return 0;
	}
      
	cube->SetSubImage (3, csRef<iImage>(loader->LoadImage (fname, Format)));
        break;
    }
  }


  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (cube, 
    ctx->HasFlags() ? ctx->GetFlags() : CS_TEXTURE_3D, fail_reason));
  if (!TexHandle)
  {
    ReportError (object_reg, "crystalspace.checkertextureloader",
	"Error creating texture: %s", fail_reason->GetData ());
    return 0;
  }

  csRef<iTextureWrapper> TexWrapper;
	csRef<iThreadedLoader> itl = csQueryRegistry<iThreadedLoader>(object_reg);
  if(itl.IsValid())
  {
    TexWrapper = Engine->GetTextureList ()->CreateTexture(TexHandle);
  }
  else
  {
    TexWrapper = Engine->GetTextureList ()->NewTexture(TexHandle);
  }
  TexWrapper->SetImageFile (cube);

  return csPtr<iBase> (TexWrapper);
}

//----------------------------------------------------------------------------

SCF_IMPLEMENT_FACTORY(csTexture3DLoader)

csTexture3DLoader::csTexture3DLoader (iBase *p) :
  scfImplementationType(this, p)
{
  InitTokenTable (xmltokens);
}

csPtr<iBase> csTexture3DLoader::Parse (iDocumentNode* node, 
				       iStreamSource*,
				       iLoaderContext* /*ldr_context*/,
				       iBase* context,
               iStringArray* failedMeshFacts)
{
  if (!context) return 0;
  csRef<iTextureLoaderContext> ctx = csPtr<iTextureLoaderContext>
    (scfQueryInterface<iTextureLoaderContext> (context));
  if (!ctx) return 0;
  
  csRef<iEngine> Engine = csQueryRegistry<iEngine> (object_reg);
  csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  csRef<iLoader> loader = csQueryRegistry<iLoader> (object_reg);
  csRef<iSyntaxService> SyntaxService = 
    csQueryRegistry<iSyntaxService> (object_reg);

  int Format = tm->GetTextureFormat ();
  csRef<csImageVolumeMaker> vol;
  int w = -1, h = -1;
  if (ctx->HasSize())
  {
    ctx->GetSize (w, h);
    vol.AttachNew (new csImageVolumeMaker (Format, w, h));
  }
  else if (ctx->HasImage())
  {
    vol.AttachNew (new csImageVolumeMaker (ctx->GetImage()));
  }
  else
    vol.AttachNew (new csImageVolumeMaker (Format));

  if (!Engine->GetSaveableFlag()) vol->SetName (0);

  const char* fname;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_LAYER:
        fname = child->GetContentsValue ();
	if (!fname)
	{
	  SyntaxService->ReportError (
	    PLUGIN_TEXTURELOADER_TEX3D,
	       child, "Expected VFS filename for 'file'!");
	  return 0;
	}
      
	vol->AddImage (csRef<iImage>(loader->LoadImage (fname, Format)));
        break;
    }
  }


  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (vol, 
    ctx->HasFlags() ? ctx->GetFlags() : CS_TEXTURE_3D, fail_reason));
  if (!TexHandle)
  {
    ReportError (object_reg, "crystalspace.3dtextureloader",
	"Error creating texture: %s", fail_reason->GetData ());
    return 0;
  }

  csRef<iTextureWrapper> TexWrapper;
	csRef<iThreadedLoader> itl = csQueryRegistry<iThreadedLoader>(object_reg);
  if(itl.IsValid())
  {
    TexWrapper = Engine->GetTextureList ()->CreateTexture(TexHandle);
  }
  else
  {
    TexWrapper = Engine->GetTextureList ()->NewTexture(TexHandle);
  }
  TexWrapper->SetImageFile (vol);

  return csPtr<iBase> (TexWrapper);
}


//----------------------------------------------------------------------------

csMissingTextureLoader::csMissingTextureLoader (iBase *p) : 
  scfImplementationType(this, p)
{
}

csPtr<iBase> csMissingTextureLoader::Parse (iDocumentNode* node, 
                                            iStreamSource*,
                                            iLoaderContext* /*ldr_context*/,
                                            iBase* context,
                                            iStringArray* failedMeshFacts)
{
  int width = 64, height = 64;
  csRef<iTextureLoaderContext> ctx;
  if (context)
  {
    ctx = scfQueryInterface<iTextureLoaderContext> (context);
    if (ctx)
    {
      if (ctx->HasSize())
      {
        ctx->GetSize (width, height);

        // Make square and next lower pow2, and at least 2^4 = 16
        int a = csLog2 (width), b = csLog2 (height);
        int newP = csMax (csMin (a, b), 4);
        width = height = 1 << newP;        
      }
    }
  }
 
  csRef<iImage> image = GenerateErrorTexture (width, height);


  csRef<iGraphics3D> G3D = csQueryRegistry<iGraphics3D> (object_reg);
  if (!G3D) return 0;
  csRef<iTextureManager> tm = G3D->GetTextureManager();
  if (!tm) return 0;
  csRef<iEngine> Engine = csQueryRegistry<iEngine> (object_reg);
  if (!Engine)
    return 0;

  csRef<scfString> fail_reason;
  fail_reason.AttachNew (new scfString ());
  csRef<iTextureHandle> TexHandle (tm->RegisterTexture (image, 
    (ctx && ctx->HasFlags()) ? ctx->GetFlags() : CS_TEXTURE_3D, fail_reason));
  if (!TexHandle)
  {
    ReportError (object_reg, "crystalspace.missingtextureloader",
      "Error creating texture: %s", fail_reason->GetData ());
    return 0;
  }

  csRef<iTextureWrapper> TexWrapper ;
  csRef<iThreadedLoader> itl = csQueryRegistry<iThreadedLoader>(object_reg);
  if(itl.IsValid())
  {
    TexWrapper = Engine->GetTextureList ()->CreateTexture(TexHandle);
  }
  else
  {
    TexWrapper = Engine->GetTextureList ()->NewTexture(TexHandle);
  }
  TexWrapper->SetImageFile (image);

  return csPtr<iBase> (TexWrapper);
}

}
CS_PLUGIN_NAMESPACE_END(csparser)
